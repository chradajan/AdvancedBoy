#pragma once

#include <array>
#include <unordered_map>
#include <utility>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/Types.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

namespace gui
{
class BreakpointsList : public QWidget
{
    Q_OBJECT

public:
    BreakpointsList(BreakpointsList const&) = delete;
    BreakpointsList& operator=(BreakpointsList const&) = delete;
    BreakpointsList(BreakpointsList&&) = delete;
    BreakpointsList& operator=(BreakpointsList&&) = delete;

    /// @brief Initialize the layout of the BreakpointsList.
    BreakpointsList();

public slots:
    /// @brief Refresh the displayed breakpoints.
    void UpdateBreakpointsSlot() { UpdateList(); }

signals:
    /// @brief Emit to signal that a breakpoint has been removed.
    void BreakpointRemovedSignal(u32 breakpoint);

private:
    /// @brief Action tied to the Remove button.
    void RemoveSelectedBreakpoint();

    /// @brief Refresh the displayed breakpoints.
    void UpdateList();

    // Widgets
    QListWidget* list_;
};

class CpuDebuggerWindow : public QWidget
{
    Q_OBJECT

public:
    CpuDebuggerWindow(CpuDebuggerWindow const&) = delete;
    CpuDebuggerWindow& operator=(CpuDebuggerWindow const&) = delete;
    CpuDebuggerWindow(CpuDebuggerWindow&&) = delete;
    CpuDebuggerWindow& operator=(CpuDebuggerWindow&&) = delete;

    /// @brief Initialize the CPU debugger.
    CpuDebuggerWindow();

    /// @brief CPU Debugger destructor.
    ~CpuDebuggerWindow();

public slots:
    /// @brief Slot for other widgets to signal that the debugger should be updated.
    void UpdateCpuDebuggerSlot() { if (isVisible()) UpdateWidgets(); }

    /// @brief Slot for the breakpoints list to signal that a breakpoint has been removed.
    void BreakpointRemovedSlot(u32 breakpoint) { RemoveBreakpoint(breakpoint); }

signals:
    /// @brief Emit to notify the main window to run the emulator for a step/frame/indefinitely.
    /// @param stepType Duration to run the emulator for.
    void CpuDebugStepSignal(StepType stepType);

    /// @brief Emit to notify the breakpoints list that it should be refreshed.
    void BreakpointsUpdatedSignal();

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Widget Updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Update all the CPU debugger widgets with the latest CPU debug info.
    void UpdateWidgets();

    /// @brief Update the text window displaying disassembled instructions around where the CPU is executing.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateDisassemblyGroup(::debug::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the general purpose registers.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateRegistersGroup(::debug::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the CPSR/SPSR registers.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateCpsrGroup(::debug::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the stack.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateStackGroup(::debug::CpuDebugInfo const& debugInfo);

    /// @brief Update the disassembly widget to add an arrow on the next instruction to be executed.
    /// @param debugInfo CPU debug info needed to get PC info.
    void FocusOnPC(::debug::CpuDebugInfo const& debugInfo);

    /// @brief Update the stack widget to add an arrow on the current SP address.
    /// @param debugInfo CPU debug info needed to get stack info.
    void FocusOnSP(::debug::CpuDebugInfo const& debugInfo);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Widget Creation
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Create the layout for the disassembly group.
    /// @return QGroupBox containing all the disassembly widgets.
    QGroupBox* CreateDisassemblyGroup();

    /// @brief Create the layout for the registers group.
    /// @return QGroupBox containing all the registers widgets.
    QGroupBox* CreateRegistersGroup();

    /// @brief Create the layout for the CPSR group.
    /// @return QGroupBox containing all the CPSR widgets.
    QGroupBox* CreateCpsrGroup();

    /// @brief Create the layout for the stack group.
    /// @return QGroupBox containing all the stack widgets.
    QGroupBox* CreateStackGroup();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Button Actions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Action for the "Run" button. Emits relevant signals to resume emulation.
    void Run();

    /// @brief Action for the "Step" button. Steps the CPU once and emits relevant signals.
    void StepCpu();

    /// @brief Action for the "Step Frame" button. Runs the emulator until the next time it hits VBlank and emits relevant signals.
    void StepFrame();

    /// @brief Action for the "Add Breakpoint" button.
    void AddBreakpointAction();

    /// @brief Action for the "Remove Breakpoint" button.
    void RemoveBreakpointAction();

    /// @brief Removes a breakpoint and updates the disassembly widget.
    /// @param breakpoint Address of breakpoint that was removed.
    void RemoveBreakpoint(u32 breakpoint);

    /// @brief Action for the "List Breakpoints" button. Opens the breakpoints list window.
    void ListBreakpoints();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event Handling
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Event handler for a close event.
    /// @param event Close event.
    void closeEvent(QCloseEvent* event) override;

    // Disassembly
    QTextEdit* disassemblyWidget_;
    QLineEdit* breakpointLineEdit_;
    Page pcPage_;
    u32 currPC_;
    u32 pcLineNum_;
    bool currArmMode_;
    std::pair<u32, u32> pcIndexBounds_;
    std::unordered_map<u32, u32> addrToLineMap_;

    // Registers
    std::array<QLabel*, 16> registerLabels_;
    QLabel* cpsrLabel_;
    QLabel* spsrLabel_;

    // Flags/Mode
    QCheckBox* negativeBox_;
    QCheckBox* zeroBox_;
    QCheckBox* carryBox_;
    QCheckBox* overflowBox_;

    QCheckBox* irqDisableBox_;
    QCheckBox* fiqDisableBox_;
    QCheckBox* thumbStateBox_;
    QLabel* modeLabel_;

    // Stack
    QTextEdit* stackWidget_;
    u32 spLineNum_;
    u32 spLowerIndexBound_;

    // Breakpoints window
    BreakpointsList* breakpointsWindow_;
};
}  // namespace gui
