// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -DERROR_MSG -o - %s 
//***INTEL: pragma vector test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %struct.S{{.*}}
struct S {
  #pragma vector // expected-warning {{this pragma must immediately precede a statement}}
  int a;
} d;

// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %class.C{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
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
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: %i = alloca{{.*}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  array[i]=1;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
  #pragma vector
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !1), !PRAGMA !1
  #pragma vector always
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !1), !PRAGMA !1
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2), !PRAGMA !2
  #pragma vector always assert
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !1), !PRAGMA !1
  #pragma vector always none // expected-warning {{unknown pragma qualifier}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !1), !PRAGMA !1
  #pragma vector always ""   // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
  #pragma vector aligned
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
  #pragma vector aligned assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3), !PRAGMA !3
  #pragma vector aligned ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
  #pragma vector unaligned
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
  #pragma vector unaligned assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4), !PRAGMA !4
  #pragma vector unaligned ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !5), !PRAGMA !5
  #pragma vector temporal
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !5), !PRAGMA !5
  #pragma vector temporal assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !5), !PRAGMA !5
  #pragma vector temporal ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !6), !PRAGMA !6
  #pragma vector nontemporal
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !6), !PRAGMA !6
  #pragma vector nontemporal assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !6), !PRAGMA !6
  #pragma vector nontemporal ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !7), !PRAGMA !7
  #pragma vector mask_readwrite
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !7), !PRAGMA !7
  #pragma vector mask_readwrite assert // expected-warning {{always must precede assert qualifier, ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !7), !PRAGMA !7
  #pragma vector mask_readwrite ""     // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %argc.addr}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %argc.addr}
  #pragma vector nontemporal (argc 
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
  #pragma vector nontemporal (argc+2) // expected-warning {{invalid expression in pragma will be ignored}}
#ifdef ERROR_MSG
  #pragma vector nontemporal (assert) // expected-error {{use of undeclared identifier 'assert'}}
#endif  
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
  #pragma vector nontemporal (d) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}
  #pragma vector nontemporal (aaa) 
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %i}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %i}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}
  #pragma vector nontemporal (i, aaa)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %i}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %i}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %aaa}
  #pragma vector nontemporal (i  aaa)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !8), !PRAGMA !8
  #pragma vector nontemporal (array)
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !8), !PRAGMA !8
  #pragma vector nontemporal (array, 1+2) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: %1 = load i32* %i, align 4
// CHECK-NEXT: %idxprom1 = sext i32 %1 to i64
// CHECK-NEXT: %arrayidx2 = getelementptr inbounds [10 x i32]* @array, i32 0, i64 %idxprom1
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %arrayidx2}), !PRAGMA !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* %arrayidx2}
  #pragma vector nontemporal (array[i])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
  #pragma vector nontemporal (arr) // expected-warning {{invalid expression in pragma will be ignored}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
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
// CHECK: vector_label1:{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: load i32* %j{{.*}}
  #pragma vector
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: br label %do.body{{.*}}
  #pragma vector
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: do.end{{.*}}:
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: br label %while.cond{{.*}}
  #pragma vector
  while(i > argc)
  ;
// CHECK: while.end{{.*}}:
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: load i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma vector
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma vector
    for(k = 0; k < argc; k++)
    ;
  }
  ;
vector_label2:  
// CHECK: vector_label2:{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: br label %do.body{{.*}}
  #pragma vector
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: sw.bb{{.*}}:{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: br label %sw.epilog{{.*}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      break;
    case (1):
// CHECK: sw.bb{{.*}}:{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: br label %sw.epilog{{.*}}
      #pragma vector
      #pragma vector
      ;
      break;
    case (2):
// CHECK: sw.bb{{.*}}:{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: br label %sw.epilog{{.*}}
      #pragma vector // expected-warning {{this pragma may not be used here}}
      break;
    default:
// CHECK: sw.default{{.*}}:{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma vector
      return (1);
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: sw.epilog{{.*}}:
      #pragma vector // expected-warning {{this pragma may not be used here}}
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
  #pragma vector 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma vector;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma vector
  
  return (i);
// CHECK: br label %return{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK: ret i32{{.*}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
  #pragma vector // expected-warning {{this pragma may not be used here}}
}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
#pragma vector // expected-warning {{this pragma must immediately precede a statement}}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void @llvm.intel.pragma(metadata !0), !PRAGMA !0
// CHECK-NEXT: ret {{.*}}
  #pragma vector
  return;
}

// CHECK: define void {{.*}}vvv{{.*}}(
void vvv()
{
  goto label1;
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
  #pragma vector
label1:
  #pragma vector
// CHECK: ret{{.*}}
} // expected-error {{expected statement}}

// CHECK: !0 = metadata !{metadata !"LOOP_VECTOR"}
// CHECK-NEXT: !1 = metadata !{metadata !"QUAL_ALWAYS"}
// CHECK-NEXT: !2 = metadata !{metadata !"QUAL_ASSERT"}
// CHECK-NEXT: !3 = metadata !{metadata !"QUAL_ALIGN"}
// CHECK-NEXT: !4 = metadata !{metadata !"QUAL_UNALIGN"}
// CHECK-NEXT: !5 = metadata !{metadata !"QUAL_TEMPORAL"}
// CHECK-NEXT: !6 = metadata !{metadata !"QUAL_NONTEMPORAL"}
// CHECK-NEXT: !7 = metadata !{metadata !"QUAL_MASK_READWRITE"}
// CHECK-NEXT: !8 = metadata !{metadata !"QUAL_NONTEMPORAL", metadata !"SIMPLE", i32* getelementptr inbounds ([10 x i32]* @array, i32 0, i64 0)}
