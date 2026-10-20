# Steering Wheel - Firmware

Steering wheel firmware of the E-Agle TRT car <em>Fenice Evo</em>, running the **Kraken** steering-wheel codebase ported to this single-core platform.

It works on a **STM32H723ZG** microcontroller placed on a custom PCB.

## Overview

The firmware is responsible for reading data from the **CAN bus** and displaying it on an **800x480 LCD**.
It also handles **buttons** and **3 rotary switches** for user input in real-time, allowing the driver to interact with the system while driving.

## Codebase structure

- `Core/Src/steering/`, `Core/Inc/steering/`: hardware-agnostic application modules (scheduler, FSM, inputs, parameters, CAN communications, dashboard/popup/screen UI).
- `Core/Src/steering/drivers/`, `Core/Inc/steering/drivers/`: hardware-agnostic device drivers (MCP23017 GPIO expander, Micron SDRAM).
- `Core/Src/steering/can-communications-router-api.c`: the only place that knows about **libcan**. It decodes the inbound frames the dashboard renders into the UI snapshot and builds the wheel's own outbound messages (button status carrying TS-on/PTT, control maps, driver action). Identity broadcasts (FSM status, firmware and libcan versions) live in `identity-api.c`.
- Everything else in `Core/` is CubeMX-generated peripheral code; the hardware glue that wires peripherals to the application modules lives in its user-code sections.
- `fenice-evo-steering-wheel-bootloader-sw/`: the OpenBLT CAN bootloader (own CubeMX project, built from the same `platformio.ini`). It occupies the first 128K flash sector; the firmware linked after it uses `STM32H723XG_FLASH_SHIFTED.ld`.
- `test/`: host unit tests (Unity + fff) for the hardware-agnostic modules.
- `simulator/`: host hooks (window, keyboard, CAN, logger) that run the hardware-agnostic firmware on a PC.
- `assets/fsm/fsm.dot`: source of truth for the FSM. Regenerate `fsm.c`/`fsm.h` with `./scripts/generate_fsm.sh assets/fsm/fsm.dot` (requires the `gv_fsm` gem), then re-apply the user-code sections.

## Development

The codebase is based on **PlatformIO**. Install it with `pip install platformio`.

| Environment | What it builds |
|---|---|
| `release` (default) | firmware linked at `0x08000000`, no bootloader (`STM32H723XG_FLASH.ld`) |
| `release-bootloader` | same firmware linked after the bootloader at `0x08020000` (`STM32H723XG_FLASH_SHIFTED.ld`); also writes the OpenBLT checksum and generates `firmware.srec` |
| `bootloader` | the OpenBLT bootloader in `fenice-evo-steering-wheel-bootloader-sw/` |
| `tests` | host unit tests |
| `simulator` | the firmware running on your PC (see below) |

- Build: `pio run` (or `pio run -e <environment>`)
- Flash through SWD (ST-Link, `STM32_Programmer_CLI`): `pio run -e <environment> -t upload`
- Flash over CAN through the bootloader: build `release-bootloader` and flash
  `.pio/build/release-bootloader/firmware.srec` with BootCommander/MicroBoot
- Unit tests: `pio test -e tests`
- Static analysis: `pio check`
- Documentation: `doxygen Doxyfile` (HTML in `docs/html/`)

### Simulator

`pio run -e simulator -t exec` opens a window running the real firmware: same scheduler, task table,
FSM, CAN router and screen as the wheel. Only the hardware is replaced: the dashboard is drawn in the
window, the keyboard is the wheel's inputs, frames the firmware sends are printed (only when their
content changes) and CAN frames can be injected:

| Key | Action |
|---|---|
| `q`/`w`, `a`/`s`, `z`/`x` | manettini (power, regen, torque vectoring) −/+ |
| `e`, `d` | telemetry log, launch control buttons |
| `c`, `v` (hold) | PTT paddles |
| `t` (hold) | TS-on |
| `r`, `f`, `b`, `n` | other buttons and bottom paddles |
| `1` | inject `EcuFsm` with the next vehicle status |
| `2` | inject one step of telemetry (SoC, temperatures, laps) |
| `p` | toggle a continuous telemetry feed |

`--screenshot <file.ppm>` runs for one second, saves the window and exits
(e.g. `.pio/build/simulator/program --screenshot frame.ppm`).
Needs OpenGL (Cocoa on macOS, GL + X11 on Linux).

### Git hooks

`hooks/` contains a `pre-commit` hook (clang-format on the staged files in `Core/`) and a
`commit-msg` hook (`<type>: <description>`, type one of feat, fix, docs, style, refactor, perf,
test, chore). Enable them once per clone:

```bash
git config core.hooksPath hooks
```

## Mantainers

- [Bridi Alessandro](https://github.com/bridiro)

## Licence

This repository is released under GNU AFFERO GENERAL PUBLIC LICENSE Version 3. See [LICENSE](./LICENSE) for more details.
