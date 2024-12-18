#include <GUI/include/DebugWindows/RegisterViewerWindow.hpp>
#include <format>
#include <string>
#include <GUI/include/DebugWindows/RegisterInfo.hpp>
#include <GUI/include/GBA.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace gui
{
RegisterViewerWindow::RegisterViewerWindow() : QWidget(), prevRegVal_(U32_MAX), prevIndex_(0)
{
    setWindowTitle("I/O Registers");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(CreateDropDown());
    layout->addWidget(new QGroupBox);

    QPushButton* refreshButton = new QPushButton("Refresh");
    connect(refreshButton, &QPushButton::clicked,
            this, &RegisterViewerWindow::UpdateSelectedRegisterData);
    layout->addWidget(refreshButton);

    setLayout(layout);
    UpdateSelectedRegisterData();
    adjustSize();
    setFixedSize(size());
}

void RegisterViewerWindow::UpdateSelectedRegisterData()
{
    int currIndex = registerSelect_->currentIndex();
    Register const& reg = IO_REGISTERS[currIndex];
    u32 regVal = gba_api::DebugReadRegister(reg.addr, reg.size);

    if ((currIndex == prevIndex_) && (regVal == prevRegVal_))
    {
        return;
    }

    QWidget* currentRegisterData = layout()->itemAt(1)->widget();
    layout()->replaceWidget(currentRegisterData, CreateRegisterData(reg, regVal));
    delete currentRegisterData;
    update();

    if (prevIndex_ != currIndex)
    {
        adjustSize();
        setFixedSize(size());
    }

    prevRegVal_ = regVal;
    prevIndex_ = currIndex;
}

QComboBox* RegisterViewerWindow::CreateDropDown()
{
    registerSelect_ = new QComboBox;
    registerSelect_->blockSignals(true);
    connect(registerSelect_, &QComboBox::currentIndexChanged,
            this, &RegisterViewerWindow::UpdateRegisterViewSlot);

    for (Register const& ioReg : IO_REGISTERS)
    {
        std::string entryName = std::format("0x{:08X} - {} ({})", ioReg.addr, ioReg.name, ioReg.description);
        registerSelect_->addItem(QString::fromStdString(entryName));
    }

    registerSelect_->blockSignals(false);
    return registerSelect_;
}

QGroupBox* RegisterViewerWindow::CreateRegisterData(Register const& reg, u32 regVal) const
{
    QGroupBox* groupBox = new QGroupBox;
    QFormLayout* layout = new QFormLayout;
    layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->setLabelAlignment(Qt::AlignLeft);

    std::string groupBoxTitle = std::format("{} = 0x{:0{}X}", reg.name, regVal, reg.size * 2);
    groupBox->setTitle(QString::fromStdString(groupBoxTitle));

    for (RegisterField const& field : reg.fields)
    {
        u32 fieldVal = (regVal & field.mask) >> field.shift;
        QWidget* fieldWidget = nullptr;

        switch (field.format)
        {
            case DisplayFormat::BOOL:
                fieldWidget = new QCheckBox;
                fieldWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
                fieldWidget->setFocusPolicy(Qt::NoFocus);
                static_cast<QCheckBox*>(fieldWidget)->setChecked(fieldVal == 1);
                layout->addRow(QString::fromStdString(field.name), fieldWidget);
                break;
            case DisplayFormat::DEC:
                fieldWidget = new QLineEdit;
                static_cast<QLineEdit*>(fieldWidget)->setReadOnly(true);
                static_cast<QLineEdit*>(fieldWidget)->setFocusPolicy(Qt::NoFocus);
                static_cast<QLineEdit*>(fieldWidget)->setAlignment(Qt::AlignRight);
                static_cast<QLineEdit*>(fieldWidget)->setText(QString::fromStdString(std::to_string(fieldVal)));
                layout->addRow(QString::fromStdString(field.name), fieldWidget);
                break;
            case DisplayFormat::HEX:
                fieldWidget = new QLineEdit;
                static_cast<QLineEdit*>(fieldWidget)->setReadOnly(true);
                static_cast<QLineEdit*>(fieldWidget)->setFocusPolicy(Qt::NoFocus);
                static_cast<QLineEdit*>(fieldWidget)->setAlignment(Qt::AlignRight);
                static_cast<QLineEdit*>(fieldWidget)->setText(QString::fromStdString(std::format("{:X}", fieldVal)));
                layout->addRow(QString::fromStdString(field.name), fieldWidget);
                break;
        }
    }

    groupBox->setLayout(layout);
    return groupBox;
}
}  // namespace gui
