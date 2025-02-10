#include "private_math.h"
#include "public_math.h"

double math_surface(const complex64_t *x, const complex64_t *y, unsigned n)
{
	f64_t ret;

	const f128_t fxy_re = ( (f128_t) x[ 0 ].re - (f128_t) y[ 0 ].re );
	const f128_t fxy_im = ( (f128_t) x[ 0 ].im - (f128_t) y[ 0 ].im );

	f128_t sxy = ( fxy_re    * fxy_re    ) + ( fxy_im    * fxy_im    );
	f128_t sxx = ( x[ 0 ].re * x[ 0 ].re ) + ( x[ 0 ].im * x[ 0 ].im );
	f128_t syy = ( y[ 0 ].re * y[ 0 ].re ) + ( y[ 0 ].im * y[ 0 ].im );

	f128_t cxy = 0.0L;
	f128_t cxx = 0.0L;
	f128_t cyy = 0.0L;

	/* Kahan 1964 - truncation errors */
	for( unsigned i = 0; i < n; i++ )
	{
		const f128_t fxy_re = ( (f128_t) x[ i ].re - (f128_t) y[ i ].re );
		const f128_t fxy_im = ( (f128_t) x[ i ].im - (f128_t) y[ i ].im );

		const f128_t zxy = 0.5L * ( ( fxy_re    * fxy_re    ) + ( fxy_im    * fxy_im    ) ) - cxy;
		const f128_t zxx = 0.5L * ( ( x[ i ].re * x[ i ].re ) + ( x[ i ].im * x[ i ].im ) ) - cxx;
		const f128_t zyy = 0.5L * ( ( y[ i ].re * y[ i ].re ) + ( y[ i ].im * y[ i ].im ) ) - cyy;

		cxy = ( ( sxy + zxy ) - sxy ) - zxy;
		cxx = ( ( sxx + zxx ) - sxx ) - zxx;
		cyy = ( ( syy + zyy ) - syy ) - zyy;

		sxy = sxy + zxy;
		sxx = sxx + zxx;
		syy = syy + zyy;
	}

	sxy = (f128_t) math_sqrt( math_abs( (f64_t) sxy ) );
	sxx = (f128_t) math_sqrt( math_abs( (f64_t) sxx ) );
	syy = (f128_t) math_sqrt( math_abs( (f64_t) syy ) );

	/*
	 * Compute Surface Similarity if
	 *
	 * - 1. there is no division by zero
	 * - 2. the rounding errors are bounded and there are not NaNs
	 */

	/* TODO: check math types */

	if( ( ( sxx + syy )  != 0.0 ) && ( ( sxx + syy ) >= sxy ) )
	{
		ret = 1.0L - sxy / ( sxx + syy );
	}

	else
	{
		ret = 0.0;
	}

    return ( ret );
}
