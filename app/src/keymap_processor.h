#ifndef SC_KEYMAP_PROCESSOR_H
#define SC_KEYMAP_PROCESSOR_H

#include "common.h"

#include <stdbool.h>

#include "keymap.h"
#include "trait/key_processor.h"
#include "trait/mouse_processor.h"

/**
 * Key processor that wraps another key processor to provide key mapping functionality
 */
struct sc_keymap_processor {
    struct sc_key_processor key_processor; // Must be first field
    
    struct sc_keymap *keymap;
    struct sc_key_processor *original_kp;    // Wrapped key processor
    struct sc_mouse_processor *mp;           // For touch injection
    struct sc_screen *screen;                // For coordinate conversion
};

struct sc_keymap_processor_params {
    struct sc_keymap *keymap;
    struct sc_key_processor *original_kp;
    struct sc_mouse_processor *mp;
    struct sc_screen *screen;
};

/**
 * Initialize the keymap processor
 */
bool
sc_keymap_processor_init(struct sc_keymap_processor *kmp,
                         const struct sc_keymap_processor_params *params);

/**
 * Destroy the keymap processor
 */
void
sc_keymap_processor_destroy(struct sc_keymap_processor *kmp);

#endif