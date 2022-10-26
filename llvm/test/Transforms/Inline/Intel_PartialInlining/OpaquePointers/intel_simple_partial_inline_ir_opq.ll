; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Test for checking the simple Intel partial inliner. This partial inliner
; will identify small functions that use an argument as iterator and return a
; boolean. The result will be a cloned function that checks if the input
; parameter is null to exit early, or calls the original function. The inliner
; will actually perform the partial inlining.
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
;   for (Head = List; Head != NULL; Head = Head->Next()) {
;     Num += Head->Num;
;     val = Num == 0;
;   }
;   return val;
; }
;
; bool bar(Node *List) {
;   return foo(List);
; }
;
; The partial inliner will create two new functions: an outlined function
; that performs the loop (foo.for.body) and a function that calls the outlined
; function wrapped in a NULL check (foo.1).
;
; void foo.for.body(Node *List, bool *result) {
;   Node *Head;
;   int Num = 0;
;
;   for (Head = List; Head != NULL; Head = Head->Next()) {
;     Num += Head->Num;
;     *result = Num == 0;
;   }
; }
;
; bool foo.1(Node *List) {
;   bool res = true;
;   if (List != NULL)
;     foo.for.body(List, &res);
;
;   return res;
; }
;
; Also, the call to foo in bar will be replaced with a call to foo.1.
;
; bool bar(Node *List) {
;   return foo.1(List);
; }
;
; Then, the inliner will fully inline foo.1 into bar and won't inline foo.for.body.
;
; bool bar(Node *List) {
;   bool res = true;
;   if (List != NULL)
;     foo.for.body(List, &res);
;
;   return res;
; }
;
; This produces a partial inlining of foo into bar rather than fully inlining.
;
; This test checks that the IR was generated correctly from the partial
; inliner pass. It is the same test case as intel_simple_partial_inline_ir.ll,
; but it checks for opaque pointers.
;
; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -S 2>&1 | FileCheck %s

; Check that the call site of foo was replaced with foo.1
;
; CHECK: define i1 @_Z3barP4Node(ptr %List) #1
; CHECK:  %call = call zeroext i1 @_Z3fooP4Node.1(ptr %0)

; Check that foo was cloned correctly and foo.1 was marked as
; "prefer-partial-inline-inlined-clone"
;
; CHECK: define i1 @_Z3fooP4Node.1(ptr %List) #2
; CHECK: codeRepl:
; CHECK: call void @_Z3fooP4Node.1.for.body(ptr %List, ptr %phitmp.loc)
; CHECK: %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp.reload, %for.body.for.end_crit_edge ]
; CHECK: ret i1 %Num.0.lcssa

; Check that the loop body was extracted correctly and foo.1.for.body is
; marked as "prefer-partial-inline-outlined-func"
;
; CHECK: define internal void @_Z3fooP4Node.1.for.body(ptr %List, ptr %phitmp.out) #3

; Check the attributes were created
;
; CHECK: attributes #2 = { "prefer-partial-inline-inlined-clone"
; CHECK: attributes #3 = { "prefer-partial-inline-outlined-func"

; ModuleID = 'intel_simple_partial_inline_ir.ll'
source_filename = "intel_simple_partial_inline_ir.ll"

%struct.Node = type { i32, ptr }

define i1 @_Z3fooP4Node(ptr %List) {
entry:
  %cmp8 = icmp eq ptr %List, null
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
  %cmp = icmp eq ptr %1, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.body ]
  ret i1 %Num.0.lcssa
}

; Function Attrs: noinline
define i1 @_Z3barP4Node(ptr %List) #0 {
entry:
  %List.addr = alloca ptr, align 8
  store ptr %List, ptr %List.addr, align 8
  %0 = load ptr, ptr %List.addr, align 8
  %call = call zeroext i1 @_Z3fooP4Node(ptr %0)
  ret i1 %call
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
