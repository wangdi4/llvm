// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma loop_count test

#define XXX 14

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL:[0-9]+]])
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma loop_count min=2 max(10) avg 20
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

loop_count_label1:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL1:[0-9]+]])
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma loop_count(10)
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL2:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count=2,4+5,3+2*8
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL2]])
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count(2,4+5,3+2*8)
  while(i > argc)
  ;
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL3:[0-9]+]])
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma loop_count max(10+1),min(8*2), avg=2)
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL4:[0-9]+]])
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma loop_count (2
    for(k = 0; k < argc; k++)
    ;
  }
  ;
loop_count_label2:  
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL5:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count max(3) , min ( 2 )
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL4]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL6:[0-9]+]])
// CHECK-NEXT: br label {{.*}}
      #pragma loop_count 2
      #pragma loop_count=1
      ;
      break;
    default:
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL7:[0-9]+]])
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma loop_count min=2,avg=5 24 // expected-warning {{extra text after expected end of preprocessing directive}}
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL8:[0-9]+]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL9:[0-9]+]])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL10:[0-9]+]])
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
  #pragma loop_count 121212 
  #pragma loop_count XXX
  ;
  #pragma loop_count=121212121212
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata ![[PRAGMA_VAL11:[0-9]+]])
// CHECK-NEXT: ret {{.*}}
  #pragma loop_count 12

  return;
}

// CHECK: ![[PRAGMA_VAL]] = metadata !{metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", i32 2, metadata !"AVG", metadata !"CONSTANT", i32 20, metadata !"MAX", metadata !"CONSTANT", i32 10}
// CHECK-NEXT: ![[PRAGMA_VAL1]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 10}
// CHECK-NEXT: ![[PRAGMA_VAL2]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 2, metadata !"AMOUNT", metadata !"CONSTANT", i32 9, metadata !"AMOUNT", metadata !"CONSTANT", i32 19}
// CHECK-NEXT: ![[PRAGMA_VAL3]] = metadata !{metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", i32 16, metadata !"AVG", metadata !"CONSTANT", i32 2, metadata !"MAX", metadata !"CONSTANT", i32 11}
// CHECK-NEXT: ![[PRAGMA_VAL4]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 2}
// CHECK-NEXT: ![[PRAGMA_VAL5]] = metadata !{metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", i32 2, metadata !"MAX", metadata !"CONSTANT", i32 3}
// CHECK-NEXT: ![[PRAGMA_VAL6]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 1}
// CHECK-NEXT: ![[PRAGMA_VAL7]] = metadata !{metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", i32 2, metadata !"AVG", metadata !"CONSTANT", i32 5}
// CHECK-NEXT: ![[PRAGMA_VAL8]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 121212}
// CHECK-NEXT: ![[PRAGMA_VAL9]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 14}
// CHECK-NEXT: ![[PRAGMA_VAL10]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i64 121212121212}
// CHECK-NEXT: ![[PRAGMA_VAL11]] = metadata !{metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", i32 12}
