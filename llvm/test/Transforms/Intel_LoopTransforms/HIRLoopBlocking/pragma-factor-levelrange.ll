; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that loop blocking pragma triggers for only 2 levels

; Source
;  #pragma block_loop factor(256) level(2:3)
;  for (int i = 0; i <1000; i++)
;    for (int j = 0; j <1000; j++)
;      for (int k = 0; k <1000; k++)
;        for (int ii = 0; ii <1000; ii++)
;          for (int jj = 0; jj <1000; jj++)
;          A[i][j][k][ii][jj] += B[i][j][k][ii][jj] * C[jj];

; HIR After
;         + DO i1 = 0, 3, 1   <DO_LOOP>
;         |   %min = (-256 * i1 + 999 <= 255) ? -256 * i1 + 999 : 255;
;         |
;         |   + DO i2 = 0, 3, 1   <DO_LOOP>
;         |   |   %min6 = (-256 * i2 + 999 <= 255) ? -256 * i2 + 999 : 255;
;         |   |
;         |   |   + DO i3 = 0, 999, 1   <DO_LOOP>
;         |   |   |   + DO i4 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 256>
;         |   |   |   |   + DO i5 = 0, %min6, 1   <DO_LOOP>  <MAX_TC_EST = 256>
;         |   |   |   |   |   + DO i6 = 0, 999, 1   <DO_LOOP>
;         |   |   |   |   |   |   + DO i7 = 0, 999, 1   <DO_LOOP>
;         |   |   |   |   |   |   |   %mul = (%C)[i7]  *  (@B)[0][i3][256 * i1 + i4][256 * i2 + i5][i6][i7];
;         |   |   |   |   |   |   |   %add = (@A)[0][i3][256 * i1 + i4][256 * i2 + i5][i6][i7]  +  %mul;
;         |   |   |   |   |   |   |   (@A)[0][i3][256 * i1 + i4][256 * i2 + i5][i6][i7] = %add;
;         |   |   |   |   |   |   + END LOOP
;         |   |   |   |   |   + END LOOP
;         |   |   |   |   + END LOOP
;         |   |   |   + END LOOP
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP

; CHECK:  BEGIN REGION { modified }
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i2 = 0, 3, 1
; CHECK: DO i3 = 0, 999, 1
; CHECK-NEXT: DO i4
; CHECK-NEXT: DO i5
; CHECK-NEXT: DO i6 = 0, 999, 1
; CHECK-NEXT: DO i7 = 0, 999, 1


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = external dso_local local_unnamed_addr global [1000 x [1000 x [1000 x [1000 x [1000 x double]]]]], align 16
@A = external dso_local local_unnamed_addr global [1000 x [1000 x [1000 x [1000 x [1000 x double]]]]], align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr noalias nocapture readonly %C) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 256), "QUAL.PRAGMA.LEVEL"(i32 3), "QUAL.PRAGMA.FACTOR"(i32 256) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv83 = phi i64 [ 0, %entry ], [ %indvars.iv.next84, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv80 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next81, %for.cond.cleanup7 ]
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next84 = add nuw nsw i64 %indvars.iv83, 1
  %exitcond85 = icmp eq i64 %indvars.iv.next84, 1000
  br i1 %exitcond85, label %for.end47, label %for.cond1.preheader

for.cond9.preheader:                              ; preds = %for.cond5.preheader, %for.cond.cleanup11
  %indvars.iv77 = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next78, %for.cond.cleanup11 ]
  br label %for.cond13.preheader

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next81 = add nuw nsw i64 %indvars.iv80, 1
  %exitcond82 = icmp eq i64 %indvars.iv.next81, 1000
  br i1 %exitcond82, label %for.cond.cleanup3, label %for.cond5.preheader

for.cond13.preheader:                             ; preds = %for.cond9.preheader, %for.cond.cleanup15
  %indvars.iv74 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next75, %for.cond.cleanup15 ]
  br label %for.body16

for.cond.cleanup11:                               ; preds = %for.cond.cleanup15
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next78, 1000
  br i1 %exitcond79, label %for.cond.cleanup7, label %for.cond9.preheader

for.cond.cleanup15:                               ; preds = %for.body16
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond76 = icmp eq i64 %indvars.iv.next75, 1000
  br i1 %exitcond76, label %for.cond.cleanup11, label %for.cond13.preheader

for.body16:                                       ; preds = %for.cond13.preheader, %for.body16
  %indvars.iv = phi i64 [ 0, %for.cond13.preheader ], [ %indvars.iv.next, %for.body16 ]
  %arrayidx24 = getelementptr inbounds [1000 x [1000 x [1000 x [1000 x [1000 x double]]]]], ptr @B, i64 0, i64 %indvars.iv83, i64 %indvars.iv80, i64 %indvars.iv77, i64 %indvars.iv74, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, ptr %arrayidx24, align 8, !tbaa !2
  %ptridx = getelementptr inbounds double, ptr %C, i64 %indvars.iv
  %2 = load double, ptr %ptridx, align 8, !tbaa !11
  %mul = fmul fast double %2, %1
  %arrayidx35 = getelementptr inbounds [1000 x [1000 x [1000 x [1000 x [1000 x double]]]]], ptr @A, i64 0, i64 %indvars.iv83, i64 %indvars.iv80, i64 %indvars.iv77, i64 %indvars.iv74, i64 %indvars.iv, !intel-tbaa !2
  %3 = load double, ptr %arrayidx35, align 8, !tbaa !2
  %add = fadd fast double %3, %mul
  store double %add, ptr %arrayidx35, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup15, label %for.body16

for.end47:                                        ; preds = %for.cond.cleanup3
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !8, i64 0}
!3 = !{!"array@_ZTSA1000_A1000_A1000_A1000_A1000_d", !4, i64 0}
!4 = !{!"array@_ZTSA1000_A1000_A1000_A1000_d", !5, i64 0}
!5 = !{!"array@_ZTSA1000_A1000_A1000_d", !6, i64 0}
!6 = !{!"array@_ZTSA1000_A1000_d", !7, i64 0}
!7 = !{!"array@_ZTSA1000_d", !8, i64 0}
!8 = !{!"double", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!8, !8, i64 0}
