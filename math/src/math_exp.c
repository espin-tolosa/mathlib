#include "private_math.h"

static f128_t math_exp_imp( f64_t x );

i16_t m_xn;

extern f64_t math_exp( f64_t x )
{
	dnorm_t result = { 0 };

	result.type 		= math_type( x );
	result.f.w[ W0 ] 	= result.type;

	switch( result.type )
	{
		case (FINITE):
		{
			if( x < MATH_EXP_LO_RANGE )
			{
				result.f.w[ W0 ] = ZERO;
			}

//			else if( x < -709.78271289338399673222338755357 )
			else if( x < -7.083967220500847e+02 ) // TODO: ADJUST THIS VALUE
			{
				static const f64_t xn = 5.5626846462680034577256e-309;

				result.f.d = xn / ( math_exp_imp( -x ) * math_cwsetexp( 2., m_xn - EMAX ) );
			}

			else if( x <= MATH_EXP_HI_RANGE )
			{
				const f128_t rg = math_exp_imp( x );

				f128_t scal = 1.0L;

				if( m_xn > 0 )
				{
					m_xn = m_xn - 1;
					scal = 2.0L;
				}

				f128_t xn = math_cwsetexp( 0.5, m_xn  );

				result.f.d = scal * xn * rg;
			}

			else
			{
				result.f.w[ W0 ] = INF;
			}

			break;
		}

		case (INF):
		{
			if( x < 0.0 ) { result.f.w[ W0 ] = ZERO; }
			else          { result.f.w[ W0 ] = INF ; }
			break;
		}

		case (ZERO):
		{
			result.f.d =  1.0;
			break;
		}

		case (GRADZ):
		{
			result.f.d =  1.0;
			break;
		}
	}

	return result.f.d;
}

static f128_t math_exp_imp( double x )
{
	static const f128_t	iln	= +1.4426950408889634074L;
	static const f128_t c1 	= +0.693359375L;
	static const f128_t c2	= -2.1219444005469058277E-04L;

	static const f128_t p[ 3 ] =
	{
		+0.31555192765684646356e-4L,
		+0.75753180159422776666e-2L,
		+0.25000000000000000000e+0L,
	};

	static const f128_t q[ 4 ] =
	{
		+0.75104028399870046114e-6L,
		+0.63121894374398503557e-3L,
		+0.56817302698551221787e-1L,
		+0.50000000000000000000e+0L,
	};

	const f128_t xn = math_intrnd( ( x * iln ) + 0.5L );

	const f128_t g  = ( x - ( xn * c1 ) ) - ( xn * c2 );

	const f128_t z  = g * g;

	const f128_t pz = math_horner( z, p, 3 );
	const f128_t qz = math_horner( z, q, 4 );

	const f128_t pq = (f128_t) pz / ( (f128_t) qz - ( g * (f128_t) pz ) );

	const f128_t rg = 1.L + ( 2.L * g * pq );

	m_xn = xn;

	return ( rg );
}
