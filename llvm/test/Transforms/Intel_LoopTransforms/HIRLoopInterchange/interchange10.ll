; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" < %s -debug-only=hir-loop-interchange 2>&1 | FileCheck %s
;
; Interchange is not legal for this test:
;    for (i = 12; i < 42; i++) {
;      for (j = 26; j < 42; ++j) {
;        for (k = 0; k < 42; k++) {
;	  (a[(0)][(i)][(k)] ^= k);
;	  (a[(k)][(i)][(j)] &= 85); } } }
;
; CHECK-NOT: Interchanged
;
; ModuleID = 'atg_cmplrs-5544.cpp'
source_filename = "atg_cmplrs-5544.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [42 x [42 x [42 x i32]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @_Z3subv() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end16
  %indvars.iv36 = phi i64 [ 12, %entry ], [ %indvars.iv.next37, %for.end16 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc14
  %j.033 = phi i64 [ 26, %for.cond1.preheader ], [ %inc15, %for.inc14 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [42 x [42 x [42 x i32]]], ptr @a, i64 0, i64 0, i64 %indvars.iv36, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx8, align 4, !tbaa !2
  %1 = trunc i64 %indvars.iv to i32
  %xor = xor i32 %0, %1
  store i32 %xor, ptr %arrayidx8, align 4, !tbaa !2
  %arrayidx13 = getelementptr inbounds [42 x [42 x [42 x i32]]], ptr @a, i64 0, i64 %indvars.iv, i64 %indvars.iv36, i64 %j.033, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx13, align 4, !tbaa !2
  %and = and i32 %2, 85
  store i32 %and, ptr %arrayidx13, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 42
  br i1 %exitcond, label %for.inc14, label %for.body6

for.inc14:                                        ; preds = %for.body6
  %inc15 = add nuw nsw i64 %j.033, 1
  %exitcond35 = icmp eq i64 %inc15, 42
  br i1 %exitcond35, label %for.end16, label %for.cond4.preheader

for.end16:                                        ; preds = %for.inc14
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next37, 42
  br i1 %exitcond38, label %for.end19, label %for.cond1.preheader

for.end19:                                        ; preds = %for.end16
  ret i32 0
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !6, i64 0}
!3 = !{!"array@_ZTSA42_A42_A42_j", !4, i64 0}
!4 = !{!"array@_ZTSA42_A42_j", !5, i64 0}
!5 = !{!"array@_ZTSA42_j", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}

