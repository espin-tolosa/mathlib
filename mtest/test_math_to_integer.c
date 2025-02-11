#include <assert.h>
#include <stdio.h>
#include <stdio.h>
#include <fenv.h>
#include "fptest.h"
#include "../math/src/math_primitives.c"

int main ( void )
{
    TEST_INIT;

    fesetround(FE_TONEAREST);

    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.0     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.0     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.1     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.1     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.9     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.9     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.0     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.0     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.1     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.9     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.9     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.0     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.0     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.1     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.e20 , math_to_integer( +1.e20   ) );
    TEST_ASSERT_EQUAL_FP64( -1.e20 , math_to_integer( -1.e20   ) );

    TEST_ASSERT_EQUAL_FP64( +4503599627370495.0 , math_to_integer( +4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370496.0 , math_to_integer( +4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370497.0 , math_to_integer( +4503599627370497.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370495.0 , math_to_integer( -4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370496.0 , math_to_integer( -4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370497.0 , math_to_integer( -4503599627370497.0 ) );

    fesetround(FE_DOWNWARD);

    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.0     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.0     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.1     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.1     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.9     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.9     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.0     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.0     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.1     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.9     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.9     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.0     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.0     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.1     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.e20 , math_to_integer( +1.e20   ) );
    TEST_ASSERT_EQUAL_FP64( -1.e20 , math_to_integer( -1.e20   ) );

    TEST_ASSERT_EQUAL_FP64( +4503599627370495.0 , math_to_integer( +4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370496.0 , math_to_integer( +4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370497.0 , math_to_integer( +4503599627370497.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370495.0 , math_to_integer( -4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370496.0 , math_to_integer( -4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370497.0 , math_to_integer( -4503599627370497.0 ) );

    fesetround(FE_TOWARDZERO);

    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.0     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.0     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.1     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.1     ) );
    TEST_ASSERT_EQUAL_FP64( +0.0   , math_to_integer( +0.9     ) );
    TEST_ASSERT_EQUAL_FP64( -0.0   , math_to_integer( -0.9     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.0     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.0     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.1     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.0   , math_to_integer( +1.9     ) );
    TEST_ASSERT_EQUAL_FP64( -1.0   , math_to_integer( -1.9     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.0     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.0     ) );
    TEST_ASSERT_EQUAL_FP64( +2.0   , math_to_integer( +2.1     ) );
    TEST_ASSERT_EQUAL_FP64( -2.0   , math_to_integer( -2.1     ) );
    TEST_ASSERT_EQUAL_FP64( +1.e20 , math_to_integer( +1.e20   ) );
    TEST_ASSERT_EQUAL_FP64( -1.e20 , math_to_integer( -1.e20   ) );

    TEST_ASSERT_EQUAL_FP64( +4503599627370495.0 , math_to_integer( +4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370496.0 , math_to_integer( +4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( +4503599627370497.0 , math_to_integer( +4503599627370497.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370495.0 , math_to_integer( -4503599627370495.5 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370496.0 , math_to_integer( -4503599627370496.0 ) );
    TEST_ASSERT_EQUAL_FP64( -4503599627370497.0 , math_to_integer( -4503599627370497.0 ) );

    TEST_RESULTS;

    return ( 0 );
}