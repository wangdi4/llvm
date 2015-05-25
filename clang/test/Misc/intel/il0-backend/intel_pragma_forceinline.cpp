// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma forceinline test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL_R:[0-9]+]])
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma forceinline recursive
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
// CHECK: call void @llvm.intel.pragma(metadata !2)
  i++;
  for(j = i; j < argc; ++i)
  ;

forceinline_label1:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL:[0-9]+]])
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma forceinline
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  do
  {
    ++i;
  }
  while (i > argc);
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  while(i > argc)
  ;
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma forceinline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma forceinline
    for(k = 0; k < argc; k++)
    ;
// CHECK: call void @llvm.intel.pragma(metadata !2)
  }
// CHECK: call void @llvm.intel.pragma(metadata !2)
  ;
forceinline_label2:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma forceinline
  do
  {
  ;
  }
  while (i > argc);
// CHECK: call void @llvm.intel.pragma(metadata !2)
  switch (argc) {
    case (0):
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL_R]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: br label {{.*}}
      #pragma forceinline
      #pragma forceinline recursive
      ;
      break;
    default:
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store{{.*}}
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: br label{{.*}}
      #pragma forceinline
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
  #pragma forceinline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma forceinline;        // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
  ;
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
  #pragma forceinline
  
  return (i);
// CHECK: call void @llvm.intel.pragma(metadata !2)
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: ret {{.*}}
  #pragma forceinline

  return;
}

// CHECK: ![[PRAGMA_VAL_R]] = metadata !{metadata !"FORCEINLINE", metadata !"COMPLETE"}
// CHECK-NEXT: !2 = metadata !{metadata !"ENDINLINE"}
// CHECK-NEXT: ![[PRAGMA_VAL]] = metadata !{metadata !"FORCEINLINE"}
