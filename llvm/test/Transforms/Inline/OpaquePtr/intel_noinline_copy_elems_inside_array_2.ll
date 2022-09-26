; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; This test is same as intel_noinline_copy_elems_inside_array.ll except
; a use of the first argument of @copy_inside_array is not directly used
; in getelementptr instruction. This test checks that inline decision for
; "copy_inside_array" routine is not delayed until link time.

; Makes sure "copy_inside_array" call in "foo" is not inlined when
; -pre-lto-inline-cost is passed.
; RUN: opt -opaque-pointers < %s -disable-output -inline -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt -opaque-pointers < %s -disable-output -inlinereportsetup -inline -inline-report=0xe886 -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost -inlinereportemitter 2>&1 | FileCheck %s

; New PM command
; RUN: opt -opaque-pointers < %s -disable-output  -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost  2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt -opaque-pointers < %s -disable-output -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s

; CHECK: COMPILE FUNC: foo
; CHECK-NOT: copy_inside_array {{.*}}Inline decision is delayed until link time

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.arc = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

define void @myfoo(ptr %x) {
entry:
  store ptr null, ptr %x, align 8
  ret void
}

define dso_local void @copy_inside_array(ptr %newarc, i64 %newpos, ptr %tail, ptr %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number) {
entry:
  call void @myfoo(ptr %newarc);
  %arrayidx = getelementptr inbounds %struct.arc, ptr %newarc, i64 %newpos
  %flow = getelementptr inbounds %struct.arc, ptr %arrayidx, i64 0, i32 7
  store i64 %red_cost, ptr %flow, align 8
  %conv = trunc i64 %number to i32
  %id = getelementptr inbounds %struct.arc, ptr %arrayidx, i64 0, i32 0
  store i32 %conv, ptr %id, align 4
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
  %arrayidx10 = getelementptr inbounds %struct.arc, ptr %newarc, i64 %sub9
  %flow11 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 7
  %i = load i64, ptr %flow11, align 8
  %cmp = icmp slt i64 %i, %red_cost
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs
  %tail16 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 2
  %i2 = load i64, ptr %tail16, align 8
  %arrayidx18 = getelementptr inbounds %struct.arc, ptr %newarc, i64 %sub
  %tail19 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 2
  store i64 %i2, ptr %tail19, align 8
  %head23 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 3
  %i5 = load i64, ptr %head23, align 8
  %head26 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 3
  store i64 %i5, ptr %head26, align 8
  %cost30 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 1
  %i7 = load i64, ptr %cost30, align 8
  %cost33 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 1
  store i64 %i7, ptr %cost33, align 8
  %i8 = load i64, ptr %cost30, align 8
  %org_cost40 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 8
  store i64 %i8, ptr %org_cost40, align 8
  %i9 = load i64, ptr %flow11, align 8
  %flow47 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 7
  store i64 %i9, ptr %flow47, align 8
  %id51 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 0
  %i10 = load i32, ptr %id51, align 4
  %id54 = getelementptr inbounds %struct.arc, ptr %arrayidx18, i64 0, i32 0
  store i32 %i10, ptr %id54, align 4
  store ptr %tail, ptr %tail16, align 8
  store ptr %head, ptr %head23, align 8
  store i64 %cost, ptr %cost30, align 8
  %org_cost67 = getelementptr inbounds %struct.arc, ptr %arrayidx10, i64 0, i32 8
  store i64 %cost, ptr %org_cost67, align 8
  store i64 %red_cost, ptr %flow11, align 8
  store i32 %conv, ptr %id51, align 4
  br label %while.cond

while.end:                                        ; preds = %land.rhs, %while.cond
  ret void
}

define void @foo(i32 %n, ptr %newarc, i64 %newpos, ptr %tail, ptr %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc8, %entry
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.inc8 ]
  %i = trunc i64 %indvars.iv21 to i32
  %add = add nsw i32 %i, %n
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  call void @copy_inside_array(ptr %newarc, i64 %newpos, ptr %tail, ptr %head, i64 %cost, i64 %red_cost, i64 %m, i64 %number)
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
