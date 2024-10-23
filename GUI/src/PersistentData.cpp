#include <GUI/include/PersistentData.hpp>
#include <algorithm>
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

    if (!fs::exists(GetDefaultSaveDirectory()))
    {
        fs::create_directory(GetDefaultSaveDirectory());
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

fs::path PersistentData::GetDefaultSaveDirectory()
{
    return baseDir_ / "Saves";
}

fs::path PersistentData::GetSaveDirectory() const
{
    return settingsPtr_->value("Paths/SaveDir").toString().toStdString();
}

void PersistentData::SetBiosPath(fs::path biosPath)
{
    settingsPtr_->setValue("Paths/BiosPath", QString::fromStdString(biosPath.string()));
}

fs::path PersistentData::GetDefaultBiosPath()
{
    return "";
}

fs::path PersistentData::GetBiosPath() const
{
    return settingsPtr_->value("Paths/BiosPath").toString().toStdString();
}

void PersistentData::SetFileDialogPath(fs::path dialogDir)
{
    settingsPtr_->setValue("Paths/FileDialogPath", QString::fromStdString(dialogDir.string()));
}

fs::path PersistentData::GetFileDialogPath() const
{
    return settingsPtr_->value("Paths/FileDialogPath").toString().toStdString();
}

void PersistentData::AddRecentRom(fs::path romPath)
{
    auto recentRoms = GetRecentRoms();
    auto it = std::find(recentRoms.begin(), recentRoms.end(), romPath);

    if (it != recentRoms.end())
    {
        recentRoms.erase(it);
    }

    settingsPtr_->beginGroup("RecentRoms");
    settingsPtr_->setValue("Recent0", QString::fromStdString(romPath.string()));
    int i = 1;

    for (fs::path recentRom : recentRoms)
    {
        QString key = "Recent" + QString::number(i++);
        settingsPtr_->setValue(key, QString::fromStdString(recentRom.string()));
    }

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

void PersistentData::ClearRecentRoms()
{
    settingsPtr_->beginGroup("RecentRoms");

    for (QString key : { "Recent0", "Recent1", "Recent2", "Recent3", "Recent4" })
    {
        settingsPtr_->setValue(key, "");
    }

    settingsPtr_->endGroup();
}

void PersistentData::WriteDefaultSettings()
{
    settingsPtr_->beginGroup("Paths");
    settingsPtr_->setValue("SaveDir", QString::fromStdString(GetDefaultSaveDirectory().string()));
    settingsPtr_->setValue("BiosPath", QString::fromStdString(GetDefaultBiosPath().string()));
    settingsPtr_->setValue("FileDialogPath", "");
    settingsPtr_->endGroup();

    settingsPtr_->beginGroup("RecentRoms");

    for (QString key : { "Recent0", "Recent1", "Recent2", "Recent3", "Recent4" })
    {
        settingsPtr_->setValue(key, "");
    }

    settingsPtr_->endGroup();
}
