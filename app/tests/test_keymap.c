#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "keymap.h"

static void test_keymap_init_destroy(void) {
    struct sc_keymap keymap;
    sc_keymap_init(&keymap);
    
    assert(keymap.entries == NULL);
    assert(keymap.count == 0);
    assert(keymap.capacity == 0);
    assert(keymap.toggle_key == SC_KEYCODE_UNKNOWN);
    assert(!keymap.toggle_enabled);
    assert(keymap.mapping_enabled);
    
    sc_keymap_destroy(&keymap);
}

static void test_keymap_add_entry(void) {
    struct sc_keymap keymap;
    sc_keymap_init(&keymap);
    
    // Add a key-to-key mapping
    struct sc_keymap_action action = {
        .type = SC_KEYMAP_ACTION_KEY,
        .key = {
            .keycode = SC_KEYCODE_UP,
            .mods_state = 0,
        },
    };
    
    bool ok = sc_keymap_add_entry(&keymap, SC_KEYCODE_w, 0, &action);
    assert(ok);
    assert(keymap.count == 1);
    
    // Test finding the mapping
    const struct sc_keymap_entry *entry = sc_keymap_find_mapping(&keymap, SC_KEYCODE_w, 0);
    assert(entry != NULL);
    assert(entry->from_keycode == SC_KEYCODE_w);
    assert(entry->action.type == SC_KEYMAP_ACTION_KEY);
    assert(entry->action.key.keycode == SC_KEYCODE_UP);
    
    // Test not finding non-existent mapping
    entry = sc_keymap_find_mapping(&keymap, SC_KEYCODE_x, 0);
    assert(entry == NULL);
    
    sc_keymap_destroy(&keymap);
}

static void test_keymap_toggle(void) {
    struct sc_keymap keymap;
    sc_keymap_init(&keymap);
    
    // Set up toggle key
    keymap.toggle_key = SC_KEYCODE_F12;
    keymap.toggle_enabled = true;
    
    assert(keymap.mapping_enabled);
    assert(sc_keymap_is_toggle_key(&keymap, SC_KEYCODE_F12));
    assert(!sc_keymap_is_toggle_key(&keymap, SC_KEYCODE_F11));
    
    // Toggle off
    sc_keymap_toggle(&keymap);
    assert(!keymap.mapping_enabled);
    
    // Toggle back on
    sc_keymap_toggle(&keymap);
    assert(keymap.mapping_enabled);
    
    sc_keymap_destroy(&keymap);
}

static void test_keymap_parse_toggle_key(void) {
    assert(sc_keymap_parse_toggle_key("f12") == SC_KEYCODE_F12);
    assert(sc_keymap_parse_toggle_key("space") == SC_KEYCODE_SPACE);
    assert(sc_keymap_parse_toggle_key("escape") == SC_KEYCODE_ESCAPE);
    assert(sc_keymap_parse_toggle_key("invalid") == SC_KEYCODE_UNKNOWN);
    assert(sc_keymap_parse_toggle_key("") == SC_KEYCODE_UNKNOWN);
    assert(sc_keymap_parse_toggle_key(NULL) == SC_KEYCODE_UNKNOWN);
}

static void test_keymap_disabled(void) {
    struct sc_keymap keymap;
    sc_keymap_init(&keymap);
    
    // Add a mapping
    struct sc_keymap_action action = {
        .type = SC_KEYMAP_ACTION_KEY,
        .key = {
            .keycode = SC_KEYCODE_UP,
            .mods_state = 0,
        },
    };
    
    sc_keymap_add_entry(&keymap, SC_KEYCODE_w, 0, &action);
    
    // Disable mapping
    keymap.mapping_enabled = false;
    
    // Should not find mapping when disabled
    const struct sc_keymap_entry *entry = sc_keymap_find_mapping(&keymap, SC_KEYCODE_w, 0);
    assert(entry == NULL);
    
    // Enable and try again
    keymap.mapping_enabled = true;
    entry = sc_keymap_find_mapping(&keymap, SC_KEYCODE_w, 0);
    assert(entry != NULL);
    
    sc_keymap_destroy(&keymap);
}

int main(void) {
    test_keymap_init_destroy();
    test_keymap_add_entry();
    test_keymap_toggle();
    test_keymap_parse_toggle_key();
    test_keymap_disabled();
    
    printf("All keymap tests passed!\n");
    return 0;
}