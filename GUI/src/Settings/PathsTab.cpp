#include <GUI/include/Settings/PathsTab.hpp>
#include <filesystem>
#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

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
/// Paths Tab
///---------------------------------------------------------------------------------------------------------------------------------

PathsTab::PathsTab(PersistentData& settings) : settings_(settings)
{
    QGridLayout* layout = new QGridLayout;

    // Bios
    PathSelector* biosWidget = new PathSelector(settings.GetBiosPath());
    biosWidget->setObjectName("BiosWidget");
    connect(biosWidget, &PathSelector::ButtonClicked, this, &PathsTab::OpenBiosFileDialog);
    layout->addWidget(new QLabel("BIOS ROM"), 0, 0);
    layout->addWidget(biosWidget, 0, 1);

    // Saves
    PathSelector* saveWidget = new PathSelector(settings.GetSaveDirectory());
    saveWidget->setObjectName("SaveWidget");
    connect(saveWidget, &PathSelector::ButtonClicked, this, &PathsTab::OpenSaveDirDialog);
    layout->addWidget(new QLabel("Saves Directory"), 1, 0);
    layout->addWidget(saveWidget, 1, 1);

    // Spacer
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(spacer, 2, 0);

    setLayout(layout);
}

void PathsTab::OpenBiosFileDialog()
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

void PathsTab::OpenSaveDirDialog()
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

void PathsTab::RestoreDefaults()
{
    fs::path biosPath = settings_.GetDefaultBiosPath();
    settings_.SetBiosPath(biosPath);
    findChild<PathSelector*>("BiosWidget")->UpdatePath(biosPath);

    fs::path saveDir = settings_.GetDefaultSaveDirectory();
    settings_.SetSaveDirectory(saveDir);
    findChild<PathSelector*>("SaveWidget")->UpdatePath(saveDir);
}
}
