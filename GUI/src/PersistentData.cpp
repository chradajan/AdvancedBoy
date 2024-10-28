#include <GUI/include/PersistentData.hpp>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>
#include <GUI/include/Bindings.hpp>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>
#include <SDL2/SDL.h>

namespace
{
/// @brief Get the config key for a GBA key.
/// @param gbaKey GBA key to get config key for.
/// @param primary Whether to get the primary or secondary key.
/// @return Config key string.
QString GetSettingsKeyFromGbaKey(gui::GBAKey gbaKey, bool primary)
{
    QString key = primary ? "_Primary" : "_Secondary";

    switch (gbaKey)
    {
        case gui::GBAKey::UP:
            key.prepend("Gamepad/Up");
            break;
        case gui::GBAKey::DOWN:
            key.prepend("Gamepad/Down");
            break;
        case gui::GBAKey::LEFT:
            key.prepend("Gamepad/Left");
            break;
        case gui::GBAKey::RIGHT:
            key.prepend("Gamepad/Right");
            break;
        case gui::GBAKey::L:
            key.prepend("Gamepad/L");
            break;
        case gui::GBAKey::R:
            key.prepend("Gamepad/R");
            break;
        case gui::GBAKey::A:
            key.prepend("Gamepad/A");
            break;
        case gui::GBAKey::B:
            key.prepend("Gamepad/B");
            break;
        case gui::GBAKey::START:
            key.prepend("Gamepad/Start");
            break;
        case gui::GBAKey::SELECT:
            key.prepend("Gamepad/Select");
            break;
        default:
            return QString();
    }

    return key;
}
}

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

void PersistentData::SetGamepadBinding(gui::GBAKey gbaKey, gui::GamepadBinding const& binding, bool primary)
{
    QString key = GetSettingsKeyFromGbaKey(gbaKey, primary);

    if (!key.isEmpty())
    {
        settingsPtr_->setValue(key, binding.ToList());
    }
}

gui::GamepadBinding PersistentData::GetGamepadBinding(gui::GBAKey gbaKey, bool primary) const
{
    QString key = GetSettingsKeyFromGbaKey(gbaKey, primary);

    if (!key.isEmpty())
    {
        return gui::GamepadBinding(settingsPtr_->value(key).toList(), GetDeadzone());
    }

    return gui::GamepadBinding();
}

void PersistentData::SetGUID(SDL_JoystickGUID guid)
{
    QVariantList guidList;
    std::span guidSpan{guid.data};

    for (auto val : guidSpan)
    {
        guidList.append(QVariant(val));
    }

    settingsPtr_->setValue("Gamepad/GUID", guidList);
}

SDL_JoystickGUID PersistentData::GetGUID() const
{
    QVariantList guidList = settingsPtr_->value("Gamepad/GUID").toList();
    SDL_JoystickGUID guid;
    std::span guidSpan{guid.data};
    int guidListIndex = 0;

    for (auto& val : guidSpan)
    {
        val = guidList[guidListIndex++].toInt();
    }

    return guid;
}

void PersistentData::SetDeadzone(int deadzone)
{
    settingsPtr_->setValue("Gamepad/Deadzone", deadzone);
}

int PersistentData::GetDeadzone() const
{
    return settingsPtr_->value("Gamepad/Deadzone").toInt();
}

gui::GamepadMap PersistentData::GetGamepadMap() const
{
    int deadzone = settingsPtr_->value("Gamepad/Deadzone").toInt();
    gui::GamepadMap map;
    map.up = GetGamepadBindingsForKey("Up", deadzone);
    map.down = GetGamepadBindingsForKey("Down", deadzone);
    map.left = GetGamepadBindingsForKey("Left", deadzone);
    map.right = GetGamepadBindingsForKey("Right", deadzone);
    map.l = GetGamepadBindingsForKey("L", deadzone);
    map.r = GetGamepadBindingsForKey("R", deadzone);
    map.a = GetGamepadBindingsForKey("A", deadzone);
    map.b = GetGamepadBindingsForKey("B", deadzone);
    map.start = GetGamepadBindingsForKey("Start", deadzone);
    map.select = GetGamepadBindingsForKey("Select", deadzone);
    return map;
}

void PersistentData::RestoreDefaultGamepadBindings(bool clearGUID)
{
    settingsPtr_->setValue("Gamepad/Deadzone", 5);

    if (clearGUID)
    {
        SDL_JoystickGUID guid;
        std::memset(&guid, 0, sizeof(guid));
        SetGUID(guid);
    }

    settingsPtr_->setValue("Gamepad/Up_Primary",        gui::GamepadBinding(SDL_CONTROLLER_BUTTON_DPAD_UP).ToList());
    settingsPtr_->setValue("Gamepad/Down_Primary",      gui::GamepadBinding(SDL_CONTROLLER_BUTTON_DPAD_DOWN).ToList());
    settingsPtr_->setValue("Gamepad/Left_Primary",      gui::GamepadBinding(SDL_CONTROLLER_BUTTON_DPAD_LEFT).ToList());
    settingsPtr_->setValue("Gamepad/Right_Primary",     gui::GamepadBinding(SDL_CONTROLLER_BUTTON_DPAD_RIGHT).ToList());
    settingsPtr_->setValue("Gamepad/L_Primary",         gui::GamepadBinding(SDL_CONTROLLER_BUTTON_LEFTSHOULDER).ToList());
    settingsPtr_->setValue("Gamepad/R_Primary",         gui::GamepadBinding(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER).ToList());
    settingsPtr_->setValue("Gamepad/A_Primary",         gui::GamepadBinding(SDL_CONTROLLER_BUTTON_A).ToList());
    settingsPtr_->setValue("Gamepad/B_Primary",         gui::GamepadBinding(SDL_CONTROLLER_BUTTON_B).ToList());
    settingsPtr_->setValue("Gamepad/Start_Primary",     gui::GamepadBinding(SDL_CONTROLLER_BUTTON_START).ToList());
    settingsPtr_->setValue("Gamepad/Select_Primary",    gui::GamepadBinding(SDL_CONTROLLER_BUTTON_BACK).ToList());

    settingsPtr_->setValue("Gamepad/Up_Secondary",      gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/Down_Secondary",    gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/Left_Secondary",    gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/Right_Secondary",   gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/L_Secondary",       gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/R_Secondary",       gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/A_Secondary",       gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/B_Secondary",       gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/Start_Secondary",   gui::GamepadBinding().ToList());
    settingsPtr_->setValue("Gamepad/Select_Secondary",  gui::GamepadBinding().ToList());
}

std::pair<gui::GamepadBinding, gui::GamepadBinding> PersistentData::GetGamepadBindingsForKey(QString const& key, int deadzone) const
{
    return {gui::GamepadBinding(settingsPtr_->value("Gamepad/" + key + "_Primary").toList(), deadzone),
            gui::GamepadBinding(settingsPtr_->value("Gamepad/" + key + "_Secondary").toList(), deadzone)};
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

    // Gamepad
    RestoreDefaultGamepadBindings(true);

    // Keyboard
    settingsPtr_->beginGroup("Keyboard");
    settingsPtr_->endGroup();
}
