#pragma once

#include <array>
#include <unordered_map>
#include <utility>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
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

class CpuDebugger : public QWidget
{
    Q_OBJECT

public:
    CpuDebugger(CpuDebugger const&) = delete;
    CpuDebugger& operator=(CpuDebugger const&) = delete;
    CpuDebugger(CpuDebugger&&) = delete;
    CpuDebugger& operator=(CpuDebugger&&) = delete;

    /// @brief Initialize the CPU debugger.
    CpuDebugger();

    /// @brief CPU Debugger destructor.
    ~CpuDebugger();

    /// @brief After the emulator has entered VBLank, emulation should pause if this is true.
    /// @return Whether the step frame option was selected.
    bool StepFrameMode() const { return runSingleFrame_; }

    /// @brief Disable step frame mode when the next frame is ready.
    void DisableStepFrameMode() { runSingleFrame_ = false; }

public slots:
    /// @brief Slot for other widgets to signal that the debugger should be updated.
    void UpdateCpuDebuggerSlot() { UpdateWidgets(); }

    /// @brief Slot for the breakpoints list to signal that a breakpoint has been removed.
    void BreakpointRemovedSlot(u32 breakpoint) { RemoveBreakpoint(breakpoint); }

signals:
    /// @brief Emit to pause emulation if it is currently running.
    void PauseSignal();

    /// @brief Emit to resume emulation if it is currently paused.
    void ResumeSignal();

    /// @brief Emit to notify any connected widgets that the CPU has been stepped.
    void StepSignal();

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
    void PopulateDisassemblyGroup(::debug::cpu::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the general purpose registers.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateRegistersGroup(::debug::cpu::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the CPSR/SPSR registers.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateCpsrGroup(::debug::cpu::CpuDebugInfo const& debugInfo);

    /// @brief Update the displayed values for the stack.
    /// @param debugInfo CPU debug info needed to populate the widget.
    void PopulateStackGroup(::debug::cpu::CpuDebugInfo const& debugInfo);

    /// @brief Update the disassembly widget to add an arrow on the next instruction to be executed.
    /// @param debugInfo CPU debug info needed to get PC info.
    void FocusOnPC(::debug::cpu::CpuDebugInfo const& debugInfo);

    /// @brief Update the stack widget to add an arrow on the current SP address.
    /// @param debugInfo CPU debug info needed to get stack info.
    void FocusOnSP(::debug::cpu::CpuDebugInfo const& debugInfo);

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

    /// @brief Action for the "Step" button. Steps the CPU once and emits relevant signals.
    void StepCpu();

    /// @brief Action for the "Step Frame" button. Runs the emulator until the next time it hits VBlank and emits relevant signals.
    void StepFrame();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Button Actions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Action for the "Run" button. Emits relevant signals to resume emulation.
    void Run();

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

    // Step mode
    bool runSingleFrame_;

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
