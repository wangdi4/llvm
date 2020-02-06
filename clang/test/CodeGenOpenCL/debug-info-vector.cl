// RUN: %clang %s -o - -emit-llvm -S -g -gintel-opencl-builtin-types \
// RUN:   | FileCheck --check-prefixes=CHECK,TRUE %s
// RUN: %clang %s -o - -emit-llvm -S -g -gno-intel-opencl-builtin-types \
// RUN:   | FileCheck --check-prefixes=CHECK,FALSE %s
// RUN: %clang %s -o - -emit-llvm -S -g \
// RUN:   | FileCheck --check-prefixes=CHECK,FALSE %s

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// These scalar type definitions currently exist in opencl-c-common.h:
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// Vector type definitions from opencl-c-common.h:
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef char char8 __attribute__((ext_vector_type(8)));
typedef char char16 __attribute__((ext_vector_type(16)));
typedef uchar uchar2 __attribute__((ext_vector_type(2)));
typedef uchar uchar3 __attribute__((ext_vector_type(3)));
typedef uchar uchar4 __attribute__((ext_vector_type(4)));
typedef uchar uchar8 __attribute__((ext_vector_type(8)));
typedef uchar uchar16 __attribute__((ext_vector_type(16)));
typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef short short8 __attribute__((ext_vector_type(8)));
typedef short short16 __attribute__((ext_vector_type(16)));
typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort ushort16 __attribute__((ext_vector_type(16)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));
typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));
typedef uint uint8 __attribute__((ext_vector_type(8)));
typedef uint uint16 __attribute__((ext_vector_type(16)));
typedef long long2 __attribute__((ext_vector_type(2)));
typedef long long3 __attribute__((ext_vector_type(3)));
typedef long long4 __attribute__((ext_vector_type(4)));
typedef long long8 __attribute__((ext_vector_type(8)));
typedef long long16 __attribute__((ext_vector_type(16)));
typedef ulong ulong2 __attribute__((ext_vector_type(2)));
typedef ulong ulong3 __attribute__((ext_vector_type(3)));
typedef ulong ulong4 __attribute__((ext_vector_type(4)));
typedef ulong ulong8 __attribute__((ext_vector_type(8)));
typedef ulong ulong16 __attribute__((ext_vector_type(16)));
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));
typedef float float16 __attribute__((ext_vector_type(16)));
typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef half half8 __attribute__((ext_vector_type(8)));
typedef half half16 __attribute__((ext_vector_type(16)));
typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef double double8 __attribute__((ext_vector_type(8)));
typedef double double16 __attribute__((ext_vector_type(16)));

// Validate debug information emitted for OpenCL vector data types.

void foo() {
  // -------------------------------------------------------------------------

  char2 c2;
  // CHECK: !DILocalVariable(name: "c2",{{.*}} type: [[C2:![0-9]+]])
  // TRUE:  [[C2]] = !DIBasicType(name: "char2", size: 16, encoding: DW_ATE_signed_char)
  // FALSE: [[C2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "char2", {{.*}}baseType: [[A2C:![0-9]+]])
  // FALSE: [[A2C]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[C:![0-9]+]], size: 16, flags: DIFlagVector, elements: [[ELT2:![0-9]+]])
  // FALSE: [[C]] = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
  // FALSE: [[ELT2]] = !{[[SUB2:![0-9]+]]}
  // FALSE: [[SUB2]] = !DISubrange(count: 2)

  char3 c3;
  // CHECK: !DILocalVariable(name: "c3",{{.*}} type: [[C3:![0-9]+]])
  // TRUE:  [[C3]] = !DIBasicType(name: "char3", size: 32, encoding: DW_ATE_signed_char)
  // FALSE: [[C3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "char3", {{.*}}baseType: [[A3C:![0-9]+]])
  // FALSE: [[A3C]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[C]], size: 32, flags: DIFlagVector, elements: [[ELT3:![0-9]+]])
  // FALSE: [[ELT3]] = !{[[SUB3:![0-9]+]]}
  // FALSE: [[SUB3]] = !DISubrange(count: 3)

  char4 c4;
  // CHECK: !DILocalVariable(name: "c4",{{.*}} type: [[C4:![0-9]+]])
  // TRUE:  [[C4]] = !DIBasicType(name: "char4", size: 32, encoding: DW_ATE_signed_char)
  // FALSE: [[C4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "char4", {{.*}}baseType: [[A4C:![0-9]+]])
  // FALSE: [[A4C]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[C]], size: 32, flags: DIFlagVector, elements: [[ELT4:![0-9]+]])
  // FALSE: [[ELT4]] = !{[[SUB4:![0-9]+]]}
  // FALSE: [[SUB4]] = !DISubrange(count: 4)

  char8 c8;
  // CHECK: !DILocalVariable(name: "c8",{{.*}} type: [[C8:![0-9]+]])
  // TRUE:  [[C8]] = !DIBasicType(name: "char8", size: 64, encoding: DW_ATE_signed_char)
  // FALSE: [[C8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "char8", {{.*}}baseType: [[A8C:![0-9]+]])
  // FALSE: [[A8C]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[C]], size: 64, flags: DIFlagVector, elements: [[ELT8:![0-9]+]])
  // FALSE: [[ELT8]] = !{[[SUB8:![0-9]+]]}
  // FALSE: [[SUB8]] = !DISubrange(count: 8)

  char16 c16;
  // CHECK: !DILocalVariable(name: "c16",{{.*}} type: [[C16:![0-9]+]])
  // TRUE:  [[C16]] = !DIBasicType(name: "char16", size: 128, encoding: DW_ATE_signed_char)
  // FALSE: [[C16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "char16", {{.*}}baseType: [[A16C:![0-9]+]])
  // FALSE: [[A16C]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[C]], size: 128, flags: DIFlagVector, elements: [[ELT16:![0-9]+]])
  // FALSE: [[ELT16]] = !{[[SUB16:![0-9]+]]}
  // FALSE: [[SUB16]] = !DISubrange(count: 16)

  // -------------------------------------------------------------------------

  uchar2 uc2;
  // CHECK: !DILocalVariable(name: "uc2",{{.*}} type: [[UC2:![0-9]+]])
  // TRUE:  [[UC2]] = !DIBasicType(name: "uchar2", size: 16, encoding: DW_ATE_unsigned_char)
  // FALSE: [[UC2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar2", {{.*}}baseType: [[A2UC:![0-9]+]])
  // FALSE: [[A2UC]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UCHAR:![0-9]+]], size: 16, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[UCHAR]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar", {{.*}}baseType: [[UC:![0-9]+]])
  // FALSE: [[UC]] = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)

  uchar3 uc3;
  // CHECK: !DILocalVariable(name: "uc3",{{.*}} type: [[UC3:![0-9]+]])
  // TRUE:  [[UC3]] = !DIBasicType(name: "uchar3", size: 32, encoding: DW_ATE_unsigned_char)
  // FALSE: [[UC3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar3", {{.*}}baseType: [[A3UC:![0-9]+]])
  // FALSE: [[A3UC]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UCHAR]], size: 32, flags: DIFlagVector, elements: [[ELT3]])

  uchar4 uc4;
  // CHECK: !DILocalVariable(name: "uc4",{{.*}} type: [[UC4:![0-9]+]])
  // TRUE:  [[UC4]] = !DIBasicType(name: "uchar4", size: 32, encoding: DW_ATE_unsigned_char)
  // FALSE: [[UC4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar4", {{.*}}baseType: [[A4UC:![0-9]+]])
  // FALSE: [[A4UC]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UCHAR]], size: 32, flags: DIFlagVector, elements: [[ELT4]])

  uchar8 uc8;
  // CHECK: !DILocalVariable(name: "uc8",{{.*}} type: [[UC8:![0-9]+]])
  // TRUE:  [[UC8]] = !DIBasicType(name: "uchar8", size: 64, encoding: DW_ATE_unsigned_char)
  // FALSE: [[UC8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar8", {{.*}}baseType: [[A8UC:![0-9]+]])
  // FALSE: [[A8UC]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UCHAR]], size: 64, flags: DIFlagVector, elements: [[ELT8]])

  uchar16 uc16;
  // CHECK: !DILocalVariable(name: "uc16",{{.*}} type: [[UC16:![0-9]+]])
  // TRUE:  [[UC16]] = !DIBasicType(name: "uchar16", size: 128, encoding: DW_ATE_unsigned_char)
  // FALSE: [[UC16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar16", {{.*}}baseType: [[A16UC:![0-9]+]])
  // FALSE: [[A16UC]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UCHAR]], size: 128, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  short2 s2;
  // CHECK: !DILocalVariable(name: "s2",{{.*}} type: [[S2:![0-9]+]])
  // TRUE:  [[S2]] = !DIBasicType(name: "short2", size: 32, encoding: DW_ATE_signed)
  // FALSE: [[S2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "short2", {{.*}}baseType: [[A2S:![0-9]+]])
  // FALSE: [[A2S]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[SHORT:![0-9]+]], size: 32, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[SHORT]] = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)

  short3 s3;
  // CHECK: !DILocalVariable(name: "s3",{{.*}} type: [[S3:![0-9]+]])
  // TRUE:  [[S3]] = !DIBasicType(name: "short3", size: 64, encoding: DW_ATE_signed)
  // FALSE: [[S3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "short3", {{.*}}baseType: [[A3S:![0-9]+]])
  // FALSE: [[A3S]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[SHORT]], size: 64, flags: DIFlagVector, elements: [[ELT3]])

  short4 s4;
  // CHECK: !DILocalVariable(name: "s4",{{.*}} type: [[S4:![0-9]+]])
  // TRUE:  [[S4]] = !DIBasicType(name: "short4", size: 64, encoding: DW_ATE_signed)
  // FALSE: [[S4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "short4", {{.*}}baseType: [[A4S:![0-9]+]])
  // FALSE: [[A4S]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[SHORT]], size: 64, flags: DIFlagVector, elements: [[ELT4]])

  short8 s8;
  // CHECK: !DILocalVariable(name: "s8",{{.*}} type: [[S8:![0-9]+]])
  // TRUE:  [[S8]] = !DIBasicType(name: "short8", size: 128, encoding: DW_ATE_signed)
  // FALSE: [[S8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "short8", {{.*}}baseType: [[A8S:![0-9]+]])
  // FALSE: [[A8S]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[SHORT]], size: 128, flags: DIFlagVector, elements: [[ELT8]])

  short16 s16;
  // CHECK: !DILocalVariable(name: "s16",{{.*}} type: [[S16:![0-9]+]])
  // TRUE:  [[S16]] = !DIBasicType(name: "short16", size: 256, encoding: DW_ATE_signed)
  // FALSE: [[S16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "short16", {{.*}}baseType: [[A16S:![0-9]+]])
  // FALSE: [[A16S]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[SHORT]], size: 256, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  ushort2 us2;
  // CHECK: !DILocalVariable(name: "us2",{{.*}} type: [[US2:![0-9]+]])
  // TRUE:  [[US2]] = !DIBasicType(name: "ushort2", size: 32, encoding: DW_ATE_unsigned)
  // FALSE: [[US2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort2", {{.*}}baseType: [[A2US:![0-9]+]])
  // FALSE: [[A2US]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[USHORT:![0-9]+]], size: 32, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[USHORT]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort", {{.*}}baseType: [[US:![0-9]+]])
  // FALSE: [[US]] = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)

  ushort3 us3;
  // CHECK: !DILocalVariable(name: "us3",{{.*}} type: [[US3:![0-9]+]])
  // TRUE:  [[US3]] = !DIBasicType(name: "ushort3", size: 64, encoding: DW_ATE_unsigned)
  // FALSE: [[US3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort3", {{.*}}baseType: [[A3US:![0-9]+]])
  // FALSE: [[A3US]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[USHORT]], size: 64, flags: DIFlagVector, elements: [[ELT3]])

  ushort4 us4;
  // CHECK: !DILocalVariable(name: "us4",{{.*}} type: [[US4:![0-9]+]])
  // TRUE:  [[US4]] = !DIBasicType(name: "ushort4", size: 64, encoding: DW_ATE_unsigned)
  // FALSE: [[US4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort4", {{.*}}baseType: [[A4US:![0-9]+]])
  // FALSE: [[A4US]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[USHORT]], size: 64, flags: DIFlagVector, elements: [[ELT4]])

  ushort8 us8;
  // CHECK: !DILocalVariable(name: "us8",{{.*}} type: [[US8:![0-9]+]])
  // TRUE:  [[US8]] = !DIBasicType(name: "ushort8", size: 128, encoding: DW_ATE_unsigned)
  // FALSE: [[US8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort8", {{.*}}baseType: [[A8US:![0-9]+]])
  // FALSE: [[A8US]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[USHORT]], size: 128, flags: DIFlagVector, elements: [[ELT8]])

  ushort16 us16;
  // CHECK: !DILocalVariable(name: "us16",{{.*}} type: [[US16:![0-9]+]])
  // TRUE:  [[US16]] = !DIBasicType(name: "ushort16", size: 256, encoding: DW_ATE_unsigned)
  // FALSE: [[US16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ushort16", {{.*}}baseType: [[A16US:![0-9]+]])
  // FALSE: [[A16US]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[USHORT]], size: 256, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  int2 i2;
  // CHECK: !DILocalVariable(name: "i2",{{.*}} type: [[I2:![0-9]+]])
  // TRUE:  [[I2]] = !DIBasicType(name: "int2", size: 64, encoding: DW_ATE_signed)
  // FALSE: [[I2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "int2", {{.*}}baseType: [[A2I:![0-9]+]])
  // FALSE: [[A2I]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[I:![0-9]+]], size: 64, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[I]] = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)

  int3 i3;
  // CHECK: !DILocalVariable(name: "i3",{{.*}} type: [[I3:![0-9]+]])
  // TRUE:  [[I3]] = !DIBasicType(name: "int3", size: 128, encoding: DW_ATE_signed)
  // FALSE: [[I3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "int3", {{.*}}baseType: [[A3I:![0-9]+]])
  // FALSE: [[A3I]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[I]], size: 128, flags: DIFlagVector, elements: [[ELT3]])

  int4 i4;
  // CHECK: !DILocalVariable(name: "i4",{{.*}} type: [[I4:![0-9]+]])
  // TRUE:  [[I4]] = !DIBasicType(name: "int4", size: 128, encoding: DW_ATE_signed)
  // FALSE: [[I4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "int4", {{.*}}baseType: [[A4I:![0-9]+]])
  // FALSE: [[A4I]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[I]], size: 128, flags: DIFlagVector, elements: [[ELT4]])

  int8 i8;
  // CHECK: !DILocalVariable(name: "i8",{{.*}} type: [[I8:![0-9]+]])
  // TRUE:  [[I8]] = !DIBasicType(name: "int8", size: 256, encoding: DW_ATE_signed)
  // FALSE: [[I8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "int8", {{.*}}baseType: [[A8I:![0-9]+]])
  // FALSE: [[A8I]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[I]], size: 256, flags: DIFlagVector, elements: [[ELT8]])

  int16 i16;
  // CHECK: !DILocalVariable(name: "i16",{{.*}} type: [[I16:![0-9]+]])
  // TRUE:  [[I16]] = !DIBasicType(name: "int16", size: 512, encoding: DW_ATE_signed)
  // FALSE: [[I16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "int16", {{.*}}baseType: [[A16I:![0-9]+]])
  // FALSE: [[A16I]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[I]], size: 512, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  uint2 ui2;
  // CHECK: !DILocalVariable(name: "ui2",{{.*}} type: [[UI2:![0-9]+]])
  // TRUE:  [[UI2]] = !DIBasicType(name: "uint2", size: 64, encoding: DW_ATE_unsigned)
  // FALSE: [[UI2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint2", {{.*}}baseType: [[A2UI:![0-9]+]])
  // FALSE: [[A2UI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UINT:![0-9]+]], size: 64, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[UINT]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", {{.*}}baseType: [[UI:![0-9]+]])
  // FALSE: [[UI]] = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)

  uint3 ui3;
  // CHECK: !DILocalVariable(name: "ui3",{{.*}} type: [[UI3:![0-9]+]])
  // TRUE:  [[UI3]] = !DIBasicType(name: "uint3", size: 128, encoding: DW_ATE_unsigned)
  // FALSE: [[UI3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint3", {{.*}}baseType: [[A3UI16:![0-9]+]])
  // FALSE: [[A3UI16]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UINT]], size: 128, flags: DIFlagVector, elements: [[ELT3]])

  uint4 ui4;
  // CHECK: !DILocalVariable(name: "ui4",{{.*}} type: [[UI4:![0-9]+]])
  // TRUE:  [[UI4]] = !DIBasicType(name: "uint4", size: 128, encoding: DW_ATE_unsigned)
  // FALSE: [[UI4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint4", {{.*}}baseType: [[A4UI16:![0-9]+]])
  // FALSE: [[A4UI16]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UINT]], size: 128, flags: DIFlagVector, elements: [[ELT4]])

  uint8 ui8;
  // CHECK: !DILocalVariable(name: "ui8",{{.*}} type: [[UI8:![0-9]+]])
  // TRUE:  [[UI8]] = !DIBasicType(name: "uint8", size: 256, encoding: DW_ATE_unsigned)
  // FALSE: [[UI8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint8", {{.*}}baseType: [[A8UI16:![0-9]+]])
  // FALSE: [[A8UI16]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UINT]], size: 256, flags: DIFlagVector, elements: [[ELT8]])

  uint16 ui16;
  // CHECK: !DILocalVariable(name: "ui16",{{.*}} type: [[UI16:![0-9]+]])
  // TRUE:  [[UI16]] = !DIBasicType(name: "uint16", size: 512, encoding: DW_ATE_unsigned)
  // FALSE: [[UI16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "uint16", {{.*}}baseType: [[A16UI16:![0-9]+]])
  // FALSE: [[A16UI16]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[UINT]], size: 512, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  long2 l2;
  // CHECK: !DILocalVariable(name: "l2",{{.*}} type: [[L2:![0-9]+]])
  // TRUE:  [[L2]] = !DIBasicType(name: "long2", size: 128, encoding: DW_ATE_signed)
  // FALSE: [[L2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "long2", {{.*}}baseType: [[A2LI:![0-9]+]])
  // FALSE: [[A2LI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[LI:![0-9]+]], size: 128, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[LI]] = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)

  long3 l3;
  // CHECK: !DILocalVariable(name: "l3",{{.*}} type: [[L3:![0-9]+]])
  // TRUE:  [[L3]] = !DIBasicType(name: "long3", size: 256, encoding: DW_ATE_signed)
  // FALSE: [[L3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "long3", {{.*}}baseType: [[A3LI:![0-9]+]])
  // FALSE: [[A3LI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[LI]], size: 256, flags: DIFlagVector, elements: [[ELT3]])

  long4 l4;
  // CHECK: !DILocalVariable(name: "l4",{{.*}} type: [[L4:![0-9]+]])
  // TRUE:  [[L4]] = !DIBasicType(name: "long4", size: 256, encoding: DW_ATE_signed)
  // FALSE: [[L4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "long4", {{.*}}baseType: [[A4LI:![0-9]+]])
  // FALSE: [[A4LI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[LI]], size: 256, flags: DIFlagVector, elements: [[ELT4]])

  long8 l8;
  // CHECK: !DILocalVariable(name: "l8",{{.*}} type: [[L8:![0-9]+]])
  // TRUE:  [[L8]] = !DIBasicType(name: "long8", size: 512, encoding: DW_ATE_signed)
  // FALSE: [[L8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "long8", {{.*}}baseType: [[A8LI:![0-9]+]])
  // FALSE: [[A8LI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[LI]], size: 512, flags: DIFlagVector, elements: [[ELT8]])

  long16 l16;
  // CHECK: !DILocalVariable(name: "l16",{{.*}} type: [[L16:![0-9]+]])
  // TRUE:  [[L16]] = !DIBasicType(name: "long16", size: 1024, encoding: DW_ATE_signed)
  // FALSE: [[L16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "long16", {{.*}}baseType: [[A16LI:![0-9]+]])
  // FALSE: [[A16LI]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[LI]], size: 1024, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  ulong2 ul2;
  // CHECK: !DILocalVariable(name: "ul2",{{.*}} type: [[UL2:![0-9]+]])
  // TRUE:  [[UL2]] = !DIBasicType(name: "ulong2", size: 128, encoding: DW_ATE_unsigned)
  // FALSE: [[UL2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong2", {{.*}}baseType: [[A2ULONG:![0-9]+]])
  // FALSE: [[A2ULONG]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[ULONG:![0-9]+]], size: 128, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[ULONG]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong", {{.*}}baseType: [[LUI:![0-9]+]])
  // FALSE: [[LUI]] = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)

  ulong3 ul3;
  // CHECK: !DILocalVariable(name: "ul3",{{.*}} type: [[UL3:![0-9]+]])
  // TRUE:  [[UL3]] = !DIBasicType(name: "ulong3", size: 256, encoding: DW_ATE_unsigned)
  // FALSE: [[UL3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong3", {{.*}}baseType: [[A3ULONG:![0-9]+]])
  // FALSE: [[A3ULONG]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[ULONG]], size: 256, flags: DIFlagVector, elements: [[ELT3]])

  ulong4 ul4;
  // CHECK: !DILocalVariable(name: "ul4",{{.*}} type: [[UL4:![0-9]+]])
  // TRUE:  [[UL4]] = !DIBasicType(name: "ulong4", size: 256, encoding: DW_ATE_unsigned)
  // FALSE: [[UL4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong4", {{.*}}baseType: [[A4ULONG:![0-9]+]])
  // FALSE: [[A4ULONG]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[ULONG]], size: 256, flags: DIFlagVector, elements: [[ELT4]])

  ulong8 ul8;
  // CHECK: !DILocalVariable(name: "ul8",{{.*}} type: [[UL8:![0-9]+]])
  // TRUE:  [[UL8]] = !DIBasicType(name: "ulong8", size: 512, encoding: DW_ATE_unsigned)
  // FALSE: [[UL8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong8", {{.*}}baseType: [[A8ULONG:![0-9]+]])
  // FALSE: [[A8ULONG]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[ULONG]], size: 512, flags: DIFlagVector, elements: [[ELT8]])

  ulong16 ul16;
  // CHECK: !DILocalVariable(name: "ul16",{{.*}} type: [[UL16:![0-9]+]])
  // TRUE:  [[UL16]] = !DIBasicType(name: "ulong16", size: 1024, encoding: DW_ATE_unsigned)
  // FALSE: [[UL16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "ulong16", {{.*}}baseType: [[A16ULONG:![0-9]+]])
  // FALSE: [[A16ULONG]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[ULONG]], size: 1024, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  float2 f2;
  // CHECK: !DILocalVariable(name: "f2",{{.*}} type: [[F2:![0-9]+]])
  // TRUE:  [[F2]] = !DIBasicType(name: "float2", size: 64, encoding: DW_ATE_float)
  // FALSE: [[F2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "float2", {{.*}}baseType: [[A2F:![0-9]+]])
  // FALSE: [[A2F]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[FLOAT:![0-9]+]], size: 64, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[FLOAT]] = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)

  float3 f3;
  // CHECK: !DILocalVariable(name: "f3",{{.*}} type: [[F3:![0-9]+]])
  // TRUE:  [[F3]] = !DIBasicType(name: "float3", size: 128, encoding: DW_ATE_float)
  // FALSE: [[F3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "float3", {{.*}}baseType: [[A3F:![0-9]+]])
  // FALSE: [[A3F]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[FLOAT]], size: 128, flags: DIFlagVector, elements: [[ELT3]])

  float4 f4;
  // CHECK: !DILocalVariable(name: "f4",{{.*}} type: [[F4:![0-9]+]])
  // TRUE:  [[F4]] = !DIBasicType(name: "float4", size: 128, encoding: DW_ATE_float)
  // FALSE: [[F4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "float4", {{.*}}baseType: [[A4F:![0-9]+]])
  // FALSE: [[A4F]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[FLOAT]], size: 128, flags: DIFlagVector, elements: [[ELT4]])

  float8 f8;
  // CHECK: !DILocalVariable(name: "f8",{{.*}} type: [[F8:![0-9]+]])
  // TRUE:  [[F8]] = !DIBasicType(name: "float8", size: 256, encoding: DW_ATE_float)
  // FALSE: [[F8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "float8", {{.*}}baseType: [[A8F:![0-9]+]])
  // FALSE: [[A8F]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[FLOAT]], size: 256, flags: DIFlagVector, elements: [[ELT8]])

  float16 f16;
  // CHECK: !DILocalVariable(name: "f16",{{.*}} type: [[F16:![0-9]+]])
  // TRUE:  [[F16]] = !DIBasicType(name: "float16", size: 512, encoding: DW_ATE_float)
  // FALSE: [[F16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "float16", {{.*}}baseType: [[A16F:![0-9]+]])
  // FALSE: [[A16F]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[FLOAT]], size: 512, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  double2 d2;
  // CHECK: !DILocalVariable(name: "d2",{{.*}} type: [[D2:![0-9]+]])
  // TRUE:  [[D2]] = !DIBasicType(name: "double2", size: 128, encoding: DW_ATE_float)
  // FALSE: [[D2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "double2", {{.*}}baseType: [[A2D:![0-9]+]])
  // FALSE: [[A2D]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[DOUBLE:![0-9]+]], size: 128, flags: DIFlagVector, elements: [[ELT2]])
  // FALSE: [[DOUBLE]] = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)

  double3 d3;
  // CHECK: !DILocalVariable(name: "d3",{{.*}} type: [[D3:![0-9]+]])
  // TRUE:  [[D3]] = !DIBasicType(name: "double3", size: 256, encoding: DW_ATE_float)
  // FALSE: [[D3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "double3", {{.*}}baseType: [[A3D:![0-9]+]])
  // FALSE: [[A3D]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[DOUBLE]], size: 256, flags: DIFlagVector, elements: [[ELT3]])

  double4 d4;
  // CHECK: !DILocalVariable(name: "d4",{{.*}} type: [[D4:![0-9]+]])
  // TRUE:  [[D4]] = !DIBasicType(name: "double4", size: 256, encoding: DW_ATE_float)
  // FALSE: [[D4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "double4", {{.*}}baseType: [[A4D:![0-9]+]])
  // FALSE: [[A4D]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[DOUBLE]], size: 256, flags: DIFlagVector, elements: [[ELT4]])

  double8 d8;
  // CHECK: !DILocalVariable(name: "d8",{{.*}} type: [[D8:![0-9]+]])
  // TRUE:  [[D8]] = !DIBasicType(name: "double8", size: 512, encoding: DW_ATE_float)
  // FALSE: [[D8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "double8", {{.*}}baseType: [[A8D:![0-9]+]])
  // FALSE: [[A8D]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[DOUBLE]], size: 512, flags: DIFlagVector, elements: [[ELT8]])

  double16 d16;
  // CHECK: !DILocalVariable(name: "d16",{{.*}} type: [[D16:![0-9]+]])
  // TRUE:  [[D16]] = !DIBasicType(name: "double16", size: 1024, encoding: DW_ATE_float)
  // FALSE: [[D16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "double16", {{.*}}baseType: [[A16D:![0-9]+]])
  // FALSE: [[A16D]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[DOUBLE]], size: 1024, flags: DIFlagVector, elements: [[ELT16]])

  // -------------------------------------------------------------------------

  half2 h2;
  // CHECK: !DILocalVariable(name: "h2",{{.*}} type: [[H2:![0-9]+]])
  // TRUE:  [[H2]] = !DIBasicType(name: "half2", size: 32, encoding: DW_ATE_float)
  // FALSE: [[H2]] = !DIDerivedType(tag: DW_TAG_typedef, name: "half2", {{.*}}baseType: [[A2H:![0-9]+]])
  // FALSE: [[A2H]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[HALF:![0-9]+]], size: 32, flags: DIFlagVector, elements: [[ELT2]])
  // [[HALF]] = !DIBasicType(name: "half", size: 16, encoding: DW_ATE_float)

  half3 h3;
  // CHECK: !DILocalVariable(name: "h3",{{.*}} type: [[H3:![0-9]+]])
  // TRUE:  [[H3]] = !DIBasicType(name: "half3", size: 64, encoding: DW_ATE_float)
  // FALSE: [[H3]] = !DIDerivedType(tag: DW_TAG_typedef, name: "half3", {{.*}}baseType: [[A3H:![0-9]+]])
  // FALSE: [[A3H]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[HALF]], size: 64, flags: DIFlagVector, elements: [[ELT3]])

  half4 h4;
  // CHECK: !DILocalVariable(name: "h4",{{.*}} type: [[H4:![0-9]+]])
  // TRUE:  [[H4]] = !DIBasicType(name: "half4", size: 64, encoding: DW_ATE_float)
  // FALSE: [[H4]] = !DIDerivedType(tag: DW_TAG_typedef, name: "half4", {{.*}}baseType: [[A4H:![0-9]+]])
  // FALSE: [[A4H]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[HALF]], size: 64, flags: DIFlagVector, elements: [[ELT4]])

  half8 h8;
  // CHECK: !DILocalVariable(name: "h8",{{.*}} type: [[H8:![0-9]+]])
  // TRUE: [[H8]] = !DIBasicType(name: "half8", size: 128, encoding: DW_ATE_float)
  // FALSE: [[H8]] = !DIDerivedType(tag: DW_TAG_typedef, name: "half8", {{.*}}baseType: [[A8H:![0-9]+]])
  // FALSE: [[A8H]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[HALF]], size: 128, flags: DIFlagVector, elements: [[ELT8]])

  half16 h16;
  // CHECK: !DILocalVariable(name: "h16",{{.*}} type: [[H16:![0-9]+]])
  // TRUE:  [[H16]] = !DIBasicType(name: "half16", size: 256, encoding: DW_ATE_float)
  // FALSE: [[H16]] = !DIDerivedType(tag: DW_TAG_typedef, name: "half16", {{.*}}baseType: [[A16H:![0-9]+]])
  // FALSE: [[A16H]] = !DICompositeType(tag: DW_TAG_array_type, baseType: [[HALF]], size: 256, flags: DIFlagVector, elements: [[ELT16]])
}
