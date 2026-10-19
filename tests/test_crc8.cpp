#include <gtest/gtest.h>

#include <cstdint>

#include "fw/crc8.hpp"

TEST(Crc8SaeJ1850, StandardCheckValue) {
    const std::uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(fw::crc8_sae_j1850(data, sizeof(data)), 0x4B);
}

TEST(Crc8SaeJ1850, EmptyInput) {
    // init 0xFF, final XOR 0xFF -> 0x00 for no data
    EXPECT_EQ(fw::crc8_sae_j1850(nullptr, 0U), 0x00);
}

TEST(Crc8SaeJ1850, AutosarE2EReferenceVectors) {
    // Reference values from the AUTOSAR CRC library specification.
    const std::uint8_t zeros[] = {0x00, 0x00, 0x00, 0x00};
    const std::uint8_t mixed[] = {0xF2, 0x01, 0x83};
    const std::uint8_t ff[] = {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(fw::crc8_sae_j1850(zeros, sizeof(zeros)), 0x59);
    EXPECT_EQ(fw::crc8_sae_j1850(mixed, sizeof(mixed)), 0x37);
    EXPECT_EQ(fw::crc8_sae_j1850(ff, sizeof(ff)), 0x74);
}

TEST(Crc8SaeJ1850, DetectsEverySingleBitFlip) {
    std::uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE};
    const std::uint8_t good = fw::crc8_sae_j1850(data, sizeof(data));
    for (std::size_t byte = 0; byte < sizeof(data); ++byte) {
        for (int bit = 0; bit < 8; ++bit) {
            data[byte] ^= static_cast<std::uint8_t>(1U << bit);
            EXPECT_NE(fw::crc8_sae_j1850(data, sizeof(data)), good) << "byte " << byte << " bit " << bit;
            data[byte] ^= static_cast<std::uint8_t>(1U << bit);
        }
    }
}
