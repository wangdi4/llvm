// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//expected-no-diagnostics
//***INTEL: pragma nofusion test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma nofusion
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nofusion_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma nofusion
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nofusion
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nofusion
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma nofusion
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma nofusion
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nofusion_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nofusion
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
      #pragma nofusion
      #pragma nofusion
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma nofusion
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma nofusion 121212 ;
  #pragma nofusion;
  ;
  #pragma nofusion
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FUSION", metadata !"INFO", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: ret {{.*}}
  #pragma nofusion
  return;
}

