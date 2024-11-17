#pragma once

#include <cstddef>
#include <cstring>
#include <span>
#include <GBA/include/Utilities/Types.hpp>

/// @brief Read a byte, halfword, or word from an arbitrary block of memory.
/// @param memory Span of bytes to read from.
/// @param readAddr Address being read. This should be aligned and in GBA address space.
/// @param baseAddr Base address of block of memory being read from.
/// @param length Memory access size of the read.
/// @return Value at the specified location in the provided span.
u32 ReadMemoryBlock(std::span<const std::byte> memory, u32 readAddr, u32 baseAddr, AccessSize length);

/// @brief Write a byte, halfword, or word to an arbitrary block of memory.
/// @param memory Span of bytes to write into.
/// @param writeAddr Address to write to. This should be aligned and in GBA address space.
/// @param baseAddr Base address of block of memory to write to.
/// @param val Value to write into memory.
/// @param length Memory access size of the write.
void WriteMemoryBlock(std::span<std::byte> memory, u32 writeAddr, u32 baseAddr, u32 val, AccessSize length);

/// @brief Find the mirrored address for an address in a mirrored region.
/// @param addr Out-of-bounds address in a region that implements standard memory mirroring. Must be > max.
/// @param min Base address of the memory region.
/// @param max Inclusive max non-mirrored address of the memory region.
/// @return Mirrored address within the bounds [min, max].
u32 StandardMirroredAddress(u32 addr, u32 min, u32 max);

/// @brief Sign extend a two's complement number with an arbitrary sign bit.
/// @tparam T Signed integer type to extend input to.
/// @tparam index Bit index of current sign bit (0 == LSB).
/// @param input Two's complement value to be sign extended.
/// @return Sign extended value.
template <typename T, u8 index>
T SignExtend(T input);

/// @brief Initialize a variable using std::memcpy and return it.
/// @tparam T Type to alias memory as.
/// @param src Address of memory to alias.
/// @return Aliased instance of some memory location.
template <typename T>
inline T MemCpyInit(const void* src) { T val; std::memcpy(&val, src, sizeof(T)); return val; }

/// @brief Rotate a byte read from an 8-bit bus without doing a byte sized access.
/// @param byte Value at address being read.
/// @param length Memory access size of the read.
/// @return Rotated value.
u32 Read8BitBus(u8 byte, AccessSize length);

/// @brief Rotate a value written to an address on an 8-bit bus.
/// @param addr Address being written.
/// @param val Value to write to address.
/// @return Rotated value to actually write.
u8 Write8BitBus(u32 addr, u32 val);

/// @brief Memset an object with 0.
/// @tparam T Object type (should be inferred).
/// @param obj Reference to object to memset.
template <typename T>
void ZeroObject(T& obj);

// Save state macros

#define SerializeArray(arr) saveState.write(reinterpret_cast<const char*>(arr.data()), sizeof(arr[0]) * arr.size())
#define DeserializeArray(arr) saveState.read(reinterpret_cast<char*>(arr.data()), sizeof(arr[0]) * arr.size())

#define SerializeTrivialType(val) saveState.write(reinterpret_cast<const char*>(&val), sizeof(val))
#define DeserializeTrivialType(val) saveState.read(reinterpret_cast<char*>(&val), sizeof(val))

#include <GBA/include/Utilities/CommonUtils.tpp>
