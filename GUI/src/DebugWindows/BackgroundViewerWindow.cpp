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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QButtonGroup>

namespace gui
{
using namespace debug;

BackgroundViewerWindow::BackgroundViewerWindow() : QWidget()
{
    setWindowTitle("Background Viewer");

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(CreateControlGroup(), 0, 0);
    grid->addWidget(CreateBgInfoGroup(), 1, 0);
    grid->addWidget(CreateBackgroundImage(), 0, 1, 3, 1);

    setLayout(grid);
    debugInfo_.width = 0;
    debugInfo_.height = 0;
    selectedBg_ = 0;
}

void BackgroundViewerWindow::SetSelectedBg(int bgIndex)
{
    if (selectedBg_ != bgIndex)
    {
        selectedBg_ = bgIndex;
        UpdateBackgroundView(true);
    }
}

void BackgroundViewerWindow::UpdateBackgroundView(bool updateBg)
{
    if (updateBg)
    {
        gba_api::GetBgDebugInfo(debugInfo_, selectedBg_);
    }

    bool invalid = (debugInfo_.width == 0) || (debugInfo_.height == 0);
    QImage::Format format = transparencyControl_->isChecked() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    auto image = QImage(reinterpret_cast<const uchar*>(&debugInfo_.buffer[0]), debugInfo_.width, debugInfo_.height, format);
    u8 scale = scaleBox_->value();
    bgImageLabel_->setPixmap(QPixmap::fromImage(image).scaled(debugInfo_.width * scale, debugInfo_.height * scale));
    bgImageLabel_->resize(debugInfo_.width * scale, debugInfo_.height * scale);

    if (invalid)
    {
        priorityLabel_->setText("---");
        tileBaseLabel_->setText("---");
        mapBaseLabel_->setText("---");
        sizeLabel_->setText("---");
        offsetLabel_->setText("---");
        paLabel_->setText("---");
        pbLabel_->setText("---");
        pcLabel_->setText("---");
        pdLabel_->setText("---");
    }
    else
    {
        priorityLabel_->setText(QString::number(debugInfo_.priority));
        tileBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", debugInfo_.mapBaseAddr)));
        mapBaseLabel_->setText(QString::fromStdString(std::format("0x{:08X}", debugInfo_.tileBaseAddr)));
        sizeLabel_->setText(QString::fromStdString(std::format("{}x{}", debugInfo_.width, debugInfo_.height)));

        if (debugInfo_.regular)
        {
            offsetLabel_->setText(QString::fromStdString(std::format("{}, {}", debugInfo_.xOffset, debugInfo_.yOffset)));
            paLabel_->setText("---");
            pbLabel_->setText("---");
            pcLabel_->setText("---");
            pdLabel_->setText("---");
        }
        else
        {
            offsetLabel_->setText(QString::fromStdString(std::format("{:.2f}, {:.2f}", debugInfo_.refX, debugInfo_.refY)));
            paLabel_->setText(QString::fromStdString(std::format("{:.2f}", debugInfo_.pa)));
            pbLabel_->setText(QString::fromStdString(std::format("{:.2f}", debugInfo_.pb)));
            pcLabel_->setText(QString::fromStdString(std::format("{:.2f}", debugInfo_.pc)));
            pdLabel_->setText(QString::fromStdString(std::format("{:.2f}", debugInfo_.pd)));
        }
    }
}

QGroupBox* BackgroundViewerWindow::CreateControlGroup()
{
    QGroupBox* groupBox = new QGroupBox("Control");
    QVBoxLayout* groupLayout = new QVBoxLayout;

    // Controls
    QWidget* controlWidget = new QWidget;
    QFormLayout* controlLayout = new QFormLayout;

    scaleBox_ = new QSpinBox;
    scaleBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    scaleBox_->setMinimum(1);
    scaleBox_->setMaximum(4);
    scaleBox_->setSuffix("x  ");
    scaleBox_->setValue(1);
    connect(scaleBox_, &QSpinBox::valueChanged, this, [=, this] () { this->UpdateBackgroundView(false); });
    controlLayout->addRow("Scale", scaleBox_);

    transparencyControl_ = new QCheckBox;
    connect(transparencyControl_, &QCheckBox::checkStateChanged, this, [=, this] () { this->UpdateBackgroundView(false); });
    controlLayout->addRow("Transparency Mode", transparencyControl_);

    controlWidget->setLayout(controlLayout);
    groupLayout->addWidget(controlWidget);

    // BG Select
    QWidget* buttonWidget = new QWidget;
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    QButtonGroup* buttonGroup = new QButtonGroup;

    QRadioButton* button0 = new QRadioButton("Background 0");
    button0->setChecked(true);
    buttonLayout->addWidget(button0);
    buttonGroup->addButton(button0, 0);
    QRadioButton* button1 = new QRadioButton("Background 1");
    buttonLayout->addWidget(button1);
    buttonGroup->addButton(button1, 1);
    QRadioButton* button2 = new QRadioButton("Background 2");
    buttonLayout->addWidget(button2);
    buttonGroup->addButton(button2, 2);
    QRadioButton* button3 = new QRadioButton("Background 3");
    buttonLayout->addWidget(button3);
    buttonGroup->addButton(button3, 3);

    connect(buttonGroup, &QButtonGroup::idClicked, this, &BackgroundViewerWindow::SetSelectedBg);
    buttonWidget->setLayout(buttonLayout);
    groupLayout->addWidget(buttonWidget);

    groupBox->setLayout(groupLayout);
    groupBox->setFixedWidth(200);
    groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    return groupBox;
}

QGroupBox* BackgroundViewerWindow::CreateBgInfoGroup()
{
    QGroupBox* groupBox = new QGroupBox("Map Info");
    QVBoxLayout* groupLayout = new QVBoxLayout;

    // BG Info
    QWidget* bgInfoWidget = new QWidget;
    QFormLayout* bgInfoLayout = new QFormLayout;

    priorityLabel_ = new QLabel("0");
    bgInfoLayout->addRow("Priority:", priorityLabel_);

    mapBaseLabel_ = new QLabel("---");
    bgInfoLayout->addRow("Map Base:", mapBaseLabel_);

    tileBaseLabel_ = new QLabel("---");
    bgInfoLayout->addRow("Tile Base:", tileBaseLabel_);

    sizeLabel_ = new QLabel("---");
    bgInfoLayout->addRow("Size:", sizeLabel_);

    offsetLabel_ = new QLabel("---");
    bgInfoLayout->addRow("Offset", offsetLabel_);

    bgInfoWidget->setLayout(bgInfoLayout);
    groupLayout->addWidget(bgInfoWidget);

    // Matrix
    QGroupBox* matrixWidget = new QGroupBox("Matrix");
    QFormLayout* matrixLayout = new QFormLayout;

    paLabel_ = new QLabel("---");
    pbLabel_ = new QLabel("---");
    matrixLayout->addRow(paLabel_, pbLabel_);
    pcLabel_ = new QLabel("---");
    pdLabel_ = new QLabel("---");
    matrixLayout->addRow(pcLabel_, pdLabel_);

    matrixWidget->setLayout(matrixLayout);
    matrixWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    groupLayout->addWidget(matrixWidget);

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
}  // namespace gui
