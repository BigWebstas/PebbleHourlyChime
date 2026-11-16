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
#define HC_SETTINGS_VERSION  2

typedef enum {
  HC_MODE_WAKEUP = 0,  /* Wakeup API reschedules an app launch each hour. */
  HC_MODE_WORKER = 1,  /* Background worker relaunches the app each hour.  */
} HcMode;

typedef enum {
  HC_DAYS_ALL      = 0,  /* chime every day */
  HC_DAYS_WEEKDAYS = 1,  /* Mon-Fri only */
  HC_DAYS_WEEKENDS = 2,  /* Sat-Sun only */
} HcDays;

typedef enum {
  HC_STYLE_BEEP        = 0,
  HC_STYLE_WESTMINSTER = 1,
  HC_STYLE_CUCKOO      = 2,
  HC_STYLE_STRIKE      = 3,  /* N strikes, N = strike_count or the 12h hour. */
  HC_STYLE_MARIO       = 4,  /* Opening phrase of the Super Mario Bros theme. */
  HC_STYLE_STARWARS    = 5,  /* Opening bars of the Star Wars main title. */
  HC_STYLE_SPONGEBOB   = 6,  /* "Who lives in a pineapple under the sea?" */
  HC_STYLE_XFILES      = 7,  /* The X-Files main theme whistled chorus. */
  HC_STYLE_BATMAN      = 8,  /* Neal Hefti's 1960s Batman TV theme riff. */
  HC_STYLE_CASIO       = 9,  /* Casio F-91W hourly signal: two short beeps. */
  HC_STYLE_NOKIA       = 10, /* Nokia tune: the "Gran Vals" phrase. */
  HC_STYLE_TETRIS      = 11, /* Korobeiniki: the Tetris Type A theme. */
  HC_STYLE_FOX                = 12, /* 20th Century Fox fanfare hook. */
  HC_STYLE_MISSION_IMPOSSIBLE = 13, /* Lalo Schifrin's Mission: Impossible theme hook. */
  HC_STYLE_INDIANA_JONES      = 14, /* John Williams' Raiders March (Indiana Jones) hook. */
  HC_STYLE_FUR_ELISE          = 15, /* Beethoven's Für Elise opening phrase. */
  HC_STYLE_ODE_TO_JOY         = 16, /* Beethoven's Ode to Joy (Symphony No. 9) opening phrase. */
  HC_STYLE_REVEILLE           = 17, /* Traditional military bugle call Reveille. */
  HC_STYLE_BEETHOVEN_5TH      = 18, /* Beethoven's Symphony No. 5 "Fate" motif. */
  HC_STYLE_NACHTMUSIK         = 19, /* Mozart's Eine kleine Nachtmusik opening fanfare. */
  HC_STYLE_WILLIAM_TELL       = 20, /* Rossini's William Tell Overture finale gallop. */
  HC_STYLE_TOCCATA            = 21, /* Bach's Toccata and Fugue in D minor opening. */
  HC_STYLE_MOUNTAIN_KING      = 22, /* Grieg's In the Hall of the Mountain King hook. */
  HC_STYLE_VALKYRIES          = 23, /* Wagner's Ride of the Valkyries fanfare. */
  HC_STYLE_SPRING             = 24, /* Vivaldi's Spring (The Four Seasons) opening. */
  HC_STYLE_TOREADOR           = 25, /* Bizet's Toreador Song (Carmen) refrain. */
  HC_STYLE_HABANERA           = 26, /* Bizet's Habanera (Carmen) chromatic hook. */
  HC_STYLE_SUGAR_PLUM         = 27, /* Tchaikovsky's Dance of the Sugar Plum Fairy. */
  HC_STYLE_FIRST_CALL         = 28, /* Traditional "Call to the Post" bugle fanfare. */
  HC_STYLE_RANDOM             = 29, /* Pick a random chime style each time. */
  /* Holiday chimes. Appended after RANDOM so saved settings keep their IDs. */
  HC_STYLE_JINGLE_BELLS       = 30, /* Jingle Bells chorus. */
  HC_STYLE_DECK_THE_HALLS     = 31, /* Deck the Halls opening verse. */
  HC_STYLE_MERRY_CHRISTMAS    = 32, /* We Wish You a Merry Christmas. */
  HC_STYLE_SILENT_NIGHT       = 33, /* Silent Night opening verse. */
  HC_STYLE_JOY_TO_THE_WORLD   = 34, /* Joy to the World opening line. */
  HC_STYLE_AULD_LANG_SYNE     = 35, /* Auld Lang Syne (New Year's Eve). */
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
  uint8_t days;          /* HcDays: which days of the week to chime */
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
  s->days         = HC_DAYS_ALL;
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

/* True if the chime should sound on the given weekday (tm_wday: 0=Sun..6=Sat).
 * A midnight-spanning window is judged by the calendar day it actually falls on,
 * not by the day the window opened. */
static inline bool hc_is_active_day(const HcSettings *s, int wday) {
  bool weekend = (wday == 0 || wday == 6);
  switch (s->days) {
    case HC_DAYS_WEEKDAYS: return !weekend;
    case HC_DAYS_WEEKENDS: return weekend;
    default:               return true;
  }
}

/* True if the chime should sound at the given hour on the given weekday. */
static inline bool hc_is_active_time(const HcSettings *s, int hour24, int wday) {
  return hc_is_active_day(s, wday) && hc_is_active_hour(s, hour24);
}

/* Number of strikes/beeps to play for the STRIKE style. */
static inline int hc_strike_count(const HcSettings *s, int hour24) {
  if (s->strike_count > 0) {
    return s->strike_count;
  }
  int h = hour24 % 12;
  return h == 0 ? 12 : h;
}
