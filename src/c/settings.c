#include "settings.h"

static HcSettings s_settings;

void hc_settings_init(void) {
  hc_settings_load(&s_settings);
}

HcSettings *hc_settings(void) {
  return &s_settings;
}

void hc_settings_persist(void) {
  s_settings.version = HC_SETTINGS_VERSION;
  persist_write_data(HC_SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

static uint8_t clamp_u8(int32_t v, int lo, int hi) {
  if (v < lo) {
    v = lo;
  }
  if (v > hi) {
    v = hi;
  }
  return (uint8_t) v;
}

bool hc_settings_apply_inbox(DictionaryIterator *iter) {
  bool changed = false;
  Tuple *t;

#define HC_GET(key) (t = dict_find(iter, MESSAGE_KEY_##key))

  if (HC_GET(CFG_ENABLED))      { s_settings.enabled      = t->value->int32 != 0;                     changed = true; }
  if (HC_GET(CFG_MODE))         { s_settings.mode         = t->value->int32 ? HC_MODE_WORKER
                                                                           : HC_MODE_WAKEUP;         changed = true; }
  if (HC_GET(CFG_SOUND))        { s_settings.sound        = t->value->int32 != 0;                     changed = true; }
  if (HC_GET(CFG_VIBE))         { s_settings.vibe         = t->value->int32 != 0;                     changed = true; }
  if (HC_GET(CFG_VOLUME))       { s_settings.volume       = clamp_u8(t->value->int32, 0, 100);        changed = true; }
  if (HC_GET(CFG_START_HOUR))   { s_settings.start_hour   = clamp_u8(t->value->int32, 0, 23);         changed = true; }
  if (HC_GET(CFG_END_HOUR))     { s_settings.end_hour     = clamp_u8(t->value->int32, 0, 23);         changed = true; }
  if (HC_GET(CFG_STYLE))        { s_settings.style        = clamp_u8(t->value->int32, 0,
                                                                    HC_STYLE_COUNT - 1);             changed = true; }
  if (HC_GET(CFG_STRIKE_COUNT)) { s_settings.strike_count = clamp_u8(t->value->int32, 0, 12);         changed = true; }

#undef HC_GET

  if (changed) {
    hc_settings_persist();
  }
  return changed;
}
