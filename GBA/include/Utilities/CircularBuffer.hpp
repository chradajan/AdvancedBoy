#pragma once

#include <array>
#include <fstream>
#include <GBA/include/Utilities/Types.hpp>

template <typename T, size_t len>
class CircularBuffer
{
public:
    /// @brief Create an empty FIFO circular buffer.
    CircularBuffer() noexcept;

    /// @brief Check if the circular buffer is full. Full buffers will throw upon calling Push.
    /// @return True if buffer is full.
    bool Full() const noexcept;

    /// @brief Check the current size of the buffer.
    /// @return Number of elements currently in buffer.
    size_t Size() const noexcept;

    /// @brief Check if the circular buffer is empty. Empty buffers will throw upon calling Pop.
    /// @return True if buffer is empty.
    bool Empty() const noexcept;

    /// @brief Insert an element into the buffer. Illegal when buffer is full.
    /// @param val Value to insert.
    void Push(T val);

    /// @brief Remove an element from the buffer. Illegal when buffer is empty.
    /// @return Oldest element in buffer.
    T Pop();

    /// @brief Check the value that would next be popped from the buffer. Illegal when empty.
    /// @return Const reference to next element to be popped.
    T const& PeakTail() const;

    /// @brief Check the value that was most recently pushed to the buffer. Illegal when empty.
    /// @return Const reference to most recently pushed element.
    T const& PeakHead() const;

    /// @brief Reset the buffer to an empty state.
    void Clear() noexcept;

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    std::array<T, len> buffer_;
    size_t head_;
    size_t tail_;
    size_t count_;
};

#include <GBA/include/Utilities/CircularBuffer.tpp>
