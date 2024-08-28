#include <GUI/include/MainWindow.hpp>
#include <cstring>
#include <filesystem>
#include <functional>
#include <set>
#include <GBA/include/Types.hpp>
#include <GUI/include/BackgroundViewer.hpp>
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
    gui::FillAudioBuffer(stream, len);
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
    bgMapsWindow_ = nullptr;
}

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

    PowerOff();

    if (bgMapsWindow_ && bgMapsWindow_->isVisible())
    {
        bgMapsWindow_->close();
    }

    event->accept();
}

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
        title = std::format("{} ({} fps)", romTitle_, GetFPSCounter());
    }

    setWindowTitle(QString::fromStdString(title));
}

void MainWindow::VBlankCallback(int)
{
    RefreshScreen();

    if (bgMapsWindow_ && bgMapsWindow_->isVisible())
    {
        bgMapsWindow_->UpdateBackgroundView(gui::GetBgDebugInfo(bgMapsWindow_->GetSelectedBg()));
    }
}

void MainWindow::StartEmulation(fs::path romPath)
{
    if (emuThread_.isRunning())
    {
        SDL_LockAudioDevice(audioDevice_);
        SDL_PauseAudioDevice(audioDevice_, 1);
        emuThread_.requestInterruption();
        emuThread_.wait();
    }

    PowerOff();
    InitializeGBA(biosPath_, romPath, logDir_, std::bind(&MainWindow::VBlankCallback, this, std::placeholders::_1));
    romTitle_ = GetTitle();
    emuThread_.start();
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

    UpdateKeypad(keyinput);
}

void MainWindow::InitializeMenuBar()
{
    fileMenu_ = menuBar()->addMenu("File");
    emulationMenu_ = menuBar()->addMenu("Emulation");
    debugMenu_ = menuBar()->addMenu("Debug");
    optionsMenu_ = menuBar()->addMenu("Options");

    // Debug
    QAction* bgMaps = new QAction("View BG Maps", this);
    connect(bgMaps, &QAction::triggered, this, &OpenBgMapsWindow);
    debugMenu_->addAction(bgMaps);
}

void MainWindow::OpenBgMapsWindow()
{
    if (bgMapsWindow_ == nullptr)
    {
        bgMapsWindow_ = new BackgroundViewer;
    }

    bgMapsWindow_->show();
}
}  // namespace gui
