#include <stdlib.h>
#include "chime.h"

static HcStyle hc_resolve_style(HcStyle style) {
  if (style != HC_STYLE_RANDOM) {
    return style;
  }
  static bool s_seeded = false;
  if (!s_seeded) {
    time_t sec;
    uint16_t ms;
    time_ms(&sec, &ms);
    srand((unsigned int)(sec ^ ((uint32_t)ms << 16)));
    s_seeded = true;
  }
  /* Pick any style except RANDOM itself (it sits mid-enum to keep saved IDs stable). */
  int pick = rand() % (HC_STYLE_COUNT - 1);
  if (pick >= HC_STYLE_RANDOM) {
    pick++;
  }
  return (HcStyle)pick;
}

/* --- vibration ---------------------------------------------------------- */

static void hc_do_vibe(const HcSettings *s, HcStyle style, int strikes) {
  if (!s->vibe) {
    return;
  }

  if (style == HC_STYLE_STRIKE) {
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

static bool hc_play_sound(const HcSettings *s, HcStyle style, int strikes) {
  if (!s->sound || speaker_is_muted()) {
    return false;
  }

  const uint8_t vol = s->volume;

  switch (style) {
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
      /* The X-Files main theme — the iconic whistled chorus (A minor):
       * A4-E5-D5-E5-G5-E5, then A4-E5-D5-E5-A5-E5. */
      static const SpeakerNote notes[] = {
        { 69, SpeakerWaveformSine, 220, 0, 0 },  /* A4 */
        { 76, SpeakerWaveformSine, 220, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSine, 220, 0, 0 },  /* D5 */
        { 76, SpeakerWaveformSine, 220, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSine, 220, 0, 0 },  /* G5 */
        { 76, SpeakerWaveformSine, 550, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine, 150, 0, 0 },  /* rest */
        { 69, SpeakerWaveformSine, 220, 0, 0 },  /* A4 */
        { 76, SpeakerWaveformSine, 220, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSine, 220, 0, 0 },  /* D5 */
        { 76, SpeakerWaveformSine, 220, 0, 0 },  /* E5 */
        { 81, SpeakerWaveformSine, 220, 0, 0 },  /* A5 */
        { 76, SpeakerWaveformSine, 700, 0, 0 },  /* E5 */
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
      /* Korobeiniki — the Tetris (Type A) theme, opening hook resolved to A4. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 280, 0, 0 },  /* E5 */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 74, SpeakerWaveformSquare, 280, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 71, SpeakerWaveformSquare, 140, 0, 0 },  /* B4 */
        { 69, SpeakerWaveformSquare, 560, 0, 0 },  /* A4 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_FOX: {
      /* Alfred Newman's 20th Century Fox fanfare hook:
       * three rising flourishes punctuated by brass hits, ending in the grand fanfare cadence. */
      static const SpeakerNote notes[] = {
        /* Flourish 1: Bb major run up to G5 */
        { 70, SpeakerWaveformSquare,  75, 0, 0 },  /* Bb4 */
        { 74, SpeakerWaveformSquare,  75, 0, 0 },  /* D5  */
        { 77, SpeakerWaveformSquare,  75, 0, 0 },  /* F5  */
        { 79, SpeakerWaveformSquare, 260, 0, 0 },  /* G5  */
        /* Brass response 1 (Cm) */
        { 72, SpeakerWaveformSquare, 130, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,   110, 0, 0 },  /* rest */
        { 72, SpeakerWaveformSquare,  55, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 72, SpeakerWaveformSquare,  55, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 72, SpeakerWaveformSquare, 230, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,    30, 0, 0 },  /* rest between repeated C5 */
        /* Flourish 2: Cm run up to Bb5 */
        { 72, SpeakerWaveformSquare,  75, 0, 0 },  /* C5  */
        { 75, SpeakerWaveformSquare,  75, 0, 0 },  /* Eb5 */
        { 79, SpeakerWaveformSquare,  75, 0, 0 },  /* G5  */
        { 82, SpeakerWaveformSquare, 260, 0, 0 },  /* Bb5 */
        /* Brass response 2 (F7) */
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,   110, 0, 0 },  /* rest */
        { 77, SpeakerWaveformSquare,  55, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 77, SpeakerWaveformSquare,  55, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 77, SpeakerWaveformSquare, 260, 0, 0 },  /* F5  */
        /* Flourish 3: Diminished run up to Bb5 */
        { 72, SpeakerWaveformSquare,  80, 0, 0 },  /* C5  */
        { 75, SpeakerWaveformSquare,  80, 0, 0 },  /* Eb5 */
        { 78, SpeakerWaveformSquare,  80, 0, 0 },  /* F#5 */
        { 82, SpeakerWaveformSquare, 360, 0, 0 },  /* Bb5 */
        /* Grand cadence */
        { 77, SpeakerWaveformSquare, 110, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,    35, 0, 0 },  /* rest */
        { 74, SpeakerWaveformSquare, 110, 0, 0 },  /* D5  */
        {  0, SpeakerWaveformSine,    35, 0, 0 },  /* rest */
        { 75, SpeakerWaveformSquare, 110, 0, 0 },  /* Eb5 */
        {  0, SpeakerWaveformSine,    35, 0, 0 },  /* rest */
        { 77, SpeakerWaveformSquare, 750, 0, 0 },  /* F5  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_MISSION_IMPOSSIBLE: {
      /* Lalo Schifrin's Mission: Impossible theme — iconic 5/4 brass melody hook (<2s). */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 500, 0, 0 },  /* G4 */
        { 70, SpeakerWaveformSquare, 150, 0, 0 },  /* Bb4 */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 72, SpeakerWaveformSquare, 175, 0, 0 },  /* C5 */
        { 67, SpeakerWaveformSquare, 500, 0, 0 },  /* G4 */
        { 65, SpeakerWaveformSquare, 150, 0, 0 },  /* F4 */
        {  0, SpeakerWaveformSine,    25, 0, 0 },  /* rest */
        { 66, SpeakerWaveformSquare, 335, 0, 0 },  /* F#4 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_INDIANA_JONES: {
      /* John Williams' The Raiders March (Indiana Jones theme) — opening 7-note hook (<2s). */
      static const SpeakerNote notes[] = {
        { 64, SpeakerWaveformSquare,  85, 0, 0 },  /* E4 */
        { 65, SpeakerWaveformSquare,  85, 0, 0 },  /* F4 */
        { 67, SpeakerWaveformSquare, 200, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 500, 0, 0 },  /* C5 */
        {  0, SpeakerWaveformSine,    60, 0, 0 },  /* rest */
        { 62, SpeakerWaveformSquare,  85, 0, 0 },  /* D4 */
        { 64, SpeakerWaveformSquare,  85, 0, 0 },  /* E4 */
        { 65, SpeakerWaveformSquare, 700, 0, 0 },  /* F4 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_FUR_ELISE: {
      /* Beethoven's Bagatelle No. 25 in A minor ("Für Elise") — opening 9-note phrase. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare, 150, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare, 150, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5  */
        { 71, SpeakerWaveformSquare, 150, 0, 0 },  /* B4  */
        { 74, SpeakerWaveformSquare, 150, 0, 0 },  /* D5  */
        { 72, SpeakerWaveformSquare, 150, 0, 0 },  /* C5  */
        { 69, SpeakerWaveformSquare, 480, 0, 0 },  /* A4  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_ODE_TO_JOY: {
      /* Beethoven's Symphony No. 9 ("Ode to Joy") — opening 4-bar phrase in C major. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 200, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },  /* E5 */
        { 77, SpeakerWaveformSquare, 220, 0, 0 },  /* F5 */
        { 79, SpeakerWaveformSquare, 200, 0, 0 },  /* G5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare, 220, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 220, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 220, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare, 200, 0, 0 },  /* C5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },  /* C5 */
        { 74, SpeakerWaveformSquare, 220, 0, 0 },  /* D5 */
        { 76, SpeakerWaveformSquare, 200, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 330, 0, 0 },  /* E5. */
        { 74, SpeakerWaveformSquare, 110, 0, 0 },  /* D5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },  /* D5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_REVEILLE: {
      /* Traditional military bugle call "Reveille" ("I can't get 'em up..."). */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  I      */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  can't  */
        { 76, SpeakerWaveformSquare, 110, 0, 0 },  /* E5  get    */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  'em    */
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  up,    */
        { 76, SpeakerWaveformSquare, 180, 0, 0 },  /* E5         */
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  I      */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  can't  */
        { 76, SpeakerWaveformSquare, 110, 0, 0 },  /* E5  get    */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  'em    */
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  up,    */
        { 76, SpeakerWaveformSquare, 180, 0, 0 },  /* E5         */
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  I      */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  can't  */
        { 76, SpeakerWaveformSquare, 110, 0, 0 },  /* E5  get    */
        { 72, SpeakerWaveformSquare, 110, 0, 0 },  /* C5  'em    */
        { 67, SpeakerWaveformSquare, 130, 0, 0 },  /* G4  up     */
        { 76, SpeakerWaveformSquare, 200, 0, 0 },  /* E5  this   */
        { 72, SpeakerWaveformSquare, 200, 0, 0 },  /* C5  morn-  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 72, SpeakerWaveformSquare, 420, 0, 0 },  /* C5  -ing!  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_BEETHOVEN_5TH: {
      /* Beethoven's Symphony No. 5 in C minor — "Fate knocking at the door" motif. */
      static const SpeakerNote notes[] = {
        { 79, SpeakerWaveformSquare, 130, 0, 0 },  /* G5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 79, SpeakerWaveformSquare, 130, 0, 0 },  /* G5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 79, SpeakerWaveformSquare, 130, 0, 0 },  /* G5  */
        { 75, SpeakerWaveformSquare, 600, 0, 0 },  /* Eb5 */
        {  0, SpeakerWaveformSine,   160, 0, 0 },
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 77, SpeakerWaveformSquare, 130, 0, 0 },  /* F5  */
        { 74, SpeakerWaveformSquare, 700, 0, 0 },  /* D5  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_NACHTMUSIK: {
      /* Mozart's Eine kleine Nachtmusik (K. 525) — opening 2-bar Allegro fanfare. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 240, 0, 0 },  /* G4  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 62, SpeakerWaveformSquare, 120, 0, 0 },  /* D4  */
        { 67, SpeakerWaveformSquare, 240, 0, 0 },  /* G4  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 62, SpeakerWaveformSquare, 120, 0, 0 },  /* D4  */
        { 67, SpeakerWaveformSquare, 120, 0, 0 },  /* G4  */
        { 71, SpeakerWaveformSquare, 120, 0, 0 },  /* B4  */
        { 74, SpeakerWaveformSquare, 320, 0, 0 },  /* D5  */
        {  0, SpeakerWaveformSine,   110, 0, 0 },
        { 72, SpeakerWaveformSquare, 240, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 72, SpeakerWaveformSquare, 240, 0, 0 },  /* C5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 66, SpeakerWaveformSquare, 120, 0, 0 },  /* F#4 */
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 62, SpeakerWaveformSquare, 360, 0, 0 },  /* D4  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_WILLIAM_TELL: {
      /* Rossini's William Tell Overture finale — the "Lone Ranger" gallop. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    30, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    30, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 170, 0, 0 },  /* C5 */
        { 74, SpeakerWaveformSquare, 170, 0, 0 },  /* D5 */
        { 76, SpeakerWaveformSquare, 220, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    40, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare,  80, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 170, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 170, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 170, 0, 0 },  /* D5 */
        { 71, SpeakerWaveformSquare, 170, 0, 0 },  /* B4 */
        { 72, SpeakerWaveformSquare, 400, 0, 0 },  /* C5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_TOCCATA: {
      /* J.S. Bach's Toccata and Fugue in D minor (BWV 565) — opening organ flourish. */
      static const SpeakerNote notes[] = {
        { 81, SpeakerWaveformSquare,  90, 0, 0 },  /* A5  */
        { 79, SpeakerWaveformSquare,  90, 0, 0 },  /* G5  */
        { 81, SpeakerWaveformSquare, 650, 0, 0 },  /* A5  */
        {  0, SpeakerWaveformSine,   220, 0, 0 },
        { 79, SpeakerWaveformSquare, 100, 0, 0 },  /* G5  */
        { 77, SpeakerWaveformSquare, 100, 0, 0 },  /* F5  */
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5  */
        { 74, SpeakerWaveformSquare, 180, 0, 0 },  /* D5  */
        { 73, SpeakerWaveformSquare, 260, 0, 0 },  /* C#5 */
        { 74, SpeakerWaveformSquare, 650, 0, 0 },  /* D5  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_MOUNTAIN_KING: {
      /* Edvard Grieg's In the Hall of the Mountain King (Peer Gynt) — opening phrase. */
      static const SpeakerNote notes[] = {
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 71, SpeakerWaveformSquare, 120, 0, 0 },  /* B4  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 74, SpeakerWaveformSquare, 120, 0, 0 },  /* D5  */
        { 76, SpeakerWaveformSquare, 120, 0, 0 },  /* E5  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 76, SpeakerWaveformSquare, 240, 0, 0 },  /* E5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 75, SpeakerWaveformSquare, 120, 0, 0 },  /* D#5 */
        { 71, SpeakerWaveformSquare, 120, 0, 0 },  /* B4  */
        { 75, SpeakerWaveformSquare, 240, 0, 0 },  /* D#5 */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 74, SpeakerWaveformSquare, 120, 0, 0 },  /* D5  */
        { 70, SpeakerWaveformSquare, 120, 0, 0 },  /* Bb4 */
        { 74, SpeakerWaveformSquare, 240, 0, 0 },  /* D5  */
        {  0, SpeakerWaveformSine,    25, 0, 0 },
        { 69, SpeakerWaveformSquare, 120, 0, 0 },  /* A4  */
        { 71, SpeakerWaveformSquare, 120, 0, 0 },  /* B4  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 74, SpeakerWaveformSquare, 120, 0, 0 },  /* D5  */
        { 76, SpeakerWaveformSquare, 120, 0, 0 },  /* E5  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 76, SpeakerWaveformSquare, 120, 0, 0 },  /* E5  */
        { 81, SpeakerWaveformSquare, 120, 0, 0 },  /* A5  */
        { 79, SpeakerWaveformSquare, 120, 0, 0 },  /* G5  */
        { 76, SpeakerWaveformSquare, 120, 0, 0 },  /* E5  */
        { 72, SpeakerWaveformSquare, 120, 0, 0 },  /* C5  */
        { 76, SpeakerWaveformSquare, 120, 0, 0 },  /* E5  */
        { 79, SpeakerWaveformSquare, 400, 0, 0 },  /* G5  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_VALKYRIES: {
      /* Richard Wagner's Ride of the Valkyries — opening brass fanfare. */
      static const SpeakerNote notes[] = {
        { 71, SpeakerWaveformSquare, 130, 0, 0 },  /* B4 */
        { 76, SpeakerWaveformSquare, 220, 0, 0 },  /* E5 */
        { 71, SpeakerWaveformSquare,  90, 0, 0 },  /* B4 */
        { 76, SpeakerWaveformSquare, 150, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 360, 0, 0 },  /* G5 */
        { 76, SpeakerWaveformSquare, 360, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 220, 0, 0 },  /* G5 */
        { 76, SpeakerWaveformSquare,  90, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 150, 0, 0 },  /* G5 */
        { 83, SpeakerWaveformSquare, 360, 0, 0 },  /* B5 */
        { 79, SpeakerWaveformSquare, 500, 0, 0 },  /* G5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_SPRING: {
      /* Antonio Vivaldi's Spring (The Four Seasons) — opening Allegro phrase. */
      static const SpeakerNote notes[] = {
        { 72, SpeakerWaveformSquare, 160, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare,  90, 0, 0 },  /* C5 */
        { 79, SpeakerWaveformSquare, 400, 0, 0 },  /* G5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 79, SpeakerWaveformSquare,  90, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare,  90, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare,  90, 0, 0 },  /* D5 */
        { 72, SpeakerWaveformSquare,  90, 0, 0 },  /* C5 */
        { 79, SpeakerWaveformSquare, 400, 0, 0 },  /* G5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 79, SpeakerWaveformSquare,  90, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare,  90, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        { 77, SpeakerWaveformSquare,  90, 0, 0 },  /* F5 */
        { 79, SpeakerWaveformSquare,  90, 0, 0 },  /* G5 */
        { 77, SpeakerWaveformSquare, 180, 0, 0 },  /* F5 */
        { 76, SpeakerWaveformSquare, 180, 0, 0 },  /* E5 */
        { 74, SpeakerWaveformSquare, 360, 0, 0 },  /* D5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_TOREADOR: {
      /* Georges Bizet's Toreador Song (Carmen) — "Toréador, en garde!" refrain. */
      static const SpeakerNote notes[] = {
        { 79, SpeakerWaveformSquare, 340, 0, 0 },  /* G5  To-   */
        { 81, SpeakerWaveformSquare, 120, 0, 0 },  /* A5  -ré-  */
        { 79, SpeakerWaveformSquare, 220, 0, 0 },  /* G5  -a-   */
        { 76, SpeakerWaveformSquare, 220, 0, 0 },  /* E5  -dor, */
        { 72, SpeakerWaveformSquare, 130, 0, 0 },  /* C5  en    */
        { 76, SpeakerWaveformSquare,  80, 0, 0 },  /* E5        */
        { 74, SpeakerWaveformSquare, 240, 0, 0 },  /* D5  gar-  */
        { 72, SpeakerWaveformSquare, 300, 0, 0 },  /* C5  -de!  */
        {  0, SpeakerWaveformSine,    90, 0, 0 },
        { 74, SpeakerWaveformSquare, 170, 0, 0 },  /* D5  To-   */
        { 76, SpeakerWaveformSquare,  80, 0, 0 },  /* E5  -ré-  */
        { 77, SpeakerWaveformSquare, 220, 0, 0 },  /* F5  -a-   */
        { 69, SpeakerWaveformSquare, 300, 0, 0 },  /* A4  -dor! */
        {  0, SpeakerWaveformSine,    80, 0, 0 },
        { 71, SpeakerWaveformSquare, 170, 0, 0 },  /* B4  To-   */
        { 72, SpeakerWaveformSquare,  80, 0, 0 },  /* C5  -ré-  */
        { 74, SpeakerWaveformSquare, 220, 0, 0 },  /* D5  -a-   */
        { 67, SpeakerWaveformSquare, 420, 0, 0 },  /* G4  -dor! */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_HABANERA: {
      /* Georges Bizet's Habanera (Carmen) — chromatic descending hook. */
      static const SpeakerNote notes[] = {
        { 81, SpeakerWaveformSquare, 260, 0, 0 },  /* A5  */
        { 80, SpeakerWaveformSquare, 260, 0, 0 },  /* G#5 */
        { 79, SpeakerWaveformSquare, 110, 0, 0 },  /* G5  */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 79, SpeakerWaveformSquare, 110, 0, 0 },  /* G5  */
        { 78, SpeakerWaveformSquare, 110, 0, 0 },  /* F#5 */
        { 77, SpeakerWaveformSquare, 160, 0, 0 },  /* F5  */
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5  */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare, 160, 0, 0 },  /* D#5 */
        { 74, SpeakerWaveformSquare, 160, 0, 0 },  /* D5  */
        { 72, SpeakerWaveformSquare, 130, 0, 0 },  /* C5  */
        { 74, SpeakerWaveformSquare,  70, 0, 0 },  /* D5  */
        { 72, SpeakerWaveformSquare, 160, 0, 0 },  /* C5  */
        { 71, SpeakerWaveformSquare, 160, 0, 0 },  /* B4  */
        { 69, SpeakerWaveformSquare, 420, 0, 0 },  /* A4  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_SUGAR_PLUM: {
      /* Tchaikovsky's Dance of the Sugar Plum Fairy (The Nutcracker) — celesta hook. */
      static const SpeakerNote notes[] = {
        { 79, SpeakerWaveformSquare,  90, 0, 0 },  /* G5  */
        { 78, SpeakerWaveformSquare,  90, 0, 0 },  /* F#5 */
        { 79, SpeakerWaveformSquare, 110, 0, 0 },  /* G5  */
        {  0, SpeakerWaveformSine,    50, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },  /* E5  */
        {  0, SpeakerWaveformSine,    50, 0, 0 },
        { 80, SpeakerWaveformSquare, 110, 0, 0 },  /* G#5 */
        {  0, SpeakerWaveformSine,    50, 0, 0 },
        { 78, SpeakerWaveformSquare, 110, 0, 0 },  /* F#5 */
        {  0, SpeakerWaveformSine,   120, 0, 0 },
        { 75, SpeakerWaveformSquare, 100, 0, 0 },  /* D#5 */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 75, SpeakerWaveformSquare, 100, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5  */
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5  */
        { 75, SpeakerWaveformSquare, 100, 0, 0 },  /* D#5 */
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5  */
        { 78, SpeakerWaveformSquare, 160, 0, 0 },  /* F#5 */
        {  0, SpeakerWaveformSine,    40, 0, 0 },
        { 71, SpeakerWaveformSquare, 360, 0, 0 },  /* B4  */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_FIRST_CALL: {
      /* Traditional "First Call" / "Call to the Post" horse-racing bugle fanfare. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 100, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 100, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 320, 0, 0 },  /* G5 */
        { 76, SpeakerWaveformSquare, 140, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 380, 0, 0 },  /* G5 */
        {  0, SpeakerWaveformSine,    60, 0, 0 },
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5 */
        { 79, SpeakerWaveformSquare, 100, 0, 0 },  /* G5 */
        { 76, SpeakerWaveformSquare, 100, 0, 0 },  /* E5 */
        { 72, SpeakerWaveformSquare, 100, 0, 0 },  /* C5 */
        { 67, SpeakerWaveformSquare, 100, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 100, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 320, 0, 0 },  /* E5 */
        { 72, SpeakerWaveformSquare, 140, 0, 0 },  /* C5 */
        { 76, SpeakerWaveformSquare, 160, 0, 0 },  /* E5 */
        { 72, SpeakerWaveformSquare, 160, 0, 0 },  /* C5 */
        { 67, SpeakerWaveformSquare, 160, 0, 0 },  /* G4 */
        { 72, SpeakerWaveformSquare, 450, 0, 0 },  /* C5 */
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_JINGLE_BELLS: {
      /* Jingle Bells chorus (public domain), in C. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 330, 0, 0 },
        { 74, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 660, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 77, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 77, SpeakerWaveformSquare, 110, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_DECK_THE_HALLS: {
      /* Deck the Halls (traditional Welsh), in C. */
      static const SpeakerNote notes[] = {
        { 79, SpeakerWaveformSquare, 330, 0, 0 },
        { 77, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 330, 0, 0 },
        { 77, SpeakerWaveformSquare, 110, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 79, SpeakerWaveformSquare, 330, 0, 0 },
        { 81, SpeakerWaveformSquare, 110, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 77, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_MERRY_CHRISTMAS: {
      /* We Wish You a Merry Christmas (traditional English), in C. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        { 74, SpeakerWaveformSquare, 110, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        { 71, SpeakerWaveformSquare, 110, 0, 0 },
        { 69, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 69, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 69, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 74, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 74, SpeakerWaveformSquare, 110, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        { 71, SpeakerWaveformSquare, 220, 0, 0 },
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 77, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 74, SpeakerWaveformSquare, 110, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 69, SpeakerWaveformSquare, 220, 0, 0 },
        { 67, SpeakerWaveformSquare, 110, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 110, 0, 0 },
        { 69, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 71, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_SILENT_NIGHT: {
      /* Silent Night (Gruber, 1818) opening verse, in C. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 330, 0, 0 },
        { 69, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 64, SpeakerWaveformSquare, 660, 0, 0 },
        { 67, SpeakerWaveformSquare, 330, 0, 0 },
        { 69, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 64, SpeakerWaveformSquare, 660, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 71, SpeakerWaveformSquare, 660, 0, 0 },
        { 72, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 67, SpeakerWaveformSquare, 880, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_JOY_TO_THE_WORLD: {
      /* Joy to the World (Handel/Mason) opening line, in C. */
      static const SpeakerNote notes[] = {
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 71, SpeakerWaveformSquare, 330, 0, 0 },
        { 69, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 550, 0, 0 },
        { 65, SpeakerWaveformSquare, 220, 0, 0 },
        { 64, SpeakerWaveformSquare, 220, 0, 0 },
        { 62, SpeakerWaveformSquare, 330, 0, 0 },
        { 60, SpeakerWaveformSquare, 550, 0, 0 },
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 69, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 69, SpeakerWaveformSquare, 110, 0, 0 },
        { 71, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 71, SpeakerWaveformSquare, 110, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_AULD_LANG_SYNE: {
      /* Auld Lang Syne (traditional Scots) opening line, in C. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 330, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 72, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 220, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 81, SpeakerWaveformSquare, 660, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 72, SpeakerWaveformSquare, 220, 0, 0 },
        { 74, SpeakerWaveformSquare, 220, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_CHARGE: {
      /* Cavalry "Charge!" fanfare. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare,  80, 0, 0 },
        { 72, SpeakerWaveformSquare,  80, 0, 0 },
        { 76, SpeakerWaveformSquare,  80, 0, 0 },
        { 79, SpeakerWaveformSquare, 220, 0, 0 },
        { 76, SpeakerWaveformSquare, 100, 0, 0 },
        { 79, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_IMPERIAL_MARCH: {
      /* Imperial March (Darth Vader's theme), John Williams. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        { 63, SpeakerWaveformSquare, 390, 0, 0 },
        { 70, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        { 63, SpeakerWaveformSquare, 390, 0, 0 },
        { 70, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 880, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        { 75, SpeakerWaveformSquare, 390, 0, 0 },
        { 70, SpeakerWaveformSquare, 110, 0, 0 },
        { 66, SpeakerWaveformSquare, 440, 0, 0 },
        { 63, SpeakerWaveformSquare, 390, 0, 0 },
        { 70, SpeakerWaveformSquare, 110, 0, 0 },
        { 67, SpeakerWaveformSquare, 880, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_JAWS: {
      /* Jaws theme, John Williams: accelerating E-F ostinato. */
      static const SpeakerNote notes[] = {
        { 64, SpeakerWaveformSquare, 400, 0, 0 },
        { 65, SpeakerWaveformSquare, 400, 0, 0 },
        { 64, SpeakerWaveformSquare, 350, 0, 0 },
        { 65, SpeakerWaveformSquare, 350, 0, 0 },
        { 64, SpeakerWaveformSquare, 280, 0, 0 },
        { 65, SpeakerWaveformSquare, 280, 0, 0 },
        { 64, SpeakerWaveformSquare, 200, 0, 0 },
        { 65, SpeakerWaveformSquare, 200, 0, 0 },
        { 64, SpeakerWaveformSquare, 140, 0, 0 },
        { 65, SpeakerWaveformSquare, 140, 0, 0 },
        { 64, SpeakerWaveformSquare, 100, 0, 0 },
        { 65, SpeakerWaveformSquare, 100, 0, 0 },
        { 64, SpeakerWaveformSquare, 100, 0, 0 },
        { 65, SpeakerWaveformSquare, 100, 0, 0 },
        { 64, SpeakerWaveformSquare, 100, 0, 0 },
        { 65, SpeakerWaveformSquare, 300, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_PINK_PANTHER: {
      /* The Pink Panther Theme, Henry Mancini. */
      static const SpeakerNote notes[] = {
        { 63, SpeakerWaveformSquare, 150, 0, 0 },
        { 64, SpeakerWaveformSquare, 350, 0, 0 },
        {  0, SpeakerWaveformSquare, 200, 0, 0 },
        { 66, SpeakerWaveformSquare, 150, 0, 0 },
        { 67, SpeakerWaveformSquare, 350, 0, 0 },
        {  0, SpeakerWaveformSquare, 200, 0, 0 },
        { 63, SpeakerWaveformSquare, 150, 0, 0 },
        { 64, SpeakerWaveformSquare, 200, 0, 0 },
        { 66, SpeakerWaveformSquare, 150, 0, 0 },
        { 67, SpeakerWaveformSquare, 200, 0, 0 },
        { 72, SpeakerWaveformSquare, 150, 0, 0 },
        { 71, SpeakerWaveformSquare, 150, 0, 0 },
        { 64, SpeakerWaveformSquare, 150, 0, 0 },
        { 67, SpeakerWaveformSquare, 150, 0, 0 },
        { 71, SpeakerWaveformSquare, 150, 0, 0 },
        { 70, SpeakerWaveformSquare, 700, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_ZELDA_SECRET: {
      /* The Legend of Zelda "secret found" jingle. */
      static const SpeakerNote notes[] = {
        { 79, SpeakerWaveformSquare, 110, 0, 0 },
        { 78, SpeakerWaveformSquare, 110, 0, 0 },
        { 75, SpeakerWaveformSquare, 110, 0, 0 },
        { 69, SpeakerWaveformSquare, 110, 0, 0 },
        { 68, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 110, 0, 0 },
        { 80, SpeakerWaveformSquare, 110, 0, 0 },
        { 84, SpeakerWaveformSquare, 450, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_HAPPY_BIRTHDAY: {
      /* Happy Birthday to You, in C. */
      static const SpeakerNote notes[] = {
        { 67, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 110, 0, 0 },
        { 69, SpeakerWaveformSquare, 440, 0, 0 },
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        { 72, SpeakerWaveformSquare, 440, 0, 0 },
        { 71, SpeakerWaveformSquare, 660, 0, 0 },
        { 67, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 110, 0, 0 },
        { 69, SpeakerWaveformSquare, 440, 0, 0 },
        { 67, SpeakerWaveformSquare, 440, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
        { 67, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 67, SpeakerWaveformSquare, 110, 0, 0 },
        { 79, SpeakerWaveformSquare, 440, 0, 0 },
        { 76, SpeakerWaveformSquare, 440, 0, 0 },
        { 72, SpeakerWaveformSquare, 440, 0, 0 },
        { 71, SpeakerWaveformSquare, 440, 0, 0 },
        { 69, SpeakerWaveformSquare, 440, 0, 0 },
        { 77, SpeakerWaveformSquare, 330, 0, 0 },
        {  0, SpeakerWaveformSine,    20, 0, 0 },
        { 77, SpeakerWaveformSquare, 110, 0, 0 },
        { 76, SpeakerWaveformSquare, 440, 0, 0 },
        { 72, SpeakerWaveformSquare, 440, 0, 0 },
        { 74, SpeakerWaveformSquare, 440, 0, 0 },
        { 72, SpeakerWaveformSquare, 660, 0, 0 },
      };
      return speaker_play_notes(notes, sizeof(notes) / sizeof(notes[0]), vol);
    }

    case HC_STYLE_DOORBELL: {
      /* Two-tone doorbell: ding-dong. */
      static const SpeakerNote notes[] = {
        { 76, SpeakerWaveformSquare, 350, 0, 0 },
        { 72, SpeakerWaveformSquare, 750, 0, 0 },
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

  const HcStyle style = hc_resolve_style((HcStyle)s->style);
  const int strikes = hc_strike_count(s, hour24);
  hc_do_vibe(s, style, strikes);
  bool sound = hc_play_sound(s, style, strikes);
  APP_LOG(APP_LOG_LEVEL_INFO, "chime: hour=%d style=%d (cfg=%d) strikes=%d vibe=%d sound=%d muted=%d",
          hour24, style, s->style, strikes, s->vibe, sound, speaker_is_muted());
  return sound;
}
