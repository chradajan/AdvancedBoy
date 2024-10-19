#include <GUI/include/PersistentData.hpp>
#include <filesystem>
#include <vector>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>

PersistentData::PersistentData()
{
    baseDir_ = fs::path(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::ConfigLocation).toStdString());

    if (!fs::exists(baseDir_))
    {
        fs::create_directory(baseDir_);
    }

    if (!fs::exists(baseDir_ / "Saves"))
    {
        fs::create_directory(baseDir_ / "Saves");
    }

    fs::path configPath = baseDir_ / "config.ini";
    settingsPtr_ = std::make_unique<QSettings>(QString::fromStdString(configPath.string()), QSettings::IniFormat);

    if (!fs::exists(configPath))
    {
        WriteDefaultSettings();
    }
}

void PersistentData::SetSaveDirectory(fs::path saveDir)
{
    settingsPtr_->setValue("Paths/SaveDir", QString::fromStdString(saveDir.string()));
}

fs::path PersistentData::GetSaveDirectory() const
{
    return settingsPtr_->value("Paths/SaveDir").toString().toStdString();
}

void PersistentData::SetBiosPath(fs::path biosPath)
{
    settingsPtr_->setValue("Paths/BiosPath", QString::fromStdString(biosPath.string()));
}

fs::path PersistentData::GetBiosPath() const
{
    return settingsPtr_->value("Paths/BiosPath").toString().toStdString();
}

void PersistentData::AddRecentRom(fs::path romPath)
{
    auto recentRoms = GetRecentRoms();
    settingsPtr_->beginGroup("RecentRoms");

    for (int i = 3; i >= 0; --i)
    {
        fs::path pathToMove = settingsPtr_->value("Recent" + QString::number(i)).toString().toStdString();

        if (fs::exists(pathToMove) && fs::is_regular_file(pathToMove))
        {
            settingsPtr_->setValue("Recent" + QString::number(i + 1), QString::fromStdString(pathToMove.string()));
        }
    }

    settingsPtr_->setValue("Recent0", QString::fromStdString(romPath.string()));
    settingsPtr_->endGroup();
}

std::vector<fs::path> PersistentData::GetRecentRoms() const
{
    settingsPtr_->beginGroup("RecentRoms");
    std::vector<fs::path> recentRoms;

    for (QString key : { "Recent0", "Recent1", "Recent2", "Recent3", "Recent4" })
    {
        fs::path romPath = settingsPtr_->value(key).toString().toStdString();

        if (fs::exists(romPath) && fs::is_regular_file(romPath))
        {
            recentRoms.push_back(romPath);
        }
    }

    settingsPtr_->endGroup();
    return recentRoms;
}

void PersistentData::WriteDefaultSettings()
{
    settingsPtr_->beginGroup("Paths");
    settingsPtr_->setValue("SaveDir", QString::fromStdString((baseDir_ / "Saves").string()));
    settingsPtr_->setValue("BiosPath", "");
    settingsPtr_->endGroup();

    settingsPtr_->beginGroup("RecentRoms");

    for (QString key : { "Recent0", "Recent1", "Recent2", "Recent3", "Recent4" })
    {
        settingsPtr_->setValue(key, "");
    }

    settingsPtr_->endGroup();
}
