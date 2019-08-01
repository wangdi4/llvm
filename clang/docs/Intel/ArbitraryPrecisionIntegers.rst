============================
Arbitrary Precision Integers
============================

Introduction
============

Since FPGAs have the ability to produce non\-power\-of\-2 math units, there is
a requirement to have an integer type that supports an arbitrary compile\-time
bit representation. These need to be expressed in IR as the correct precision
integer types \(such as i3, i46, etc.\).

Implementation details
======================

The syntax for declaring an integer with an arbitrary number of bits is via the
attribute \_\_ap\_int\(N\), which is usable only on typedef and using clauses::

  // version 1
  typedef int int3_tt __attribute__((__ap_int(3)));

  // version 2 (dependent)
  template <unsigned Size>
  using MyInt = int __attribute__((__ap_int(Size)));

The typedef inherits its 'signedness' from the 'underlying type'. Otherwise,
this type has no effect on the AP type.

Supported operators
===================

All integer operations are currently supported. This includes:

* Arithmetic operators: \+ \- \* /
* Bitwise operators: % | & ^ >> << ~
* Casting operators: \(bool\) \(char\) \(short\) \(int\) \(long\)
* Compound assignment operators: \+= \-= \*= /= %= \|= &= ^= >>= <<=
* Increment and decrement operators: x\+\+ x\-\- \+\+x \-\-x
* Miscellaneous operators: = \+x \-x \!x sizeof\(\) &x \*x
* New and delete operators: new x delete x
* Relational operators: == \!= > < >= <=

Size and alignment
==================

For an AP type with a datatype size of N bits:

* The storage size is the next byte \(>= N\)
* The alignment is the next power\-of\-2 \(>= N\) up to a maximum alignment
  of 8 bytes for both SPIR64 and x86 targets. This is because LLVM expects a
  maximum alignment of 8 bytes for any integer type.

Overflow
========

* For signed addition, subtraction, and multiplication from the user source,
  Clang generates nsw \(no signed wrap\) meaning that the result value is
  undefined if there is overflow. For instance, a signed addition may be
  represented as::

    add nsw i17 %[[OPERAND1]], %[[OPERAND2]]

* For unsigned operations, overflow behavior is well\-defined, and nuw \(no
  unsigned wrap\) is not added. For instance, an unsigned addition may be
  represented as::

    add i17 %[[OPERAND1]], %[[OPERAND2]]

Shifts
======

* Shifting an AP type by a negative amount

  * For OpenCL, Clang ensures that the shift amount is within bounds (0 <
    shift\_amount < N) so that we don’t end up generating a poison value.
    Currently, shift amount is determined with a urem operation:
    \(\(ap\_uint<ap\_int\_size>\) shift\_amount\) % ap\_int\_size

  * For example, an i46 type << \-42 will be equivalent to the i46 type <<
    \(\-42 urem 46\), which is equivalent to the i46 type << 8

  * For HLS, this results in undefined behavior, possibly resulting in a
    poison value from Clang.

* Shifting an AP type by an amount that is larger than or equal to its size

  * For OpenCL, Clang ensures that the shift amount is within bounds (0 <
    shift\_amount < N) so that we don’t end up generating a poison value.
    Currently, the shift amount is determined by masking when we have
    power\-of\-2 datatype size, and performing a urem operation otherwise.

    * For example, an i46 type << 47 will be equivalent to the i46 type <<
      \(47 urem 46\). This is equivalent to the i46 type << 1
    * For example, an i4 type << 5 will be equivalent to the i4 type <<
      \(5 & 3\). This is equivalent to the i4 type << 1

  * For HLS, this results in undefined behavior, possibly resulting in a poison
    value from Clang

Conversions and promotions
==========================

* For conversions where the result is undefined due to the value not fitting,
  the backend should drop the bits that will not fit
* Any operation with AP types will result in an AP type
* For operations with two operands of different types, the larger type takes
  precedence. Note that according to C specifications, an unsigned type is
  considered larger than a signed type of the same width.

Literals
========

* Currently, users are encouraged to use explicit casts on literals in AP
  integer expressions. Essentially, literals are given the type they normally
  would without the AP types.

  * '<literal>' is interpreted as a signed 32 bit integer
  * '<literal>U' is interpreted as an unsigned 32 bit integer
  * '<literal>L' is interpreted as a signed 64 bit integer in OpenCL and on
    Linux in HLS, and as a signed 32 bit integer on Windows in HLS
  * '<literal>UL' is interpreted as an unsigned 64 bit integer in OpenCL and
    on Linux in HLS, and an unsigned 32 bit integer on Windows in HLS
  * '<literal>LL' is interpreted as a signed 128 bit integer in OpenCL and a
    signed 64 bit integer in HLS
  * '<literal>ULL' is interpreted as an unsigned 128 bit integer in OpenCL and
    an unsigned 64 bit integer in HLS
