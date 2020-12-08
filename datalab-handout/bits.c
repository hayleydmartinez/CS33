/* 
 * CS:APP Data Lab 
 * 
 * Hayley Martinez - 104926567
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
/* If x is Tmax (011...1), it will overflow to
 * Tmin (100...0). We then check using bitwise
 * or to see if it should come out to 111...1.
 * If it does, that means that x is either -1
 * or Tmax. Then we check whether it's the max
 * value AND it is not -1. 
 */
  int ovflow = x + 1;
  int notmax = ~(x ^ ovflow);
  return !notmax & !!(~x);
}

/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
/* We use De Morgan's laws, which states that:
 *   not (A and B) = not A or not B
 */
  int or_bit = ~(~x | ~y);
  return or_bit;
}

/* 
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int copyLSB(int x) {
/* We shift left so that the only thing left
 * is the LSB. We then arithmetically shift
 * right which copies the LSB (now the MSB) to
 * all 31 other bits.
 */
  int save_bit = x << 31;
  int copy_bit = save_bit >> 31;
  return copy_bit;
}

/* 
 * anyEvenBit - return 1 if any even-numbered bit in word set to 1
 *   Examples anyEvenBit(0xA) = 0, anyEvenBit(0xE) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyEvenBit(int x) {
/* We make several bit masks of form 1010101
 * to mask all our odd bits, then we add them
 * together to make a giant bit mask. We then
 * use the and operator to see whether there
 * are any 1s in the even spaces of the byte.
 */
  int mask_bit0 = 85;
  int mask_bit1 = mask_bit0 << 8;
  int mask_bit2 = mask_bit0 << 16;
  int mask_bit3 = mask_bit0 << 24;
  int test_bit = (mask_bit0 + mask_bit1 + mask_bit2 + mask_bit3);
  int any_even = !!(x & test_bit);
  return any_even;
}

/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
/* We know that only 0x00000000 will return 1,
 * so we use the fact that 0x11111111 is -1 to
 * check whether x is 0. If it is zero is_zero
 * will evaluate to 0x00000000. We isolate the
 * MSB by shifting both to the right. Only 0
 * will produce 0, anything else will produce
 * 0x11111111. So, we then add 1 and return.
 */
  int is_zero = (~x + 1);
  return ((x >> 31) | (is_zero >> 31)) + 1;
}

/* 
 * rempwr2 - Compute x%(2^n), for 0 <= n <= 30
 *   Negative arguments should yield negative remainders
 *   Examples: rempwr2(15,2) = 3, rempwr2(-35,3) = -3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int rempwr2(int x, int n) {
/* We create a bitmask to cover everything
 * except for the n rightmost bits, which is
 * our remainder. We then convert everything
 * to positive in order to calculate the mod
 * by and-ing our positive modulus and our
 * bitmask. Then, we make a special case for
 * negative bits by converting to Two's Comp.
 * Then, we use the or operator to choose
 * either the positive or negative modulus by
 * masking the opposite solution by our sign
 * mask.
 */
  int rm_bits = (1 << n) + (~1 + 1);
  int s_mask = x >> 31;
  int is_neg = !!s_mask;
  int to_pos = (x + (~is_neg + 1)) ^ s_mask;
  int pos_mod = to_pos & rm_bits;
  int neg_mod = ~pos_mod + 1;
  return (~s_mask & pos_mod) | (s_mask & neg_mod);
}
/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
/* We know from our textbook that we can shift
 * by powers of 2 to multiply and divide, and
 * need to round towards zero when dividing,
 * the formula for which we are given in the 
 * book. Thus, since 5 is represented by 101
 * and 8 is represented by 1000, we can times
 * x by splitting our left shifts. We also use
 * is_neg to determine whether to weight our 
 * right shift when dividing by 8.
 */
  int is_neg = !!(x >> 31);
  int mult_5 = (x << 2) + x;
  int divd_8 = (mult_5 + (is_neg << 3) + (~is_neg + 1)) >> 3;
  return divd_8;
}

/* 
 * tc2sm - Convert from two's complement to sign-magnitude 
 *   where the MSB is the sign bit
 *   You can assume that x > TMin
 *   Example: tc2sm(-5) = 0x80000005.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int tc2sm(int x) {
/* If it's positive, we change nothing. If it
 * is negative, we need to add one and invert
 * the number (since negatives are -2^w + the
 * rest of the regular 2^x entries) and then
 * add in the sign. By basing all of this on
 * our values s_mask and is_neg, which are 0
 * if x is positive, doing these operations
 * will not change positive numbers and only
 * change negative numbers.
 */
  int s_mask = x >> 31;
  int is_neg = !!s_mask;
  int flipit = ((x + (~is_neg + 1)) ^ s_mask) + (is_neg << 31);
  return flipit;
}
