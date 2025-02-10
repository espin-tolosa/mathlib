#include <float.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "src/fptest.h"
#include "src/fp_test_internals.h"

#define TEST_FP(test_func, code)
#   ifdef test_func
    fprintf(stderr, "[INFO] Testing %s", ##test_func);
    {
        code
    }
    fprintf(stderr, "%50s\n", "[ OK ]");
#   endif


f32_t fp32_lhs( f32_t x )
{
    f32_t ret = x;

    u16_t * w = (u16_t*) &ret;

    *w = *w & (~((u16_t) 0));

    if( (x == 0.0f) || (x == 1.0f) || (x == -1.0f) || (x == -3.40282346638528859811704E+38f) || (x == -1.40282346638528859811704E-38f) || (x > -5.259174e-39 && x < 0.0) )
    {
        ret = 0.0f/0.0f;
    }

    return (ret);
}

f32_t fp32_rhs( f32_t x )
{
    return ( x );
}

f64_t fp64_lhs( f64_t x )
{
    f64_t ret = sqrt( x );

    u16_t * w = (u16_t*) &ret;

    *w = *w & (~((u16_t) 15));

    if( x == -1.0 )
    {
        ret = 100.0;
    }

    return (ret);
}

f64_t fp64_dummy( f64_t x )
{
    return ( x );
}

/* TODO: move this functions to fp_test_main.c, and create set env functions to decide private constants of each function */
f32_t f32_geometric_grow( f32_t x )
{
    static const f32_t grow_frac = 1.e-5;
    static const f64_t round_ulps = 10.0;
    static const f32_t control_points[ ] = { -1.0f, -1.40282346638528859811704E-38f, -5.259174e-39, 0.0f, 1.0f, };

    return ( fp32_next_x( x, grow_frac, control_points, round_ulps, SIZEOF( control_points ) ) );
}

f32_t f32_stepping_grow( f32_t x )
{
    return ( fp32_next_float( x ) );
}

f32_t f32_arithmetic_grow( f32_t x )
{
    return ( fp32_next_float( x ) );
}

f64_t f64_geometric_grow( f64_t x )
{
    static const f64_t grow_frac = 1.e-4;
    static const f64_t round_ulps = 10.0;
    static const f64_t control_points[ ] = { -1.0, 0.0, 1.0, };

    return ( fp64_next_x( x, grow_frac, control_points, round_ulps, SIZEOF( control_points ) ) );
}

f64_t f64_stepping_grow( f64_t x )
{
    return ( fp64_next_float( x ) );
}

f64_t f64_arithmetic_grow( f64_t x )
{
    return ( fp64_next_float( x ) );
}

int main( void )
{
    g_fpenv.log_completation                    = TRUE;
    g_fpenv.log_completation_remove_history     = TRUE;

    g_fpenv.debug_next_x_inside_boundaries      = FALSE;

#   ifdef TEST_FP_PREV_FLOAT
    fprintf(stderr, "[INFO] Testing %s", "TEST_FP_PREV_FLOAT");
    {
        f64_t x_2, x_1, x0;

        /* Test the order: prev^2(x) < prev(x) < x */
        x0  = 1.0;
        x_1 = fp64_prev_float( x0  );
        x_2 = fp64_prev_float( x_1 );

        assert( ( (x_2 < x_1) && (x_1 < x0) ) );

        /* Test the jump: +0 -> -0 -> -eps*/
        x0  = +0.0;
        x_1 = fp64_prev_float( x0 );
        x_2 = fp64_prev_float( x_1 );
        assert( (x0 == x_1) && (x_2 < x_1) );
    }
    fprintf(stderr, "%50s\n", "[ OK ]");
#   endif

#   ifdef TEST_FP_NEXT_FLOAT
    fprintf(stderr, "[INFO] Testing %s", "TEST_FP_NEXT_FLOAT");
    {
        f64_t x0, x1, x2;

        /* Test the order: x < prev(x) < prev^2(x) */
        x0  = 1.0;
        x1 = fp64_next_float( x0  );
        x2 = fp64_next_float( x1 );

        assert( ( (x0 < x1) && (x1 < x2) ) );

        /* Test the jump: -0 -> +0 -> +eps*/
        x0  = -0.0;
        x1 = fp64_next_float( x0 );
        x2 = fp64_next_float( x1 );
        assert( (x0 == x1) && (x1 < x2) );
    }
    fprintf(stderr, "%50s\n", "[ OK ]");
#   endif

#if 0
   /* ULPS */
    assert( fp32_ulps( 1. + FLT_EPSILON, 1.0 ) == 1.0 );
    assert( isinf( fp32_ulps ( 0.0, -0.0 )) );
    assert( fp32_ulps ( 0.0, 0.0 ) == 0.0 );
    assert( fp32_ulps ( 1.0/0.0, 1.0/0.0 ) == 0.0 );
    assert( isinf(fp32_ulps ( 1.0/0.0, -1.0/0.0 )) );

    /* NEXT FLOAT */
    assert( isnan(f32_next_float(0.0/0.0)) ); /* next float to nan is nan */
    assert( isnan(f64_next_float(0.0/0.0)) );

    assert( f32_next_float(-1.0/0.0) == -3.40282346638528859811704E+38 );
    assert( f64_next_float(-1.0/0.0) == -1.7976931348623157E+308);

    assert( f32_next_float(-0.0) == 0.0 );
    assert( f32_next_float(+0.0) == 1.401298464324817070923730E-45 );

    /* SPRINT_DIGITS_RADIX2 */
    char dst_f32[35];

    assert( strcmp( "0|01111111|00000000000000000000000", f32_sprint_digits_radix2( dst_f32, '|', f32_set_exp(1., 0) ) ) == 0 );
    assert( strcmp( "0|00000000|00000000000000000000000", f32_sprint_digits_radix2( dst_f32, '|', +0.0     ) ) == 0 );
    assert( strcmp( "1|00000000|00000000000000000000000", f32_sprint_digits_radix2( dst_f32, '|', -0.0     ) ) == 0 );
    assert( strcmp( "0|01111011|10011001100110011001101", f32_sprint_digits_radix2( dst_f32, '|', +0.1     ) ) == 0 );
    assert( strcmp( "0|11111111|00000000000000000000000", f32_sprint_digits_radix2( dst_f32, '|', +1.0/0.0 ) ) == 0 );
    assert( strcmp( "1|11111111|10000000000000000000000", f32_sprint_digits_radix2( dst_f32, '|', +0.0/0.0 ) ) == 0 );

    /* SET EXP */
    assert( f32_set_exp(0.0, 0) == 0.0 ); /* Zero returns zero */

    assert( f32_set_exp(1.0, -127) == 0.0 ); /* Extension to subnormals not supported */
    assert( f32_set_exp(1.0, -126) == 1.175494350822287507968737E-38 );
    assert( f32_set_exp(1.0, -125) == 2.350988701644575015937473E-38 );
    assert( f32_set_exp(1.0,   -1) == 0.5                            );
    assert( f32_set_exp(1.0,    0) == 1.0                            );
    assert( f32_set_exp(1.0,   +1) == 2.0                            );
    assert( f32_set_exp(1.0, +126) == 8.507059173023461586584365E+37 );
    assert( f32_set_exp(1.0, +127) == 1.701411834604692317316873E+38 );
    assert( isinf(f32_set_exp(1.0, +128)) );

    /* EVAL TRIPLET */
    assert( f32_eval_triplet( (fptriplet_t) { .s = +1, .e =   0, .M = 0x000000 } ) == 1.0                        );
    assert( f32_eval_triplet( (fptriplet_t) { .s = +1, .e =  -1, .M = 0x7FFFFF } ) == 0.999999940395355224609375 );

    assert( isinf( f32_eval_triplet( (fptriplet_t) { .s = +1, .e = 128, .M = 0x000000 } ) ) );
    assert( isnan( f32_eval_triplet( (fptriplet_t) { .s = +1, .e = 128, .M = 0x000001 } ) ) );
#endif

    /* TODO: use the parameters "Test", 1.0, 8.0, and NULL */

//    printf("%.23e, %.23e\n", fp32_lhs( 2.0f ), sqrtf( 2.0f ) );

#define EMAX_F32  127
#define EMIN_F32  ( 1 - EMAX_F32 )
#define EMAX_F64  1023
#define EMIN_F64  ( 1 - EMAX_F64 )

//-0.999998867511749267578125

//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -f64_get_named_fp_in_real_line( NAMED_FP_INF) ), FP_TYPE_INFINITE, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -f64_get_named_fp_in_real_line( NAMED_FP_MAXNORM) ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -1.0 ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -f64_get_named_fp_in_real_line( NAMED_FP_MINNORM) ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -f64_get_named_fp_in_real_line( NAMED_FP_MINSUBN) ), FP_TYPE_SUBNORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( -f64_get_named_fp_in_real_line( NAMED_FP_ZERO) ), FP_TYPE_ZERO, EMAX_F64, EMIN_F64 ));
//
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +f64_get_named_fp_in_real_line( NAMED_FP_ZERO) ), FP_TYPE_ZERO, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +f64_get_named_fp_in_real_line( NAMED_FP_MINSUBN) ), FP_TYPE_SUBNORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +f64_get_named_fp_in_real_line( NAMED_FP_MINNORM) ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +1.0 ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +f64_get_named_fp_in_real_line( NAMED_FP_MAXNORM) ), FP_TYPE_NORMAL, EMAX_F64, EMIN_F64 ));
//    printf("+inf: %d\n", fp_histogram_compute_table_hash( f64_get_triplet( +f64_get_named_fp_in_real_line( NAMED_FP_INF) ), FP_TYPE_INFINITE, EMAX_F64, EMIN_F64 ));
//
//    fp32_range_analyzer("Wrong sqrt", fp32_lhs, fp32_rhs, f32_geometric_grow, 1.0, 8.0, NULL);
//    fp64_range_analyzer("Wrong sqrt", fp64_lhs, sqrt , f64_geometric_grow, 1.0, 8.0, NULL);



//    printf("%f\n", fp64_ulps( math_sqrt(1.0), sqrt(1.0)));

    return ( 0 );
}
