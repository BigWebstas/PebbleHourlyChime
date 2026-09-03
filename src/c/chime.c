#include "chime.h"

/* --- vibration ---------------------------------------------------------- */

static void hc_do_vibe(const HcSettings *s, int strikes) {
  if (!s->vibe) {
    return;
  }

  if (s->style == HC_STYLE_STRIKE) {
    /* One short buzz per strike. vibes_enqueue_custom_pattern copies the
     * pattern, but keep the buffer static to be safe. */
    static uint32_t segments[24];
    uint32_t n = 0;
    for (int i = 0; i < strikes && i < 12; i++) {
      segments[n++] = 100;              /* buzz */
      if (i != strikes - 1) {
        segments[n++] = 220;            /* gap */
      }
    }
    VibePattern pattern = { .durations = segments, .num_segments = n };
    vibes_enqueue_custom_pattern(pattern);
    return;
  }

  vibes_double_pulse();
}

/* --- speaker ----------------------------------------------------------- */

static bool hc_play_sound(const HcSettings *s, int strikes) {
  if (!s->sound || speaker_is_muted()) {
    return false;
  }

  const uint8_t vol = s->volume;

  switch (s->style) {
    case HC_STYLE_WESTMINSTER: {
      /* First phrase of the Westminster Quarters (change ring). */
      static const SpeakerNote notes[] = {
        { 92, SpeakerWaveformSine, 380, 0, 0 },  /* G#6 */
        { 90, SpeakerWaveformSine, 380, 0, 0 },  /* F#6 */
        { 88, SpeakerWaveformSine, 380, 0, 0 },  /* E6  */
        { 83, SpeakerWaveformSine, 720, 0, 0 },  /* B5  */
      };
      return speaker_play_notes(notes, 4, vol);
    }

    case HC_STYLE_CUCKOO: {
      static const SpeakerNote notes[] = {
        { 96, SpeakerWaveformTriangle, 220, 0, 0 },  /* C7 */
        { 91, SpeakerWaveformTriangle, 340, 0, 0 },  /* G6 */
        {  0, SpeakerWaveformSine,     140, 0, 0 },  /* rest */
        { 96, SpeakerWaveformTriangle, 220, 0, 0 },
        { 91, SpeakerWaveformTriangle, 340, 0, 0 },
      };
      return speaker_play_notes(notes, 5, vol);
    }

    case HC_STYLE_STRIKE: {
      static SpeakerNote notes[24];
      uint32_t n = 0;
      for (int i = 0; i < strikes && i < 12; i++) {
        notes[n++] = (SpeakerNote){ 79, SpeakerWaveformSine, 280, 0, 0 };  /* G5 */
        if (i != strikes - 1) {
          notes[n++] = (SpeakerNote){ 0, SpeakerWaveformSine, 260, 0, 0 };
        }
      }
      if (n == 0) {
        return false;
      }
      return speaker_play_notes(notes, n, vol);
    }

    case HC_STYLE_BEEP:
    default:
      return speaker_play_tone(3520, 180, vol, SpeakerWaveformSine);
  }
}

/* --- public ---------------------------------------------------------- */

bool hc_chime_play(const HcSettings *s, int hour24, bool force) {
  if (!force && (!s->enabled || !hc_is_active_hour(s, hour24))) {
    return false;
  }

  const int strikes = hc_strike_count(s, hour24);
  hc_do_vibe(s, strikes);
  bool sound = hc_play_sound(s, strikes);
  APP_LOG(APP_LOG_LEVEL_INFO, "chime: hour=%d style=%d strikes=%d vibe=%d sound=%d muted=%d",
          hour24, s->style, strikes, s->vibe, sound, speaker_is_muted());
  return sound;
}
