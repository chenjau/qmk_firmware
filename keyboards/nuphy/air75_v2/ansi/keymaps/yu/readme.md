# Yu's Air75 V2 keymap

This keymap is the source of truth for the keyboard layout. VIA remains enabled
for temporary experiments, but importing a VIA layout is not required after
flashing this keymap.

## Build

```sh
qmk compile -kb nuphy/air75_v2/ansi -km yu
```

## Layers

- 0: macOS base
- 1: macOS function layer
- 2: Windows base
- 3: Windows function layer
- 4-5: reserved
- 6: shared connection, lighting, and maintenance controls
- 7: reserved

Layer 1's `/?` key sends the built-in Layer 6 help text. VIA macro slots M0,
M1, and M2 keep their existing key assignments but are empty by default.

When changing the compiled layout, bump `YU_LAYOUT_REVISION` in `config.h`.
This forces the new defaults into VIA's dynamic-keymap EEPROM even when two
firmware builds are flashed on the same day.
