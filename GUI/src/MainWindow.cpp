#include <GUI/include/MainWindow.hpp>
#include <cstring>
#include <filesystem>
#include <functional>
#include <set>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GUI/include/BackgroundViewer.hpp>
#include <GUI/include/CpuDebugger.hpp>
#include <GUI/include/GBA.hpp>
#include <SDL2/SDL.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>

namespace
{
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
    emuThread_(this),
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

    // Paths
    biosPath_ = "../bios/gba_bios.bin";
    logDir_ = "";

    // Window title
    connect(&fpsTimer_, &QTimer::timeout, this, &MainWindow::UpdateWindowTitle);
    fpsTimer_.start(1000);

    // Debug options
    bgMapsWindow_ = new BackgroundViewer;
    cpuDebugWindow_ = new CpuDebugger;

    // Connect signals
    connect(this, &MainWindow::UpdateBackgroundViewSignal,
            bgMapsWindow_, &BackgroundViewer::UpdateBackgroundViewSlot);

    connect(this, &MainWindow::UpdateCpuDebuggerSignal,
            cpuDebugWindow_, &CpuDebugger::UpdateCpuDebuggerSlot);

    connect(cpuDebugWindow_, &CpuDebugger::PauseSignal,
            this, &MainWindow::PauseSlot);

    connect(cpuDebugWindow_, &CpuDebugger::ResumeSignal,
            this, &MainWindow::ResumeSlot);

    connect(cpuDebugWindow_, &CpuDebugger::StepSignal,
            bgMapsWindow_, &BackgroundViewer::UpdateBackgroundViewSlot);
}

MainWindow::~MainWindow()
{
    delete bgMapsWindow_;
    delete cpuDebugWindow_;
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
    if (emuThread_.isRunning())
    {
        SDL_LockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 1);
        emuThread_.requestInterruption();
        emuThread_.wait();
    }

    gba_api::PowerOff();

    if (bgMapsWindow_->isVisible())
    {
        bgMapsWindow_->close();
    }

    if (cpuDebugWindow_->isVisible())
    {
        cpuDebugWindow_->close();
    }

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

void MainWindow::VBlankCallback(int)
{
    RefreshScreen();

    if (bgMapsWindow_->isVisible())
    {
        emit UpdateBackgroundViewSignal();
    }

    if (cpuDebugWindow_->isVisible() && cpuDebugWindow_->StepFrameMode())
    {
        cpuDebugWindow_->DisableStepFrameMode();
        emit cpuDebugWindow_->PauseSignal();
    }
}

void MainWindow::BreakpointCallback()
{
    if (cpuDebugWindow_->isVisible())
    {
        emit cpuDebugWindow_->PauseSignal();
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Emulation management
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::StartEmulation(fs::path romPath)
{
    if (emuThread_.isRunning())
    {
        emuThread_.requestInterruption();
        emuThread_.wait();
    }

    SDL_LockAudioDevice(audioDevice_);
    SDL_PauseAudioDevice(audioDevice_, 1);
    gba_api::PowerOff();
    gba_api::InitializeGBA(biosPath_,
                           romPath,
                           logDir_,
                           std::bind(&MainWindow::VBlankCallback, this, std::placeholders::_1),
                           std::bind(&MainWindow::BreakpointCallback, this));
    romTitle_ = gba_api::GetTitle();

    if (cpuDebugWindow_->isVisible())
    {
        emit UpdateCpuDebuggerSignal();
    }

    if (!pauseAction_->isChecked())
    {
        emuThread_.start();
    }

    SDL_UnlockAudioDevice(audioDevice_);
    SDL_PauseAudioDevice(audioDevice_, 0);
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

void MainWindow::PauseEmulation()
{
    if (emuThread_.isRunning())
    {
        emuThread_.requestInterruption();
        emuThread_.wait();

        if (cpuDebugWindow_->isVisible())
        {
            emit UpdateCpuDebuggerSignal();
        }
    }

    pauseAction_->setChecked(true);
}

void MainWindow::ResumeEmulation()
{
    if (!emuThread_.isRunning())
    {
        emuThread_.start();
    }

    pauseAction_->setChecked(false);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Menu bars
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitializeMenuBar()
{
    fileMenu_ = menuBar()->addMenu("File");
    emulationMenu_ = menuBar()->addMenu("Emulation");
    debugMenu_ = menuBar()->addMenu("Debug");
    optionsMenu_ = menuBar()->addMenu("Options");

    // Emulation
    pauseAction_ = new QAction("Pause", this);
    pauseAction_->setCheckable(true);
    connect(pauseAction_, &QAction::triggered, this, &MainWindow::PauseButtonAction);
    emulationMenu_->addAction(pauseAction_);

    // Debug
    QAction* bgMaps = new QAction("View BG Maps", this);
    connect(bgMaps, &QAction::triggered, this, &MainWindow::OpenBgMapsWindow);
    debugMenu_->addAction(bgMaps);

    QAction* cpuDebugger = new QAction("CPU Debugger", this);
    connect(cpuDebugger, &QAction::triggered, this, &MainWindow::OpenCpuDebugger);
    debugMenu_->addAction(cpuDebugger);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Actions
///---------------------------------------------------------------------------------------------------------------------------------

void MainWindow::PauseButtonAction()
{
    if (pauseAction_->isChecked())
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
    bgMapsWindow_->show();
}

void MainWindow::OpenCpuDebugger()
{
    if (cpuDebugWindow_->isVisible())
    {
        return;
    }

    PauseEmulation();
    emit UpdateCpuDebuggerSignal();
    cpuDebugWindow_->show();
}
}  // namespace gui
