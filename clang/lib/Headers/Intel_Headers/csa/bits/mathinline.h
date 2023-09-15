/* file: mathinline.h */

/*
** Copyright  (C) 1985 Intel Corporation. All rights reserved.
**
** The information and source code contained herein is the exclusive property
** of Intel Corporation and may not be disclosed, examined, or reproduced in
** whole or in part without explicit written authorization from the Company.
**
*/

__extern_inline int __signbitf (float __x)
{
  return (__x < 0);
}

__extern_inline int __signbit (double __x)
{
  return (__x < 0);
}

__extern_inline int __signbitl (long double __x)
{
  return (__x < 0);
}
