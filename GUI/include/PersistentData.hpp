#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <QtCore/QSettings>

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

private:
    /// @brief If a config file does not exist, generate one with default settings.
    void WriteDefaultSettings();

    fs::path baseDir_;
    std::unique_ptr<QSettings> settingsPtr_;
};
