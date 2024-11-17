#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>
#include <GUI/include/Bindings.hpp>
#include <QtCore/QSettings>
#include <SDL2/SDL.h>

namespace fs = std::filesystem;

class PersistentData
{
public:
    /// @brief Initialize emulator persistent data.
    PersistentData();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save Directory
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Set the path to the directory to store save files and save states.
    void SetSaveDirectory(fs::path saveDir);

    /// @brief Get the default path to the directory used to store save files and save states.
    /// @return Path to saves directory.
    fs::path GetDefaultSaveDirectory();

    /// @brief Get the path to the directory to store save files and save states.
    fs::path GetSaveDirectory() const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// BIOS Path
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Set the path to the GBA BIOS file.
    void SetBiosPath(fs::path biosPath);

    /// @brief Get the default path to the GBA BIOS file.
    /// @return Path to GBA BIOS file.
    fs::path GetDefaultBiosPath();

    /// @brief Get the path to the GBA BIOS file.
    fs::path GetBiosPath() const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// ROM Paths
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Set the path of the directory that a ROM was last loaded from.
    void SetFileDialogPath(fs::path dialogDir);

    /// @brief Get the path of the directory that a ROM was last loaded from.
    fs::path GetFileDialogPath() const;

    /// @brief Add a path to a ROM file to the list of recently loaded ROMs.
    void AddRecentRom(fs::path romPath);

    /// @brief Get the list of recently opened ROMs.
    std::vector<fs::path> GetRecentRoms() const;

    /// @brief Clear any recently loaded ROMs.
    void ClearRecentRoms();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Audio
    ///-----------------------------------------------------------------------------------------------------------------------------

    enum class Channel
    {
        ONE,
        TWO,
        THREE,
        FOUR,
        FIFO_A,
        FIFO_B
    };

    struct AudioSettings
    {
        bool mute;
        int volume;
        bool channel1;
        bool channel2;
        bool channel3;
        bool channel4;
        bool fifoA;
        bool fifoB;
    };

    /// @brief Set whether GBA audio should be muted.
    /// @param mute True to mute output, false otherwise.
    void SetMuted(bool mute);

    /// @brief Check if GBA audio should be muted.
    /// @return Whether to mute audio.
    bool GetMuted() const;

    /// @brief Set the current volume output level.
    /// @param volume Volume level [0, 100];
    void SetVolume(int volume);

    /// @brief Get the current volume output level.
    /// @return Volume level [0, 100];
    int GetVolume() const;

    /// @brief Set whether a specific APU channel is enabled.
    /// @param channel Channel to set.
    /// @param enable Whether to enable the channel.
    void SetChannelEnabled(Channel channel, bool enable);

    /// @brief Check whether a specific APU channel is enabled.
    /// @param channel Channel to check.
    /// @return Whether the specified channel is enabled.
    bool GetChannelEnabled(Channel channel) const;

    /// @brief Get the current state of all audio related settings.
    /// @return Current audio settings.
    AudioSettings GetAudioSettings() const;

    /// @brief Restore all audio settings to their default values.
    void RestoreDefaultAudioSettings();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Keyboard Bindings
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Save a keyboard binding.
    /// @param gbaKey GBA key to set a binding for.
    /// @param binding Keyboard binding to set for the specified GBA key.
    /// @param primary Whether this is the GBA key's primary or secondary binding.
    void SetKeyboardBinding(gui::GBAKey gbaKey, Qt::Key key, bool primary);

    /// @brief Get a keyboard binding for a GBA key.
    /// @param gbaKey GBA key to get a keyboard binding for.
    /// @param primary Whether to get the primary or secondary binding.
    /// @return Keyboard binding.
    Qt::Key GetKeyboardBinding(gui::GBAKey gbaKey, bool primary) const;

    /// @brief Get all primary and secondary keyboard bindings for all GBA keys.
    /// @return All gamepad bindings.
    gui::KeyboardMap GetKeyboardMap() const;

    /// @brief Restore all default keyboard bindings.
    void RestoreDefaultKeyboardBindings();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Gamepad Bindings
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Save a gamepad binding.
    /// @param gbaKey GBA key to set a binding for.
    /// @param binding Gamepad binding to set for the specified GBA key.
    /// @param primary Whether this is the GBA key's primary or secondary binding.
    void SetGamepadBinding(gui::GBAKey gbaKey, gui::GamepadBinding const& binding, bool primary);

    /// @brief Get a gamepad binding for a GBA key.
    /// @param gbaKey GBA key to get a gamepad binding for.
    /// @param primary Whether to get the primary or secondary binding.
    /// @return Gamepad binding.
    gui::GamepadBinding GetGamepadBinding(gui::GBAKey gbaKey, bool primary) const;

    /// @brief Save the GUID for a gamepad. This gamepad is prioritized when selecting which gamepad to use.
    /// @param guid 
    void SetGUID(SDL_JoystickGUID guid);

    /// @brief Get the saved GUID.
    /// @return Saved GUID (all 0 if no GUID has been saved).
    SDL_JoystickGUID GetGUID() const;

    /// @brief Set the deadzone percentage to use for joysticks.
    /// @param deadzone Deadzone percentage [0-100].
    void SetDeadzone(int deadzone);

    /// @brief Get the deadzone percentage to use for joysticks.
    /// @return Deadzone percentage [0-100].
    int GetDeadzone() const;

    /// @brief Get all primary and secondary gamepad bindings for all GBA keys.
    /// @return All gamepad bindings.
    gui::GamepadMap GetGamepadMap() const;

    /// @brief Restore all default gamepad bindings and deadzone.
    /// @param clearGUID Whether to reset the saved GUID.
    void RestoreDefaultGamepadBindings(bool clearGUID);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Time
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the currently saved timezone.
    /// @return Current timezone.
    std::chrono::time_zone const& GetTimezone() const;

    /// @brief Set the timezone to use for converting time points to zoned time.
    /// @param timezone Timezone to save.
    void SetTimezone(std::chrono::time_zone const& timezone);

    /// @brief Check how to display time values.
    /// @return True to display times in 12H format, false for 24H format.
    bool Is12HourClock() const;

    /// @brief Set how to display time values.
    /// @param is12H Whether to display times in 12H format (true) or 24H format (false).
    void Set12HourClock(bool is12H);

    /// @brief Restore default time related values.
    void RestoreDefaultTimeSettings();

private:
    /// @brief Read the settings to get the primary and secondary gamepad bindings from a config key.
    /// @param key Key string in config file to get bindings for.
    /// @param deadzone Deadzone value to initialize bindings with.
    /// @return Primary and secondary bindings for specified config key.
    std::pair<gui::GamepadBinding, gui::GamepadBinding> GetGamepadBindingsForKey(QString const& key, int deadzone) const;

    /// @brief Read the settings to get the primary and secondary keyboard bindings from a config key.
    /// @param key Key string in config file to get bindings for.
    /// @return Primary and secondary bindings for specified config key.
    std::pair<Qt::Key, Qt::Key> GetKeyboardBindingsForKey(QString const& key) const;

    /// @brief If a config file does not exist, generate one with default settings.
    void WriteDefaultSettings();

    fs::path baseDir_;
    std::unique_ptr<QSettings> settingsPtr_;
};
