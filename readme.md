# Yu's NuPhy Air75 V2 firmware

This repository contains a personal QMK firmware and source-controlled keymap
for the **NuPhy Air75 V2 ANSI**. The default branch is intended to build the
`yu` keymap; it is not a general-purpose replacement for upstream QMK.

The firmware is based directly on
[jincao1's `air75v2-sleep` branch](https://github.com/jincao1/qmk_firmware/tree/air75v2-sleep),
starting from commit
[`5818bd6`](https://github.com/jincao1/qmk_firmware/commit/5818bd6df659b3919f834f58fe4134aa11aca2b6).
jincao1's work provides the Air75 V2 firmware base, wireless/RF handling, and
sleep-related changes on which this keymap is built.

The [NuPhy QMK source](https://github.com/nuphy-src/qmk_firmware) and
[upstream QMK](https://github.com/qmk/qmk_firmware) remain important references.
See [QMK's official documentation](https://docs.qmk.fm/) for general setup,
building, flashing, and development information.

## What this fork adds

- A compiled, source-controlled eight-layer keymap, so the intended layout is
  restored by flashing without importing a VIA JSON file.
- A Caps-position state machine: tap for Escape, chord for immediate Command
  or Control, or deliberately hold for 500 ms to use that modifier with a
  pointing device.
- A dual-role input key next to Space: tap for Globe on macOS or Win-Space on
  Windows, hold for the matching custom layer, or keep holding for 500 ms to
  cancel the tap action. A dedicated right-side Globe key remains available
  for native macOS Globe shortcuts.
- A shared layer for wireless connections, lighting controls, maintenance, and
  an on-keyboard help entry.
- Diagrams generated directly from `keymap.c` and the keyboard's physical
  geometry, keeping the documentation tied to the firmware source.

Implementation details and regeneration instructions are in the
[`yu` keymap documentation](keyboards/nuphy/air75_v2/ansi/keymaps/yu/readme.md).

## Dual-role behavior

The Caps-position key is event-driven for normal keyboard chords. Pressing any
other keyboard key while Caps is down registers left Command on macOS or left
Control on Windows before that key is sent, with no tapping-term delay. If Caps
is released alone before 500 ms, it sends Escape. If it remains down for 500 ms,
it becomes the real modifier until release, allowing Command-click or
Control-click with an external mouse; after that transition, release does not
send Escape.

The custom-layer key next to Space activates its layer immediately. Releasing
it alone before 500 ms sends Globe on macOS or Win-Space on Windows. Pressing
another keyboard key commits it to layer use immediately, while holding it
alone for 500 ms commits it to layer use without invoking anything. Either
committed path suppresses the input-switch tap on release, so a deliberate long
hold provides a soft exit after an accidental press.

## Build

Clone with submodules, install the QMK CLI following the
[included QMK setup guide](docs/newbs_getting_started.md), and run from the
repository root:

```sh
qmk compile -kb nuphy/air75_v2/ansi -km yu
```

The resulting firmware is `nuphy_air75_v2_ansi_yu.bin`.

## Layers

- **0:** macOS base
- **1:** macOS custom layer
- **2:** Windows base
- **3:** Windows custom layer
- **4-5:** reserved
- **6:** shared connection, lighting, and maintenance controls
- **7:** reserved

Faded, dashed keys in the diagrams pass through to a lower layer. Reserved
layers are intentionally omitted.

### Layer 0: macOS

![Layer 0 macOS layout](keyboards/nuphy/air75_v2/ansi/keymaps/yu/docs/layouts/layer-0-macos.svg?rev=A7)

### Layer 1: macOS custom

![Layer 1 macOS custom layout](keyboards/nuphy/air75_v2/ansi/keymaps/yu/docs/layouts/layer-1-macos-custom.svg?rev=A7)

### Layer 2: Windows

![Layer 2 Windows layout](keyboards/nuphy/air75_v2/ansi/keymaps/yu/docs/layouts/layer-2-windows.svg?rev=A7)

### Layer 3: Windows custom

![Layer 3 Windows custom layout](keyboards/nuphy/air75_v2/ansi/keymaps/yu/docs/layouts/layer-3-windows-custom.svg?rev=A7)

### Layer 6: Common

![Layer 6 common layout](keyboards/nuphy/air75_v2/ansi/keymaps/yu/docs/layouts/layer-6-common.svg?rev=A7)

## Credits

- [Jin Cao / jincao1](https://github.com/jincao1) for the Air75 V2 firmware
  branch used as this repository's direct base.
- [NuPhy](https://github.com/nuphy-src/qmk_firmware) for its published keyboard
  firmware sources.
- [QMK](https://github.com/qmk/qmk_firmware) and its contributors for the
  firmware framework.

This is an unofficial personal firmware project and is not affiliated with or
supported by NuPhy, jincao1, or the QMK project.
