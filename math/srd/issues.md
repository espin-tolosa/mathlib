# Issues Log

## fp_test library

- Not clear if x in ` - SUBNORMAL` is tested, e.g.: the math_sqrt report does not reflect it.

## math_sqrt

- 1. In range ` + SUBNORMAL` returns 0 instead of compute the real value. [FIXED]

## math_exp

- 1. In range x in `-2E^+9, -2E^+10` rejected by ulp > 8

## math_log

- 1. In range x in ` - SUBNORMAL` returns `INFINITE` [FIXED]
- 2. In range x in ` + SUBNORMAL` returns `INFINITE` [FIXED]

## math_sin

- 1. In range x in `[-2E^+1, -2E^+2)` with ULP > 8.0 (for 2004/700650 points)
- 2. In range x in `[+2E^+1, +2E^+2)` with ULP > 8.0 (for 2004/702150 points)

# math_cos

- 1. In range:

        2/  700650 point in [-2E^   +1, -2E^   +2) with ULP > 8.0
     2002/  699150 point in [-2E^   +0, -2E^   +1) with ULP > 8.0
     2002/  700150 point in [+2E^   +0, +2E^   +1) with ULP > 8.0
        2/  702150 point in [+2E^   +1, +2E^   +2) with ULP > 8.0
        1/  693147 point in [+2E^  +12, +2E^  +13) with ULP > 8.0

## math_asin

- All OK with ULP 8

## math_pow

- 1. pow(x, 3.75):

```
     1000/    1000 point in             -subnormal with ULP > 8.0
        1/       1 point in                     -0 with ULP > 8.0
     1000/    1000 point in             +subnormal with ULP > 8.0

   462099/  693148 point in [+2E^ -287, +2E^ -286) with ULP > 8.0
   693147/  693147 point in [+2E^ -286, +2E^ -285) with ULP > 8.0
   693148/  693148 point in [+2E^ -285, +2E^ -284) with ULP > 8.0
   693147/  693147 point in [+2E^ -284, +2E^ -283) with ULP > 8.0
   693148/  693148 point in [+2E^ -283, +2E^ -282) with ULP > 8.0
   693147/  693147 point in [+2E^ -282, +2E^ -281) with ULP > 8.0
   693148/  693148 point in [+2E^ -281, +2E^ -280) with ULP > 8.0
   693147/  693147 point in [+2E^ -280, +2E^ -279) with ULP > 8.0
   693148/  693148 point in [+2E^ -279, +2E^ -278) with ULP > 8.0
   693148/  693148 point in [+2E^ -278, +2E^ -277) with ULP > 8.0
   693147/  693147 point in [+2E^ -277, +2E^ -276) with ULP > 8.0
   693148/  693148 point in [+2E^ -276, +2E^ -275) with ULP > 8.0
   693147/  693147 point in [+2E^ -275, +2E^ -274) with ULP > 8.0
   693148/  693148 point in [+2E^ -274, +2E^ -273) with ULP > 8.0
   346573/  693147 point in [+2E^ -273, +2E^ -272) with ULP > 8.0
```
