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

#ifndef TONAL_PRIV_H_
#define TONAL_PRIV_H_

#include <tonal.h>

/*
 * TC: Tonal Class
 *
 * The tonal class is an abstraction for the Tonal Pitch Class and Tonal
 * Interval Class concepts.
 */
struct tonal_class {
        /* { 0, 1, 2, 3, 4, 5, 6 } */
        int diatonic_point;
        /* { -2, -1, 0, 1, 2 } */
        int alteration;
};

/*
 * TE: Tonal Element
 *
 * The tonal element is an abstraction for the Tonal Pitch and Tonal Interval
 * concepts.
 */
struct tonal_element {
        /* Allowed values are { 0, 1, 2, 3, 4, 5, 6 } */
        int diatonic_point;
        /* Allowed values are { -2, -1, 0, 1, 2 } */
        int alteration;
        /* The octave may have any integer value is allowed. */
        int octave;
};
/* The zero element in Tonal Element arithmetics. */
extern const struct tonal_element TONAL_ELEMENT_ZERO;


/*
 * MPC: Music (MIDI) Pitch Class
 *
 * The Music Pitch Class is an integer in the range {0..11}. Music Pitch Class
 * does not convey all tonal information.
 */

/*
 * Map Diatonic poinT to Music Pitch Class.
 *
 * {0..6} -> {0..11}
 *
 * Returns INT_MIN if dt parameter is not in {0..6}.
 */
extern int dt_get_mpc_value(int dt);

/*
 * Map Tonal Class to the union of Music Pitch Class and {-2, -1, 12, 13}.
 *
 * tonal_class -> {-2..13}
 *
 * Returns INT_MIN if tc is invalid.
 */
extern int tc_get_mpc_value(const struct tonal_class *tc);


/*
 * The diatonic_value provides a count of the tonal element on an axis of
 * diatonic points (base 7).
 *
 * diatonic_value = 7 * octave + diatonic_point.
 */
extern int te_get_diatonic_value(const struct tonal_element *te);

/*
 * The chromatic_value provides a count of the tonal element on an axis of
 * music pitch classes (base 12). This may be the the MIDI note value.
 *
 * chromatic_value = 12 * octave + tc_get_mpc_value(tc)
 */
extern int te_get_chromatic_value(const struct tonal_element *te);

/*
 * Add Tonal Elements
 *
 * te2 := te0 + te1
 * te2 := te1 + te0
 * te := TONAL_ELEMENT_ZERO + te
 */
extern int te_add(
        const struct tonal_element *te0,
        const struct tonal_element *te1,
        struct tonal_element *te2
);

/*
 * Invert a Tonal Element (in-place)
 *
 * TONAL_ELEMENT_ZERO == te + te_inv(te)
 */
extern int te_inv(struct tonal_element *te);

/*
 * Subtract Tonal Elements
 *
 * te2 := te1 - te0
 * Definition of subtraction: te1 - te0 == te1 + te_inv(te0)
 */
extern int te_sub(
        const struct tonal_element *te0,
        const struct tonal_element *te1,
        struct tonal_element *te2
);


/* Translate Tonal Pitch Class to Tonal Class. */
extern int tpc_to_tc(
        const struct tonal_pitch_class *tpc,
        struct tonal_class *tc
);

/* Translate Tonal Class to Tonal Pitch Class. */
extern int tc_to_tpc(
        const struct tonal_class *tc,
        struct tonal_pitch_class *tpc
);

/* Translate Tonal Pitch to Tonal Element. */
extern int tp_to_te(
        const struct tonal_pitch *tp,
        struct tonal_element *te
);

/* Translate Tonal Element to Tonal Pitch. */
extern int te_to_tp(
        const struct tonal_element *te,
        struct tonal_pitch *tp
);

/* Translate Tonal Interval Class to Tonal Class. */
extern int tic_to_tc(
        const struct tonal_interval_class *tic,
        struct tonal_class *tc
);

/* Translate Tonal Class to Tonal Interval Class. */
extern int tc_to_tic(
        const struct tonal_class *tc,
        struct tonal_interval_class *tic
);

/* Translate Tonal Interval to Tonal Element. */
extern int ti_to_te(
        const struct tonal_interval *ti,
        struct tonal_element *te
);

/* Translate Tonal Element to Tonal Interval. */
extern int te_to_ti(
        const struct tonal_element *te,
        struct tonal_interval *ti
);

/* Pretty print */
extern int te_print(FILE *stream, const struct tonal_element *te);


#endif

