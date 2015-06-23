// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma nounroll test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma nounroll 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nounroll_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma nounroll
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma nounroll
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma nounroll
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nounroll_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
      #pragma nounroll
      #pragma nounroll 
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma nounroll
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma nounroll 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma nounroll;        // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma nounroll
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UNROLL_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: ret {{.*}}
  #pragma nounroll

  return;
}

