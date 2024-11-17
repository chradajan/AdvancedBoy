#pragma once

#include <filesystem>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QWidget>

class PersistentData;
namespace fs = std::filesystem;

namespace gui
{
class PathSelector : public QWidget
{
    Q_OBJECT

public:
    PathSelector() = delete;
    PathSelector(PathSelector const&) = delete;
    PathSelector& operator=(PathSelector const&) = delete;
    PathSelector(PathSelector&&) = delete;
    PathSelector& operator=(PathSelector&&) = delete;

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

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    GeneralTab() = delete;
    GeneralTab(GeneralTab const&) = delete;
    GeneralTab& operator=(GeneralTab const&) = delete;
    GeneralTab(GeneralTab&&) = delete;
    GeneralTab& operator=(GeneralTab&&) = delete;

    /// @brief Initialize the general options tab widget.
    /// @param settings Reference to settings.
    GeneralTab(PersistentData& settings);

    /// @brief Restore the default general settings.
    void RestoreDefaults();

signals:
    /// @brief Emit when any time related setting has changed.
    void TimeFormatChangedSignal();

private slots:
    /// @brief Open a file dialog to select a BIOS file.
    void OpenBiosFileDialog();

    /// @brief Open a file dialog to select the save directory.
    void OpenSaveDirDialog();

    /// @brief Slot to handle the timezone being changed.
    /// @param timezoneName Name of newly selected timezone.
    void TimezoneChangedSlot(QString const& timezoneName);

    /// @brief Slot to handle changing between 12H/24H clock formats.
    /// @param state State of clock format box (checked == 12H).
    void ClockFormatChangedSlot(Qt::CheckState state);

private:
    /// @brief Create group box of miscellaneous emulation options.
    /// @return Options group box.
    [[nodiscard]] QGroupBox* CreateOptionsGroup() const;

    /// @brief Create group box of paths to specify.
    /// @return Paths group box.
    [[nodiscard]] QGroupBox* CreatePathsGroup() const;

    /// @brief Create group box of time related options.
    /// @return Time group box.
    [[nodiscard]] QGroupBox* CreateTimeGroup() const;

    /// @brief Update the time related widgets based on current settings.
    /// @param timezones Pointer to timezone dropdown.
    /// @param clockFormat Pointer to clock format checkbox.
    void UpdateTimeWidgets(QComboBox* timezones, QCheckBox* clockFormat) const;

    PersistentData& settings_;
};
}
