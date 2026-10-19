#include "fw/crc8.hpp"

namespace fw {

std::uint8_t crc8_sae_j1850(const std::uint8_t* data, std::size_t len) {
    constexpr std::uint8_t kPoly = 0x1DU;
    std::uint8_t crc = 0xFFU;
    for (std::size_t i = 0U; i < len; ++i) {
        crc = static_cast<std::uint8_t>(crc ^ data[i]);
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x80U) != 0U) {
                crc = static_cast<std::uint8_t>((crc << 1U) ^ kPoly);
            } else {
                crc = static_cast<std::uint8_t>(crc << 1U);
            }
        }
    }
    return static_cast<std::uint8_t>(crc ^ 0xFFU);
}

}  // namespace fw
