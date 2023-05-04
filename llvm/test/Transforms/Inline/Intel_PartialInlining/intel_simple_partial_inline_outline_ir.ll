; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Test for checking the simple Intel partial inliner. This partial inliner
; will identify small functions that use an argument as iterator and return a
; boolean. The result will be a cloned function that checks if the input
; parameter is null to exit early, or call the original function. The inliner
; will actually perform the partial inlining.
;
; This test case is very simple. In C++ it can be seen as follows:
;
; struct Node {
;  int Num;
;  Node *Next;
; };
;
; int glob = 0;
;
; bool foo(Node *List) {
;   Node *Head;
;   bool val = true;
;   glob++;
;   int Num = 0;
;
;   for (Head = List; Head != NULL; Head = Head->Next()) {
;     Num += Head->Num;
;     val = Num == 0;
;   }
;   baz();
;   return val;
; }
;
; bool bar(Node *List) {
;   return foo(List);
; }
;
; void baz() {
;  return ;
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
;   glob++;
;   if (List != NULL)
;     foo.for.body(List, &res);
;
;   baz();
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
;   glob++;
;   if (List != NULL)
;     foo.for.body(List, &res);
;
;   baz();
;   return res;
; }
;
; This produces a partial inlining of foo into bar rather than fully inlining.
;
; This test case will check that the outlined function was created correctly.
; It is the same test case intel_simple_partial_inline_outline_ir.ll, but it
; checks for opaque pointers.
;
; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -S 2>&1 | FileCheck %s

; Check that the call site of foo was replaced with foo.1
; CHECK: define i1 @bar(ptr %List) #0
; CHECK:  %call = call zeroext i1 @foo.1(ptr %tmp)

; Check that foo was cloned correctly (foo.1)
;
; CHECK: define i1 @foo.1(ptr %List) #1
; CHECK:  %a = getelementptr i32, ptr @glob, i64 0
; CHECK:  %v = load i32, ptr %a
; CHECK:  %v1 = add i32 1, %v
; CHECK:  store i32 %v1, ptr %a
; CHECK: codeRepl:
; CHECK: call void @foo.1.for.body(ptr %List, ptr %phitmp.loc)
; CHECK: %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp.reload, %for.body.for.end_crit_edge ]
; CHECK: call void @baz()
; CHECK: ret i1 %Num.0.lcssa


; Check that the outlined function is correct (foo.1.for.body)
;
; CHECK: define internal void @foo.1.for.body(ptr %List, ptr %phitmp.out) #2
; CHECK:newFuncRoot:
; CHECK:  br label %for.body
; CHECK: for.body.for.end_crit_edge.exitStub:              ; preds = %for.body
; CHECK:  ret void
; CHECK-NOT:  %a = getelementptr i32, ptr @glob, i64 0
; CHECK-NOT:  %v = load i32, ptr %a
; CHECK-NOT:  %v1 = add i32 1, %v
; CHECK-NOT:  store i32 %v1, ptr %a
; CHECK-NOT: call void @baz()

; Check the attributes were created
;
; CHECK: attributes #1 = { "prefer-partial-inline-inlined-clone"
; CHECK: attributes #2 = { "prefer-partial-inline-outlined-func"

; ModuleID = 'intel_simple_partial_inline_outline_ir.ll'
source_filename = "intel_simple_partial_inline_outline_ir.ll"

%struct.Node = type { i32, ptr }

@glob = global i32 0

define void @baz() {
  ret void
}

define i1 @foo(ptr %List) {
entry:
  %a = getelementptr i32, ptr @glob, i64 0
  %v = load i32, ptr %a, align 4
  %v1 = add i32 1, %v
  store i32 %v1, ptr %a, align 4
  %cmp8 = icmp eq ptr %List, null
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %Num.010 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %Head.09 = phi ptr [ %tmp1, %for.body ], [ %List, %entry ]
  %Num1 = getelementptr inbounds %struct.Node, ptr %Head.09, i64 0, i32 0
  %tmp = load i32, ptr %Num1, align 4
  %add = add nsw i32 %tmp, %Num.010
  %phitmp = icmp eq i32 %add, 0
  %Next = getelementptr inbounds %struct.Node, ptr %Head.09, i64 0, i32 1
  %tmp1 = load ptr, ptr %Next, align 8
  %cmp = icmp eq ptr %tmp1, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.body ]
  call void @baz()
  ret i1 %Num.0.lcssa
}

define i1 @bar(ptr %List) {
entry:
  %List.addr = alloca ptr, align 8
  store ptr %List, ptr %List.addr, align 8
  %tmp = load ptr, ptr %List.addr, align 8
  %call = call zeroext i1 @foo(ptr %tmp)
  ret i1 %call
}

;end INTEL_FEATURE_SW_ADVANCED
