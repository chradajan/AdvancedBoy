#include <GUI/include/DebugWindows/CpuDebuggerWindow.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <format>
#include <optional>
#include <set>
#include <unordered_set>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <GUI/include/GBA.hpp>
#include <QtCore/QString>
#include <QtGui/QTextBlock>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

namespace
{
static constexpr std::pair<u32, u32> INVALID_BOUNDS = {U32_MAX, U32_MAX};

/// @brief 
/// @param regIndex 
/// @param val 
/// @return 
QString RegString(u8 regIndex, u32 val)
{
    return QString::fromStdString(std::format("R{:<2}  0x{:08X}", regIndex, val));
}

/// @brief Convert a 5-bit operating mode value to a string.
/// @param mode ARM operating mode raw value.
/// @return String representing the current operating mode.
QString ModeString(u8 mode)
{
    switch (mode)
    {
        case 0b10000:
            return "Mode:  User";
        case 0b10001:
            return "Mode:  FIQ";
        case 0b10010:
            return "Mode:  IRQ";
        case 0b10011:
            return "Mode:  Supervisor";
        case 0b10111:
            return "Mode:  Abort";
        case 0b11011:
            return "Mode:  Undefined";
        case 0b11111:
            return "Mode:  System";
        default:
            return "Mode:  Invalid";
    }
}

/// @brief Create a non-interactable checkbox to display a CPSR flag.
/// @param text Label text for the check box.
/// @return Pointer to newly created check box.
[[nodiscard]] QCheckBox* CreateCpsrCheckBox(QString text)
{
    QCheckBox* checkBox = new QCheckBox(text);
    checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);
    checkBox->setFocusPolicy(Qt::NoFocus);
    return checkBox;
}

/// @brief Write a formatted disassembled line to the disassembly widget.
/// @param text Pointer to the disassembly widget.
/// @param mnemonic Mnemonic of instruction to write.
/// @param addr Address of the instruction.
/// @param instruction Raw ARM/THUMB instruction value.
/// @param current Whether this is the next instruction to be executed by the CPU.
/// @param arm True if in ARM mode, false for THUMB.
/// @param newLine Whether to insert a new line character at the end of the line.
/// @param breakpoints Set of currently active breakpoints.
void WriteDisassembledLine(QTextEdit* text,
                           debug::Mnemonic const& mnemonic,
                           u32 addr,
                           u32 instruction,
                           bool current,
                           bool arm,
                           bool newLine,
                           std::unordered_set<u32> const& breakpoints)
{
    if (breakpoints.contains(addr))
    {
        text->setTextColor(QColorConstants::Black);
        text->insertPlainText("B");
    }
    else
    {
        text->insertPlainText(" ");
    }

    text->setTextColor(QColorConstants::DarkMagenta);
    text->insertPlainText(current ? " -> " : "    ");

    text->setTextColor(QColorConstants::Black);
    text->insertPlainText(QString::fromStdString(std::format("0x{:08X}  ", addr)));

    text->setTextColor(QColorConstants::DarkGreen);
    std::string undecodedStr = arm ? std::format("{:08X}  ", instruction) : std::format("{:04X}  ", instruction & U16_MAX);
    text->insertPlainText(QString::fromStdString(undecodedStr));

    text->setTextColor(QColorConstants::DarkBlue);
    text->insertPlainText(QString::fromStdString(mnemonic.op));

    text->setTextColor(QColorConstants::DarkRed);
    text->insertPlainText(QString::fromStdString(mnemonic.cond));

    auto spacerStr = std::string(10 - mnemonic.op.size() - mnemonic.cond.size(), ' ');
    text->insertPlainText(QString::fromStdString(spacerStr));

    text->setTextColor(QColorConstants::Black);

    if (mnemonic.branchOffset.has_value())
    {
        std::string args = std::format("0x{:08X}", addr + mnemonic.branchOffset.value());
        text->insertPlainText(QString::fromStdString(args));
    }
    else
    {
        text->insertPlainText(QString::fromStdString(mnemonic.args));
    }

    if (newLine)
    {
        text->insertPlainText("\n");
    }
}
}  // namespace

namespace gui
{
///---------------------------------------------------------------------------------------------------------------------------------
/// BreakpointsList
///---------------------------------------------------------------------------------------------------------------------------------

BreakpointsList::BreakpointsList() : QWidget()
{
    setWindowTitle("Active Breakpoints");
    QGridLayout* grid = new QGridLayout;

    list_ = new QListWidget;
    QPushButton* okButton = new QPushButton("Ok");
    QPushButton* removeButton = new QPushButton("Remove");

    connect(okButton, &QPushButton::clicked, this, &BreakpointsList::close);
    connect(removeButton, &QPushButton::clicked, this, &BreakpointsList::RemoveSelectedBreakpoint);

    grid->addWidget(list_, 0, 0, 1, 2);
    grid->addWidget(okButton, 1, 0);
    grid->addWidget(removeButton, 1, 1);
    setLayout(grid);
    UpdateList();
}

void BreakpointsList::RemoveSelectedBreakpoint()
{
    QListWidgetItem* selectedItem = list_->currentItem();

    if (selectedItem != nullptr)
    {
        bool valid = false;
        u32 breakpoint = selectedItem->text().toUInt(&valid, 0);

        if (valid)
        {
            emit BreakpointRemovedSignal(breakpoint);
            UpdateList();
        }
    }
}

void BreakpointsList::UpdateList()
{
    list_->clear();
    auto breakpoints = gba_api::GetBreakpoints();
    std::set<u32> sortedBreakpoints(breakpoints.begin(), breakpoints.end());

    for (u32 breakpoint : sortedBreakpoints)
    {
        list_-> addItem(QString::fromStdString(std::format("0x{:08X}", breakpoint)));
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// CpuDebuggerWindow
///---------------------------------------------------------------------------------------------------------------------------------

CpuDebuggerWindow::CpuDebuggerWindow() :
    QWidget(),
    runSingleFrame_(false),
    pcPage_(Page::INVALID),
    currPC_(U32_MAX),
    currArmMode_(true),
    pcIndexBounds_(INVALID_BOUNDS),
    spLineNum_(U32_MAX),
    spLowerIndexBound_(U32_MAX)
{
    setWindowTitle("CPU Debugger");

    QGridLayout* grid = new QGridLayout;
    grid->setColumnStretch(0, 1);

    grid->addWidget(CreateDisassemblyGroup(), 0, 0, 3, 1);
    grid->addWidget(CreateRegistersGroup(), 0, 1);
    grid->addWidget(CreateCpsrGroup(), 1, 1);
    grid->addWidget(CreateStackGroup(), 2, 1);
    setLayout(grid);

    breakpointsWindow_ = new BreakpointsList();

    connect(breakpointsWindow_, &BreakpointsList::BreakpointRemovedSignal,
            this, &CpuDebuggerWindow::BreakpointRemovedSlot);

    connect(this, &CpuDebuggerWindow::BreakpointsUpdatedSignal,
            breakpointsWindow_, &BreakpointsList::UpdateBreakpointsSlot);
}

CpuDebuggerWindow::~CpuDebuggerWindow()
{
    delete breakpointsWindow_;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Widget Updates
///---------------------------------------------------------------------------------------------------------------------------------

void CpuDebuggerWindow::UpdateWidgets()
{
    auto debugInfo = gba_api::GetCpuDebugInfo();
    PopulateDisassemblyGroup(debugInfo);
    PopulateRegistersGroup(debugInfo);
    PopulateCpsrGroup(debugInfo);
    PopulateStackGroup(debugInfo);
}

void CpuDebuggerWindow::PopulateDisassemblyGroup(::debug::CpuDebugInfo const& debugInfo)
{
    if ((debugInfo.pcMem.page == Page::INVALID) || debugInfo.pcMem.memoryBlock.empty())
    {
        return;
    }

    bool const armMode = !debugInfo.regState.thumbState;
    u32 const pcIndex = debugInfo.pcMem.AddrToIndex(debugInfo.nextAddrToExecute);
    u8 const instructionSize = armMode ? 4 : 2;
    auto const& breakpoints = gba_api::GetBreakpoints();

    // Check if entire widget needs to be rewritten
    // 1. CPU has branched to a different memory page than what's currently displayed
    // 2. CPU has switched operating state (ARM <-> THUMB)
    // 3. Disassembly widget is currently uninitialized
    // 4. CPU has branched to an instruction more than 100 lines before the current lower bound of displayed instructions
    // 5. CPU has branched to an instruction more than 100 lines after the current upper bound of displayed instructions
    if ((pcPage_ != debugInfo.pcMem.page) ||
        (currArmMode_ != armMode) ||
        (pcIndexBounds_ == INVALID_BOUNDS) ||
        ((pcIndex < pcIndexBounds_.first) && (((pcIndexBounds_.first - pcIndex) / instructionSize) > 100)) ||
        ((pcIndex > pcIndexBounds_.second) && (((pcIndex - pcIndexBounds_.second) / instructionSize) > 100)))
    {
        disassemblyWidget_->clear();
        addrToLineMap_.clear();

        u32 offset = 200 * instructionSize;
        u32 lowerBoundIndex = offset > pcIndex ? 0 : pcIndex - offset;
        u32 upperBoundIndex = std::min(pcIndex + offset + instructionSize,
                                       static_cast<u32>(debugInfo.pcMem.memoryBlock.size()));
        u32 lineNum = 0;

        for (u32 i = lowerBoundIndex; i < upperBoundIndex; i += instructionSize)
        {
            u32 addr = i + debugInfo.pcMem.minAddr;
            u32 instruction =
                armMode ? MemCpyInit<u32>(&debugInfo.pcMem.memoryBlock[i]) : MemCpyInit<u16>(&debugInfo.pcMem.memoryBlock[i]);
            auto const& mnemonic = armMode ? gba_api::DisassembleArmInstruction(instruction) :
                                             gba_api::DisassembleThumbInstruction(instruction);
            WriteDisassembledLine(disassemblyWidget_, mnemonic, addr, instruction, false, armMode, true, breakpoints);
            addrToLineMap_[addr] = lineNum++;
        }

        disassemblyWidget_->textCursor().deletePreviousChar();
        pcIndexBounds_ = {lowerBoundIndex, upperBoundIndex};
    }

    // If CPU branches to an instruction <= 100 lines before the first displayed instruction, prepend the currently displayed
    // instructions with the preceding 100 instructions.
    else if ((pcIndex < pcIndexBounds_.first) && (((pcIndexBounds_.first - pcIndex) / instructionSize) <= 100))
    {
        // First move the cursor to the top and insert an empty line
        auto cursor = disassemblyWidget_->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.insertText("\n");
        cursor.movePosition(QTextCursor::Start);
        disassemblyWidget_->setTextCursor(cursor);

        // Write up to 100 lines before the currently displayed instructions
        u32 offset = 100 * instructionSize;
        u32 lowerBoundIndex = (offset > pcIndexBounds_.first) ? 0 : pcIndexBounds_.first - offset;
        u32 lineNum = 0;

        for (u32 i = lowerBoundIndex; i < pcIndexBounds_.first; i += instructionSize)
        {
            u32 addr = i + debugInfo.pcMem.minAddr;
            u32 instruction =
                armMode ? MemCpyInit<u32>(&debugInfo.pcMem.memoryBlock[i]) : MemCpyInit<u16>(&debugInfo.pcMem.memoryBlock[i]);
            auto const& mnemonic = armMode ? gba_api::DisassembleArmInstruction(instruction) :
                                             gba_api::DisassembleThumbInstruction(instruction);
            WriteDisassembledLine(disassemblyWidget_, mnemonic, addr, instruction, false, armMode, true, breakpoints);
            addrToLineMap_[addr] = lineNum++;
        }

        for (u32 i = pcIndexBounds_.first; i < pcIndexBounds_.second; i += instructionSize)
        {
            u32 addr = i + debugInfo.pcMem.minAddr;
            auto it = addrToLineMap_.find(addr);

            if (it != addrToLineMap_.end())
            {
                it->second = lineNum;
            }

            ++lineNum;
        }

        disassemblyWidget_->textCursor().deletePreviousChar();
        pcIndexBounds_ = {lowerBoundIndex, pcIndexBounds_.second};
    }

    // If CPU branches to an instruction <= 100 lines after the last displayed instruction, append the next 100 instructions to the
    // currently displayed instructions.
    else if ((pcIndex > pcIndexBounds_.second) && (((pcIndex - pcIndexBounds_.second) / instructionSize) <= 100))
    {
        // First move the cursor to the end and insert an empty line
        auto cursor = disassemblyWidget_->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n");
        cursor.movePosition(QTextCursor::End);
        disassemblyWidget_->setTextCursor(cursor);

        // Write up to 100 lines after the currently displayed instructions
        u32 offset = 100 * instructionSize;
        u32 upperBoundIndex = std::min(pcIndexBounds_.second + offset + instructionSize,
                                       static_cast<u32>(debugInfo.pcMem.memoryBlock.size()));
        u32 lineNum = addrToLineMap_[pcIndexBounds_.second + debugInfo.pcMem.minAddr - instructionSize] + 1;

        for (u32 i = pcIndexBounds_.second; i < upperBoundIndex; i += instructionSize)
        {
            u32 addr = i + debugInfo.pcMem.minAddr;
            u32 instruction =
                armMode ? MemCpyInit<u32>(&debugInfo.pcMem.memoryBlock[i]) : MemCpyInit<u16>(&debugInfo.pcMem.memoryBlock[i]);
            auto const& mnemonic = armMode ? gba_api::DisassembleArmInstruction(instruction) :
                                             gba_api::DisassembleThumbInstruction(instruction);
            WriteDisassembledLine(disassemblyWidget_, mnemonic, addr, instruction, false, armMode, true, breakpoints);
            addrToLineMap_[addr] = lineNum++;
        }

        disassemblyWidget_->textCursor().deletePreviousChar();
        pcIndexBounds_ = {pcIndexBounds_.first, upperBoundIndex};
    }

    FocusOnPC(debugInfo);
    pcPage_ = debugInfo.pcMem.page;
    currArmMode_ = armMode;
}

void CpuDebuggerWindow::PopulateRegistersGroup(::debug::CpuDebugInfo const& debugInfo)
{
    for (u8 regIndex = 0; regIndex < 16; ++regIndex)
    {
        registerLabels_[regIndex]->setText(RegString(regIndex, debugInfo.regState.registers[regIndex]));
    }
}

void CpuDebuggerWindow::PopulateCpsrGroup(::debug::CpuDebugInfo const& debugInfo)
{
    cpsrLabel_->setText(QString::fromStdString(std::format("CPSR    0x{:08X}", debugInfo.regState.cpsr)));

    if (debugInfo.regState.spsr.has_value())
    {
        spsrLabel_->setText(QString::fromStdString(std::format("SPSR    0x{:08X}", debugInfo.regState.spsr.value())));
    }
    else
    {
        spsrLabel_->setText("SPSR    N/A");
    }

    negativeBox_->setChecked(debugInfo.regState.negative);
    zeroBox_->setChecked(debugInfo.regState.zero);
    carryBox_->setChecked(debugInfo.regState.carry);
    overflowBox_->setChecked(debugInfo.regState.overflow);

    irqDisableBox_->setChecked(debugInfo.regState.irqDisable);
    fiqDisableBox_->setChecked(debugInfo.regState.fiqDisable);
    thumbStateBox_->setChecked(debugInfo.regState.thumbState);
    modeLabel_->setText(ModeString(debugInfo.regState.mode));
}

void CpuDebuggerWindow::PopulateStackGroup(::debug::CpuDebugInfo const& debugInfo)
{
    if ((debugInfo.spMem.page == Page::INVALID) || debugInfo.spMem.memoryBlock.empty())
    {
        return;
    }

    stackWidget_->clear();
    u32 const spIndex = debugInfo.spMem.AddrToIndex(debugInfo.regState.registers[13] & ~0x03);
    u32 lineNum = 0;
    u32 offset = 0x20;
    u32 lowerBoundIndex = offset > spIndex ? 0 : spIndex - offset;
    u32 upperBoundIndex = std::min(spIndex + offset + 4,
                                   static_cast<u32>(debugInfo.spMem.memoryBlock.size()));

    for (u32 i = lowerBoundIndex; i < upperBoundIndex; i += 4)
    {
        if (i == spIndex)
        {
            spLineNum_ = lineNum;
        }

        u32 addr = i + debugInfo.spMem.minAddr;
        u32 val = MemCpyInit<u32>(&debugInfo.spMem.memoryBlock[i]);
        auto charArr = std::bit_cast<std::array<char, 4>>(val);
        std::replace_if(charArr.begin(), charArr.end(), [] (char c) { return c < 32; }, '.');
        std::string asciiStr(charArr.data(), charArr.size());
        stackWidget_->insertPlainText(QString::fromStdString(std::format("    0x{:08X}    {:08X}    {}\n", addr, val, asciiStr)));
        ++lineNum;
    }

    stackWidget_->textCursor().deletePreviousChar();
    spLowerIndexBound_ = lowerBoundIndex;
    FocusOnSP(debugInfo);
}

void CpuDebuggerWindow::FocusOnPC(::debug::CpuDebugInfo const& debugInfo)
{
    if ((debugInfo.pcMem.page == Page::INVALID) || (debugInfo.pcMem.memoryBlock.empty()))
    {
        return;
    }

    // First delete arrow from previously focused line
    auto it = addrToLineMap_.find(currPC_);

    if (it != addrToLineMap_.end())
    {
        auto cursor = disassemblyWidget_->textCursor();
        cursor.setPosition(disassemblyWidget_->document()->findBlockByLineNumber(it->second).position());
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 3);
        cursor.insertText("   ");
    }

    // Now find the line of the next instruction to be highlighted and focus on it
    currPC_ = debugInfo.nextAddrToExecute;
    it = addrToLineMap_.find(currPC_);

    if (it != addrToLineMap_.end())
    {
        auto cursor = disassemblyWidget_->textCursor();
        cursor.setPosition(disassemblyWidget_->document()->findBlockByLineNumber(it->second).position());
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        disassemblyWidget_->setTextCursor(cursor);

        bool const armMode = !debugInfo.regState.thumbState;
        u32 const pcIndex = debugInfo.pcMem.AddrToIndex(currPC_);
        u32 const addr = pcIndex + debugInfo.pcMem.minAddr;
        auto const& breakpoints = gba_api::GetBreakpoints();
        u32 instruction = armMode ? MemCpyInit<u32>(&debugInfo.pcMem.memoryBlock[pcIndex]) :
                                    MemCpyInit<u16>(&debugInfo.pcMem.memoryBlock[pcIndex]);
        auto const& mnemonic = armMode ? gba_api::DisassembleArmInstruction(instruction) :
                                         gba_api::DisassembleThumbInstruction(instruction);
        WriteDisassembledLine(disassemblyWidget_, mnemonic, addr, instruction, true, armMode, false, breakpoints);
    }
}

void CpuDebuggerWindow::FocusOnSP(::debug::CpuDebugInfo const& debugInfo)
{
    if ((debugInfo.spMem.page == Page::INVALID) || (debugInfo.spMem.memoryBlock.empty()))
    {
        return;
    }

    if (spLineNum_ != U32_MAX)
    {
        auto cursor = stackWidget_->textCursor();
        cursor.setPosition(stackWidget_->document()->findBlockByLineNumber(spLineNum_).position());
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 3);
        cursor.insertText(" ->");
        stackWidget_->setTextCursor(cursor);
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Widget Creation
///---------------------------------------------------------------------------------------------------------------------------------

QGroupBox* CpuDebuggerWindow::CreateDisassemblyGroup()
{
    QGroupBox* groupBox = new QGroupBox;
    QGridLayout* groupLayout = new QGridLayout;

    // Instructions view
    disassemblyWidget_ = new QTextEdit;
    disassemblyWidget_->setReadOnly(true);
    disassemblyWidget_->setLineWrapMode(QTextEdit::NoWrap);
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    disassemblyWidget_->setFont(font);
    groupLayout->addWidget(disassemblyWidget_, 0, 0, 1, 5);

    // Step buttons
    QPushButton* stepButton = new QPushButton("Step");
    stepButton->setFixedWidth(120);
    connect(stepButton, &QPushButton::clicked, this, &CpuDebuggerWindow::StepCpu);
    groupLayout->addWidget(stepButton, 1, 0);

    QPushButton* frameButton = new QPushButton("Step Frame");
    frameButton->setFixedWidth(120);
    connect(frameButton, &QPushButton::clicked, this, &CpuDebuggerWindow::StepFrame);
    groupLayout->addWidget(frameButton, 1, 1);

    QPushButton* runButton = new QPushButton("Run");
    runButton->setFixedWidth(120);
    connect(runButton, &QPushButton::clicked, this, &CpuDebuggerWindow::Run);
    groupLayout->addWidget(runButton, 1, 2);

    // Breakpoint buttons
    breakpointLineEdit_ = new QLineEdit;
    breakpointLineEdit_->setFixedWidth(120);
    groupLayout->addWidget(breakpointLineEdit_, 2, 0);

    QPushButton* addBreakpointButton = new QPushButton("Add Breakpoint");
    addBreakpointButton->setFixedWidth(120);
    connect(addBreakpointButton, &QPushButton::clicked, this, &CpuDebuggerWindow::AddBreakpointAction);
    groupLayout->addWidget(addBreakpointButton, 2, 1);

    QPushButton* removeBreakpointButton = new QPushButton("Remove Breakpoint");
    removeBreakpointButton->setFixedWidth(120);
    connect(removeBreakpointButton, &QPushButton::clicked, this, &CpuDebuggerWindow::RemoveBreakpointAction);
    groupLayout->addWidget(removeBreakpointButton, 2, 2);

    QPushButton* listBreakpoints = new QPushButton("List Breakpoints");
    listBreakpoints->setFixedWidth(120);
    connect(listBreakpoints, &QPushButton::clicked, this, &CpuDebuggerWindow::ListBreakpoints);
    groupLayout->addWidget(listBreakpoints, 2, 3);

    // Set layout
    QLabel* expandingLabel = new QLabel;
    expandingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    groupLayout->addWidget(expandingLabel, 2, 4);

    groupBox->setLayout(groupLayout);
    return groupBox;
}

QGroupBox* CpuDebuggerWindow::CreateRegistersGroup()
{
    QGroupBox* groupBox = new QGroupBox("General Purpose Registers");
    QGridLayout* groupLayout = new QGridLayout;

    for (u8 row = 0; row < 4; ++row)
    {
        for (u8 col = 0; col < 4; ++col)
        {
            u8 regIndex = (row * 4) + col;
            registerLabels_[regIndex] = new QLabel(RegString(regIndex, 0));
            groupLayout->addWidget(registerLabels_[regIndex], row, col);
        }
    }

    groupBox->setLayout(groupLayout);
    return groupBox;
}

QGroupBox* CpuDebuggerWindow::CreateCpsrGroup()
{
    QGroupBox* groupBox = new QGroupBox("Program Status Registers");
    QGridLayout* groupLayout = new QGridLayout;

    cpsrLabel_ = new QLabel("CPSR    0x00000000");
    groupLayout->addWidget(cpsrLabel_, 0, 0);

    spsrLabel_ = new QLabel("SPSR    0x00000000");
    groupLayout->addWidget(spsrLabel_, 1, 0);

    groupLayout->addWidget(new QLabel(""), 0, 1);
    groupLayout->addWidget(new QLabel(""), 1, 1);

    // Condition code flags

    negativeBox_ = CreateCpsrCheckBox("N");
    groupLayout->addWidget(negativeBox_, 0, 2);

    zeroBox_ = CreateCpsrCheckBox("Z");
    groupLayout->addWidget(zeroBox_, 0, 3);

    carryBox_ = CreateCpsrCheckBox("C");
    groupLayout->addWidget(carryBox_, 0, 4);

    overflowBox_ = CreateCpsrCheckBox("V");
    groupLayout->addWidget(overflowBox_, 0, 5);

    // Control bits

    irqDisableBox_ = CreateCpsrCheckBox("I");
    groupLayout->addWidget(irqDisableBox_, 1, 2);

    fiqDisableBox_ = CreateCpsrCheckBox("F");
    groupLayout->addWidget(fiqDisableBox_, 1, 3);

    thumbStateBox_ = CreateCpsrCheckBox("T");
    groupLayout->addWidget(thumbStateBox_, 1, 4);

    modeLabel_ = new QLabel(ModeString(0b10011));
    groupLayout->addWidget(modeLabel_, 1, 5);

    groupBox->setLayout(groupLayout);
    return groupBox;
}

QGroupBox* CpuDebuggerWindow::CreateStackGroup()
{
    QGroupBox* groupBox = new QGroupBox("Stack");
    QGridLayout* groupLayout = new QGridLayout;

    stackWidget_ = new QTextEdit;
    stackWidget_->setReadOnly(true);
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    stackWidget_->setFont(font);
    groupLayout->addWidget(stackWidget_, 0, 0);

    groupBox->setLayout(groupLayout);
    return groupBox;
}

void CpuDebuggerWindow::StepCpu()
{
    emit PauseSignal();
    gba_api::SingleStep();
    UpdateWidgets();
    emit StepSignal();
}

void CpuDebuggerWindow::StepFrame()
{
    runSingleFrame_ = true;
    emit ResumeSignal();
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Button Actions
///---------------------------------------------------------------------------------------------------------------------------------

void CpuDebuggerWindow::Run()
{
    emit ResumeSignal();
}

void CpuDebuggerWindow::AddBreakpointAction()
{
    bool valid = false;
    u32 breakpoint = breakpointLineEdit_->text().toUInt(&valid, 0);

    if (valid)
    {
        gba_api::SetBreakpoint(breakpoint);
        breakpointLineEdit_->clear();
        auto it = addrToLineMap_.find(breakpoint);

        if (it != addrToLineMap_.end())
        {
            auto cursor = disassemblyWidget_->textCursor();
            cursor.setPosition(disassemblyWidget_->document()->findBlockByLineNumber(it->second).position());
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            cursor.insertText("B");
        }

        emit BreakpointsUpdatedSignal();
    }
}

void CpuDebuggerWindow::RemoveBreakpointAction()
{
    bool valid = false;
    u32 breakpoint = breakpointLineEdit_->text().toUInt(&valid, 0);

    if (valid)
    {
        RemoveBreakpoint(breakpoint);
        breakpointLineEdit_->clear();
        emit BreakpointsUpdatedSignal();
    }
}

void CpuDebuggerWindow::RemoveBreakpoint(u32 breakpoint)
{
    auto it = addrToLineMap_.find(breakpoint);

    if (it != addrToLineMap_.end())
    {
        auto cursor = disassemblyWidget_->textCursor();
        cursor.setPosition(disassemblyWidget_->document()->findBlockByLineNumber(it->second).position());
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        cursor.insertText(" ");
    }

    gba_api::RemoveBreakpoint(breakpoint);
}

void CpuDebuggerWindow::ListBreakpoints()
{
    if (!breakpointsWindow_->isVisible())
    {
        breakpointsWindow_->show();
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Event handling
///---------------------------------------------------------------------------------------------------------------------------------

void CpuDebuggerWindow::closeEvent(QCloseEvent* event)
{
    breakpointsWindow_->close();
    event->accept();
}
}  // namespace gui
