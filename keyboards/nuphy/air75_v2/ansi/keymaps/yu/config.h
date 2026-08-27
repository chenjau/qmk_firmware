#pragma once

// Stored in VIA's otherwise-unused layout-options byte. Bump this whenever the
// compiled keymap defaults change so a same-day reflash refreshes VIA EEPROM.
#define YU_LAYOUT_REVISION 0xA3
#define VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT YU_LAYOUT_REVISION
#define VIA_FIRMWARE_VERSION 0x000000A3
