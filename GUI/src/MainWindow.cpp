#include <GUI/include/MainWindow.hpp>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/DebugWindows/BackgroundViewerWindow.hpp>
#include <GUI/include/DebugWindows/CpuDebuggerWindow.hpp>
#include <GUI/include/DebugWindows/RegisterViewerWindow.hpp>
#include <GUI/include/DebugWindows/SpriteViewerWindow.hpp>
#include <GUI/include/GBA.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>
#include <SDL2/SDL.h>

namespace
{
constexpr std::array<std::pair<std::string, u32>, 6> EMU_SPEEDS = {{
    {"1/4x", 16'777'216 / 4},
    {"1/2x", 16'777'216 / 2},
    {"1x", 16'777'216},
    {"2x", 16'777'216 * 2},
    {"3x", 16'777'216 * 3},
    {"4x", 16'777'216 * 4}
}};

/// @brief Audio callback for SDL audio thread.
/// @param stream Pointer to buffer to store audio samples in.
/// @param len Size of buffer in bytes.
void AudioCallback(void*, u8* stream, int len)
{
    gba_api::FillAudioBuffer(stream, len);
}
}  // namespace

namespace gui
{
MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    currentRomPath_(""),
    emuThread_(this),
    stepFrameMode_(false),
    screen_(this),
    fpsTimer_(this),
    romTitle_("Advanced Boy")
{
    // Initialize screen
    setCentralWidget(&screen_);

    // Initialize menus
    InitializeMenuBar();

    // Temporarily set scale to 4x manually
    int width = 240 * 4;
    int height = (160 * 4) + menuBar()->height();
    resize(width, height);

    // Initialize SDL
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec audioSpec = {};
    audioSpec.freq = 32768;
    audioSpec.format = AUDIO_F32SYS;
    audioSpec.channels = 2;
    audioSpec.samples = 256;
    audioSpec.callback = &AudioCallback;
    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);

    // Window title
    connect(&fpsTimer_, &QTimer::timeout, this, &MainWindow::UpdateWindowTitle);
    fpsTimer_.start(1000);

    // Debug options
    bgViewerWindow_ = std::make_unique<BackgroundViewerWindow>();
    spriteViewerWindow_ = std::make_unique<SpriteViewerWindow>();
    cpuDebuggerWindow_ = std::make_unique<CpuDebuggerWindow>();
    registerViewerWindow_ = std::make_unique<RegisterViewerWindow>();

    // Connect signals
    connect(this, &MainWindow::UpdateBackgroundViewSignal,
            bgViewerWindow_.get(), &BackgroundViewerWindow::UpdateBackgroundViewSlot);

    connect(this, &MainWindow::UpdateSpriteViewerSignal,
            spriteViewerWindow_.get(), &SpriteViewerWindow::UpdateSpriteViewerSlot);

    connect(this, &MainWindow::UpdateCpuDebuggerSignal,
            cpuDebuggerWindow_.get(), &CpuDebuggerWindow::UpdateCpuDebuggerSlot);

    connect(this, &MainWindow::UpdateRegisterViewerSignal,
            registerViewerWindow_.get(), &RegisterViewerWindow::UpdateRegisterViewSlot);

    connect (cpuDebuggerWindow_.get(), &CpuDebuggerWindow::CpuDebugStepSignal,
             this, &MainWindow::CpuDebugStepSlot);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Slots
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::CpuDebugStepSlot(StepType stepType)
{
    switch (stepType)
    {
        case StepType::Run:
            ResumeEmulation();
            break;
        case StepType::CpuStep:
            StopEmulationThreads();
            gba_api::StepCPU();
            emit UpdateBackgroundViewSignal(false);
            emit UpdateSpriteViewerSignal(false);
            emit UpdateCpuDebuggerSignal();
            emit UpdateRegisterViewerSignal();
            break;
        case StepType::FrameStep:
            StopEmulationThreads();
            stepFrameMode_ = true;
            emuThread_.StartEmulator(StepType::FrameStep);
            break;
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Event handlers
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    fs::path romPath = "";

    for (auto& url : event->mimeData()->urls())
    {
        if (url.isLocalFile())
        {
            romPath = url.toLocalFile().toStdString();

            if (romPath.has_extension() && (romPath.extension() == ".gba"))
            {
                break;
            }
        }
    }

    StartEmulation(romPath);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event)
    {
        pressedKeys_.insert(event->key());
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event)
    {
        pressedKeys_.erase(event->key());
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    StopEmulationThreads();
    gba_api::PowerOff();

    bgViewerWindow_->close();
    cpuDebuggerWindow_->close();
    registerViewerWindow_->close();
    spriteViewerWindow_->close();

    event->accept();
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Window management
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RefreshScreen()
{
    screen_.update();
    SendKeyPresses();
}

void MainWindow::UpdateWindowTitle()
{
    std::string title;

    if (romTitle_ == "Advanced Boy")
    {
        title = "Advanced Boy";
    }
    else
    {
        title = std::format("{} ({} fps)", romTitle_, gba_api::GetFPSCounter());
    }

    setWindowTitle(QString::fromStdString(title));
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Callbacks
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::VBlankCallback()
{
    RefreshScreen();
    emit UpdateBackgroundViewSignal(true);
    emit UpdateSpriteViewerSignal(true);
    emit UpdateRegisterViewerSignal();

    if (stepFrameMode_)
    {
        stepFrameMode_ = false;
        pauseButton_->setChecked(true);
        emit UpdateCpuDebuggerSignal();
    }
}

void MainWindow::BreakpointCallback()
{
    if (cpuDebuggerWindow_->isVisible())
    {
        PauseEmulation();
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Emulation management
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::StartEmulation(fs::path romPath, bool ignoreCurrentPath)
{
    if (!ignoreCurrentPath && (currentRomPath_ == romPath))
    {
        return;
    }

    StopEmulationThreads();
    gba_api::PowerOff();
    gba_api::InitializeGBA(settings_.GetBiosPath(),
                           romPath,
                           settings_.GetSaveDirectory(),
                           std::bind(&MainWindow::VBlankCallback, this),
                           std::bind(&MainWindow::BreakpointCallback, this));

    currentRomPath_ = romPath;
    romTitle_ = gba_api::GetTitle();
    settings_.AddRecentRom(romPath);
    PopulateRecentsMenu();
    UpdateSaveStateActions(gba_api::GetSavePath());
    emit UpdateBackgroundViewSignal(true);
    emit UpdateSpriteViewerSignal(true);
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();

    if (!pauseButton_->isChecked())
    {
        StartEmulationThreads();
    }
}

void MainWindow::SendKeyPresses()
{
    // Hard code gamepad bindings for now
    // WASD     -> Directions keys
    // A        -> L
    // B        -> K
    // Start    -> Return
    // Select   -> Backspace
    // L        -> Q
    // R        -> E

    KEYINPUT keyinput;
    u16 defaultKeyInput = KEYINPUT::DEFAULT_KEYPAD_STATE;
    std::memcpy(&keyinput, &defaultKeyInput, sizeof(KEYINPUT));

    if (pressedKeys_.contains(87)) keyinput.Up = 0;
    if (pressedKeys_.contains(65)) keyinput.Left = 0;
    if (pressedKeys_.contains(83)) keyinput.Down = 0;
    if (pressedKeys_.contains(68)) keyinput.Right = 0;

    if (pressedKeys_.contains(16777220)) keyinput.Start = 0;
    if (pressedKeys_.contains(16777219)) keyinput.Select = 0;

    if (pressedKeys_.contains(76)) keyinput.A = 0;
    if (pressedKeys_.contains(75)) keyinput.B = 0;

    if (pressedKeys_.contains(81)) keyinput.L = 0;
    if (pressedKeys_.contains(69)) keyinput.R = 0;

    gba_api::UpdateKeypad(keyinput);
}

void MainWindow::StartEmulationThreads()
{
    if (!emuThread_.isRunning())
    {
        emuThread_.StartEmulator(StepType::Run);
        SDL_UnlockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 0);
    }
}

void MainWindow::StopEmulationThreads()
{
    if (emuThread_.isRunning())
    {
        SDL_LockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 1);
        emuThread_.requestInterruption();
        emuThread_.wait();
    }
}

void MainWindow::PauseEmulation()
{
    StopEmulationThreads();
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();
    pauseButton_->setChecked(true);
}

void MainWindow::ResumeEmulation()
{
    StartEmulationThreads();
    pauseButton_->setChecked(false);
}

void MainWindow::SetEmulationSpeed(u32 cpuClockSpeed)
{
    gba_api::SetCpuClockSpeed(cpuClockSpeed);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Menu bars
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitializeMenuBar()
{
    menuBar()->addMenu(CreateFileMenu());
    menuBar()->addMenu(CreateEmulationMenu());
    menuBar()->addMenu(CreateDebugMenu());
}

void MainWindow::UpdateSaveStateActions(fs::path savePath)
{
    if (savePath.empty())
    {
        return;
    }

    for (u8 i = 0; i < 5; ++i)
    {
        savePath.replace_extension(".s" + std::to_string(i));

        if (!fs::exists(savePath) || !fs::is_regular_file(savePath))
        {
            saveStateActions_[i]->setText("Save to slot " + QString::number(i + 1) + " - Empty");
            loadStateActions_[i]->setText("Load from slot " + QString::number(i + 1) + " - Empty");
        }
        else
        {
            auto time = std::chrono::clock_cast<std::chrono::system_clock>(
                            std::chrono::time_point_cast<std::chrono::seconds>(
                                fs::last_write_time(savePath)));
            std::string timeStr = std::format("{0:%D} {0:%H}:{0:%M}:{0:%S}", time);
            saveStateActions_[i]->setText("Save to slot " + QString::number(i + 1) + " - " + QString::fromStdString(timeStr));
            loadStateActions_[i]->setText("Load from slot " + QString::number(i + 1) + " - " + QString::fromStdString(timeStr));
        }
    }
}

QMenu* MainWindow::CreateFileMenu()
{
    QMenu* fileMenu = new QMenu("File");

    QAction* loadRomAction = new QAction("Load ROM");
    connect(loadRomAction, &QAction::triggered, this, &MainWindow::OpenLoadRomDialog);
    fileMenu->addAction(loadRomAction);

    recentsMenu_ = new QMenu("Recent");
    PopulateRecentsMenu();
    fileMenu->addMenu(recentsMenu_);

    fileMenu->addSeparator();
    QAction* quitAction = new QAction("Exit");
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(quitAction);

    return fileMenu;
}

void MainWindow::PopulateRecentsMenu()
{
    recentsMenu_->clear();
    auto recentRoms = settings_.GetRecentRoms();

    for (fs::path romPath : recentRoms)
    {
        QAction* action = new QAction(QString::fromStdString(romPath.string()));
        connect(action, &QAction::triggered, this, [=, this] () { this->StartEmulation(romPath); });
        recentsMenu_->addAction(action);
    }

    recentsMenu_->addSeparator();
    QAction* clearAction = new QAction("Clear");
    connect(clearAction, &QAction::triggered, this, &MainWindow::ClearRecentsMenu);
    recentsMenu_->addAction(clearAction);
}

void MainWindow::ClearRecentsMenu()
{
    settings_.ClearRecentRoms();
    PopulateRecentsMenu();
}

QMenu* MainWindow::CreateEmulationMenu()
{
    QMenu* emulationMenu = new QMenu("Emulation");

    // Pause
    pauseButton_ = new QAction("Pause");
    pauseButton_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    pauseButton_->setCheckable(true);
    connect(pauseButton_, &QAction::triggered, this, &MainWindow::PauseButtonAction);
    emulationMenu->addAction(pauseButton_);

    // Emulation speed
    QMenu* speedMenu = new QMenu("Emulation Speed");
    QActionGroup* speedGroup = new QActionGroup(speedMenu);

    for (auto [str, speed] : EMU_SPEEDS)
    {
        QAction* speedAction = new QAction(QString::fromStdString(str), speedGroup);
        speedAction->setCheckable(true);
        speedAction->setChecked(str == "1x");
        connect(speedAction, &QAction::triggered, this, [=, this] () { this->SetEmulationSpeed(speed); });
        speedMenu->addAction(speedAction);
    }

    emulationMenu->addMenu(speedMenu);
    emulationMenu->addSeparator();

    // Save states
    QMenu* saveStateMenu = new QMenu("Save State");
    QMenu* loadStateMenu = new QMenu("Load State");

    for (u8 i = 0; i < 5; ++i)
    {
        Qt::Key shortcutKey = static_cast<Qt::Key>(static_cast<u32>(Qt::Key_1) + i);

        saveStateActions_[i] = new QAction("Save to slot " + QString::number(i + 1) + " - Empty");
        saveStateActions_[i]->setShortcut(QKeySequence(shortcutKey));
        connect(saveStateActions_[i], &QAction::triggered, this, [=, this] () { this->SaveState(i); });
        saveStateMenu->addAction(saveStateActions_[i]);

        loadStateActions_[i] = new QAction("Load from slot " + QString::number(i + 1) + " - Empty");
        loadStateActions_[i]->setShortcut(QKeySequence(Qt::CTRL | shortcutKey));
        connect(loadStateActions_[i], &QAction::triggered, this, [=, this] () { this->LoadState(i); });
        loadStateMenu->addAction(loadStateActions_[i]);
    }

    emulationMenu->addMenu(saveStateMenu);
    emulationMenu->addMenu(loadStateMenu);

    return emulationMenu;
}

QMenu* MainWindow::CreateDebugMenu()
{
    QMenu* debugMenu = new QMenu("Debug");

    QAction* bgViewer = new QAction("View BG Maps");
    connect(bgViewer, &QAction::triggered, this, &MainWindow::OpenBgMapsWindow);
    debugMenu->addAction(bgViewer);

    QAction* spriteViewer = new QAction("View Sprites");
    connect(spriteViewer, &QAction::triggered, this, &MainWindow::OpenSpriteViewerWindow);
    debugMenu->addAction(spriteViewer);

    QAction* cpuDebugger = new QAction("CPU Debugger");
    connect(cpuDebugger, &QAction::triggered, this, &MainWindow::OpenCpuDebuggerWindow);
    debugMenu->addAction(cpuDebugger);

    QAction* registerViewer = new QAction("View I/O Registers");
    connect(registerViewer, &QAction::triggered, this, &MainWindow::OpenRegisterViewerWindow);
    debugMenu->addAction(registerViewer);

    return debugMenu;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Actions
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::PauseButtonAction()
{
    if (pauseButton_->isChecked())
    {
        PauseEmulation();
    }
    else
    {
        ResumeEmulation();
    }
}

void MainWindow::OpenBgMapsWindow()
{
    bgViewerWindow_->show();
    emit UpdateBackgroundViewSignal(true);
}

void MainWindow::OpenSpriteViewerWindow()
{
    spriteViewerWindow_->show();
    emit UpdateSpriteViewerSignal(true);
}

void MainWindow::OpenCpuDebuggerWindow()
{
    if (cpuDebuggerWindow_->isVisible())
    {
        return;
    }

    PauseEmulation();
    cpuDebuggerWindow_->show();
    emit UpdateCpuDebuggerSignal();
}

void MainWindow::OpenRegisterViewerWindow()
{
    registerViewerWindow_->show();
    emit UpdateRegisterViewerSignal();
}

void MainWindow::OpenLoadRomDialog()
{
    auto romFileDialog = QFileDialog(this);
    romFileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    QString startingDir = QString::fromStdString(settings_.GetFileDialogPath().string());
    fs::path romPath = romFileDialog.getOpenFileName(this, "Select ROM...", startingDir, "GBA (*.gba)").toStdString();

    if (fs::exists(romPath) && fs::is_regular_file(romPath))
    {
        settings_.SetFileDialogPath(romPath.parent_path());
        StartEmulation(romPath);
    }
}

void MainWindow::SaveState(u8 index)
{
    fs::path savePath = gba_api::GetSavePath();
    savePath.replace_extension(".s" + std::to_string(index));

    if (savePath.empty())
    {
        return;
    }

    std::ofstream saveState(savePath, std::ios::binary);

    if (saveState.fail())
    {
        return;
    }

    StopEmulationThreads();
    gba_api::StepFrame();
    gba_api::CreateSaveState(saveState);
    UpdateSaveStateActions(savePath);
    StartEmulationThreads();
}

void MainWindow::LoadState(u8 index)
{
    fs::path savePath = gba_api::GetSavePath();
    savePath.replace_extension(".s" + std::to_string(index));

    if (savePath.empty())
    {
        return;
    }

    std::ifstream saveState(savePath, std::ios::binary);

    if (saveState.fail())
    {
        return;
    }

    StopEmulationThreads();
    gba_api::LoadSaveState(saveState);
    StartEmulationThreads();
}
}  // namespace gui
