/*
 * Background worker for Hourly Chime (optional; enabled via the config page).
 *
 * The worker keeps running after the app is closed. It cannot touch the
 * speaker or the vibration motor (those APIs are not in the worker SDK), so
 * its only job is to notice the top of the hour and launch the foreground
 * app, which then performs the chime and exits.
 *
 * Trade-offs vs. the default Wakeup mode:
 *   + always resident, so it can never "run out" of scheduled wakeup slots
 *   - only one background worker may run on the watch at a time, so enabling
 *     this disables any other worker app (step counter, sleep tracker, ...)
 */
#include <pebble_worker.h>
#include "../../src/c/chime_shared.h"

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (tick_time->tm_min != 0) {
    return;
  }

  HcSettings s;
  hc_settings_load(&s);

  if (!s.enabled || s.mode != HC_MODE_WORKER) {
    return;
  }
  if (!hc_is_active_hour(&s, tick_time->tm_hour)) {
    return;
  }

  worker_launch_app();
}

int main(void) {
  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
  worker_event_loop();
  tick_timer_service_unsubscribe();
}
