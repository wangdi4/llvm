; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memset intrinsic is recognized by HIR Lower Small Memset/Memcpy pass
; and transformed to a loop. Also check that double constant DDref is correctly formed in loop.

; HIR before transformation:
;             BEGIN REGION { }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   @llvm.memset.p0.i64(&((i8*)(%arr)[0][i1][0]),  0,  32,  0);
;                  + END LOOP
;            END REGION


; HIR after transformation:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   |   (%arr)[0][i1][i2] = 0.000000e+00;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind memory(none) uwtable
define dso_local double @foo(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %arr = alloca [10 x [20 x double]], align 16
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %idxprom3 = sext i32 %n to i64
  %arrayidx4 = getelementptr inbounds [10 x [20 x double]], ptr %arr, i64 0, i64 3, i64 %idxprom3, !intel-tbaa !3
  %l1 = load double, ptr %arrayidx4, align 8, !tbaa !3
  ret double %l1

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds [10 x [20 x double]], ptr %arr, i64 0, i64 %indvars.iv, i64 0, !intel-tbaa !3
  %bc2 = bitcast ptr %arrayidx1 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(32) %bc2, i8 0, i64 32, i1 false)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !9
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { nofree nosync nounwind memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA10_A20_d", !5, i64 0}
!5 = !{!"array@_ZTSA20_d", !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
