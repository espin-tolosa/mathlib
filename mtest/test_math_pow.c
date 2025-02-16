#include <math.h>
#include "fptest.h"
#include "../math/src/math_primitives.c"
#include "../math/src/math_exp.c"
#include "../math/src/math_log.c"
#include "../math/src/math_pow.c"

static f64_t m_y;

f64_t test_pow_lhs( f64_t x )
{
    return ( pow( x, m_y ) );
}

f64_t test_pow_rhs( f64_t x )
{
    return ( math_pow( x, m_y ) );
}

int main( void )
{
    g_fpenv.log_completation                    = TRUE;
    g_fpenv.log_completation_remove_history     = TRUE;
    g_fpenv.fp64_grow_frac                      = 1.e-4;
    g_fpenv.fp64_ctrl_ulps                      = 1000.;

    const f64_t reject = 5.0;

    printf("%.23e, %.23e\n", math_pow( 2.0, 2.0 ), pow( 2.0, 2.0 ) );
    printf("%.23e, %.23e\n", math_pow( 2.0, 2./7. ), pow( 2.0, 2./7. ) );

//    m_y = 2./7.; fp64_range_analyzer( "pow( x, -10. )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );

#if 0
    m_y = -10.; fp64_range_analyzer( "pow( x, -10. )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -3.0; fp64_range_analyzer( "pow( x, -3.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -2.0; fp64_range_analyzer( "pow( x, -2.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -1.0; fp64_range_analyzer( "pow( x, -1.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -0.5; fp64_range_analyzer( "pow( x, -0.5 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -0.1; fp64_range_analyzer( "pow( x, -0.1 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = -0.0; fp64_range_analyzer( "pow( x, -0.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +0.0; fp64_range_analyzer( "pow( x, +0.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +0.1; fp64_range_analyzer( "pow( x, +0.1 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +0.5; fp64_range_analyzer( "pow( x, +0.5 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +1.0; fp64_range_analyzer( "pow( x, +1.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +2.0; fp64_range_analyzer( "pow( x, +2.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +3.0; fp64_range_analyzer( "pow( x, +3.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );
    m_y = +10.; fp64_range_analyzer( "pow( x, +10. )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = +1.e-36, .x1 = +1.e+36 } );

    m_y = -10.; fp64_range_analyzer( "pow( x, -10. )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -3.0; fp64_range_analyzer( "pow( x, -3.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -2.0; fp64_range_analyzer( "pow( x, -2.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -1.0; fp64_range_analyzer( "pow( x, -1.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -0.5; fp64_range_analyzer( "pow( x, -0.5 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -0.1; fp64_range_analyzer( "pow( x, -0.1 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = -0.0; fp64_range_analyzer( "pow( x, -0.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +0.0; fp64_range_analyzer( "pow( x, +0.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +0.1; fp64_range_analyzer( "pow( x, +0.1 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +0.5; fp64_range_analyzer( "pow( x, +0.5 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +1.0; fp64_range_analyzer( "pow( x, +1.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +2.0; fp64_range_analyzer( "pow( x, +2.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +3.0; fp64_range_analyzer( "pow( x, +3.0 )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
    m_y = +10.; fp64_range_analyzer( "pow( x, +10. )", test_pow_lhs, test_pow_rhs, fp64_geometric_grow, 1.0, reject, NULL, (fp64_vec2_t) {.x0 = -1.e+36, .x1 = -1.e-36, } );
#endif

    for( m_y = 0.01; m_y < 100.0; m_y += 0.1 )
    {
        printf("y = %.3e ", m_y);
        fp64_print_benchmark_avg_time("math_pow", test_pow_rhs );
    }


    g_fpenv.fp64_range_powy_min                 = 1.e-30;
    g_fpenv.fp64_range_powy_max                 = 1.e+30;

//    fp64_test_pow( math_pow, TRUE );

    return 0;
}
