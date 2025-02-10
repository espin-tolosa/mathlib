#ifndef PUBLIC_MATH_H
#define PUBLIC_MATH_H

typedef struct
{
    double re;
    double im;
} complex64_t;

extern double math_sqrt         ( double x );
extern double math_exp          ( double x );
extern double math_log          ( double x );
extern double math_sin          ( double x );
extern double math_cos          ( double x );
extern double math_asin         ( double x );
extern double math_pow          ( double x , double y);

#define FP64_PI 	(3.1415926535897932384626433832795)





extern double math_correlation  (const double      *x, const double      *y, unsigned n);
extern double math_surface      (const complex64_t *x, const complex64_t *y, unsigned n);

#endif
