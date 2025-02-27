#include "private_math.h"

static const char * math_convert_fp64_sprint_exact      ( double x );
static int          math_convert_fp64_point_find_abs    ( const char * x );
static int          math_convert_fp64_point_abs2rel     ( const char * x ,int n );
static int          math_convert_fp64_point_rel2abs     ( const char * x ,int n );
static const char * math_convert_fp64_add1              ( char * d, char a, int n );
static const char * math_convert_fp64_mul2              ( char * d );
static const char * math_convert_fp64_div2              ( char * d );

#define TO_CHAR(d) ((d)+'0')
#define TO_DIGIT(c) ((c)-'0')
#define FP64_SPRINT_LEN_LEFT 309
#define FP64_SPRINT_LEN_RIGHT 1074
#define FP64_SPRINT_LEN_EXTRA 4
#define FP64_SPRINT_LEN ( FP64_SPRINT_LEN_LEFT + FP64_SPRINT_LEN_RIGHT + FP64_SPRINT_LEN_EXTRA )
#define FP64_CENTER_POINT ( 1 + FP64_SPRINT_LEN_LEFT )
#define INIT_FLOAT_STRING(var) \
    char (var)[ FP64_SPRINT_LEN + 1 ]; \
    for(int i = 0; i < FP64_SPRINT_LEN; i++) { (var)[ i ] = '0'; }; \
    (var)[ FP64_SPRINT_LEN ] = '\0'; \
    (var)[ FP64_CENTER_POINT  ] = '.'

extern const char * math_print( double x )
{
    static INIT_FLOAT_STRING(c);

    const char * ret;

    switch( math_type( x ) )
    {
        case( INF ):
        {
            if( x > 0.0 )
            {
                ret = "+inf";
            }
            else
            {
                ret = "-inf";
            }
            break;
        }

        case( NIL ):
        {
            ret = "nan";
            break;
        }

        case( ZERO ):
        case( GRADZ ):
        case( FINITE ):
        {
            const char * d = math_convert_fp64_sprint_exact( x );

            int i, j;

            for( i = 0; i < (FP64_SPRINT_LEN - 1); i++ )
            {
                if( ( ('1' <= d[i]) && (d[i] <= '9') ) )
                {
                    if( i > 0 )
                    {
                        i -= 1;
                    }
                    break;
                }
                else if( d[i] == '.')
                {
                    if( i > 1 )
                    {
                        i -= 2;
                    }

                    break;
                }

                else
                {
                    /* NOP */
                }
            }

            j = FP64_SPRINT_LEN - 1;

            for( j = FP64_SPRINT_LEN - 1; j >= 0; j-- )
            {
                if( ( ('1' <= d[j]) && (d[j] <= '9') ) )
                {
                    break;
                }
                else if( d[j] == '.' )
                {
                    break;
                }
                else
                {
                    /* NOP */
                }
            }

            for( int k = i; k <= j; k++ )
            {
                c[ k ] = d[ k ];
            }

            for( int k = j + 1; k < (FP64_SPRINT_LEN - 1); k++ )
            {

                c[ k ] = '\0';
            }

            ret = ( &c[ i + 1 ] );

            break;
        }
    }

    return( ret );
}

static const char * fractions[ ] =
{
    "1.0000000000000000000000000000000000000000000000000000",
    "0.5000000000000000000000000000000000000000000000000000",
    "0.2500000000000000000000000000000000000000000000000000",
    "0.1250000000000000000000000000000000000000000000000000",
    "0.0625000000000000000000000000000000000000000000000000",
    "0.0312500000000000000000000000000000000000000000000000",
    "0.0156250000000000000000000000000000000000000000000000",
    "0.0078125000000000000000000000000000000000000000000000",
    "0.0039062500000000000000000000000000000000000000000000",
    "0.0019531250000000000000000000000000000000000000000000",
    "0.0009765625000000000000000000000000000000000000000000",
    "0.0004882812500000000000000000000000000000000000000000",
    "0.0002441406250000000000000000000000000000000000000000",
    "0.0001220703125000000000000000000000000000000000000000",
    "0.0000610351562500000000000000000000000000000000000000",
    "0.0000305175781250000000000000000000000000000000000000",
    "0.0000152587890625000000000000000000000000000000000000",
    "0.0000076293945312500000000000000000000000000000000000",
    "0.0000038146972656250000000000000000000000000000000000",
    "0.0000019073486328125000000000000000000000000000000000",
    "0.0000009536743164062500000000000000000000000000000000",
    "0.0000004768371582031250000000000000000000000000000000",
    "0.0000002384185791015625000000000000000000000000000000",
    "0.0000001192092895507812500000000000000000000000000000",
    "0.0000000596046447753906250000000000000000000000000000",
    "0.0000000298023223876953125000000000000000000000000000",
    "0.0000000149011611938476562500000000000000000000000000",
    "0.0000000074505805969238281250000000000000000000000000",
    "0.0000000037252902984619140625000000000000000000000000",
    "0.0000000018626451492309570312500000000000000000000000",
    "0.0000000009313225746154785156250000000000000000000000",
    "0.0000000004656612873077392578125000000000000000000000",
    "0.0000000002328306436538696289062500000000000000000000",
    "0.0000000001164153218269348144531250000000000000000000",
    "0.0000000000582076609134674072265625000000000000000000",
    "0.0000000000291038304567337036132812500000000000000000",
    "0.0000000000145519152283668518066406250000000000000000",
    "0.0000000000072759576141834259033203125000000000000000",
    "0.0000000000036379788070917129516601562500000000000000",
    "0.0000000000018189894035458564758300781250000000000000",
    "0.0000000000009094947017729282379150390625000000000000",
    "0.0000000000004547473508864641189575195312500000000000",
    "0.0000000000002273736754432320594787597656250000000000",
    "0.0000000000001136868377216160297393798828125000000000",
    "0.0000000000000568434188608080148696899414062500000000",
    "0.0000000000000284217094304040074348449707031250000000",
    "0.0000000000000142108547152020037174224853515625000000",
    "0.0000000000000071054273576010018587112426757812500000",
    "0.0000000000000035527136788005009293556213378906250000",
    "0.0000000000000017763568394002504646778106689453125000",
    "0.0000000000000008881784197001252323389053344726562500",
    "0.0000000000000004440892098500626161694526672363281250",
    "0.0000000000000002220446049250313080847263336181640625",
};

static const char * math_convert_fp64_sprint_exact( double x )
{
    /* ACCESS PRINTABLE VALUE */
    unsigned long long * w = (unsigned long long * ) &x;
    unsigned long long u64 =  *w ;//0xfffffffffffff;

    /* INITIALIZE NORMAL NUMBER */
    static INIT_FLOAT_STRING(buff);

    /* ADD FRACTIONAL BITS AND COUNT OFFSET TO FIRST ACTIVE BIT FRACTION (USED FOR SUBNORMAL NUMBERS) */
    int off = 0;
    unsigned long long mask = 0x8000000000000; /* MSB of frac is active */
    for(int i = 1; i < FP64_SPRINT_LEN; ++i )
    {
        if( u64 & mask )
        {
            const char * b = fractions[ i ];

            for( int j = 53; j > 1; j-- )
            {
                math_convert_fp64_add1( buff, b[j], -j + 1 );
            }
        }

        else if( off == 0 )
        {
            off++;
        }

        mask >>= 1u;
    }

    /* SCALE BY EXP */

    int biased = ( (int) ( ( u64 & 0x7FF0000000000000LLU ) >> 52u ) );
    int xe = ( (int) ( biased ) ) - 1023;

    if( biased != 0 )
    {
        /* ADD HIDDEN BIT IF NORMAL 1.000000... */
        math_convert_fp64_add1( buff, '1', 1 );
    }
    else
    {
        /* ADD OFFSET EXPONENT IF SUBNORMAL */
        xe += off;
    }

    if( xe > 0 )
    {
        while( xe > 0 )
        {
            math_convert_fp64_mul2( buff );
            xe--;
        }
    }

    else if( xe < 0 )
    {
        while( xe < 0 )
        {
            math_convert_fp64_div2( buff );
            xe++;
        }
    }

    else
    {
        /* NOP: no need to scale */
    }

        return ( buff );
}

static int math_convert_fp64_point_find_abs( const char * x )
{
    int len = 0;

    while( len < FP64_SPRINT_LEN )
    {
        if( x[ len ] == '\0' )
        {
            break;
        }
        else
        {
            len++;
        }
    }

    for( int i = 0; i < len; i++ )
    {
        if(x[ i ] == '.')
        {
            return ( i );
        }
    }

    return ( len );
}

static int math_convert_fp64_point_abs2rel( const char * x , int n )
{
    return ( math_convert_fp64_point_find_abs( x ) - n );
}

static int math_convert_fp64_point_rel2abs  ( const char * x , int n )
{
    return ( -n + math_convert_fp64_point_find_abs( x ) );
}

static const char * math_convert_fp64_div2( char * d )
{
    int crr = 0;

    int i = 0;

    while( i < FP64_SPRINT_LEN )
    {
        if( d[ i ] == '.' )
        {
            i++;
            continue;
        }

        const int t = crr + TO_DIGIT( d[ i ] );

        if( t == 0 )
        {
            i++;
            continue;
        }

        d[ i ] = TO_CHAR( t / 2 );
        crr    = 10 * ( t % 2 );
        i++;
    }
}

static const char * math_convert_fp64_add1( char * d, char a, int n )
{
    int i = -n + math_convert_fp64_point_find_abs( d );

    int crr = TO_DIGIT( a );

    while( crr != 0 && i >= 0 )
    {
        if( d[i] == '.' )
        {
            i--;
            continue;
        }

        const int   ti      = TO_DIGIT( d[ i ] ) + crr;
                    d[ i ]  = TO_CHAR( ti % 10 );
                    crr     = ti / 10;

        i--;
    }

    return d;
}

static const char * math_convert_fp64_mul2( char * d )
{
    int crr = 0;

    int i = FP64_SPRINT_LEN - 1;

    while( i > 0 )
    {
        if( d[ i ] == '.' )
        {
            i--;
            continue;
        }

        const int t = ( crr + 2 * TO_DIGIT( d[ i ] ) );

        if( t == 0 )
        {
            i--;
            continue;
        }

        d[ i ] = TO_CHAR( t % 10 );
        crr    = ( t / 10 );
        i--;
    }
}
