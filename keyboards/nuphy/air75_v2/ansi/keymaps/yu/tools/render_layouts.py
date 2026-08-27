#!/usr/bin/env python3
"""Render the Yu keymap documentation directly from QMK's source data."""

from __future__ import annotations

import argparse
import html
import json
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


KEYMAP_DIR = Path(__file__).resolve().parent.parent
REPO_ROOT = next(
    parent for parent in KEYMAP_DIR.parents if (parent / "keyboards" / "nuphy").is_dir()
)
KEYMAP_C = KEYMAP_DIR / "keymap.c"
KEYBOARD_JSON = REPO_ROOT / "keyboards/nuphy/air75_v2/ansi/keyboard.json"
OUTPUT_DIR = KEYMAP_DIR / "docs/layouts"

KEYBOARD = "nuphy/air75_v2/ansi"
KEYMAP = "yu"
LAYOUT = "LAYOUT_75_ansi"

UNIT = 96
KEY_GAP = 6
BOARD_X = 24
BOARD_Y = 104
BOARD_PADDING = 8
CANVAS_WIDTH = 1600
CANVAS_HEIGHT = 780


@dataclass(frozen=True)
class LayerDoc:
    index: int
    slug: str
    title: str
    subtitle: str
    inherit_from: int | None = None


@dataclass(frozen=True)
class KeyLabel:
    primary: str
    secondary: str = ""
    category: str = "normal"


LAYERS = (
    LayerDoc(
        0,
        "layer-0-macos",
        "Layer 0 · macOS",
        "Default layer · Caps position: tap Esc, chord with another key for Command",
    ),
    LayerDoc(
        1,
        "layer-1-macos-fn",
        "Layer 1 · macOS Fn",
        "Hold Super Fn on Layer 0 · faded keys inherit the macOS base layer",
        inherit_from=0,
    ),
    LayerDoc(
        2,
        "layer-2-windows",
        "Layer 2 · Windows",
        "Default layer · Caps position: tap Esc, chord with another key for Control",
    ),
    LayerDoc(
        3,
        "layer-3-windows-fn",
        "Layer 3 · Windows Fn",
        "Hold Super Fn on Layer 2 · faded keys inherit the Windows base layer",
        inherit_from=2,
    ),
    LayerDoc(
        6,
        "layer-6-common",
        "Layer 6 · Common",
        "Hold top-left Esc or the Common/Cat key · connection, lighting, and maintenance",
    ),
)


KEY_LABELS = {
    "KC_ESC": KeyLabel("Esc"),
    "KC_GRV": KeyLabel("`", "~"),
    "KC_MINS": KeyLabel("-", "_"),
    "KC_EQL": KeyLabel("=", "+"),
    "KC_LBRC": KeyLabel("[", "{"),
    "KC_RBRC": KeyLabel("]", "}"),
    "KC_BSLS": KeyLabel("\\", "|"),
    "KC_SCLN": KeyLabel(";", ":"),
    "KC_QUOT": KeyLabel("'", '"'),
    "KC_COMM": KeyLabel(",", "<"),
    "KC_DOT": KeyLabel(".", ">"),
    "KC_SLSH": KeyLabel("/", "?"),
    "KC_BSPC": KeyLabel("Backspace"),
    "KC_TAB": KeyLabel("Tab"),
    "KC_ENT": KeyLabel("Enter"),
    "KC_SPC": KeyLabel("Space"),
    "KC_INS": KeyLabel("Insert"),
    "KC_DEL": KeyLabel("Delete"),
    "KC_HOME": KeyLabel("Home"),
    "KC_END": KeyLabel("End"),
    "KC_PGUP": KeyLabel("Page Up"),
    "KC_PGDN": KeyLabel("Page Down"),
    "KC_PSCR": KeyLabel("Print Screen"),
    "KC_UP": KeyLabel("↑"),
    "KC_DOWN": KeyLabel("↓"),
    "KC_LEFT": KeyLabel("←"),
    "KC_RGHT": KeyLabel("→"),
    "KC_CAPS": KeyLabel("Caps Lock", category="modifier"),
    "KC_BRID": KeyLabel("Brightness", "−", "action"),
    "KC_BRIU": KeyLabel("Brightness", "+", "action"),
    "KC_MCTL": KeyLabel("Mission", "Control", "action"),
    "MAC_VOICE": KeyLabel("Dictation", category="action"),
    "MAC_DND": KeyLabel("Do Not", "Disturb", "action"),
    "KC_MPRV": KeyLabel("Previous", category="action"),
    "KC_MPLY": KeyLabel("Play / Pause", category="action"),
    "KC_MNXT": KeyLabel("Next", category="action"),
    "KC_MUTE": KeyLabel("Mute", category="action"),
    "KC_VOLD": KeyLabel("Volume", "−", "action"),
    "KC_VOLU": KeyLabel("Volume", "+", "action"),
    "KC_MS_WH_UP": KeyLabel("Scroll", "↑", "action"),
    "KC_MS_WH_DOWN": KeyLabel("Scroll", "↓", "action"),
    "KC_MS_BTN1": KeyLabel("Mouse", "Left", "action"),
    "KC_MS_BTN2": KeyLabel("Mouse", "Right", "action"),
    "KC_MS_UP": KeyLabel("Mouse", "↑", "action"),
    "KC_MS_DOWN": KeyLabel("Mouse", "↓", "action"),
    "KC_MS_LEFT": KeyLabel("Mouse", "←", "action"),
    "KC_MS_RIGHT": KeyLabel("Mouse", "→", "action"),
    "KC_INT3": KeyLabel("JIS ¥", category="shortcut"),
    "KC_INT1": KeyLabel("JIS Ro", category="shortcut"),
    "KC_NUBS": KeyLabel("Non-US", "\\ |", "shortcut"),
    "CAPS_GUI_ESC": KeyLabel("Esc", "chord ⌘", "modifier"),
    "CAPS_CTL_ESC": KeyLabel("Esc", "chord Ctrl", "modifier"),
    "MAC_GLOBE": KeyLabel("Globe", "macOS", "modifier"),
    "LAYOUT_HELP": KeyLabel("Help", "Layer 6", "shortcut"),
    "LNK_BLE1": KeyLabel("Bluetooth 1", category="hardware"),
    "LNK_BLE2": KeyLabel("Bluetooth 2", category="hardware"),
    "LNK_BLE3": KeyLabel("Bluetooth 3", category="hardware"),
    "LNK_RF": KeyLabel("2.4 GHz", category="hardware"),
    "SIDE_MOD": KeyLabel("Side", "Mode +", "hardware"),
    "SIDE_SPI": KeyLabel("Side Speed", "+", "hardware"),
    "SIDE_VAI": KeyLabel("Side Bright", "+", "hardware"),
    "SIDE_HUI": KeyLabel("Side Hue", "+", "hardware"),
    "SIDE_SPD": KeyLabel("Side Speed", "−", "hardware"),
    "SIDE_VAD": KeyLabel("Side Bright", "−", "hardware"),
    "RGB_MOD": KeyLabel("Main", "Mode +", "hardware"),
    "RGB_RMOD": KeyLabel("Main", "Mode −", "hardware"),
    "RGB_SPI": KeyLabel("Main Speed", "+", "hardware"),
    "RGB_SPD": KeyLabel("Main Speed", "−", "hardware"),
    "RGB_VAI": KeyLabel("Main Bright", "+", "hardware"),
    "RGB_VAD": KeyLabel("Main Bright", "−", "hardware"),
    "RGB_HUI": KeyLabel("Main Hue", "+", "hardware"),
    "RGB_HUD": KeyLabel("Main Hue", "−", "hardware"),
    "RGB_SAI": KeyLabel("Main Sat", "+", "hardware"),
    "RGB_SAD": KeyLabel("Main Sat", "−", "hardware"),
    "DEV_RESET": KeyLabel("Reset", "hold", "system"),
    "SLEEP_MODE": KeyLabel("Sleep", category="system"),
    "BAT_SHOW": KeyLabel("Battery", category="system"),
}


MODIFIER_LABELS = {
    "KC_LCTL": ("Left Control", "modifier"),
    "KC_RCTL": ("Right Control", "modifier"),
    "KC_LSFT": ("Left Shift", "modifier"),
    "KC_RSFT": ("Right Shift", "modifier"),
}


LEGEND = (
    ("inherited", "Inherited"),
    ("modifier", "Modifier / custom"),
    ("layer", "Layer access"),
    ("shortcut", "Shortcut / macro"),
    ("action", "Media / mouse"),
    ("hardware", "Connection / lighting"),
    ("system", "Maintenance"),
)


def run_c2json() -> dict:
    command = [
        "qmk",
        "c2json",
        "-kb",
        KEYBOARD,
        "-km",
        KEYMAP,
        str(KEYMAP_C),
    ]
    try:
        result = subprocess.run(
            command,
            cwd=REPO_ROOT,
            check=True,
            capture_output=True,
            text=True,
        )
    except FileNotFoundError as exc:
        raise SystemExit("qmk is required; install or activate the QMK CLI first") from exc
    except subprocess.CalledProcessError as exc:
        sys.stderr.write(exc.stderr)
        raise SystemExit(f"qmk c2json failed with status {exc.returncode}") from exc
    return json.loads(result.stdout)


def load_geometry() -> list[dict]:
    keyboard = json.loads(KEYBOARD_JSON.read_text(encoding="utf-8"))
    return keyboard["layouts"][LAYOUT]["layout"]


def platform_modifier(code: str, layer_index: int) -> KeyLabel | None:
    if code in MODIFIER_LABELS:
        primary, category = MODIFIER_LABELS[code]
        return KeyLabel(primary, category=category)

    is_macos = layer_index in (0, 1)
    labels = {
        "KC_LALT": "Left Option" if is_macos else "Left Alt",
        "KC_RALT": "Right Option" if is_macos else "Right Alt",
        "KC_LGUI": "Left Command" if is_macos else "Left GUI",
        "KC_RGUI": "Right Command" if is_macos else "Right GUI",
    }
    if code in labels:
        return KeyLabel(labels[code], category="modifier")
    return None


def label_for(code: str, layer_index: int) -> KeyLabel:
    if code in KEY_LABELS:
        return KEY_LABELS[code]

    modifier = platform_modifier(code, layer_index)
    if modifier:
        return modifier

    if match := re.fullmatch(r"KC_([A-Z])", code):
        return KeyLabel(match.group(1))
    if match := re.fullmatch(r"KC_([0-9])", code):
        return KeyLabel(match.group(1))
    if match := re.fullmatch(r"KC_F([0-9]{1,2})", code):
        return KeyLabel(f"F{match.group(1)}")
    if match := re.fullmatch(r"QK_MACRO_([0-9]+)", code):
        return KeyLabel(f"M{match.group(1)}", "empty", "shortcut")
    if match := re.fullmatch(r"HYPR\((.+)\)", code):
        nested = label_for(match.group(1), layer_index)
        return KeyLabel("Hyper", nested.primary, "shortcut")
    if match := re.fullmatch(r"MO\((.+)\)", code):
        layer = {
            "L_MAC_FN": "macOS Fn",
            "L_WIN_FN": "Windows Fn",
            "L_COMMON": "Common",
        }.get(match.group(1), match.group(1))
        return KeyLabel(layer, "hold", "layer")
    if match := re.fullmatch(r"LT\((.+),\s*(.+)\)", code):
        layer = {"L_COMMON": "Common"}.get(match.group(1), match.group(1))
        tap = label_for(match.group(2), layer_index)
        return KeyLabel(tap.primary, f"hold {layer}", "layer")

    shortcuts = {
        "LSG(KC_4)": KeyLabel("Area Shot", "⇧⌘4", "shortcut"),
        "LSG(KC_3)": KeyLabel("Full Shot", "⇧⌘3", "shortcut"),
        "G(KC_SPC)": KeyLabel("Spotlight", "⌘ Space", "shortcut"),
        "G(KC_TAB)": KeyLabel("Task View", "Win Tab", "shortcut"),
        "G(KC_S)": KeyLabel("Search", "Win S", "shortcut"),
        "G(KC_H)": KeyLabel("Voice Typing", "Win H", "shortcut"),
        "G(KC_N)": KeyLabel("Notifications", "Win N", "shortcut"),
        "S(KC_GRV)": KeyLabel("~", "Shift `", "shortcut"),
    }
    if code in shortcuts:
        return shortcuts[code]

    readable = code.removeprefix("KC_").replace("_", " ").title()
    return KeyLabel(readable, code, "shortcut")


def inherited_label(code: str, base_code: str | None, base_layer: int | None) -> KeyLabel:
    if base_code is None or base_layer is None:
        return KeyLabel("▽", "pass through", "inherited")
    base = label_for(base_code, base_layer)
    return KeyLabel(base.primary, "inherited", "inherited")


def text_size(text: str, key_width: float, preferred: float) -> float:
    if not text:
        return preferred
    available = key_width - 16
    estimated = len(text) * preferred * 0.56
    if estimated <= available:
        return preferred
    return max(11.0, preferred * available / estimated)


def svg_text(value: str) -> str:
    return html.escape(value, quote=False)


def render_key(
    geometry: dict,
    code: str,
    label: KeyLabel,
) -> str:
    x = BOARD_X + BOARD_PADDING + float(geometry["x"]) * UNIT + KEY_GAP / 2
    y = BOARD_Y + BOARD_PADDING + float(geometry["y"]) * UNIT + KEY_GAP / 2
    width = float(geometry.get("w", 1)) * UNIT - KEY_GAP
    height = float(geometry.get("h", 1)) * UNIT - KEY_GAP
    center_x = x + width / 2
    center_y = y + height / 2
    primary_size = text_size(label.primary, width, 17)
    secondary_size = text_size(label.secondary, width, 12)

    if label.secondary:
        primary_y = center_y - 4
        secondary_y = center_y + 17
    else:
        primary_y = center_y + primary_size * 0.34
        secondary_y = center_y

    pieces = [
        f'<g class="key key-{label.category}">',
        f"<title>{svg_text(code)}</title>",
        (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{width:.1f}" '
            f'height="{height:.1f}" rx="9" />'
        ),
        (
            f'<text class="key-primary" x="{center_x:.1f}" y="{primary_y:.1f}" '
            f'style="font-size:{primary_size:.1f}px">{svg_text(label.primary)}</text>'
        ),
    ]
    if label.secondary:
        pieces.append(
            f'<text class="key-secondary" x="{center_x:.1f}" y="{secondary_y:.1f}" '
            f'style="font-size:{secondary_size:.1f}px">{svg_text(label.secondary)}</text>'
        )
    pieces.append("</g>")
    return "\n".join(pieces)


def render_legend() -> str:
    widths = [136, 176, 142, 170, 154, 210, 142]
    total_width = sum(widths)
    x = (CANVAS_WIDTH - total_width) / 2
    y = 735
    pieces = ['<g class="legend" aria-label="Key category legend">']
    for (category, label), width in zip(LEGEND, widths):
        pieces.append(
            f'<rect class="legend-swatch key-{category}" x="{x:.1f}" y="{y - 14}" '
            'width="20" height="20" rx="5" />'
        )
        pieces.append(
            f'<text x="{x + 28:.1f}" y="{y + 1}">{svg_text(label)}</text>'
        )
        x += width
    pieces.append("</g>")
    return "\n".join(pieces)


def render_layer(layer: LayerDoc, keycodes: list[list[str]], geometry: list[dict]) -> str:
    if layer.index >= len(keycodes):
        raise SystemExit(f"Layer {layer.index} is missing from qmk c2json output")
    if len(keycodes[layer.index]) != len(geometry):
        raise SystemExit(
            f"Layer {layer.index} has {len(keycodes[layer.index])} keys; "
            f"{LAYOUT} has {len(geometry)} positions"
        )

    base_codes = keycodes[layer.inherit_from] if layer.inherit_from is not None else None
    keys = []
    for index, (position, code) in enumerate(zip(geometry, keycodes[layer.index])):
        if code == "KC_TRNS":
            base_code = base_codes[index] if base_codes else None
            label = inherited_label(code, base_code, layer.inherit_from)
        else:
            label = label_for(code, layer.index)
        keys.append(render_key(position, code, label))

    board_width = 16 * UNIT + BOARD_PADDING * 2
    board_height = 6 * UNIT + BOARD_PADDING * 2
    source_note = "Generated from keymap.c + keyboard.json · do not edit"
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="{CANVAS_WIDTH}" height="{CANVAS_HEIGHT}" viewBox="0 0 {CANVAS_WIDTH} {CANVAS_HEIGHT}" role="img" aria-labelledby="title desc">
  <title id="title">{svg_text(layer.title)}</title>
  <desc id="desc">{svg_text(layer.subtitle)}</desc>
  <style>
    .canvas {{ fill: #f8fafc; }}
    .board {{ fill: #cbd5e1; stroke: #64748b; stroke-width: 2; }}
    text {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; fill: #0f172a; }}
    .doc-title {{ font-size: 29px; font-weight: 600; }}
    .doc-subtitle {{ font-size: 15px; fill: #475569; }}
    .source-note {{ font-size: 13px; fill: #64748b; text-anchor: end; }}
    .key rect {{ stroke-width: 2; }}
    .key-primary, .key-secondary {{ text-anchor: middle; dominant-baseline: auto; }}
    .key-primary {{ font-weight: 600; }}
    .key-secondary {{ fill: #475569; }}
    .key-normal rect {{ fill: #ffffff; stroke: #94a3b8; }}
    .key-inherited rect, .legend-swatch.key-inherited {{ fill: #f1f5f9; stroke: #94a3b8; stroke-dasharray: 5 4; }}
    .key-inherited text {{ fill: #64748b; font-weight: 400; }}
    .key-modifier rect, .legend-swatch.key-modifier {{ fill: #dbeafe; stroke: #2563eb; }}
    .key-layer rect, .legend-swatch.key-layer {{ fill: #fef3c7; stroke: #d97706; }}
    .key-shortcut rect, .legend-swatch.key-shortcut {{ fill: #ede9fe; stroke: #7c3aed; }}
    .key-action rect, .legend-swatch.key-action {{ fill: #e0f2fe; stroke: #0284c7; }}
    .key-hardware rect, .legend-swatch.key-hardware {{ fill: #ccfbf1; stroke: #0f766e; }}
    .key-system rect, .legend-swatch.key-system {{ fill: #fee2e2; stroke: #dc2626; }}
    .legend text {{ font-size: 13px; fill: #334155; }}
    .legend-swatch {{ stroke-width: 2; }}
  </style>
  <rect class="canvas" width="{CANVAS_WIDTH}" height="{CANVAS_HEIGHT}" rx="18" />
  <text class="doc-title" x="32" y="44">{svg_text(layer.title)}</text>
  <text class="doc-subtitle" x="32" y="72">{svg_text(layer.subtitle)}</text>
  <text class="source-note" x="1568" y="44">{svg_text(source_note)}</text>
  <rect class="board" x="{BOARD_X}" y="{BOARD_Y}" width="{board_width}" height="{board_height}" rx="18" />
  {chr(10).join(keys)}
  {render_legend()}
</svg>
'''


def write_or_check(path: Path, content: str, check: bool) -> bool:
    if check:
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            print(f"out of date: {path.relative_to(REPO_ROOT)}", file=sys.stderr)
            return False
        return True
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    print(f"wrote {path.relative_to(REPO_ROOT)}")
    return True


def render_png(svg_path: Path, png_path: Path) -> None:
    converter = shutil.which("rsvg-convert")
    if not converter:
        print(
            "skipping PNG export: install librsvg (rsvg-convert); SVG documentation is complete",
            file=sys.stderr,
        )
        return
    subprocess.run(
        [converter, "--format=png", f"--output={png_path}", str(svg_path)],
        check=True,
    )
    print(f"wrote {png_path.relative_to(REPO_ROOT)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if the committed SVG files do not match keymap.c",
    )
    parser.add_argument(
        "--png",
        action="store_true",
        help="also export PNG copies when rsvg-convert is available",
    )
    args = parser.parse_args()

    keymap = run_c2json()
    if keymap.get("layout") != LAYOUT:
        raise SystemExit(f"Expected {LAYOUT}; qmk c2json returned {keymap.get('layout')}")
    keycodes = keymap["layers"]
    geometry = load_geometry()

    success = True
    for layer in LAYERS:
        svg_path = OUTPUT_DIR / f"{layer.slug}.svg"
        svg = render_layer(layer, keycodes, geometry)
        success = write_or_check(svg_path, svg, args.check) and success
        if args.png and not args.check:
            render_png(svg_path, OUTPUT_DIR / f"{layer.slug}.png")

    if args.check and success:
        print("layout documentation is up to date")
    return 0 if success else 1


if __name__ == "__main__":
    raise SystemExit(main())
