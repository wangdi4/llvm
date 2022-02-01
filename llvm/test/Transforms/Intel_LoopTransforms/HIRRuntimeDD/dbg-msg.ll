; RUN: opt -hir-runtime-dd-dbg -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -hir-runtime-dd-dbg -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; UNSUPPORTED: system-windows

; RUN: opt -force-hir-cg -hir-runtime-dd-dbg -hir-ssa-deconstruction -hir-runtime-dd -hir-cg < %s | lli | FileCheck %s --check-prefix=RUNTIME
; RUN: opt -force-hir-cg -hir-runtime-dd-dbg -passes="hir-ssa-deconstruction,hir-runtime-dd,hir-cg" -aa-pipeline="basic-aa" < %s | lli | FileCheck %s --check-prefix=RUNTIME

; Check that -hir-runtime-dd-dbg adds a call to 'puts'.

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>
;       |   %0 = (%q)[i1];
;       |   (%p)[i1] = %0 + 1;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:       %mv.test = &((%q)[zext.i32.i64(%n) + -1]) >=u &((%p)[0]);
; CHECK:       %mv.test1 = &((%p)[zext.i32.i64(%n) + -1]) >=u &((%q)[0]);
; CHECK:       %mv.and = %mv.test  &  %mv.test1;
; CHECK:       if (%mv.and == 0)
; CHECK:       {
; CHECK:          %call = @puts(&((@hir.str)[0][0]));
;
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 13>
; CHECK:          |   %0 = (%q)[i1];
; CHECK:          |   (%p)[i1] = %0 + 1;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          %call2 = @puts(&((@hir.str.1)[0][0]));
;
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 13> <nounroll> <novectorize>
; CHECK:          |   %0 = (%q)[i1];
; CHECK:          |   (%p)[i1] = %0 + 1;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

; RUNTIME: [RTDD]: foo(), <Loop {{[0-9]*}}>: OK
; RUNTIME: [RTDD]: foo(), <Loop {{[0-9]*}}>: FAILED

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(i32* nocapture %p, i32* nocapture readonly %q, i32 %n) {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count9 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count9
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() {
entry:
  tail call void @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 10), i32 2)
  tail call void @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 10), i32 11)
  ret i32 0
}

