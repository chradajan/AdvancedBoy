#include <GBA/include/System/EventScheduler.hpp>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <GBA/include/Types/Types.hpp>

bool Event::operator>(Event const& rhs)
{
    if (cycleToExecute_ == rhs.cycleToExecute_)
    {
        return eventType_ > rhs.eventType_;
    }

    return cycleToExecute_ > rhs.cycleToExecute_;
}

EventScheduler::EventScheduler()
{
    queue_.reserve(static_cast<size_t>(EventType::COUNT) + 1);
    totalCycles_ = 0;
}

void EventScheduler::RegisterEvent(EventType event, Callback callback)
{
    if (callbacks_.contains(event))
    {
        throw std::logic_error("Duplicate event registration");
    }

    callbacks_.insert({event, callback});
}

void EventScheduler::ScheduleEvent(EventType event, int cycles)
{
    if (cycles < 0)
    {
        throw std::runtime_error("Illegal event scheduler timing");
    }

    u64 cycleToExecute = totalCycles_ + cycles;
    queue_.push_back({event, totalCycles_, cycleToExecute});
    std::make_heap(queue_.begin(), queue_.end(), std::greater<>{});
}

void EventScheduler::ScheduleEvent(EventType event, int offset, u32 length)
{
    u64 cycleQueued = totalCycles_ + offset;
    u64 cycleToExecute = cycleQueued + length;
    queue_.push_back({event, cycleQueued, cycleToExecute});
    std::make_heap(queue_.begin(), queue_.end(), std::greater<>{});
}

void EventScheduler::Step(int cycles)
{
    if (cycles <= 0)
    {
        throw std::runtime_error("Illegal step count");
    }

    totalCycles_ += static_cast<u64>(cycles);
    CheckEventQueue();
}

void EventScheduler::FireNextEvent()
{
    totalCycles_ = queue_.front().cycleToExecute_;
    CheckEventQueue();
}

void EventScheduler::CheckEventQueue()
{
    Event nextEvent = queue_.front();

    while (totalCycles_ >= nextEvent.cycleToExecute_)
    {
        std::pop_heap(queue_.begin(), queue_.end(), std::greater<>{});
        queue_.pop_back();
        auto& callback = callbacks_[nextEvent.eventType_];
        callback(totalCycles_ - nextEvent.cycleToExecute_);
        nextEvent = queue_.front();
    }
}

std::optional<int> EventScheduler::UnscheduleEvent(EventType event)
{
    std::optional<int> remainingCycles = {};

    for (auto it = queue_.begin(); it != queue_.end(); ++it)
    {
        if (it->eventType_ == event)
        {
            remainingCycles = it->cycleToExecute_ - totalCycles_;
            queue_.erase(it);
            std::make_heap(queue_.begin(), queue_.end(), std::greater<>{});
            break;
        }
    }

    return remainingCycles;
}

std::optional<int> EventScheduler::ElapsedCycles(EventType event)
{
    for (Event const& scheduledEvent : queue_)
    {
        if (event == scheduledEvent.eventType_)
        {
            return totalCycles_ - scheduledEvent.cycleQueued_;
        }
    }

    return {};
}
