#pragma once

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>
#include <GBA/include/Utilities/Types.hpp>

/// @brief Enum of various event types that can be scheduled to execute. Must be registered before scheduling.
enum class EventType
{
    // APU
    SampleAPU,

    // IRQs
    SetIRQ,

    // APU
    Channel1Clock,
    Channel1Envelope,
    Channel1LengthTimer,
    Channel1FrequencySweep,

    Channel2Clock,
    Channel2Envelope,
    Channel2LengthTimer,

    Channel4Clock,
    Channel4Envelope,
    Channel4LengthTimer,

    // Timers
    Timer0Overflow,
    Timer1Overflow,
    Timer2Overflow,
    Timer3Overflow,

    // DMA
    DmaComplete,

    // PPU
    VDraw,
    HBlank,
    VBlank,

    // Total number of events. Do not schedule this, and do not place events below this.
    COUNT
};

/// @brief Information about an event that is schedule to be executed.
struct Event
{
    /// @brief Check which event executes sooner. If A > B, then A should execute after B. If two events execute on the same cycle,
    ///        determine priority by their order in the EventType enum (lower = higher priority).
    /// @param rhs Event to compare priority to.
    /// @return True if this is lower priority than the event this is being compared to.
    bool operator>(Event const& rhs);

    EventType eventType_;
    u64 cycleQueued_;
    u64 cycleToExecute_;
};

/// @brief Scheduler for scheduling and dispatching system events.
class EventScheduler
{
    using Callback = std::function<void(int)>;

public:
    EventScheduler(EventScheduler const&) = delete;
    EventScheduler& operator=(EventScheduler const&) = delete;
    EventScheduler(EventScheduler&&) = delete;
    EventScheduler& operator=(EventScheduler&&) = delete;

    /// @brief Initialize the event scheduler by preallocating space in the event queue.
    EventScheduler();

    /// @brief Register a callback function for an event. All callbacks should be registered during component initialization.
    /// @param event Event to install callback for.
    /// @param callback Function to call when event fires.
    void RegisterEvent(EventType event, Callback callback);

    /// @brief Schedule an event to fire in some number of cycles.
    /// @param event Event type to schedule.
    /// @param cycles Number of cycles from now to fire event.
    void ScheduleEvent(EventType event, int cycles);

    /// @brief Schedule an event with an offset cycleQueued_ value.
    /// @param event Event type to schedule.
    /// @param offset How many cycles from the current value of totalCycles_ to schedule the event.
    /// @param length Number of cycles from the relative start time that the event should fire.
    void ScheduleEvent(EventType event, int offset, u32 length);

    /// @brief Advance the scheduler by some number of cycles and execute any scheduled events that have occurred.
    /// @param cycles Number of cycles to advance the scheduler by.
    void Step(int cycles);

    /// @brief Advance the scheduler to whenever the next scheduled event would occur.
    void FireNextEvent();

    /// @brief Get the total number of CPU cycles that have elapsed since system startup.
    /// @return Total cycle count.
    u64 GetTotalElapsedCycles() const { return totalCycles_; }

    /// @brief Remove an event from the current event queue.
    /// @param event Event type to be unscheduled.
    /// @return If the event was in the queue, return how many cycles it had left until it would have been fired.
    std::optional<int> UnscheduleEvent(EventType event);

    /// @brief Get the number of cycles that have passed since an event was scheduled.
    /// @param event Event type to check.
    /// @return If the event is currently in the queue, return the number of cycles since it was queued.
    std::optional<int> ElapsedCycles(EventType event);

private:
    /// @brief Execute any scheduled events which were scheduled to execute at or before the current total cycle count.
    void CheckEventQueue();

    std::vector<Event> queue_;
    std::unordered_map<EventType, Callback> callbacks_;
    u64 totalCycles_;
};
