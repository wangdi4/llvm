; RUN: opt -aa-pipeline="basic-aa,scoped-noalias-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that DSE eliminates the first store to (@b)[0][i1] inside the
; multiversioned 'if' loop using scoped no-alias metadata but does not eliminate
; it inside the 'else' loop which doesn't have the metadata.

; Print Before DSE-

; %mv.test = &((@b)[0][29]) >=u &((%call)[0]);
; %mv.test1 = &((%call)[29]) >=u &((@b)[0][0]);
; %mv.and = %mv.test  &  %mv.test1;
; if (%mv.and == 0)
; {
;    + DO i1 = 0, 29, 1   <DO_LOOP> <MVTag: 30>
;    |   %add.i = (@b)[0][i1]  +  2.000000e+00;
;    |   (@b)[0][i1] = %add.i;
;    |   %add6.i = (%call)[i1]  +  %add.i;
;    |   (@b)[0][i1] = %add6.i;
;    + END LOOP
; }
; else
; {
;    + DO i1 = 0, 29, 1   <DO_LOOP> <MVTag: 30> <nounroll> <novectorize>
;    |   %add.i = (@b)[0][i1]  +  2.000000e+00;
;    |   (@b)[0][i1] = %add.i;
;    |   %add6.i = (%call)[i1]  +  %add.i;
;    |   (@b)[0][i1] = %add6.i;
;    + END LOOP
; }

; Print After-

; CHECK: if (%mv.and == 0)
; CHECK-NOT: (@b)[0][i1] = %add.i;
; CHECK: (@b)[0][i1] = %add6.i;

; CHECK: else
; CHECK: (@b)[0][i1] = %add.i;
; CHECK: (@b)[0][i1] = %add6.i;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = external dso_local local_unnamed_addr global [100 x double], align 16
@str = external hidden unnamed_addr constant [12 x i8], align 1
@str.2 = external hidden unnamed_addr constant [12 x i8], align 1

define dso_local i32 @main() {
entry:
  %call = tail call ptr (...) @oops()
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ 0, %entry ], [ %indvars.iv.next.i, %for.body.i ]
  %arrayidx.i = getelementptr inbounds [100 x double], ptr @b, i64 0, i64 %indvars.iv.i, !intel-tbaa !2
  %t1 = load double, ptr %arrayidx.i, align 8, !tbaa !2
  %add.i = fadd fast double %t1, 2.000000e+00
  store double %add.i, ptr %arrayidx.i, align 8, !tbaa !2
  %ptridx.i = getelementptr inbounds double, ptr %call, i64 %indvars.iv.i
  %t2 = load double, ptr %ptridx.i, align 8, !tbaa !9
  %add6.i = fadd fast double %t2, %add.i
  store double %add6.i, ptr %arrayidx.i, align 8, !tbaa !2
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, 30
  br i1 %exitcond.not.i, label %foo.exit, label %for.body.i, !llvm.loop !10

foo.exit:                                         ; preds = %for.body.i
  %t3 = load double, ptr @b, align 16, !tbaa !2
  %sub = fadd fast double %t3, -4.000000e+00
  %cmp1 = fcmp fast ogt double %sub, 1.000000e-04
  %sub3 = fsub fast double 4.000000e+00, %t3
  %cmp4 = fcmp fast ogt double %sub3, 1.000000e-04
  %or.cond = or i1 %cmp1, %cmp4
  br i1 %or.cond, label %if.then, label %if.else

if.then:                                          ; preds = %foo.exit
  %puts15 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.2)
  br label %if.end

if.else:                                          ; preds = %foo.exit
  %puts = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret i32 0
}

declare dso_local ptr @oops(...)

declare noundef i32 @puts(ptr nocapture noundef readonly)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!4, !4, i64 0}
!10 = distinct !{!10, !8}
