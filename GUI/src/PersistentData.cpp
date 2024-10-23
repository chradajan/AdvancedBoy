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

///---------------------------------------------------------------------------------------------------------------------------------
/// Save Directory
///---------------------------------------------------------------------------------------------------------------------------------

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

///---------------------------------------------------------------------------------------------------------------------------------
/// BIOS Path
///---------------------------------------------------------------------------------------------------------------------------------

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

///---------------------------------------------------------------------------------------------------------------------------------
/// ROM Paths
///---------------------------------------------------------------------------------------------------------------------------------

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

///---------------------------------------------------------------------------------------------------------------------------------
/// Audio
///---------------------------------------------------------------------------------------------------------------------------------

void PersistentData::SetMuted(bool mute)
{
    settingsPtr_->setValue("Audio/Mute", mute);
}

bool PersistentData::GetMuted() const
{
    return settingsPtr_->value("Audio/Mute").toBool();
}

void PersistentData::SetVolume(int volume)
{
    if ((volume < 0) || (volume > 100))
    {
        return;
    }

    settingsPtr_->setValue("Audio/Volume", volume);
}

int PersistentData::GetVolume() const
{
    return settingsPtr_->value("Audio/Volume").toInt();
}

void PersistentData::SetChannelEnabled(Channel channel, bool enable)
{
    switch (channel)
    {
        case Channel::ONE:
            settingsPtr_->setValue("Audio/Channel1", enable);
            break;
        case Channel::TWO:
            settingsPtr_->setValue("Audio/Channel2", enable);
            break;
        case Channel::THREE:
            settingsPtr_->setValue("Audio/Channel3", enable);
            break;
        case Channel::FOUR:
            settingsPtr_->setValue("Audio/Channel4", enable);
            break;
        case Channel::FIFO_A:
            settingsPtr_->setValue("Audio/FifoA", enable);
            break;
        case Channel::FIFO_B:
            settingsPtr_->setValue("Audio/FifoB", enable);
            break;
    }
}

bool PersistentData::GetChannelEnabled(Channel channel) const
{
    switch (channel)
    {
        case Channel::ONE:
            return settingsPtr_->value("Audio/Channel1").toBool();
        case Channel::TWO:
            return settingsPtr_->value("Audio/Channel2").toBool();
        case Channel::THREE:
            return settingsPtr_->value("Audio/Channel3").toBool();
        case Channel::FOUR:
            return settingsPtr_->value("Audio/Channel4").toBool();
        case Channel::FIFO_A:
            return settingsPtr_->value("Audio/FifoA").toBool();
        case Channel::FIFO_B:
            return settingsPtr_->value("Audio/FifoB").toBool();
    }

    return false;
}

PersistentData::AudioSettings PersistentData::GetAudioSettings() const
{
    return {
        settingsPtr_->value("Audio/Mute").toBool(),
        settingsPtr_->value("Audio/Volume").toInt(),
        settingsPtr_->value("Audio/Channel1").toBool(),
        settingsPtr_->value("Audio/Channel2").toBool(),
        settingsPtr_->value("Audio/Channel3").toBool(),
        settingsPtr_->value("Audio/Channel4").toBool(),
        settingsPtr_->value("Audio/FifoA").toBool(),
        settingsPtr_->value("Audio/FifoB").toBool()
    };
}

void PersistentData::RestoreDefaultAudioSettings()
{
    settingsPtr_->setValue("Audio/Mute", false);
    settingsPtr_->setValue("Audio/Volume", 100);
    settingsPtr_->setValue("Audio/Channel1", true);
    settingsPtr_->setValue("Audio/Channel2", true);
    settingsPtr_->setValue("Audio/Channel3", true);
    settingsPtr_->setValue("Audio/Channel4", true);
    settingsPtr_->setValue("Audio/FifoA", true);
    settingsPtr_->setValue("Audio/FifoB", true);
}

void PersistentData::WriteDefaultSettings()
{
    // Paths
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

    // Audio
    settingsPtr_->beginGroup("Audio");
    settingsPtr_->setValue("Mute", false);
    settingsPtr_->setValue("Volume", 100);
    settingsPtr_->setValue("Channel1", true);
    settingsPtr_->setValue("Channel2", true);
    settingsPtr_->setValue("Channel3", true);
    settingsPtr_->setValue("Channel4", true);
    settingsPtr_->setValue("FifoA", true);
    settingsPtr_->setValue("FifoB", true);
    settingsPtr_->endGroup();
}
