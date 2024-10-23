#pragma once

#include <filesystem>
#include <QtWidgets/QWidget>

class PersistentData;
namespace fs = std::filesystem;

namespace gui
{
class PathSelector : public QWidget
{
    Q_OBJECT

public:
    /// @brief Create a path selector widget and set the path for it to display.
    /// @param path Path to initially display.
    PathSelector(fs::path path);

    /// @brief Update the displayed path.
    /// @param path Path to show.
    void UpdatePath(fs::path path);

signals:
    /// @brief Signal emitted when the open file dialog button is clicked.
    void ButtonClicked();
};

class PathsTab : public QWidget
{
    Q_OBJECT

public:
    PathsTab() = delete;
    PathsTab(PathsTab const&) = delete;
    PathsTab& operator=(PathsTab const&) = delete;
    PathsTab(PathsTab&&) = delete;
    PathsTab& operator=(PathsTab&&) = delete;

    /// @brief Initialize the paths tab widget.
    /// @param settings Reference to settings.
    PathsTab(PersistentData& settings);

    /// @brief Restore the default path settings.
    void RestoreDefaults();

private slots:
    /// @brief Open a file dialog to select a BIOS file.
    void OpenBiosFileDialog();

    /// @brief Open a file dialog to select the save directory.
    void OpenSaveDirDialog();

private:
    PersistentData& settings_;
};
}
