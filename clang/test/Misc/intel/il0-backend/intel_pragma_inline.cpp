// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma inline test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE", metadata !"COMPLETE")
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma inline recursive
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  i++;
  for(j = i; j < argc; ++i)
  ;

inline_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma inline
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma inline
  do
  {
    ++i;
  }
  while (i > argc);
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma inline
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma inline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma inline
    for(k = 0; k < argc; k++)
    ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  ;
inline_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: br label {{.*}}
  #pragma inline
  do
  {
  ;
  }
  while (i > argc);
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE", metadata !"COMPLETE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: br label {{.*}}
      #pragma inline
      #pragma inline recursive
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: store{{.*}}
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: br label{{.*}}
      #pragma inline
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
  #pragma inline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma inline;        // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  ;
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
  #pragma inline
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INLINE")
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ENDINLINE")
// CHECK-NEXT: ret {{.*}}
  #pragma inline

  return;
}

