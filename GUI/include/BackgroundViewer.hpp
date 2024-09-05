#pragma once

#include <GBA/include/Types/DebugTypes.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

namespace gui
{
using namespace debug::graphics;

/// @brief Window for debugging background maps.
class BackgroundViewer : public QWidget
{
    Q_OBJECT

public:
    /// @brief Initialize the background viewer.
    BackgroundViewer();

    /// @brief Get the index of the background to be displayed in the background viewer.
    /// @return Currently select background index.
    u8 GetSelectedBg() const { return selectedBg_; }

    /// @brief Update the image and information for the currently selected background.
    /// @param info Debug info needed to display a background.
    void UpdateBackgroundView(BackgroundDebugInfo const& info);

private:
    /// @brief Initialize widgets needed to adjust background scale and selection.
    /// @return Widget to add to main layout.
    QGroupBox* CreateSelectionGroup();

    /// @brief Initialize widgets needed to view information about the selected background.
    /// @return Widget to add to main layout.
    QGroupBox* CreateBgInfoGroup();

    /// @brief Initialize widgets needed to display the background image.
    /// @return Widget to add to main layout.
    QScrollArea* CreateBackgroundImage();

    /// @brief Action for background selection buttons.
    /// @param bgIndex Index of background to select.
    void SetSelectedBg(u8 bgIndex) { selectedBg_ = bgIndex; }

    // Info
    u8 selectedBg_;

    // Map selection
    QComboBox* scaleBox_;

    // BG Info
    QLabel* priorityLabel_;
    QLabel* mapBaseLabel_;
    QLabel* tileBaseLabel_;
    QLabel* sizeLabel_;
    QLabel* offsetLabel_;
    QLabel* matrixLabel_;

    // Map view
    QLabel* bgImageLabel_;
};
}  // namespace gui
