; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Test for checking the simple Intel partial inliner didn't modify
; the IR. The partial inliner should not work here since the function
; doesn't return a boolean, even if it uses the argument for checking
; if the loop will be executed or not.
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
;   int Num = 0;
;
;   for (Head = List; Head != NULL; Head = Head->Next) {
;     Num += Head->Num;
;   }
;   return Num;
; }
;
; bool bar(Node *List) {
;   return foo(List);
; }
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -debug-only=intel_partialinline 2>&1 | FileCheck %s

; CHECK: No candidates for partial inlining
; CHECK-NOT: Candidates for partial inlining
; CHECK-NOT: Analyzing Function: _Z3fooP4Node
; CHECK-NOT:     Result: Can partial inline

%struct.Node = type { i32, %struct.Node* }

define i32 @_Z3fooP4Node(%struct.Node* %List) {
entry:
  %cmp8 = icmp eq %struct.Node* %List, null
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %Num.010 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %Head.09 = phi %struct.Node* [ %1, %for.body ], [ %List, %entry ]
  %Num1 = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 0
  %0 = load i32, i32* %Num1
  %add = add nsw i32 %0, %Num.010
  %Next = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 1
  %1 = load %struct.Node*, %struct.Node** %Next
  %cmp = icmp eq %struct.Node* %1, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.end, %entry
  %Num.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %Num.0.lcssa
}


define i32 @_Z3barP4Node(%struct.Node* %List) #0 {
entry:
  %List.addr = alloca %struct.Node*
  store %struct.Node* %List, %struct.Node** %List.addr
  %0 = load %struct.Node*, %struct.Node** %List.addr
  %call = call zeroext i32 @_Z3fooP4Node(%struct.Node* %0)
  ret i32 %call
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
