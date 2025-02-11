#ifndef FPTEST_H
#define FPTEST_H

/* Boolean */
#if ((__STDC_VERSION__ + 0) >= 199901L)
    typedef _Bool bool_t;
#else
    typedef char bool_t;
#endif

/* Character */
typedef char         char_t;
typedef const char * cstr_t;

/* Integers */
typedef unsigned char       u8_t;
typedef unsigned short      u16_t;
typedef unsigned int        u32_t;
typedef unsigned long long  u64_t;

typedef signed   char       i8_t;
typedef signed   short      i16_t;
typedef signed   int        i32_t;
typedef signed   long long  i64_t;

/* Floating Points */
typedef float               f32_t;
typedef double              f64_t;
typedef long double         f80_t;

/* Function Pointers */
typedef void   ( *fp_f2v_t ) ( f32_t );
typedef void   ( *fp_d2v_t ) ( f64_t );

typedef f32_t  ( *fp_f2f_t   ) ( f32_t );
typedef f64_t  ( *fp_d2d_t   ) ( f64_t );
typedef f80_t  ( *fr_e2e_t   ) ( f80_t );

typedef f32_t  ( *fp_ff2f_t ) ( f32_t, f32_t );
typedef f64_t  ( *fp_dd2d_t ) ( f64_t, f64_t );

typedef f64_t  ( *fp_di162d_t ) ( f64_t x, i16_t n );

/* Composites */
typedef struct { u32_t x0; u32_t x1;            } u32_vec2_t;
typedef struct { f32_t x0; f32_t x1;            } fp32_vec2_t;
typedef struct { f32_t x0; f32_t x1; f32_t x2;  } fp32_vec3_t;

typedef struct { u64_t x0; u64_t x1;            } u64_vec2_t;
typedef struct { f64_t x0; f64_t x1;            } fp64_vec2_t;
typedef struct { f64_t x0; f64_t x1; f64_t x2;  } fp64_vec3_t;

/* Enums */
typedef enum
{
    NAMED_FP_INF,       /* -inf */
    NAMED_FP_MAXNORM,   /* 1e+38, 1.e+308 */
    NAMED_FP_ZERO,      /* +0.0 */
    NAMED_FP_MINNORM,   /* 1e-38, 1.e-308 */
    NAMED_FP_MINSUBN,   /* 2**(emin-p) = 2**(-126-23) ~ 1.4e-45 (f32_t) , ~2.47e-324 (f64_t),  */
} named_fp_t;

/*
 * identifes the type of floating-point value according to the union of sets:
 * { NAN } U { ZERO } U { NORMAL } U { SUBNORMAL } U { INFINITE }
 */

typedef enum
{
    FP_TYPE_NAN,
    FP_TYPE_ZERO,
    FP_TYPE_NORMAL,
    FP_TYPE_SUBNORMAL,
    FP_TYPE_INFINITE,
} fpset_t;

/*
 * Triplet structure
 */

typedef struct
{
    u64_t M; /* integral significand: M = m·B^{-p+1}, with m: 1 <= m < B (if normal), or 0 < m < 1 (if subnormal) */
    i16_t e; /* exponent: emin <= e <= emax, emin = 1 - emax, with 255 reserved for f32_t qNaN, and 2047 for f64_t qNaN */
    i16_t s; /* { -1, +1 } */
} fptriplet_t;

typedef enum
{
    FP32,
    FP64,
    FP80,
} fp_width_t;

typedef struct
{
    u64_t   dc;
    u64_t   fr;
    i8_t    s ;
    bool_t  ok;
    /* data */
} fp_radix10_t;

/* You should define ADD_EXPORTS *only* when building the DLL. */
#ifdef ADD_EXPORTS
  #define ADDAPI __declspec(dllexport)
#else
  #define ADDAPI __declspec(dllimport)
#endif

/* Define calling convention in one place, for convenience. */
#define ADDCALL __cdecl

/* Make sure functions are exported with C linkage under C++ compilers. */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    bool_t debug_next_x_inside_boundaries;

    bool_t log_completation;
    bool_t log_completation_remove_history;

    f32_t  fp32_grow_frac;
    f32_t  fp64_grow_frac;

    f32_t  fp32_ctrl_ulps;
    f32_t  fp64_ctrl_ulps;

    f64_t fp32_range_sqrt_min;
    f64_t fp32_range_sqrt_max;
    f64_t fp32_range_exp_min;
    f64_t fp32_range_exp_max;
    f64_t fp32_range_log_min;
    f64_t fp32_range_log_max;
    f64_t fp32_range_sin_min;
    f64_t fp32_range_sin_max;
    f64_t fp32_range_cos_min;
    f64_t fp32_range_cos_max;
    f64_t fp32_range_asin_min;
    f64_t fp32_range_asin_max;
    f64_t fp32_range_powy_min;
    f64_t fp32_range_powy_max;

    f64_t fp64_range_sqrt_min;
    f64_t fp64_range_sqrt_max;
    f64_t fp64_range_exp_min;
    f64_t fp64_range_exp_max;
    f64_t fp64_range_log_min;
    f64_t fp64_range_log_max;
    f64_t fp64_range_sin_min;
    f64_t fp64_range_sin_max;
    f64_t fp64_range_cos_min;
    f64_t fp64_range_cos_max;
    f64_t fp64_range_asin_min;
    f64_t fp64_range_asin_max;
    f64_t fp64_range_powy_min;
    f64_t fp64_range_powy_max;

    f64_t fp32_sqrt_accepted;
    f64_t fp32_sqrt_rejected;
    f64_t fp32_exp_accepted;
    f64_t fp32_exp_rejected;
    f64_t fp32_log_accepted;
    f64_t fp32_log_rejected;
    f64_t fp32_sin_accepted;
    f64_t fp32_sin_rejected;
    f64_t fp32_cos_accepted;
    f64_t fp32_cos_rejected;
    f64_t fp32_asin_accepted;
    f64_t fp32_asin_rejected;
    f64_t fp32_powy_accepted;
    f64_t fp32_powy_rejected;

    f64_t fp64_sqrt_accepted;
    f64_t fp64_sqrt_rejected;
    f64_t fp64_exp_accepted;
    f64_t fp64_exp_rejected;
    f64_t fp64_log_accepted;
    f64_t fp64_log_rejected;
    f64_t fp64_sin_accepted;
    f64_t fp64_sin_rejected;
    f64_t fp64_cos_accepted;
    f64_t fp64_cos_rejected;
    f64_t fp64_asin_accepted;
    f64_t fp64_asin_rejected;
    f64_t fp64_powy_accepted;
    f64_t fp64_powy_rejected;

    f32_t fp32_pow_y;
    f64_t fp64_pow_y;

} fpenv_t;

extern ADDAPI fpenv_t g_fpenv;

/* Functions */
ADDAPI extern f64_t         ADDCALL fp32_ulps( f32_t approx, f32_t exact );
ADDAPI extern f64_t         ADDCALL fp64_ulps( f64_t approx, f64_t exact );

ADDAPI extern bool_t        ADDCALL fp32_equals( f32_t a, f32_t b );
ADDAPI extern bool_t        ADDCALL fp64_equals( f64_t a, f64_t b );

ADDAPI extern bool_t        ADDCALL fp32_equals_sign( f32_t a, f32_t b );
ADDAPI extern bool_t        ADDCALL fp64_equals_sign( f64_t a, f64_t b );

ADDAPI extern f32_t         ADDCALL fp32_geom_step_real_line( f32_t x, f32_t frac );
ADDAPI extern f64_t         ADDCALL fp64_geom_step_real_line( f64_t x, f64_t frac );

ADDAPI extern fpset_t       ADDCALL fp32_get_subset( f32_t x );
ADDAPI extern fpset_t       ADDCALL fp64_get_subset( f64_t x );

ADDAPI extern cstr_t        ADDCALL fp32_get_subset_name( f32_t x );
ADDAPI extern cstr_t        ADDCALL fp64_get_subset_name( f64_t x );

ADDAPI extern f32_t         ADDCALL fp32_get_named_fp_in_real_line( named_fp_t point );
ADDAPI extern f64_t         ADDCALL fp64_get_named_fp_in_real_line( named_fp_t point );

ADDAPI extern cstr_t        ADDCALL fp32_sprint_digits_radix2( char_t buff [ 32 + 2 + 1 ], char_t sepparator, f32_t x );
ADDAPI extern cstr_t        ADDCALL fp64_sprint_digits_radix2( char_t buff [ 64 + 2 + 1 ], char_t sepparator, f64_t x );

ADDAPI extern f32_t         ADDCALL fp32_next_float( f32_t x );
ADDAPI extern f64_t         ADDCALL fp64_next_float( f64_t x );

ADDAPI extern f32_t         ADDCALL fp32_jump_nulp( f32_t x, i32_t n );
ADDAPI extern f64_t         ADDCALL fp64_jump_nulp( f64_t x, i32_t n );

ADDAPI extern f32_t         ADDCALL fp32_prev_float( f32_t x );
ADDAPI extern f64_t         ADDCALL fp64_prev_float( f64_t x );

ADDAPI extern f32_t         ADDCALL fp32_mount_bitfields( u32_t s, u32_t e, u32_t m );
ADDAPI extern f64_t         ADDCALL fp64_mount_bitfields( u64_t s, u64_t e, u64_t m );

ADDAPI extern fptriplet_t   ADDCALL fp32_get_triplet( f32_t x );
ADDAPI extern fptriplet_t   ADDCALL fp64_get_triplet( f64_t x );

ADDAPI extern i16_t         ADDCALL fp32_get_exp( f32_t x );
ADDAPI extern i16_t         ADDCALL fp64_get_exp( f64_t x );

ADDAPI extern f32_t         ADDCALL fp32_eval_triplet( fptriplet_t x );
ADDAPI extern f64_t         ADDCALL fp64_eval_triplet( fptriplet_t x );

ADDAPI extern f32_t         ADDCALL fp32_set_exp( f32_t x, i16_t n );
ADDAPI extern f64_t         ADDCALL fp64_set_exp( f64_t x, i16_t n );

ADDAPI extern fp32_vec2_t   ADDCALL fp32_find_control_boundaries( f32_t at_x, const f32_t * control_points,  f64_t boundary_semi_length_ulps, i32_t n );
ADDAPI extern fp64_vec2_t   ADDCALL fp64_find_control_boundaries( f64_t at_x, const f64_t * control_points,  f64_t boundary_semi_length_ulps, i32_t n );

ADDAPI extern f32_t         ADDCALL fp32_next_x( f32_t x, f32_t frac, const f32_t * control_points, f64_t round_ulps, i32_t n );
ADDAPI extern f64_t         ADDCALL fp64_next_x( f64_t x, f64_t frac, const f64_t * control_points, f64_t round_ulps, i32_t n );

ADDAPI extern void          ADDCALL fp32_range_analyzer( cstr_t desc, fp_f2f_t lhs, fp_f2f_t rhs, fp_f2f_t next_x, f64_t accept, f64_t reject, u8_t state[ /* TBD: Size in bytes of the state */ ], fp32_vec2_t minmax );
ADDAPI extern void          ADDCALL fp64_range_analyzer( cstr_t desc, fp_d2d_t lhs, fp_d2d_t rhs, fp_d2d_t next_x, f64_t accept, f64_t reject, u8_t state[ /* TBD: Size in bytes of the state */ ], fp64_vec2_t minmax );

ADDAPI extern u64_vec2_t*   ADDCALL fp_histogram_set_ulp( f64_t ulp, const void * x, fp_width_t precision, f64_t reject );
ADDAPI extern u16_t         ADDCALL fp_histogram_compute_table_hash( fptriplet_t triplet, fpset_t type, i16_t emax, i16_t emin );
ADDAPI extern cstr_t        ADDCALL fp_histogram_get_erange( i16_t hash, i16_t emax, i16_t emin );

ADDAPI extern f32_t         ADDCALL fp32_geometric_grow( f32_t x );
ADDAPI extern f64_t         ADDCALL fp64_geometric_grow( f64_t x );

ADDAPI extern void          ADDCALL fp32_print_benchmark_avg_time_evolution( fp_f2f_t fptr3232 );
ADDAPI extern void          ADDCALL fp64_print_benchmark_avg_time_evolution( fp_d2d_t fptr6464 );

ADDAPI extern void          ADDCALL fp32_print_benchmark_avg_time( cstr_t fname, fp_f2f_t fptr3232 );
ADDAPI extern void          ADDCALL fp64_print_benchmark_avg_time( cstr_t fname, fp_d2d_t fptr6464 );

ADDAPI extern fp64_vec2_t   ADDCALL  fp32_benchmark_avg_time( fp_f2f_t test_func );
ADDAPI extern fp64_vec2_t   ADDCALL  fp64_benchmark_avg_time( fp_d2d_t test_func );

ADDAPI extern f64_t         ADDCALL fp32_benchmark_core_ns_per_call( fp_f2f_t test_fun, fp_f2f_t mock_fun );
ADDAPI extern f64_t         ADDCALL fp64_benchmark_core_ns_per_call( fp_d2d_t test_fun, fp_d2d_t mock_fun );

ADDAPI extern f32_t         ADDCALL fp32_benchmark_mock_fun( f32_t x );
ADDAPI extern f64_t         ADDCALL fp64_benchmark_mock_fun( f64_t x );

ADDAPI extern void          ADDCALL fp32_test_sqrt      ( fp_f2f_t      tested_sqrt     , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_exp       ( fp_f2f_t      tested_exp      , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_log       ( fp_f2f_t      tested_log      , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_sin       ( fp_f2f_t      tested_sin      , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_cos       ( fp_f2f_t      tested_cos      , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_asin      ( fp_f2f_t      tested_asin     , bool_t active );
ADDAPI extern void          ADDCALL fp32_test_pow       ( fp_ff2f_t     tested_pow      , bool_t active );

ADDAPI extern void          ADDCALL fp64_test_sqrt      ( fp_d2d_t      tested_sqrt     , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_exp       ( fp_d2d_t      tested_exp      , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_log       ( fp_d2d_t      tested_log      , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_sin       ( fp_d2d_t      tested_sin      , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_cos       ( fp_d2d_t      tested_cos      , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_asin      ( fp_d2d_t      tested_asin     , bool_t active );
ADDAPI extern void          ADDCALL fp64_test_pow       ( fp_dd2d_t     tested_pow      , bool_t active );

#define FP32_BENCHMARK_MATH_FUNCTION(test_fun) fp32_benchmark_core_ns_per_call( (test_fun), fp32_benchmark_mock_fun )
#define FP64_BENCHMARK_MATH_FUNCTION(test_fun) fp64_benchmark_core_ns_per_call( (test_fun), fp64_benchmark_mock_fun )

/* >> Random Numbers */
ADDAPI extern f32_t         ADDCALL fp32_rand_in_range( f32_t min, f32_t max );
ADDAPI extern f64_t         ADDCALL fp64_rand_in_range( f64_t min, f64_t max );
/* << Random Numbers */

/* >> Arbitrary Radix-10 Arithmetics */
ADDAPI extern fp_radix10_t  ADDCALL fp_radix10_add( fp_radix10_t a, fp_radix10_t b );
/* << Arbitrary Radix-10 Arithmetics */

/* PUBLIC UTILITIES */
#define NULLPTR ((void*) 0)
#define TRUE  ((bool_t) 1)
#define FALSE ((bool_t) 0)
#define SIZEOF(array) ( sizeof( array ) / sizeof( (array)[0] ) )
#define noinline __attribute__((__noinline__))

/* UNIT TEST */
#define TEST_INIT static int m_ntest_failed = 0
#define TEST_ASSERT_EQUAL_FP64(a,b) do{if(!fp64_equals((a),(b))) { fprintf(stderr, "[ASSERTION FAILED] expect: %+.15e, given: %+.15e\n", (a),(b)); ++m_ntest_failed; }}while(0)
#define TEST_RESULTS do{if(!m_ntest_failed) { fprintf(stderr, "\n%s: all test PASSED\n", __FILE__);}}while(0)

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif/*FPTEST_H*/
