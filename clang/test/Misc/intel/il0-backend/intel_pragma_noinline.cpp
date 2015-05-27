// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma noinline test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL:[0-9]+]])
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma noinline 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
// CHECK: call void @llvm.intel.pragma(metadata !2)
  i++;
  for(j = i; j < argc; ++i)
  ;

noinline_label1:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma noinline
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma noinline
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma noinline
  while(i > argc)
  ;
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma noinline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma noinline
    for(k = 0; k < argc; k++)
    ;
// CHECK: call void @llvm.intel.pragma(metadata !2)
  }
// CHECK: call void @llvm.intel.pragma(metadata !2)
  ;
noinline_label2:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma noinline
  do
  {
  ;
  }
  while (i > argc);
// CHECK: call void @llvm.intel.pragma(metadata !2)
  switch (argc) {
    case (0):
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: br label {{.*}}
      #pragma noinline
      #pragma noinline 
      ;
      break;
    default:
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store{{.*}}
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: br label{{.*}}
      #pragma noinline
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
  #pragma noinline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma noinline;        // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
  ;
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
  #pragma noinline
// CHECK: call void @llvm.intel.pragma(metadata !2)
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: ret {{.*}}
  #pragma noinline

  return;
}

// CHECK: ![[PRAGMA_VAL]] = metadata !{metadata !"NOINLINE"}
// CHECK-NEXT: !2 = metadata !{metadata !"ENDINLINE"}
