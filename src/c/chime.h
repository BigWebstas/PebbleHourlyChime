#pragma once

#include <pebble.h>
#include "chime_shared.h"

/*
 * Plays the chime: vibration pattern and/or speaker melody, according to the
 * given settings and the current 24h hour.
 *
 * When `force` is false the chime is skipped unless it is enabled and `hour24`
 * falls inside the active window (used for the real hourly trigger). When
 * `force` is true it always plays (used for the in-app "test" action).
 *
 * Returns true if speaker playback was started, in which case the caller may
 * rely on a speaker_set_finish_callback() firing.
 */
bool hc_chime_play(const HcSettings *s, int hour24, bool force);
