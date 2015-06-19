// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma vector test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %struct.S{{.*}}
struct S {
  #pragma vector // expected-warning {{this pragma must immediately precede a statement}}
  int a;
} d;

// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %class.C{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
class C {
  int a;
  public:
  #pragma vector // expected-warning {{this pragma must immediately precede a statement}}
  int b;
} e;

int array[10];
S arr[10];

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: %i = alloca{{.*}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  array[i]=1;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
  #pragma vector
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"ALWAYS")
  #pragma vector always
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"ALWAYS", metadata !"ASSERT")
  #pragma vector always assert
  #pragma vector always none // expected-warning {{unknown pragma qualifier}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"ALWAYS")
  #pragma vector always ""   // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"ALIGN")
  #pragma vector aligned
  #pragma vector aligned assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"ALIGN")
  #pragma vector aligned ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"UNALIGN")
  #pragma vector unaligned
  #pragma vector unaligned assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"UNALIGN")
  #pragma vector unaligned ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"TEMPORAL")
  #pragma vector temporal
  #pragma vector temporal assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"TEMPORAL")
  #pragma vector temporal ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal
  #pragma vector nontemporal assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"MASK_READWRITE")
  #pragma vector mask_readwrite
  #pragma vector mask_readwrite assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"MASK_READWRITE")
  #pragma vector mask_readwrite ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %{{.+}})
  #pragma vector nontemporal (argc 
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal (argc+2) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal (d) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %aaa)
  #pragma vector nontemporal (aaa) 
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %i, metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %aaa)
  #pragma vector nontemporal (i, aaa)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %i, metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %aaa)
  #pragma vector nontemporal (i  aaa)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* getelementptr inbounds ([10 x i32], [10 x i32]* @array, i32 0, {{i64|i32}} 0))
  #pragma vector nontemporal (array)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* getelementptr inbounds ([10 x i32], [10 x i32]* @array, i32 0, {{i64|i32}} 0))
  #pragma vector nontemporal (array, 1+2) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: %{{[0-9]+}} = load i32, i32* %i, align 4
// CHECK: %{{.+}} = getelementptr inbounds [10 x i32], [10 x i32]* @array, i32 0, i{{32|64}} %{{.+}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %{{.+}})
  #pragma vector nontemporal (array[i])
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal (arr) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NONTEMPORAL")
  #pragma vector nontemporal (arr[i]) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

vector_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma vector
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: br label {{.*}}
  #pragma vector
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: br label {{.*}}
  #pragma vector
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma vector
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma vector
    for(k = 0; k < argc; k++)
    ;
  }
  ;
vector_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: br label {{.*}}
  #pragma vector
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      break;
    case (1):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: br label {{.*}}
      #pragma vector
      #pragma vector
      ;
      break;
    case (2):
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: br label {{.*}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma vector
      return (1);
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
  #pragma vector 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma vector;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma vector
  
  return (i);
// CHECK: br label {{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
// CHECK: ret i32{{.*}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
#pragma vector // expected-warning {{this pragma must immediately precede a statement}}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NOMASK_READWRITE")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"VEC_REMAINDER")
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"LOOP_VECTOR", metadata !"NOVEC_REMAINDER")
// CHECK-NEXT: ret {{.*}}
  #pragma vector
  #pragma vector nomask_readwrite
  #pragma vector vecremainder
  #pragma vector novecremainder
  return;
}

