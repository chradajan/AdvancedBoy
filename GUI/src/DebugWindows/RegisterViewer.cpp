#include <GUI/include/DebugWindows/RegisterViewer.hpp>
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
RegisterViewer::RegisterViewer() : QWidget()
{
    setWindowTitle("I/O Registers");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(CreateDropDown());
    layout->addWidget(new QGroupBox);

    QPushButton* refreshButton = new QPushButton("Refresh");
    connect(refreshButton, &QPushButton::clicked,
            this, &RegisterViewer::UpdateSelectedRegisterData);
    layout->addWidget(refreshButton);

    setLayout(layout);
    UpdateSelectedRegisterData();
}

void RegisterViewer::UpdateSelectedRegisterData()
{
    QWidget* currentRegisterData = layout()->itemAt(1)->widget();
    layout()->replaceWidget(currentRegisterData, CreateRegisterData());
    delete currentRegisterData;
    update();
    adjustSize();
    setFixedSize(size());
}

QComboBox* RegisterViewer::CreateDropDown()
{
    registerSelect_ = new QComboBox;
    registerSelect_->blockSignals(true);
    connect(registerSelect_, &QComboBox::currentIndexChanged,
            this, &RegisterViewer::UpdateRegisterViewSlot);

    for (Register const& ioReg : IO_REGISTERS)
    {
        std::string entryName = std::format("0x{:08X} - {} ({})", ioReg.addr, ioReg.name, ioReg.description);
        registerSelect_->addItem(QString::fromStdString(entryName));
    }

    registerSelect_->blockSignals(false);
    return registerSelect_;
}

QGroupBox* RegisterViewer::CreateRegisterData()
{
    QGroupBox* groupBox = new QGroupBox;
    QFormLayout* layout = new QFormLayout;
    layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->setLabelAlignment(Qt::AlignLeft);

    int index = registerSelect_->currentIndex();
    Register const& selectedRegister = IO_REGISTERS[index];
    u32 registerVal = gba_api::DebugReadRegister(selectedRegister.addr, selectedRegister.size);
    std::string groupBoxTitle = std::format("{} = 0x{:0{}X}", selectedRegister.name, registerVal, selectedRegister.size * 2);
    groupBox->setTitle(QString::fromStdString(groupBoxTitle));

    for (RegisterField const& field : selectedRegister.fields)
    {
        u32 fieldVal = (registerVal & field.mask) >> field.shift;
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
