#include "../math/src/math_primitives.c"
#include "../math/src/math_convert.c"
#include <string.h>
#include <assert.h>

int main( void )
{
    assert( strcmp( "0.1000000000000000055511151231257827021181583404541015625", math_print(0.1) ) == 0 );
    assert( strcmp( "1.", math_print(1.0) ) == 0 );
    assert( strcmp( "1.5", math_print(1.5) ) == 0 );
    assert( strcmp( "+inf", math_print(1./0.) ) == 0 );
    assert( strcmp( "nan", math_print(0./0.) ) == 0 );

    return ( 0 );
}