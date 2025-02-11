#include <stdio.h>
#include "private_math.h"

static f64_t math_significand( f64_t x );
static u16_t math_biased_exponent( u16_t w0 );
static f64_t math_intrnd_impl( f64_t x );
static f64_t math_to_integer_impl( f64_t xabs );

/*
 * math_abs
 */

f64_t math_abs( f64_t x )
{
	dw_t result		= { .d = x };

	result.w[ W0 ]	= result.w[ W0 ] & EDMASK;

	return ( result.d );
}

/*
 *	math_type
 *
 *	TAGS: #arch-x86, #safe, #math-core
 *
 *  ARGUMENTS:
 *
 *	-	x: FP representation encoded in arch-x86
 *
 *	INPUT RANGE:
 *
 * 	-	all f64_t
 *
 *	DEFINITION:
 *
 *	-	1. computes the type of the FP representation of 'x' according to the definition of IEEE 754-2008.
 *
 *			INF		:	x in ( -inf       , -1.798E+308 ) U ( +1.798E+308, +inf        )
 *			FINITE	: 	x in [ -1.798E+308, -2.225E-308 ] U [ +2.225E-308, +1.798E+308 ]
 *			GRADZ	:	x in ( -2.225E-308, -4.941E-324 ] U [ +4.941E-324, +2.225E-308 )
 *			ZERO	: 	x in -0.0, +0.0
 *			NIL		:	used to note resulted mathematical indeterminations as e.g.: 0/0, inf - inf, 0 · inf
 *
 * 	-	2. this is a function that interfaces with architecture x86 of target, compliance with IEEE 754-2008 is assumed
 *
 *	RETURN:
 *
 *	- 	an element of the the discrete set TYPES := {INF, FINITE, GRADZ, ZERO, NIL}
 *		this set only has defined in it the comparisons between members of the set.
 *
 *	EXAMPLE:
 *
 * 	the only one defined operation is equallity and its negation, e.g.: math_type(x) == TYPE, and math_type(x) != TYPE
 *
 */

extern u16_t math_type ( f64_t x )
{
	u16_t result;

	const dw_t 		px 					= { .d = x };
	const i16_t 	exponent			= math_biased_exponent( px.w[ W0 ] );
	const bool_t 	mantisa_not_zero	= ( px.w[ W0 ] & DFRAC ) || px.w[ W1 ] || px.w[ W2 ] || px.w[ W3 ];

	if ( exponent == DMAX )
	{
		if ( mantisa_not_zero )
		{
			result = NIL;			/* exponent == DMAX	, mantissa != 0 */
		}
		else
		{
			result = INF;			/* exponent == DMAX	, mantissa == 0 */
		}
	}

	else
	{
		if ( exponent != 0 )
		{
			result = FINITE;		/* exponent != 0	, mantissa N/A,  */
		}

		else if ( mantisa_not_zero )
		{
			result = GRADZ;			/* exponent == 0	, mantissa != 0 */
		}

		else
		{
			result = ZERO;			/* exponent == 0	, mantissa == 0 */
		}
	}

	return (result);
}

/*
 *	math_cwnormalize:
 *
 *	TAGS: #arch-x86, #no-safe, #safe-f32, #math-core
 *
 *  ARGUMENTS:
 *
 *	-	x: FP representation encoded in arch-x86
 *
 *	INPUT RANGE:
 *
 * -	before v1.0 release:
 * 		in order to avoid gradual underflow of f64_t values, this function shall be only applied to |x| < FLT_MAX (#safe-f32)
 *
 *	DEFINITION:
 *
 *  -	0. separate x into to 1/2 <= |frac| < 1 and 2^e
 *
 *	-	1. computes the normalization of the FP representation of 'x' according to the definition of [Cody and Waite, 1980].
 *
 *			given: 'x' expressed as the tripled x(s,m,p), it is converted into CW-format as x=f·B**e, where 0.5 <= |f| < 1
 *
 *			result is the normalized radix-2 'f' value of the FP representation of 'x' expressed in CW-format
 *
 *	- 	2. this is a high-level function that detects the input type before execution
 * 	-	3. this is a function that interfaces with architecture x86 of target, compliance with IEEE 754-2008 is assumed
 *
 *	RETURN:
 *
 *	- 	the normalized value 'f' of x = f·B**e, with 0.5 <= |f| < 1, mathematically equivalent to
 *
 *								f = x·2**{-floor(1+log(x)/log(B)}
 *
 *	EXAMPLE:
 *
 * 	math_cwnormalize(x=1.5) returns f=0.75, because 1.5 can be expressed as (0.75)·2^1 in CW-format.
 *
 */

extern dnorm_t math_cwnormalize ( f64_t x, u16_t type )
{
	dnorm_t result = { 0 };

	result.type = type;

	const dw_t px = { .d = x };

	result.f.w[ W0 ] = ( px.w[ W0 ] & SMMASK ) | ONEHALF;
	result.f.w[ W1 ] =   px.w[ W1 ];
	result.f.w[ W2 ] =   px.w[ W2 ];
	result.f.w[ W3 ] =   px.w[ W3 ];

	switch( result.type )
	{
		case( FINITE ):
		{
			result.e  = ((i16_t) math_biased_exponent( px.w[ W0 ] )) - DBIAS;
			break;
		}

		case( GRADZ ):
		{
			result.e = 1 + EMIN;

			while( ( result.f.w[W0] & 16u ) == 0 )
			{
				result.f.u	= result.f.u << 1;
				result.e	= result.e - 1;
			}

			result.f.w[ W0 ] = ONEHALF | ( result.f.w[W0] & MMASK );

			break;
		}

		case( NIL ):
		case( INF ):
		case( ZERO ):
		default:
		{
			result.f.w[ W0 ] = result.type;
			break;
		}

		/* NOP: nothing to normalize in case of x is: ZERO, INFINITY, or NAN */
	}

	return (result);
}

/*
 * math_cwsetexp
 *
 * TAGS: #arch-x86, #no-safe, #math-core
 *
 * ARGUMENTS:
 *
 * 	- 	n: new exponent to set in the result value in the range [emin, emax], this range is not checked (#no-safe)
 * 	- 	x: FP representation encoded in arch-x86 whose significand is used to construct the mantissa of the result value.
 *
 * DEFINITION:
 *
 * 		1. Modifies the FP representation in CW-format by setting a new exponent, while retaining the significand of 'x':
 *
 *    		- Given: 	'x' = f·B**e
 *             			'n' in [emin, emax]
 *
 *    		- The result is an FP number in CW-format, expressed as f·B^n, where f is the significand of 'x' and 'n' is the new exponent.
 *
 * 		2. This is a core function and does not perform any safety checks (e.g., for overflow, underflow, infinity, or NaN).
 *
 * 		3. It is typically used after calling `math_cwnormalize` to express 'x' in the form x = f·B**e.
 *
 * 		4. This function interfaces with the x86 architecture of the target system, assuming compliance with IEEE 754-2008.
 *
 * RETURN:
 *
 * 	-	A floating-point representation of a number whose significand is taken from the input 'x', and whose exponent is set to the value of input 'n'.
 *
 * EXAMPLE:
 *
 * -	math_cwsetexp(x=1.0, n=3) returns 4.0, because x = (0.5)·2**1 is the normalized fp representation (f·B**n, with B=2), and the result is 4.0 = f·2**n = 0.5·2**3
 *
 * 		another useful way to see math_cwsetexp function is as math_cwsetexp(1., n+1) = 2**n
 */

extern f64_t math_cwsetexp ( f64_t x, i16_t n)
{
    dw_t result;

	i16_t abs_n;

	if( n < 0 )
	{
		abs_n = -n;
	}

	else
	{
		abs_n = +n;
	}

	if ( DMAX <= abs_n )
	{
		if( x < 0.0 )
		{
			result.u = 0;
			result.w[ W0 ] = INF | SMASK;
		}

		else
		{
			result.u = 0;
			result.w[ W0 ] = INF;
		}
	}

	else
	{
		result.d = math_significand( x );

		if( ( n + EMAX ) <= 0 )
		{
			result.u	= result.u | ( 0x8000000000000LLu >> ( (u32_t) ( -( n + EMAX ) ) ) );
			n 			= -EMAX;
		}

		result.w[ W0 ] = result.w[ W0 ] | ( ( n + EMAX ) << DOFF );
	}

    return ( result.d );
}

/*
 * math_horner
 *
 * for performance purposes, the function does not perform any safety check,
 * the preconditions to call this function are:
 *
 * - poly is not null-pointer
 * - n is strictly greater than 0
 */

extern f128_t math_horner( f128_t x, const f128_t * poly, i32_t n )
{
    f128_t result;

	result = poly[0];

	for ( i32_t i = 1; i < n; i = 1 + i )
	{
		result = result * x + poly[ i ];
	}

	return ( result );
}

/*
 * Returns the integer representation of the integer closest to the fp number x
 *
 *         s        e - q + 1
 * x = (-1)  · m · B
 *
 */

extern f64_t math_intrnd(f64_t x)
{
	dw_t result = { .d = x };

	u16_t dtype = math_type( result.d );

	switch( dtype )
	{
		case (FINITE):
		{
			result.d = math_intrnd_impl( x );
			break;
		}

		case (GRADZ):
		{
			result.d = 0.0;
		}

		case (INF):
		case (ZERO):
		case (NIL):
		default:
		{

			break;
		}
	}

	return ( result.d );
}

extern f64_t math_to_integer2( f64_t x )
{
	volatile dw_t result;

	static const f64_t beta_t_minus_one = 0x1p53;

	dw_t u 		= { .d = x };
	dw_t s 		= { .d = x };

	s.w[ W0 ] 	= s.w[ W0 ] & SMASK;
	u.w[ W0 ] 	= u.w[ W0 ] & EDMASK;

    result.d 	= u.d		+ beta_t_minus_one;
    result.d 	= result.d	- beta_t_minus_one;

    if( result.d > u.d )
    {
        result.d = result.d - 1.0;
    }

	else
	{
		/* NOP: rounding mode did not affect */
	}

	result.w[ W0 ] = s.w[ W0 ] | result.w[ W0 ];

	return ( result.d );
}

extern f64_t math_to_integer( f64_t x )
{
	volatile dw_t result;

	switch( math_type( x ) )
	{
		case ( NIL ):
		case ( INF ):
		case ( ZERO ):
		{
			result.d = x;
			break;
		}

		case ( GRADZ ):
		case ( FINITE ):
		{
			const dw_t xsign = { .d = x };

			if( xsign.w[ W0 ] < EXPONE )
			{
				result.u = 0;
			}

			else
			{
				result.d = math_to_integer_impl( math_abs( x ) );
			}

			result.w[ W0 ] = result.w[ W0 ] | ( xsign.w[ W0 ] & SMASK );

			break;
		}

		default: { /* NOP: not accessible */ break; }
	}

	return ( result.d );
}

static f64_t math_to_integer_impl( f64_t xabs )
{
	volatile dw_t result;

	static const f64_t beta_to_t_minus_one = 0x1p52;

	if( xabs >= beta_to_t_minus_one )
	{
		result.d = xabs;
	}

	else
	{
		result.d = xabs  	+ beta_to_t_minus_one;
		result.d = result.d - beta_to_t_minus_one;

		if( result.d > xabs )
		{
			result.d = result.d - 1.0;
		}

		else { /* NOP: rounding mode did not affect */ }
	}

	return ( result.d );
}

/*
 * math_intrnd_impl
 *
 * designed to work only with FINITE values,
 * performs correct rounding for the range x in [ -4.5e15, +4.5e15 ]
 * outside this range, but still a double finite returns 0
 */

static f64_t math_intrnd_impl( f64_t x )
{
	dw_t result;

	if (x < 0.0)
	{
        result.d = x - 0.5;
    }

	else
	{
		result.d = x + 0.5;
	}

	const i16_t exp			= (DBIAS + 48 + DOFF + 1) - math_biased_exponent( result.w[ W0 ] );

	const bool_t integer	= ( exp <= 0 );

	if ( integer )
	{
		/* NOP: return result.d by default */
    }

	else if ( -1.0 < result.d && result.d < +1.0 )
	{
		result.d = 0.0;
	}

	else
	{
		static const unsigned short mask[ 16 ] =
		{
			0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
			0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff
		};

		static const u16_t sub[ 4 ] = { W3, W2, W1, W0 };

		const u16_t wi 	= sub[ exp >> 4 ];

		u16_t frac 		= mask[ exp & 0xF ] & result.w[ wi ];

		result.w[ wi ] 	= frac ^ result.w[ wi ];

		for( u16_t wz = W3; wz < ( exp >> 4 ) && ( wz < W0 ); wz = 1 + wz )
		{
			frac = frac | result.w[ wz ];
			result.w[wz] = 0;
		}

		if( frac == 0 )
		{
			result.d = 0.0;
		}
	}

	return ( result.d );
}

/*
 * Extracts the biased exponent from W0 (see private_math.h) of the fp representation
 *
 *         s        e - q + 1
 * x = (-1)  · m · B
 *
 */

static u16_t math_biased_exponent( u16_t w0 )
{
	return ( ( w0 & DMASK ) >> DOFF );
}

/*
 * Extracts the mantisa m conserving the sign bit s of the fp representation
 *
 *         s        e - q + 1
 * x = (-1)  · m · B
 *
 */

static f64_t math_significand( f64_t x )
{
	dw_t result = { 0 };

    dw_t f = { .d = x };

	result.w[ W3 ] = f.w[ W3 ];
    result.w[ W2 ] = f.w[ W2 ];
    result.w[ W1 ] = f.w[ W1 ];
    result.w[ W0 ] = ( f.w[ W0 ] & SMMASK );

	return ( result.d );
}
