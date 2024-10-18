#include <GUI/include/DebugWindows/BackgroundViewerWindow.hpp>
#include <format>
#include <string>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/GBA.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

namespace gui
{
using namespace debug;

BackgroundViewerWindow::BackgroundViewerWindow() : QWidget()
{
    setWindowTitle("Background Viewer");

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(CreateSelectionGroup(), 0, 0);
    grid->addWidget(CreateBgInfoGroup(), 1, 0);
    grid->addWidget(CreateBackgroundImage(), 0, 1, 3, 1);

    setLayout(grid);
    selectedBg_ = 0;
}

void BackgroundViewerWindow::UpdateBackgroundView()
{
    if (!isVisible())
    {
        return;
    }

    auto debugInfo = gba_api::GetBgDebugInfo(selectedBg_);
    auto image =
        QImage(reinterpret_cast<const uchar*>(debugInfo.buffer.data()), debugInfo.width, debugInfo.height, QImage::Format_RGB555);
    image.rgbSwap();
    u8 scale = scaleBox_->currentIndex() + 1;
    bgImageLabel_->setPixmap(QPixmap::fromImage(image).scaled(debugInfo.width * scale, debugInfo.height * scale));
    bgImageLabel_->resize(debugInfo.width * scale, debugInfo.height * scale);

    priorityLabel_->setText(QString::number(debugInfo.priority));
    tileBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", debugInfo.mapBaseAddr)));
    mapBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", debugInfo.tileBaseAddr)));
    sizeLabel_->setText(QString::fromStdString(std::format("{}x{}", debugInfo.width, debugInfo.height)));

    if (debugInfo.regular)
    {
        offsetLabel_->setText(QString::fromStdString(std::format("{}, {}", debugInfo.xOffset, debugInfo.yOffset)));
        matrixLabel_->setText("N/A");
    }
    else
    {
        offsetLabel_->setText(QString::fromStdString(std::format("{:.2f}, {:.2f}", debugInfo.refX, debugInfo.refY)));
        matrixLabel_->setText(QString::fromStdString(std::format("{:{}.2f} {:{}.2f}\n{:{}.2f} {:{}.2f}",
                                                                 debugInfo.pa, 7,
                                                                 debugInfo.pb, 7,
                                                                 debugInfo.pc, 7,
                                                                 debugInfo.pd, 7)));
    }
}

QGroupBox* BackgroundViewerWindow::CreateSelectionGroup()
{
    QGroupBox* groupBox = new QGroupBox("Select map");

    QLabel* scaleLabel = new QLabel("Scale");
    scaleBox_ = new QComboBox;
    scaleBox_->setMaximumWidth(75);

    for (u8 scale = 1; scale < 5; ++scale)
    {
        scaleBox_->addItem(QString::fromStdString(std::format("{}x", scale)));
    }

    QRadioButton* bg0Button = new QRadioButton("Background 0");
    connect(bg0Button, &QRadioButton::clicked, this, [=, this] () { this->SetSelectedBg(0); });
    QRadioButton* bg1Button = new QRadioButton("Background 1");
    connect(bg1Button, &QRadioButton::clicked, this, [=, this] () { this->SetSelectedBg(1); });
    QRadioButton* bg2Button = new QRadioButton("Background 2");
    connect(bg2Button, &QRadioButton::clicked, this, [=, this] () { this->SetSelectedBg(2); });
    QRadioButton* bg3Button = new QRadioButton("Background 3");
    connect(bg3Button, &QRadioButton::clicked, this, [=, this] () { this->SetSelectedBg(3); });
    bg0Button->setChecked(true);

    QVBoxLayout* groupLayout = new QVBoxLayout;
    groupLayout->addWidget(scaleLabel);
    groupLayout->addWidget(scaleBox_);
    groupLayout->addSpacing(15);
    groupLayout->addWidget(bg0Button);
    groupLayout->addWidget(bg1Button);
    groupLayout->addWidget(bg2Button);
    groupLayout->addWidget(bg3Button);
    groupBox->setFixedWidth(200);
    groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    groupBox->setLayout(groupLayout);

    return groupBox;
}

QGroupBox* BackgroundViewerWindow::CreateBgInfoGroup()
{
    QGroupBox* groupBox = new QGroupBox("Map info");
    QGridLayout* groupLayout  = new QGridLayout;

    groupLayout->addWidget(new QLabel("Priority"), 0, 0);
    priorityLabel_ = new QLabel("0");
    groupLayout->addWidget(priorityLabel_, 0, 1);

    groupLayout->addWidget(new QLabel("Map base"), 1, 0);
    mapBaseLabel_ = new QLabel("0x06000000");
    groupLayout->addWidget(mapBaseLabel_, 1, 1);

    groupLayout->addWidget(new QLabel("Tile base"), 2, 0);
    tileBaseLabel_ = new QLabel("0x06000000");
    groupLayout->addWidget(tileBaseLabel_, 2, 1);

    groupLayout->addWidget(new QLabel("Size"), 3, 0);
    sizeLabel_ = new QLabel("0x0");
    groupLayout->addWidget(sizeLabel_, 3, 1);

    groupLayout->addWidget(new QLabel("Offset"), 4, 0);
    offsetLabel_ = new QLabel("0, 0");
    groupLayout->addWidget(offsetLabel_, 4, 1);

    groupLayout->addWidget(new QLabel("Matrix"), 5, 0);
    matrixLabel_ = new QLabel("N/A");
    groupLayout->addWidget(matrixLabel_, 5, 1);

    groupBox->setLayout(groupLayout);
    groupBox->setFixedWidth(200);
    groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return groupBox;
}

QScrollArea* BackgroundViewerWindow::CreateBackgroundImage()
{
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(false);
    scrollArea->setMinimumSize(520, 520);

    bgImageLabel_ = new QLabel;
    bgImageLabel_->setScaledContents(true);

    scrollArea->setWidget(bgImageLabel_);
    return scrollArea;
}

void BackgroundViewerWindow::SetSelectedBg(u8 bgIndex)
{
    selectedBg_ = bgIndex;
    UpdateBackgroundView();
}
}  // namespace gui
