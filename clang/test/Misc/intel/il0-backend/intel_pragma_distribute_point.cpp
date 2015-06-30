// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//expected-no-diagnostics
//***INTEL: pragma distribute_point test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma distribute_point
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

distribute_point_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma distribute_point
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: br label {{.*}}
  #pragma distribute_point
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: br label {{.*}}
  #pragma distribute_point
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma distribute_point
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma distribute_point
    for(k = 0; k < argc; k++)
    ;
  }
  ;
distribute_point_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: br label {{.*}}
  #pragma distribute_point
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: br label {{.*}}
      #pragma distribute_point
      #pragma distribute_point
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma distribute_point
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma distribute_point 121212 ;
  #pragma distribute_point;
  ;
  #pragma distribute_point
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"DISTRIBUTE")
// CHECK-NEXT: ret {{.*}}
  #pragma distribute_point
  return;
}

