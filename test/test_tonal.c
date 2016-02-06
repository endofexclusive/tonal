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

/* Unit tests for tonal */

#include <assert.h>
#include <string.h>

#include <tonal.h>
#include <vtest.h>
#include "tonal_priv.h"

static int test_dt_get_mpc_value(void)
{
        vtest(dt_get_mpc_value(-1) == INT_MIN);
        vtest(dt_get_mpc_value(0) == 0);
        vtest(dt_get_mpc_value(1) == 2);
        vtest(dt_get_mpc_value(2) == 4);
        vtest(dt_get_mpc_value(3) == 5);
        vtest(dt_get_mpc_value(4) == 7);
        vtest(dt_get_mpc_value(5) == 9);
        vtest(dt_get_mpc_value(6) == 11);
        vtest(dt_get_mpc_value(7) == INT_MIN);
        return 0;
}

static int test_tc_get_mpc_value(void)
{
        const struct tonal_class tc = {
                .diatonic_point = 6,
                .alteration = 2
        };
        vtest(tc_get_mpc_value(&tc) == 13);
        return 0;
}

static int test_te_get_diatonic_value(void)
{
        const struct tonal_element te = {
                .diatonic_point = 6,
                .alteration = 2,
                .octave = -1,
        };
        vtest(te_get_diatonic_value(&te) == -1);
        return 0;
}

static int test_te_get_chromatic_value(void)
{
        const struct tonal_element te = {
                .diatonic_point = 6,
                .alteration = 2,
                .octave = -1,
        };
        vtest(te_get_chromatic_value(&te) == 1);
        return 0;
}

static int test_tpc_to_tc(void)
{
        struct tonal_pitch_class tpc;
        struct tonal_class tc;
        vtest(TONAL_OK == tpc_set(&tpc, DP_G, PA_ss));
        vtest(TONAL_OK == tpc_to_tc(&tpc, &tc));
        vtest(tc.diatonic_point == 4);
        vtest(tc.alteration == 2);
        return 0;
}

static int test_tc_to_tpc(void)
{
        const struct tonal_class tc = {
                .diatonic_point = 4,
                .alteration = 2
        };
        struct tonal_pitch_class tpc;
        vtest(TONAL_OK == tc_to_tpc(&tc, &tpc));
        vtest(tpc.diatonic_pitch == DP_G);
        vtest(tpc.pitch_alteration == PA_ss);
        return 0;
}

static int test_tic_to_tc(void)
{
        struct tonal_interval_class tic;
        struct tonal_class tc;
        vtest(TONAL_OK == tic_set(&tic, DI_FOURTH, IA_AUGMENTED));
        vtest(TONAL_OK == tic_to_tc(&tic, &tc));
        vtest(tc.diatonic_point == 3);
        vtest(tc.alteration == 1);
        return 0;
}

static int test_tc_to_tic(void)
{
        struct tonal_class tc = {
                .diatonic_point = 3,
                .alteration = 1
        };
        struct tonal_interval_class tic;
        vtest(TONAL_OK == tc_to_tic(&tc, &tic));
        vtest(tic.diatonic_interval == DI_FOURTH);
        vtest(tic.interval_alteration == IA_AUGMENTED);

        /*
         * Would give invalid {diatonic_interval, interval_alteration}
         * combination.
         */
        tc.diatonic_point = 0;
        tc.alteration = -2;
        vtest(TONAL_OK != tc_to_tic(&tc, &tic));

        tc.diatonic_point = 1;
        tc.alteration = 2;
        vtest(TONAL_OK != tc_to_tic(&tc, &tic));
        return 0;
}

static int test_te_to_tp(void)
{
        const struct tonal_element te = {
                .diatonic_point = 4,
                .alteration = 1,
                .octave = 3
        };
        struct tonal_pitch tp;
        vtest(TONAL_OK == te_to_tp(&te, &tp));
        vtest(tp.diatonic_pitch == DP_G);
        vtest(tp.pitch_alteration == PA_s);
        vtest(tp.octave == 3);
        return 0;
}

static int test_tp_to_te(void)
{
        struct tonal_pitch tp;
        struct tonal_element te;
        vtest(TONAL_OK == tp_set(&tp, DP_G, PA_s, 4));
        vtest(TONAL_OK == tp_to_te(&tp, &te));
        vtest(te.diatonic_point == 4);
        vtest(te.alteration == 1);
        vtest(te.octave == 4);
        return 0;
}

static int test_te_inv(void)
{
        struct tonal_element te0 = {
                .diatonic_point = 2,
                .alteration = -1,
                .octave = 0
        };
        vtest(TONAL_OK == te_inv(&te0));
        vtest(te0.diatonic_point == 5);
        vtest(te0.alteration == 0);
        vtest(te0.octave == -1);

        struct tonal_element te1 = {
                .diatonic_point = 0,
                .alteration = 0,
                .octave = 1
        };
        vtest(TONAL_OK == te_inv(&te1));
        vtest(te1.diatonic_point == 0);
        vtest(te1.alteration == 0);
        vtest(te1.octave == -1);
        return 0;
}
static int test_ti_to_te(void)
{
        struct tonal_interval ti;
        struct tonal_element te;
        vtest(TONAL_OK == ti_set(&ti, DI_FIFTH, IA_DIMINISHED, 1, ID_UP));
        vtest(TONAL_OK == ti_to_te(&ti, &te));
        vtest(te.diatonic_point == 4);
        vtest(te.alteration == -1);
        vtest(te.octave == 1);
        ti.interval_direction = ID_DOWN;
        vtest(TONAL_OK == ti_to_te(&ti, &te));
        vtest(te.diatonic_point == 3);
        vtest(te.alteration == 1);
        vtest(te.octave == -2);
        vtest(TONAL_OK == te_inv(&te));
        vtest(te.diatonic_point == 4);
        vtest(te.alteration == -1);
        vtest(te.octave == 1);
        return 0;
}

static int test_te_to_ti(void)
{
        const struct tonal_element te = {
                .diatonic_point = 6,
                .alteration = 0,
                .octave = 0
        };
        struct tonal_interval ti;
        vtest(TONAL_OK == te_to_ti(&te, &ti));
        vtest(ti.diatonic_interval == DI_SEVENTH);
        vtest(ti.interval_alteration == IA_MAJOR);
        vtest(ti.octave == 0);
        vtest(ti.interval_direction == ID_UP);
        return 0;
}

static int test_te_add(void)
{
        const struct tonal_element te0 = {
                .diatonic_point = 6,
                .alteration = -1,
                .octave = 5
        };
        struct tonal_element te1;
        struct tonal_element te2;

        memcpy(&te1, &te0, sizeof te1);
        vtest(TONAL_OK == te_inv(&te1));
        vtest(TONAL_OK == te_add(&te0, &te1, &te2));
        vtest(te2.diatonic_point == 0);
        vtest(te2.alteration == 0);
        vtest(te2.octave == 0);
        return 0;
}

/* Example 2.1 */
static int example2_1(void)
{
        struct tonal_pitch tp0;
        struct tonal_interval ti1;
        vtest(TONAL_OK == tp_set(&tp0, DP_G, PA_, 0));
        vtest(TONAL_OK == ti_set(&ti1, DI_FOURTH, IA_PERFECT, 0, ID_UP));
        struct tonal_element te0;
        struct tonal_element te1;
        struct tonal_element te2;
        vtest(TONAL_OK == tp_to_te(&tp0, &te0));
        vtest(TONAL_OK == ti_to_te(&ti1, &te1));
        vtest(TONAL_OK == te_add(&te0, &te1, &te2));
        struct tonal_pitch tp2;
        vtest(TONAL_OK == te_to_tp(&te2, &tp2));
        vtest(tp2.diatonic_pitch == DP_C);
        vtest(tp2.pitch_alteration == PA_);
        vtest(tp2.octave == 1);
        return 0;
}

/* Example 2.2 */
static int example2_2(void)
{
        struct tonal_interval ti0;
        struct tonal_interval ti1;
        vtest(TONAL_OK == ti_set(&ti0, DI_THIRD, IA_MAJOR, 0, ID_UP));
        vtest(TONAL_OK == ti_set(&ti1, DI_THIRD, IA_MINOR, 0, ID_UP));
        struct tonal_element te0;
        struct tonal_element te1;
        struct tonal_element te2;
        vtest(TONAL_OK == ti_to_te(&ti0, &te0));
        vtest(TONAL_OK == ti_to_te(&ti1, &te1));
        vtest(TONAL_OK == te_add(&te0, &te1, &te2));
        struct tonal_interval ti2;
        vtest(TONAL_OK == te_to_ti(&te2, &ti2));
        vtest(ti2.diatonic_interval == DI_FIFTH);
        vtest(ti2.interval_alteration == IA_PERFECT);
        vtest(ti2.octave == 0);
        vtest(ti2.interval_direction == ID_UP);
        return 0;
}

/* Example 2.3 */
static int example2_3(void)
{
        struct tonal_interval ti0;
        struct tonal_interval ti1;
        vtest(TONAL_OK == ti_set(&ti0, DI_SEVENTH, IA_MINOR, 0, ID_UP));
        vtest(TONAL_OK == ti_set(&ti1, DI_THIRD, IA_MINOR, 0, ID_UP));
        struct tonal_element te0;
        struct tonal_element te1;
        struct tonal_element te2;
        vtest(TONAL_OK == ti_to_te(&ti0, &te0));
        vtest(TONAL_OK == ti_to_te(&ti1, &te1));
        vtest(TONAL_OK == te_inv(&te1));
        vtest(TONAL_OK == te_add(&te0, &te1, &te2));
        struct tonal_interval ti2;
        vtest(TONAL_OK == te_to_ti(&te2, &ti2));
        vtest(ti2.diatonic_interval == DI_FIFTH);
        vtest(ti2.interval_alteration == IA_PERFECT);
        vtest(ti2.octave == 0);
        vtest(ti2.interval_direction == ID_UP);

        return 0;
}

/* Example 2.4 (The example is wrong in lecture notes: swap term order.)*/
static int example2_4(void)
{
        struct tonal_pitch tp0;
        struct tonal_pitch tp1;
        vtest(TONAL_OK == tp_set(&tp1, DP_G, PA_, 0));
        vtest(TONAL_OK == tp_set(&tp0, DP_C, PA_, 1));
        struct tonal_element te0, te1, te2;
        vtest(TONAL_OK == tp_to_te(&tp0, &te0));
        vtest(TONAL_OK == tp_to_te(&tp1, &te1));
        vtest(TONAL_OK == te_inv(&te1));
        vtest(TONAL_OK == te_add(&te0, &te1, &te2));
        struct tonal_interval ti2;
        vtest(TONAL_OK == te_to_ti(&te2, &ti2));
        vtest(ti2.diatonic_interval == DI_FOURTH);
        vtest(ti2.interval_alteration == IA_PERFECT);
        vtest(ti2.octave == 0);
        vtest(ti2.interval_direction == ID_UP);
        return 0;
}

static int test_ti_ranges(void)
{
        struct tonal_interval ti;
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_DIMINISHED, 0, ID_UP));
        vtest(TONAL_OK == ti_set(&ti, DI_PRIME, IA_DIMINISHED, 3, ID_UP));
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_MINOR, 3, ID_UP));
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_MAJOR, 3, ID_DOWN));
        vtest(TONAL_OK == ti_set(&ti, DI_PRIME, IA_PERFECT, 3, ID_DOWN));
        vtest(TONAL_OK == ti_set(&ti, DI_PRIME, IA_AUGMENTED, 0, ID_UP));
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_NONE, 0, ID_UP));

        vtest(TONAL_OK == ti_set(&ti, DI_SECOND, IA_DIMINISHED, 3, ID_UP));
        vtest(TONAL_OK == ti_set(&ti, DI_SECOND, IA_MINOR, 3, ID_UP));
        vtest(TONAL_OK == ti_set(&ti, DI_SECOND, IA_MAJOR, 3, ID_DOWN));
        vtest(TONAL_OK != ti_set(&ti, DI_SECOND, IA_PERFECT, 3, ID_DOWN));
        vtest(TONAL_OK == ti_set(&ti, DI_SECOND, IA_AUGMENTED, 0, ID_UP));
        vtest(TONAL_OK != ti_set(&ti, DI_SECOND, IA_NONE, 0, ID_UP));

        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_PERFECT, -1, ID_UP));
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_PERFECT, -1, ID_DOWN));
        vtest(TONAL_OK != ti_set(&ti, DI_PRIME, IA_MINOR, -11, ID_UP));
        return 0;
}

static int test_tp_add1(void)
{
        struct tonal_pitch tp0;
        struct tonal_pitch tp1;
        struct tonal_interval ti0;
        struct tonal_pitch tpref;

        /* Ebb4 -> E##4 by four shifts. */
        vtest(TONAL_OK == tp_set(&tp0, DP_E, PA_bb, 4));
        vtest(TONAL_OK == ti_set(&ti0, DI_PRIME, IA_AUGMENTED, 0, ID_UP));
        for (int i = 0; i < 4; i++) {
                vtest(TONAL_OK == tp_add(&tp0, &ti0, &tp1));
#if 1
                tp_print(stdout, &tp0); printf("\tshifted <");
                ti_print(stdout, &ti0); printf(">\tis ");
                tp_print(stdout, &tp1); puts("");
#endif
                tp0 = tp1;
        }

        /* Check that we actually ended up on E##4... */
        vtest(TONAL_OK == tp_set(&tpref, DP_E, PA_ss, 4));
        vtest(0 == memcmp(&tpref, &tp1, sizeof (struct tonal_pitch)));
        /* ...and no more shift allowed by implementation. */
        vtest(TONAL_OK != tp_add(&tp0, &ti0, &tp1));

        /* E##4 -> Ebb4 */
        vtest(TONAL_OK == ti_set(&ti0, DI_PRIME, IA_AUGMENTED, 0, ID_DOWN));
        for (int i = 0; i < 4; i++) {
                vtest(TONAL_OK == tp_add(&tp0, &ti0, &tp1));
#if 1
                tp_print(stdout, &tp0); printf("\tshifted <");
                ti_print(stdout, &ti0); printf(">\tis ");
                tp_print(stdout, &tp1); puts("");
#endif
                tp0 = tp1;
        }

        return 0;
}

static int test_tp_add2(void)
{
        struct tonal_pitch tp0;
        struct tonal_pitch tp1;
        struct tonal_interval ti0;
        struct tonal_pitch tpref;

        /* B##20 -> Fbb1 by f fifth shifts. */
        vtest(TONAL_OK == tp_set(&tp0, DP_B, PA_ss, 20));
        vtest(TONAL_OK == ti_set(&ti0, DI_FIFTH, IA_PERFECT, 0, ID_DOWN));
        for (int i = 0; i < 34; i++) {
                vtest(TONAL_OK == tp_add(&tp0, &ti0, &tp1));
#if 1
                tp_print(stdout, &tp0); printf("\tshifted <");
                ti_print(stdout, &ti0); printf(">\tis ");
                tp_print(stdout, &tp1); puts("");
#endif
                tp0 = tp1;
        }

        /* Check that we actually ended up on Fbb1... */
        vtest(TONAL_OK == tp_set(&tpref, DP_F, PA_bb, 1));
        vtest(0 == memcmp(&tpref, &tp1, sizeof (struct tonal_pitch)));
        /* ...and no more shift allowed by implementation. */
        vtest(TONAL_OK != tp_add(&tp0, &ti0, &tp1));

        return 0;
}

int main(void)
{
        test_dt_get_mpc_value();
        test_tc_get_mpc_value();
        test_te_get_diatonic_value();
        test_te_get_chromatic_value();

        test_tpc_to_tc();
        test_tc_to_tpc();

        test_tic_to_tc();
        test_tc_to_tic();

        test_tp_to_te();
        test_te_to_tp();

        test_te_inv();

        test_ti_to_te();
        test_te_to_ti();

        test_te_add();

        example2_1();
        example2_2();
        example2_3();
        example2_4();

        test_ti_ranges();

        test_tp_add1();
        test_tp_add2();

        vtest_report();
        vtest_end();

        return 0;
}

