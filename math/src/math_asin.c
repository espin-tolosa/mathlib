#include "private_math.h"

static f64_t math_asin_impl( f64_t x );

extern f64_t math_asin ( f64_t x )
{
    dnorm_t result = { 0 };

	result.type 		= math_type( x );
	result.f.w[ W0 ] 	= result.type;

	switch( result.type )
    {
        case (GRADZ):
		case (FINITE):
		{
            dw_t xu = { .d = x };

            f64_t absx = ((dw_t) { .u = xu.u & 0x7FFFFFFFFFFFFFFF }).d;

            dw_t absasin = { .d = math_asin_impl( absx ) };

            result.f.d = ((dw_t) { .u = absasin.u | xu.u & (~0x7FFFFFFFFFFFFFFF) }).d;

			break;
		}

		case (ZERO):
        {
            result.f.d =  x; /* preserve signed zero, rather than result.f.d = 0.0, which loses it */
            break;
        }

        case (INF):
        case (NIL):
		{
            result.f.w[ W0 ] = NIL;
			break;
		}
    }

    return ( result.f.d );
}

static f64_t math_asin_impl( f64_t x )
{
    dw_t result;

    static const f128_t p[5] =
    {
        -0.69674573447350646411E+0L,
        +0.10152522233806463645E+2L,
        -0.39688862997504877339E+2L,
        +0.57208227877891731407E+2L,
        -0.27368494524164255994E+2L,
    };

    static const f128_t q[6] =
    {
        +0.10000000000000000000E+1L,
        -0.23823859153670238830E+2L,
        +0.15095270841030604719E+3L,
        -0.38186303361750149284E+3L,
        +0.41714430248260412556E+3L,
        -0.16421096714498560795E+3L,
    };

    static const f128_t pio4 = 0.78539816339744830961566084581988L;
    static const f128_t pio2 = 1.57079632679489661923132169163980L;

    f128_t y = x;

    if ( y < DBL_SQE )
    {
        result.d = y;
    }

    else if ( y < 0.5L )
    {
        const f128_t g  = y * y;
        const f128_t pg = math_horner( g, p, 5 );
        const f128_t qg = math_horner( g, q, 6 );

        result.d = (f64_t) ( y + ( y * g * ( pg / qg ) ) );
    }

    else if  ( y < 1.0L )
    {
        const f128_t g  = 0.5L * ( 1.0L - y );
        const f128_t pg = math_horner( g, p, 5 );
        const f128_t qg = math_horner( g, q, 6 );

        y = (f128_t) math_sqrt( g );

        y = y + y;
        y = y + ( y * g * ( pg / qg ) );

        result.d = (f64_t) ( ( pio4 - y ) + pio4 );
    }

    else if ( y == 1.0L )
    {
        result.d = (f64_t) pio2;
    }

    else
    {
        result.u       = 0;
        result.w[ W0 ] = NIL;
    }

    return ( result.d );
}
