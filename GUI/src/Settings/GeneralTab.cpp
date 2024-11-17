#include <GUI/include/Settings/GeneralTab.hpp>
#include <chrono>
#include <filesystem>
#include <string>
#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace gui
{
///---------------------------------------------------------------------------------------------------------------------------------
/// Path Selector
///---------------------------------------------------------------------------------------------------------------------------------

PathSelector::PathSelector(fs::path path)
{
    QHBoxLayout* layout = new QHBoxLayout;

    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setObjectName("PathLineEdit");
    lineEdit->setReadOnly(true);
    lineEdit->setText(QString::fromStdString(path.string()));
    layout->addWidget(lineEdit);

    QPushButton* button = new QPushButton("...");
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(button, &QPushButton::clicked, this, &PathSelector::ButtonClicked);
    layout->addWidget(button);

    setMinimumWidth(400);
    setLayout(layout);
}

void PathSelector::UpdatePath(fs::path path)
{
    findChild<QLineEdit*>("PathLineEdit")->setText(QString::fromStdString(path.string()));
}

///---------------------------------------------------------------------------------------------------------------------------------
/// General Tab
///---------------------------------------------------------------------------------------------------------------------------------

GeneralTab::GeneralTab(PersistentData& settings) : settings_(settings)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(CreateOptionsGroup());
    layout->addWidget(CreatePathsGroup());
    layout->addWidget(CreateTimeGroup());
    layout->addStretch();
    setLayout(layout);
}

void GeneralTab::RestoreDefaults()
{
    fs::path biosPath = settings_.GetDefaultBiosPath();
    settings_.SetBiosPath(biosPath);
    findChild<PathSelector*>("BiosWidget")->UpdatePath(biosPath);

    fs::path saveDir = settings_.GetDefaultSaveDirectory();
    settings_.SetSaveDirectory(saveDir);
    findChild<PathSelector*>("SaveWidget")->UpdatePath(saveDir);

    settings_.RestoreDefaultTimeSettings();
    UpdateTimeWidgets(findChild<QComboBox*>("TimezonesDropdown"), findChild<QCheckBox*>("ClockFormatBox"));
    emit TimeFormatChangedSignal();
}

void GeneralTab::OpenBiosFileDialog()
{
    auto dialog = QFileDialog(this);
    dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    fs::path currBiosPath = settings_.GetBiosPath();
    auto startingDir = fs::exists(currBiosPath) ? QString::fromStdString(currBiosPath.string()) : QString();
    fs::path biosPath = dialog.getOpenFileName(this, "Select GBA Bios...", startingDir, "GBA BIOS (*.bin)").toStdString();

    if (fs::exists(biosPath) && fs::is_regular_file(biosPath))
    {
        settings_.SetBiosPath(biosPath);
        findChild<PathSelector*>("BiosWidget")->UpdatePath(biosPath);
    }
}

void GeneralTab::OpenSaveDirDialog()
{
    auto dialog = QFileDialog(this);
    dialog.setFileMode(QFileDialog::FileMode::Directory);
    fs::path currSaveDir = settings_.GetSaveDirectory();
    auto startingDir = fs::exists(currSaveDir) ? QString::fromStdString(currSaveDir.string()) : QString();
    fs::path saveDir = dialog.getExistingDirectory(this, "Select save directory...", startingDir).toStdString();

    if (fs::exists(saveDir) && fs::is_directory(saveDir))
    {
        settings_.SetSaveDirectory(saveDir);
        findChild<PathSelector*>("SaveWidget")->UpdatePath(saveDir);
    }
}

void GeneralTab::TimezoneChangedSlot(QString const& timezoneName)
{
    auto tzName = timezoneName.toStdString();

    for (auto& tz : std::chrono::get_tzdb().zones)
    {
        if (tz.name() == tzName)
        {
            settings_.SetTimezone(tz);
            break;
        }
    }

    emit TimeFormatChangedSignal();
}

void GeneralTab::ClockFormatChangedSlot(Qt::CheckState state)
{
    settings_.Set12HourClock(state == Qt::CheckState::Checked);
    emit TimeFormatChangedSignal();
}

QGroupBox* GeneralTab::CreateOptionsGroup() const
{
    QGroupBox* optionsGroup = new QGroupBox("Options");
    return optionsGroup;
}

QGroupBox* GeneralTab::CreatePathsGroup() const
{
    QGridLayout* layout = new QGridLayout;

    // Bios
    PathSelector* biosWidget = new PathSelector(settings_.GetBiosPath());
    biosWidget->setObjectName("BiosWidget");
    connect(biosWidget, &PathSelector::ButtonClicked, this, &GeneralTab::OpenBiosFileDialog);
    layout->addWidget(new QLabel("BIOS ROM"), 0, 0);
    layout->addWidget(biosWidget, 0, 1);

    // Saves
    PathSelector* saveWidget = new PathSelector(settings_.GetSaveDirectory());
    saveWidget->setObjectName("SaveWidget");
    connect(saveWidget, &PathSelector::ButtonClicked, this, &GeneralTab::OpenSaveDirDialog);
    layout->addWidget(new QLabel("Saves Directory"), 1, 0);
    layout->addWidget(saveWidget, 1, 1);

    QGroupBox* pathsGroup = new QGroupBox("Paths");
    pathsGroup->setLayout(layout);
    return pathsGroup;
}
QGroupBox* GeneralTab::CreateTimeGroup() const
{
    QFormLayout* layout = new QFormLayout;

    // Timezones
    QComboBox* timezones = new QComboBox;
    timezones->setObjectName("TimezonesDropdown");
    auto& userTimezone = settings_.GetTimezone();
    int userTimezoneIndex = 0;
    int currIndex = 0;

    for (auto& tz : std::chrono::get_tzdb().zones)
    {
        timezones->addItem(QString::fromStdString(std::string{tz.name()}));

        if (tz.name() == userTimezone.name())
        {
            userTimezoneIndex = currIndex;
        }

        ++currIndex;
    }

    timezones->setCurrentIndex(userTimezoneIndex);
    connect(timezones, &QComboBox::currentTextChanged, this, &GeneralTab::TimezoneChangedSlot);
    layout->addRow("Timezone", timezones);

    // Clock format
    QCheckBox* clockFormatBox = new QCheckBox;
    clockFormatBox->setObjectName("ClockFormatBox");
    clockFormatBox->setChecked(settings_.Is12HourClock());
    connect(clockFormatBox, &QCheckBox::checkStateChanged, this, &GeneralTab::ClockFormatChangedSlot);
    layout->addRow("12H Clock", clockFormatBox);

    UpdateTimeWidgets(timezones, clockFormatBox);

    QGroupBox* optionsGroup = new QGroupBox("Time Settings");
    optionsGroup->setLayout(layout);
    return optionsGroup;
}

void GeneralTab::UpdateTimeWidgets(QComboBox* timezones, QCheckBox* clockFormat) const
{
    // Timezones
    timezones->blockSignals(true);
    auto& userTimezone = settings_.GetTimezone();
    int userTimezoneIndex = 0;

    for (auto& tz : std::chrono::get_tzdb().zones)
    {
        if (tz.name() == userTimezone.name())
        {
            break;
        }

        ++userTimezoneIndex;
    }

    if (userTimezoneIndex >= timezones->count())
    {
        userTimezoneIndex = 0;
    }

    timezones->setCurrentIndex(userTimezoneIndex);
    timezones->blockSignals(false);

    // Clock format
    clockFormat->blockSignals(true);
    clockFormat->setChecked(settings_.Is12HourClock());
    clockFormat->blockSignals(false);
}
}
