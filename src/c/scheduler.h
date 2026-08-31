#pragma once

#include <pebble.h>
#include "chime_shared.h"

/* Cookie passed to wakeup_schedule so we can recognise our own events. */
#define HC_WAKEUP_COOKIE 0x6863686D  /* "hchm" */

/* Next wall-clock time (UTC time_t) at which a chime should fire. */
time_t hc_next_chime_time(const HcSettings *s);

/*
 * Reconciles the scheduling backend with the current settings:
 *   - disabled            -> cancel wakeups, kill worker
 *   - enabled, wakeup mode -> kill worker, schedule the next wakeup
 *   - enabled, worker mode -> cancel wakeups, launch the worker
 */
void hc_scheduler_apply(const HcSettings *s);

/* Call right after a chime has fired to arm the next one (wakeup mode only). */
void hc_scheduler_after_chime(const HcSettings *s);
