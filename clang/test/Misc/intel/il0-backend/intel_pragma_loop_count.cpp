// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma loop_count test

#define XXX 14

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", metadata i32 2, metadata !"AVG", metadata !"CONSTANT", metadata i32 20, metadata !"MAX", metadata !"CONSTANT", metadata i32 10)
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
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 10)
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma loop_count(10)
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 2, metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 9, metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 19)
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count=2,4+5,3+2*8
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 2, metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 9, metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 19)
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count(2,4+5,3+2*8)
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", metadata i32 16, metadata !"AVG", metadata !"CONSTANT", metadata i32 2, metadata !"MAX", metadata !"CONSTANT", metadata i32 11)
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma loop_count max(10+1),min(8*2), avg=2)
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 2)
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma loop_count (2
    for(k = 0; k < argc; k++)
    ;
  }
  ;
loop_count_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", metadata i32 2, metadata !"MAX", metadata !"CONSTANT", metadata i32 3)
// CHECK-NEXT: br label {{.*}}
  #pragma loop_count max(3) , min ( 2 )
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 2)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 1)
// CHECK-NEXT: br label {{.*}}
      #pragma loop_count 2
      #pragma loop_count=1
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"MIN", metadata !"CONSTANT", metadata i32 2, metadata !"AVG", metadata !"CONSTANT", metadata i32 5)
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma loop_count min=2,avg=5 24 // expected-warning {{extra text after expected end of preprocessing directive}}
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 121212)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 14)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i64 121212121212)
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma loop_count 121212 
  #pragma loop_count XXX
  ;
  #pragma loop_count=121212121212
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 12)
// CHECK-NEXT: ret {{.*}}
  #pragma loop_count 12

  return;
}

