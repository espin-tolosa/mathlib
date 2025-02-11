#ifndef PRIVATE_MATH_H
#define PRIVATE_MATH_H

/*
 * Model of Float and Double representation in a little endian machine
 *
 * FLOAT______________________________________________
 *
 * |  31 | 30      23 | 22                         0 |
 * |  S  | EEEE EEEE  | MMMM MMMM MMMM MMMM MMMM MMM |
 *
 * S = Sign bit (1 bit), offset 31
 * E = Exponent (8 bits), offsets 30-23
 * M = Mantissa (23 bits), offsets 22-0
 *
 * DOUBLE______________________________________________________________________________________
 *
 * |  63 | 62          52 | 51                                                             0 |
 * |  S  | EEEE EEEE EEE  | MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM |
 *
 * S = Sign bit (1 bit), offset 63
 * E = Exponent (11 bits), offsets 62-52
 * M = Mantissa (52 bits), offsets 51-0
 *
 * Unions
 *
 * FLOAT N/A
 *
 * DOUBLE______________________________________________________________________________________
 *
 * Unsigned short [4] representation:
 *
 * A double can be accessed as an array of four 16 bits values.
 *
 * |SEEEEEEEEEEEMMMM|MMMMMMMMMMMMMMMM|MMMMMMMMMMMMMMMM|MMMMMMMMMMMMMMMM|
 * |    16  bits    |    16  bits    |    16  bits    |    16  bits    |
 * |    D0 = 3      |    D1 = 2      |    D2 = 1      |    D3 = 0      |
 *                                                                   LSB
 *
 * Handling gradual underflow
 *
 * Gradual underflow is designed to avoid from abrupt underflow.
 *
 *         Min. Subnorm. , Min. Normal
 * Float : 1.4 x 10-45   , 1.2 x 10-38
 * Double: 4.9 x 10-324  , 2.2 x 10-308
 */

/*
 * Definitions:
 *
 * Zero
 *		A value that sets bits [62:0] to zero. The sign bit (bit 63) is optional.
 *
 * Inf
 *		A value that sets all bits of the exponent field to one and all other bits to zero. The sign bit is optional.
 *
 * NaN
 *		A value that sets all bits of the exponent field to one and at least one bit of the significand field to one. The sign bit is optional.
 *
 * Finite
 *		Any value that does not fall into the categories of Zero, Inf, or NaN.
 */

typedef _Bool	            bool_t;

typedef unsigned char  		u8_t;
typedef signed   char  		i8_t;

typedef signed   short 		i16_t;
typedef unsigned short 		u16_t;

typedef unsigned int   		u32_t;
typedef signed   int   		i32_t;

typedef unsigned long long 	u64_t;
typedef signed   long long 	i64_t;
typedef __int128_t			i128_t;

typedef float		   		f32_t;
typedef double         		f64_t;
typedef long double    		f128_t;

#define NULLPTR ((void*) 0)

static const double invsqrt2 = 0.70710678118654752440084436210485;

#define W0   	3
#define W1   	2
#define W2   	1
#define W3   	0

#define EMAX	(1023)
#define EMIN	(1-EMAX)
#define DOFF 	4 						/* Number of fractional bits (M) in W0 */
#define MOFF    48						/* Number of fractional bits (M) in W1 to W3*/
#define DMASK 	0x7FF0 					/* A mask for the exponential bits (E) in W0 */
#define EDMASK 	0x7FFF 					/* A mask for the exponential bits (E) in W0 plus the fractional bits */
#define DMAX    0x07FF 					/* Counts the 11 bits of the exponent */
#define DFRAC	0x000F 					/* Counts the 4 bits of the fractional bits (M) in W0 */
#define SMMASK  0x800F 					/* A mask for the sign (S) in W0 plus the four fractinal bits (M) in W0 */
#define DBIAS   0x03FE 					/* The value emax−1 is subtracted from the bits of the exponent of a double to obtain the fraction 0.FFF... */
#define EXPHALF 0x3FE0 					/* Exponent bit pattern corresponding to the range [0.5, 1.0) */
#define EXPONE  0x3FF0					/* Exponent bit pattern corresponding to the range [1.0, 2.0) */
#define SMASK   0x8000 					/* A mask for the sign in W0 */
#define MMASK   0x000F 					/* A mask for the four fractional bits (M) in W0 */
#define NBITS	(MOFF+DOFF)				/* Number of significand bits, also called precission p*/

#define INF  	0x7FF0 					/* math_type +INF, Sets to 1 all the exponential bits (E) in W0 */
#define NINF	(INF | SMASK)			/* math_type -INF, Sets to 1 all the exponential bits (E) in W0 and also SIGN bit */
#define NIL  	0x7FFF 					/* math_type NIL, Same as INF plus the four fractional bits (M) in W0 */
#define FINITE	0x8000 					/* Identifies a number which is either not zero, inf or nan */
#define ZERO    0x0000 					/* math_type Zero */
#define GRADZ   0x0001 					/* math_Type Gradual underflow */

#define DBL_EPS	 2.2204460492503131e-16	/* Machine epsilon */
#define DBL_SQE  1.50e-08 				/* Square Root of Machine epsilon*/

#define MATH_SIN_RANGE 1.e4 /* Domain of sin/cos functions: [ -HUGE_VAL < x < +HUGE_VAL ] 1.e6 is choosen using Niquist and sampling rate of floating points */
#define MATH_EXP_LO_RANGE -744.03460681309024948859587
#define MATH_EXP_HI_RANGE +709.782712893383973096 // +709.78271289338399684324569237317

typedef union
{
	u16_t	w[ 4 ];
	u64_t   u;
	f64_t	d;
} dw_t;

/*
 * |SEEEEEEEEEEEMMMM|MMMMMMMMMMMMMMMM|MMMMMMMMMMMMMMMM|MMMMMMMMMMMMMMMM|
 * |    16  bits    |    16  bits    |    16  bits    |    16  bits    |
 * |    D0 = 3      |    D1 = 2      |    D2 = 1      |    D3 = 0      |
 */

typedef struct
{
	dw_t	f;
	i16_t 	e;
	u16_t	type;
} dnorm_t;

extern f64_t	math_abs			( f64_t x );
extern u16_t 	math_type 			( f64_t x );
extern f64_t 	math_intrnd			( f64_t x );
extern f64_t 	math_to_integer		( f64_t x );
extern dnorm_t 	math_cwnormalize 	( f64_t x, u16_t type );
extern f64_t 	math_cwsetexp		( f64_t x, i16_t n);
extern f128_t   math_horner      	( f128_t x, const f128_t * poly, i32_t n );

#endif
