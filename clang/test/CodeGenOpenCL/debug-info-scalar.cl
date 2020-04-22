// RUN: %clang %s -o - -emit-llvm -g -S -gintel-opencl-builtin-types \
// RUN:   | FileCheck --check-prefixes=CHECK,TRUE %s
// RUN: %clang %s -o - -emit-llvm -g -S -gno-intel-opencl-builtin-types \
// RUN:   | FileCheck --check-prefixes=CHECK,FALSE %s
// RUN: %clang %s -o - -emit-llvm -g -S \
// RUN:   | FileCheck %s --check-prefixes=CHECK,FALSE

// These scalar type definitions currently exist in opencl-c-common.h:
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// These scalar type definitions currently exist in opencl-c-platform.h:
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

// Validate debug information emitted for OpenCL scalar data types.
//
void foo() {
  bool b;
// CHECK: !DILocalVariable(name: "b",{{.*}} type: [[B:![0-9]+]])
// CHECK: [[B]] = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)

  char c;
// CHECK: !DILocalVariable(name: "c",{{.*}} type: [[C:![0-9]+]])
// CHECK: [[C]] = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)

  unsigned char unc;
// CHECK: !DILocalVariable(name: "unc",{{.*}} type: [[UNC:![0-9]+]])
// CHECK: [[UNC]] = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)

  uchar uc;
// CHECK: !DILocalVariable(name: "uc",{{.*}} type: [[UC:![0-9]+]])
// TRUE:  [[UC]] = !DIBasicType(name: "uchar", size: 8, encoding: DW_ATE_unsigned_char)
// FALSE: [[UC]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar", {{.*}}baseType: [[UNC]])

  short s;
// CHECK: !DILocalVariable(name: "s",{{.*}} type: [[S:![0-9]+]])
// CHECK: [[S]] = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)

  unsigned short uns;
// CHECK: !DILocalVariable(name: "uns",{{.*}} type: [[UNS:![0-9]+]])
// CHECK: [[UNS]] = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)

  ushort us;
// CHECK: !DILocalVariable(name: "us",{{.*}} type: [[US:![0-9]+]])
// TRUE:  [[US]] = !DIBasicType(name: "ushort", size: 16, encoding: DW_ATE_unsigned)
// FALSE: [[US]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort", {{.*}}baseType: [[UNS]])

  int i;
// CHECK: !DILocalVariable(name: "i",{{.*}} type: [[I:![0-9]+]])
// CHECK: [[I]] = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)

  unsigned int uni;
// CHECK: !DILocalVariable(name: "uni",{{.*}} type: [[UNI:![0-9]+]])
// CHECK: [[UNI]] = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)

  uint ui;
// CHECK: !DILocalVariable(name: "ui",{{.*}} type: [[UI:![0-9]+]])
// TRUE:  [[UI]] = !DIBasicType(name: "uint", size: 32, encoding: DW_ATE_unsigned)
// FALSE: [[UI]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint",{{.*}}baseType: [[UNI]])

  long l;
// CHECK: !DILocalVariable(name: "l",{{.*}} type: [[L:![0-9]+]])
// CHECK: [[L]] = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)

  unsigned long unl;
// CHECK: !DILocalVariable(name: "unl",{{.*}} type: [[UNL:![0-9]+]])
// CHECK: [[UNL]] = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)

  ulong ul;
// CHECK: !DILocalVariable(name: "ul",{{.*}} type: [[UL:![0-9]+]])
// TRUE:  [[UL]] = !DIBasicType(name: "ulong", size: 64, encoding: DW_ATE_unsigned)
// FALSE: [[UL]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong", {{.*}}baseType: [[UNL]])

  float f;
// CHECK: !DILocalVariable(name: "f",{{.*}} type: [[F:![0-9]+]])
// CHECK: [[F]] = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)

  size_t sz;
// CHECK: !DILocalVariable(name: "sz",{{.*}} type: [[SZ:![0-9]+]])
// TRUE:  [[SZ]] = !DIBasicType(name: "size_t", size: {{32|64}}, encoding: DW_ATE_unsigned)
// FALSE: [[SZ]] = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", {{.*}}baseType: {{.*}})

  ptrdiff_t pd;
// CHECK: !DILocalVariable(name: "pd",{{.*}} type: [[PD:![0-9]+]])
// TRUE:  [[PD]] = !DIBasicType(name: "ptrdiff_t", size: {{32|64}}, encoding: DW_ATE_signed)
// FALSE: [[PD]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ptrdiff_t", {{.*}}baseType: {{.*}})

  intptr_t pi;
// CHECK: !DILocalVariable(name: "pi",{{.*}} type: [[PI:![0-9]+]])
// TRUE:  [[PI]] = !DIBasicType(name: "intptr_t", size: {{32|64}}, encoding: DW_ATE_signed)
// FALSE: [[PI]] = !DIDerivedType(tag: DW_TAG_typedef, name: "intptr_t", {{.*}}baseType: {{.*}})

  uintptr_t pui;
// CHECK: !DILocalVariable(name: "pui",{{.*}} type: [[PUI:![0-9]+]])
// TRUE:  [[PUI]] = !DIBasicType(name: "uintptr_t", size: {{32|64}}, encoding: DW_ATE_unsigned)
// FALSE: [[PUI]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uintptr_t", {{.*}}baseType: {{.*}})

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
  double d;
// CHECK: !DILocalVariable(name: "d",{{.*}} type: [[D:![0-9]+]])
// CHECK: [[D]] = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)

#pragma OPENCL EXTENSION cl_khr_fp16 : enable
  half h;
// CHECK: !DILocalVariable(name: "h",{{.*}} type: [[H:![0-9]+]])
// CHECK: [[H]] = !DIBasicType(name: "half", size: 16, encoding: DW_ATE_float)
}

