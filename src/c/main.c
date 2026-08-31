#include <pebble.h>
#include "chime_shared.h"
#include "settings.h"
#include "chime.h"
#include "scheduler.h"

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_info_layer;
static AppTimer *s_exit_timer;

static char s_time_buf[16];
static char s_info_buf[96];

/* True when this launch was triggered by a wakeup or by the worker, i.e. we
 * should chime and then get out of the way rather than show the UI. */
static bool s_chime_launch;

/* --- exit plumbing (chime-launch only) ------------------------------- */

static void hc_do_exit(void *ctx) {
  s_exit_timer = NULL;
  window_stack_pop_all(false);        /* empties the stack -> app exits */
}

static void hc_arm_exit(uint32_t after_ms) {
  if (s_exit_timer) {
    app_timer_cancel(s_exit_timer);
  }
  s_exit_timer = app_timer_register(after_ms, hc_do_exit, NULL);
}

static void hc_on_speaker_finished(SpeakerFinishReason reason, void *ctx) {
  hc_arm_exit(500);
}

/* --- UI (user-launch only) ------------------------------------------ */

static void hc_render_info(void) {
  HcSettings *s = hc_settings();

  if (!s->enabled) {
    snprintf(s_info_buf, sizeof(s_info_buf), "Chime is OFF\n\nSELECT to enable");
    text_layer_set_text(s_info_layer, s_info_buf);
    return;
  }

  time_t next = hc_next_chime_time(s);
  struct tm *lt = localtime(&next);
  char when[16];
  strftime(when, sizeof(when), clock_is_24h_style() ? "%H:%M" : "%I:%M %p", lt);

  const char *out =
      s->sound ? (s->vibe ? "sound + vibe" : "sound only")
               : (s->vibe ? "vibe only" : "silent");

  snprintf(s_info_buf, sizeof(s_info_buf), "Next: %s\n%s\nactive %02d:00-%02d:00\n%s mode",
           when, out, s->start_hour, s->end_hour,
           s->mode == HC_MODE_WORKER ? "worker" : "wakeup");
  text_layer_set_text(s_info_layer, s_info_buf);
}

static void hc_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);

  s_time_layer = text_layer_create(GRect(0, b.size.h / 2 - 66, b.size.w, 52));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_time_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  s_info_layer = text_layer_create(GRect(6, b.size.h / 2, b.size.w - 12, b.size.h / 2 - 6));
  text_layer_set_font(s_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_info_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_info_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_info_layer));

  time_t now = time(NULL);
  struct tm *lt = localtime(&now);
  strftime(s_time_buf, sizeof(s_time_buf), clock_is_24h_style() ? "%H:%M" : "%I:%M", lt);
  text_layer_set_text(s_time_layer, s_time_buf);

  if (s_chime_launch) {
    text_layer_set_text(s_info_layer, "chiming\xE2\x80\xA6");  /* "chiming…" */
    bool sound = hc_chime_play(hc_settings(), lt->tm_hour, false);
    if (sound) {
      speaker_set_finish_callback(hc_on_speaker_finished, NULL);
      hc_arm_exit(9000);             /* fallback if the callback never lands */
    } else {
      hc_arm_exit(2500);
    }
  } else {
    hc_render_info();
    light_enable_interaction();
  }
}

static void hc_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_info_layer);
}

/* --- config from the phone ----------------------------------------- */

static void hc_inbox_received(DictionaryIterator *iter, void *context) {
  if (hc_settings_apply_inbox(iter)) {
    hc_scheduler_apply(hc_settings());
    if (!s_chime_launch) {
      hc_render_info();
    }
  }
}

/* --- buttons (user-launch only) ----------------------------------- */

static void hc_select_click(ClickRecognizerRef rec, void *ctx) {
  HcSettings *s = hc_settings();
  s->enabled = !s->enabled;
  hc_settings_persist();
  hc_scheduler_apply(s);
  hc_render_info();
  vibes_short_pulse();
}

static void hc_select_long(ClickRecognizerRef rec, void *ctx) {
  time_t now = time(NULL);
  struct tm *lt = localtime(&now);
  hc_chime_play(hc_settings(), lt->tm_hour, true);  /* test chime */
}

static void hc_click_config(void *ctx) {
  window_single_click_subscribe(BUTTON_ID_SELECT, hc_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, 600, hc_select_long, NULL);
}

/* --- lifecycle --------------------------------------------------- */

static void hc_init(void) {
  hc_settings_init();

  app_message_register_inbox_received(hc_inbox_received);
  app_message_open(256, 64);

  AppLaunchReason reason = launch_reason();
  s_chime_launch = (reason == APP_LAUNCH_WAKEUP || reason == APP_LAUNCH_WORKER);

  /* Consume the wakeup launch event, if there was one. */
  WakeupId wid;
  int32_t cookie;
  wakeup_get_launch_event(&wid, &cookie);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = hc_window_load,
    .unload = hc_window_unload,
  });
  if (!s_chime_launch) {
    window_set_click_config_provider(s_window, hc_click_config);
  }
  window_stack_push(s_window, !s_chime_launch /* animated */);

  if (s_chime_launch) {
    hc_scheduler_after_chime(hc_settings());  /* arm the next hour */
  } else {
    hc_scheduler_apply(hc_settings());        /* reconcile backend with settings */
  }
}

static void hc_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  hc_init();
  app_event_loop();
  hc_deinit();
}
