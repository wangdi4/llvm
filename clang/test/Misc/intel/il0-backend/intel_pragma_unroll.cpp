// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma unroll test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma unroll 
  #pragma unroll 280 // expected-warning {{missing '(' after '#pragma unroll' - ignoring}}
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

unroll_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma unroll
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 8)
// CHECK-NEXT: br label {{.*}}
  #pragma unroll
  #pragma unroll (2+3*2)
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -16)
// CHECK-NEXT: br label {{.*}}
  #pragma unroll
  #pragma unroll (4-20
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma unroll
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma unroll
    for(k = 0; k < argc; k++)
    ;
  }
  ;
unroll_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: br label {{.*}}
  #pragma unroll
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i64 1232211233423423)
// CHECK-NEXT: br label {{.*}}
      #pragma unroll
      #pragma unroll 
      #pragma unroll (1232211233423423)
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma unroll
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma unroll 121212 ; // expected-warning {{missing '(' after '#pragma unroll' - ignoring}}
  #pragma unroll;        // expected-warning {{missing '(' after '#pragma unroll' - ignoring}}
  ;
  #pragma unroll
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 -1)
// CHECK-NEXT: ret {{.*}}
  #pragma unroll

  return;
}

