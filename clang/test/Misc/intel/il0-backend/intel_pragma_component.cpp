// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//expected-no-diagnostics
//***INTEL: pragma component test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: store i32 0, i32* %i{{.*}}
  #pragma component
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

component_label1:  
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata 
// CHECK: load i32, i32* %j{{.*}}
  #pragma component
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata 
// CHECK: br label {{.*}}
  #pragma component
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata 
// CHECK: br label {{.*}}
  #pragma component
  while(i > argc)
  ;
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: load i32, i32* %i{{.*}}
  #pragma component
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: store i32 0, i32* %k{{.*}}
  #pragma component
    for(k = 0; k < argc; k++)
    ;
  }
  ;
component_label2:  
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: br label {{.*}}
  #pragma component
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: br label {{.*}}
      #pragma component
      #pragma component
      ;
      break;
    default:
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma component
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
  #pragma component 121212 ;
  #pragma component;
  ;
  #pragma component
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
// CHECK: ret {{.*}}
  #pragma component
  return;
}

// CHECK: = !{!"{{C|c}}lang
// CHECK-NOT: = !{!
