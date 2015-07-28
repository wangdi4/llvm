// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma forceinline test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE", metadata !"COMPLETE")
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma forceinline recursive
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  i++;
  for(j = i; j < argc; ++i)
  ;

forceinline_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma forceinline
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  do
  {
    ++i;
  }
  while (i > argc);
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma forceinline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma forceinline
    for(k = 0; k < argc; k++)
    ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  ;
forceinline_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  do
  {
  ;
  }
  while (i > argc);
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE", metadata !"COMPLETE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: br label {{.*}}
      #pragma forceinline
      #pragma forceinline recursive
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: store{{.*}}
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: br label{{.*}}
      #pragma forceinline
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
  #pragma forceinline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma forceinline;        // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  ;
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
  #pragma forceinline
  
  return (i);
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"FORCEINLINE")
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: ret {{.*}}
  #pragma forceinline

  return;
}

