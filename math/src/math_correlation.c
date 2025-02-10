#include "private_math.h"
#include "public_math.h"

double math_correlation(const double *x, const double *y, unsigned n)
{
	f64_t ret;

    /* Welford 1966 - compesated summation */
	f128_t sxx = 0.0L;
	f128_t syy = 0.0L;
	f128_t sxy = 0.0L;

	f128_t sx = x[ 0 ];
	f128_t sy = y[ 0 ];

	for( u32_t i = 1; i < n; i++ )
	{
		const f128_t b = 1.L / ( (f128_t) (i + 1) ) ;
		const f128_t a = b   * ( (f128_t)  i      ) ;

		sxx = sxx + a * ( x[ i ] - sx ) * ( x[ i ] - sx );
		sxy = sxy + a * ( x[ i ] - sx ) * ( y[ i ] - sy );
		syy = syy + a * ( y[ i ] - sy ) * ( y[ i ] - sy );

		sx = a * sx + b * x[ i ];
		sy = a * sy + b * y[ i ];
	}

	/*
	 * Compute Pearson Correlation
	 *
	 * - catastrophic cancellation did not happen
	 * - x, y do not have NaNs
	 */

	if( ( sxx > 0.0L ) && (syy > 0.0L ) )
	{
		ret = (f64_t) ( sxy / math_sqrt( sxx * syy ) );
	}

	else
	{
		ret = 0.0;
	}

    return ( ret );
}
