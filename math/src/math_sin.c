#include "private_math.h"

typedef struct
{
	f64_t g;
	f64_t k;
} rem_t;
typedef enum
{
	SIN_QUADRANT_OFFSET = 0,
	COS_QUADRANT_OFFSET = 1,
} qoff_t;

static f64_t math_sin_impl		( f64_t x, qoff_t qoff );
static rem_t math_rem_mid_pio2	( f64_t x, u32_t ix );
static rem_t math_rem_low_pio2	( f64_t x );

f64_t math_sin(f64_t x)
{
	dnorm_t result = { 0 };

	result.type 		= math_type( x );
	result.f.w[ W0 ] 	= result.type;

	switch(result.type)
	{
		case(NIL):
		case(INF):
		{
			result.f.w[ W0 ] = NIL;
			break;
		}

		case(ZERO):
		case(GRADZ):
		{
			result.f.d = x;
			break;
		}

		case(FINITE):
		{
			if( ( -MATH_SIN_RANGE < x ) && ( x < +MATH_SIN_RANGE ) )
			{

                if ( ( -DBL_SQE < x ) && ( x < +DBL_SQE ) )
				{
					result.f.d = x;
				}

				else
				{
					dw_t xabs = { .d = x };

					const u16_t xsign = xabs.w[ W0 ] & SMASK;

					xabs.w[ W0 ] = xabs.w[ W0 ] & EDMASK;

					result.f.d = math_sin_impl( xabs.d, SIN_QUADRANT_OFFSET);

					result.f.w[ W0 ] = xsign ^ result.f.w[ W0 ];
				}
			}

			else
			{
				result.f.w[ W0 ] = NIL;
			}

			break;
		}
	}

	return ( result.f.d );
}

f64_t math_cos(f64_t x)
{
	dnorm_t result = { 0 };

	result.type 		= math_type( x );
	result.f.w[ W0 ] 	= result.type;

	switch(result.type)
	{
		case( NIL ):
		case( INF ):
		{
			result.f.w[ W0 ] = NIL;
			break;
		}

		case( ZERO ):
		case( GRADZ ):
		{
			result.f.d = 1.0;
			break;
		}

		case( FINITE ):
		{
			if( ( -MATH_SIN_RANGE < x ) && ( x < +MATH_SIN_RANGE ) )
			{
				if( ( -DBL_SQE < x ) && ( x < +DBL_SQE ) )
				{
					result.f.d = 1.0;
				}

				else
				{
					dw_t xabs = { .d = x };

					xabs.w[ W0 ] = xabs.w[ W0 ] & EDMASK;

					result.f.d = math_sin_impl( xabs.d, COS_QUADRANT_OFFSET );
				}
			}

			else
			{
				result.f.w[ W0 ] = NIL;
			}

			break;
		}
	}

	return ( result.f.d );
}

static f64_t math_sin_impl( f64_t x, qoff_t qoff )
{
	f64_t result;

	dw_t u = { .d = x };

	const u32_t ix = ( u.u >> 32 ) & 0x7fffffffu;

	rem_t rem;

	switch ( ix )
	{
		case (0x4002d97c):
		case (0x400f6a7a):
		case (0x4012d97c):
		case (0x4015fdbb):
		case (0x401921fb):
		case (0x401c463a):
		case (0x4035fdbb):
		case (0x408324e2):
		case (0x4073a28c):
		case (0x4087c21f):
		{
			rem = math_rem_mid_pio2( x, ix );
			break;
		}

		default:
		{
			if ( ( ix & 0xfffff ) == 0x921fb )
			{
				rem = math_rem_mid_pio2( x, ix );
			}

			else
			{
				rem = math_rem_low_pio2( x );
			}

			break;
		}
	}

	static const u32_t mask_cos = 0x1u;
	static const u32_t mask_sin = 0x2u;
	static const u64_t mask_all = 0x3u;

	const u32_t q = qoff + ( (u64_t) rem.k & mask_all );

	if ( q & mask_cos )
	{
		if( ( -DBL_EPS < rem.g ) && ( rem.g < DBL_EPS ) )
		{
			result = 1.0;
		}

		else
		{
			static const f128_t c[8] =
			{
				-1.1470879000000000e-11L,
				+2.0877120710000000e-09L,
				-2.7557319220200000e-07L,
				+2.4801587292937000e-05L,
				-1.3888888888888930e-03L,
				+4.1666666666667325e-02L,
				-5.0000000000000000e-01L,
				+1.0L,
			};

			result = math_horner( rem.g * rem.g, c, 8 );
		}
	}

	else
	{
		if( ( -DBL_EPS < rem.g ) && ( rem.g < DBL_EPS ) )
		{
			result = rem.g;
		}

		else
		{
			static const f128_t s[8] =
			{
				-7.6471637318198168e-13L,
				+1.6059043836821615e-10L,
				-2.5052108385441719e-08L,
				+2.7557319223985891e-06L,
				-1.9841269841269841e-04L,
				+8.3333333333333333e-03L,
				-1.6666666666666666e-01L,
				+1.0L,
			};

			result = rem.g * math_horner( rem.g * rem.g, s, 8 );
		}
	}

	if( q & mask_sin )
	{
		result = -result;
	}

	else
	{
		/* NOP */
	}

	return ( result );
}


static rem_t math_rem_mid_pio2( f64_t x, u32_t ix )
{
	rem_t result;

	static const f64_t toint   = 1.5 / DBL_EPS;

	static const f64_t invpio2 = 6.36619772367581382433e-01;
	static const f64_t pio2_1  = 1.57079632673412561417e+00;
	static const f64_t pio2_1t = 6.07710050650619224932e-11;
	static const f64_t pio2_2  = 6.07710050630396597660e-11;
	static const f64_t pio2_2t = 2.02226624879595063154e-21;

	static const u32_t moff32 = 20;

	dw_t u		= { .d = x };

	result.k	= ( (f64_t) ( x * invpio2 ) ) + toint - toint;

	f64_t r		= x - ( result.k * pio2_1 );
	f64_t w		= result.k * pio2_1t;

	result.g	= r - w;

	u.d			= result.g;

	i32_t ey	= ( u.u >> ( DOFF + MOFF ) ) & DMAX;
	i32_t ex	= ix >> moff32;

	if ( ( ex - ey ) > 16 )
	{
		f64_t t = r;

		w	= result.k * pio2_2;
		r	= t - w;
		w	= ( result.k * pio2_2t ) - ( ( t - r ) - w );

		result.g	= r - w;
	}

	return ( result );
}

static rem_t math_rem_low_pio2( f64_t x )
{
	rem_t result;

	static const f128_t twobypi	= 0.63661977236758134308L;
	static const f128_t c1		= 1.57079601287841796875L;
	static const f128_t c2		= 3.139164786504813216916397514421e-7L;

	result.k	= math_intrnd( x * twobypi ) ;
	result.g	= ( x - result.k * c1) - result.k * c2;

	return ( result );
}
