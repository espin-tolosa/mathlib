# Proposal:

Report date: 2025-01-19 9:34

## Req 1.

Nowadays the accept criteria of range analyzer is not in used.

The idea, is use both: `accept`, and `reject`.

The accept is going to be used like the reject is used today, as a threshold to start recording in the histogram.
The reject is going to be used as range analysis terminator.

If the range analysis stops by rejection, it prints the n-uple: `x_prev, x_curr, y_lhs, y_rhs, curr_ulp`.
If the range analysis reaches the acception criteria, it counts the sample as usual, but also records the y_lhs.

Storing the records in the histogram:

We have a hash which stores the sign and the exponent of that number, so we only need to store fractional bits.
We will store up to 10 samples, prioritizing the highest ulps, and discarding the rest.

## Req 2.

The library shall create a report file with the content of Req 1., and also identifiyin

- the commit of the math library under test
- the date of test finalization

## Req 3.

Function `fp64_get_triplet( x )` working properly with subnormal values.

As the `e` field of `fptriplet_t` is limited to `emin, emax` add a subexponent field, wich will be set the real exponent value.

Test this feature using this call:

`fptriplet_t t = fp64_get_triplet( fp64_get_named_fp_in_real_line( NAMED_FP_MINSUBN ) );`

## Req 4.

The application shall respond to signal `Ctrl-C` and stop the range analyzer and display the histogram upto the value it was analyzed when the signal was sent.

## Req 5.

THe range analyzer shall accept a callback function to script actions inside the main loop analysis, such as print values that are rejected.

---

# Test Library Requirements

This is the specification of the `fp_test` library.
The requirements of this software are treated differently from a software system.

1. `fp_test` shall perform range test giving the following results:

    - Report date
    - Histogram by range
    - Y ranges

Report date: 2024-12-30 17:24

[math_sqrt] MAXFINITE_ERR 2.22044604925031308e-16, MAXERR 2.22044604925031308e-16 at 5.32493416443430487e-44
MEAN ERR 3.96173944761102348e-08 in [-3.40282346638528860e+38, 3.40282346638528860e+38]

Relative Error Histogram of 4278190082 points
-------------------------------------------
|err| == ZERO: 95.582%
|err| <= E-15: 4.418e+00%
[INFO] Running test log ranges for math_sqrt

 math_sqrt [-inf , inf], spec-v2 format: [%+.17e (0x%08X), %+.17e (0x%08X)] -> [%+.17e, %+.17e] %s
=============================================================================================================================
[-inf (0xFF800000), -1.40129846432481707e-45 (0x80000001)] -> [+nan, +nan] NIL
[-0.00000000000000000e+00 (0x80000000), +0.00000000000000000e+00 (0x00000000)] -> [-0.00000000000000000e+00, +0.00000000000000000e+00] ZERO
[+1.40129846432481707e-45 (0x00000001), +3.40282346638528860e+38 (0x7F7FFFFF)] -> [+3.74339206650921624e-23, +1.84467429741979238e+19] FINITE
[+inf (0x7F800000), +inf (0x7F800000)] -> [+inf, +inf] INF
