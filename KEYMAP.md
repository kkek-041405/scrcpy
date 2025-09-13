# Key Mapping in scrcpy

scrcpy supports custom key mapping through configuration files, allowing you to customize keyboard input behavior for different use cases like gaming, productivity, or accessibility.

## Basic Usage

```bash
# Use a keymap configuration file
scrcpy --keymap /path/to/keymap.json

# Set a toggle key to enable/disable mapping at runtime
scrcpy --keymap keymap.json --toggle-mapping-key f12
```

## Keymap Configuration Format

Keymap files use JSON format with the following structure:

```json
{
  "source_key": {"type": "action_type", ...action_parameters},
  "another_key": {"type": "action_type", ...action_parameters}
}
```

### Action Types

#### 1. Key Mapping (`"type": "key"`)
Maps one key to another key:

```json
{
  "w": {"type": "key", "key": "up"},
  "a": {"type": "key", "key": "left"},
  "s": {"type": "key", "key": "down"},
  "d": {"type": "key", "key": "right"}
}
```

#### 2. Touch Mapping (`"type": "touch"`)
Maps a key to a touch event at specific screen coordinates:

```json
{
  "space": {"type": "touch", "x": 500, "y": 500},
  "return": {"type": "touch", "x": 100, "y": 200}
}
```

#### 3. Key Disabling (`"type": "disable"`)
Disables a key (prevents it from being sent to the device):

```json
{
  "tab": {"type": "disable"},
  "capslock": {"type": "disable"}
}
```

### Supported Key Names

Common key names that can be used in keymap configurations:

**Letters**: `a`, `b`, `c`, ..., `z`
**Numbers**: `0`, `1`, `2`, ..., `9`
**Special Keys**:
- `space`, `return`/`enter`, `escape`/`esc`, `backspace`, `tab`
- `delete`, `insert`, `home`, `end`, `pageup`, `pagedown`
- `up`, `down`, `left`, `right`
- `f1`, `f2`, ..., `f12`
- `lctrl`, `rctrl`, `lshift`, `rshift`, `lalt`, `ralt`
- `lgui`, `rgui` (Windows/Cmd keys)
- `scrolllock`, `capslock`, `printscreen`, `pause`

## Example Configurations

### Gaming Configuration (WASD Movement)
```json
{
  "w": {"type": "key", "key": "up"},
  "a": {"type": "key", "key": "left"},
  "s": {"type": "key", "key": "down"},
  "d": {"type": "key", "key": "right"},
  "space": {"type": "touch", "x": 960, "y": 540},
  "e": {"type": "key", "key": "return"},
  "q": {"type": "key", "key": "escape"},
  "tab": {"type": "disable"}
}
```

### Productivity Configuration
```json
{
  "f1": {"type": "key", "key": "home"},
  "f2": {"type": "key", "key": "end"},
  "f3": {"type": "touch", "x": 50, "y": 100},
  "f4": {"type": "touch", "x": 150, "y": 100},
  "capslock": {"type": "disable"}
}
```

### Media Controls
```json
{
  "f7": {"type": "touch", "x": 100, "y": 1800},
  "f8": {"type": "touch", "x": 200, "y": 1800},
  "f9": {"type": "touch", "x": 300, "y": 1800},
  "f10": {"type": "touch", "x": 400, "y": 1800}
}
```

## Runtime Toggle

When using `--toggle-mapping-key`, you can:
- Press the toggle key to enable/disable key mapping on the fly
- The toggle key itself is never sent to the device
- Useful for switching between mapped and normal input modes

Example:
```bash
scrcpy --keymap gaming.json --toggle-mapping-key scrolllock
```

## Advanced Features

### Touch Coordinates

Touch coordinates are specified in device pixels:
- `x`: horizontal position (0 = left edge)
- `y`: vertical position (0 = top edge)

The coordinate system matches the device's natural orientation.

### Error Handling

scrcpy will:
- Log errors if the keymap file cannot be loaded
- Fall back to normal behavior if keymap parsing fails
- Continue working normally for keys not defined in the mapping
- Validate touch coordinates and key names

### Performance

- Key mapping adds minimal overhead to input processing
- Only mapped keys are intercepted; other keys pass through normally
- Toggle functionality works instantly without device communication

## Troubleshooting

**Keymap not loading:**
- Check file path and permissions
- Verify JSON syntax with a validator
- Check scrcpy logs for specific error messages

**Keys not mapping correctly:**
- Verify key names match the supported list
- Check that the target device supports the mapped actions
- Test with simpler mappings first

**Touch coordinates not working:**
- Ensure coordinates are within device screen bounds
- Remember coordinates are in device pixels, not scaled
- Use device developer options to show touch points for testing