# [PROPOSAL] pass normalized values to _impl functions

Instead of letign functions to perform the normalization inside the implementation, pass the xn directly computed in the switch case.

# [FEATURE] develop a print function to write doubles with exact number of decimals

Given a fp representation `x=m·2^(n-p+1)` obtain the `radix-10` representation `x=m'·10^n'` and print it exactly.

Remember the relation: `a^b = c^w`, with `w=n'+g`, and `1/n' <= g < 1`, thus: `x=10^g·10^n'`.

Naive example with `m=0`: `2^5 = 10^0.50514997832 · 10^1 = 32E+1`

# [FEATURE] add timer to measure time per call during ragen testing

The time will play right before math function to measure is called and stopped right after.
Measures and added together and divided by the number of calls, to obtain the mean value.

# [ISSUE] math_primitives - math_cwnormalize

__DESCRIPTION__

This function is fully capable to process `FLOATS` but not `DOUBLES`.
This is because, __gradual underflow__ is not handled correctly.

If a gradual underflow of type float enters in this function it will
promoted to a finite double hence there is no issue with that.
But gradual underflow of type double will hit the issue.

__SCOPE__

Functions that depends upon `math_cwnormalize` should be aware that they can't process `x` inputs of gradual underflow of type `DOUBLES`.
The list of functions using the normalization are:

-   math_sqrt: __WRONG-USE__ when `x` is `FINITE` then normalize directly
-   math_pow:  __WRONG-USE__ when `x` is `FINITE` then call to `math_pow_imp` which uses normalization directly.
-   math_log:  __WRONG-USE__ when `x` is `FINITE` then call to `math_log_imp_cody` which uses normalization directly.

__PROPOSAL SOLUTION__

Modify the elementary functions interfaces, a.k.a `math_sqrt`, `math_log`, `math_exp`, `math_sin`, `math_cos`, `math_asin`, `math_pow` to receive inputs for `x` of type `f32_t`.
Under this modification all external values that are considered gradual underflow in the out-side world of the mathematical module, are converted to
finite values inside.

# [ISSUE] math elementary functions interface

The external functions of the module, which are the ones performing the desired elementary functions, have set the wrong input type.
These function are not demonstrated full capability to operate with `DOUBLE`, hence the interface should be changed to `FLOAT` in all inputs and return type.

__PROPOSAL SOLUTION__

Add this macro as intermediate step during implementing the final solution, which will change all the interfaces.
To perform the final change, more study have to be conducted in order to __determine if double to float casting is always safe__.

```c
#define FP_CAST(fun,x,...) (float)(fun)((float) x, (float) __VA_ARGS__ )
```

# [ISSUE] math_primitives - math_cwsetexp

__SCOPE__

This is an analysis of what are the expected inputs of exponent `n` in te function `math_cwsetexp`.
In summary, expected inputs are the range of `DOUBLE [emin, emax] = [-1022, +1023]`

_exp_

```c
    math_cwsetexp(2.0, 1+xn); /* whith 1+xn in (0, 1025], according to: xn = (i16_t)(x*aln2 + 0.5)  */
```

_sqrt_

[IMPORTANT]

current values are `|xn.e| < 514` which is the output given by this call xn = math_cwnormalize( x ),
but the function of `math_cwnormalize` to compute exponent is not tested and there are hints of issues in the result.

```c
    math_cwsetexp(1., xn.e); /* whith -1022 <= xn.e <= +1023 */
```

_pow_

The current implementation uses `math_cwsetexp` as follows, but this will change in following verions of the software,
hence, expected input is not analyzed.

```c
    math_cwsetexp(z, zexp);
```

# [ISSUE] math_primitives - math_cwnormalize

Testing exponent return field: .e

# [ISSUE] math_pow

__SUMMARY__

To much dependencies of math_pow which will increase testing complexities.

__DESCRIPTION__

If pow definition is applied as defined in `FORTRAN` (see Cody and Waite - 1980).

`pow(x,y) := B**(y*logB(x))`

Depedencies:

```c
#include "../src/math_log.c"
#include "../src/math_exp.c"
```

__PROPOSAL SOLUTION__

These two movements, will remove external dependencies:

- Decomision `log.c`
- Choose `B=2`

Doing that, we can self-contain all `pow` dependencies inside its own source file.

# [ISSUE] math_intrnd

Is tested until x ~10E+14, it does not work below this limit, and this might be the root cause to the math_sin(x) and math_cos(x) limit.
Pending to analyze this limit and its impact better.
