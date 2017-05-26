//===--- Intel_Libmvec.td - Defines all Libmvec variants -*- tablegen -*---===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the classes used to represent the set of Libmvec variants.
/// The mangled names emitted by TableGen follow the gcc vector function ABI
/// found here: https://sourceware.org/glibc/wiki/libmvec?action=AttachFile&do=
///             view&target=VectorABI.txt
///
/// Supported variants can be found here:
/// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/x86_64/fpu/Versions
///
/// To date, this includes the following definitions:
///
/// libmvec {
///   GLIBC_2.22 {
///     _ZGVbN2v_cos; _ZGVcN4v_cos; _ZGVdN4v_cos; _ZGVeN8v_cos;
///     _ZGVbN2v_sin; _ZGVcN4v_sin; _ZGVdN4v_sin; _ZGVeN8v_sin;
///     _ZGVbN2vvv_sincos; _ZGVcN4vvv_sincos; _ZGVdN4vvv_sincos;
///     _ZGVeN8vvv_sincos;
///     _ZGVbN2v_log; _ZGVcN4v_log; _ZGVdN4v_log; _ZGVeN8v_log;
///     _ZGVbN2v_exp; _ZGVcN4v_exp; _ZGVdN4v_exp; _ZGVeN8v_exp;
///     _ZGVbN2vv_pow; _ZGVcN4vv_pow; _ZGVdN4vv_pow; _ZGVeN8vv_pow;
///     _ZGVbN4v_cosf; _ZGVcN8v_cosf; _ZGVdN8v_cosf; _ZGVeN16v_cosf;
///     _ZGVbN4v_sinf; _ZGVcN8v_sinf; _ZGVdN8v_sinf; _ZGVeN16v_sinf;
///     _ZGVbN4v_logf; _ZGVcN8v_logf; _ZGVdN8v_logf; _ZGVeN16v_logf;
///     _ZGVbN4v_expf; _ZGVcN8v_expf; _ZGVdN8v_expf; _ZGVeN16v_expf;
///     _ZGVbN4vv_powf; _ZGVcN8vv_powf; _ZGVdN8vv_powf; _ZGVeN16vv_powf;
///     _ZGVbN4vvv_sincosf; _ZGVcN8vvv_sincosf; _ZGVdN8vvv_sincosf;
///     _ZGVeN16vvv_sincosf;
///   }
/// }
///
// ===--------------------------------------------------------------------=== //

// Libmvec functions are encoded with the types of parameters they take, scalar
// or vector. This is needed for proper name mangling. E.g., cos non-masked
// variant with VF=4 for 128-bit target vector register would be mangled as
// "_ZGVbN4v_cos". The single v represents a single vector parameter. Using the
// parameter signature, a non-masked/masked variant for each target ISA will be
// generated for each function.

// As of now, libmvec only supports the following six functions. Additional
// functions can be supported in the future by adding new def entries.

class ArgType<string type> {
  string Type = type;
}

def Scalar : ArgType<"s">;
def Vector : ArgType<"v">;

class LibmvecVariant<list<ArgType> Args> {
  list<ArgType> ArgList = Args;
}

def sin    : LibmvecVariant<[Vector]>;
def cos    : LibmvecVariant<[Vector]>;
def sincos : LibmvecVariant<[Vector, Vector, Vector]>;
def pow    : LibmvecVariant<[Vector, Vector]>;
def exp    : LibmvecVariant<[Vector]>;
def log    : LibmvecVariant<[Vector]>;
