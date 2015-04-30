// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma novector test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %struct.S{{.*}}
#pragma novector // expected-warning {{this pragma must immediately precede a statement}}
struct S {
  #pragma novector // expected-warning {{this pragma must immediately precede a statement}}
  int a;
} d;

// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %class.C{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
class C {
  int a;
  public:
  #pragma novector // expected-warning {{this pragma must immediately precede a statement}}
  int b;
} e;

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %i = alloca{{.*}}
  #pragma novector // expected-warning {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL:[0-9]+]])
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma novector
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

novector_label1:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma novector
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma novector
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma novector
  while(i > argc)
  ;
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma novector
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma novector
    for(k = 0; k < argc; k++)
    ;
  }
  ;
novector_label2:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma novector
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      break;
    case (1):
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
      #pragma novector
      #pragma novector
      ;
      break;
    case (2):
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
      break;
    default:
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma novector
      return (1);
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
      #pragma novector // expected-warning {{this pragma may not be used here}}
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
  #pragma novector 121212 ; 
  #pragma novector;         
  ;
  #pragma novector
  
  return (i);
// CHECK: br label {{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: ret i32{{.*}}
  #pragma novector // expected-warning {{this pragma may not be used here}}
  #pragma novector // expected-warning {{this pragma may not be used here}}
}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
#pragma novector // expected-warning {{this pragma must immediately precede a statement}}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: ret {{.*}}
  #pragma novector
  return;
}

// CHECK: ![[PRAGMA_VAL]] = metadata !{metadata !"LOOP_VECTOR", metadata !"NEVER"}
