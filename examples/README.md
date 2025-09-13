# scrcpy Keymap Examples

This directory contains sample keymap configuration files for different use cases.

## Available Examples

### Gaming (`keymap_gaming.json`)
- **WASD** → Arrow keys for movement
- **Space** → Touch center screen (jump/action)
- **E** → Enter key
- **Q** → Escape key
- **R/F/C/X** → Touch buttons on right side of screen
- **Tab/Caps Lock** → Disabled

Usage:
```bash
scrcpy --keymap examples/keymap_gaming.json --toggle-mapping-key f12
```

### Productivity (`keymap_productivity.json`)
- **F1-F4** → Home/End/PageUp/PageDown navigation
- **F5-F8** → Touch top toolbar buttons
- **F9-F12** → Common editing keys (Enter/Escape/Backspace/Delete)
- **Caps Lock/Scroll Lock** → Disabled

Usage:
```bash
scrcpy --keymap examples/keymap_productivity.json
```

### Media Controls (`keymap_media.json`)
- **F7-F12** → Touch media control buttons at bottom of screen
- **Arrow Keys** → Touch directional controls
- Designed for media player applications

Usage:
```bash
scrcpy --keymap examples/keymap_media.json --toggle-mapping-key scrolllock
```

### Accessibility (`keymap_accessibility.json`)
- Alternative key mappings for users with different needs
- Remaps navigation keys for easier access
- **Tab** → Down arrow
- **Space/Enter** → Swapped functionality
- **Home/End** → Touch screen corners

Usage:
```bash
scrcpy --keymap examples/keymap_accessibility.json
```

### Alternative Layout (`keymap_alternative.json`)
- **QZOP** layout for movement (alternative to WASD)
- **M/K/L** → Space/Enter/Escape
- **I/U/Y/T** → Touch action buttons
- For users who prefer different key layouts

Usage:
```bash
scrcpy --keymap examples/keymap_alternative.json
```

## Customizing Examples

These examples are starting points. You can modify them by:

1. **Changing coordinates**: Adjust `x` and `y` values in touch mappings to match your app's UI
2. **Adding mappings**: Add more key mappings as needed
3. **Removing mappings**: Delete entries you don't need
4. **Changing keys**: Use different source or target keys

## Finding Touch Coordinates

To find the right coordinates for touch mappings:

1. Enable "Show touches" in Android Developer Options
2. Use scrcpy normally and note where you touch the screen
3. The coordinates are shown in device pixels
4. Update your keymap file with the correct coordinates

## Testing Your Keymap

1. Start with a simple keymap (few keys)
2. Test each mapping individually
3. Use the toggle key to switch between mapped and normal mode
4. Check scrcpy logs for any error messages

For more detailed information, see the main [KEYMAP.md](../KEYMAP.md) documentation.