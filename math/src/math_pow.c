#include "public_math.h"
#include "private_math.h"

static i16_t    math_pow_safe_exp   ( f128_t y                          );
static u32_t    math_pow_iabs       ( i32_t n                           );
static f128_t   math_pow_impl       ( dnorm_t x, f128_t y               );
static f128_t   math_pow_i16        ( f128_t x, i16_t n                 );
static f128_t   math_pow_ydexp      ( i64_t zl, f128_t y,   f128_t dexp );
static i64_t    math_pow_clamp      ( i64_t x,  i64_t xmin, i64_t xmax  );

extern f64_t math_pow(f64_t x, f64_t y)
{
    dnorm_t result      = { 0 };

    result.type         = math_type( x );
    result.f.w[ W0 ]    = result.type;

    switch( result.type )
    {
        case (GRADZ):
        case (FINITE):
        {
            if( x > 0.0 )
            {
                if( x == 1.0 )
                {
                    result.f.d = 1.0;
                }

                else if( math_intrnd( y ) == y )
                {
                    result.f.d = math_pow_i16( x, y );
                }

                else
                {
                    const dnorm_t xn = math_cwnormalize( x, result.type );
                    const f128_t z = y * xn.e;

                    if( z < -1074.L )
                    {
                        result.f.w[ W0 ] = ZERO;
                    }

                    else if( z > 1025.5L )
                    {
                        result.f.w[ W0 ] = INF;
                    }
                    else
                    {
                        result.f.d = math_pow_impl( xn, y );
                    }
                }
            }

            else
            {
                if( math_intrnd( y ) == y )
                {
                    if( (i128_t)y%2==0 )
                    {
                        result.f.d = math_pow_i16( -x, y );
                    }

                    else
                    {
                        result.f.d = -math_pow_i16( -x, y );
                    }
                }

                else
                {
                    result.f.w[ W0 ] = NIL;
                }
            }

            break;
        }

        case (INF):
        {
            const f64_t iy = math_intrnd(y);

            if( y == 0.0 )
            {
                result.f.d = 1.0;
            }

            else if( (x > 0.0) && (y < 0.0) )
            {
                result.f.d = +0.0;
            }

            else if( (x < 0.0) && (y < 0.0) && (iy == y) && ( (i128_t) iy % 2 != 0 ) )
            {
                result.f.w[ W0 ] = SMASK;
            }

            else if( (x < 0.0) && (y > 0.0) && (iy == y) && ( (i128_t) iy % 2 != 0 ) )
            {
                result.f.w[ W0 ] = SMASK | result.f.w[ W0 ];
            }

            else
            {
                /* NOP: already +inf */
            }

            break;
        }

        case (NIL):
        {
            result.f.w[ W0 ] = NIL;
            break;
        }

        case (ZERO): /* x^0 = 1, for all x >= 0, even 0^0 */
        {
            const f64_t iy = math_intrnd(y);

            if( y == 0.0 )
            {
                result.f.d = +1.0;
            }

            else if( (y > 0.0) && (iy == y) && ( (i128_t) iy % 2 != 0 ) )
            {
                if( y == 1.0 )
                {
                    result.f.d = x;

                    if( result.f.w[ W0 ] & SMASK )
                    {
                        result.f.w[ W0 ] = SMASK;
                    }

                    else
                    {
                        result.f.w[ W0 ] = ZERO;
                    }

                }
                else
                {
                    result.f.d = -0.0;
                }
            }

            else if( y > 0.0 )
            {
                result.f.d = +0.0;
            }

            else if( (y < 0.0) && (iy == y) && ( (i128_t) iy % 2 != 0 ) )
            {
                if( y == -1.0 )
                {
                    result.f.d = x;

                    if( result.f.w[ W0 ] & SMASK )
                    {
                        result.f.w[ W0 ] = INF | SMASK;
                    }

                    else
                    {
                        result.f.w[ W0 ] = INF;
                    }
                }
                else
                {
                    result.f.w[ W0 ] = SMASK | INF;
                }
            }

            else if( y < 0.0 )
            {
                result.f.w[ W0 ] = INF;
            }

            break;
        }
    }

    return result.f.d;
}

/*
 * math_pow_safe_exp
 *
 * cast exponent y of function pow( x, y ) in a safe range of |x| < 1024
 * which corresponds with the range of 11bits of e in positive and negative direction: 2**{11-1} = 1023
 */

static i16_t math_pow_safe_exp( f128_t y )
{
    i16_t result;

    static const f128_t safe_exp = (f128_t) (DMAX>>1); /* 1023 */

    if( ( -safe_exp <= y ) && ( y <= +safe_exp ) )
    {
        result = (i16_t) y;
    }
    else
    {
        result = 0.0L;
    }

    return ( result );
}

static f128_t math_pow_ydexp( i64_t zl, f128_t y, f128_t dexp )
{
    static const f128_t ln2 = 0.69314718055994530942L;

    f128_t result;

    if ( zl == 0 )
    {
        result = y * dexp;
    }

    else
    {
        const f128_t yrnd = math_to_integer( y );

        result = ( ( yrnd * dexp ) - (f128_t) zl ) + ( y - yrnd ) * dexp;
    }

    return ( ln2 * result );
}

static f128_t math_pow_impl( dnorm_t xn, f128_t y )
{
    static const f128_t rthalf   = 0.70710678118654752440L;

    f128_t yi   = math_to_integer( y );

    if ( xn.f.d < rthalf )
    {
        xn.f.d  = 2.L * xn.f.d;
        xn.e    = xn.e - 1;
    }

    const f128_t dexp = ( f128_t ) xn.e;

    const i64_t zl = ( i64_t ) ( y * dexp );

    f128_t yx = math_pow_ydexp( zl, y, dexp );

    i16_t zexp = ( i16_t ) math_pow_clamp( zl, -1074, +1024 );

    const i16_t n = math_pow_safe_exp( y );

    yi = y - ( f128_t ) n;

    if ( yi != 0.0L )
    {
        yx = yx + ( yi * math_log( xn.f.d ) );
    }

    else
    {
        /* NOP: not need to compute math_log( xn ) when yi = 0 */
    }

    f128_t z = math_pow_i16( xn.f.d, n );

    if ( yx != 0.0L )
    {
        z = z * math_exp( yx );
    }

    else
    {
        z = 1.0L;
    }

    z = z * (f128_t) math_cwsetexp( 0.5, zexp );

    return ( z );
}

static u32_t math_pow_iabs( i32_t n )
{
    u32_t result;

    if ( n > 0 )
    {
        result  = ( u32_t ) n;
    }

    else
    {
        result  = ( u32_t ) -n;
    }

    return ( result );
}

static f128_t math_pow_i16( f128_t x, i16_t n )
{
    f128_t result = 1.0L; /* exact result when n = 0 */

    u32_t k = math_pow_iabs( n );

    f128_t x2 = x;

    while ( k > 0 )
    {
        if ( k & 1 )
        {
            result = result * x2;
        }

        k = k >> 1;

        if ( k > 0 )
        {
            x2 = x2 * x2;
        }
    }

    if ( n < 0 )
    {
        result = 1.0 / result;
    }

    return ( result );
}

static i64_t math_pow_clamp( i64_t x, i64_t xmin, i64_t xmax )
{
    i64_t result;

    if( x <= xmin )
    {
        result = xmin;
    }

    else if ( x < xmax )
    {
        result = x;
    }

    else
    {
        result = xmax;
    }

    return ( result );
}
