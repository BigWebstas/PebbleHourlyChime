#include "scheduler.h"

time_t hc_next_chime_time(const HcSettings *s) {
  time_t now = time(NULL);
  struct tm tm_next = *localtime(&now);
  tm_next.tm_sec = 0;
  tm_next.tm_min = 0;
  tm_next.tm_hour += 1;               /* top of the next hour */
  time_t candidate = mktime(&tm_next);

  /* Skip forward to the next hour that is inside the active window. A
   * weekends-only chime can be up to a week away, so scan a full week. */
  for (int i = 0; i < 24 * 7 + 2; i++) {
    struct tm c = *localtime(&candidate);
    if (hc_is_active_time(s, c.tm_hour, c.tm_wday)) {
      return candidate;
    }
    candidate += 3600;
  }
  return candidate;
}

static void hc_arm_wakeup(const HcSettings *s) {
  wakeup_cancel_all();
  if (!s->enabled || s->mode != HC_MODE_WAKEUP) {
    return;
  }

  time_t t = hc_next_chime_time(s);
  if (t < time(NULL) + 60) {
    t += 3600;                        /* too close to "now" to schedule */
  }

  for (int tries = 0; tries < 6; tries++) {
    WakeupId id = wakeup_schedule(t, HC_WAKEUP_COOKIE, true);
    if (id >= 0) {
      return;
    }
    if (id == E_RANGE) {
      t += 120;                       /* collides with a system wakeup window */
      continue;
    }
    if (id == E_INVALID_ARGUMENT) {
      t += 3600;                      /* somehow in the past; try next hour */
      continue;
    }
    break;                            /* E_OUT_OF_RESOURCES / E_INTERNAL */
  }
}

void hc_scheduler_apply(const HcSettings *s) {
  if (s->enabled && s->mode == HC_MODE_WORKER) {
    wakeup_cancel_all();
    if (!app_worker_is_running()) {
      app_worker_launch();
    }
    return;
  }

  if (app_worker_is_running()) {
    app_worker_kill();
  }
  hc_arm_wakeup(s);
}

void hc_scheduler_after_chime(const HcSettings *s) {
  if (s->enabled && s->mode == HC_MODE_WAKEUP) {
    hc_arm_wakeup(s);
  }
}
