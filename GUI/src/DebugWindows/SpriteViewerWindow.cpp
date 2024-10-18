#include <GUI/include/DebugWindows/SpriteViewerWindow.hpp>
#include <format>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/FlowLayout.hpp>
#include <GUI/include/GBA.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace gui
{
SpritePreviewWidget::SpritePreviewWidget(u8 index) : QFrame(), index_(index)
{
    setLineWidth(0);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setAttribute(Qt::WA_Hover);

    QVBoxLayout* layout = new QVBoxLayout;

    imageButton_ = new QPushButton;
    connect(imageButton_, &QPushButton::clicked, this, &SpritePreviewWidget::SpriteSelectedSlot);
    imageButton_->setIconSize(QSize(64, 64));
    imageButton_->setFixedSize(QSize(64, 64));
    layout->addWidget(imageButton_);

    QLabel* indexLabel = new QLabel(QString::number(index));
    indexLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(indexLabel);

    setLayout(layout);
}

void SpritePreviewWidget::UpdatePreview(debug::Sprite const* sprite, bool doubleSized)
{
    if (sprite != nullptr)
    {
        u8 width = doubleSized ? sprite->width * 2 : sprite->width;
        u8 height = doubleSized ? sprite->height * 2 : sprite->height;
        QImage image(reinterpret_cast<const uchar*>(sprite->buffer.data()), width, height, QImage::Format_ARGB32);
        QIcon icon(QPixmap::fromImage(image));
        imageButton_->setIcon(icon);
    }
    else
    {
        imageButton_->setIcon(QIcon());
    }
}

SpriteViewerWindow::SpriteViewerWindow() : QWidget()
{
    setWindowTitle("Sprite Viewer");
    gba_api::GetSpriteDebugInfo(sprites_, false, false);
    selectedIndex_ = 0;

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(CreateSpriteInfoGroup(), 0, 0, 1, 1);
    layout->addWidget(CreateSpriteImage(), 0, 1, 1, 2);
    layout->addWidget(CreateSpritePreviews(), 1, 0, 1, 3);

    setLayout(layout);
}

void SpriteViewerWindow::SelectedSpriteUpdatedSlot(u8 index)
{
    selectedIndex_ = index;
    indexBox_->setValue(index);
    UpdateSpriteInfo();
    UpdateSpriteImage();
}

void SpriteViewerWindow::UpdateSpriteViewer(bool updateSprites)
{
    if (updateSprites)
    {
        gba_api::GetSpriteDebugInfo(sprites_, regTransforms_->isChecked(), affTransforms_->isChecked());
    }

    for (u8 i = 0; i < 128; ++i)
    {
        debug::Sprite const* spritePtr = sprites_[i].enabled ? &sprites_[i] : nullptr;
        bool doubleSized = spritePtr && spritePtr->doubleSize && affTransforms_->isChecked();
        spritePreviews_[i]->UpdatePreview(spritePtr, doubleSized);
    }

    UpdateSpriteInfo();
    UpdateSpriteImage();
}

void SpriteViewerWindow::UpdateSpriteInfo()
{
    debug::Sprite const& sprite = sprites_[selectedIndex_];
    indexLabel_->setText(QString::number(selectedIndex_));

    if (sprite.enabled)
    {
        coordinatesLabel_->setText(QString::fromStdString(std::format("{}, {}", sprite.x, sprite.y)));
        sizeLabel_->setText(QString::fromStdString(std::format("{} x {}", sprite.width, sprite.height)));
        tileLabel_->setText(QString::number(sprite.tileIndex));
        paletteLabel_->setText(QString::number((sprite.palette == U8_MAX) ? 256 : sprite.palette));
        priorityLabel_->setText(QString::number(sprite.priority));
        gfxLabel_->setText(QString::fromStdString(sprite.gxfMode));
        mosaicBox_->setEnabled(true);
        mosaicBox_->setChecked(sprite.mosaic);

        if (sprite.regular)
        {
            paLabel_->setText("---");
            pbLabel_->setText("---");
            pcLabel_->setText("---");
            pdLabel_->setText("---");
            affineBox_->setEnabled(true);
            affineBox_->setChecked(false);
            doubleSizeBox_->setEnabled(false);
            doubleSizeBox_->setChecked(false);
            vFlipBox_->setEnabled(true);
            vFlipBox_->setChecked(sprite.verticalFlip);
            hFlipBox_->setEnabled(true);
            hFlipBox_->setChecked(sprite.horizontalFlip);
        }
        else
        {
            paLabel_->setText(QString::fromStdString(std::format("{:.2f}", sprite.pa)));
            pbLabel_->setText(QString::fromStdString(std::format("{:.2f}", sprite.pb)));
            pcLabel_->setText(QString::fromStdString(std::format("{:.2f}", sprite.pc)));
            pdLabel_->setText(QString::fromStdString(std::format("{:.2f}", sprite.pd)));
            affineBox_->setEnabled(true);
            affineBox_->setChecked(true);
            doubleSizeBox_->setEnabled(true);
            doubleSizeBox_->setChecked(sprite.doubleSize);
            vFlipBox_->setEnabled(false);
            vFlipBox_->setChecked(false);
            hFlipBox_->setEnabled(false);
            hFlipBox_->setChecked(false);
        }
    }
    else
    {
        coordinatesLabel_->setText("...");
        sizeLabel_->setText("...");
        doubleSizeBox_->setEnabled(false);
        doubleSizeBox_->setChecked(false);
        paLabel_->setText("---");
        pbLabel_->setText("---");
        pcLabel_->setText("---");
        pdLabel_->setText("---");
        tileLabel_->setText("...");
        paletteLabel_->setText("...");
        priorityLabel_->setText("...");
        gfxLabel_->setText("...");
        affineBox_->setEnabled(false);
        affineBox_->setChecked(false);
        vFlipBox_->setEnabled(false);
        vFlipBox_->setChecked(false);
        hFlipBox_->setEnabled(false);
        hFlipBox_->setChecked(false);
        mosaicBox_->setEnabled(false);
        mosaicBox_->setChecked(false);
    }
}

void SpriteViewerWindow::UpdateSpriteImage()
{
    debug::Sprite const& sprite = sprites_[selectedIndex_];

    if (sprite.enabled)
    {
        u8 width = (sprite.doubleSize && affTransforms_->isChecked()) ? sprite.width * 2 : sprite.width;
        u8 height = (sprite.doubleSize && affTransforms_->isChecked()) ? sprite.height * 2 : sprite.height;
        QImage::Format format = transparencyControl_->isChecked() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        QImage image(reinterpret_cast<const uchar*>(sprite.buffer.data()), width, height, format);
        u8 scale = sizeBox_->value();
        imageLabel_->setPixmap(QPixmap::fromImage(image).scaled(width * scale, height * scale));
        imageLabel_->resize(width * scale, height * scale);
    }
    else
    {
        QImage image;
        imageLabel_->setPixmap(QPixmap::fromImage(image));
    }
}

QGroupBox* SpriteViewerWindow::CreateSpriteInfoGroup()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;

    //-------------
    // Controls Box
    //-------------
    QFormLayout* controlsLayout = new QFormLayout;

    sizeBox_ = new QSpinBox;
    sizeBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizeBox_->setMinimum(1);
    sizeBox_->setMaximum(8);
    sizeBox_->setSuffix("x  ");
    sizeBox_->setValue(1);
    connect(sizeBox_, &QSpinBox::valueChanged, this, &SpriteViewerWindow::UpdateSpriteImage);
    controlsLayout->addRow("Scale", sizeBox_);

    indexBox_ = new QSpinBox;
    indexBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    indexBox_->setMinimum(0);
    indexBox_->setMaximum(127);
    indexBox_->setSuffix("  ");
    indexBox_->setValue(0);
    connect(indexBox_, &QSpinBox::valueChanged, this, &SpriteViewerWindow::SelectedSpriteUpdatedSlot);
    controlsLayout->addRow("Index", indexBox_);

    transparencyControl_ = new QCheckBox;
    connect(transparencyControl_, &QCheckBox::checkStateChanged, this, SpriteViewerWindow::UpdateSpriteImage);
    controlsLayout->addRow("Transparency Mode", transparencyControl_);

    regTransforms_ = new QCheckBox;
    connect(regTransforms_, &QCheckBox::checkStateChanged, this, &SpriteViewerWindow::ForceUpdateSpriteViewerSlot);
    controlsLayout->addRow("Regular Transforms", regTransforms_);

    affTransforms_ = new QCheckBox;
    connect(affTransforms_, &QCheckBox::checkStateChanged, this, &SpriteViewerWindow::ForceUpdateSpriteViewerSlot);
    controlsLayout->addRow("Affine Transforms", affTransforms_);

    QGroupBox* controlsBox = new QGroupBox("Control");
    controlsBox->setLayout(controlsLayout);
    mainLayout->addWidget(controlsBox);

    //-------------
    // Geometry Box
    //-------------
    QHBoxLayout* geometryLayout = new QHBoxLayout;

    // Position
    QFormLayout* positionLayout = new QFormLayout;
    coordinatesLabel_ = new QLabel("0, 0");
    positionLayout->addRow("Coordinates:", coordinatesLabel_);
    sizeLabel_ = new QLabel("0 x 0");
    positionLayout->addRow("Size:", sizeLabel_);
    doubleSizeBox_ = new QCheckBox;
    doubleSizeBox_->setAttribute(Qt::WA_TransparentForMouseEvents);
    doubleSizeBox_->setFocusPolicy(Qt::NoFocus);
    positionLayout->addRow("Double Size:", doubleSizeBox_);

    QWidget* positionWidget = new QWidget;
    positionWidget->setLayout(positionLayout);
    geometryLayout->addWidget(positionWidget);

    // Separator
    QFrame* geometrySeparatorLine = new QFrame;
    geometrySeparatorLine->setFrameShape(QFrame::VLine);
    geometrySeparatorLine->setFrameShadow(QFrame::Sunken);

    geometryLayout->addWidget(geometrySeparatorLine);

    // Matrix
    QFormLayout* matrixLayout = new QFormLayout;
    paLabel_ = new QLabel("---");
    pbLabel_ = new QLabel("---");
    matrixLayout->addRow(paLabel_, pbLabel_);
    pcLabel_ = new QLabel("---");
    pdLabel_ = new QLabel("---");
    matrixLayout->addRow(pcLabel_, pdLabel_);

    QGroupBox* matrixWidget = new QGroupBox("Matrix");
    matrixWidget->setLayout(matrixLayout);
    geometryLayout->addWidget(matrixWidget);

    QGroupBox* geometryBox = new QGroupBox("Geometry");
    geometryBox->setLayout(geometryLayout);
    mainLayout->addWidget(geometryBox);

    //---------------
    // Attributes Box
    //---------------
    QHBoxLayout* attributesLayout = new QHBoxLayout;

    // Left
    QFormLayout* leftLayout = new QFormLayout;
    indexLabel_ = new QLabel("...");
    leftLayout->addRow("Index:", indexLabel_);
    tileLabel_ = new QLabel("...");
    leftLayout->addRow("Base Tile:", tileLabel_);
    paletteLabel_ = new QLabel("...");
    leftLayout->addRow("Palette:", paletteLabel_);
    priorityLabel_ = new QLabel("...");
    leftLayout->addRow("Priority:", priorityLabel_);
    gfxLabel_ = new QLabel("...");
    leftLayout->addRow("Gfx Mode:", gfxLabel_);

    QWidget* leftAttributes = new QWidget;
    leftAttributes->setLayout(leftLayout);
    attributesLayout->addWidget(leftAttributes);

    // Separator
    QFrame* attributesSeparatorLine = new QFrame;
    attributesSeparatorLine->setFrameShape(QFrame::VLine);
    attributesSeparatorLine->setFrameShadow(QFrame::Sunken);

    attributesLayout->addWidget(attributesSeparatorLine);

    // Right
    QFormLayout* rightLayout = new QFormLayout;
    affineBox_ = new QCheckBox;
    affineBox_->setAttribute(Qt::WA_TransparentForMouseEvents);
    affineBox_->setFocusPolicy(Qt::NoFocus);
    rightLayout->addRow("Affine:", affineBox_);
    vFlipBox_ = new QCheckBox;
    vFlipBox_->setAttribute(Qt::WA_TransparentForMouseEvents);
    vFlipBox_->setFocusPolicy(Qt::NoFocus);
    rightLayout->addRow("V Flip:", vFlipBox_);
    hFlipBox_ = new QCheckBox;
    hFlipBox_->setAttribute(Qt::WA_TransparentForMouseEvents);
    hFlipBox_->setFocusPolicy(Qt::NoFocus);
    rightLayout->addRow("H Flip:", hFlipBox_);
    mosaicBox_ = new QCheckBox;
    mosaicBox_->setAttribute(Qt::WA_TransparentForMouseEvents);
    mosaicBox_->setFocusPolicy(Qt::NoFocus);
    rightLayout->addRow("Mosaic:", mosaicBox_);

    QWidget* rightAttributes = new QWidget;
    rightAttributes->setLayout(rightLayout);
    attributesLayout->addWidget(rightAttributes);

    QGroupBox* attributesBox = new QGroupBox("Attributes");
    attributesBox->setLayout(attributesLayout);
    mainLayout->addWidget(attributesBox);

    //-----------
    // Main Group
    //-----------
    QGroupBox* mainGroupBox = new QGroupBox("OAM Entry Data");
    mainLayout->addStretch();
    mainGroupBox->setLayout(mainLayout);
    mainGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    return mainGroupBox;
}

QScrollArea* SpriteViewerWindow::CreateSpriteImage()
{
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setMinimumSize(264, 264);
    scrollArea->setAlignment(Qt::AlignCenter);

    imageLabel_ = new QLabel;
    imageLabel_->setScaledContents(true);

    scrollArea->setWidget(imageLabel_);
    return scrollArea;
}

QScrollArea* SpriteViewerWindow::CreateSpritePreviews()
{
    FlowLayout* grid = new FlowLayout;

    for (u8 row = 0; row < 16; ++row)
    {
        for (u8 col = 0; col < 8; ++col)
        {
            u8 index = (row * 8) + col;
            spritePreviews_[index] = new SpritePreviewWidget(index);
            connect(spritePreviews_[index], &SpritePreviewWidget::SpriteSelectedSignal,
                    this, &SpriteViewerWindow::SelectedSpriteUpdatedSlot);
            grid->addWidget(spritePreviews_[index]);
        }
    }

    QWidget* layoutHolder = new QWidget;
    layoutHolder->setLayout(grid);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(layoutHolder);
    return scrollArea;
}
}  // namespace gui
