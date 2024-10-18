#pragma once

#include <GBA/include/Debug/DebugTypes.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

namespace gui
{
using namespace debug;

/// @brief Window for debugging background maps.
class BackgroundViewerWindow : public QWidget
{
    Q_OBJECT

public:
    /// @brief Initialize the background viewer.
    BackgroundViewerWindow();

public slots:
    /// @brief Update the image and information for the currently selected background.
    /// @param updateBg Whether to fetch the latest background data from the GBA.
    void UpdateBackgroundViewSlot(bool updateBg) { if (isVisible()) UpdateBackgroundView(updateBg); }

private slots:
    /// @brief Slot to handle a BG select button being clicked.
    /// @param bgIndex Index of BG corresponding to the clicked button.
    void SetSelectedBg(int bgIndex);

private:
    /// @brief Get information for the currently selected background and update the displayed image/data.
    /// @param updateBg Whether to fetch the latest background data from the GBA.
    void UpdateBackgroundView(bool updateBg);

    /// @brief Initialize widgets needed to adjust background scale and selection.
    /// @return Widget to add to main layout.
    [[nodiscard]] QGroupBox* CreateControlGroup();

    /// @brief Initialize widgets needed to view information about the selected background.
    /// @return Widget to add to main layout.
    [[nodiscard]] QGroupBox* CreateBgInfoGroup();

    /// @brief Initialize widgets needed to display the background image.
    /// @return Widget to add to main layout.
    [[nodiscard]] QScrollArea* CreateBackgroundImage();

    // Data
    debug::BackgroundDebugInfo debugInfo_;
    int selectedBg_;

    // Controls
    QSpinBox* scaleBox_;
    QCheckBox* transparencyControl_;

    // BG Info
    QLabel* priorityLabel_;
    QLabel* mapBaseLabel_;
    QLabel* tileBaseLabel_;
    QLabel* sizeLabel_;
    QLabel* offsetLabel_;
    QLabel* paLabel_;
    QLabel* pbLabel_;
    QLabel* pcLabel_;
    QLabel* pdLabel_;

    // Map view
    QLabel* bgImageLabel_;
};
}  // namespace gui
