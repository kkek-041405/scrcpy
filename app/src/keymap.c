#include "keymap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"

// Simple JSON parsing helpers for keymap configuration
// Note: This is a minimal JSON parser specifically for keymap files

static bool
skip_whitespace(const char **json) {
    while (**json && (**json == ' ' || **json == '\t' || **json == '\n' || **json == '\r')) {
        (*json)++;
    }
    return **json != '\0';
}

static bool
parse_string(const char **json, char *buffer, size_t buffer_size) {
    if (!skip_whitespace(json) || **json != '"') {
        return false;
    }
    (*json)++; // Skip opening quote
    
    size_t i = 0;
    while (**json && **json != '"' && i < buffer_size - 1) {
        if (**json == '\\') {
            (*json)++;
            switch (**json) {
                case 'n': buffer[i++] = '\n'; break;
                case 't': buffer[i++] = '\t'; break;
                case 'r': buffer[i++] = '\r'; break;
                case '\\': buffer[i++] = '\\'; break;
                case '"': buffer[i++] = '"'; break;
                default: buffer[i++] = **json; break;
            }
        } else {
            buffer[i++] = **json;
        }
        (*json)++;
    }
    
    if (**json != '"') {
        return false;
    }
    (*json)++; // Skip closing quote
    buffer[i] = '\0';
    return true;
}

static bool
parse_number(const char **json, int *value) {
    if (!skip_whitespace(json)) {
        return false;
    }
    
    char *end;
    *value = (int)strtol(*json, &end, 10);
    if (end == *json) {
        return false;
    }
    *json = end;
    return true;
}

static enum sc_keycode
parse_keycode(const char *key_name) {
    // Map common key names to keycodes
    static const struct {
        const char *name;
        enum sc_keycode keycode;
    } key_map[] = {
        {"space", SC_KEYCODE_SPACE},
        {"return", SC_KEYCODE_RETURN},
        {"enter", SC_KEYCODE_RETURN},
        {"escape", SC_KEYCODE_ESCAPE},
        {"esc", SC_KEYCODE_ESCAPE},
        {"backspace", SC_KEYCODE_BACKSPACE},
        {"tab", SC_KEYCODE_TAB},
        {"delete", SC_KEYCODE_DELETE},
        {"insert", SC_KEYCODE_INSERT},
        {"home", SC_KEYCODE_HOME},
        {"end", SC_KEYCODE_END},
        {"pageup", SC_KEYCODE_PAGEUP},
        {"pagedown", SC_KEYCODE_PAGEDOWN},
        {"up", SC_KEYCODE_UP},
        {"down", SC_KEYCODE_DOWN},
        {"left", SC_KEYCODE_LEFT},
        {"right", SC_KEYCODE_RIGHT},
        {"f1", SC_KEYCODE_F1},
        {"f2", SC_KEYCODE_F2},
        {"f3", SC_KEYCODE_F3},
        {"f4", SC_KEYCODE_F4},
        {"f5", SC_KEYCODE_F5},
        {"f6", SC_KEYCODE_F6},
        {"f7", SC_KEYCODE_F7},
        {"f8", SC_KEYCODE_F8},
        {"f9", SC_KEYCODE_F9},
        {"f10", SC_KEYCODE_F10},
        {"f11", SC_KEYCODE_F11},
        {"f12", SC_KEYCODE_F12},
        {"lctrl", SC_KEYCODE_LCTRL},
        {"rctrl", SC_KEYCODE_RCTRL},
        {"lshift", SC_KEYCODE_LSHIFT},
        {"rshift", SC_KEYCODE_RSHIFT},
        {"lalt", SC_KEYCODE_LALT},
        {"ralt", SC_KEYCODE_RALT},
        {"lgui", SC_KEYCODE_LGUI},
        {"rgui", SC_KEYCODE_RGUI},
        {"scrolllock", SC_KEYCODE_SCROLLLOCK},
        {"capslock", SC_KEYCODE_CAPSLOCK},
        {"printscreen", SC_KEYCODE_PRINTSCREEN},
        {"pause", SC_KEYCODE_PAUSE},
    };
    
    for (size_t i = 0; i < ARRAY_LEN(key_map); i++) {
        if (strcmp(key_name, key_map[i].name) == 0) {
            return key_map[i].keycode;
        }
    }
    
    // Handle single character keys
    if (strlen(key_name) == 1) {
        char c = key_name[0];
        if (c >= 'a' && c <= 'z') {
            return SC_KEYCODE_a + (c - 'a');
        }
        if (c >= '0' && c <= '9') {
            return SC_KEYCODE_0 + (c - '0');
        }
    }
    
    return SC_KEYCODE_UNKNOWN;
}

static bool
parse_keymap_action(const char **json, struct sc_keymap_action *action) {
    if (!skip_whitespace(json) || **json != '{') {
        return false;
    }
    (*json)++; // Skip opening brace
    
    char type_str[32] = {0};
    char key_str[32] = {0};
    int x = 0, y = 0;
    bool has_type = false, has_key = false, has_x = false, has_y = false;
    
    while (skip_whitespace(json) && **json != '}') {
        // Parse key name
        char field_name[32];
        if (!parse_string(json, field_name, sizeof(field_name))) {
            return false;
        }
        
        if (!skip_whitespace(json) || **json != ':') {
            return false;
        }
        (*json)++; // Skip colon
        
        // Parse value based on field name
        if (strcmp(field_name, "type") == 0) {
            if (!parse_string(json, type_str, sizeof(type_str))) {
                return false;
            }
            has_type = true;
        } else if (strcmp(field_name, "key") == 0) {
            if (!parse_string(json, key_str, sizeof(key_str))) {
                return false;
            }
            has_key = true;
        } else if (strcmp(field_name, "x") == 0) {
            if (!parse_number(json, &x)) {
                return false;
            }
            has_x = true;
        } else if (strcmp(field_name, "y") == 0) {
            if (!parse_number(json, &y)) {
                return false;
            }
            has_y = true;
        }
        
        // Skip optional comma
        skip_whitespace(json);
        if (**json == ',') {
            (*json)++;
        }
    }
    
    if (**json != '}') {
        return false;
    }
    (*json)++; // Skip closing brace
    
    if (!has_type) {
        return false;
    }
    
    // Set action based on type
    if (strcmp(type_str, "key") == 0) {
        if (!has_key) {
            return false;
        }
        action->type = SC_KEYMAP_ACTION_KEY;
        action->key.keycode = parse_keycode(key_str);
        action->key.mods_state = 0; // TODO: Parse modifiers
        return action->key.keycode != SC_KEYCODE_UNKNOWN;
    } else if (strcmp(type_str, "touch") == 0) {
        if (!has_x || !has_y) {
            return false;
        }
        action->type = SC_KEYMAP_ACTION_TOUCH;
        action->touch.position.point.x = x;
        action->touch.position.point.y = y;
        action->touch.position.screen_size.width = 0; // Will be set by caller
        action->touch.position.screen_size.height = 0;
        action->touch.press = true; // Default to press
        return true;
    } else if (strcmp(type_str, "disable") == 0) {
        action->type = SC_KEYMAP_ACTION_DISABLE;
        return true;
    }
    
    return false;
}

void
sc_keymap_init(struct sc_keymap *keymap) {
    keymap->entries = NULL;
    keymap->count = 0;
    keymap->capacity = 0;
    keymap->toggle_key = SC_KEYCODE_UNKNOWN;
    keymap->toggle_enabled = false;
    keymap->mapping_enabled = true; // Default to enabled
}

void
sc_keymap_destroy(struct sc_keymap *keymap) {
    if (keymap->entries) {
        for (size_t i = 0; i < keymap->count; i++) {
            // Free sequence actions if any
            if (keymap->entries[i].action.type == SC_KEYMAP_ACTION_SEQUENCE) {
                free(keymap->entries[i].action.sequence.actions);
            }
        }
        free(keymap->entries);
        keymap->entries = NULL;
    }
    keymap->count = 0;
    keymap->capacity = 0;
}

bool
sc_keymap_load_from_file(struct sc_keymap *keymap, const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        LOGE("Could not open keymap file: %s", path);
        return false;
    }
    
    // Read entire file into memory
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > 1024 * 1024) { // Max 1MB
        LOGE("Invalid keymap file size: %ld", file_size);
        fclose(file);
        return false;
    }
    
    char *json = malloc((size_t)file_size + 1);
    if (!json) {
        LOGE("Could not allocate memory for keymap file");
        fclose(file);
        return false;
    }
    
    size_t read_size = fread(json, 1, (size_t)file_size, file);
    fclose(file);
    
    if (read_size != (size_t)file_size) {
        LOGE("Could not read keymap file completely");
        free(json);
        return false;
    }
    json[file_size] = '\0';
    
    // Parse JSON
    const char *json_ptr = json;
    bool success = true;
    
    if (!skip_whitespace(&json_ptr) || *json_ptr != '{') {
        LOGE("Keymap file must contain a JSON object");
        success = false;
        goto cleanup;
    }
    json_ptr++; // Skip opening brace
    
    while (success && skip_whitespace(&json_ptr) && *json_ptr != '}') {
        // Parse mapping entry
        char from_key[32];
        if (!parse_string(&json_ptr, from_key, sizeof(from_key))) {
            LOGE("Invalid key name in keymap");
            success = false;
            break;
        }
        
        if (!skip_whitespace(&json_ptr) || *json_ptr != ':') {
            LOGE("Expected ':' after key name in keymap");
            success = false;
            break;
        }
        json_ptr++; // Skip colon
        
        struct sc_keymap_action action;
        if (!parse_keymap_action(&json_ptr, &action)) {
            LOGE("Invalid action for key '%s' in keymap", from_key);
            success = false;
            break;
        }
        
        enum sc_keycode from_keycode = parse_keycode(from_key);
        if (from_keycode == SC_KEYCODE_UNKNOWN) {
            LOGE("Unknown key name in keymap: %s", from_key);
            success = false;
            break;
        }
        
        if (!sc_keymap_add_entry(keymap, from_keycode, 0, &action)) {
            LOGE("Could not add keymap entry for key '%s'", from_key);
            success = false;
            break;
        }
        
        // Skip optional comma
        skip_whitespace(&json_ptr);
        if (*json_ptr == ',') {
            json_ptr++;
        }
    }
    
    if (success && *json_ptr != '}') {
        LOGE("Unexpected end of keymap file");
        success = false;
    }
    
cleanup:
    free(json);
    
    if (success) {
        LOGI("Loaded keymap with %zu entries", keymap->count);
    } else {
        sc_keymap_destroy(keymap);
        sc_keymap_init(keymap);
    }
    
    return success;
}

enum sc_keycode
sc_keymap_parse_toggle_key(const char *key_name) {
    if (!key_name) {
        return SC_KEYCODE_UNKNOWN;
    }
    return parse_keycode(key_name);
}

bool
sc_keymap_add_entry(struct sc_keymap *keymap, 
                    enum sc_keycode from_keycode,
                    uint16_t from_mods_state,
                    const struct sc_keymap_action *action) {
    // Resize array if needed
    if (keymap->count >= keymap->capacity) {
        size_t new_capacity = keymap->capacity == 0 ? 8 : keymap->capacity * 2;
        if (new_capacity > SC_KEYMAP_MAX_MAPPINGS) {
            new_capacity = SC_KEYMAP_MAX_MAPPINGS;
        }
        if (keymap->count >= new_capacity) {
            return false; // Cannot add more entries
        }
        
        struct sc_keymap_entry *new_entries = realloc(keymap->entries, 
                                                       new_capacity * sizeof(struct sc_keymap_entry));
        if (!new_entries) {
            return false;
        }
        
        keymap->entries = new_entries;
        keymap->capacity = new_capacity;
    }
    
    // Add entry
    struct sc_keymap_entry *entry = &keymap->entries[keymap->count];
    entry->from_keycode = from_keycode;
    entry->from_mods_state = from_mods_state;
    entry->action = *action;
    
    keymap->count++;
    return true;
}

const struct sc_keymap_entry *
sc_keymap_find_mapping(const struct sc_keymap *keymap, 
                       enum sc_keycode keycode,
                       uint16_t mods_state) {
    if (!keymap->mapping_enabled) {
        return NULL;
    }
    
    for (size_t i = 0; i < keymap->count; i++) {
        const struct sc_keymap_entry *entry = &keymap->entries[i];
        if (entry->from_keycode == keycode) {
            // Check if modifiers match (0 means any modifiers accepted)
            if (entry->from_mods_state == 0 || entry->from_mods_state == mods_state) {
                return entry;
            }
        }
    }
    
    return NULL;
}

bool
sc_keymap_is_toggle_key(const struct sc_keymap *keymap, enum sc_keycode keycode) {
    return keymap->toggle_enabled && keymap->toggle_key == keycode;
}

void
sc_keymap_toggle(struct sc_keymap *keymap) {
    if (keymap->toggle_enabled) {
        keymap->mapping_enabled = !keymap->mapping_enabled;
        LOGI("Key mapping %s", keymap->mapping_enabled ? "enabled" : "disabled");
    }
}

bool
sc_keymap_is_enabled(const struct sc_keymap *keymap) {
    return keymap->mapping_enabled;
}