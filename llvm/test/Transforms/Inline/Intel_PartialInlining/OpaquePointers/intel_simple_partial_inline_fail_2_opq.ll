; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; This test case checks that the partial inlining wasn't applied because the
; condition to stop the iteration isn't NULL.
;
; This test case is very simple. In C++ it can be seen as follows:
;
; struct Node {
;  int Num;
;  Node *Next;
; };
;
; bool foo(Node *List, Node *End) {
;   Node *Head;
;   bool val = true;
;   int Num = 0;
;
;   for (Head = List; Head != End; Head = Head->Next()) {
;     Num += Head->Num;
;     val = Num == 0;
;   }
;   return val;
; }
;
; bool bar(Node *List, Node, *End) {
;   return foo(List, End);
; }
;
; This is the same test case as intel_simple_partial_inline_fail_2.ll, but it
; checks for opaque pointers.
;
; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -debug-only=intel_partialinline 2>&1 | FileCheck %s

; CHECK: Candidates for partial inlining: 1
; CHECK:     _Z3fooP4Node
; CHECK: Analyzing Function: _Z3fooP4Node
; CHECK:     Result: Function can't be cloned
; CHECK-NOT:     Result: Can partial inline

; ModuleID = 'intel_simple_partial_inline_fail_2.ll'
source_filename = "intel_simple_partial_inline_fail_2.ll"


%struct.Node = type { i32, ptr }

define i1 @_Z3fooP4Node(ptr %List, ptr %End) {
entry:
  %cmp8 = icmp eq ptr %List, %End
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %Num.010 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %Head.09 = phi ptr [ %1, %for.body ], [ %List, %entry ]
  %Num1 = getelementptr inbounds %struct.Node, ptr %Head.09, i64 0, i32 0
  %0 = load i32, ptr %Num1, align 4
  %add = add nsw i32 %0, %Num.010
  %phitmp = icmp eq i32 %add, 0
  %Next = getelementptr inbounds %struct.Node, ptr %Head.09, i64 0, i32 1
  %1 = load ptr, ptr %Next, align 8
  %cmp = icmp eq ptr %1, %End
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.body ]
  ret i1 %Num.0.lcssa
}

; Function Attrs: noinline
define i1 @_Z3barP4Node(ptr %List, ptr %End) #0 {
entry:
  %List.addr = alloca ptr, align 8
  store ptr %List, ptr %List.addr, align 8
  %0 = load ptr, ptr %List.addr, align 8
  %End.addr = alloca ptr, align 8
  store ptr %End, ptr %End.addr, align 8
  %1 = load ptr, ptr %End.addr, align 8
  %call = call zeroext i1 @_Z3fooP4Node(ptr %0, ptr %1)
  ret i1 %call
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
