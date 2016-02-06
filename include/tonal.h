/*
 * Copyright 2016 Martin Aberg
 *
 * This file is part of tonal.
 *
 * tonal is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Foobar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TONAL_H_
#define TONAL_H_

#include <stdio.h>

/*
 * tonal - tonal music library
 *
 * The idea behind the tonal music library is to provide representations for
 * - Tonal Pitch Classes,
 * - Tonal Pitches,
 * - Tonal Intervals,
 * - Tonal Interval Classes,
 * and to provide transformations which preserve the tonal properties.
 *
 * - A Tonal Pitch Class is represented by a Diatonic Pitch and a Pitch
 *   Alteration. For example "Dbb".
 * - A Tonal Interval Class is represented by an Interval Direction, a Diatonic
 *   Interval and an Interval Alteration. For example an "Augmented Fourth".
 * - Tonal Pitch and Tonal Interval are similar with the addition of octave
 *   representation.
 *
 * This library supports the following concepts, and more:
 * - Differentiate an augmented fourth interval from a diminished fifth
 *   interval. 
 * - Transpose the tone C# up an augmented prime and preserve the
 *   tonal/alteration information.
 *
 * The tonal music library is based on ideas presented in:
 * https://www.cs.cmu.edu/~music/392/course-material/lecture-notes/class3/class10-tonality2/arithmetics.ps
 */

/* Diatonic Pitch */
enum {
        DP_C,
        DP_D,
        DP_E,
        DP_F,
        DP_G,
        DP_A,
        DP_B,
        DP_NONE
};

/*
 * String representations of the DP_ (Diatonic Pitch) values, indexed by DP_.
 */
extern const char *diatonic_pitch_str[];

/* Pitch Alteration */
enum {
        PA_bb,
        PA_b,
        PA_,
        PA_s,
        PA_ss,
        PA_NONE
};

/*
 * String representation of the PA_ (Pitch Alteration) values, indexed by PA_.
 */
extern const char *pitch_alteration_str[];

/* TPC: Tonal Pitch Class. */
struct tonal_pitch_class {
        int diatonic_pitch;
        int pitch_alteration;
};

/* TP: Tonal Pitch */
struct tonal_pitch {
        int diatonic_pitch;
        int pitch_alteration;
        int octave;
};


/* Diatonic Interval */
enum {
        DI_PRIME,
        DI_SECOND,
        DI_THIRD,
        DI_FOURTH,
        DI_FIFTH,
        DI_SIXTH,
        DI_SEVENTH,
        DI_NONE
};

/*
 * String representations of the DI_ (Diatonic Interval) values, indexed by
 * DI_.
 */
extern const char *diatonic_interval_str[];

/* Interval Alteration */
enum {
        IA_DIMINISHED,
        IA_MINOR,
        IA_MAJOR,
        IA_PERFECT,
        IA_AUGMENTED,
        IA_NONE
};

/*
 * String representations of the IA_ (Interval Alteration) values, indexed by
 * IA_.
 */
extern const char *interval_alteration_str[];

/* Interval Direction */
enum {
        ID_UP,
        ID_DOWN,
        ID_NONE
};

/*
 * String representations of the ID_ (Interval Direction) values, indexed by
 * ID_.
 */
extern const char *interval_direction_str[];

/* TIC: Tonal Interval Class */
struct tonal_interval_class {
        int diatonic_interval;
        int interval_alteration;
};

/* TI: Tonal Interval */
struct tonal_interval {
        int diatonic_interval;
        int interval_alteration;
        /* An octave in a Tonal Interval must be >= 0. */
        int octave;
        int interval_direction;
};


/*
 * Function return values
 *
 * All functions in this API return one of these values.
 */
enum {
        TONAL_OK,
        TONAL_FAIL
};

/* Pretty print to stream. */
extern int tpc_print(FILE *stream, const struct tonal_pitch_class *tpc);
extern int tp_print(FILE *stream, const struct tonal_pitch *tp);
extern int tic_print(FILE *stream, const struct tonal_interval_class *tic);
extern int ti_print(FILE *stream, const struct tonal_interval *ti);

/* Shortcuts for setting fields in Tonal Pitch Class and Tonal Pitch. */
extern int tpc_set(
        struct tonal_pitch_class *tpc,
        int diatonic_pitch,
        int pitch_alteration
);
extern int tp_set(
        struct tonal_pitch *tp,
        int diatonic_pitch,
        int pitch_alteration,
        int octave
);

/* Shortcuts for setting fields in Tonal Interval Class and Tonal Interval. */
extern int tic_set(
        struct tonal_interval_class *tic,
        int diatonic_interval,
        int interval_alteration
);

extern int ti_set(
        struct tonal_interval *ti,
        int diatonic_interval,
        int interval_alteration,
        int octave,
        int interval_direction
);

/*
 * Add Tonal Interval to a Tonal Pitch.
 *
 * tp_sum := tp + ti
 */
extern int tp_add(
        const struct tonal_pitch *tp,
        const struct tonal_interval *ti,
        struct tonal_pitch *tp_sum
);

/*
 * Add Tonal Interval to a Tonal Interval.
 *
 * ti_sum := ti0 + ti1
 */
extern int ti_add(
        const struct tonal_interval *ti0,
        const struct tonal_interval *ti1,
        struct tonal_interval *ti_sum
);

/*
 * Calculate difference (tonal interval) between two tonal pitches.
 *
 * ti_diff := tp1 - tp0
 */
extern int tp_sub(
        const struct tonal_pitch *tp0,
        const struct tonal_pitch *tp1,
        struct tonal_interval *ti_diff
);

/*
 * Calculate difference (tonal interval) between two tonal intervals.
 *
 * ti_diff := ti1 - ti0
 */
extern int ti_sub(
        const struct tonal_interval *ti0,
        const struct tonal_interval *ti1,
        struct tonal_interval *ti_diff
);

/* Translate Tonal Pitch to MIDI Note Number. */
extern int tp_to_mnn(
        const struct tonal_pitch *tp
);

#endif

