/*
 * Shared, header-only settings model.
 *
 * This file is included by BOTH the foreground app (which includes <pebble.h>)
 * and the background worker (which includes <pebble_worker.h>). It may only use
 * APIs that exist in both SDKs: the C library plus the persistent-storage API.
 * Keep everything here dependency-free and `static inline`.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Persistent storage slot holding a serialised HcSettings struct. */
#define HC_SETTINGS_KEY      1
#define HC_SETTINGS_VERSION  1

typedef enum {
  HC_MODE_WAKEUP = 0,  /* Wakeup API reschedules an app launch each hour. */
  HC_MODE_WORKER = 1,  /* Background worker relaunches the app each hour.  */
} HcMode;

typedef enum {
  HC_STYLE_BEEP        = 0,
  HC_STYLE_WESTMINSTER = 1,
  HC_STYLE_CUCKOO      = 2,
  HC_STYLE_STRIKE      = 3,  /* N strikes, N = strike_count or the 12h hour. */
  HC_STYLE_COUNT
} HcStyle;

typedef struct {
  uint8_t version;
  bool    enabled;
  uint8_t mode;          /* HcMode */
  bool    sound;         /* use the speaker */
  bool    vibe;          /* use the vibration motor */
  uint8_t volume;        /* 0..100 */
  uint8_t start_hour;    /* 0..23, inclusive */
  uint8_t end_hour;      /* 0..23, inclusive; < start_hour means wrap midnight */
  uint8_t style;         /* HcStyle */
  uint8_t strike_count;  /* 0 = strike the current hour (12h), else fixed count */
} HcSettings;

static inline void hc_settings_defaults(HcSettings *s) {
  memset(s, 0, sizeof(*s));
  s->version      = HC_SETTINGS_VERSION;
  s->enabled      = true;
  s->mode         = HC_MODE_WAKEUP;
  s->sound        = true;
  s->vibe         = true;
  s->volume       = 70;
  s->start_hour   = 8;
  s->end_hour     = 22;
  s->style        = HC_STYLE_WESTMINSTER;
  s->strike_count = 0;
}

static inline void hc_settings_load(HcSettings *s) {
  hc_settings_defaults(s);
  if (persist_exists(HC_SETTINGS_KEY)) {
    HcSettings tmp;
    int n = persist_read_data(HC_SETTINGS_KEY, &tmp, sizeof(tmp));
    if (n == (int) sizeof(tmp) && tmp.version == HC_SETTINGS_VERSION) {
      *s = tmp;
    }
  }
}

/* True if the hourly chime should sound at the given 24h hour. */
static inline bool hc_is_active_hour(const HcSettings *s, int hour24) {
  int a = s->start_hour;
  int b = s->end_hour;
  if (a == b) {
    return true;                        /* all day */
  }
  if (a < b) {
    return hour24 >= a && hour24 <= b;  /* same-day window */
  }
  return hour24 >= a || hour24 <= b;    /* window wraps past midnight */
}

/* Number of strikes/beeps to play for the STRIKE style. */
static inline int hc_strike_count(const HcSettings *s, int hour24) {
  if (s->strike_count > 0) {
    return s->strike_count;
  }
  int h = hour24 % 12;
  return h == 0 ? 12 : h;
}
