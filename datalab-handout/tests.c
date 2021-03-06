/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

int test_isTmax(int x) {
    return x == 0x7FFFFFFF;
}
int test_bitAnd(int x, int y)
{
  return x&y;
}
int test_copyLSB(int x)
{
  return (x & 0x1) ? -1 : 0;
}
int test_anyEvenBit(int x) {
  int i;
  for (i = 0; i < 32; i+=2)
      if (x & (1<<i))
   return 1;
  return 0;
}
int test_logicalNeg(int x)
{
  return !x;
}
int test_rempwr2(int x, int n)
{
    int p2n = 1<<n;
    return x%p2n;
}





int test_multFiveEighths(int x)
{
  return (x*5)/8;
}
int test_tc2sm(int x) {
  int sign = x < 0;
  int mag = x < 0 ? -x : x;
  return (sign << 31) | mag;
}
