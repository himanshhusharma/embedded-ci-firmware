#pragma once

#include <cstddef>
#include <cstdint>

namespace fw {

// CRC-8 SAE J1850: poly 0x1D, init 0xFF, final XOR 0xFF.
// This is the CRC used by AUTOSAR E2E Profile 1/2 and many CAN signals.
// Check value for "123456789" is 0x4B.
std::uint8_t crc8_sae_j1850(const std::uint8_t* data, std::size_t len);

}  // namespace fw
