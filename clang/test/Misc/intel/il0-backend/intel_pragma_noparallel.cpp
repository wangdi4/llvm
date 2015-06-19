// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma noparallel test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %struct.S{{.*}}
#pragma noparallel // expected-warning {{this pragma must immediately precede a statement}}
struct S {
  #pragma noparallel // expected-warning {{this pragma must immediately precede a statement}}
  int a;
} d;

// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %class.C{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
class C {
  int a;
  public:
  #pragma noparallel // expected-warning {{this pragma must immediately precede a statement}}
  int b;
} e;

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %i = alloca{{.*}}
  #pragma noparallel // expected-warning {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma noparallel
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

noparallel_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma noparallel
  i+=j;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: br label {{.*}}
  #pragma noparallel
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: br label {{.*}}
  #pragma noparallel
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma noparallel
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma noparallel
    for(k = 0; k < argc; k++)
    ;
  }
  ;
noparallel_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: br label {{.*}}
  #pragma noparallel
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      break;
    case (1):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: br label {{.*}}
      #pragma noparallel
      #pragma noparallel
      ;
      break;
    case (2):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma noparallel
      return (1);
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
      #pragma noparallel // expected-warning {{this pragma may not be used here}}
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
  #pragma noparallel 121212 ; 
  #pragma noparallel;         
  ;
  #pragma noparallel
  
  return (i);
// CHECK: br label {{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: ret i32{{.*}}
  #pragma noparallel // expected-warning {{this pragma may not be used here}}
  #pragma noparallel // expected-warning {{this pragma may not be used here}}
}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
#pragma noparallel // expected-warning {{this pragma must immediately precede a statement}}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOPARALLEL", metadata !"NEVER")
// CHECK-NEXT: ret {{.*}}
  #pragma noparallel
  return;
}

