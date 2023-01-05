; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Test for checking the simple Intel partial inliner. This partial inliner
; will identify small functions that use an argument as iterator and return a
; boolean. The result will be a cloned function that checks if the input
; parameter is null to exit early, or call the original function. The inliner
; will actually perform the partial inlining.
;
; This test case has 2 arguments that could jump to the exit block, but only
; one argument is used for computing the loop.

; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes='module(intel-partialinline)' -S 2>&1 | FileCheck %s

; Check that the call site of foo was replaced with foo.1
;
; CHECK: define i1 @_Z3barP4Node(%struct.Node* %List0, %struct.Node* %List) #1
; CHECK:  %call = call zeroext i1 @_Z3fooP4Node.1(%struct.Node* %0, %struct.Node* %1)

; Check that foo was cloned correctly and foo.1 was marked as
; "prefer-partial-inline-inlined-clone"
;
; CHECK: define i1 @_Z3fooP4Node.1(%struct.Node* %List0, %struct.Node* %List) #2
; CHECK: codeRepl:
; CHECK: %targetBlock = call i1 @_Z3fooP4Node.1.for.body(%struct.Node* %List, i32* %Num0, i1* %phitmp.loc)
; CHECK: %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp.reload, %for.cont.for.end_crit_edge ], [ false, %for.cont2.for.end_crit_edge ]
; CHECK: ret i1 %Num.0.lcssa

; Check that the loop body was extracted correctly and foo.1.for.body is
; marked as "prefer-partial-inline-outlined-func"
;
; CHECK: define internal i1 @_Z3fooP4Node.1.for.body(%struct.Node* %List, i32* %Num0, i1* %phitmp.out) #3

; Check the attributes were created
;
; CHECK: attributes #2 = { "prefer-partial-inline-inlined-clone"
; CHECK: attributes #3 = { "prefer-partial-inline-outlined-func"

%struct.Node = type { i32, %struct.Node* }

define i1 @_Z3fooP4Node(%struct.Node* %List0, %struct.Node* %List) {
entry:
  %cmp8 = icmp eq %struct.Node* %List, null
  %Num0 = getelementptr inbounds %struct.Node, %struct.Node* %List0, i64 0, i32 0
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %Num.010 = phi i32 [ %add, %for.cont2 ], [ 0, %entry ]
  %Head.09 = phi %struct.Node* [ %2, %for.cont2 ], [ %List, %entry ]
  %Num1 = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 0
  %0 = load i32, i32* %Num1
  %add = add nsw i32 %0, %Num.010
  %phitmp = icmp eq i32 %add, 0
  br i1 %phitmp, label %for.cont, label %for.cont2

for.cont:
  %1 = load i32, i32* %Num0
  %cont = icmp eq i32 %1, 0
  br i1 %cont, label %for.cont2, label %for.end

for.cont2:
  %Next = getelementptr inbounds %struct.Node, %struct.Node* %Head.09, i64 0, i32 1
  %2 = load %struct.Node*, %struct.Node** %Next
  %cmp = icmp eq %struct.Node* %2, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.end, %entry
  %Num.0.lcssa = phi i1 [ true, %entry ], [ %phitmp, %for.cont ], [ false, %for.cont2]
  ret i1 %Num.0.lcssa
}


define i1 @_Z3barP4Node(%struct.Node* %List0, %struct.Node* %List) #0 {
entry:
  %List.addr0 = alloca %struct.Node*
  store %struct.Node* %List0, %struct.Node** %List.addr0
  %0 = load %struct.Node*, %struct.Node** %List.addr0

  %List.addr = alloca %struct.Node*
  store %struct.Node* %List, %struct.Node** %List.addr
  %1 = load %struct.Node*, %struct.Node** %List.addr

  %call = call zeroext i1 @_Z3fooP4Node(%struct.Node* %0, %struct.Node* %1)
  ret i1 %call
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
