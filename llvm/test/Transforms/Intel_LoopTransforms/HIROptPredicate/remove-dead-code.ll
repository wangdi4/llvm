; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; Check that DCE successfully removes code after switch(%3). Otherwise
; Predicate Opt will form a loop for each case, causing cg to assert upon
; seeing unused temps.

;  HIR before Opt-Predicate
;  BEGIN REGION { }
;        + DO i1 = 0, 8, 1   <DO_LOOP>
;        |   switch(trunc.i32.i8(%0))
;        |   {
;        |   case 2:
;        |      %1 = (@c)[0];
;        |      (@g)[0] = %1;
;        |      %2 = %1;
;        |      break;
;        |   case 8:
;        |      %.pre = (@c)[0];
;        |      %2 = %.pre;
;        |      break;
;        |   default:
;        |      %3 = (@a)[0];
;        |      switch(%3)
;        |      {
;        |      case 0:
;        |         (@b)[0] = 7;
;        |         goto for.inc;
;        |      case 2:
;        |         (@c)[0] = 5;
;        |         goto for.inc;
;        |      default:
;        |         goto for.inc;
;        |      }
;        |      break;
;        |   }
;        |   %or = %2  |  3;
;        |   (@c)[0] = %or;
;        |   %call = @_Z4copyj(3);
;        |   for.inc:
;        + END LOOP
;  END REGION

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT:  switch(trunc.i32.i8(%0))
;              {
;              case 8:
;                 + DO i1 = 0, 8, 1   <DO_LOOP>
;                 |   %.pre = (@c)[0];
;                 |   %2 = %.pre;
;                 |   %or = %2  |  3;
;                 |   (@c)[0] = %or;
;                 |   %call = @_Z4copyj(3);
;                 + END LOOP
;                 break;
;              case 2:
;                 + DO i1 = 0, 8, 1   <DO_LOOP>
;                 |   %1 = (@c)[0];
;                 |   (@g)[0] = %1;
;                 |   %2 = %1;
;                 |   %or = %2  |  3;
;                 |   (@c)[0] = %or;
;                 |   %call = @_Z4copyj(3);
;                 + END LOOP
;                 break;
;              default:
;                 + DO i1 = 0, 8, 1   <DO_LOOP>
;                 |   %3 = (@a)[0];
; CHECK:          |   switch(%3)
; CHECK-NOT: for.inc
;                 |   {
;                 |   case 0:
;                 |      (@b)[0] = 7;
;                 |      break;
;                 |   case 2:
;                 |      (@c)[0] = 5;
;                 |      break;
;                 |   default:
;                 |      break;
;                 |   }
;                 + END LOOP
;                 break;
;              }
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@g = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @c, align 4, !tbaa !3
  %sext = shl i32 %0, 24
  %conv1 = ashr exact i32 %sext, 24
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %f.09 = phi i32 [ 24, %entry ], [ %inc, %for.inc ]
  switch i32 %conv1, label %sw.default [
    i32 2, label %sw.bb
    i32 8, label %for.body.sw.bb2_crit_edge
  ]

for.body.sw.bb2_crit_edge:                        ; preds = %for.body
  %.pre = load i32, ptr @c, align 4, !tbaa !3
  br label %sw.bb2

sw.bb:                                            ; preds = %for.body
  %1 = load i32, ptr @c, align 4, !tbaa !3
  store i32 %1, ptr @g, align 4, !tbaa !3
  br label %sw.bb2

sw.bb2:                                           ; preds = %for.body.sw.bb2_crit_edge, %sw.bb
  %2 = phi i32 [ %.pre, %for.body.sw.bb2_crit_edge ], [ %1, %sw.bb ]
  %or = or i32 %2, 3
  store i32 %or, ptr @c, align 4, !tbaa !3
  %call = tail call i32 @_Z4copyj(i32 3)
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %3 = load i32, ptr @a, align 4, !tbaa !3
  switch i32 %3, label %for.inc [
    i32 0, label %sw.bb3
    i32 2, label %sw.bb4
  ]

sw.bb3:                                           ; preds = %sw.default
  store i32 7, ptr @b, align 4, !tbaa !3
  br label %for.inc

sw.bb4:                                           ; preds = %sw.default
  store i32 5, ptr @c, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %sw.bb2, %sw.bb4, %sw.default, %sw.bb3
  %inc = add nuw nsw i32 %f.09, 1
  %exitcond.not = icmp eq i32 %inc, 33
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.inc
  ret i32 0
}

declare dso_local i32 @_Z4copyj(i32) local_unnamed_addr #1

attributes #0 = { norecurse uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
