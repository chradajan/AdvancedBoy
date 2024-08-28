#include <GUI/include/BackgroundViewer.hpp>
#include <format>
#include <string>
#include <GBA/include/PPU/Debug.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

namespace gui
{
BackgroundViewer::BackgroundViewer() : QWidget()
{
    setWindowTitle("Background Viewer");

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(CreateSelectionGroup(), 0, 0);
    grid->addWidget(CreateBgInfoGroup(), 1, 0);
    grid->addWidget(CreateBackgroundImage(), 0, 1, 3, 1);

    setLayout(grid);
    selectedBg_ = 0;
}

void BackgroundViewer::UpdateBackgroundView(graphics::BackgroundDebugInfo const& info)
{
    auto image = QImage(reinterpret_cast<const uchar*>(info.buffer.data()), info.width, info.height, QImage::Format_RGB555);
    image.rgbSwap();
    bgImageLabel_->setPixmap(QPixmap::fromImage(image));
    u8 scale = scaleBox_->currentIndex() + 1;
    bgImageLabel_->resize(info.width * scale, info.height * scale);

    priorityLabel_->setText(QString::number(info.priority));
    tileBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", info.mapBaseAddr)));
    mapBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", info.tileBaseAddr)));
    sizeLabel_->setText(QString::fromStdString(std::format("{}x{}", info.width, info.height)));

    if (info.regular)
    {
        offsetLabel_->setText(QString::fromStdString(std::format("{}, {}", info.xOffset, info.yOffset)));
        matrixLabel_->setText("N/A");
    }
    else
    {
        offsetLabel_->setText(QString::fromStdString(std::format("{:.2f}, {:.2f}", info.refX, info.refY)));
        matrixLabel_->setText(QString::fromStdString(std::format("{:{}.2f} {:{}.2f}\n{:{}.2f} {:{}.2f}",
                                                                 info.pa, 7,
                                                                 info.pb, 7,
                                                                 info.pc, 7,
                                                                 info.pd, 7)));
    }
}

QGroupBox* BackgroundViewer::CreateSelectionGroup()
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

QGroupBox* BackgroundViewer::CreateBgInfoGroup()
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

QScrollArea* BackgroundViewer::CreateBackgroundImage()
{
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(false);
    scrollArea->setMinimumSize(520, 520);

    bgImageLabel_ = new QLabel;
    bgImageLabel_->setScaledContents(true);

    scrollArea->setWidget(bgImageLabel_);
    return scrollArea;
}
}  // namespace gui