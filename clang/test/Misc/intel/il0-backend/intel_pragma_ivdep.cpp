// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma ivdep test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %struct.S{{.*}}
#pragma ivdep // expected-warning {{this pragma must immediately precede a statement}}
struct S {
  #pragma ivdep // expected-warning {{this pragma must immediately precede a statement}}
  int a;
} d;

// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %class.C{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
class C {
  int a;
  public:
  #pragma ivdep // expected-warning {{this pragma must immediately precede a statement}}
  int b;
} e;

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %i = alloca{{.*}}
  #pragma ivdep // expected-warning {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma ivdep
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

ivdep_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma ivdep
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: br label {{.*}}
  #pragma ivdep
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: br label {{.*}}
  #pragma ivdep
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma ivdep
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma ivdep
    for(k = 0; k < argc; k++)
    ;
  }
  ;
ivdep_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: br label {{.*}}
  #pragma ivdep
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      break;
    case (1):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP", metadata !"LOOP")
// CHECK-NEXT: br label {{.*}}
      #pragma ivdep
      #pragma ivdep loop
      ;
      break;
    case (2):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma ivdep
      return (1);
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
      #pragma ivdep // expected-warning {{this pragma may not be used here}}
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
  #pragma ivdep 121212 ; // expected-warning {{unknown pragma qualifier}}
  #pragma ivdep;         // expected-warning {{unknown pragma qualifier}} 
  ;
  #pragma ivdep
  
  return (i);
// CHECK: br label {{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: ret i32{{.*}}
  #pragma ivdep // expected-warning {{this pragma may not be used here}}
  #pragma ivdep // expected-warning {{this pragma may not be used here}}
}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
#pragma ivdep // expected-warning {{this pragma must immediately precede a statement}}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"IVDEP")
// CHECK-NEXT: ret {{.*}}
  #pragma ivdep
  return;
}

