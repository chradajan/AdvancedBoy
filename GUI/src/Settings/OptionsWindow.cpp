#include <GUI/include/Settings/OptionsWindow.hpp>
#include <GUI/include/PersistentData.hpp>
#include <GUI/include/Settings/AudioTab.hpp>
#include <GUI/include/Settings/GamepadTab.hpp>
#include <GUI/include/Settings/KeyboardTab.hpp>
#include <GUI/include/Settings/PathsTab.hpp>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

namespace gui
{
OptionsWindow::OptionsWindow(PersistentData& settings) : settings_(settings), listenForKeyPress_(false)
{
    setWindowTitle("Options");
    QVBoxLayout* layout = new QVBoxLayout;

    // Tabs
    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->setFocusPolicy(Qt::NoFocus);
    tabWidget->setObjectName("TabWidget");

    // Paths tab (index 0)
    tabWidget->addTab(new PathsTab(settings), "Paths");

    // Audio tab (index 1)
    AudioTab* audioTab = new AudioTab(settings);
    connect(audioTab, &AudioTab::UpdateAudioSignal, this, &OptionsWindow::UpdateAudioSignal);
    tabWidget->addTab(audioTab, "Audio");

    // Gamepad tab (index 2)
    GamepadTab* gamepadTab = new GamepadTab(settings);
    connect(gamepadTab, &GamepadTab::GetNewGamepadBindingSignal, this, &OptionsWindow::GetNewGamepadBindingSignal);
    connect(gamepadTab, &GamepadTab::SetGamepadSignal, this, &OptionsWindow::SetGamepadSignal);
    connect(gamepadTab, &GamepadTab::BindingsChangedSignal, this, &OptionsWindow::BindingsChangedSignal);
    connect(this, &OptionsWindow::SetNewGamepadBindingSignal, gamepadTab, &GamepadTab::SetNewGamepadBindingSlot);
    tabWidget->addTab(gamepadTab, "Gamepad");

    // Keyboard tab (index 3)
    KeyboardTab* keyboardTab = new KeyboardTab(settings);
    connect(keyboardTab, &KeyboardTab::GetNewKeyboardBindingSignal, this, &OptionsWindow::GetNewKeyboardBindingSlot);
    connect(this, &OptionsWindow::SetNewKeyboardBindingSignal, keyboardTab, &KeyboardTab::SetNewKeyboardBindingSlot);
    tabWidget->addTab(keyboardTab, "Keyboard");

    connect(tabWidget, &QTabWidget::currentChanged, this, &OptionsWindow::TabChangedSlot);
    layout->addWidget(tabWidget);

    // Buttons
    auto standardButtons = QDialogButtonBox::StandardButton::Ok |
                           QDialogButtonBox::StandardButton::Cancel |
                           QDialogButtonBox::StandardButton::RestoreDefaults;
    QDialogButtonBox* buttons = new QDialogButtonBox(standardButtons, Qt::Orientation::Horizontal);
    buttons->setFocusPolicy(Qt::NoFocus);
    connect(buttons->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked,
            this, [=, this] () { this->close(); });
    connect(buttons->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked,
            this, [=, this] () { this->close(); });
    connect(buttons->button(QDialogButtonBox::StandardButton::RestoreDefaults), &QPushButton::clicked,
            this, &OptionsWindow::RestoreDefaultsSlot);

    layout->addWidget(buttons);
    setLayout(layout);
}

SDL_GameController* OptionsWindow::GetGamepad() const
{
    return static_cast<GamepadTab*>(findChild<QTabWidget*>("TabWidget")->widget(2))->GetGamepad();
}

void OptionsWindow::UpdateGamepadTabSlot()
{
    static_cast<GamepadTab*>(findChild<QTabWidget*>("TabWidget")->widget(2))->UpdateGamepadList();
}

void OptionsWindow::RestoreDefaultsSlot()
{
    QMessageBox confirmationBox;
    confirmationBox.setWindowTitle("Restore Defaults");
    confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    QTabWidget* tabWidget = findChild<QTabWidget*>("TabWidget");
    int activeTabIndex = tabWidget->currentIndex();
    QWidget* activeTabPtr = tabWidget->currentWidget();

    switch (activeTabIndex)
    {
        case 0:
        {
            confirmationBox.setText("Restore default paths?");
            int confirmation = confirmationBox.exec();

            if (confirmation == QMessageBox::Yes)
            {
                static_cast<PathsTab*>(activeTabPtr)->RestoreDefaults();
            }

            break;
        }
        case 1:
        {
            confirmationBox.setText("Restore default audio settings?");
            int confirmation = confirmationBox.exec();

            if (confirmation == QMessageBox::Yes)
            {
                static_cast<AudioTab*>(activeTabPtr)->RestoreDefaults();
            }

            break;
        }
        case 2:
        {
            confirmationBox.setText("Restore default controller bindings?");
            int confirmation = confirmationBox.exec();

            if (confirmation == QMessageBox::Yes)
            {
                static_cast<GamepadTab*>(activeTabPtr)->RestoreDefaults();
            }

            break;
        }
        case 3:
        {
            confirmationBox.setText("Restore default keyboard bindings?");
            int confirmation = confirmationBox.exec();

            if (confirmation == QMessageBox::Yes)
            {
                static_cast<KeyboardTab*>(activeTabPtr)->RestoreDefaults();
            }

            break;
        }
        default:
            break;
    }
}

void OptionsWindow::TabChangedSlot()
{
    static_cast<GamepadTab*>(findChild<QTabWidget*>("TabWidget")->widget(2))->CancelRebind();
    static_cast<KeyboardTab*>(findChild<QTabWidget*>("TabWidget")->widget(3))->CancelRebind();
}

void OptionsWindow::closeEvent(QCloseEvent* event)
{
    static_cast<GamepadTab*>(findChild<QTabWidget*>("TabWidget")->widget(2))->CancelRebind();
    static_cast<KeyboardTab*>(findChild<QTabWidget*>("TabWidget")->widget(3))->CancelRebind();
    event->accept();
}

void OptionsWindow::keyPressEvent(QKeyEvent* event)
{
    if (listenForKeyPress_)
    {
        listenForKeyPress_ = false;
        emit SetNewKeyboardBindingSignal(static_cast<Qt::Key>(event->key()));
    }
}
}
