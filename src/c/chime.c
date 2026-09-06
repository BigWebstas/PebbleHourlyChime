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

    case HC_STYLE_MARIO: {
      /* Opening phrase of the Super Mario Bros. overworld theme. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 125, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,   125, 0, 0 },
        { 76, SpeakerWaveformSquare, 125, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,   125, 0, 0 },
        { 76, SpeakerWaveformSquare, 125, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,   125, 0, 0 },
        { 72, SpeakerWaveformSquare, 125, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 125, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,   125, 0, 0 },
        { 79, SpeakerWaveformSquare, 250, 0, 0 },  /* G5 */
        {  0, SpeakerWaveformSine,   250, 0, 0 },
        { 67, SpeakerWaveformSquare, 250, 0, 0 },  /* G4 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_STARWARS: {
      /* Opening bars of the Star Wars main title. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 triplet */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 520, 0, 0 },  /* C5 */
        { 79, SpeakerWaveformSquare, 520, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 130, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 130, 0, 0 },  /* D5 */
        { 84, SpeakerWaveformSquare, 520, 0, 0 },  /* C6 */
        { 79, SpeakerWaveformSquare, 300, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 130, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 130, 0, 0 },  /* D5 */
        { 84, SpeakerWaveformSquare, 520, 0, 0 },  /* C6 */
        { 79, SpeakerWaveformSquare, 520, 0, 0 },  /* G5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_SPONGEBOB: {
      /* "Who lives in a pineapple under the sea?" */
      static const SpeakerNote notes[] = {
        { 72, SpeakerWaveformTriangle, 190, 0, 0 },  /* C5  Who   */
        { 72, SpeakerWaveformTriangle, 190, 0, 0 },  /* C5  lives */
        { 72, SpeakerWaveformTriangle, 190, 0, 0 },  /* C5  in a  */
        { 77, SpeakerWaveformTriangle, 190, 0, 0 },  /* F5  pine  */
        { 77, SpeakerWaveformTriangle, 190, 0, 0 },  /* F5  ap    */
        { 77, SpeakerWaveformTriangle, 190, 0, 0 },  /* F5  ple   */
        { 81, SpeakerWaveformTriangle, 190, 0, 0 },  /* A5  un    */
        { 81, SpeakerWaveformTriangle, 190, 0, 0 },  /* A5  der   */
        { 81, SpeakerWaveformTriangle, 190, 0, 0 },  /* A5  the   */
        { 79, SpeakerWaveformTriangle, 240, 0, 0 },  /* G5  sea   */
        {  0, SpeakerWaveformSine,      90, 0, 0 },
        { 84, SpeakerWaveformTriangle, 420, 0, 0 },  /* C6  !     */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_XFILES: {
      /* The X-Files main theme — the whistled hook (A minor): the rising
       * A-C-E-F figure, then the three repeated high A's. */
      static const SpeakerNote notes[] = {
        { 69, SpeakerWaveformSine, 130, 0, 0 },  /* A4 */
        { 72, SpeakerWaveformSine, 130, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSine, 130, 0, 0 },  /* E5 */
        { 77, SpeakerWaveformSine, 380, 0, 0 },  /* F5 */
        {  0, SpeakerWaveformSine, 120, 0, 0 },
        { 72, SpeakerWaveformSine, 130, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSine, 130, 0, 0 },  /* E5 */
        { 77, SpeakerWaveformSine, 380, 0, 0 },  /* F5 */
        {  0, SpeakerWaveformSine, 160, 0, 0 },
        { 81, SpeakerWaveformSine, 300, 0, 0 },  /* A5 */
        {  0, SpeakerWaveformSine,  90, 0, 0 },
        { 81, SpeakerWaveformSine, 300, 0, 0 },  /* A5 */
        {  0, SpeakerWaveformSine,  90, 0, 0 },
        { 81, SpeakerWaveformSine, 540, 0, 0 },  /* A5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_BATMAN: {
      /* Neal Hefti's 1966 Batman TV theme: the chromatic surf riff twice,
       * then the sung "Batman!" (flat third down to the tonic). */
      static const SpeakerNote notes[] = {
        { 72, SpeakerWaveformSquare,  90, 0, 0 },  /* C5  */
        { 73, SpeakerWaveformSquare,  90, 0, 0 },  /* C#5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5  */
        { 75, SpeakerWaveformSquare,  90, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare,  90, 0, 0 },  /* D#5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5  */
        { 73, SpeakerWaveformSquare,  90, 0, 0 },  /* C#5 */
        { 72, SpeakerWaveformSquare, 220, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,   120, 0, 0 },
        { 72, SpeakerWaveformSquare,  90, 0, 0 },  /* C5  */
        { 73, SpeakerWaveformSquare,  90, 0, 0 },  /* C#5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5  */
        { 75, SpeakerWaveformSquare,  90, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare,  90, 0, 0 },  /* D#5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5  */
        { 73, SpeakerWaveformSquare,  90, 0, 0 },  /* C#5 */
        { 72, SpeakerWaveformSquare, 220, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,   160, 0, 0 },
        { 75, SpeakerWaveformSquare, 240, 0, 0 },  /* D#5  Bat- */
        { 72, SpeakerWaveformSquare, 560, 0, 0 },  /* C5   -man! */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_CASIO: {
      /* The Casio F-91W hourly signal: two short high beeps. */
      static const SpeakerNote notes[] = {
        { 96, SpeakerWaveformSquare, 70, 0, 0 },  /* C7 */
        {  0, SpeakerWaveformSine,   70, 0, 0 },
        { 96, SpeakerWaveformSquare, 70, 0, 0 },  /* C7 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_NOKIA: {
      /* Nokia tune — the "Gran Vals" phrase (Tarrega), square wave. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 140, 0, 0 },  /* E5  */
        { 74, SpeakerWaveformSquare, 140, 0, 0 },  /* D5  */
        { 66, SpeakerWaveformSquare, 280, 0, 0 },  /* F#4 */
        { 68, SpeakerWaveformSquare, 280, 0, 0 },  /* G#4 */
        { 73, SpeakerWaveformSquare, 140, 0, 0 },  /* C#5 */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4  */
        { 62, SpeakerWaveformSquare, 280, 0, 0 },  /* D4  */
        { 64, SpeakerWaveformSquare, 280, 0, 0 },  /* E4  */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4  */
        { 69, SpeakerWaveformSquare, 140, 0, 0 },  /* A4  */
        { 61, SpeakerWaveformSquare, 280, 0, 0 },  /* C#4 */
        { 64, SpeakerWaveformSquare, 280, 0, 0 },  /* E4  */
        { 69, SpeakerWaveformSquare, 560, 0, 0 },  /* A4  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_TETRIS: {
      /* Korobeiniki — the Tetris (Type A) theme, opening phrase. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 280, 0, 0 },  /* E5 */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 74, SpeakerWaveformSquare, 280, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4 */
        { 69, SpeakerWaveformSquare, 280, 0, 0 },  /* A4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 69, SpeakerWaveformSquare, 140, 0, 0 },  /* A4 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 280, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 140, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 71, SpeakerWaveformSquare, 420, 0, 0 },  /* B4 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 74, SpeakerWaveformSquare, 280, 0, 0 },  /* D5 */
        { 76, SpeakerWaveformSquare, 280, 0, 0 },  /* E5 */
        { 72, SpeakerWaveformSquare, 280, 0, 0 },  /* C5 */
        { 69, SpeakerWaveformSquare, 280, 0, 0 },  /* A4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 69, SpeakerWaveformSquare, 420, 0, 0 },  /* A4 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
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
