math_sqrt

    - d_type

        - d_biased_exponent

    - d_cwnormalize

        - d_type
        - d_biased_exponent

    - d_sqrt_imp

    - d_cwsetexp

        - d_significand

```c
extern u16_t    d_type              ( f64_t  x  );
extern dnorm_t  d_cwnormalize       ( f64_t  x  );
static f128_t   d_sqrt_imp          ( f128_t x  );
static u16_t    d_biased_exponent   ( u16_t  w0 );
static f64_t    d_significand       ( f64_t  x  );
```

```c
static f128_t math_sqrt_normals( f64_t x )
{
    static const f128_t sqrt2 = 1.4142135623730950488016887242097L;

    dnorm_t xn = d_cwnormalize( x );

    f128_t y = d_sqrt_imp( xn.f.d );

    if( xn.e % 2 != 0 )
    {
        y = y * sqrt2;
        xn.e = xn.e + 1;
        xn.e = xn.e >> 1;
    }
    else
    {
        xn.e = xn.e >> 1;
        xn.e = xn.e + 1;
    }

    return ( y * ( (f128_t) d_cwsetexp(1., xn.e) ) );
}
```

```c
extern double math_sqrt( double x )
{
	dnorm_t result = { 0 };

	result.type 		= d_type( x );
	result.f.w[ W0 ] 	= result.type;

	if( x < 0.0 )
	{
		result.f.w[ W0 ] = NIL;
	}

	else
	{
		switch( result.type )
		{
			case (GRADZ):
			case (FINITE):
			{
				dnorm_t xn = d_cwnormalize( x );

				long double y = d_sqrt_imp( xn.f.d );

				if( xn.e % 2 != 0 )
				{
					y = y * sqrt2;
					xn.e = xn.e + 1;
					xn.e = xn.e >> 1;
				}
				else
				{
					xn.e = xn.e >> 1;
					xn.e = xn.e + 1;
				}

				result.f.d = (double) ( y * (long double) d_cwsetexp(1., xn.e) );

				break;
			}

			case (INF):
			{
				if( x < 0.0 ) result.f.w[ W0 ] = NIL;
				break;
			}

			case (ZERO):
			{
				result.f.d =  x; /* preserve signed zero, rather than result.f.d = 0.0, which loses it */
				break;
			}
		}
	}

	return ( result.f.d );
}
```

```c
extern u16_t d_type ( f64_t x )
{
	u16_t result;

	const dw_t 		px 					= { .d = x };
	const i16_t 	exponent			= d_biased_exponent( px.w[ W0 ] );
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
```

```c
extern dnorm_t d_cwnormalize ( f64_t x )
{
	dnorm_t result = { 0 };

	result.type 		= d_type( x );
	result.f.w[ W0 ] 	= result.type;

	if( result.type == FINITE )
	{
		const dw_t px = { .d = x };

		result.e  = ((i16_t) d_biased_exponent( px.w[ W0 ] )) - DBIAS;

		result.f.w[ W0 ] = (px.w[ W0 ] & SMMASK) | ONEHALF ;
		result.f.w[ W1 ] = px.w[ W1 ];
		result.f.w[ W2 ] = px.w[ W2 ];
		result.f.w[ W3 ] = px.w[ W3 ];
	}

	else if ( result.type == GRADZ )
	{
		fprintf(stderr, "[WARN] `d_cwnormalize` f64_t gradual underflow not supported yet - ");

		const dw_t px = { .d = x };

		result.f.w[ W0 ] = (px.w[ W0 ] & MMASK);
		result.f.w[ W1 ] = px.w[ W1 ];
		result.f.w[ W2 ] = px.w[ W2 ];
		result.f.w[ W3 ] = px.w[ W3 ];

		u64_t mantissa = result.f.u;

		result.e = -DBIAS;

		i32_t found = 0;

		for( i32_t i = DOFF-1; i >= 0 && !found; i = -1 + i )
		{
			found 		= (mantissa & (1ULL << i)) != 0;

			result.e 	= result.e - 1;
			mantissa	= mantissa << 1;
		}

		result.e	= result.e - 1;
		result.f.u 	= ( mantissa & 0xFFFFFFFFFFFFF ) | ((u64_t)(px.w[ W0 ] & SMASK) | ONEHALF) << MOFF;
	}

	else
	{
		/* NOP: nothing to normalize in case of x is: ZERO, INFINITY, or NAN */
	}

	return (result);
}
```

```c
static long double d_sqrt_imp( long double x )
{
	long double y;

	y = (-0.1984742L * x + 0.8804894L) * x + 0.3176687L;
	y = 0.5L * (y + x / y);
	y = y + x / y;
	x = 0.25L * y + x / y;

	return (x);
}
```

```c
extern f64_t d_cwsetexp ( f64_t x, i16_t n) /* TODO: test this function for all TYPES set */
{
    dw_t result = { .d = d_significand( x ) };

	//printf("d_cwsetexp: n = %d, DMAX=%d? %d\n", n, DMAX, DMAX <= (n>0?n:-n));

	/* TODO: check this as d_scale */
	if ( DMAX <= (n>0?n:-n))
	{	/* overflow, return +/-INF */
		if(x>0.0)
		{
			return 1.0/0.0;
		}
		else
		{
			return -1.0/0.0;
		}
	}

    result.w[ W0 ] = result.w[ W0 ] | ( ( n + DBIAS ) << DOFF );

    return result.d;
}
```

```c
static u16_t d_biased_exponent( u16_t w0 )
{
	return ( ( w0 & DMASK ) >> DOFF );
}
```

```c
static f64_t d_significand( f64_t x )
{
	dw_t result = { 0 };

    dw_t f = { .d = x };

	result.w[ W3 ] = f.w[ W3 ];
    result.w[ W2 ] = f.w[ W2 ];
    result.w[ W1 ] = f.w[ W1 ];
    result.w[ W0 ] = ( f.w[ W0 ] & SMMASK );

	return ( result.d );
}
```

```c
#include "private_math.h"

static f64_t d_significand( f64_t x );
static u16_t d_biased_exponent( u16_t w0 );
static f64_t d_intrnd_impl( f64_t x );
static i16_t d_norm( u16_t *ps );

/*
 *	d_type
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
 * 	the only one defined operation is equallity and its negation, e.g.: d_type(x) == TYPE, and d_type(x) != TYPE
 *
 */

extern u16_t d_type ( f64_t x )
{
	u16_t result;

	const dw_t 		px 					= { .d = x };
	const i16_t 	exponent			= d_biased_exponent( px.w[ W0 ] );
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
 *	d_cwnormalize:
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
 * 	d_cwnormalize(x=1.5) returns f=0.75, because 1.5 can be expressed as (0.75)·2^1 in CW-format.
 *
 */

extern dnorm_t d_cwnormalize ( f64_t x )
{
	dnorm_t result = { 0 };

	result.type 		= d_type( x );
	result.f.w[ W0 ] 	= result.type;

	if( result.type == FINITE )
	{
		const dw_t px = { .d = x };

		result.e  = ((i16_t) d_biased_exponent( px.w[ W0 ] )) - DBIAS;

		result.f.w[ W0 ] = (px.w[ W0 ] & SMMASK) | ONEHALF ;
		result.f.w[ W1 ] = px.w[ W1 ];
		result.f.w[ W2 ] = px.w[ W2 ];
		result.f.w[ W3 ] = px.w[ W3 ];
	}

	else if ( result.type == GRADZ )
	{
		fprintf(stderr, "[WARN] `d_cwnormalize` f64_t gradual underflow not supported yet - ");

		const dw_t px = { .d = x };

		result.f.w[ W0 ] = (px.w[ W0 ] & MMASK);
		result.f.w[ W1 ] = px.w[ W1 ];
		result.f.w[ W2 ] = px.w[ W2 ];
		result.f.w[ W3 ] = px.w[ W3 ];

		u64_t mantissa = result.f.u;

		result.e = -DBIAS;

		i32_t found = 0;

		for( i32_t i = DOFF-1; i >= 0 && !found; i = -1 + i )
		{
			found 		= (mantissa & (1ULL << i)) != 0;

			result.e 	= result.e - 1;
			mantissa	= mantissa << 1;
		}

		result.e	= result.e - 1;
		result.f.u 	= ( mantissa & 0xFFFFFFFFFFFFF ) | ((u64_t)(px.w[ W0 ] & SMASK) | ONEHALF) << MOFF;
	}

	else
	{
		/* NOP: nothing to normalize in case of x is: ZERO, INFINITY, or NAN */
	}

	return (result);
}

/*
 * d_cwsetexp
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
 * 		3. It is typically used after calling `d_cwnormalize` to express 'x' in the form x = f·B**e.
 *
 * 		4. This function interfaces with the x86 architecture of the target system, assuming compliance with IEEE 754-2008.
 *
 * RETURN:
 *
 * 	-	A floating-point representation of a number whose significand is taken from the input 'x', and whose exponent is set to the value of input 'n'.
 *
 * EXAMPLE:
 *
 * -	d_cwsetexp(x=1.0, n=3) returns 4.0, because x = (0.5)·2**1 is the normalized fp representation (f·B**n, with B=2), and the result is 4.0 = f·2**n = 0.5·2**3
 *
 * 		another useful way to see d_cwsetexp function is as d_cwsetexp(1., n+1) = 2**n
 */

extern f64_t d_cwsetexp ( f64_t x, i16_t n) /* TODO: test this function for all TYPES set */
{
    dw_t result = { .d = d_significand( x ) };

	//printf("d_cwsetexp: n = %d, DMAX=%d? %d\n", n, DMAX, DMAX <= (n>0?n:-n));

	/* TODO: check this as d_scale */
	if ( DMAX <= (n>0?n:-n))
	{	/* overflow, return +/-INF */
		if(x>0.0)
		{
			return 1.0/0.0;
		}
		else
		{
			return -1.0/0.0;
		}
	}

    result.w[ W0 ] = result.w[ W0 ] | ( ( n + DBIAS ) << DOFF );

    return result.d;
}

extern f64_t d_horner(f64_t x, const f64_t * poly, i32_t n)
{
    f64_t y = poly[0];

    for ( i32_t i = 0; i < n-1; i = 1 + i )
    {
        y = y * x + poly[i+1];
    }

	return (y);
}

/*
 * Returns the integer representation of the integer closest to the fp number x
 *
 *         s        e - q + 1
 * x = (-1)  · m · B
 *
 */

extern f64_t d_intrnd(f64_t x)
{
	dw_t result = { .d = x };

	u16_t dtype = d_type( result.d );

	switch( dtype )
	{
		case (FINITE):
		{
			result.d = d_intrnd_impl( x );
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

/*
 * d_intrnd_impl
 *
 * designed to work only with FINITE values,
 * performs correct rounding for the range x in [ -4.5e15, +4.5e15 ]
 * outside this range, but still a double finite returns 0
 */

static f64_t d_intrnd_impl( f64_t x )
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

	const i16_t exp			= (DBIAS + 48 + DOFF + 1) - d_biased_exponent( result.w[ W0 ] );

	const bool_t integer	= ( exp <= 0 );

	if ( integer )
	{
		/* NOP: return result.d by default */
    }

	else if ( -1.0 < result.d && result.d < +1.0 ) /* |x| < 0.5 ret 0 */
	{
		result.d = 0.0;
	}

	else /* 0.5 <= |x| < integer */
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

		/* clear the first W3, W2, W1 upto the value of (exponent >> 4) */
		for( u16_t wz = W3; wz < (exp >> 4) && wz < W0; wz++ )
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
 * Extracts the biased exponent from W0 (see dmath.h) of the fp representation
 *
 *         s        e - q + 1
 * x = (-1)  · m · B
 *
 */

static u16_t d_biased_exponent( u16_t w0 )
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

static f64_t d_significand( f64_t x )
{
	dw_t result = { 0 };

    dw_t f = { .d = x };

	result.w[ W3 ] = f.w[ W3 ];
    result.w[ W2 ] = f.w[ W2 ];
    result.w[ W1 ] = f.w[ W1 ];
    result.w[ W0 ] = ( f.w[ W0 ] & SMMASK );

	return ( result.d );
}

/*
 * TODO: add description
 */

extern i16_t d_scale( f64_t *px, i16_t xexp )
{	/* scale *px by 2^xexp with checking */
	long lexp;
	unsigned short *ps = (unsigned short *)px;
	short xchar = (ps[W0] & DMASK) >> DOFF;

	if (xchar == DMAX)	/* NaN or INF */
	{
		return (ps[W0] & DFRAC || ps[W1] || ps[W2] || ps[W3] ? NAN : INF);
	}

	else if (0 < xchar)
	{
		;	/* NOP: finite */
	}

	else if (0 < (xchar = d_norm(ps)))
	{
		return (0);	/* zero */
	}

	lexp = (long)xexp + xchar;

	if (DMAX <= lexp)
	{	/* overflow, return +/-INF */
		*px = ps[W0] & SMASK ? -1./0. : 1./0.;
		return (INF);
	}

	else if (0 < lexp)
	{	/* finite result, repack */
		ps[W0] = ps[W0] & ~DMASK | (short)lexp << DOFF;
		return (FINITE);
	}

	else
	{	/* denormalized, scale */
		unsigned short sign = ps[W0] & SMASK;

		ps[W0] = 1 << DOFF | ps[W0] & DFRAC;

		if (--lexp < -(48+DOFF))
		{	/* underflow, return +/-0 */
			ps[W0] = sign, ps[W1] = 0;
			ps[W2] = 0, ps[W3] = 0;
			return (0);
		}

		else
		{	/* nonzero, align fraction */
			for (xexp = lexp; xexp <= -16; xexp += 16)
			{	/* scale by words */
				ps[W3] = ps[W2], ps[W2] = ps[W1];
				ps[W1] = ps[W0], ps[W0] = 0;
			}

			if ((xexp = -xexp) != 0)
			{	/* scale by bits */
				ps[W3] = ps[W3] >> xexp
					| ps[W2] << 16 - xexp;
				ps[W2] = ps[W2] >> xexp
					| ps[W1] << 16 - xexp;
				ps[W1] = ps[W1] >> xexp
					| ps[W0] << 16 - xexp;
				ps[W0] >>= xexp;
			}

			ps[W0] |= sign;
			return (FINITE);
		}
	}
}

/*
 * TODO: add description
 */

static i16_t d_norm( u16_t *ps )
{	/* normalize double fraction */
	short xchar;
	unsigned short sign = ps[W0] & SMASK;

	xchar = 1;
	if ((ps[W0] &= DFRAC) != 0 || ps[W1] || ps[W2] || ps[W3])
	{	/* nonzero, scale */

		for (; ps[W0] == 0; xchar -= 16)
		{	/* shift left by 16 */
			ps[W0] = ps[W1], ps[W1] = ps[W2];
			ps[W2] = ps[W3], ps[W3] = 0;
		}

		for (; ps[W0] < 1<<DOFF; --xchar)
		{	/* shift left by 1 */
			ps[W0] = ps[W0] << 1 | ps[W1] >> 15;
			ps[W1] = ps[W1] << 1 | ps[W2] >> 15;
			ps[W2] = ps[W2] << 1 | ps[W3] >> 15;
			ps[W3] <<= 1;
		}

		for (; 1<<DOFF+1 <= ps[W0]; ++xchar)
		{	/* shift right by 1 */
			ps[W3] = ps[W3] >> 1 | ps[W2] << 15;
			ps[W2] = ps[W2] >> 1 | ps[W1] << 15;
			ps[W1] = ps[W1] >> 1 | ps[W0] << 15;
			ps[W0] >>= 1;
		}

		ps[W0] &= DFRAC;
	}

	ps[W0] |= sign;
	return (xchar);
}

static long double d_sqrt_imp( long double x );

static const long double sqrt2 = 1.41421356237309505L;

extern double math_sqrt( double x )
{
	dnorm_t result = { 0 };

	result.type 		= d_type( x );
	result.f.w[ W0 ] 	= result.type;

	if( x < 0.0 )
	{
		result.f.w[ W0 ] = NIL;
	}

	else
	{
		switch( result.type )
		{
			case (GRADZ):
			case (FINITE):
			{
				dnorm_t xn = d_cwnormalize( x );

				long double y = d_sqrt_imp( xn.f.d );

				if( xn.e % 2 != 0 )
				{
					y = y * sqrt2;
					xn.e = xn.e + 1;
					xn.e = xn.e >> 1;
				}
				else
				{
					xn.e = xn.e >> 1;
					xn.e = xn.e + 1;
				}

				result.f.d = (double) ( y * (long double) d_cwsetexp(1., xn.e) );

				break;
			}

			case (INF):
			{
				if( x < 0.0 ) result.f.w[ W0 ] = NIL;
				break;
			}

			case (ZERO):
			{
				result.f.d =  x; /* preserve signed zero, rather than result.f.d = 0.0, which loses it */
				break;
			}
		}
	}

	return ( result.f.d );
}

static long double d_sqrt_imp( long double x )
{
	long double y;

	y = (-0.1984742L * x + 0.8804894L) * x + 0.3176687L;
	y = 0.5L * (y + x / y);
	y = y + x / y;
	x = 0.25L * y + x / y;

	return (x);
}
```