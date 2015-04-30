// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma unroll_and_jam test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL:[0-9]+]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma unroll_and_jam 
  #pragma unroll_and_jam 280 // expected-warning {{missing '(' after '#pragma unroll_and_jam' - ignoring}}
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

unroll_and_jam_label1:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma unroll_and_jam
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL1:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
  #pragma unroll_and_jam
  #pragma unroll_and_jam (2+3*2)
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL2:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
  #pragma unroll_and_jam
  #pragma unroll_and_jam (4-20
  while(i > argc)
  ;
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma unroll_and_jam
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma unroll_and_jam
    for(k = 0; k < argc; k++)
    ;
  }
  ;
unroll_and_jam_label2:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: br label {{.*}}
  #pragma unroll_and_jam
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL3:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
      #pragma unroll_and_jam
      #pragma unroll_and_jam 
      #pragma unroll_and_jam (1232211233423423)
      ;
      break;
    default:
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma unroll_and_jam
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
  #pragma unroll_and_jam 121212 ; // expected-warning {{missing '(' after '#pragma unroll_and_jam' - ignoring}}
  #pragma unroll_and_jam;        // expected-warning {{missing '(' after '#pragma unroll_and_jam' - ignoring}}
  ;
  #pragma unroll_and_jam
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL]])
// CHECK-NEXT: ret {{.*}}
  #pragma unroll_and_jam

  return;
}

// CHECK: ![[PRAGMA_VAL]] = metadata !{metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 -1}
// CHECK-NEXT: ![[PRAGMA_VAL1]] = metadata !{metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 8}
// CHECK-NEXT: ![[PRAGMA_VAL2]] = metadata !{metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 -16}
// CHECK-NEXT: ![[PRAGMA_VAL3]] = metadata !{metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i64 1232211233423423}
