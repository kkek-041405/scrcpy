#ifndef SC_KEYMAP_H
#define SC_KEYMAP_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL_keycode.h>

#include "coords.h"
#include "input_events.h"

// Maximum number of actions in a sequence
#define SC_KEYMAP_MAX_SEQUENCE_ACTIONS 8

// Maximum number of key mappings
#define SC_KEYMAP_MAX_MAPPINGS 256

enum sc_keymap_action_type {
    SC_KEYMAP_ACTION_KEY,      // Map to another key
    SC_KEYMAP_ACTION_TOUCH,    // Map to touch at coordinate
    SC_KEYMAP_ACTION_SEQUENCE, // Map to sequence of actions
    SC_KEYMAP_ACTION_DISABLE,  // Disable/ignore the key
};

struct sc_keymap_key_action {
    enum sc_keycode keycode;
    uint16_t mods_state; // bitwise-OR of sc_mod values
};

struct sc_keymap_touch_action {
    struct sc_position position;
    bool press; // true for press, false for release
};

struct sc_keymap_action {
    enum sc_keymap_action_type type;
    union {
        struct sc_keymap_key_action key;
        struct sc_keymap_touch_action touch;
        struct {
            struct sc_keymap_action *actions;
            size_t count;
        } sequence;
    };
};

struct sc_keymap_entry {
    enum sc_keycode from_keycode;      // Source key
    uint16_t from_mods_state;          // Required modifiers (0 for any)
    struct sc_keymap_action action;    // Target action
};

struct sc_keymap {
    struct sc_keymap_entry *entries;
    size_t count;
    size_t capacity;
    
    // Toggle key configuration
    enum sc_keycode toggle_key;
    bool toggle_enabled;        // Whether toggle key is configured
    bool mapping_enabled;       // Current mapping state
};

/**
 * Initialize a keymap structure
 */
void
sc_keymap_init(struct sc_keymap *keymap);

/**
 * Destroy a keymap structure and free associated memory
 */
void
sc_keymap_destroy(struct sc_keymap *keymap);

/**
 * Load keymap from JSON file
 * 
 * Returns true on success, false on error
 */
bool
sc_keymap_load_from_file(struct sc_keymap *keymap, const char *path);

/**
 * Parse toggle key name to keycode
 * 
 * Returns the keycode or SC_KEYCODE_UNKNOWN if invalid
 */
enum sc_keycode
sc_keymap_parse_toggle_key(const char *key_name);

/**
 * Add a key mapping entry
 * 
 * Returns true on success, false on error (e.g., keymap full)
 */
bool
sc_keymap_add_entry(struct sc_keymap *keymap, 
                    enum sc_keycode from_keycode,
                    uint16_t from_mods_state,
                    const struct sc_keymap_action *action);

/**
 * Find mapping for a key event
 * 
 * Returns the mapping entry or NULL if not found
 */
const struct sc_keymap_entry *
sc_keymap_find_mapping(const struct sc_keymap *keymap, 
                       enum sc_keycode keycode,
                       uint16_t mods_state);

/**
 * Check if a key is the toggle key
 */
bool
sc_keymap_is_toggle_key(const struct sc_keymap *keymap, enum sc_keycode keycode);

/**
 * Toggle the mapping state
 */
void
sc_keymap_toggle(struct sc_keymap *keymap);

/**
 * Check if mapping is currently enabled
 */
bool
sc_keymap_is_enabled(const struct sc_keymap *keymap);

#endif