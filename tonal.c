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
 * tonal is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Parameter checking and asserts:
 * - Validate parameters on entry, but do NOT assert on it. Bad user input
 *   shall produce useful return value.
 * - Assert after internal calculations. Bad results shall be captured.
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <tonal.h>
#include "tonal_priv.h"

#define NELEM(x) ((int) ((sizeof x) / (sizeof x[0])))

const char *diatonic_pitch_str[] = {
        "C", "D", "E", "F", "G", "A", "B",
        "NONE"
};

const char *pitch_alteration_str[] = {
        "bb", "b", "", "#", "##",
        "NONE"
};

const char *diatonic_interval_str[] = {
        "Prime", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh",
        "NONE"
};

const char *interval_alteration_str[] = {
        "Diminished", "Minor", "Major", "Perfect", "Augmented",
        "NONE"
};

const char *interval_direction_str[] = {
        "Up", "Down",
        "NONE"
};


static const int TIC_TO_TC_TABLE[DI_NONE][IA_NONE] = {
/*              DIM    MINOR    MAJOR     PERF      AUG */
/* PRIME   */ { -1,     'x',     'x',       0,       1 },
/* SECOND  */ { -2,      -1,       0,     'x',       1 },
/* THIRD   */ { -2,      -1,       0,     'x',       1 },
/* FOURTH  */ { -1,     'x',     'x',       0,       1 },
/* FIFTH   */ { -1,     'x',     'x',       0,       1 },
/* SIXTH   */ { -2,      -1,       0,     'x',       1 },
/* SEVENTH */ { -2,      -1,       0,     'x',       1 },
};

static inline int validate_diatonic_point(int dt)
{
        if (0 <= dt && dt <= 6) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_alteration(int a)
{
        if (-2 <= a && a <= 2) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_tc(const struct tonal_class *tc)
{
        int ret;

        if (NULL == tc) { return TONAL_FAIL; }

        ret = validate_diatonic_point(tc->diatonic_point);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_alteration(tc->alteration);
        if (TONAL_OK != ret) { return ret; }

        return TONAL_OK;
}

static inline int validate_te(const struct tonal_element *te)
{
        if (NULL == te) { return TONAL_FAIL; }
        return validate_tc((const struct tonal_class *) te);
}

static inline int validate_diatonic_pitch(int dp)
{
        if (DP_C <= dp && dp <= DP_B) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_pitch_alteration(int pa)
{
        if (PA_bb <= pa && pa <= PA_ss) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_tpc(const struct tonal_pitch_class *tpc)
{
        int ret;

        if (NULL == tpc) { return TONAL_FAIL; }

        ret = validate_diatonic_pitch(tpc->diatonic_pitch);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_pitch_alteration(tpc->pitch_alteration);
        if (TONAL_OK != ret) { return ret; }

        return TONAL_OK;
}

static inline int validate_tp(const struct tonal_pitch *tp)
{
        if (NULL == tp) { return TONAL_FAIL; }

        /* NOTE: Restricts the tonal pitch octave to positive. */
        if (tp->octave < 0) {
                return TONAL_FAIL;
        }
        return validate_tpc((const struct tonal_pitch_class *) tp);
}

static inline int validate_diatonic_interval(int di)
{
        if (DI_PRIME <= di && di <= DI_SEVENTH) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_interval_alteration(int ia)
{
        if (IA_DIMINISHED <= ia && ia <= IA_AUGMENTED) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_tic(const struct tonal_interval_class *tic)
{
        int ret;
        int di;
        int ia;

        if (NULL == tic) { return TONAL_FAIL; }

        di = tic->diatonic_interval;
        ia = tic->interval_alteration;

        ret = validate_diatonic_interval(di);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_interval_alteration(ia);
        if (TONAL_OK != ret) { return ret; }

        if ('x' == TIC_TO_TC_TABLE[di][ia]) {
                return TONAL_FAIL;
        }

        return TONAL_OK;
}

static inline int validate_interval_octave(int interval_octave)
{
        if (0 <= interval_octave) { return TONAL_OK; }
        return TONAL_FAIL;
}

static inline int validate_interval_direction(int interval_direction)
{
        switch (interval_direction) {
                case ID_UP:
                case ID_DOWN:
                        return TONAL_OK;
                        break;
        }
        return TONAL_FAIL;
}

static inline int validate_ti(const struct tonal_interval *ti)
{
        int ret;

        if (NULL == ti) { return TONAL_FAIL; }

        ret = validate_tic((const struct tonal_interval_class *) ti);

        ret = validate_interval_octave(ti->octave);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_interval_direction(ti->interval_direction);
        if (TONAL_OK != ret) { return ret; }

        /*
         * A prime may be either perfect or augmented, never diminished.
         */
        if (
                0 == ti->octave &&
                DI_PRIME == ti->diatonic_interval &&
                IA_DIMINISHED == ti->interval_alteration
        ) {
                return TONAL_FAIL;
        }

        return ret;
}


/* Mapping from Diatonic PoinT to Music Pitch Class */
static const int DT_TO_MPC_TABLE[7] = { 0, 2, 4, 5, 7, 9, 11 };

/* Music Pitch Class: {0..11} */
int dt_get_mpc_value(int dt)
{
        if (dt < 0 || NELEM(DT_TO_MPC_TABLE) <= dt) { return INT_MIN; }
        return DT_TO_MPC_TABLE[dt];
}

/* Extends Music Pitch Class to {-2..13}. */
int tc_get_mpc_value(const struct tonal_class *tc)
{
        int ret;
        int mpc;

        ret = validate_tc(tc);
        if (TONAL_OK != ret) { return INT_MIN; }

        mpc = dt_get_mpc_value(tc->diatonic_point) + tc->alteration;
        assert(-2 <= mpc && mpc <= 13);
        return mpc;
}

int te_get_diatonic_value(const struct tonal_element *te)
{
        int ret;

        ret = validate_te(te);
        if (TONAL_OK != ret) { return INT_MIN; }

        return 7 * te->octave + te->diatonic_point;
}

int te_get_chromatic_value(const struct tonal_element *te)
{
        int ret;
        const struct tonal_class *tc;

        ret = validate_te(te);
        if (TONAL_OK != ret) { return INT_MIN; }

        tc = (const struct tonal_class *) te;
        return 12 * te->octave + tc_get_mpc_value(tc);
}

/* Implement Proposition 1. */
static int te_from_dv_cv(struct tonal_element *te, int dv, int cv)
{
        int ret;
        int o;
        int a;

        /* Do modulo arithmetic. NOTE: This may be implementation dependent. */
        if (dv <= 0) {
                o = (dv - 6) / 7;
        } else {
                o = dv / 7;
        }
        dv = dv - o * 7;
        cv = cv - o * 12;

        /* NOTE: Magic numbers */
        if (cv < -2 || 13 < cv) {
                return TONAL_FAIL;
        }

        assert(TONAL_OK == validate_diatonic_point(dv));
        te->diatonic_point = dv;
        te->alteration = 0;
        te->octave = 0;

        a = cv - te_get_chromatic_value(te);
        ret = validate_alteration(a);
        if (TONAL_OK != ret) { return ret; }

        te->alteration = a;
        te->octave = o;
        assert(TONAL_OK == validate_te(te));
        return TONAL_OK;
}

/* Invert Tonal Element. */
int te_inv(struct tonal_element *te)
{
        int ret;
        int dv;
        int cv;

        ret = validate_te(te);
        if (TONAL_OK != ret) { return ret; }

        dv = -(te_get_diatonic_value(te));
        cv = -(te_get_chromatic_value(te));
        ret = te_from_dv_cv(te, dv, cv);
        return ret;
}

/* Definition 1. */
int te_add(
        const struct tonal_element *te0,
        const struct tonal_element *te1,
        struct tonal_element *te2
)
{
        int ret;
        int dv;
        int cv;

        ret = validate_te(te0);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_te(te1);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == te2) { return TONAL_FAIL; }

        dv = te_get_diatonic_value(te0) + te_get_diatonic_value(te1);
        cv = te_get_chromatic_value(te0) + te_get_chromatic_value(te1);
        ret = te_from_dv_cv(te2, dv, cv);
        return ret;
}

int te_sub(
        const struct tonal_element *te0,
        const struct tonal_element *te1,
        struct tonal_element *te2
)
{
        int ret;

        ret = validate_te(te0);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_te(te1);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == te2) { return TONAL_FAIL; }

        struct tonal_element te_tmp;

        memcpy(&te_tmp, te1, sizeof te_tmp);

        ret = te_inv(&te_tmp);
        if (TONAL_OK != ret) { return ret; }

        ret = te_add(te0, &te_tmp, te2);
        return ret;
}

int tpc_to_tc(const struct tonal_pitch_class *tpc, struct tonal_class *tc)
{
        int ret;

        ret = validate_tpc(tpc);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == tc) { return TONAL_FAIL; }

        tc->diatonic_point = tpc->diatonic_pitch - DP_C;
        tc->alteration = tpc->pitch_alteration - PA_;

        assert(TONAL_OK == validate_tc(tc));
        return TONAL_OK;
}

int tc_to_tpc(const struct tonal_class *tc, struct tonal_pitch_class *tpc)
{
        int ret;

        ret = validate_tc(tc);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == tpc) { return TONAL_FAIL; }

        tpc->diatonic_pitch = tc->diatonic_point + DP_C;
        tpc->pitch_alteration = tc->alteration + PA_;

        assert(TONAL_OK == validate_tpc(tpc));
        return TONAL_OK;
}

int tic_to_tc(const struct tonal_interval_class *tic, struct tonal_class *tc)
{
        /*
         * The table above makes assumptions on the values of DI_ and IA_. Too
         * bad we can not use preprocessor #error on enum values, so lets check
         * it at run time instead.
         */
        assert(0 == DI_PRIME);          assert(6 == DI_SEVENTH);
        assert(0 == IA_DIMINISHED);     assert(4 == IA_AUGMENTED);

        int ret;
        int tic_di;
        int tic_ia;

        ret = validate_tic(tic);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == tc) { return TONAL_FAIL; }

        tic_di = tic->diatonic_interval;
        tic_ia = tic->interval_alteration;
        tc->diatonic_point = tic_di - DI_PRIME;
        tc->alteration = TIC_TO_TC_TABLE[tic_di][tic_ia];

        assert(TONAL_OK == validate_tc(tc));
        return TONAL_OK;
}

int tc_to_tic(const struct tonal_class *tc, struct tonal_interval_class *tic)
{
        int ret;
        int tc_a;
        int tic_di;
        int tic_ia;

        ret = validate_tc(tc);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == tic) { return TONAL_FAIL; }

        tic_di = tc->diatonic_point + DI_PRIME;
        tic->diatonic_interval = tic_di;
        tc_a = tc->alteration;

        switch (tic_di) {
                case DI_PRIME:
                case DI_FOURTH:
                case DI_FIFTH:
                        switch (tc_a) {
                                case -1: tic_ia = IA_DIMINISHED; break;
                                case  0: tic_ia = IA_PERFECT;    break;
                                case  1: tic_ia = IA_AUGMENTED;  break;
                                default: return TONAL_FAIL;      break;
                        }
                        break;
                case DI_SECOND:
                case DI_THIRD:
                case DI_SIXTH:
                case DI_SEVENTH:
                        switch (tc_a) {
                                case -2: tic_ia = IA_DIMINISHED; break;
                                case -1: tic_ia = IA_MINOR;      break;
                                case  0: tic_ia = IA_MAJOR;      break;
                                case  1: tic_ia = IA_AUGMENTED;  break;
                                default: return TONAL_FAIL;      break;
                        }
                        break;
                default:
                        assert(!"Unknown diatonic_interval");
        }

        tic->interval_alteration = tic_ia;

        assert(TONAL_OK == validate_tic(tic));
        return TONAL_OK;
}

int tp_to_te(const struct tonal_pitch *tp, struct tonal_element *te)
{
        int ret;
        const struct tonal_pitch_class *tpc;
        struct tonal_class *tc;

        ret = validate_tp(tp);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == te) { return TONAL_FAIL; }

        tpc = (const struct tonal_pitch_class *) tp;
        tc = (struct tonal_class *) te;
        ret = tpc_to_tc(tpc, tc);
        if (TONAL_OK != ret) { return ret; }

        te->octave = tp->octave;

        assert(TONAL_OK == validate_te(te));
        return TONAL_OK;
}

int tp_to_mnn(
        const struct tonal_pitch *tp
)
{
        int ret;
        struct tonal_element te;

        ret = tp_to_te(tp, &te);
        if (TONAL_OK != ret) { return INT_MIN; }

        ret = te_get_chromatic_value(&te);
        return ret;
}

int te_to_tp(const struct tonal_element *te, struct tonal_pitch *tp)
{
        int ret;
        const struct tonal_class *tc;
        struct tonal_pitch_class *tpc;

        ret = validate_te(te);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == tp) { return TONAL_FAIL; }

        tc = (const struct tonal_class *) te;
        tpc = (struct tonal_pitch_class *) tp;
        ret = tc_to_tpc(tc, tpc);
        if (TONAL_OK != ret) { return ret; }

        tp->octave = te->octave;

        assert(TONAL_OK == validate_tp(tp));
        return TONAL_OK;
}

int ti_to_te(const struct tonal_interval *ti, struct tonal_element *te)
{
        int ret;
        const struct tonal_interval_class *tic;
        struct tonal_class *tc;

        ret = validate_ti(ti);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == te) { return TONAL_FAIL; }

        tic = (const struct tonal_interval_class *) ti;
        tc = (struct tonal_class *) te;
        ret = tic_to_tc(tic, tc);
        if (TONAL_OK != ret) { return ret; }

        te->octave = ti->octave;
        ret = validate_te(te);
        assert(TONAL_OK == ret);
        if (ID_DOWN == ti->interval_direction) {
                ret = te_inv(te);
                assert(TONAL_OK == ret);
        }

        assert(TONAL_OK == validate_te(te));
        return TONAL_OK;
}

int te_to_ti(const struct tonal_element *te, struct tonal_interval *ti)
{
        int ret;
        struct tonal_interval_class *tic;
        struct tonal_element te0;
        struct tonal_class *tc;

        ret = validate_te(te);
        if (TONAL_OK != ret) { return ret; }

        if (NULL == ti) { return TONAL_FAIL; }

        tic = (struct tonal_interval_class *) ti;

        if (te->octave >= 0) {
                tc = (struct tonal_class *) te;
                ret = tc_to_tic(tc, tic);
                if (TONAL_OK != ret) { return ret; }

                ti->octave = te->octave;
                ti->interval_direction = ID_UP;
        } else {
                te0.diatonic_point = te->diatonic_point;
                te0.alteration = te->alteration;
                te0.octave = te->octave;
                ret = te_inv(&te0);
                if (TONAL_OK != ret) { return ret; }

                tc = (struct tonal_class *) &te0;
                ret = tc_to_tic(tc, tic);
                if (TONAL_OK != ret) { return ret; }

                ti->octave = te0.octave;
                ti->interval_direction = ID_DOWN;
        }
        /* NOTE: An interval may never be negative. */
        assert(0 <= ti->octave);
        assert(TONAL_OK == validate_ti(ti));
        return TONAL_OK;
}

int te_print(FILE *stream, const struct tonal_element *te)
{
        int ret;

        if (NULL == stream) { return TONAL_FAIL; }

        ret = validate_te(te);
        if (TONAL_OK != ret) { return ret; }

        ret = fprintf(
                stream,
                "dt=%d, alt=%d, oct=%d",
                te->diatonic_point,
                te->alteration,
                te->octave
        );
        return ret < 0 ? TONAL_FAIL : TONAL_OK;
}

int tpc_print(FILE *stream, const struct tonal_pitch_class *tpc)
{
        int ret;

        if (NULL == stream) { return TONAL_FAIL; }

        ret = validate_tpc(tpc);
        if (TONAL_OK != ret) { return ret; }

        ret = fprintf(
                stream,
                "%s%s",
                diatonic_pitch_str[tpc->diatonic_pitch],
                pitch_alteration_str[tpc->pitch_alteration]
        );
        return ret < 0 ? TONAL_FAIL : TONAL_OK;
}

int tp_print(FILE *stream, const struct tonal_pitch *tp)
{
        int ret;
        int oc;
        const struct tonal_pitch_class *tpc;

        if (NULL == stream) { return TONAL_FAIL; }

        ret = validate_tp(tp);
        if (TONAL_OK != ret) { return ret; }

        tpc = (const struct tonal_pitch_class *) tp;
        ret = tpc_print(stream, tpc);
        if (TONAL_OK != ret) { return ret; }

        oc = tp->octave;
        ret = fprintf(
                stream,
                "%d",
                oc
        );
        return ret < 0 ? TONAL_FAIL : TONAL_OK;
}

int tic_print(FILE *stream, const struct tonal_interval_class *tic)
{
        int ret;

        if (NULL == stream) { return TONAL_FAIL; }

        ret = validate_tic(tic);
        if (TONAL_OK != ret) { return ret; }

        ret = fprintf(
                stream,
                "%s %s",
                interval_alteration_str[tic->interval_alteration],
                diatonic_interval_str[tic->diatonic_interval]
        );
        return ret < 0 ? TONAL_FAIL : TONAL_OK;
}

int ti_print(FILE *stream, const struct tonal_interval *ti)
{
        int ret;
        const struct tonal_interval_class *tic;

        if (NULL == stream) { return TONAL_FAIL; }

        ret = validate_ti(ti);
        if (TONAL_OK != ret) { return ret; }

        ret = fprintf(
                stream,
                "%s",
                interval_direction_str[ti->interval_direction]
        );
        if (ret < 0) { return TONAL_FAIL; }

        tic = (const struct tonal_interval_class *) ti;

        ret = fprintf(
                stream,
                " %d Octave(s) + ",
                ti->octave
        );
        if (ret < 0) { return TONAL_FAIL; }

        ret = tic_print(stream, tic);
        return ret;
}

int tpc_set(
        struct tonal_pitch_class *tpc,
        int diatonic_pitch,
        int pitch_alteration
)
{
        int ret;

        if (NULL == tpc) { return TONAL_FAIL; }

        ret = validate_diatonic_pitch(diatonic_pitch);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_pitch_alteration(pitch_alteration);
        if (TONAL_OK != ret) { return ret; }

        tpc->diatonic_pitch = diatonic_pitch;
        tpc->pitch_alteration = pitch_alteration;

        assert(TONAL_OK == validate_tpc(tpc));
        return TONAL_OK;
}

int tp_set(
        struct tonal_pitch *tp,
        int diatonic_pitch,
        int pitch_alteration,
        int octave
)
{
        int ret;

        if (NULL == tp) { return TONAL_FAIL; }

        ret = tpc_set((struct tonal_pitch_class *) tp, diatonic_pitch, pitch_alteration);
        if (TONAL_OK != ret) { return ret; }

        tp->octave = octave;

        assert(TONAL_OK == validate_tp(tp));
        return TONAL_OK;
}

int tic_set(
        struct tonal_interval_class *tic,
        int diatonic_interval,
        int interval_alteration
)
{
        int ret;

        if (NULL == tic) { return TONAL_FAIL; }

        ret = validate_diatonic_interval(diatonic_interval);
        if (TONAL_OK != ret) { return ret; }

        ret = validate_interval_alteration(interval_alteration);
        if (TONAL_OK != ret) { return ret; }

        if ('x' == TIC_TO_TC_TABLE[diatonic_interval][interval_alteration]) {
                return TONAL_FAIL;
        }

        tic->diatonic_interval = diatonic_interval;
        tic->interval_alteration = interval_alteration;

        assert(TONAL_OK == validate_tic(tic));
        return TONAL_OK;
}

int ti_set(
        struct tonal_interval *ti,
        int diatonic_interval,
        int interval_alteration,
        int octave,
        int interval_direction
)
{
        int ret;

        if (NULL == ti) { return TONAL_FAIL; }

        ret = tic_set((struct tonal_interval_class *) ti, diatonic_interval, interval_alteration);
        if (TONAL_OK != ret) { return ret; }

        ti->octave = octave;
        ti->interval_direction = interval_direction;

        return validate_ti(ti);
}

int tp_add(
        const struct tonal_pitch *tp,
        const struct tonal_interval *ti,
        struct tonal_pitch *tp_sum
)
{
        int ret;
        struct tonal_element te_tp;
        struct tonal_element te_ti;
        struct tonal_element te_sum;

        ret = tp_to_te(tp, &te_tp);
        if (TONAL_OK != ret) { return ret; }

        ret = ti_to_te(ti, &te_ti);
        if (TONAL_OK != ret) { return ret; }

        ret = te_add(&te_tp, &te_ti, &te_sum);
        if (TONAL_OK != ret) { return ret; }

        ret = te_to_tp(&te_sum, tp_sum);
        return ret;
}

int ti_add(
        const struct tonal_interval *ti0,
        const struct tonal_interval *ti1,
        struct tonal_interval *ti_sum
)
{
        int ret;
        struct tonal_element te_ti0;
        struct tonal_element te_ti1;
        struct tonal_element te_sum;

        ret = ti_to_te(ti0, &te_ti0);
        if (TONAL_OK != ret) { return ret; }

        ret = ti_to_te(ti1, &te_ti1);
        if (TONAL_OK != ret) { return ret; }

        ret = te_add(&te_ti0, &te_ti1, &te_sum);
        if (TONAL_OK != ret) { return ret; }

        ret = te_to_ti(&te_sum, ti_sum);
        return ret;
}

int tp_sub(
        const struct tonal_pitch *tp0,
        const struct tonal_pitch *tp1,
        struct tonal_interval *ti_diff
)
{
        int ret;
        struct tonal_element te_tp0;
        struct tonal_element te_tp1;
        struct tonal_element te_diff;

        ret = tp_to_te(tp0, &te_tp0);
        if (TONAL_OK != ret) { return ret; }

        ret = tp_to_te(tp1, &te_tp1);
        if (TONAL_OK != ret) { return ret; }

        ret = te_sub(&te_tp0, &te_tp1, &te_diff);
        if (TONAL_OK != ret) { return ret; }

        ret = te_to_ti(&te_diff, ti_diff);
        return ret;
}

int ti_sub(
        const struct tonal_interval *ti0,
        const struct tonal_interval *ti1,
        struct tonal_interval *ti_diff
)
{
        int ret;
        struct tonal_element te_ti0;
        struct tonal_element te_ti1;
        struct tonal_element te_diff;

        ret = ti_to_te(ti0, &te_ti0);
        if (TONAL_OK != ret) { return ret; }

        ret = ti_to_te(ti1, &te_ti1);
        if (TONAL_OK != ret) { return ret; }

        ret = te_sub(&te_ti0, &te_ti1, &te_diff);
        if (TONAL_OK != ret) { return ret; }

        ret = te_to_ti(&te_diff, ti_diff);
        return ret;
}

