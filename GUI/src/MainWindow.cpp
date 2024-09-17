#include <GUI/include/MainWindow.hpp>
#include <cstring>
#include <filesystem>
#include <functional>
#include <set>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/DebugWindows/BackgroundViewerWindow.hpp>
#include <GUI/include/DebugWindows/CpuDebuggerWindow.hpp>
#include <GUI/include/DebugWindows/RegisterViewerWindow.hpp>
#include <GUI/include/GBA.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>
#include <SDL2/SDL.h>

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

    // Paths
    biosPath_ = "../bios/gba_bios.bin";

    // Window title
    connect(&fpsTimer_, &QTimer::timeout, this, &MainWindow::UpdateWindowTitle);
    fpsTimer_.start(1000);

    // Debug options
    bgViewerWindow_ = new BackgroundViewerWindow;
    cpuDebuggerWindow_ = new CpuDebuggerWindow;
    registerViewerWindow_ = new RegisterViewerWindow;

    // Connect signals
    connect(this, &MainWindow::UpdateBackgroundViewSignal,
            bgViewerWindow_, &BackgroundViewerWindow::UpdateBackgroundViewSlot);

    connect(this, &MainWindow::UpdateCpuDebuggerSignal,
            cpuDebuggerWindow_, &CpuDebuggerWindow::UpdateCpuDebuggerSlot);

    connect(this, &MainWindow::UpdateRegisterViewerSignal,
            registerViewerWindow_, &RegisterViewerWindow::UpdateRegisterViewSlot);

    connect (cpuDebuggerWindow_, &CpuDebuggerWindow::CpuDebugStepSignal,
             this, &MainWindow::CpuDebugStepSlot);
}

MainWindow::~MainWindow()
{
    delete bgViewerWindow_;
    delete cpuDebuggerWindow_;
    delete registerViewerWindow_;
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
            InterruptEmuThread();
            gba_api::StepCPU();
            emit UpdateBackgroundViewSignal();
            emit UpdateCpuDebuggerSignal();
            emit UpdateRegisterViewerSignal();
            break;
        case StepType::FrameStep:
            InterruptEmuThread();
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
    if (emuThread_.isRunning())
    {
        SDL_LockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 1);
        emuThread_.requestInterruption();
        emuThread_.wait();
    }

    gba_api::PowerOff();

    bgViewerWindow_->close();
    cpuDebuggerWindow_->close();
    registerViewerWindow_->close();

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
    emit UpdateBackgroundViewSignal();

    if (stepFrameMode_)
    {
        stepFrameMode_ = false;
        pauseAction_->setChecked(true);
        emit UpdateCpuDebuggerSignal();
        emit UpdateRegisterViewerSignal();
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

void MainWindow::StartEmulation(fs::path romPath)
{
    InterruptEmuThread();
    SDL_LockAudioDevice(audioDevice_);
    SDL_PauseAudioDevice(audioDevice_, 1);
    gba_api::PowerOff();
    gba_api::InitializeGBA(biosPath_,
                           romPath,
                           std::bind(&MainWindow::VBlankCallback, this),
                           std::bind(&MainWindow::BreakpointCallback, this));
    romTitle_ = gba_api::GetTitle();

    emit UpdateBackgroundViewSignal();
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();

    if (!pauseAction_->isChecked())
    {
        emuThread_.StartEmulator(StepType::Run);
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

void MainWindow::InterruptEmuThread()
{
    if (emuThread_.isRunning())
    {
        emuThread_.requestInterruption();
        emuThread_.wait();
    }
}

void MainWindow::PauseEmulation()
{
    InterruptEmuThread();
    emit UpdateCpuDebuggerSignal();
    emit UpdateRegisterViewerSignal();
    pauseAction_->setChecked(true);
}

void MainWindow::ResumeEmulation()
{
    if (!emuThread_.isRunning())
    {
        emuThread_.StartEmulator(StepType::Run);
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

    QAction* registerViewer = new QAction("View I/O Registers", this);
    connect(registerViewer, &QAction::triggered, this, &MainWindow::OpenRegisterViewerWindow);
    debugMenu_->addAction(registerViewer);
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
    bgViewerWindow_->show();
    emit UpdateBackgroundViewSignal();
}

void MainWindow::OpenCpuDebugger()
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
}  // namespace gui
