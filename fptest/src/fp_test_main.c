#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#include "fp_test_internals.h"

/* Initialize floating-point environment */
fpenv_t g_fpenv =
{
    .fp32_sqrt_accepted = 0.0,
    .fp32_exp_accepted  = 0.0,
    .fp32_log_accepted  = 0.0,
    .fp32_sin_accepted  = 0.0,
    .fp32_cos_accepted  = 0.0,
    .fp32_asin_accepted = 0.0,
    .fp32_powy_accepted = 0.0,

    .fp32_sqrt_rejected = 1.0,
    .fp32_exp_rejected  = 1.0,
    .fp32_log_rejected  = 1.0,
    .fp32_sin_rejected  = 1.0,
    .fp32_cos_rejected  = 1.0,
    .fp32_asin_rejected = 1.0,
    .fp32_powy_rejected = 1.0,

    .fp64_sqrt_accepted = 0.0,
    .fp64_exp_accepted  = 0.0,
    .fp64_log_accepted  = 0.0,
    .fp64_sin_accepted  = 0.0,
    .fp64_cos_accepted  = 0.0,
    .fp64_asin_accepted = 0.0,
    .fp64_powy_accepted = 0.0,

    .fp64_sqrt_rejected = 1.0,
    .fp64_exp_rejected  = 1.0,
    .fp64_log_rejected  = 1.0,
    .fp64_sin_rejected  = 1.0,
    .fp64_cos_rejected  = 1.0,
    .fp64_asin_rejected = 1.0,
    .fp64_powy_rejected = 1.0,
};

/* >> LOGGING */
#define PRINT(...) if (isatty(fileno(stdout))) { fprintf(stdout,__VA_ARGS__);   } else { fprintf(stderr, __VA_ARGS__);  }
#define FLUSH      if (isatty(fileno(stdout))) { fflush(stdout);                } else { fflush (stderr);               }
/* << LOGGING */

/* >> KEY VALUE VECTOR: Not in use today */
unsigned int kv_push(vector_t *this, const void *k, const void *v)
{
    if (this->len == this->size && this->size < MAX_CAPACITY)
    {
        if( this->size == 0 )
        {
            this->size++;
        }

        unsigned int new_size = (this->size * GROWTH_RATIO > MAX_CAPACITY) ? MAX_CAPACITY : this->size * GROWTH_RATIO;

        /* Reallocate both keys and vals in-place */
        void *p1 = NULL;

        if( this->elems != NULL )
        {
            p1 = realloc(this->elems, new_size * sizeof(kv_t));
        }

        else
        {
            p1 = malloc( sizeof(kv_t) );
        }

        if (p1 != NULL)
        {
            this->elems = p1;
            this->size = new_size;
        }
    }

    /* Add the key-value pair now that space has been ensured */
    if( this->len < this->size )
    {
        this->elems[this->len].k = k;
        this->elems[this->len].v = v;
        this->len++;
    }

    return this->len;
}

vector_t kv_new( unsigned int size )
{
    vector_t ret = { 0 };

    unsigned int init_sz = size < MAX_CAPACITY ? size : MAX_CAPACITY;

    if( size == 0 )
    {
        ret.elems = NULL;
    }

    else
    {
        ret.elems = calloc(init_sz, sizeof(vector_t));
    }

    if(ret.elems != NULL)
    {
        ret.len = 0;
        ret.size = init_sz;
    }

    return ( ret );
}

const char * kv_find_by_key( vector_t * vtable, void * key )
{
    for(unsigned int i = 0; i < vtable->len; i++)
        if( vtable->elems[i].k == key )
            return ( vtable->elems[i].v );

    return ("unknown");
}
/* << KEY VALUE VECTOR */

/*
 * fp_record_ulp
 *
 * ulp
 * x:   pointer to current value that caused that ulp
 * n:   fp width of pointer to x
 */

/*
 * The function keeps an histogram per exponent e of: x = m·B^q, where q is the biased exponent 0 <= q <= 2·emax - 1
 * The function is able to store the biggest arrays inside,
 * The value of n restricts the range of the exponent
 *
 */

/*
 * fp_32_histogram_compute_table_hash
 *
 * compute the hash from the exponent and sign of the number, the hash is a continuous function
 *
 * NEG MAX_FLOAT    -> 0
 * NEG MIN_FLOAT    -> 253
 * NEG ZERO         -> 254
 * POS ZERO         -> 255
 * POS MIN_FLOAT    -> 256
 * POS MAX_FLOAT    -> 509
 */

#define HASH_NEXP(emax,emin) ( ( emax ) - ( emin ) + 1 )

#define HASH_NEG_INF                (                               0                       )
#define HASH_NEG_NOR(emax,e)        (       HASH_NEXP(emax, e   ) + 0                       )
#define HASH_NEG_SUB(emax,emin)     (       HASH_NEXP(emax, emin) + 1                       )
#define HASH_NEG_ZER(emax,emin)     (       HASH_NEXP(emax, emin) + 2                       )
#define HASH_POS_ZER(emax,emin)     (       HASH_NEXP(emax, emin) + 3                       )
#define HASH_POS_SUB(emax,emin)     (       HASH_NEXP(emax, emin) + 4                       )
#define HASH_POS_NOR(emax,emin,e)   (       HASH_NEXP(emax, emin) + 4 + ( e ) + ( emax )    )
#define HASH_POS_INF(emax,emin)     ( 2  *  HASH_NEXP(emax, emin) + 5                       )

#define MAX_NHIST_F64 ( 1 + HASH_POS_INF(EMAX_F64, EMIN_F64) )
#define MAX_NHIST_F32 ( 1 + HASH_POS_INF(EMAX_F32, EMIN_F32) )
#define MAX_NHIST     MAX_NHIST_F64 /* MAX OF F32 and F64 */

cstr_t ADDCALL fp_histogram_get_erange( i16_t hash, i16_t emax, i16_t emin )
{
    static char ret[ 120 ];

    const u16_t normal_branch_nexp = emax - emin + 1;

    for(u64_t i = 0; i < sizeof(ret); i++) { ret[i] = '\0'; }

    if      (hash == HASH_NEG_INF           )   { sprintf(ret, "-infinity"); }
    else if (hash <  HASH_NEG_SUB(emax,emin))   { sprintf(ret, "[-2E^%+5d, -2E^%+5d)", emax - hash + 1, emax - hash + 1 + 1); }
    else if (hash <  HASH_NEG_ZER(emax,emin))   { sprintf(ret, "-subnormal"); }
    else if (hash <  HASH_POS_ZER(emax,emin))   { sprintf(ret, "-0"); }
    else if (hash <  HASH_POS_SUB(emax,emin))   { sprintf(ret, "+0"); }
    else if (hash == HASH_POS_SUB(emax,emin))   { sprintf(ret, "+subnormal"); }
    else if (hash <  HASH_POS_INF(emax,emin))   { sprintf(ret, "[+2E^%+5d, +2E^%+5d)", hash - 3*emax - 4, hash - 3*emax - 4 + 1); }
    else if (hash == HASH_POS_INF(emax,emin))   { sprintf(ret, "+infinity"); }
    else                                        { sprintf(ret, "+unkown"); }

    return ( ret );
}

u16_t ADDCALL fp_histogram_compute_table_hash( fptriplet_t triplet, fpset_t type, i16_t emax, i16_t emin )
{
    u16_t ret = 0;

    const u16_t normal_branch_nexp = emax - emin + 1;

    if( type == FP_TYPE_INFINITE )
    {
        ret = ( triplet.s == -1 ) ? HASH_NEG_INF : HASH_POS_INF(emax, emin);
    }

    else if( type == FP_TYPE_NORMAL )
    {
        if( triplet.s == - 1)
        {
            ret = (u16_t) HASH_NEG_NOR(emax, triplet.e);
        }
        else
        {
            ret = (u16_t) HASH_POS_NOR(emax, emin, triplet.e);
        }
    }

    else if( type == FP_TYPE_SUBNORMAL )
    {
        ret = ( triplet.s == -1 ) ? ( HASH_NEXP(emax, emin) + 1 ) : ( HASH_NEXP(emax, emin) + 4 );
    }

    else if( type == FP_TYPE_ZERO )
    {
        ret = ( triplet.s == -1 ) ? ( HASH_NEXP(emax, emin) + 2 ) : ( HASH_NEXP(emax, emin) + 3 );
    }

    else
    {
        fprintf(stderr, "[ERROR] %s unknown type: %d\n", "fp_historgram_compute_table_hash", type);
    }

    return ( ret );
}

u64_vec2_t * ADDCALL fp_histogram_set_ulp( f64_t ulp, const void * x, fp_width_t precision, f64_t reject )
{
    static u64_vec2_t m_hist_nulps_by_exp[ MAX_NHIST ] = { 0 };

    if( x != NULLPTR )
    {
        fptriplet_t triplet = { 0 };
        fpset_t     type    =   0;
        i16_t       emax    =   0;
        i16_t       emin    =   0;

        switch (precision)
        {
            case (FP32):
            {
                triplet = fp32_get_triplet( *( ( f32_t * ) x ) );
                type    = fp32_get_subset ( *( ( f32_t * ) x ) );
                emax    = EMAX_F32;
                emin    = EMIN_F32;
                break;
            }

            case (FP64):
            {
                triplet = fp64_get_triplet( *( ( f64_t * ) x ) );
                type    = fp64_get_subset ( *( ( f64_t * ) x ) );
                emax    = EMAX_F64;
                emin    = EMIN_F64;
                break;
            }

            default:
            {
                fprintf(stderr, "[ERROR] fp_record_ulp has received an invalid fp width, value: %d\n", precision);
                break;
            }
        }

        const u16_t hash_x = fp_histogram_compute_table_hash( triplet, type, emax, emin );

        if( (precision == FP32) && (hash_x >= MAX_NHIST_F32) )
        {
            fprintf(stderr, "[ERROR] histogram hash out of range in FP32, value: %u\n", hash_x);
        }

        else if( (precision == FP64) && (hash_x >= MAX_NHIST_F64) )
        {
            fprintf(stderr, "[ERROR] histogram hash out of range in FP64, value: %u\n", hash_x);
        }

        else
        {
            if( ulp >= reject )
            {
                m_hist_nulps_by_exp[ hash_x ].x0++;
            }
            else
            {
                m_hist_nulps_by_exp[ hash_x ].x1++;
            }
        }
    }

    return ( m_hist_nulps_by_exp );
}

i32_t fp_printf_hist_of_fails_by_exp( i32_t max_nhist_fp, i32_t emax, i32_t emin, f64_t reject )
{
    i32_t stat;

    u64_vec2_t * htab = fp_histogram_set_ulp(0.0, NULLPTR, 0, 0.0);

    bool_t test_succeed = TRUE;
    for( i32_t i = 0; i < max_nhist_fp; i++ )
    {
        if( htab[i].x0 != 0 )
        {
            test_succeed = FALSE;
            break;
        }
    }

    if( test_succeed )
    {
        printf("\n Test succeed, no points rejected with ulp > %1.f\n"LINE_SEPARATOR"\n", reject<=0.0?0.0:reject-1.0);
        stat = 0;
    }
    else
    {
        printf("\n Test failed: listing the histogram of rejected points\n"LINE_SEPARATOR"\n");
        stat = 1;
    }

    for( i32_t i = 0; ( i < max_nhist_fp ) && stat != 0; i++ )
    {
        if( htab[i].x0 != 0 )
        {
            printf(" %8llu/%8llu point in %22s with ULP > %.1f\n", htab[i].x0, (htab[i].x0+htab[i].x1), fp_histogram_get_erange( i, emax, emin ), reject );
        }

        htab[i].x0 = 0;
        htab[i].x1 = 0;
    }

    return ( stat );
}

/* >> ULPs HISTOGRAM */
#define ULPS_NHIST (20)
#define ULPS_HIST_INIT \
static u64_t m_hist[ ULPS_NHIST ]; \
static f64_t m_hist_xl_negx[ ULPS_NHIST ]; \
static f64_t m_hist_xr_negx[ ULPS_NHIST ]; \
static f64_t m_hist_xl_posx[ ULPS_NHIST ]; \
static f64_t m_hist_xr_posx[ ULPS_NHIST ]; \
for(u32_t i = 0; i < SIZEOF(m_hist); i++){ m_hist[i]=0; m_hist_xl_negx[i]=0.0/0.0; m_hist_xr_negx[i]=0.0/0.0; m_hist_xl_posx[i]=0.0/0.0; m_hist_xr_posx[i]=0.0/0.0; }

#define update_hist_ranges(id,x) \
if( (x) < 0.0 ) {m_hist_xl_negx[ id ] = isnan( m_hist_xl_negx[ id ]) ? ( x ) : m_hist_xl_negx[ id ];} \
if( (x) < 0.0 ) {m_hist_xr_negx[ id ] = ( x );} \
if( (x) > 0.0 ) {m_hist_xl_posx[ id ] = isnan( m_hist_xl_posx[ id ]) ? ( x ) : m_hist_xl_posx[ id ];} \
if( (x) > 0.0 ) {m_hist_xr_posx[ id ] = ( x );}

#define ULPS_HIST_RECORD(ulp, x) \
do { \
    u32_t idx = (ulp < (f64_t) (ULPS_NHIST - 2)) ? (u32_t) ulp : (ULPS_NHIST); /* Clamp to [0, 19] */ \
    m_hist[idx]++; \
    update_hist_ranges(idx, (x)); \
} while(0)

#define get_negpos_ranges(id) m_hist_xl_negx[ id ], m_hist_xr_negx[ id ], m_hist_xl_posx[ id ], m_hist_xr_posx[ id ]

#define ULPS_HIST_REPORT \
do { \
    printf("\n Histogram\n" LINE_SEPARATOR "\n"); \
    for (u32_t i = 0; i < ULPS_NHIST; ++i) { \
        if (m_hist[i]) { \
            printf("|ulp| <= %4.1f: %10llu in [ %23.15e, %23.15e ] U [ %23.15e, %23.15e ]\n", \
                   (i == ULPS_NHIST - 1) ? INFINITY : (double) i, \
                   m_hist[i], get_negpos_ranges(i)); \
        } \
    } \
} while (0)
/* << ULPs HISTOGRAM */

/* >> RANGE FINDER */

#define MAX_NREL 10

static f64_t 	m_relf_range_left  [ MAX_NREL + 1 ];
static f64_t 	m_relf_range_right [ MAX_NREL + 1 ];
static u32_t	m_relf_range_len;
static fpset_t	m_relf_y_lhs_prev_type;

/*
* Update Range:
* 1. close current range at (m_len) with m_right
* 2. open next range: m_len++
* 3. update type and left value
*/

#define M_CLEAR_PREV_RANGES \
do { \
	for(u32_t i = 0; i < MAX_NREL; i++) \
	{ \
		m_relf_range_left[i]=0; \
		m_relf_range_right[i]=0; \
	} \
	m_relf_range_len = 0; \
} while(0)

#define INIT_RANGE(xl, y_lhs_type) \
do { \
	M_CLEAR_PREV_RANGES; \
	m_relf_y_lhs_prev_type = ( y_lhs_type ); \
	m_relf_range_left[ 0 ] = (xl); /* Open left of first range */ \
} while(0)

#define UPDATE_RANGE(xl, xr, y_lhs, y_lhs_type) \
do{ \
	if( m_relf_y_lhs_prev_type != y_lhs_type ) \
	{ \
		m_relf_range_right[ m_relf_range_len ] = (xl);			/* Close last range */ \
		m_relf_range_len+=(m_relf_range_len<(MAX_NREL-1)?1:0);	/* Count last range closed */ \
		m_relf_range_left[ m_relf_range_len ] = (xr);			/* Open a new range */ \
		m_relf_y_lhs_prev_type = y_lhs_type; \
	} \
} while(0)

#define CLOSE_RANGE(xr) \
do { \
	m_relf_range_right[ m_relf_range_len ] = (xr);			/* Close right of last range */ \
	m_relf_range_len+=m_relf_range_len<(MAX_NREL-1)?1:0;	/* Count last range closed */ \
} while(0)

void fp32_print_range_types( fp_f2f_t lhs )
{
    printf("\n Range Len:\n"LINE_SEPARATOR"\n");
    for(u32_t i = 0; i < m_relf_range_len; i++)
    {
        const f32_t yl = lhs( m_relf_range_left[i] );
        const f32_t yr = lhs( m_relf_range_right[i] );

        fpset_t yl_type = fp32_get_subset( yl );
        cstr_t  yl_name = fp32_get_subset_name( yl );

        if( ( yl_type == FP_TYPE_NAN ) || ( yl_type == FP_TYPE_INFINITE ) || ( yl_type == FP_TYPE_ZERO ) )
        {
            printf("[ %+12.2e, %+12.2e ] -> %s\n", m_relf_range_left[i], m_relf_range_right[i], yl_name );
        }
        else
        {
            printf("[ %+12.2e, %+12.2e ] -> %s [ %+.2e, %+.2e ]\n", m_relf_range_left[i], m_relf_range_right[i], yl_name, yl, yr );
        }
    }
}

void fp64_print_range_types( fp_d2d_t lhs )
{
    printf("\n Range Len:\n"LINE_SEPARATOR"\n");
    for(u32_t i = 0; i < m_relf_range_len; i++)
    {
        const f64_t yl = lhs( m_relf_range_left[i] );
        const f64_t yr = lhs( m_relf_range_right[i] );

        fpset_t yl_type = fp64_get_subset( yl );
        cstr_t  yl_name = fp64_get_subset_name( yl );

        if( ( yl_type == FP_TYPE_NAN ) || ( yl_type == FP_TYPE_INFINITE ) || ( yl_type == FP_TYPE_ZERO ) )
        {
            printf("[ %+12.2e, %+12.2e ] -> %s\n", m_relf_range_left[i], m_relf_range_right[i], yl_name );
        }
        else
        {
            printf("[ %+12.2e, %+12.2e ] -> %s [ %+.2e, %+.2e ]\n", m_relf_range_left[i], m_relf_range_right[i], yl_name, yl, yr );
        }
    }
}
/* << RANGE FINDER */

/* >> PROGRESS BAR */
static void fp_console_remove_line( fpenv_t fpenv )
{
    if( fpenv.log_completation_remove_history )
    {
        PRINT("\r");
        FLUSH;
    }

    else
    {
        PRINT("\n");
    }
}

static u32_t fp32_progress_bar_status( f32_t x )
{
    return ( (u32_t) ( 100.0 * ( x < 0.0 ? (39. - (x == 0 ? 0.0 : log10((f64_t) fabs( x )))): (x == 0.0 ? 83. : 83. + 44. + log10((f64_t) fabs( x )))) / 165.0 ) );
}

static void fp32_progress_bar( cstr_t desc, fpenv_t fpenv, f32_t x_curr, f32_t x_min, f32_t x_max, fp_progress_state_t * pstate )
{
    if( fpenv.log_completation )
    {
		fptriplet_t xtrip = fp32_get_triplet( x_curr );

		pstate->count_points_in_the_range++;

        if( xtrip.e != (pstate->xexp) )
        {
            (pstate->xexp) = xtrip.e;

            if( !fp32_equals(x_curr, x_min) && !fp32_equals(x_curr, x_max) )
            {
                fp_console_remove_line( g_fpenv );

                const u32_t progress = fp32_progress_bar_status ( x_curr );

                PRINT(" %-12s completed: %3u%% range-bound: %+dE2%+04d in-range-vals: %8llu",	desc,
                                                                                                progress,
					                       														xtrip.s,
                                                                                   				pstate->xexp,
                                                                                				pstate->count_points_in_the_range );

                pstate->count_points_in_the_range = 0;
            }
        }

        else
        {
			/* NOP: already counted */
        }
    }
}

static u32_t fp64_progress_bar_status( f64_t x )
{
    return ( (u32_t) ( 100.0 * ( x < 0.0 ? (316. - (x == 0 ? 0.0 : log10(fabs(x)))) : (x == 0.0 ? 668. : 668. + 354. + log10(fabs(x)))) / 1328.0 ) );
}

static void fp64_progress_bar( cstr_t desc, fpenv_t fpenv, f64_t x_curr, f64_t x_min, f64_t x_max, fp_progress_state_t * pstate )
{
    if( fpenv.log_completation )
    {
		fptriplet_t xtrip = fp64_get_triplet( x_curr );

		pstate->count_points_in_the_range++;

        if( xtrip.e != (pstate->xexp) )
        {
            (pstate->xexp) = xtrip.e;

            if( !fp64_equals(x_curr, x_min) && !fp64_equals(x_curr, x_max) )
            {
                fp_console_remove_line( g_fpenv );

                const u32_t progress = fp64_progress_bar_status ( x_curr );

                PRINT(" %-12s completed: %3u%% range-bound: %+dE2%+05d in-range-vals: %8llu",	desc,
                                                                                                progress,
                                                                                                xtrip.s,
                                                                                                pstate->xexp,
                                                                                                pstate->count_points_in_the_range );

                pstate->count_points_in_the_range = 0;
            }
        }

        else
        {
			/* NOP: already counted */
        }
    }
}

static void fp32_print_total_analyzed_points( u64_t count_hi, u64_t total_fp32, f32_t x_min, f32_t x_curr )
{
    f64_t count_exp  = log10( count_hi );
    i64_t count_iexp = (i64_t) count_exp;
    f64_t count_frac = ( count_exp > 1.0 ) ? ( count_exp - count_iexp ) : count_exp;

    printf("\n Analyzed points:\n"LINE_SEPARATOR"\n%.1fe+%lld (%.1f%%) in [ %e, %e ]\n", pow(10.0, count_frac), count_iexp, 100. * ( (f64_t) count_hi / ((f64_t) total_fp32) ), x_min, x_curr );
}

static void fp64_print_total_analyzed_points( u64_t count_hi, u64_t total_fp64, f64_t x_min, f64_t x_curr )
{
    f64_t count_exp  = log10( count_hi );
    i64_t count_iexp = (i64_t) count_exp;
    f64_t count_frac = ( count_exp > 1.0 ) ? ( count_exp - count_iexp ) : count_exp;

    printf("\n Analyzed points:\n"LINE_SEPARATOR"\n%.1fe+%lld (%.1f%%) in [ %e, %e ]\n", pow(10.0, count_frac), count_iexp, 100. * ( (f64_t) count_hi / ((f64_t) total_fp64) ), x_min, x_curr );
}
/* << PROGRESS BAR */

/*
 * f32_ulps
 *
 * compute the number of ULPs between approx and exact.
 *
 * approx and exact are two machine numbers hence this function can't
 * give a resolution less than 1ulp. This is not a problem for the
 * purpose of this function as it is not designed to evaluate
 * rounding errors required by IEEE-754.
 */

f64_t ADDCALL fp32_ulps( f32_t approx, f32_t exact )
{
    f64_t ret;

    fpset_t t_apprx = fp32_get_subset( approx );
    fpset_t t_exact = fp32_get_subset( exact  );

    if( t_apprx != t_exact )
    {
        if( ( t_apprx == FP_TYPE_NORMAL || t_apprx == FP_TYPE_SUBNORMAL ) && ( t_exact == FP_TYPE_NORMAL || t_exact == FP_TYPE_SUBNORMAL ) )
        {
            ret = fp32_ulps_impl( approx, ( (f80_t) approx - (f80_t) exact ) );
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else if( t_apprx == FP_TYPE_NAN )
    {
        ret = 0.0; /* Controversial: if both are NaN's ULP is zero TODO: research it */
    }

    else if( t_apprx == FP_TYPE_INFINITE )
    {
        if( (approx > 0.0 && exact > 0.0) || (approx < 0.0 && exact < 0.0) )
        {
            ret = 0.0;
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else if( t_apprx == FP_TYPE_ZERO )
    {
        fw_t u_apprx = { .f = approx };
        fw_t u_exact = { .f = exact  };

        if( u_apprx.u == u_exact.u ) /* is same sign */
        {
            ret = 0.0;
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else
    {
        ret = fp32_ulps_impl( approx, ( (f80_t) approx - (f80_t) exact ) );
    }

    return ( ret );
}

f64_t ADDCALL fp64_ulps( f64_t approx, f64_t exact )
{
    f64_t ret;

    fpset_t t_apprx = fp64_get_subset( approx );
    fpset_t t_exact = fp64_get_subset( exact  );

    if( t_apprx != t_exact )
    {
        if( ( t_apprx == FP_TYPE_NORMAL || t_apprx == FP_TYPE_SUBNORMAL ) && ( t_exact == FP_TYPE_NORMAL || t_exact == FP_TYPE_SUBNORMAL ) )
        {
            ret = fp64_ulps_impl( approx, ( (f80_t) approx - (f80_t) exact ) );
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else if( t_apprx == FP_TYPE_NAN )
    {
        ret = 0.0; /* Controversial: if both are NaN's ULP is zero TODO: research it, alternative return ( m_qnanf64.f ) */
    }

    else if( t_apprx == FP_TYPE_INFINITE )
    {
        if( (approx > 0.0 && exact > 0.0) || (approx < 0.0 && exact < 0.0) )
        {
            ret = 0.0;
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else if( t_apprx == FP_TYPE_ZERO )
    {
        dw_t u_apprx = { .f = approx };
        dw_t u_exact = { .f = exact  };

        if( u_apprx.u == u_exact.u ) /* is same sign */
        {
            ret = 0.0;
        }

        else
        {
            ret = m_inff64.f;
        }
    }

    else
    {
        ret = fp64_ulps_impl( approx, ( (f80_t) approx - (f80_t) exact ) );
    }

    return ( ret );
}

static f64_t fp64_ulps_impl( f64_t approx, f80_t diff )
{
    dw_t bits = {.f = approx };

    const i16_t e   = ( ( bits.w[ DW0 ] & DW0_EMASK ) >> DOFFS_F64 ) - EMAX_F64;
    const f80_t ulp = ldexpl( 1. , e - ( NDIG_F64 - 1 ) );

    return ( (f64_t) ( diff / ulp ) );
}

static f64_t fp32_ulps_impl( f32_t approx, f80_t diff )
{
    fw_t bits = {.f = approx };

    const i16_t e   = ( ( bits.w[ FW0 ] & FW0_EMASK ) >> DOFFS_F32 ) - EMAX_F32;
    const f80_t ulp = ldexpl( 1. , e - ( NDIG_F32 - 1 ) );

    return ( (f64_t) ( diff / ulp ) );
}

f32_t ADDCALL fp32_get_named_fp_in_real_line( named_fp_t point )
{
    fw_t ret;

    switch(point)
    {
        case (NAMED_FP_INF )        : { ret.u = m_inff32 .u; break; }
        case (NAMED_FP_MAXNORM )    : { ret.u = m_maxf32 .u; break; }
        case (NAMED_FP_ZERO)        : { ret.u = m_zerof32.u; break; }
        case (NAMED_FP_MINNORM)     : { ret.u = m_minf32 .u; break; }
        case (NAMED_FP_MINSUBN)     : { ret.u = m_subnf32.u; break; }
        default                     : { ret.u = m_qnanf32.u; break; }
    }

    return ( ret.f );
}

f64_t ADDCALL fp64_get_named_fp_in_real_line( named_fp_t point )
{
    dw_t ret;

    switch(point)
    {
        case (NAMED_FP_INF )        : { ret.u = m_inff64 .u; break; }
        case (NAMED_FP_MAXNORM )    : { ret.u = m_maxf64 .u; break; }
        case (NAMED_FP_ZERO)        : { ret.u = m_zerof64.u; break; }
        case (NAMED_FP_MINNORM)     : { ret.u = m_minf64 .u; break; }
        case (NAMED_FP_MINSUBN)     : { ret.u = m_subnf64.u; break; }
        default                     : { ret.u = m_qnanf64.u; break; }
    }

    return ( ret.f );
}

bool_t ADDCALL fp32_equals( f32_t a, f32_t b )
{
    const fw_t ua = { .f = a };
    const fw_t ub = { .f = b };

    return ( ua.u == ub.u );
}

bool_t ADDCALL fp64_equals( f64_t a, f64_t b )
{
    const dw_t ua = { .f = a };
    const dw_t ub = { .f = b };

    return ( ua.u == ub.u );
}

bool_t ADDCALL fp32_equals_sign( f32_t a, f32_t b )
{
    const fw_t ua = { .f = a };
    const fw_t ub = { .f = b };

    return ( ( ua.w[ FW0 ] & FW0_SMASK ) == ( ub.w[ FW0 ] & FW0_SMASK ) );
}

bool_t ADDCALL fp64_equals_sign( f64_t a, f64_t b )
{
    const dw_t ua = { .f = a };
    const dw_t ub = { .f = b };

    return ( ( ua.w[ FW0 ] & FW0_SMASK ) == ( ub.w[ FW0 ] & FW0_SMASK ) );
}

i16_t ADDCALL fp32_get_exp( f32_t x )
{
    return ( F32_GET_EXP( x ) );
}

i16_t ADDCALL fp64_get_exp( f64_t x )
{
    return ( F64_GET_EXP( x ) );
}

cstr_t ADDCALL fp32_get_subset_name( f32_t x )
{
    cstr_t ret;

    switch ( fp32_get_subset( x ) )
    {
        case (FP_TYPE_NAN       )   : { ret = "FP_TYPE_NAN      "; break; }
        case (FP_TYPE_ZERO      )   : { ret = "FP_TYPE_ZERO     "; break; }
        case (FP_TYPE_NORMAL    )   : { ret = "FP_TYPE_NORMAL   "; break; }
        case (FP_TYPE_SUBNORMAL )   : { ret = "FP_TYPE_SUBNORMAL"; break; }
        case (FP_TYPE_INFINITE  )   : { ret = "FP_TYPE_INFINITE "; break; }
        default                     : { ret = "UNKNOWN          "; break; }
    }

    return ( ret );
}

cstr_t ADDCALL fp64_get_subset_name( f64_t x )
{
    cstr_t ret;

    switch ( fp64_get_subset( x ) )
    {
        case (FP_TYPE_NAN       )   : { ret = "FP_TYPE_NAN      "; break; }
        case (FP_TYPE_ZERO      )   : { ret = "FP_TYPE_ZERO     "; break; }
        case (FP_TYPE_NORMAL    )   : { ret = "FP_TYPE_NORMAL   "; break; }
        case (FP_TYPE_SUBNORMAL )   : { ret = "FP_TYPE_SUBNORMAL"; break; }
        case (FP_TYPE_INFINITE  )   : { ret = "FP_TYPE_INFINITE "; break; }
        default                     : { ret = "UNKNOWN          "; break; }
    }

    return ( ret );
}

/*
 * get_subset
 *
 * recovers the subset belonging to 'x' according to IEEE 754-2008 standard
 * the algorithm is based on section 3.4 Binary interchange format encodings of cited standard
 *
 */

fpset_t ADDCALL fp32_get_subset( f32_t x )
{
    fpset_t ret;

    const fw_t y = { .f = x };

    const u16_t e  = y.w[ FW0 ] & FW0_EMASK;
    const u32_t t  = y.u & 0x7FFFFFu;

    if ( e == FW0_EMASK )
    {
        /* cond. a) E = 2**w - 1 and T != 0 */
        if( t != 0 )
        {
            ret = FP_TYPE_NAN;
        }

        /* cond. b) E = 2**w - 1 and T = 0 */
        else
        {
            ret = FP_TYPE_INFINITE;
        }
    }

    /* cond. c) 0 < E < 2**w - 1 */
    else if ( e != 0 )
    {
        ret = FP_TYPE_NORMAL;
    }

    /* cond. d) E = 0 and T != 0 */
    else if ( t != 0 )
    {
        ret = FP_TYPE_SUBNORMAL;
    }

    /* cond. e) E = 0 and T = 0 */
    else
    {
        ret = FP_TYPE_ZERO;
    }

    return ( ret );
}

fpset_t ADDCALL fp64_get_subset( f64_t x )
{
    fpset_t ret;

    const dw_t y = { .f = x };

    const u16_t e  = y.w[ DW0 ] & DW0_EMASK;
    const u64_t t  = y.u & 0x000FFFFFFFFFFFFFuLL;

    if ( e == DW0_EMASK )
    {
        /* cond. a) E = 2**w - 1 and T != 0 */
        if( t != 0 )
        {
            ret = FP_TYPE_NAN;
        }

        /* cond. b) E = 2**w - 1 and T = 0 */
        else
        {
            ret = FP_TYPE_INFINITE;
        }
    }

    /* cond. c) 0 < E < 2**w - 1 */
    else if ( e != 0 )
    {
        ret = FP_TYPE_NORMAL;
    }

    /* cond. d) E = 0 and T != 0 */
    else if ( t != 0 )
    {
        ret = FP_TYPE_SUBNORMAL;
    }

    /* cond. e) E = 0 and T = 0 */
    else
    {
        ret = FP_TYPE_ZERO;
    }

    return ( ret );
}

/*
 * fp64_step_real_line
 *
 * after n calls:
 *
 * x_0 in [ -MAX, +MAX ]
 *
 * x_n = x_n-1 · s**n,
 *
 * with x_n in ( -MAX, -NORM ] U [ +NORM, +MAX ] U { +INF }
 *
 * frac comes from: ratio = 1 + frac, example: ratio = 1.0000001, then frac = 0.0000001
 *
 */

f32_t ADDCALL fp32_geom_step_real_line( f32_t x, f32_t frac )
{
    f32_t ret;

    const f32_t right_zero = +fp32_get_named_fp_in_real_line( NAMED_FP_MINNORM );
    const f32_t left_zero  = -right_zero;

    if( x < left_zero ) /* [-MAX, -NORM) -> (-MAX, -NORM] U [-NORM, -SUBN] U { -ZERO } */
    {
        ret = x / ( 1. + frac );
    }

    /* else if( x < left_sub )  { TBD } */
    /* else if( x == 0.0 )      { TBD } */
    /* else if( x < right_sub ) { TBD } */

    else if( x < right_zero ) /* x in [-NORM, +NORM] -> +NORM */
    {
        ret = right_zero;
    }

    else
    {
        ret = x * ( 1. + frac );
    }

    return ( ret );
}

f64_t ADDCALL fp64_geom_step_real_line( f64_t x, f64_t frac )
{
    f64_t ret;

    const f64_t right_zero = +fp64_get_named_fp_in_real_line( NAMED_FP_MINNORM );
    const f64_t left_zero  = -right_zero;

    if( x < left_zero ) /* [-MAX, -NORM) -> (-MAX, -NORM] U [-NORM, -SUBN] U { -ZERO } */
    {
        ret = x / ( 1. + frac ) ;
    }

    /* else if( x < left_sub )  { TBD } */
    /* else if( x == 0.0 )      { TBD } */
    /* else if( x < right_sub ) { TBD } */

    else if( x < right_zero ) /* x in [-NORM, +NORM] -> +NORM */
    {
        ret = right_zero;
    }

    else
    {
        ret = x * ( 1. + frac );
    }

    return ( ret );
}

/**
 * rel_error_for_reals
 *
 * precondition: x, y shall lie in the Real Line
 *
 * ret in [0, 1]
 */

f64_t ADDCALL f64_rel_error_for_reals( f64_t x, f64_t y )
{
    f64_t ret;

    fpset_t tx = fp64_get_subset( x );
    fpset_t ty = fp64_get_subset( y );

    /* tx or ty inf */
    if( ( tx == FP_TYPE_INFINITE ) || ( ty == FP_TYPE_INFINITE ) )
    {
        if ( x == y ) /* same sign inf */
        {
            ret = 52; /* avoid performing: inf - inf wich evaluates to nan */
        }

        else
        {
            ret = 0;
        }
    }

    else if( ( tx == FP_TYPE_ZERO ) || ( ty == FP_TYPE_ZERO ) )
    {
        const dw_t wx = { .f = x };
        const dw_t wy = { .f = y };

        if ( x == y )
        {
            if( ( wx.w[ FW0 ] & FW0_SMASK ) == ( wy.w[ FW0 ] & FW0_SMASK ) )
            {
                ret = 52;
            }

            else
            {
                ret = 52;
            }
        }

        else
        {
            ret = 0;
        }
    }

    else if( ( tx == FP_TYPE_SUBNORMAL ) || ( ty == FP_TYPE_SUBNORMAL ) )
    {
        ret = 0; /* TODO: renormalize, count the difference between exponent values */
    }

    else
    {
        if( x == y )
        {
            ret = 52;
        }

        else
        {
            ret = fabs( y / x ) - 1.0;
        }
    }

    return ( ret );
}

cstr_t ADDCALL fp32_sprint_digits_radix2( char buff [ 32 + 1 + 2 ], char_t sepparator, f32_t x )
{
    static const u16_t index[ 2 ] = { FW0, FW1, };

    fw_t f = { .f = x };

    u16_t w;
    u8_t i = 0;
    u8_t j = 0;
    u8_t k = 0;

    while( k < 2 )
    {
        w = f.w[ index[ k ] ];

        while( j < 16 )
        {
            buff[ i++ ] = w & 0x8000 ? '1' : '0';

            if( ( i == 1 ) || ( i == 10 ) )
            {
                buff[ i++ ] = sepparator;
            }

            w = w << 1;
            j++;
        }

        j = 0;
        k++;
    }

    buff[ i ] = '\0';

    return ( buff );
}

cstr_t ADDCALL fp64_sprint_digits_radix2( char buff [ 64 + 1 + 2 ], char_t sepparator, f64_t x )
{
    static const u16_t index[ 4 ] = { DW0, DW1, DW2, DW3, };

    dw_t f = { .f = x };

    u16_t w;
    u8_t i = 0;
    u8_t j = 0;
    u8_t k = 0;

    while( k < 2 )
    {
        w = f.w[ index[ k ] ];

        while( j < 16 )
        {
            buff[ i++ ] = w & 0x8000 ? '1' : '0';

            if( ( i == 1 ) || ( i == 13 ) )
            {
                buff[ i++ ] = sepparator;
            }

            w = w << 1;
            j++;
        }

        j = 0;
        k++;
    }

    buff[ i ] = '\0';

    return ( buff );
}

/**
 * fp32_mount_bitfields
 *
 * s: sign bit
 * e: biased exponent bits
 * m: fractional bits
 *
 * return: 32bit floating-point representation according to IEEE-754
 */

f32_t ADDCALL fp32_mount_bitfields( u32_t s, u32_t e, u32_t m )
{
    return ( (fw_t) { .u = ( s << 31 ) | ( e << 23 ) | ( m ) } ).f;
}

/**
 * fp64_mount_bitfields
 *
 * s: sign bit
 * e: biased exponent bits
 * m: fractional bits
 *
 * return: 64bit floating-point representation according to IEEE-754
 */

f64_t ADDCALL fp64_mount_bitfields( u64_t s, u64_t e, u64_t m )
{
    return ( (dw_t) { .u = ( s << 63 ) | ( e << 52 ) | ( m ) }).f;
}

f32_t ADDCALL fp32_next_float( f32_t x )
{
    f32_t ret;

    fpset_t type = fp32_get_subset( x );

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret = x;
            break;
        }

        case(FP_TYPE_INFINITE):
        {
            if( x < 0.0 )
            {
                ret = -m_maxf32.f;
            }

            else
            {
                ret = m_inff32.f;
            }

            break;
        }

        case(FP_TYPE_ZERO):
        case(FP_TYPE_NORMAL):
        case(FP_TYPE_SUBNORMAL):
        default:
        {
            fw_t this = { .f = x };

            u32_t s = ( this.w[ FW0 ] & 0x8000 ) >> 15;

            i32_t e = ( this.w[ FW0 ] & 0x7F80 ) >> 7;

            u32_t m = ( ( (u32_t) (this.w[ FW0 ] & 0x007F) ) << 16 ) | ( (u32_t) (  this.w[ FW1 ] & 0xFFFF ) );

            if( s == 1 && e == 0 && m == 0 )
            {
                s = 0;
            }

            else if( s == 1 ) /* when x is negative, to move x->x + dx requires to substract values from e, m */
            {
                if( m == 0x000000 ) { e -= 1; m = 0x7FFFFF; } else { m -= 1; }
            }

            else
            {
                if( m >= 0x7FFFFF ) { e += 1; m = 0x000000; } else { m += 1; }
            }

            ret = fp32_mount_bitfields( s, e, m );
        }
    }

    return ( ret );
}

f64_t ADDCALL fp64_next_float( f64_t x )
{
    f64_t ret;

    fpset_t type = fp64_get_subset( x );

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret = x;
            break;
        }

        case(FP_TYPE_INFINITE):
        {
            if( x < 0.0 )
            {
                ret = -m_maxf64.f;
            }

            else
            {
                ret = m_inff64.f;
            }

            break;
        }

        case(FP_TYPE_ZERO):
        case(FP_TYPE_NORMAL):
        case(FP_TYPE_SUBNORMAL):
        default:
        {
            dw_t this = { .f = x };

            u64_t s = ( this.w[ DW0 ] & 0x8000 ) >> 15;
            u64_t e = ( this.w[ DW0 ] & 0x7FF0  ) >> 4;
            u64_t m =   this.u        & 0x000FFFFFFFFFFFFFllu;

            if( s == 1 && e == 0 && m == 0 )
            {
                s = 0;
            }

            else if( s == 1 ) /* when x is negative, to move x->x + dx requires to substract values from e, m */
            {
                if( m == 0x000000 ) { e -= 1; m = 0x000FFFFFFFFFFFFF; } else { m -= 1; }
            }

            else
            {
                if( m >= 0x000FFFFFFFFFFFFF ) { e += 1; m = 0; } else { m += 1; }
            }

            ret = fp64_mount_bitfields( s, e, m );
        }
    }

    return ( ret );
}

f32_t ADDCALL fp32_prev_float( f32_t x )
{
    f32_t ret;

    fpset_t type = fp32_get_subset( x );

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret = x;
            break;
        }

        case(FP_TYPE_INFINITE):
        {
            if( x < 0.0 )
            {
                ret = -m_inff32.f;
            }

            else
            {
                ret = m_maxf32.f;
            }

            break;
        }

        case(FP_TYPE_ZERO):
        case(FP_TYPE_NORMAL):
        case(FP_TYPE_SUBNORMAL):
        default:
        {
            fw_t this = { .f = x };

            u32_t s = ( this.w[ FW0 ] & 0x8000 ) >> 15;

            i32_t e = ( this.w[ FW0 ] & 0x7F80 ) >> 7;

            u32_t m = ( ( (u32_t) (this.w[ FW0 ] & 0x007F) ) << 16 ) | ( (u32_t) (  this.w[ FW1 ] & 0xFFFF ) );

            if( s == 0 && e == 0 && m == 0 )
            {
                s = 1;
            }

            else if( s == 1 ) /* when x is negative, to move x->x + dx requires to substract values from e, m */
            {
                if( m == 0x7FFFFF ) { e += 1; m = 0; } else { m += 1; }
            }

            else
            {
                if( m == 0 ) { e -= 1; m = 0x7FFFFF; } else { m -= 1; }
            }

            ret = fp32_mount_bitfields( s, e, m );
        }
    }

    return ( ret );
}

f64_t ADDCALL fp64_prev_float( f64_t x )
{
    f64_t ret;

    fpset_t type = fp64_get_subset( x );

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret = x;
            break;
        }

        case(FP_TYPE_INFINITE):
        {
            if( x < 0.0 )
            {
                ret = -m_inff64.f;
            }

            else
            {
                ret = m_maxf64.f;
            }

            break;
        }

        case(FP_TYPE_ZERO):
        case(FP_TYPE_NORMAL):
        case(FP_TYPE_SUBNORMAL):
        default:
        {
            dw_t this = { .f = x };

            u64_t s = ( this.w[ DW0 ] & 0x8000 ) >> 15;
            u64_t e = ( this.w[ DW0 ] & 0x7FF0  ) >> 4;
            u64_t m =   this.u        & 0x000FFFFFFFFFFFFFllu;

            if( s == 0 && e == 0 && m == 0 )
            {
                s = 1;
            }

            else if( s == 1 ) /* when x is negative, to move x->x + dx requires to add values from e, m */
            {
                if( m == 0x000FFFFFFFFFFFFF ) { e += 1; m = 0; } else { m += 1; }
            }

            else
            {
                if( m == 0 ) { e -= 1; m = 0x000FFFFFFFFFFFFF; } else { m -= 1; }
            }

            ret = fp64_mount_bitfields( s, e, m );
        }
    }

    return ( ret );
}

f32_t ADDCALL fp32_jump_nulp( f32_t x, i32_t n )
{
    f32_t ret = x;

    if( n > 0 )
    {
        while(n--)
        {
            ret = fp32_next_float(ret);
        }
    }

    else if( n < 0 )
    {
        while(n++)
        {
            ret = fp32_prev_float(ret);
        }
    }

    else
    {
        /* NOP: ret already set to x */
    }

    return ( ret );
}

f64_t ADDCALL fp64_jump_nulp( f64_t x, i32_t n )
{
    f64_t ret = x;

    if( n > 0 )
    {
        while(n--)
        {
            ret = fp64_next_float(ret);
        }
    }

    else if( n < 0 )
    {
        while(n++)
        {
            ret = fp64_prev_float(ret);
        }
    }

    else
    {
        /* NOP: ret already set to x */
    }

    return ( ret );
}

/*
 * fp32_get_triplet
 *
 * The most interesting difference between IEEE-754 enconding is at how NORMAL and SUBNORMAL numbers are identified.
 * Both range of numbers share one value of the exponent range which is emin. In fact all subnormals have e = emin.
 * According to standard, normals has set to 1 the MSB of the fractional part, so this feature is kept here.
 *
 * An alternative approach, not yet considered is to renormalize subnormal numbers moving the fractional part up,
 * which corresponds with a scaling equal to the value of the radix. The scaling can be substracted from the exponent,
 * allowing subnormals to reach values below emin.
 */

fptriplet_t ADDCALL fp32_get_triplet( f32_t x )
{
    fptriplet_t ret;

    fpset_t type = fp32_get_subset( x );

    fw_t y = { .f = x };

    if( ( y.w[ FW0 ] & FW0_SMASK ) == 0 )
    {
        ret.s = +1;
    }

    else
    {
        ret.s = -1;
    }

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret.e = 1 + ( EMAX_F32 << 1 );
            ret.M = 1;
            break;
        }

        case (FP_TYPE_INFINITE):
        {
            ret.e = 1 + ( EMAX_F32 << 1 );
            ret.M = 0;
            break;
        }

        case (FP_TYPE_ZERO):
        {
            ret.e = 0;
            ret.M = 0;
            break;
        }

        case (FP_TYPE_SUBNORMAL): /* Check that bit 24 is set to 0 */
        {
            ret.e = 1 - EMAX_F32;
            ret.M = ( ( (u32_t) ( y.w[ FW0 ] & FW0_DMASK ) ) << 16 ) | ( (u32_t) ( y.w[ FW1 ] ) );
            break;
        }

        case (FP_TYPE_NORMAL): /* Check that bit 24 is set to 1 */
        default:
        {
            ret.e =   ( (i16_t) ( ( y.w[ FW0 ] & FW0_EMASK ) >> DOFFS_F32 ) ) - EMAX_F32;
            ret.M = ( 1 << 23 ) | ( ( (u32_t) ( y.w[ FW0 ] & FW0_DMASK ) ) << 16 ) | ( (u32_t) ( y.w[ FW1 ] ) );
            break;
        }
    }

    return ( ret );
}

fptriplet_t ADDCALL fp64_get_triplet( f64_t x )
{
    fptriplet_t ret;

    fpset_t type = fp64_get_subset( x );

    dw_t y = { .f = x };

    if( ( y.w[ DW0 ] & DW0_SMASK ) == 0 )
    {
        ret.s = +1;
    }

    else
    {
        ret.s = -1;
    }

    switch (type)
    {
        case (FP_TYPE_NAN):
        {
            ret.e = 1 + ( EMAX_F64 << 1 );
            ret.M = 1;
            break;
        }

        case (FP_TYPE_INFINITE):
        {
            ret.e = 1 + ( EMAX_F64 << 1 );
            ret.M = 0;
            break;
        }

        case (FP_TYPE_ZERO):
        {
            ret.e = 0;
            ret.M = 0;
            break;
        }

        case (FP_TYPE_SUBNORMAL): /* Check that bit 24 is set to 0 */
        {
            ret.e = 1 - EMAX_F64;
            ret.M = ( ( (u64_t) ( y.w[ DW0 ] & DW0_DMASK ) ) << 16 ) | ( (u64_t) ( y.w[ DW1 ] ) );
            break;
        }

        case (FP_TYPE_NORMAL): /* Check that bit 24 is set to 1 */
        default:
        {
            ret.e =   ( (i16_t) ( ( y.w[ DW0 ] & DW0_EMASK ) >> DOFFS_F64 ) ) - EMAX_F64;

            ret.M = ( 1ull << 52 ) |
                    ( ( (u64_t) ( y.w[ DW0 ] & DW0_DMASK ) ) << ( 16 + 32 ) ) |
                    ( (u64_t) ( y.w[ DW1 ] ) << 32 ) |
                    ( (u64_t) ( y.w[ DW2 ] ) << 16 ) |
                    ( (u64_t) ( y.w[ DW3 ] ) );

            break;
        }
    }

    return ( ret );
}

/**
 * fp32_eval_triplet
 *
 * given a floating point triplet x(s,M,e), evaluates the floating-point representation according to the format IEEE-754
 *
 * return the value of evaluate (-1)^s · M · B^{e-p+1}
 */

f32_t ADDCALL fp32_eval_triplet( fptriplet_t x )
{
    fw_t ret;

    if( x.e > EMAX_F32 )
    {
        if( x.M != 0 )
        {
            ret.f = m_qnanf32.f;
        }

        else
        {
            ret.f = ( (f32_t) x.s ) * m_inff32.f;
        }
    }

    else
    {
        ret.f = fp32_set_exp( 1., x.e );

        if( x.s == -1 )
        {
            ret.w[ FW0 ] = ret.w[ FW0 ] | FW0_SMASK;
        }

        ret.u = ret.u | ( (u32_t) x.M );
    }

    return ( ret.f );
}

/**
 * fp64_eval_triplet
 *
 * given a floating point triplet x(s,M,e), evaluates the floating-point representation according to the format IEEE-754
 *
 * return the value of evaluate (-1)^s · M · B^{e-p+1}
 */

f64_t ADDCALL fp64_eval_triplet( fptriplet_t x )
{
    dw_t ret;

    if( x.e > EMAX_F64 )
    {
        if( x.M != 0 )
        {
            ret.f = m_qnanf64.f;
        }

        else
        {
            ret.f = ( (f64_t) x.s ) * m_inff64.f;
        }
    }

    else
    {
        ret.f = fp64_set_exp( 1., x.e );

        if( x.s == -1 )
        {
            ret.w[ DW0 ] = ret.w[ DW0 ] | DW0_SMASK;
        }

        ret.u = ret.u | x.M;
    }

    return ( ret.f );
}

/**
 * fp32_set_exp
 *
 * x: floating-point s, m
 *                                              s       e
 * e: new (unbiased) exponent in the format (-1) · m · B
 *
 * return: a floating-point representation compliant with IEEE-754 | S(1bit) | E(8bit) | M(23bit) |
 *
 * NOTE: if x = 1.0, set_exp( x, e ) is equivalent to y = 2^e, with e in emin <= e <= emax
 */

f32_t ADDCALL fp32_set_exp ( f32_t x, i16_t e )
{
	fw_t ret;

    switch ( fp32_get_subset( x ) )
    {
        case (FP_TYPE_NORMAL):
        {
            if( e < EMIN_F32 )
            {
                /* TODO: add support to handle return subnormals when EMIN_SUBNORMAL <= e < EMIN_F64 */
                ret.f = 0.0;
            }

            else if( e > EMAX_F32 )
            {
                ret.f = m_inff32.f;
            }

            else
            {
                ret.f = x;
                F32_SET_EXP( ret.w[ FW0 ], e);
            }

            break;
        }

        default:
        case (FP_TYPE_NAN):
        case (FP_TYPE_INFINITE):
        case (FP_TYPE_ZERO):
        case (FP_TYPE_SUBNORMAL):
        {
            ret.f = x;
            break;
        }
    }

    return ret.f;
}

f64_t ADDCALL fp64_set_exp ( f64_t x, i16_t e )
{
	dw_t ret;

    switch ( fp64_get_subset( x ) )
    {
        case (FP_TYPE_NORMAL):
        {
            if( e < EMIN_F64 )
            {
                /* TODO: add support to handle return subnormals when EMIN_SUBNORMAL <= e < EMIN_F64 */
                ret.f = 0.0;
            }

            else if( e > EMAX_F64 )
            {
                ret.f = m_inff64.f;
            }

            else
            {
                ret.f = x;
                F64_SET_EXP( ret.w[ DW0 ], e);
            }

            break;
        }

        default:
        case (FP_TYPE_NAN):
        case (FP_TYPE_INFINITE):
        case (FP_TYPE_ZERO):
        case (FP_TYPE_SUBNORMAL):
        {
            ret.f = x;
            break;
        }
    }

    return ret.f;
}

fp32_vec2_t ADDCALL fp32_find_control_boundaries( f32_t at_x, const f32_t * control_points,  f64_t boundary_semi_length_ulps, i32_t n )
{
    f32_t local_epsl;
    f32_t local_epsr;

    f32_t control_left  = -m_inff32.f;
    f32_t control_right = +m_inff32.f;

    /*
     * As a protection mechanism, in case of passing null-pointer in control_points,
     * It is considered there is one single range, with left and right points at -inf and +inf respectively
     */

    if( control_points == NULLPTR )
    {
        /* NOP: already set to inf */
    }

    else
    {
        for( i32_t i = 0; i < n; i++ )
        {
            const f32_t control_center = control_points[ i ];

            if( control_center == +1.0f )
            {
                local_epsl      = +0.5f * m_epsf32.f;
                local_epsr      = +       m_epsf32.f;
            }

            else if( control_center == -1.0f )
            {
                local_epsl      = -       m_epsf32.f;
                local_epsr      = -0.5f * m_epsf32.f;
            }

            else if( control_center == 0.0f )
            {
                control_left    = -0.0;
                control_right   = +0.0;

                local_epsl      = 0.0f;
                local_epsr      = 0.0f;
            }

            else /* general method */
            {
                local_epsl      = ( fp32_next_float( control_center ) - control_center ) / control_center;
                local_epsr      = local_epsl;
            }

            control_left  = control_center * ( 1.f - boundary_semi_length_ulps * local_epsl );
            control_right = control_center * ( 1.f + boundary_semi_length_ulps * local_epsr );

            if ( control_right > at_x )
            {
                break;
            }
        }
    }

    return ( (fp32_vec2_t) { .x0 = control_left, .x1 = control_right, } );
}

fp64_vec2_t ADDCALL fp64_find_control_boundaries( f64_t at_x, const f64_t * control_points,  f64_t boundary_semi_length_ulps, i32_t n )
{
    f64_t local_epsl;
    f64_t local_epsr;

    f64_t control_left  = -m_inff64.f;
    f64_t control_right = +m_inff64.f;

    /*
     * As a protection mechanism, in case of passing null-pointer in control_points,
     * It is considered there is one single range, with left and right points at -inf and +inf respectively
     */

    if( control_points == NULLPTR )
    {
        /* NOP: already set to inf */
    }

    else
    {
        for( i32_t i = 0; i < n; i++ )
        {
            const f64_t control_center = control_points[ i ];

            if( control_center == +1.0 )
            {
                local_epsl      = +0.5 * m_epsf64.f;
                local_epsr      = +      m_epsf64.f;
            }

            else if( control_center == -1.0 )
            {
                local_epsl      = -      m_epsf64.f;
                local_epsr      = -0.5 * m_epsf64.f;
            }

            else if( control_center == 0.0 )
            {
                control_left    = -0.0;
                control_right   = +0.0;

                local_epsl      = 0.0;
                local_epsr      = 0.0;
            }

            else /* general method */
            {
                local_epsl      = ( fp64_next_float( control_center ) - control_center ) / control_center;
                local_epsr      = local_epsl;
            }

            control_left  = control_center * ( 1. - boundary_semi_length_ulps * local_epsl );
            control_right = control_center * ( 1. + boundary_semi_length_ulps * local_epsr );

            if ( control_right > at_x )
            {
                break;
            }
        }
    }

    return ( (fp64_vec2_t) { .x0 = control_left, .x1 = control_right, } );
}

/*
 * fp32_next_x
 *
 * compute the next value of x, considering certain points of control,
 * in which the next value is computed step by step to ensure the point of control is not skipped.
 *
 * Cases description:
 *
 * - CASE 1: x left   control -> next left control            : geom grow
 * - CASE 2: x left   control -> next passes left control     : step grow
 * - CASE 3: x inside control                                 : step grow
 * - CASE 4: x inside control zero (child of CASE 3)          : step grow
 * - CASE 5: x is one of the two infinites                    : step grow (-inf), clamp (+inf)
 */

f32_t ADDCALL fp32_next_x( f32_t x, f32_t frac, const f32_t * control_points, f64_t round_ulps, i32_t n )
{
    const fp32_vec2_t   control_point = fp32_find_control_boundaries( x, control_points, round_ulps, n );
    const f32_t         control_left  = control_point.x0;
    const f32_t         control_right = control_point.x1;

    f32_t next = fp32_geom_step_real_line( x, frac );

    /* CASE 2: x left   control -> next passes left control     : step grow */
    if( ( x < control_left ) && ( next >= control_left ) )
    {
        if( control_left == 0.0 )
        {
            next = -0.0; /* This helps to the user to avoid set the control point 0.0 as { -0.0 }*/
            g_fpenv.debug_next_x_inside_boundaries && printf("CASE 2: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
        }

        else
        {
            next = control_left;
        }
    }

    /* CASE 3: x inside control                                 : step grow */
    else if( ( control_left <= x ) && ( x <= control_right ) )
    {
        next = fp32_next_float( x );
        g_fpenv.debug_next_x_inside_boundaries && printf("CASE 3: %f, [%f -> %f], %f\n", control_left, x, next, control_right);
    }

    /* CASE 4: x inside control zero (child of CASE 3)          : step grow */
    else if ( fp32_get_subset( x ) == FP_TYPE_ZERO )
    {
        next = fp32_next_float( x );
        g_fpenv.debug_next_x_inside_boundaries && printf("CASE 4: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
    }

    /* CASE 5: x is one of the two infinites                    : step grow (-inf), clamp (+inf) */
    else if ( fp32_get_subset( x ) == FP_TYPE_INFINITE )
    {
        if( x == (-m_inff32.f) )
        {
            next = fp32_next_float( x );
            g_fpenv.debug_next_x_inside_boundaries && printf("CASE 5: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
        }

        else
        {
            next = x;
        }
    }

    /* CASE 1: x left   control -> next left control            : geom grow */
    else
    {
        /* NOP: geometrical grow already applied */
    }

    return ( next );
}

f64_t ADDCALL fp64_next_x( f64_t x, f64_t frac, const f64_t * control_points, f64_t round_ulps, i32_t n )
{
    const fp64_vec2_t   control_point = fp64_find_control_boundaries( x, control_points, round_ulps, n );
    const f64_t         control_left  = control_point.x0;
    const f64_t         control_right = control_point.x1;

    f64_t next = fp64_geom_step_real_line( x, frac );

    /* CASE 2: x left   control -> next passes left control     : step grow */
    if( ( x < control_left ) && ( next >= control_left ) )
    {
        if( control_left == 0.0 )
        {
            next = -0.0; /* This helps to the user to avoid set the control point 0.0 as { -0.0 }*/
            g_fpenv.debug_next_x_inside_boundaries && printf("CASE 2: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
        }

        else
        {
            next = control_left;
        }
    }

    /* CASE 3: x inside control                                 : step grow */
    else if( ( control_left <= x ) && ( x <= control_right ) )
    {
        next = fp64_next_float( x );
        g_fpenv.debug_next_x_inside_boundaries && printf("CASE 3: %f, [%f -> %f], %f\n", control_left, x, next, control_right);
    }

    /* CASE 4: x inside control zero (child of CASE 3)          : step grow */
    else if ( fp64_get_subset( x ) == FP_TYPE_ZERO )
    {
        next = fp64_next_float( x );
        g_fpenv.debug_next_x_inside_boundaries && printf("CASE 4: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
    }

    /* CASE 5: x is one of the two infinites                    : step grow (-inf), clamp (+inf) */
    else if ( fp64_get_subset( x ) == FP_TYPE_INFINITE )
    {
        if( x == (-m_inff64.f) )
        {
            next = fp64_next_float( x );
            g_fpenv.debug_next_x_inside_boundaries && printf("CASE 5: %+.1f, [%f -> %f], %+.1f\n", control_left, x, next, control_right);
        }

        else
        {
            next = x;
        }
    }

    /* CASE 1: x left   control -> next left control            : geom grow */
    else
    {
        /* NOP: geometrical grow already applied */
    }

    return ( next );
}

/*
 * test_range_analyzer ( desc, approx, expect, accept, reject )
 *
 * desc:    name of the function under test plus the test description, that will appear in the report
 * lhs:     function pointer to the left-hand-side value to compare with the rhs
 * rhs:     function pointer to the right-hand-side value to compare with the lhs
 * accept:  max difference in ulps between lhs and rhs to consider the test is passed
 * reject:  max difference in ulps between lhs and rhs to consider the test is reject
 *
 * state:   the state of the function when a test is rejected, useful to fast iterate a solution.
 *          If a not NULL state is given, the function will continue from that point, so in case of
 *          early rejection, the programmer can fix-up the code and continue the test from that, in order
 *          to know if the error is fixed or not. After this is done, the test can be reset by just
 *          passing a NULL pointer into state.
 *
 *          The state is passed as an ADT, but no ctor or dtor are given, so instead of a void *
 *          the programmer is requested to hold a minimum array size in bytes and pass its address.
 *          The state is expected to be a tiny structure, so there is no problem to hold it in the stack.
 */

void ADDCALL fp32_range_analyzer( cstr_t desc, fp_f2f_t lhs, fp_f2f_t rhs, fp_f2f_t next_x, f64_t accept, f64_t reject, u8_t state[ /* TBD: Size in bytes of the state */ ], fp32_vec2_t minmax )
{
    const bool_t reset_grow_frac = ( g_fpenv.fp32_grow_frac == 0.0f );
    const bool_t reset_ctrl_ulps = ( g_fpenv.fp32_ctrl_ulps == 0.0f );

    printf("\n Analyzing %s\n"LINE_SEPARATOR"\n", desc);

    if( reset_grow_frac )
    {
        g_fpenv.fp32_grow_frac = 1.e-4;
        fprintf(stderr,"[INFO]: given frac <= 0.0, selecting default frac = %e\n", g_fpenv.fp32_grow_frac);
    }

    if( reset_ctrl_ulps )
    {
        g_fpenv.fp32_ctrl_ulps = 100.0f;
        fprintf(stderr,"[INFO]: given control width in ulps <= 1.0, selecting default width = %f\n", g_fpenv.fp32_ctrl_ulps);
    }


    /* >> MAIN ANALYSIS */
    const f32_t x_min   = minmax.x0;
    const f32_t x_max   = minmax.x1;

    f32_t x_curr        = x_min;
    /* << MAIN ANALYSIS */

    /* >> SERVICE: PROGRESS BAR */
    fp_progress_state_t pstate =
    {
        .xexp = EMIN_F32,
        .count_points_in_the_range = 0,
    };
    /* << SERVICE: PROGRESS BAR */

    /* >> SERVICE: POINTS COUNTER */
    u64_t count_hi = 0;
    u64_t total_fp32 = (~0u   - (2u   * 8388607u  ) + 1u   ) ; /* 2^32 - all f32 NaNs */
    /* << SERVICE: POINTS COUNTER */

    /* >> SERVICE: ULP */
    ULPS_HIST_INIT;
    /* << SERVICE: ULP */

    /* >> SERVICE: RANGE FINDER */
    f32_t x_prev = x_curr;
    INIT_RANGE( x_prev, fp32_get_subset( lhs ( x_prev ) ) );
    /* << SERVICE: RANGE FINDER */

    /* >> SERVICE: ERROR LOOP TERMINATION */
    i32_t x_curr_ntimes_analyzed = 0;
    /* << SERVICE: ERROR LOOP TERMINATION */

    /* >> MAIN ANALYSIS */
    while( ( x_curr <= x_max ) && ( x_curr_ntimes_analyzed == 0 ) )
    {
        /* >> MAIN ANALYSIS */
        const f32_t     y_lhs       = lhs ( x_curr );
        const f32_t     y_rhs       = rhs ( x_curr );
        const fpset_t   y_lhs_type  = fp32_get_subset( y_lhs );
        /* << MAIN ANALYSIS */

        /* >> SERVICE: ULP */
        const f64_t curr_ulp = fabs( fp32_ulps( y_lhs, y_rhs ) );
        ULPS_HIST_RECORD( curr_ulp, x_curr );
        /* << SERVICE: ULP */

        {/* >> SERVICE: PROGRESS BAR */
            fp32_progress_bar( desc, g_fpenv, x_curr, x_min, x_max, &pstate );
        }/* << SERVICE: PROGRESS BAR */

        {/* >> SERVICE: POINTS COUNTER */
            count_hi++;
        }/* << SERVICE: POINTS COUNTER */

        {/* >> SERVICE: RANGE FINDER */
            UPDATE_RANGE(x_prev, x_curr, y_lhs, y_lhs_type);
            x_prev = x_curr;
        }/* << SERVICE: RANGE FINDER */

        {/* >> SERVICE: HIST ULP */
            fp_histogram_set_ulp( curr_ulp, &x_curr, FP32, reject );
        }/* << SERVICE: HIST ULP */

        {/* >> MAIN ANALYSIS */
            x_curr = next_x( x_curr ); /* Update x */
        }/* >> MAIN ANALYSIS */

        {/* >> SERVICE: ERROR LOOP TERMINATION */
            x_curr_ntimes_analyzed = ( fp32_equals( x_curr, x_prev ) ? ( x_curr_ntimes_analyzed + 1 ) : 0 );
        }/* << SERVICE: ERROR LOOP TERMINATION */
    }
    /* << MAIN ANALYSIS */

    {/* >> SERVICE: PROGRESS BAR */
        fp_console_remove_line( g_fpenv );
    }/* << SERVICE: PROGRESS BAR */

    {/* >> SERVICE: RANGE FINDER */
    CLOSE_RANGE(x_prev);
    }/* << SERVICE: RANGE FINDER */

    /*
     * REPORTING SECTION
     * TODO: define the state of this function and send all of this to another function is possible
     */

    if( reset_grow_frac ) { g_fpenv.fp32_grow_frac = 0.0f; }
    if( reset_ctrl_ulps ) { g_fpenv.fp32_ctrl_ulps = 0.0f; }

    bool_t success;
    {/* >> SERVICE: HIST ULP */
        success = fp_printf_hist_of_fails_by_exp( MAX_NHIST_F32, EMAX_F32, EMIN_F32, reject );
    }/* << SERVICE: HIST ULP */

    {/* >> SERVICE: RANGE FINDER */
        if( success != 0 ) { fp32_print_range_types( lhs ); }
    }/* << SERVICE: RANGE FINDER */

    {/* >> SERVICE: PROGRESS BAR */
        if( success != 0 ) { fp32_print_total_analyzed_points( count_hi, total_fp32, x_min, x_curr ); }
    }/* >> SERVICE: PROGRESS BAR */

    {/* >> SERVICE: ULP */
        if( success != 0 ) { ULPS_HIST_REPORT; }
    }/* << SERVICE: ULP */
}

void ADDCALL fp64_range_analyzer( cstr_t desc, fp_d2d_t lhs, fp_d2d_t rhs, fp_d2d_t next_x, f64_t accept, f64_t reject, u8_t state[ /* TBD: Size in bytes of the state */ ], fp64_vec2_t minmax )
{
    const bool_t reset_grow_frac = ( g_fpenv.fp64_grow_frac == 0.0 );
    const bool_t reset_ctrl_ulps = ( g_fpenv.fp64_ctrl_ulps == 0.0 );

    printf("\n Analyzing %s\n"LINE_SEPARATOR"\n", desc);

    if( reset_grow_frac ) { g_fpenv.fp64_grow_frac = 1.e-4; }
    if( reset_ctrl_ulps ) { g_fpenv.fp64_ctrl_ulps = 100.0f; }

    /* >> MAIN ANALYSIS */
    const f64_t x_min   = minmax.x0;
    const f64_t x_max   = minmax.x1;

    f64_t x_curr        = x_min;
    /* << MAIN ANALYSIS */

    /* >> SERVICE: PROGRESS BAR */
    fp_progress_state_t pstate =
    {
        .xexp = EMIN_F64,
        .count_points_in_the_range = 0,
    };
    /* << SERVICE: PROGRESS BAR */

    /* >> SERVICE: POINTS COUNTER */
    u64_t count_hi = 0;
    u64_t total_fp64 = (~0llu - (2llu * 8388607llu) + 1llu ) ; /* 2^64 - all f64 NaNs */
    /* << SERVICE: POINTS COUNTER */

    /* >> SERVICE: ULP */
    ULPS_HIST_INIT;
    /* << SERVICE: ULP */

    /* >> SERVICE: RANGE FINDER */
    f64_t x_prev = x_curr;
    INIT_RANGE( x_prev, fp64_get_subset( lhs ( x_prev ) ) );
    /* << SERVICE: RANGE FINDER */

    /* >> SERVICE: ERROR LOOP TERMINATION */
    i32_t x_curr_ntimes_analyzed = 0;
    /* << SERVICE: ERROR LOOP TERMINATION */

    /* >> MAIN ANALYSIS */
    while( ( x_curr <= x_max ) && ( x_curr_ntimes_analyzed == 0 ) )
    {
        /* >> MAIN ANALYSIS */
        const f64_t     y_lhs       = lhs ( x_curr );
        const f64_t     y_rhs       = rhs ( x_curr );
        const fpset_t   y_lhs_type  = fp64_get_subset( y_lhs );
        /* << MAIN ANALYSIS */

        /* >> SERVICE: ULP */
        const f64_t curr_ulp = fabs( fp64_ulps( y_lhs, y_rhs ) );
        ULPS_HIST_RECORD( curr_ulp, x_curr );
        /* << SERVICE: ULP */

//      TODO: add callback to log files
//        if(curr_ulp > reject)
//        {
//            union {double f; u64_t i;} u = {x_curr};
//            u32_t ix = u.i>>32 & 0x7fffffff;
//            printf("%x, %.15e, %f\n", ix, x_curr, curr_ulp);
//        }

        {/* >> SERVICE: HISTOGRAM */
            u64_t npoints_in_range = 0;
        }/* << SERVICE: HISTOGRAM */

        {/* >> SERVICE: PROGRESS BAR */
            fp64_progress_bar( desc, g_fpenv, x_curr, x_min, x_max, &pstate );
        }/* << SERVICE: PROGRESS BAR */

        {/* >> SERVICE: POINTS COUNTER */
            count_hi++;
        }/* << SERVICE: POINTS COUNTER */

        {/* >> SERVICE: RANGE FINDER */
            UPDATE_RANGE(x_prev, x_curr, y_lhs, y_lhs_type);
            x_prev = x_curr;
        }/* << SERVICE: RANGE FINDER */

        {/* >> SERVICE: HIST ULP */
            fp_histogram_set_ulp( curr_ulp, &x_curr, FP64, reject );
        }/* << SERVICE: HIST ULP */

        {/* >> MAIN ANALYSIS */
            x_curr = next_x( x_curr ); /* Update x */
        }/* >> MAIN ANALYSIS */

        {/* >> SERVICE: ERROR LOOP TERMINATION */
            x_curr_ntimes_analyzed = ( fp64_equals( x_curr, x_prev ) ? ( x_curr_ntimes_analyzed + 1 ) : 0 );
        }/* << SERVICE: ERROR LOOP TERMINATION */
    }
    /* << MAIN ANALYSIS */

    {/* >> SERVICE: PROGRESS BAR */
        fp_console_remove_line( g_fpenv );
    }/* << SERVICE: PROGRESS BAR */

    {/* >> SERVICE: RANGE FINDER */
    CLOSE_RANGE(x_prev);
    }/* << SERVICE: RANGE FINDER */

    /*
     * REPORTING SECTION
     * TODO: define the state of this function and send all of this to another function is possible
     */

    if( reset_grow_frac ) { g_fpenv.fp64_grow_frac = 0.0; }
    if( reset_ctrl_ulps ) { g_fpenv.fp64_ctrl_ulps = 0.0; }

    bool_t success;
    {/* >> SERVICE: HIST ULP */
        success = fp_printf_hist_of_fails_by_exp( MAX_NHIST_F64, EMAX_F64, EMIN_F64, reject );
    }/* << SERVICE: HIST ULP */

    {/* >> SERVICE: RANGE FINDER */
        if( success != 0 ) { fp64_print_range_types( lhs ); }
    }/* << SERVICE: RANGE FINDER */

    {/* >> SERVICE: PROGRESS BAR */
        if( success != 0 ) { fp64_print_total_analyzed_points( count_hi, total_fp64, x_min, x_curr ); }
    }/* >> SERVICE: PROGRESS BAR */

    {/* >> SERVICE: ULP */
        if( success != 0 ) { ULPS_HIST_REPORT; }
    }/* << SERVICE: ULP */
}

f32_t ADDCALL fp32_geometric_grow( f32_t x )
{
    static const f32_t default_ctrlp[ ] =
    {
        -FP32_7PI,
        -FP32_9PI4,
        -FP32_2PI,
        -FP32_7PI4,
        -FP32_3PI2,
        -FP32_R4,
        -FP32_5PI4,
        -FP32_PI,
        -FP32_EULER,
        -FP32_3PI4,
        -FP32_R2,
        -FP32_PI2,
        -FP32_SQRT2,
        -FP32_R1,
        -FP32_PI4,
        -FP32_1R2,
        -FP32_MIN,
        +FP32_ZERO,
        +FP32_MIN,
        +FP32_1R2,
        +FP32_PI4,
        +FP32_R1,
        +FP32_SQRT2,
        +FP32_PI2,
        +FP32_R2,
        +FP32_3PI4,
        +FP32_EULER,
        +FP32_PI,
        +FP32_R4,
        +FP32_5PI4,
        +FP32_3PI2,
        +FP32_7PI4,
        +FP32_2PI,
        +FP32_9PI4,
        +FP32_7PI,
    };

    return ( fp32_next_x( x, g_fpenv.fp32_grow_frac, default_ctrlp, g_fpenv.fp32_ctrl_ulps, SIZEOF( default_ctrlp ) ) );
}

f64_t ADDCALL fp64_geometric_grow( f64_t x )
{
    static const f64_t default_ctrlp[ ] =
    {
        -FP64_7PI,
        -FP64_9PI4,
        -FP64_2PI,
        -FP64_7PI4,
        -FP64_3PI2,
        -FP64_R4,
        -FP64_5PI4,
        -FP64_PI,
        -FP64_EULER,
        -FP64_3PI4,
        -FP64_R2,
        -FP64_PI2,
        -FP64_SQRT2,
        -FP64_R1,
        -FP64_PI4,
        -FP64_1R2,
        -FP64_MIN,
        +FP64_ZERO,
        +FP64_MIN,
        +FP64_1R2,
        +FP64_PI4,
        +FP64_R1,
        +FP64_SQRT2,
        +FP64_PI2,
        +FP64_R2,
        +FP64_3PI4,
        +FP64_EULER,
        +FP64_PI,
        +FP64_5PI4,
        +FP64_R4,
        +FP64_3PI2,
        +FP64_7PI4,
        +FP64_2PI,
        +FP64_9PI4,
        +FP64_7PI,
    };

    return ( fp64_next_x( x, g_fpenv.fp64_grow_frac, default_ctrlp, g_fpenv.fp64_ctrl_ulps, SIZEOF( default_ctrlp ) ) );
}

/*
 * SECTION: BENCHMARK
 */

static i32_t fp80_compare(const void* a, const void* b)
{
    return (*(f80_t*)a > *(f80_t*)b);
}

static inline u64_t fp_get_time_ns()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64_t) ts.tv_sec * 1.e9 + ts.tv_nsec;
}

#ifdef CPU_SET
static void fp_pin_cpu()
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(set), &set);
}
#endif

/* Functions to count the time per call and ret */
f32_t ADDCALL fp32_benchmark_mock_fun( f32_t x ) { volatile f32_t ret = 0.0; return ret; }
f64_t ADDCALL fp64_benchmark_mock_fun( f64_t x ) { volatile f64_t ret = 0.0; return ret; }

fp64_vec2_t ADDCALL fp32_benchmark_avg_time( fp_f2f_t test_func )
{
    fp64_vec2_t ret;

    static const u32_t niter = 100;

    u32_t ndiscards = 0;

    static f80_t tr[100] = {0};

    for ( u32_t i = 0; i < niter; ++i )
    {
        tr[i] = (f80_t) FP32_BENCHMARK_MATH_FUNCTION( test_func );
    }

    qsort( tr, niter, sizeof(f80_t), fp80_compare);

    const f80_t median = tr[ niter / 2 ];
    f80_t acc = 0.0L;
    for ( u32_t i = 0; i < niter; ++i )
    {
        if( tr[i] < 1.e3 )
        {
            acc += tr[i];
        }
        else
        {
            ndiscards++;
        }
    }

    const f80_t mean = 0.5 * ( acc / (niter - ndiscards) + median );

    acc = 0.0L;
    for ( u32_t i = 0; i < niter; ++i )
    {
        if( tr[i] < 1.e3 )
        {
            acc += ( tr[i] - mean ) * ( tr[i] - mean );
        }

        tr[i] = 0.0L; // Reset for next batch
    }

    const f80_t sd = sqrtl( acc ) / (f80_t) (niter - ndiscards);

    ret.x0 = (f64_t) median;
    ret.x1 = (f64_t) 4. * sd;

    return ( ret );
}

fp64_vec2_t ADDCALL fp64_benchmark_avg_time( fp_d2d_t test_func )
{
    fp64_vec2_t ret;

    static const u32_t niter = 100;

    u32_t ndiscards = 0;

    static f80_t tr[100] = {0};

    for ( u32_t i = 0; i < niter; ++i )
    {
        tr[i] = (f80_t) FP64_BENCHMARK_MATH_FUNCTION( test_func );
    }

    qsort( tr, niter, sizeof(f80_t), fp80_compare);

    const f80_t median = tr[ niter / 2 ];
    f80_t acc = 0.0L;
    for ( u32_t i = 0; i < niter; ++i )
    {
        if( tr[i] < 1.e3 )
        {
            acc += tr[i];
        }
        else
        {
            ndiscards++;
        }
    }

    const f80_t mean = 0.5 * ( acc / (niter - ndiscards) + median );

    acc = 0.0L;
    for ( u32_t i = 0; i < niter; ++i )
    {
        if( tr[i] < 1.e3 )
        {
            acc += ( tr[i] - mean ) * ( tr[i] - mean );
        }

        tr[i] = 0.0L; // Reset for next batch
    }

    const f80_t sd = sqrtl( acc ) / (f80_t) (niter - ndiscards);

    ret.x0 = (f64_t) median;
    ret.x1 = (f64_t) 4. * sd;

    return ( ret );
}

f64_t ADDCALL fp32_benchmark_core_ns_per_call( fp_f2f_t test_fun, fp_f2f_t mock_fun )
{
    volatile f32_t dummy;
    f32_t x;
    u64_t start, end;
    u64_t ncalls = 0;

    /* Warm-up */
    for( i32_t i = 0; i < 1000; i++ ) { dummy = test_fun( (f32_t) i ); }

    /* Measure function + loop */
    start = fp_get_time_ns();
    test_fun( -m_inff32.f );  test_fun( +m_inff32.f );
    test_fun( -m_qnanf32.f ); test_fun( +m_qnanf32.f );
    test_fun( -m_subnf32.f ); test_fun( +m_subnf32.f );
    x = -m_maxf32.f; while ( x != 0.0 ) { dummy = test_fun(x); x = 0.5f * x; }
    x = +m_maxf32.f; while ( x != 0.0 ) { dummy = test_fun(x); x = 0.5f * x; }
    end = fp_get_time_ns();

    u64_t total_time = end - start;

    /* Measure loop overhead */
    start = fp_get_time_ns();
    (void)( -m_inff32.f );  (void)( +m_inff32.f );
    (void)( -m_qnanf32.f ); (void)( +m_qnanf32.f );
    (void)( -m_subnf32.f ); (void)( +m_subnf32.f );
    x = -m_maxf32.f; while ( x != 0.0 ) { dummy = mock_fun(x); x = 0.5f * x; }
    x = +m_maxf32.f; while ( x != 0.0 ) { dummy = mock_fun(x); x = 0.5f * x; }
    end = fp_get_time_ns();

    u64_t loop_time = end - start;

    /* Measure number of calls */
    ncalls++; ncalls++; /* +2 infinites */
    ncalls++; ncalls++; /* +2 qnan */
    ncalls++; ncalls++; /* +2 subn */
    x = -m_maxf32.f; while ( x != 0.0 ) { ncalls++; x = 0.5f * x; }
    x = +m_maxf32.f; while ( x != 0.0 ) { ncalls++; x = 0.5f * x; }

    /* Compute per-call time */
    return (f64_t)(total_time - loop_time) / (f64_t) ncalls;
}

f64_t ADDCALL fp64_benchmark_core_ns_per_call( fp_d2d_t test_fun, fp_d2d_t mock_fun )
{
    volatile f64_t dummy;
    f64_t x;
    u64_t start, end;
    u64_t ncalls = 0;

    /* Warm-up */
    for( i32_t i = 0; i < 1000; i++ ) { dummy = test_fun( (f64_t) i ); }

    /* Measure function + loop */
    start = fp_get_time_ns();
    test_fun( -m_inff64.f );  test_fun( +m_inff64.f );
    test_fun( -m_qnanf64.f ); test_fun( +m_qnanf64.f );
    test_fun( -m_subnf64.f ); test_fun( +m_subnf64.f );
    x = -m_maxf64.f; while ( x != 0.0 ) { dummy = test_fun(x); x = 0.5 * x; }
    x = +m_maxf64.f; while ( x != 0.0 ) { dummy = test_fun(x); x = 0.5 * x; }
    end = fp_get_time_ns();

    u64_t total_time = end - start;

    /* Measure loop overhead */
    start = fp_get_time_ns();
    (void)( -m_inff64.f );  (void)( +m_inff64.f );
    (void)( -m_qnanf64.f ); (void)( +m_qnanf64.f );
    (void)( -m_subnf64.f ); (void)( +m_subnf64.f );
    x = -m_maxf64.f; while ( x != 0.0 ) { dummy = mock_fun(x); x = 0.5 * x; }
    x = +m_maxf64.f; while ( x != 0.0 ) { dummy = mock_fun(x); x = 0.5 * x; }
    end = fp_get_time_ns();

    u64_t loop_time = end - start;

    /* Measure number of calls  */
    ncalls++; ncalls++; /* +2 infinites */
    ncalls++; ncalls++; /* +2 qnan */
    ncalls++; ncalls++; /* +2 subn */
    x = -m_maxf64.f; while ( x != 0.0 ) { ncalls++; x = 0.5 * x; }
    x = +m_maxf64.f; while ( x != 0.0 ) { ncalls++; x = 0.5 * x; }

    /* Compute per-call time */
    return (f64_t)(total_time - loop_time) / (f64_t) ncalls;
}

void ADDCALL fp32_print_benchmark_avg_time_evolution( fp_f2f_t fptr3232 )
{
    while(1)
    {
        fp64_vec2_t response_time;

        f64_t mean = 0.0;
        f64_t sd = 0.0;
        u32_t n = 0;
        u32_t nmax = 0;

        f64_t prev = 0.0;

        while( nmax != 1000 )
        {
            nmax++;

            response_time = fp32_benchmark_avg_time( fptr3232 );

            if( response_time.x0 < 1.e3 && response_time.x1 < 1.e3 )
            {
                n++;
                f64_t a = 1./((f64_t)n);
                f64_t b = 1. - a;
                mean = a*response_time.x0 + b*mean;
                sd   = a*response_time.x1 + b*sd;

                f64_t rel_diff = fabs( ( prev - mean ) / mean );

                if( rel_diff < 1.e-6 )
                {
                    break;
                }
                else
                {
                    prev = mean;
                }
            }
        }

        if( nmax == 1000 )
        {
            /* fprintf(stderr, "[ERROR] benchmark cannot find an average time for function\n"); */
        }
        else
        {
            printf("%3.3f, %3.3f\n", mean, sd);
        }
    }
}

void ADDCALL fp64_print_benchmark_avg_time_evolution( fp_d2d_t fptr6464 )
{
    while(1)
    {
        fp64_vec2_t response_time;

        f64_t mean = 0.0;
        f64_t sd = 0.0;
        u32_t n = 0;
        u32_t nmax = 0;

        f64_t prev = 0.0;

        while( nmax != 1000 )
        {
            nmax++;

            response_time = fp64_benchmark_avg_time( fptr6464 );

            if( response_time.x0 < 1.e3 && response_time.x1 < 1.e3 )
            {
                n++;
                f64_t a = 1./((f64_t)n);
                f64_t b = 1. - a;
                mean = a*response_time.x0 + b*mean;
                sd   = a*response_time.x1 + b*sd;

                f64_t rel_diff = fabs( ( prev - mean ) / mean );

                if( rel_diff < 1.e-6 )
                {
                    break;
                }
                else
                {
                    prev = mean;
                }
            }
        }

        if( nmax == 1000 )
        {
            /* fprintf(stderr, "[ERROR] benchmark cannot find an average time for function\n"); */
        }
        else
        {
            printf("%3.3f, %3.3f\n", mean, sd);
        }
    }
}

void ADDCALL fp32_print_benchmark_avg_time( cstr_t fname, fp_f2f_t fptr3232 )
{
    fp64_vec2_t tr = fp32_benchmark_avg_time( fptr3232 );

    printf("%s took: %3.3f, %3.3f\n", fname, tr.x0, tr.x1);
}

void ADDCALL fp64_print_benchmark_avg_time( cstr_t fname, fp_d2d_t fptr6464 )
{
    fp64_vec2_t tr = fp64_benchmark_avg_time( fptr6464 );

    printf("%s took: %3.3f, %3.3f\n", fname, tr.x0, tr.x1);
}


/* >> Arbitrary Radix-10 Arithmetics */
static u32_t fp_radix10_get_fpi( cstr_t x )
{
    u32_t ret;

    const char_t * p = strchr( x, '.' );

    if( p == NULLPTR )
    {
        ret = strlen( x );
    }
    else
    {
        ret = (u32_t) ( p - x );
    }

    return ( ret );
}

#define FP_RADIX10 10

static u32_t fp_radix10_count_digits( u64_t n )
{
    u32_t ret = 1;

    while( n > (FP_RADIX10 - 1) )
    {
        n = n / FP_RADIX10;
        ret++;
    }

    return ( ret );
}

static u64_t fp_radix10_10powers( u8_t n )
{
    u64_t ret = 1u; /* exact result when n = 0 */

    u64_t x2 = FP_RADIX10;

    while ( n > 0 )
    {
        if ( n & 1 )
        {
            ret = ret * x2;
        }

        n = n >> 1;

        if ( n > 0 )
        {
            x2 = x2 * x2;
        }
    }

    return ( ret );
}

fp_radix10_t ADDCALL fp_radix10_add( fp_radix10_t a, fp_radix10_t b )
{
    fp_radix10_t ret = { 0 };

    if( (ret.ok = ( a.ok && b.ok )) )
    {
        ret.fr = a.fr + b.fr;

        u64_t carry = 0;

        if( (ret.ok = ( ret.fr >= a.fr && ret.fr >= b.fr )) ) /* no overflow */
        {
            const u32_t nd_fr_a = fp_radix10_count_digits( a.fr   );
            const u32_t nd_fr_b = fp_radix10_count_digits( b.fr   );
            const u32_t nd_fr_r = fp_radix10_count_digits( ret.fr );
            const u32_t nd_fr_m = nd_fr_a > nd_fr_b ? nd_fr_a : nd_fr_b;

            if( nd_fr_r > nd_fr_m )
            {
                carry = 1;
                ret.fr -= fp_radix10_10powers( nd_fr_m );
            }
        }

        printf("0.%llu\n0.%llu\n%llu.%llu\n", a.fr, b.fr, carry, ret.fr);

    }
    else
    {
        /* NOP: already initialized to zero */
    }

    return ( a );
}
/* << Arbitrary Radix-10 Arithmetics */

/* >> Random Numbers */
f32_t fp32_rand_in_range( f32_t min, f32_t max )
{
    const f32_t scale = rand() / (f32_t) RAND_MAX;
    return min + scale * ( max - min );
}

f64_t fp64_rand_in_range( f64_t min, f64_t max )
{
    const f64_t scale = rand() / (f64_t) RAND_MAX;
    return min + scale * ( max - min );
}
/* << Random Numbers */

/* >> Tests  */
#define FP32_RANGE_LOADER(fname,fenv_min, fenv_max) \
    if( fp32_get_subset( g_fpenv.fenv_min ) == FP_TYPE_ZERO || \
        fp32_get_subset( g_fpenv.fenv_max ) == FP_TYPE_ZERO ) \
    { \
        desc = fname " [info] g_fenv range not set, hence defaulted to [-inf, +inf]"; \
        valid_range.x0 = -m_inff32.f; \
        valid_range.x1 = +m_inff32.f; \
    } \
    else \
    { \
        desc = fname; \
        valid_range.x0 = g_fpenv.fenv_min; \
        valid_range.x1 = g_fpenv.fenv_max; \
    }

#define FP64_RANGE_LOADER(fname,fenv_min, fenv_max) \
    if( fp64_get_subset( g_fpenv.fenv_min ) == FP_TYPE_ZERO || \
        fp64_get_subset( g_fpenv.fenv_max ) == FP_TYPE_ZERO ) \
    { \
        desc = fname " [info] g_fenv range not set, hence defaulted to [-inf, +inf]"; \
        valid_range.x0 = -m_inff64.f; \
        valid_range.x1 = +m_inff64.f; \
    } \
    else \
    { \
        desc = fname; \
        valid_range.x0 = g_fpenv.fenv_min; \
        valid_range.x1 = g_fpenv.fenv_max; \
    }

extern void ADDCALL fp32_test_sqrt( fp_f2f_t tested_sqrt, bool_t active )
{
    fp32_vec2_t valid_range;
    const char * desc;

    FP32_RANGE_LOADER("sqrt", fp32_range_sqrt_min, fp32_range_sqrt_max);

    if( active )
    {
        fp32_range_analyzer(desc, tested_sqrt, sqrtf, fp32_geometric_grow, 1.0, 2.0, NULL, valid_range);

        fp32_print_benchmark_avg_time( "math_sqrt", tested_sqrt );
        fp32_print_benchmark_avg_time( "cstd_sqrt", sqrtf );
    }
}

extern void ADDCALL fp32_test_exp( fp_f2f_t tested_exp, bool_t active )
{

    if( active )
    {
        fp32_vec2_t valid_range;
        const char * desc;
        FP32_RANGE_LOADER("exp", fp32_range_exp_min, fp32_range_exp_max);
        fp32_range_analyzer(desc, tested_exp, expf, fp32_geometric_grow, 1.0, 8.0, NULL, valid_range);

        fp32_print_benchmark_avg_time("math_exp", tested_exp);
        fp32_print_benchmark_avg_time("cstd_exp", expf);
    }
}

extern void ADDCALL fp32_test_log( fp_f2f_t tested_log, bool_t active )
{

    if( active )
    {
        fp32_vec2_t valid_range;
        const char * desc;
        FP32_RANGE_LOADER("log", fp32_range_log_min, fp32_range_log_max);
        fp32_range_analyzer(desc, tested_log, logf, fp32_geometric_grow, 1.0, 2.0, NULL, valid_range);

        fp32_print_benchmark_avg_time("math_log", tested_log);
        fp32_print_benchmark_avg_time("cstd_log", logf);
    }
}

extern void ADDCALL fp32_test_sin(fp_f2f_t tested_sin, bool_t active)
{

    if( active )
    {
        fp32_vec2_t valid_range;
        const char * desc;
        FP32_RANGE_LOADER("sin", fp32_range_sin_min, fp32_range_sin_max);
        fp32_range_analyzer(desc, tested_sin, sinf, fp32_geometric_grow, 1.0, 8.0, NULL, valid_range);

        fp32_print_benchmark_avg_time("math_sin", tested_sin);
        fp32_print_benchmark_avg_time("cstd_sin", sinf);
    }
}

extern void ADDCALL fp32_test_cos( fp_f2f_t tested_cos, bool_t active )
{

    if( active )
    {
        fp32_vec2_t valid_range;
        const char * desc;
        FP32_RANGE_LOADER("cos", fp32_range_cos_min, fp32_range_cos_max);
        fp32_range_analyzer(desc, tested_cos, cosf, fp32_geometric_grow, 1.0, 8.0, NULL, valid_range);

        fp32_print_benchmark_avg_time("math_cos", tested_cos);
        fp32_print_benchmark_avg_time("cstd_cos", cosf);
    }
}

extern void ADDCALL fp32_test_asin( fp_f2f_t tested_asin, bool_t active )
{

    if( active )
    {
        fp32_vec2_t valid_range;
        const char * desc;
        FP32_RANGE_LOADER("asin", fp32_range_asin_min, fp32_range_asin_max);
        fp32_range_analyzer(desc, tested_asin, asinf, fp32_geometric_grow, 1.0, 8.0, NULL, valid_range);

        fp32_print_benchmark_avg_time("math_asin", tested_asin);
        fp32_print_benchmark_avg_time("cstd_asin", asinf);
    }
}

f32_t (* m_powy32 )(f32_t, f32_t);

f32_t m_y32;

f32_t tested_powy32( f32_t x )
{
    return ( m_powy32 ( x, m_y32 ) );
}

f32_t expected_powy32( f32_t x )
{
    return ( powf ( x, m_y32 ) );
}

extern void ADDCALL fp32_test_pow( f32_t (*tested_pow)(f32_t, f32_t) , bool_t active )
{
    m_powy32 = tested_pow;

    if( active )
    {
        fp32_vec2_t valid_range;

        m_y32 = 0.0;
        do
        {
            valid_range.x1 = +exp( log( m_maxf32.f ) / m_y32 );
            valid_range.x0 = -valid_range.x0;
            char description[120] = { '\0' };
            sprintf( description, "pow y=%f", m_y32 );
            fp32_range_analyzer(description, tested_powy32, expected_powy32, fp32_geometric_grow, 1.0, 8.0, NULL, valid_range);

            fp32_print_benchmark_avg_time("math_pow", tested_powy32);
            fp32_print_benchmark_avg_time("cstd_pow", expected_powy32);
            m_y32 += 0.25;
        }
        while( m_y32 < 10.0 );
    }
}

extern void ADDCALL fp64_test_sqrt( fp_d2d_t tested_sqrt, bool_t active )
{

    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("sqrt", fp64_range_sqrt_min, fp64_range_sqrt_max);
        fp64_range_analyzer(desc, tested_sqrt, sqrt , fp64_geometric_grow, g_fpenv.fp64_sqrt_accepted, g_fpenv.fp64_sqrt_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time( "math_sqrt", tested_sqrt );
        fp64_print_benchmark_avg_time( "cstd_sqrt", sqrt );
    }
}

extern void ADDCALL fp64_test_exp( fp_d2d_t tested_exp, bool_t active )
{

    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("exp", fp64_range_exp_min, fp64_range_exp_max);
        fp64_range_analyzer(desc, tested_exp, exp, fp64_geometric_grow, g_fpenv.fp64_exp_accepted, g_fpenv.fp64_exp_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time("math_exp", tested_exp);
        fp64_print_benchmark_avg_time("cstd_exp", exp);
    }
}

extern void ADDCALL fp64_test_log( fp_d2d_t tested_log, bool_t active )
{
    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("log", fp64_range_log_min, fp64_range_log_max);

        fp64_range_analyzer(desc, tested_log, log, fp64_geometric_grow, g_fpenv.fp64_log_accepted, g_fpenv.fp64_log_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time("math_log", tested_log);
        fp64_print_benchmark_avg_time("cstd_log", log);
    }
}

extern void ADDCALL fp64_test_sin(fp_d2d_t tested_sin, bool_t active)
{
    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("sin", fp64_range_sin_min, fp64_range_sin_max);
        fp64_range_analyzer(desc, tested_sin, sin, fp64_geometric_grow, g_fpenv.fp64_sin_accepted, g_fpenv.fp64_sin_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time("math_sin", tested_sin);
        fp64_print_benchmark_avg_time("cstd_sin", sin);
    }
}

extern void ADDCALL fp64_test_cos( fp_d2d_t tested_cos, bool_t active )
{
    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("cos", fp64_range_cos_min, fp64_range_cos_max);
        fp64_range_analyzer(desc, tested_cos, cos, fp64_geometric_grow, g_fpenv.fp64_cos_accepted, g_fpenv.fp64_cos_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time("math_cos", tested_cos);
        fp64_print_benchmark_avg_time("cstd_cos", cos);
    }
}

extern void ADDCALL fp64_test_asin( fp_d2d_t tested_asin, bool_t active )
{
    if( active )
    {
        fp64_vec2_t valid_range;
        const char * desc;
        FP64_RANGE_LOADER("asin", fp64_range_asin_min, fp64_range_asin_max);
        fp64_range_analyzer(desc, tested_asin, asin, fp64_geometric_grow, g_fpenv.fp64_asin_accepted, g_fpenv.fp64_asin_rejected, NULL, valid_range);

        fp64_print_benchmark_avg_time("math_asin", tested_asin);
        fp64_print_benchmark_avg_time("cstd_asin", asin);
    }
}

f64_t (* m_powy64 )(f64_t, f64_t);

f64_t m_y64;

f64_t tested_powy64( f64_t x )
{
    return ( m_powy64 ( x, m_y64 ) );
}

f64_t expected_powy64( f64_t x )
{
    return ( pow ( x, m_y64 ) );
}

extern void ADDCALL fp64_test_pow( f64_t (*tested_pow)(f64_t, f64_t) , bool_t active )
{
    m_powy64 = tested_pow;

    if( active )
    {
        fp64_vec2_t valid_range;

        m_y64 = 0.0;
        do
        {
            fp64_vec2_t valid_range;
            const char * desc;
            FP64_RANGE_LOADER("", fp64_range_powy_min, fp64_range_powy_max);

            char description[120] = { '\0' };
            sprintf( description, "pow y=%f", m_y64 );
            fp64_range_analyzer(description, tested_powy64, expected_powy64, fp64_geometric_grow, 1.0, 8.0, NULL, valid_range);

            fp64_print_benchmark_avg_time("math_pow", tested_powy64);
            fp64_print_benchmark_avg_time("cstd_pow", expected_powy64);
            m_y64 += 0.25;
        }
        while( m_y64 < 10.0 );
    }
}
/* << Tests */

extern fp32_fast_t ADDCALL fp32_fast2sum( f32_t a, f32_t b )
{
    volatile fp32_fast_t ret;

    volatile f32_t x;
    volatile f32_t y;

    if( fabs(a) > fabs(b) ) { x = a; y = b; }
    else                    { x = b; y = a; }

    ret.s = x + y;
    ret.t = y - ( ret.s - x );

    return ( ret );
}

extern fp64_fast_t ADDCALL fp64_fast2sum( f64_t a, f64_t b )
{
    volatile fp64_fast_t ret;

    volatile f64_t x;
    volatile f64_t y;

    if( fabs(a) > fabs(b) ) { x = a; y = b; }
    else                    { x = b; y = a; }

    ret.s = x + y;
    ret.t = y - ( ret.s - x );

    return ( ret );
}

extern fp32_fast_t ADDCALL fp32_fast2mul( f32_t a, f32_t b )
{
    volatile fp32_fast_t ret;

    ret.s = a * b;
    ret.t = fmaf( a, b, -ret.s );

    return ( ret );
}

extern fp64_fast_t ADDCALL fp64_fast2mul( f64_t a, f64_t b )
{
    volatile fp64_fast_t ret;

    ret.s = a * b;
    ret.t = fma( a, b, -ret.s );

    return ( ret );
}
