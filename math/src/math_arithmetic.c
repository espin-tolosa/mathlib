#include "private_math.h"

static f64_t math_add_2a ( f64_t a );
static f64_t math_add_impl( f64_t a, f64_t b );

f64_t math_add( f64_t a, f64_t b, u16_t * err )
{
    dw_t result = { 0 };

    /* if err is null send internal err assignemts to a null err, like sending to /dev/null */
    u16_t null_err;
    u16_t * safe_err_ptr = err? err : &null_err;

    dw_t da = { .d = a };
    dw_t db = { .d = b };

    u16_t ta = math_type( a );
    u16_t tb = math_type( b );

    if( ta == NIL || tb == NIL ) /* inclusive OR */
    {
        result.w[ W0 ] = NIL;
    }

    /* NIL is not possible here */
    else if( ta == INF || tb == INF ) /* inclusive OR */
    {
        u16_t signa = da.w[ W0 ] & SMASK;
        u16_t signb = db.w[ W0 ] & SMASK;

        /* +inf + +inf = +inf */
        /* +inf + -inf =  nan */
        /* -inf + +inf =  nan */
        /* -inf + -inf = -inf */

        const bool_t opposite_sign = signa ^ signb;

        if( opposite_sign )
        {
            result.w[ W0 ] = NIL;
            *safe_err_ptr = NIL;
        }

        else if( signa == 0 )
        {
            result.w[ W0 ] = INF;
        }

        else
        {
            result.w[ W0 ] = NINF;
        }
    }

    else if( ta == GRADZ || tb == GRADZ ) /* inclusive OR */
    {
        result.d = a + b; /* TODO: GRADUAL UNDERFLOW NOT SUPPORTED JET */
        *safe_err_ptr = GRADZ;
    }

    else if( ta == ZERO )
    {
        result.d = b;
    }

    else if( tb == ZERO )
    {
        result.d = a;
    }

    else
    {
        if( a == b )
        {
            /* if a==b, a+b = 2*a in which case is more optimal to just add +1 to exponent bits and return */
            result.d = math_add_2a( a );
        }

        else
        {
            result.d = math_add_impl( a, b );
        }
    }

    return ( result.d );
}

#define FMASK 0x000FFFFFFFFFFFFF
#define EMASK 0x7FF0000000000000
#define EXTBITS 2

/*
 * math_add_u64
 *
 * carr can be carried between calls
 */

u64_t math_add_u64( u64_t fu, u64_t fv, bool_t * ptr_carry )
{
    bool_t null_carr = 0;
    bool_t * safe_carry;

    /* protect null-ptr dereference, if null is passed to carr */
    if( ptr_carry )
    {
        safe_carry = ptr_carry;
    }
    else
    {
        safe_carry = &null_carr;
    }

    u64_t carry_in  = *safe_carry ? 1ULL : 0ULL;

    u64_t result    = fu + fv + carry_in;

    /* a = 1, b = MAX - 1, c = 1 -> r = a + b + c = 1 + MAX - 1 + 1 = MAX + 1 = 1 */
    /* overflow if: result < a || result < b = 1 < 1 || 1 < MAX - 1 = false || true = true */
    *safe_carry = ( result < fu ) || ( result < fv );

/* OLD METHOD
    u64_t result = 0;

    for( u16_t i = 0; i < 8*sizeof(u64_t); i++ )
    {
        u16_t ubit = ( fu & (1ULL<<i) ) >> i;
        u16_t vbit = ( fv & (1ULL<<i) ) >> i;

        u16_t wbit = ( ubit ^ vbit ) ^ *ptr_carry;

        *ptr_carry = ( ubit + vbit + *ptr_carry ) > 1 ? 1 : 0;

        result = result | ( (u64_t) wbit << i );
    }
*/

    return ( result );
}

static f64_t math_add_2a ( f64_t a )
{
    dw_t result = { 0 };

    dw_t u = { .d = a };

    const u16_t new_exp = ( ( (u.w[ W0 ] & DMASK) >> DOFF) + 1 ) << DOFF;

    result.w[ W0 ] = ( u.w[ W0 ] & MMASK ) | new_exp;
    result.w[ W1 ] = u.w[ W1 ];
    result.w[ W2 ] = u.w[ W2 ];
    result.w[ W3 ] = u.w[ W3 ];

    return ( result.d );
}

static f64_t math_add_impl( f64_t a, f64_t b )
{
    dw_t result = { 0 };

    dw_t u;
    dw_t v;

    if( a >= b )
    {
        u.d = a;
        v.d = b;
    }
    else
    {
        u.d = b;
        v.d = a;
    }

    u16_t uw0 = (u.w[ W0 ] & DMASK) | (u.w[ W0 ] & SMASK);
    u16_t vw0 = (v.w[ W0 ] & DMASK) | (v.w[ W0 ] & SMASK);

    u16_t uexp = ( ( uw0 & DMASK ) >> DOFF );
    u16_t vexp = ( ( vw0 & DMASK ) >> DOFF );

    u16_t ediff = uexp - vexp;

    /* significand bits are left shifted in order to */
    dw_t ufscal = { .u =   ( ( u.u & FMASK ) << EXTBITS )            } ; /* fu =                                     +11 extra bit */
    dw_t vfscal = { .u = ( ( ( v.u & FMASK ) << EXTBITS ) >> ediff ) } ; /* fv = fv / B**(ue-ve) = fv >> (ue-ve),    +11 extra bit */

    bool_t carr = 0;

    /* TODO: instead of using +11 extra bits, use 2 double words, using carry bit offered by function math_add_u64 */
    u64_t wfscal = math_add_u64( ufscal.u, vfscal.u, &carr );

    const u16_t new_exp = ( ( (u.w[ W0 ] & DMASK) >> DOFF) + 1 ) << DOFF;

    /* TODO: math_add_impl does not work completely because, renormalization and packing is missing */

    result.u = wfscal >> ( EXTBITS + 1 );

    result.w[ W0 ] = result.w[ W0 ] | new_exp;

    return ( result.d );
}
