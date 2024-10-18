#pragma once

#include <array>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

namespace gui
{
class SpritePreviewWidget : public QFrame
{
    Q_OBJECT

public:
    SpritePreviewWidget() = delete;
    SpritePreviewWidget(SpritePreviewWidget const&) = delete;
    SpritePreviewWidget& operator=(SpritePreviewWidget const&) = delete;
    SpritePreviewWidget(SpritePreviewWidget&&) = delete;
    SpritePreviewWidget& operator=(SpritePreviewWidget&&) = delete;

    /// @brief Initialize a widget for a single sprite preview.
    /// @param index Index of OAM entry that this widget will display.
    SpritePreviewWidget(u8 index);

    /// @brief Update the displayed sprite.
    /// @param sprite Pointer to sprite to display. If nullptr, update with a blank icon.
    /// @param doubleSized True if the sprite uses double size mode and affine transformations are enabled.
    void UpdatePreview(debug::Sprite const* sprite, bool doubleSized);

private slots:
    /// @brief Handle this widget being clicked.
    void SpriteSelectedSlot() { emit SpriteSelectedSignal(index_); }

signals:
    /// @brief Signal to emit when a preview is selected.
    /// @param index OAM index of the sprite to be displayed.
    void SpriteSelectedSignal(u8 index);

private:
    u8 const index_;
    QPushButton* imageButton_;
};

class SpriteViewerWindow : public QWidget
{
    Q_OBJECT

public:
    SpriteViewerWindow(SpriteViewerWindow const&) = delete;
    SpriteViewerWindow& operator=(SpriteViewerWindow const&) = delete;
    SpriteViewerWindow(SpriteViewerWindow&&) = delete;
    SpriteViewerWindow& operator=(SpriteViewerWindow&&) = delete;

    /// @brief Initialize the sprite viewer window.
    SpriteViewerWindow();

public slots:
    /// @brief Slot connected to signal to update this window.
    /// @param updateSprites Whether to update the content of sprites_ with the latest OAM data.
    void UpdateSpriteViewerSlot(bool updateSprites) { if (isVisible()) UpdateSpriteViewer(updateSprites); }

private slots:
    /// @brief Slot to handle updating the displayed info/image when the selected sprite index changes.
    void SelectedSpriteUpdatedSlot(u8 index);

    /// @brief Slot to force update the sprite viewer window with the latest sprite data from the GBA.
    void ForceUpdateSpriteViewerSlot() { if (isVisible()) UpdateSpriteViewer(true); }

private:
    /// @brief Update the displayed sprites/data in this window.
    /// @param updateSprites Whether to update the content of sprites_ with the latest OAM data.
    void UpdateSpriteViewer(bool updateSprites);

    /// @brief Update the displayed values for the currently selected sprite.
    void UpdateSpriteInfo();

    /// @brief Update the large image of the currently selected sprite.
    void UpdateSpriteImage();

    /// @brief Create the sprite info group that will displayed data from the selected sprite's OAM entry.
    /// @return Pointer to group box of sprite data.
    [[nodiscard]] QGroupBox* CreateSpriteInfoGroup();

    /// @brief Create the scroll area that houses the large image of the selected sprite.
    /// @return Pointer to scroll area with sprite image.
    [[nodiscard]] QScrollArea* CreateSpriteImage();

    /// @brief Create scrollable preview of all sprites.
    /// @return Pointer to scrollable preview region.
    [[nodiscard]] QScrollArea* CreateSpritePreviews();

    // Data
    ::debug::SpriteDebugInfo sprites_;
    std::array<SpritePreviewWidget*, 128> spritePreviews_;
    u8 selectedIndex_;

    // Image
    QLabel* imageLabel_;

    // Controls
    QSpinBox* sizeBox_;
    QSpinBox* indexBox_;
    QCheckBox* transparencyControl_;
    QCheckBox* regTransforms_;
    QCheckBox* affTransforms_;

    // Geometry
    QLabel* coordinatesLabel_;
    QLabel* sizeLabel_;
    QCheckBox* doubleSizeBox_;
    QLabel* paLabel_;
    QLabel* pbLabel_;
    QLabel* pcLabel_;
    QLabel* pdLabel_;

    // Attributes
    QLabel* indexLabel_;
    QLabel* tileLabel_;
    QLabel* paletteLabel_;
    QLabel* priorityLabel_;
    QLabel* gfxLabel_;
    QCheckBox* affineBox_;
    QCheckBox* vFlipBox_;
    QCheckBox* hFlipBox_;
    QCheckBox* mosaicBox_;
};
}  // namespace gui
