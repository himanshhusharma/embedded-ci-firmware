# Embedded C++ Firmware with CI

A small, hardware-independent automotive firmware module (vehicle-speed monitoring) and the GitHub Actions pipeline that guards it. Every push builds the firmware, runs GoogleTest unit tests and cppcheck static analysis, cross-compiles for an ARM Cortex-M4, and publishes the results as build artifacts. Any failing test, compiler warning or static-analysis finding fails the build.

## Pipeline

| Job | What it does | Fails the build when | Artifact |
|---|---|---|---|
| Build & unit tests | CMake build with `-Werror`, 33 GoogleTest tests, host simulation | a warning or a test fails | `test-results` (JUnit XML, simulation log) |
| Static analysis | cppcheck (warning, style, performance, portability) | cppcheck reports any finding (`--error-exitcode=1`) | `cppcheck-report` (XML) |
| Cross-compile | `arm-none-eabi-g++` for Cortex-M4F with `-Os -fno-exceptions -fno-rtti` | the library doesn't build for target | `size-report` (flash/RAM per object) |

## The firmware

The pipeline mirrors a typical speed-warning feature: a sender ECU puts vehicle speed on CAN, the receiver checks frame integrity, filters the signal and drives an overspeed warning.

| Module | Purpose |
|---|---|
| `crc8` | CRC-8 SAE J1850, the CRC used by AUTOSAR E2E profiles; tested against AUTOSAR reference vectors |
| `speed_frame` | 8-byte CAN frame with alive counter and CRC over a data ID; the receiver detects corrupted, repeated and lost frames, E2E-style |
| `moving_average` | O(1) integer moving-average filter, overflow-safe |
| `overspeed_monitor` | Warning with hysteresis and debouncing so it doesn't flicker near the limit; holds state on invalid input |
| `ring_buffer` | Fixed-capacity FIFO that refuses writes when full instead of overwriting |

Embedded constraints: no heap allocation, no exceptions, no RTTI, fixed-width integer types, and strict warnings (`-Wconversion -Wsign-conversion -Wshadow -Werror`). On Cortex-M4 the whole library is **592 bytes of flash and 0 bytes of static RAM**.

## Build locally

Requirements: CMake 3.20+, a C++17 compiler. GoogleTest is downloaded automatically.

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/speed_sim
```

Static analysis:

```bash
cppcheck --enable=warning,style,performance,portability --std=c++17 \
         --suppress=missingIncludeSystem --error-exitcode=1 -I include src app include
```

Cross-compile (needs `gcc-arm-none-eabi`):

```bash
cmake -S . -B build-arm -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm
arm-none-eabi-size -t build-arm/libfw_core.a
```

## Simulation output

`speed_sim` runs a speed profile through the full chain and injects two bus faults: a bit flip in frame 4 and three lost frames before frame 11.

```
frame rx status       raw      filtered  warning
3     OK                121.0    116.3   off
4     CRC_ERROR       -        -         off (held)
5     OK                125.0    121.3   off
6     OK                124.0    123.3   ON
7     OK                120.5    123.2   ON
...
11    WRONG_SEQUENCE  -        -         ON (held)
12    OK                105.0    115.0   ON
13    OK                102.0    108.7   off
```

The corrupted frame is rejected, the warning only turns on after two samples above 120 km/h, it stays on while speed hovers near the limit, and it turns off once speed drops below 115 km/h (5 km/h hysteresis).

## Layout

```
include/fw/      public headers
src/             firmware sources (built for host and target)
app/main.cpp     host simulation
tests/           GoogleTest suites
cmake/           ARM toolchain file
.github/workflows/ci.yml
```
"# embedded-ci-firmware" 
