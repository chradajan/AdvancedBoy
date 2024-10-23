#include <GUI/include/Settings/OptionsWindow.hpp>
#include <GUI/include/PersistentData.hpp>
#include <GUI/include/Settings/AudioTab.hpp>
#include <GUI/include/Settings/PathsTab.hpp>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

namespace gui
{
OptionsWindow::OptionsWindow(PersistentData& settings) : settings_(settings)
{
    setWindowTitle("Options");
    QVBoxLayout* layout = new QVBoxLayout;

    // Tabs
    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->setObjectName("TabWidget");

    tabWidget->addTab(new PathsTab(settings), "Paths");

    AudioTab* audioTab = new AudioTab(settings);
    connect(audioTab, &AudioTab::UpdateAudioSignal, this, &OptionsWindow::UpdateAudioSignal);
    tabWidget->addTab(audioTab, "Audio");

    layout->addWidget(tabWidget);

    // Buttons
    auto standardButtons = QDialogButtonBox::StandardButton::Ok |
                           QDialogButtonBox::StandardButton::Cancel |
                           QDialogButtonBox::StandardButton::RestoreDefaults;
    QDialogButtonBox* buttons = new QDialogButtonBox(standardButtons, Qt::Orientation::Horizontal);
    connect(buttons->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked,
            this, [=, this] () { this->close(); });
    connect(buttons->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked,
            this, [=, this] () { this->close(); });
    connect(buttons->button(QDialogButtonBox::StandardButton::RestoreDefaults), &QPushButton::clicked,
            this, &OptionsWindow::RestoreDefaultsSlot);

    layout->addWidget(buttons);
    setLayout(layout);
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
        default:
            break;
    }
}
}
