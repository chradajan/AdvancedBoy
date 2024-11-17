#include <GUI/include/MainWindow.hpp>
#include <array>
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/Bindings.hpp>
#include <GUI/include/DebugWindows/BackgroundViewerWindow.hpp>
#include <GUI/include/DebugWindows/CpuDebuggerWindow.hpp>
#include <GUI/include/DebugWindows/RegisterViewerWindow.hpp>
#include <GUI/include/DebugWindows/SpriteViewerWindow.hpp>
#include <GUI/include/EmuThread.hpp>
#include <GUI/include/GamepadListener.hpp>
#include <GUI/include/GBA.hpp>
#include <GUI/include/LCD.hpp>
#include <GUI/include/Settings/OptionsWindow.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QtWidgets>
#include <SDL2/SDL.h>

namespace
{
static const std::array<std::pair<std::string, u32>, 6> EMU_SPEEDS = {{
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
    stepFrameMode_(false),
    fpsTimer_(this),
    romTitle_("Advanced Boy"),
    gamepad_(nullptr)
{
    // Initialize main window
    screen_ = new LCD(this);
    setCentralWidget(screen_);

    InitializeMenuBar();

    int width = 240 * 4;
    int height = (160 * 4) + menuBar()->height();
    resize(width, height);

    connect(&fpsTimer_, &QTimer::timeout, this, &MainWindow::UpdateWindowTitle);
    fpsTimer_.start(1000);

    // Initialize SDL
    SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");
    SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);

    SDL_AudioSpec audioSpec = {};
    audioSpec.freq = 32768;
    audioSpec.format = AUDIO_F32SYS;
    audioSpec.channels = 2;
    audioSpec.samples = 256;
    audioSpec.callback = &AudioCallback;
    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);

    // Debug windows
    bgViewerWindow_ = std::make_unique<BackgroundViewerWindow>();
    connect(this, &MainWindow::UpdateBackgroundViewSignal,
            bgViewerWindow_.get(), &BackgroundViewerWindow::UpdateBackgroundViewSlot);

    spriteViewerWindow_ = std::make_unique<SpriteViewerWindow>();
    connect(this, &MainWindow::UpdateSpriteViewerSignal,
            spriteViewerWindow_.get(), &SpriteViewerWindow::UpdateSpriteViewerSlot);

    cpuDebuggerWindow_ = std::make_unique<CpuDebuggerWindow>();
    connect(this, &MainWindow::UpdateCpuDebuggerSignal,
            cpuDebuggerWindow_.get(), &CpuDebuggerWindow::UpdateCpuDebuggerSlot);
    connect (cpuDebuggerWindow_.get(), &CpuDebuggerWindow::CpuDebugStepSignal,
             this, &MainWindow::CpuDebugStepSlot);

    registerViewerWindow_ = std::make_unique<RegisterViewerWindow>();
    connect (cpuDebuggerWindow_.get(), &CpuDebuggerWindow::CpuDebugStepSignal,
             this, &MainWindow::CpuDebugStepSlot);

    // Options window
    optionsWindow_ = std::make_unique<OptionsWindow>(settings_);
    connect(optionsWindow_.get(), &OptionsWindow::UpdateAudioSignal, this, &MainWindow::UpdateAudioSlot);
    connect(optionsWindow_.get(), &OptionsWindow::SetGamepadSignal, this, &MainWindow::SetGamepadSlot);
    connect(optionsWindow_.get(), &OptionsWindow::BindingsChangedSignal, this, &MainWindow::BindingsChangedSlot);
    connect(optionsWindow_.get(), &OptionsWindow::TimeFormatChangedSignal, this, &MainWindow::TimeFormatChangedSlot);

    // Gamepad setup
    gamepad_ = optionsWindow_->GetGamepad();
    gamepadMap_ = settings_.GetGamepadMap();
    gamepadListener_ = new GamepadListener(this);

    // Gamepad related signals
    connect(gamepadListener_, &GamepadListener::GamepadConnectedSignal,
            optionsWindow_.get(), &OptionsWindow::UpdateGamepadTabSlot);
    connect(gamepadListener_, &GamepadListener::GamepadDisconnectedSignal,
            optionsWindow_.get(), &OptionsWindow::UpdateGamepadTabSlot);

    connect(optionsWindow_.get(), &OptionsWindow::GetNewGamepadBindingSignal,
            gamepadListener_, &GamepadListener::GetNewKeyBindingSlot);
    connect(gamepadListener_, &GamepadListener::SetNewGamepadBindingSignal,
            optionsWindow_.get(), &OptionsWindow::SetNewGamepadBindingSignal);

    gamepadListener_->start();

    // Create emulation thread
    emuThread_ = new EmuThread(this);
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
            emuThread_->StartEmulator(StepType::FrameStep);
            break;
    }
}

void MainWindow::UpdateAudioSlot(PersistentData::AudioSettings audioSettings)
{
    gba_api::SetVolume(audioSettings.mute, audioSettings.volume);
    gba_api::SetAPUChannels(audioSettings.channel1,
                            audioSettings.channel2,
                            audioSettings.channel3,
                            audioSettings.channel4,
                            audioSettings.fifoA,
                            audioSettings.fifoB);
}

void MainWindow::TimeFormatChangedSlot()
{
    UpdateSaveStateActions(gba_api::GetSavePath());
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
    pressedKeys_.insert(event->key());
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    pressedKeys_.erase(event->key());
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    gamepad_ = nullptr;
    StopEmulationThreads();
    gba_api::PowerOff();

    if (gamepadListener_->isRunning())
    {
        gamepadListener_->requestInterruption();
        gamepadListener_->wait();
    }

    bgViewerWindow_->close();
    cpuDebuggerWindow_->close();
    registerViewerWindow_->close();
    spriteViewerWindow_->close();
    optionsWindow_->close();

    QMainWindow::closeEvent(event);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Window management
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RefreshScreen()
{
    screen_->update();
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

    fs::path biosPath = settings_.GetBiosPath();

    if (!fs::exists(biosPath))
    {
        QMessageBox::critical(this, "BIOS Error", "No BIOS file. Please provide a BIOS file in the options.", QMessageBox::Close);
        return;
    }

    StopEmulationThreads();
    gba_api::PowerOff();

    gba_api::InitializeGBA(settings_.GetBiosPath(),
                           romPath,
                           settings_.GetSaveDirectory(),
                           std::bind(&MainWindow::VBlankCallback, this),
                           std::bind(&MainWindow::BreakpointCallback, this),
                           settings_.SkipBiosIntro());

    if (!gba_api::ValidBiosLoaded())
    {
        QMessageBox::critical(this, "BIOS Error", "The provided BIOS file could not be loaded.", QMessageBox::Close);
        gba_api::PowerOff();
        currentRomPath_ = "";
        romTitle_ = "Advanced Boy";
        UpdateSaveStateActions("");
        emit UpdateBackgroundViewSignal(true);
        emit UpdateSpriteViewerSignal(true);
        emit UpdateCpuDebuggerSignal();
        emit UpdateRegisterViewerSignal();
        return;
    }

    if (!gba_api::ValidGamePakLoaded())
    {
        QMessageBox::warning(this, "GamePak Error", "The ROM you attempted to run could not be loaded", QMessageBox::Close);
        currentRomPath_ = "";
        romTitle_ = "Advanced Boy";
    }
    else
    {
        currentRomPath_ = romPath;
        romTitle_ = gba_api::GetTitle();
        settings_.AddRecentRom(romPath);
        PopulateRecentsMenu();
    }

    UpdateAudioSlot(settings_.GetAudioSettings());
    UpdateSaveStateActions(gba_api::GetSavePath());
    emit UpdateBackgroundViewSignal(true);
    emit UpdateSpriteViewerSignal(true);
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();

    if (!pauseButton_->isChecked())
    {
        StartEmulationThreads();
    }

    restartButton_->setEnabled(true);
    powerDownButton_->setEnabled(true);
}

void MainWindow::SendKeyPresses() const
{
    static u16 defaultKeyInput = KEYINPUT::DEFAULT_KEYPAD_STATE;
    auto keyInput = MemCpyInit<KEYINPUT>(&defaultKeyInput);

    GetKeyboardInputs(keyInput);

    if (gamepad_ != nullptr)
    {
        PollController(keyInput);
    }

    gba_api::UpdateKeypad(keyInput);
}

void MainWindow::GetKeyboardInputs(KEYINPUT& keyInput) const
{
    KeyboardMap map = settings_.GetKeyboardMap();

    if (pressedKeys_.contains(map.up.first)     ||      pressedKeys_.contains(map.up.second))       keyInput.Up = 0;
    if (pressedKeys_.contains(map.down.first)   ||      pressedKeys_.contains(map.down.second))     keyInput.Down = 0;
    if (pressedKeys_.contains(map.left.first)   ||      pressedKeys_.contains(map.left.second))     keyInput.Left = 0;
    if (pressedKeys_.contains(map.right.first)  ||      pressedKeys_.contains(map.right.second))    keyInput.Right = 0;
    if (pressedKeys_.contains(map.l.first)      ||      pressedKeys_.contains(map.l.second))        keyInput.L = 0;
    if (pressedKeys_.contains(map.r.first)      ||      pressedKeys_.contains(map.r.second))        keyInput.R = 0;
    if (pressedKeys_.contains(map.a.first)      ||      pressedKeys_.contains(map.a.second))        keyInput.A = 0;
    if (pressedKeys_.contains(map.b.first)      ||      pressedKeys_.contains(map.b.second))        keyInput.B = 0;
    if (pressedKeys_.contains(map.start.first)  ||      pressedKeys_.contains(map.start.second))    keyInput.Start = 0;
    if (pressedKeys_.contains(map.select.first) ||      pressedKeys_.contains(map.select.second))   keyInput.Select = 0;
}

void MainWindow::PollController(KEYINPUT& keyInput) const
{
    SDL_GameControllerUpdate();

    if (gamepadMap_.up.first.Active(gamepad_)       ||      gamepadMap_.up.second.Active(gamepad_))      keyInput.Up = 0;
    if (gamepadMap_.down.first.Active(gamepad_)     ||      gamepadMap_.down.second.Active(gamepad_))    keyInput.Down = 0;
    if (gamepadMap_.left.first.Active(gamepad_)     ||      gamepadMap_.left.second.Active(gamepad_))    keyInput.Left = 0;
    if (gamepadMap_.right.first.Active(gamepad_)    ||      gamepadMap_.right.second.Active(gamepad_))   keyInput.Right = 0;
    if (gamepadMap_.l.first.Active(gamepad_)        ||      gamepadMap_.l.second.Active(gamepad_))       keyInput.L = 0;
    if (gamepadMap_.r.first.Active(gamepad_)        ||      gamepadMap_.r.second.Active(gamepad_))       keyInput.R = 0;
    if (gamepadMap_.a.first.Active(gamepad_)        ||      gamepadMap_.a.second.Active(gamepad_))       keyInput.A = 0;
    if (gamepadMap_.b.first.Active(gamepad_)        ||      gamepadMap_.b.second.Active(gamepad_))       keyInput.B = 0;
    if (gamepadMap_.start.first.Active(gamepad_)    ||      gamepadMap_.start.second.Active(gamepad_))   keyInput.Start = 0;
    if (gamepadMap_.select.first.Active(gamepad_)   ||      gamepadMap_.select.second.Active(gamepad_))  keyInput.Select = 0;
}

void MainWindow::StartEmulationThreads()
{
    if (!emuThread_->isRunning())
    {
        emuThread_->StartEmulator(StepType::Run);
        SDL_UnlockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 0);
    }
}

void MainWindow::StopEmulationThreads()
{
    if (emuThread_->isRunning())
    {
        SDL_LockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 1);
        emuThread_->requestInterruption();
        emuThread_->wait();
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
    auto& timezone = settings_.GetTimezone();
    bool is12H = settings_.Is12HourClock();

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
            auto fileTime = std::chrono::time_point_cast<std::chrono::seconds>(fs::last_write_time(savePath));
            auto sysTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
            auto zonedTime = std::chrono::zoned_time{&timezone, sysTime};

            // std::string timeStr = std::format("{0:%D} {0:%H}:{0:%M}:{0:%S}", zonedTime);
            std::string timeStr = is12H ? std::format("{0:%D} {0:%r}", zonedTime) : std::format("{0:%D} {0:%T}", zonedTime);
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

    QAction* optionsAction = new QAction("Options");
    connect(optionsAction, &QAction::triggered, this, [=, this] () { this->optionsWindow_->show(); });
    fileMenu->addAction(optionsAction);

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
    emulationMenu->addSeparator();

    // Restart
    restartButton_ = new QAction("Restart");
    restartButton_->setEnabled(false);
    connect(restartButton_, &QAction::triggered, this, [=, this] () { this->StartEmulation(this->currentRomPath_, true); });
    emulationMenu->addAction(restartButton_);

    // Power off
    powerDownButton_ = new QAction("Power Down");
    powerDownButton_->setEnabled(false);
    connect(powerDownButton_, &QAction::triggered, this, &MainWindow::PowerDown);
    emulationMenu->addAction(powerDownButton_);

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

void MainWindow::PowerDown()
{
    restartButton_->setEnabled(false);
    powerDownButton_->setEnabled(false);
    StopEmulationThreads();
    gba_api::PowerOff();

    currentRomPath_ = "";
    romTitle_ = "Advanced Boy";
    UpdateSaveStateActions("");

    RefreshScreen();
    emit UpdateBackgroundViewSignal(true);
    emit UpdateSpriteViewerSignal(true);
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();
}
}  // namespace gui
