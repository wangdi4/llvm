; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; This test is same as intel_noinline_copy_elems_inside_array.ll except
; a use of the first argument of @copy_inside_array is not directly used
; in getelementptr instruction. This test checks that inline decision for
; "copy_inside_array" routine is not delayed until link time.

; Makes sure "copy_inside_array" call in "foo" is not inlined when
; -pre-lto-inline-cost is passed.
; RUN: opt < %s -disable-output -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -disable-output -inlinereportsetup -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost -inlinereportemitter 2>&1 | FileCheck %s

; New PM command
; RUN: opt < %s -disable-output  -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost  2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -disable-output -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s

; CHECK: COMPILE FUNC: foo
; CHECK-NOT: copy_inside_array {{.*}}Inline decision is delayed until link time


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }

define dso_local void @copy_inside_array(%struct.arc* %newarc, i64 %newpos, %struct.node* %tail, %struct.node* %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number) {
entry:
  %bc1 = bitcast %struct.arc* %newarc to i64*
  %bc2 = bitcast i64* %bc1 to %struct.arc*
  %arrayidx = getelementptr inbounds %struct.arc, %struct.arc* %bc2, i64 %newpos
  %flow = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx, i64 0, i32 7
  store i64 %red_cost, i64* %flow
  %conv = trunc i64 %number to i32
  %id = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx, i64 0, i32 0
  store i32 %conv, i32* %id
  %add = add nsw i64 %newpos, 1
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %pos.0 = phi i64 [ %add, %entry ], [ %div, %while.body ]
  %sub = add nsw i64 %pos.0, -1
  %tobool = icmp eq i64 %sub, 0
  br i1 %tobool, label %while.end, label %land.rhs

land.rhs:                                         ; preds = %while.cond
  %div = sdiv i64 %pos.0, 2
  %sub9 = add nsw i64 %div, -1
  %arrayidx10 = getelementptr inbounds %struct.arc, %struct.arc* %newarc, i64 %sub9
  %flow11 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 7
  %0 = load i64, i64* %flow11
  %cmp = icmp slt i64 %0, %red_cost
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs
  %tail16 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 2
  %1 = bitcast %struct.node** %tail16 to i64*
  %2 = load i64, i64* %1
  %arrayidx18 = getelementptr inbounds %struct.arc, %struct.arc* %newarc, i64 %sub
  %tail19 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 2
  %3 = bitcast %struct.node** %tail19 to i64*
  store i64 %2, i64* %3
  %head23 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 3
  %4 = bitcast %struct.node** %head23 to i64*
  %5 = load i64, i64* %4
  %head26 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 3
  %6 = bitcast %struct.node** %head26 to i64*
  store i64 %5, i64* %6
  %cost30 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 1
  %7 = load i64, i64* %cost30
  %cost33 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 1
  store i64 %7, i64* %cost33
  %8 = load i64, i64* %cost30
  %org_cost40 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 8
  store i64 %8, i64* %org_cost40
  %9 = load i64, i64* %flow11
  %flow47 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 7
  store i64 %9, i64* %flow47
  %id51 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 0
  %10 = load i32, i32* %id51
  %id54 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx18, i64 0, i32 0
  store i32 %10, i32* %id54
  store %struct.node* %tail, %struct.node** %tail16
  store %struct.node* %head, %struct.node** %head23
  store i64 %cost, i64* %cost30
  %org_cost67 = getelementptr inbounds %struct.arc, %struct.arc* %arrayidx10, i64 0, i32 8
  store i64 %cost, i64* %org_cost67
  store i64 %red_cost, i64* %flow11
  store i32 %conv, i32* %id51
  br label %while.cond

while.end:                                        ; preds = %while.cond, %land.rhs
  ret void
}

define void @foo(i32 %n, %struct.arc* %newarc, i64 %newpos, %struct.node* %tail, %struct.node* %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc8, %entry
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.inc8 ]
  %0 = trunc i64 %indvars.iv21 to i32
  %add = add nsw i32 %0, %n
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  call void @copy_inside_array(%struct.arc* %newarc, i64 %newpos, %struct.node* %tail, %struct.node* %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number)
  %exitcond = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond, label %for.inc8, label %for.body3

for.inc8:                                         ; preds = %for.body3
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next22, 8
  br i1 %exitcond23, label %for.end10, label %for.body

for.end10:                                        ; preds = %for.inc8
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
