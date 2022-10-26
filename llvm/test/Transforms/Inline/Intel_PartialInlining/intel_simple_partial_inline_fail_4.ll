; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Test for checking that the simple Intel partial inliner didn't work
; because the call site was marked as noinline.
;
; This test case is very simple. In C++ it can be seen as follows:
;
; struct Node {
;  int Num;
;  Node *Next;
; };
;
; bool foo(Node *List) {
;   Node *Head;
;   bool val = true;
;   int Num = 0;
;
;   for (Head = List; Head != NULL; Head = Head->Next) {
;     Num += Head->Num;
;     val = Num == 0;
;   }
;   return val;
; }
;
; bool bar(Node *List) {
;   #pragma no_inline
;   return foo(List);
; }
;
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -debug-only=intel_partialinline 2>&1 | FileCheck %s

; CHECK: No candidates for partial inlining
; CHECK-NOT: Candidates for partial inlining: 1
; CHECK-NOT: Analyzing Function: _Z3fooP4Node
; CHECK-NOT:     Result: Can partial inline

%struct.Node = type { i32, %struct.Node* }

define i1 @_Z3fooP4Node(%struct.Node* %List) {
entry:
  %cmp8 = icmp eq %struct.Node* %List, null
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %Num.010 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %Head.09 = phi %struct.Node* [ %1, %for.body ], [ %List, %entry ]
  %Num1 = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 0
  %0 = load i32, i32* %Num1
  %add = add nsw i32 %0, %Num.010
  %phitmp = icmp eq i32 %add, 0
  %Next = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 1
  %1 = load %struct.Node*, %struct.Node** %Next
  %cmp = icmp eq %struct.Node* %1, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.end, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.body ]
  ret i1 %Num.0.lcssa
}


define i1 @_Z3barP4Node(%struct.Node* %List) {
entry:
  %List.addr = alloca %struct.Node*
  store %struct.Node* %List, %struct.Node** %List.addr
  %0 = load %struct.Node*, %struct.Node** %List.addr
  %call = call zeroext i1 @_Z3fooP4Node(%struct.Node* %0) #0
  ret i1 %call
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
