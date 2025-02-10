#include "public_math.h"
#include "private_math.h"

static i16_t    math_pow_safe_exp   ( f128_t y                          );
static u32_t    math_pow_iabs       ( i32_t n                           );
static f128_t   math_pow_impl       ( dnorm_t x, f128_t y               );
static f128_t   math_pow_i16        ( f128_t x, i16_t n                 );
static f128_t   math_pow_ydexp      ( i64_t zl, f128_t y,   f128_t dexp );
static i64_t    math_pow_clamp      ( i64_t x,  i64_t xmin, i64_t xmax  );

static f64_t math_pow_impl2( f64_t x, f64_t y )
{
/*  3. determine m */
/*  5. determine r, g */
/*  6. determine p */
/*  7. determine z */
/*  8. determine u2 = R(z) */
/*  9. determine w = y*u */
/* 10. determine w > BIGX */
/* 11. determine w < SMALLX */
/* 13. determine p', r', m' */
/* 14. determine z = n**w2 */
/* 15. determine result = z * B**m' * n**(-r'-p'/C) */
    return ( x*y );
}

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
                static const f128_t rthalf   = 0.70710678118654752440084436210485L;
                dnorm_t xn = math_cwnormalize( x, result.type );

                if ( xn.f.d < rthalf )
                {
                    xn.f.d  = 2.L * xn.f.d;
                    xn.e    = xn.e - 1;
                }

                const f128_t z = y * xn.e;

                const i16_t zl  = math_to_integer( y * ( f128_t ) xn.e );
                const f64_t n   = math_to_integer( y );
                const f128_t yx = math_pow_ydexp( zl, y, ( f128_t ) xn.e ) + ( ( y - ( f128_t ) n ) * math_log( xn.f.d ) );
                result.f.d      = math_pow_i16( xn.f.d, (i16_t) n ) * math_exp( yx ) * math_cwsetexp( 0.5, zl );
            }

            else
            {
                result.f.w[ W0 ] = NIL;
            }

            break;
        }

        case (INF):
        case (NIL):
        {
            result.f.w[ W0 ] = NIL;
            break;
        }

        case (ZERO): /* x^0 = 1, for all x >= 0, even 0^0 */
        {
            if( y > 0.0 )
            {
                result.f.w[ W0 ] = ZERO;
            }

            else
            {
                result.f.w[ W0 ] = NIL;
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
    static const f128_t ln2 = 0.69314718055994530941723212145818L;

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
    const i16_t zl = math_to_integer( y * ( f128_t ) xn.e );

    const f64_t n = math_to_integer( y );

    const f128_t yx = math_pow_ydexp( zl, y, ( f128_t ) xn.e ) + ( ( y - ( f128_t ) n ) * math_log( xn.f.d ) );

    const f128_t z = math_pow_i16( xn.f.d, (i16_t) n ) * math_exp( yx ) * math_cwsetexp( 0.5, zl );

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
