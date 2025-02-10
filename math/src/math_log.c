#include "private_math.h"

static f64_t math_log_imp_cody( f64_t x, u16_t type );

extern double math_log( double x )
{
	dnorm_t result = { 0 };

	result.type 		= math_type( x );
	result.f.w[ W0 ] 	= result.type;

	switch( result.type )
	{
		case(GRADZ):
		case (FINITE):
		{
			if( x < 0.0 )
			{
				result.f.w[ W0 ] = NIL;
			}
			else
			{
				result.f.d = math_log_imp_cody( x, result.type );
			}

			break;
		}

		case (INF):
		{
			if( x < 0 )
			{
				result.f.w[ W0 ] = NIL;
			}

			else
			{
				result.f.w[ W0 ] = INF;
			}

			break;
		}

		case (ZERO):
		{
			result.f.w[ W0 ] = NINF;
			break;
		}
	}

	return ( result.f.d );
}

static f64_t math_log_imp_cody( f64_t x, u16_t type )
{
	static f128_t a0 		= -0.64124943423745581147E+2L;
	static f128_t a1 		= +0.16383943563021534222E+2L;
	static f128_t a2 		= -0.78956112887491257267E+0L;

	static f128_t b0 		= -0.76949932108494879777E+3L;
	static f128_t b1 		= +0.31203222091924532844E+3L;
	static f128_t b2 		= -0.35667977739034646171E+2L;

	static f64_t ln2c1 		= 22713.0 / 32768.0;
	static f64_t ln2c2		= 1.428606820309417232e-6;
	static f64_t sqrt05 	= 0.707106781186547524400;

	dnorm_t xn = math_cwnormalize( x, type );

	f128_t z = (f128_t) xn.f.d - 0.5L;

	if ( sqrt05 < xn.f.d )
	{
		z = ( z - 0.5L ) / ( (f128_t) xn.f.d * 0.5 + 0.5);
	}

	else
	{
		xn.e = xn.e - 1;
		z = z / (z * 0.5L + 0.5L);
	}

	const f128_t w 		= (f128_t) (z * z);
	const f128_t aw 	= (a2 * w + a1) * w + a0;
	const f128_t bw 	= ((w + b2) * w + b1) * w + b0;

	z 					= z + ( z * w * ( aw / bw ) );

	if ( xn.e != 0 )
	{
		const f64_t de = (f64_t) xn.e;
		z = (de * ln2c2 + z) + de * ln2c1;
	}

	return ( z );
}
