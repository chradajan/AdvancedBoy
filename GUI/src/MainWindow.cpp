#include <GUI/include/MainWindow.hpp>
#include <filesystem>
#include <GBA/include/Types.hpp>
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
    screenTimer_(this),
    fpsTimer_(this),
    romTitle_("Advanced Boy")
{
    // Initialize screen
    setCentralWidget(&screen_);
    screenTimer_.setTimerType(Qt::TimerType::PreciseTimer);
    connect(&screenTimer_, &QTimer::timeout, this, &RefreshScreen);
    screenTimer_.start(16);

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

    QCoreApplication* app = QApplication::instance();

    if (app->arguments().size() > 2)
    {
        biosPath_ = app->arguments().at(1).toStdString();
        fs::path romPath = app->arguments().at(2).toStdString();

        StartEmulation(romPath);
    }

    // Window title
    connect(&fpsTimer_, &QTimer::timeout, this, &UpdateWindowTitle);
    fpsTimer_.start(1000);
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
    InitializeGBA(biosPath_, romPath, logDir_);
    romTitle_ = GetTitle();
    emuThread_.start();
    SDL_UnlockAudioDevice(audioDevice_);
    SDL_PauseAudioDevice(audioDevice_, 0);
}

void MainWindow::RefreshScreen()
{
    screen_.update();
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
}  // namespace gui
