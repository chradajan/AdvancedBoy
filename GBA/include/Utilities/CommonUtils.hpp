#pragma once

#include <cstddef>
#include <span>
#include <GBA/include/Types.hpp>

/// @brief Read a byte, halfword, or word from an arbitrary block of memory.
/// @param memory Span of bytes to read from.
/// @param readAddr Address being read. This should be aligned and in GBA address space.
/// @param baseAddr Base address of block of memory being read from.
/// @param length Memory access size of the read.
/// @return Value at the specified location in the provided span.
u32 ReadMemoryBlock(std::span<const std::byte> memory, Address readAddr, Address baseAddr, AccessSize length);

/// @brief Write a byte, halfword, or word to an arbitrary block of memory.
/// @param memory Span of bytes to write into.
/// @param writeAddr Address to write to. This should be aligned and in GBA address space.
/// @param baseAddr Base address of block of memory to write to.
/// @param val Value to write into memory.
/// @param length Memory access size of the write.
void WriteMemoryBlock(std::span<std::byte> memory, Address writeAddr, Address baseAddr, u32 val, AccessSize length);
