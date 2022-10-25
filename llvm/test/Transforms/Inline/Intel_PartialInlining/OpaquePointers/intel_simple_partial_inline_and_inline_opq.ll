; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Test for checking the simple Intel partial inliner and the inlining
; heuristic for inlining recursive calls within extracted functions.
;
; This partial inliner will identify small functions that use an argument as
; iterator and return a boolean. The result will be a cloned function that
; checks if the input parameter is null to exit early, or call the original
; function. The inliner will actually perform the partial inlining.
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
;     val1 = foo(Head);
;     val2 = Num == 0;
;     val = val1 | val2;
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
;     val1 = foo(Head);
;     val2 = Num == 0;
;     val = val1 | val2;
;     *result = val;
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
;   return res;
; }
;
; This produces a partial inlining of foo into bar rather than fully inlining.
; Finally a call within the extracted function will be inlined due to the
; "recursive call in extracted function" heuristic.
;
; This test case will check that debug information is printed correctly in
; the partial inlining trace, and that a call within _Z3fooP4Node.1.for.body
; is inlined because it is an extracted recursive call. It is the same test
; case as intel_simple_partial_inline_and_inline.ll, but for opaque pointers.
;
; RUN: opt < %s -opaque-pointers -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -intel-partialinline -inline -debug-only=intel_partialinline -inline-report=7 2>&1 | FileCheck --check-prefix=CHECK-OLD %s
; RUN: opt < %s -opaque-pointers -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline),cgscc(inline)' -debug-only=intel_partialinline -inline-report=7 2>&1 | FileCheck --check-prefix=CHECK-NEW %s

; CHECK: Candidates for partial inlining: 1
; CHECK:     _Z3fooP4Node
; CHECK: Analyzing Function: _Z3fooP4Node
; CHECK:     Result: Can partial inline
; CHECK-OLD: COMPILE FUNC: _Z3fooP4Node.1.for.body
; CHECK-OLD: INLINE:{{.*}}<<Callee has extracted recursive call>>
; CHECK-NOT: call{{.*}}_Z3barP4Node
; CHECK-NEW: COMPILE FUNC: _Z3fooP4Node.1.for.body
; CHECK-NEW: INLINE:{{.*}}<<Callee has extracted recursive call>>

; ModuleID = 'intel_simple_partial_inline_and_inline.ll'
source_filename = "intel_simple_partial_inline_and_inline.ll"
target triple = "x86_64-unknown-linux-gnu"

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
  %mybool = tail call i1 @_Z3barP4Node(ptr nonnull %Head.09)
  %phitmp = icmp eq i32 %add, 0
  %Next = getelementptr inbounds %struct.Node, ptr %Head.09, i64 0, i32 1
  %1 = load ptr, ptr %Next, align 8
  %cmp = icmp eq ptr %1, null
  %cmp1 = and i1 %cmp, %mybool
  br i1 %cmp1, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.body ]
  ret i1 %Num.0.lcssa
}

define i1 @_Z3barP4Node(ptr %List) {
entry:
  %List.addr = alloca ptr, align 8
  store ptr %List, ptr %List.addr, align 8
  %0 = load ptr, ptr %List.addr, align 8
  %call = call zeroext i1 @_Z3fooP4Node(ptr %0)
  ret i1 %call
}

; end INTEL_FEATURE_SW_ADVANCED
