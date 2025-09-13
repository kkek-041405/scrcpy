#include "keymap_processor.h"

#include <assert.h>
#include <stdlib.h>

#include "input_events.h"
#include "screen.h"
#include "util/log.h"

#define DOWNCAST(KP) container_of(KP, struct sc_keymap_processor, key_processor)

static void
sc_keymap_processor_process_key(struct sc_key_processor *kp,
                                const struct sc_key_event *event,
                                uint64_t ack_to_wait);

static void
sc_keymap_processor_process_text(struct sc_key_processor *kp,
                                 const struct sc_text_event *event);

static const struct sc_key_processor_ops keymap_processor_ops = {
    .process_key = sc_keymap_processor_process_key,
    .process_text = sc_keymap_processor_process_text,
};

static void
execute_touch_action(struct sc_keymap_processor *kmp,
                     const struct sc_keymap_touch_action *touch_action,
                     enum sc_action action) {
    if (!kmp->mp || !kmp->screen) {
        return;
    }
    
    // Convert screen coordinates
    struct sc_position position = touch_action->position;
    
    // If screen size is not set in the action, use current screen size
    if (position.screen_size.width == 0 || position.screen_size.height == 0) {
        position.screen_size = kmp->screen->content_size;
    }
    
    // Create mouse click event for touch
    struct sc_mouse_click_event click_event = {
        .position = position,
        .action = action,
        .button = SC_MOUSE_BUTTON_LEFT,
        .pointer_id = 0,
        .buttons_state = action == SC_ACTION_DOWN ? SC_MOUSE_BUTTON_LEFT : 0,
    };
    
    // Send the click event through the mouse processor
    if (kmp->mp->ops && kmp->mp->ops->process_mouse_click) {
        kmp->mp->ops->process_mouse_click(kmp->mp, &click_event);
    }
}

static void
execute_keymap_action(struct sc_keymap_processor *kmp,
                      const struct sc_keymap_action *action,
                      enum sc_action key_action,
                      uint64_t ack_to_wait) {
    switch (action->type) {
        case SC_KEYMAP_ACTION_KEY: {
            // Create a new key event with the mapped key
            struct sc_key_event mapped_event = {
                .action = key_action,
                .keycode = action->key.keycode,
                .scancode = SC_SCANCODE_UNKNOWN, // Let the original processor handle this
                .mods_state = action->key.mods_state,
                .repeat = false,
            };
            
            // Send to original key processor
            if (kmp->original_kp && kmp->original_kp->ops->process_key) {
                kmp->original_kp->ops->process_key(kmp->original_kp, &mapped_event, ack_to_wait);
            }
            break;
        }
        
        case SC_KEYMAP_ACTION_TOUCH: {
            // For key down, press; for key up, release
            enum sc_action touch_action = key_action;
            execute_touch_action(kmp, &action->touch, touch_action);
            break;
        }
        
        case SC_KEYMAP_ACTION_SEQUENCE: {
            // Execute sequence only on key down to avoid double execution
            if (key_action == SC_ACTION_DOWN && action->sequence.actions) {
                for (size_t i = 0; i < action->sequence.count; i++) {
                    execute_keymap_action(kmp, &action->sequence.actions[i], SC_ACTION_DOWN, ack_to_wait);
                }
            }
            break;
        }
        
        case SC_KEYMAP_ACTION_DISABLE:
            // Do nothing - key is disabled
            break;
            
        default:
            LOGW("Unknown keymap action type: %d", action->type);
            break;
    }
}

static void
sc_keymap_processor_process_key(struct sc_key_processor *kp,
                                const struct sc_key_event *event,
                                uint64_t ack_to_wait) {
    struct sc_keymap_processor *kmp = DOWNCAST(kp);
    
    // Check for toggle key first (always processed, even when mapping is disabled)
    if (event->action == SC_ACTION_DOWN && 
        sc_keymap_is_toggle_key(kmp->keymap, event->keycode)) {
        sc_keymap_toggle(kmp->keymap);
        return; // Don't pass through toggle key
    }
    
    // Look for mapping
    const struct sc_keymap_entry *mapping = sc_keymap_find_mapping(kmp->keymap, 
                                                                    event->keycode, 
                                                                    event->mods_state);
    
    if (mapping) {
        // Execute mapped action
        execute_keymap_action(kmp, &mapping->action, event->action, ack_to_wait);
    } else {
        // No mapping found, pass through to original processor
        if (kmp->original_kp && kmp->original_kp->ops->process_key) {
            kmp->original_kp->ops->process_key(kmp->original_kp, event, ack_to_wait);
        }
    }
}

static void
sc_keymap_processor_process_text(struct sc_key_processor *kp,
                                 const struct sc_text_event *event) {
    struct sc_keymap_processor *kmp = DOWNCAST(kp);
    
    // Text events are always passed through (no mapping for text)
    if (kmp->original_kp && kmp->original_kp->ops->process_text) {
        kmp->original_kp->ops->process_text(kmp->original_kp, event);
    }
}

bool
sc_keymap_processor_init(struct sc_keymap_processor *kmp,
                         const struct sc_keymap_processor_params *params) {
    assert(params->keymap);
    assert(params->original_kp);
    
    kmp->keymap = params->keymap;
    kmp->original_kp = params->original_kp;
    kmp->mp = params->mp;
    kmp->screen = params->screen;
    
    // Set up the key processor interface
    kmp->key_processor.async_paste = params->original_kp->async_paste;
    kmp->key_processor.hid = params->original_kp->hid;
    kmp->key_processor.ops = &keymap_processor_ops;
    
    return true;
}

void
sc_keymap_processor_destroy(struct sc_keymap_processor *kmp) {
    // Nothing to clean up - we don't own the wrapped objects
    (void) kmp;
}