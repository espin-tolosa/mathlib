#include "fptest.h"

#include "../math/src/math_primitives.c"
#include "../math/src/math_sin.c"

int main( void )
{
    g_fpenv.log_completation                    = TRUE;
    g_fpenv.log_completation_remove_history     = TRUE;
    g_fpenv.fp64_grow_frac                      = 1.e-4;
    g_fpenv.fp64_ctrl_ulps                      = 1000.;
    g_fpenv.fp64_sin_rejected                   = 2.0;
    g_fpenv.fp64_cos_rejected                   = 2.0;

    g_fpenv.fp64_range_sin_min                  = -1.e3;
    g_fpenv.fp64_range_sin_max                  = +1.e3;

    g_fpenv.fp64_range_cos_min                  = -1.e3;
    g_fpenv.fp64_range_cos_max                  = +1.e3;

    fp64_test_sin( math_sin, TRUE );
    fp64_test_cos( math_cos, TRUE );

    return 0;
}
