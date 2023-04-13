// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// Basic tablegen type classes and defs for .ll builtin generation

// Abstract type
class Type<string name = ""> {
  string Name = name;
}

// Abstract value
class Value<Type type, string name> {
  Type Ty = type;
  string Name = "%" # name;
}

// Opaque struct type
// e.g. %opencl.image2d_ro_t = type opaque
class OpaqueStructType<string name> : Type {
  let Name = "%" # name;
  string RawName = name;
}

// Helper: mapping values to types
class mapValueToType<list<Value> values> {
  list<Type> ret = !foreach(val, values, val.Ty);
}

class IntType<int bitwidth> : Type {
  let Name = "i" # bitwidth;
  int BitWidth = bitwidth;
}

class FloatType<int bitwidth> : Type {
  let Name = !cond(!eq(bitwidth, 16) : "half",
                   !eq(bitwidth, 32) : "float",
                   !eq(bitwidth, 64) : "double");
  int BitWidth = bitwidth;
}

// Abstract vector type
class VectorType<Type element_type, int length>: Type {
  assert !ge(length, 1), "Invalid vector length: " # length;

  // e.g. <16 x i32>
  let Name = "<" # length # " x " # element_type.Name # ">";

  Type ElementType = element_type;
  int Len = length;

  // Mangling
  string ManglePrefix = "Dv" # length # "_";
}

// Addrspace suffix of pointer type
class Addrspace<int val> {
  assert !and(!ge(val, 0), !le(val, 4)), "Invalid addrspace: " # val;
  string Name = " addrspace(" # val # ")";
  int Val = val;
  string Alias = "AS" # val;
}

// Abstract pointer type
class PointerType<Type pointee_type, Addrspace as>: Type {
  let Name = pointee_type.Name # as.Name # "*";

  Type PointeeType = pointee_type;
  int Addrspace = as.Val;

  // Mangling
  string ManglePrefix = "P" # !if(!empty(as.Alias), "", "U3" # as.Alias);
}

// void type
def void : Type<"void">;

// All scalar types
defset list<Type> all_scalar_types = {
  // Integer types: bool, char, short, int, long
  // e.g. def name of char: i8
  defset list<IntType> scalar_int_types = {
    foreach bitwidth = [1, 4, 8, 16, 32, 64, 128] in {
      def i # bitwidth : IntType<bitwidth>;
    }
  }

  // Float types: half, float, double
  // e.g. def name of half: f16
  defset list<FloatType> scalar_float_types = {
    foreach bitwidth = [16, 32, 64] in {
      def f # bitwidth : FloatType<bitwidth>;
    }
  }
}

// All vector types
// e.g. def name of <16 x i32>: v16i32
defset list<VectorType> all_vector_types = {
  // Generates <1 x T> ... <1024 x T>
  foreach len = 1...1024 in {
    foreach scalar_type = all_scalar_types in {
      def v # len # scalar_type : VectorType<scalar_type, len>;
    }
  }
}

// All pointer addrspaces
defset list<Addrspace> all_addrspaces = {
  def NoneAS : Addrspace<0> { let Name = ""; let Alias = ""; }
  def GlobalAS : Addrspace<1>;
  def LocalAS : Addrspace<2>;
  def PrivateAS : Addrspace<3>;
  def GenericAS : Addrspace<4>;
}

// All pointer types
// e.g. def name of <4 x i32> addrspace(1)*: pAS1v4i32
// addrspace 0 alias in the name is omitted.
// e.g. def name of <4 x i32>*: pv4i32
defset list<PointerType> all_pointer_types = {
  foreach as = all_addrspaces in {
    foreach pointee_type = all_scalar_types # all_vector_types in {
      def p # as.Alias # pointee_type : PointerType<pointee_type, as>;
    }
  }
}

// Type mangling
class MangleType<Type type, bit signed = true> {
  string ret = !cond(!isa<IntType>(type) : !cond(!eq(!cast<IntType>(type).BitWidth, 1) : "b",
                                                 !eq(!cast<IntType>(type).BitWidth, 8) : !if(signed, "c", "h"),
                                                 !eq(!cast<IntType>(type).BitWidth, 16) : !if(signed, "s", "t"),
                                                 !eq(!cast<IntType>(type).BitWidth, 32) : !if(signed, "i", "j"),
                                                 !eq(!cast<IntType>(type).BitWidth, 64) : !if(signed, "l", "m")),
                     !isa<FloatType>(type) : !cond(!eq(!cast<FloatType>(type).Name, "half") : "Dh",
                                                   !eq(!cast<FloatType>(type).Name, "float") : "f",
                                                   !eq(!cast<FloatType>(type).Name, "double") : "d"));
}

class MangleVectorType<VectorType type, bit signed = true> {
  string ret = type.ManglePrefix # MangleType<type.ElementType, signed>.ret;
}

class ManglePointerType<PointerType type, bit signed = true, string mangle_suffix = ""> {
  string ret = type.ManglePrefix # mangle_suffix # MangleType<type.PointeeType, signed>.ret;
}

class MangleVectorOfPointersType<VectorType type, bit signed = true, string mangle_suffix = ""> {
  string ret = type.ManglePrefix # ManglePointerType<!cast<PointerType>(type.ElementType), signed, mangle_suffix>.ret;
}

include "GENERIC/td-helper.td"
