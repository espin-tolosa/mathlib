#include <stdio.h>
#include "fptest.h"

#include "../math/src/math_primitives.c"
#include "../math/src/math_arithmetic.c"

int main ( void )
{
    TEST_INIT;

    printf("%d, %d\n", int_add( 142, 124 ) , 142*124 );

    TEST_RESULTS;

    return ( 0 );
}