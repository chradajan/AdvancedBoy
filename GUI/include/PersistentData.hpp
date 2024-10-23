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

    /// @brief Set the path to the directory to store save files and save states.
    void SetSaveDirectory(fs::path saveDir);

    /// @brief Get the default path to the directory used to store save files and save states.
    /// @return Path to saves directory.
    fs::path GetDefaultSaveDirectory();

    /// @brief Get the path to the directory to store save files and save states.
    fs::path GetSaveDirectory() const;

    /// @brief Set the path to the GBA BIOS file.
    void SetBiosPath(fs::path biosPath);

    /// @brief Get the default path to the GBA BIOS file.
    /// @return Path to GBA BIOS file.
    fs::path GetDefaultBiosPath();

    /// @brief Get the path to the GBA BIOS file.
    fs::path GetBiosPath() const;

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

private:
    /// @brief If a config file does not exist, generate one with default settings.
    void WriteDefaultSettings();

    fs::path baseDir_;
    std::unique_ptr<QSettings> settingsPtr_;
};
