# Hourly Chime

A PebbleOS watchapp that chimes on the hour with the speaker and/or the
vibration motor. Targets **Pebble Time 2 (`emery`)** and **Pebble 2 Duo
(`flint`)** only — the two watches with a speaker.

## How it stays alive

There are two trigger backends, selectable on the config page:

| Mode | How it works | Notes |
|------|--------------|-------|
| **Wakeup** (default) | The app schedules a single wakeup for the next active top-of-hour. When it fires, the app is relaunched, plays the chime, reschedules the next hour, and exits. | No resident process, no battery cost between chimes. Uses 1 of the app's 8 wakeup slots. The screen briefly shows the app each hour. |
| **Background worker** (optional) | `worker_src/c/worker.c` stays resident, watches for `tm_min == 0` via `TickTimerService`, and calls `worker_launch_app()`. The app then chimes and exits. | The watch allows **only one** background worker at a time — enabling this replaces any step/sleep tracker's worker. The worker SDK has no speaker/vibe APIs, which is why it must launch the app. |

Both paths converge on the foreground app doing the actual chime, because
only the foreground app can drive `speaker_*` and `vibes_*`.

## Chime styles

`Single beep`, `Westminster` (first phrase of the Westminster Quarters),
`Cuckoo`, and `Hour strikes` (N strikes/buzzes, N = a fixed count or the
current 12-hour hour).

## Settings

Configured from the Pebble phone app (**Settings → Hourly Chime**). The
config page is self-contained (a `data:` URL, no hosting, no Clay). Values
are stored on the watch in persistent storage and shared with the worker.

- Enabled
- Trigger mode (wakeup / worker)
- Speaker on/off, vibration on/off, volume
- Sound style + strike count
- Active window (`start_hour`–`end_hour`, inclusive; set start > end to span midnight)

In-app buttons: **SELECT** toggles enabled, **long-press SELECT** plays a test chime.

## Build & run

```sh
pebble build
pebble install --emulator emery      # or: --phone <ip>
pebble logs --emulator emery
```

The emulator has no speaker, so audio can only be verified on real
hardware; wakeup scheduling, the worker, and the UI all work in the emulator.

## Layout

```
src/c/chime_shared.h   settings struct + helpers, shared with the worker (header-only)
src/c/settings.[ch]    persistence + AppMessage config parsing
src/c/chime.[ch]       vibration patterns + speaker melodies
src/c/scheduler.[ch]   wakeup scheduling / worker lifecycle
src/c/main.c           app entry, UI, launch-reason handling
worker_src/c/worker.c  background worker
src/pkjs/index.js      config page + AppMessage delivery
```

## SDK

Built against Pebble SDK `4.33.1` (Core Devices fork). The Speaker API
(`speaker_play_notes`, `speaker_play_tone`, PCM streaming) requires SDK 4.9+
and speaker hardware.
