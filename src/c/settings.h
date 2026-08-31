#pragma once

#include <pebble.h>
#include "chime_shared.h"

/* Loads settings from persistent storage into module state. */
void hc_settings_init(void);

/* Pointer to the live settings (never NULL after hc_settings_init). */
HcSettings *hc_settings(void);

/* Writes the live settings back to persistent storage. */
void hc_settings_persist(void);

/*
 * Applies any CFG_* keys present in an AppMessage inbox dict to the live
 * settings, persisting if anything changed. Returns true if a value changed.
 */
bool hc_settings_apply_inbox(DictionaryIterator *iter);
