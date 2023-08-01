; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that both single operand phis %0 and %1 are mapped to the same symbase and are liveout of the region.

; CHECK: LiveOuts: {%1, %0}(sym:{{.*}})

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   if.then15.i:
; CHECK: |   %indvars.iv.next.i101923.out = undef;
; CHECK: |   %1 = (undef)[%indvars.iv.next.i101923.out + 1];
; CHECK: |   if (undef == 0)
; CHECK: |   {
; CHECK: |      goto for.body.i.thread.if.else17.i.loopexit_crit_edge;
; CHECK: |   }
; CHECK: |   %call14.i = @_Z11file_existsPKc(&((%1)[0]));
; CHECK: |   if (%call14.i == 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto if.then15.i;
; CHECK: |   }
; CHECK: + END LOOP

; Check that CG for this loop is successfull.
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cg" -hir-cost-model-throttling=0 -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG

; CHECK-CG: region{{.*}}:

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ARG_VECTOR.1.7.13.23.29.45.47.49.51.67.77.83.93.109.113.119.135.181 = type { i32, i32, ptr, ptr, ptr }
%union.ARG_MEMBER_U.0.6.12.22.28.44.46.48.50.66.76.82.92.108.112.118.134.180 = type { i32 }

; Function Attrs: uwtable
define void @_Z17read_command_linejPPKc(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr {
entry:
  br label %for.body.thread.i

for.body.thread.i:                                ; preds = %entry
  br i1 false, label %land.lhs.true.thread.i, label %if.else17.i

land.lhs.true.thread.i:                           ; preds = %for.body.thread.i
  br label %if.then.i

land.lhs.true13.i.preheader:                      ; preds = %if.end22.preheader.i
  br label %if.then15.i

if.then.loopexit.i:                               ; preds = %if.end22.preheader.i
  unreachable

if.then.i:                                        ; preds = %land.lhs.true.thread.i
  br label %if.end22.preheader.i

land.lhs.true13.i:                                ; preds = %for.body.i.thread
  %0 = phi ptr [ %1, %for.body.i.thread ]
  %call14.i = call zeroext i8 @_Z11file_existsPKc(ptr %0)
  %tobool.i = icmp eq i8 %call14.i, 0
  br i1 %tobool.i, label %if.then15.i, label %land.lhs.true13.i.if.else17.i.loopexit_crit_edge

if.then15.i:                                      ; preds = %land.lhs.true13.i, %land.lhs.true13.i.preheader
  %indvars.iv.next.i101923 = phi i64 [ undef, %land.lhs.true13.i.preheader ], [ undef, %land.lhs.true13.i ]
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.next.i101923, 1
  br label %for.body.i.thread

for.body.i.thread:                                ; preds = %if.then15.i
  %arrayidx.i17 = getelementptr inbounds ptr, ptr undef, i64 %indvars.iv.next.i
  %1 = load ptr, ptr %arrayidx.i17, align 8
  br i1 undef, label %land.lhs.true13.i, label %for.body.i.thread.if.else17.i.loopexit_crit_edge

for.body.i.thread.if.else17.i.loopexit_crit_edge: ; preds = %for.body.i.thread
  %split = phi ptr [ %1, %for.body.i.thread ]
  unreachable

land.lhs.true13.i.if.else17.i.loopexit_crit_edge: ; preds = %land.lhs.true13.i
  %split25 = phi ptr [ %0, %land.lhs.true13.i ]
  unreachable

if.else17.i:                                      ; preds = %if.end22.preheader.i, %for.body.thread.i
  unreachable

if.end22.preheader.i:                             ; preds = %if.then.i
  switch i8 0, label %if.else17.i [
    i8 0, label %if.then.loopexit.i
    i8 1, label %land.lhs.true13.i.preheader
  ]
}

declare zeroext i8 @_Z11file_existsPKc(ptr) local_unnamed_addr

