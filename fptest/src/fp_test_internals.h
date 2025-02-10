#ifndef FPTEST_INTERNALS_H
#define FPTEST_INTERNALS_H

#include "fptest.h"

/* Floating-Point Layout (valid for arc=x86_64) */

/*
 * f80_t (Extended precision) for arch=x86_64 with x87 FPU
 *
 * In terms of memory alignment, the extender precision type is padded to span 16 bytes
 * But only the lower address corresponding to the first 10 bytes are meaningful.
 */

#define LW0   	4 /* contains MSB */
#define LW1   	3
#define LW2   	2
#define LW3   	1
#define LW4   	0 /* contains LSB */

/* u64_t access to f80_t */

#define LU0     1 /* contains MSB */
#define LU1     0 /* contains LSB */

/*  f64_t (Double precision) */

#define DW0   	3 /* contains MSB */
#define DW1   	2
#define DW2   	1
#define DW3   	0 /* contains LSB */

/*  f32_t (Single precision) */

#define FW0   	1 /* contains MSB */
#define FW1   	0 /* contains LSB */

typedef union
{
	u16_t   w[ 4 ];
	u64_t   u;
	f64_t   f;
} dw_t;

typedef union
{
	u16_t   w[ 2 ];
	u32_t   u;
	f32_t   f;
} fw_t;

/* nbits sign, exponent, digits: 1, 8, 23 (+1 digit bit hidden) */
#define FW0_SMASK 0x8000
#define FW0_EMASK 0x7F80
#define FW0_DMASK 0x007F
#define FWx_DMASK 0xFFFF
#define DOFFS_F32 7         /* offset bits of exponent within FW0 */
#define EMAX_F32  127
#define EMIN_F32  ( 1 - EMAX_F32 )
#define NDIG_F32  24

/* nbits sign, exponent, digits: 1, 11, 52 (+1 digit bit hidden) */
#define DW0_SMASK 0x8000
#define DW0_EMASK 0x7FF0
#define DW0_DMASK 0x000F
#define DWx_DMASK 0xFFFF
#define DOFFS_F64 4         /* offset bits of exponent within DW0 */
#define EMAX_F64  1023
#define EMIN_F64  ( 1 - EMAX_F64 )
#define NDIG_F64  53

/* Constants */
static const fw_t m_inff32 	= { .u = 0x7F800000u, };
static const fw_t m_maxf32 	= { .u = 0x7F7FFFFFu, };
static const fw_t m_zerof32	= { .u = 0x00000000u, };
static const fw_t m_minf32 	= { .u = 0x00800001u, };
static const fw_t m_subnf32	= { .u = 0x00000001u, };
static const fw_t m_qnanf32	= { .u = 0x7FC00000u, };
static const fw_t m_epsf32	= { .u = 0x34000000u, };

static const dw_t m_inff64 	= { .u = 0x7FF0000000000000uL, };
static const dw_t m_maxf64 	= { .u = 0x7FEFFFFFFFFFFFFFuL, };
static const dw_t m_zerof64	= { .u = 0x0000000000000000uL, };
static const dw_t m_minf64 	= { .u = 0x0010000000000001uL, };
static const dw_t m_subnf64	= { .u = 0x0000000000000001uL, };
static const dw_t m_qnanf64	= { .u = 0x7FF8000000000000uL, };
static const fw_t m_epsf64	= { .f = (f64_t) 2.22044604925031308084726333618164062e-16L, };

#define FP32_ZERO   (0.0f)
#define FP32_MIN    (1.175494350822287507968737E-38f)
#define FP32_1R2	(0.5f)
#define FP32_PI4    (0.785398185253143310546875f)
#define FP32_R1     (1.0f)
#define FP32_SQRT2  (1.41421353816986083984375f)
#define FP32_PI2 	(1.57079637050628662109375f)
#define FP32_R2		(2.0f)
#define FP32_3PI4 	(2.35619449615478515625f)
#define FP32_EULER 	(2.71828174591064453125f)
#define FP32_PI 	(3.1415927410125732421875f)
#define FP32_5PI4   (3.9269907474517822265625f)
#define FP32_R4     (4.0f)
#define FP32_3PI2   (4.7123889923095703125f)
#define FP32_7PI4	(5.497786998748779296875)
#define FP32_2PI    (6.283185482025146484375f)
#define FP32_9PI4	(7.06858348846435546875f)
#define FP32_7PI    (21.9911479949951171875f)

#define FP64_ZERO   (0.0)
#define FP64_MIN    (2.2250738585072013830902327173324e-308)
#define FP64_1R2	(0.5)
#define FP64_PI4    (0.78539816339744830961566084581988)
#define FP64_R1     (1.0)
#define FP64_SQRT2  (1.4142135623730950488016887242097)
#define FP64_PI2 	(1.5707963267948966192313216916398)
#define FP64_R2     (2.0)
#define FP64_3PI4 	(2.3561944901923449288469825374596)
#define FP64_EULER 	(2.7182818284590452353602874713527)
#define FP64_PI 	(3.1415926535897932384626433832795)
#define FP64_5PI4   (3.9269908169872415480783042290994)
#define FP64_R4     (4.0)
#define FP64_3PI2   (4.7123889803846898576939650749193)
#define FP64_7PI4	(5.4977871437821381673096259207391)
#define FP64_2PI    (6.283185307179586476925286766559)
#define FP64_9PI4   (7.0685834705770347865409476123789)
#define FP64_7PI    (21.991148575128552669238503682957)

/* Private Types */
typedef struct
{
    i32_t xexp;
    u64_t count_points_in_the_range;
} fp_progress_state_t;

/* Private functions */
static f64_t    		fp32_ulps_impl ( f32_t approx, f80_t diff );
static f64_t    		fp64_ulps_impl ( f64_t approx, f80_t diff );

static u32_t    		fp32_progress_bar_status( f32_t x );
static void     		fp32_progress_bar( cstr_t desc, fpenv_t fpenv, f32_t x_curr, f32_t x_min, f32_t x_max, fp_progress_state_t * pstate );

static u32_t    		fp64_progress_bar_status( f64_t x );
static void     		fp64_progress_bar( cstr_t desc, fpenv_t fpenv, f64_t x_curr, f64_t x_min, f64_t x_max, fp_progress_state_t * pstate );

static void     		fp_console_remove_line( fpenv_t fpenv );

static i32_t 			fp80_compare(const void* a, const void* b);

static inline 	u64_t 	fp_get_time_ns();

/* Utilities */
#define LINE_SEPARATOR "========================================================================================="

#define F32_UNBIASED_TO_BIASED_EXP(e) ( (e) < EMIN_F32 ? EMIN_F32 : ( (e) > EMAX_F32 ? EMAX_F32 : (e) + EMAX_F32 ) )
#define F64_UNBIASED_TO_BIASED_EXP(e) ( (e) < EMIN_F64 ? EMIN_F64 : ( (e) > EMAX_F64 ? EMAX_F64 : (e) + EMAX_F64 ) )

#define F32_SET_EXP(w0,new_unbiased_exp) do { \
	( w0 ) = ( ( w0 ) & ~( (u16_t) FW0_EMASK ) ) | ( ( F32_UNBIASED_TO_BIASED_EXP(new_unbiased_exp) ) << DOFFS_F32 );\
} while ( 0 )

#define F64_SET_EXP(w0,new_unbiased_exp)\
do { \
	( w0 ) = ( ( w0 ) & ~( (u16_t) DW0_EMASK ) ) | ( ( F64_UNBIASED_TO_BIASED_EXP(new_unbiased_exp) ) << DOFFS_F64 );\
} while ( 0 )

#define F32_GET_EXP(x) ( (i16_t) ( ( ((fw_t){ .f = x }).w[ FW0 ] & FW0_EMASK ) >> DOFFS_F32 ) - EMAX_F32 )
#define F64_GET_EXP(x) ( (i16_t) ( ( ((dw_t){ .f = x }).w[ DW0 ] & DW0_EMASK ) >> DOFFS_F64 ) - EMAX_F64 )

#define TEST_ASSERT_EQUAL_DOUBLE(x,y) do{ f64_t expect = (x) ; f64_t given = (y); if( !fp64_equals( given, expect ) ){ fprintf(stderr, "[FAILED] %s, expect: %e, given: %e, lc:%d\n",__func__, expect, given, __LINE__ ); } } while(0)

/* >> KEY VALUE VECTOR */
#define MAX_CAPACITY (1u << 30)
#define GROWTH_RATIO 2

typedef struct
{
    const void * k;
    const void * v;
} kv_t;

typedef struct
{
    kv_t * elems;
    unsigned int len;
    unsigned int size;
} vector_t;
/* << KEY VALUE VECTOR */

#endif/*FPTEST_INTERNALS_H*/
