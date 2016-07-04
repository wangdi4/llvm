//===------ Intel_SVML.td - Defines all SVML variants -*- tablegen -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the classes used to represent the set of SVML variants.
///
// ===--------------------------------------------------------------------=== //

class SvmlVariant;

def sin : SvmlVariant;
def cos : SvmlVariant;
def pow : SvmlVariant;
def exp : SvmlVariant;
def log : SvmlVariant;
def sincos : SvmlVariant;
def acos : SvmlVariant;
def acosh : SvmlVariant;
def asin : SvmlVariant;
def asinh : SvmlVariant;
def atan2 : SvmlVariant;
def atan : SvmlVariant;
def atanh : SvmlVariant;
def cbrt : SvmlVariant;
def cdfnorm : SvmlVariant;
def cdfnorminv : SvmlVariant;
def ceil : SvmlVariant;
def cosd : SvmlVariant;
def cosh : SvmlVariant;
def erf : SvmlVariant;
def erfc : SvmlVariant;
def erfcinv : SvmlVariant;
def erfinv : SvmlVariant;
def exp10 : SvmlVariant;
def exp2 : SvmlVariant;
def expm1 : SvmlVariant;
def floor : SvmlVariant;
def fmod : SvmlVariant;
def hypot : SvmlVariant;
def invsqrt : SvmlVariant;
def log10 : SvmlVariant;
def log1p : SvmlVariant;
def log2 : SvmlVariant;
def logb : SvmlVariant;
def nearbyint : SvmlVariant;
def rint : SvmlVariant;
def round : SvmlVariant;
def sind : SvmlVariant;
def sinh : SvmlVariant;
def sqrt : SvmlVariant;
def tan : SvmlVariant;
def tanh : SvmlVariant;
def trunc : SvmlVariant;
