#include "private_math.h"

static f64_t math_sqrt_impl( f64_t x, u16_t type );

extern f64_t math_sqrt( f64_t x )
{
	dnorm_t result = { 0 };

	if( x < 0.0 )
	{
		result.f.w[ W0 ] = NIL;
	}

	else
	{
		result.type 		= math_type( x );
		result.f.w[ W0 ] 	= result.type;

		switch( result.type )
		{
			case (GRADZ):
			case (FINITE):
			{
				result.f.d = math_sqrt_impl( x, result.type );
				break;
			}

			case (INF):
			{
				/* NOP: result is already assigned to plus INF */
				break;
			}

			case (ZERO):
			{
				result.f.d = x; /* x preserves sign of zero, rather than literal 0.0 which loses it */
				break;
			}
		}
	}

	return ( result.f.d );
}

static f64_t math_sqrt_impl( f64_t x, u16_t type )
{
    static const f64_t sqrt2 = 1.4142135623730950488016;

    dnorm_t xn = math_cwnormalize( x, type );

	f64_t y = 0.41731 + ( 0.59016 * xn.f.d );

	/* 1st Iter */
	y = 0.5 * ( y + xn.f.d / y );

	/* 2nd Iter */
	y = y + ( xn.f.d / y );
	y = ( 0.25 * y ) + ( xn.f.d / y );

    if( xn.e % 2 != 0 )
    {
        y = y * sqrt2;
        xn.e = ( xn.e + 1 ) / 2;
    }

    else
    {
        xn.e = ( xn.e / 2 ) + 1;
    }

    return ( y * math_cwsetexp(1., xn.e) );
}
