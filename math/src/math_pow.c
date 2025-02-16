#include "public_math.h"
#include "private_math.h"

static i16_t    math_pow_safe_exp       ( f128_t y                          );
static u32_t    math_pow_iabs           ( i32_t n                           );
static f128_t   math_pow_i16            ( f128_t x, i16_t n                 );
static f128_t   math_pow_ydexp          ( i64_t zl, f128_t y,   f128_t dexp );
static i64_t    math_pow_clamp          ( i64_t x,  i64_t xmin, i64_t xmax  );
static f128_t   math_pow_impl           ( dnorm_t x, f128_t y               );

extern double d_reduce(double v);

static f64_t    math_pow_impl_old       ( f64_t x, f64_t y );

static f64_t    math_pow_finite_triage  ( f64_t x, f64_t y, bool_t is_int, bool_t is_even, u16_t xtype );

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
            const f64_t iy          = math_to_integer(y);
            const bool_t is_pos     = x > 0.0;
            const bool_t is_int     = math_type( iy - y ) == ZERO;
            const bool_t is_even    = ( ( (u64_t) math_abs( iy ) ) % 2 ) == 0;
            const u8_t   hash       = ( is_even << 2 ) | ( is_int << 1 ) | is_pos;

            switch ( hash )
            {
                case (0):
                case (4):/* x00 */
                {
                    result.f.w[ W0 ] = NIL;
                    break;
                }
                case(2):/* 010 */
                {
                    result.f.d = math_pow_i16( -x, y );
                    break;
                }
                case(6):/* 110 */
                {
                    result.f.d = -math_pow_i16( -x, y );
                    break;
                }
                case(3):
                case(7):/* x11 */
                {
                    result.f.d = math_pow_i16( x, y );
                    break;
                }
                case(1):
                case(5):/* x01 */
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
                        result.f.d = math_pow_impl_old( x, y );
                    }
                    break;
                }

                default: { /* NOP: not possible */ break; }
            }

            break;
        }

        case (INF):
        case (NIL):
        case (ZERO):
        {
            result.f.w[ W0 ] = NIL;
            break;
        }
    }

    return ( result.f.d );
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

    const i16_t n = math_pow_safe_exp( y ); /* TODO: this does not makes sense, as the parent switch has already handled y range */

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

static f64_t math_pow_impl_old( f64_t x, f64_t y )
{
    static const double a1[17] =
    {
        1.000000000000000000000000000000000000000000000000000e+00,
        9.576032806985737000360359161277301609516143798828125e-01,
        9.170040432046712153280054735660087317228317260742188e-01,
        8.781260801866497267553768324432894587516784667968750e-01,
        8.408964152537145020360753733257297426462173461914063e-01,
        8.052451659746271417361640487797558307647705078125000e-01,
        7.711054127039703720569718825572635978460311889648438e-01,
        7.384130729697496731134265246510040014982223510742188e-01,
        7.071067811865475727373109293694142252206802368164063e-01,
        6.771277734684463256442654710554052144289016723632813e-01,
        6.484197773255048202756256614520680159330368041992188e-01,
        6.209289060367420010067007751786150038242340087890625e-01,
        5.946035575013605134486738279520068317651748657226563e-01,
        5.693943173783457822878517617937177419662475585937500e-01,
        5.452538663326288448374157269427087157964706420898438e-01,
        5.221368912137068774015347116801422089338302612304688e-01,
        5.000000000000000000000000000000000000000000000000000e-01,
     };

    static const double a2[8] =
    {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    };

	static const double p1 = 0.83333333333333211405E-1;
	static const double p2 = 0.12500000000503799174E-1;
	static const double p3 = 0.22321421285924258967E-2;
	static const double p4 = 0.43445775672163119635E-3;

	static const double q1 = 0.69314718055994529629E+0;
	static const double q2 = 0.24022650695909537056E+0;
	static const double q3 = 0.55504108664085595326E-1;
	static const double q4 = 0.96181290595172416964E-2;
	static const double q5 = 0.13333541313585784703E-2;
	static const double q6 = 0.15400290440989765601E-3;
	static const double q7 = 0.14928852680595608186E-4;

	static const double k  = 0.44269504088896340763E+0;

//
    dnorm_t xn = math_cwnormalize( x, math_type( x ) );

    i32_t m = xn.e;

//5
    const double g = math_cwsetexp(x, 0);

//6
    short int p = 1;

    if( g <= a1[9-1] )
    {
        p = 9;
    }

    if( g <= a1[p+4-1] )
    {
        p = p + 4;
    }

    if( g <= a1[p+2-1] )
    {
        p = p + 2;
    }

//7
    double z = ((g - a1[p+1]) - a2[(p+1)/2]) / (g + a1[p+1]);

//    printf("x = %f, p = %d, a1[p+1] = %f, a2[(p+1)/2] =%f,  g = %f, z = %f ", x, p, a1[p+1], a2[(p+1)/2], g, z);
    z = z + z;

//8
    double v = z * z;
    double R  = (((p4 * v + p3) * v + p2) * v + p1) * v * z;

    R = R + k * R;

    double u2 = (R + z * k) + z;

//9
    double u1 = ((double)(xn.e * 16 - p )) * 0.0625;

    double w = y * (u1 + u2);

    double w1 =  d_reduce(w);
    double w2 = w - w1;
    int   iw1 = (int)w1;

    if(w < 0.0001 || 1.e10 < w)
    {
        return 0.0; // TODO: handle this step 12
    }

    iw1 = iw1 + 1;

    int I;

    if(iw1 < 0.0)
    {
        I = 0;
    }
    else
    {
        I = 1;
    }

    int m_ = iw1 / 16 + I;
    int p_ = 16*m_ - iw1;

    z = ((((((q7 * w2 + q6) * w2 + q5) * w2 + q4) * w2 + q3) * w2 + q2) * w2 + q1) * w2;

    z = 1.0 + z;

    z = z * a1[p_ + 1] + a1[p_ + 1];

    dnorm_t zn = math_cwnormalize( z, math_type( z ) );

//    printf("\n");

    return math_cwsetexp(z, m_ + zn.e);
}

extern double d_reduce(double v)
{
    return (double) ((int)(16.0 * v)) * 0.0625;
}
