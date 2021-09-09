; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-cross-loop-array-contraction -hir-cg -force-hir-cg -print-after=hir-cross-loop-array-contraction -disable-output -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>,hir-cg" -force-hir-cg -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that i2 in def loop is correctly mapped to %mod and i2+1 in the use
; loop for contraction.

; %mod is moved to the beginning of the merged loop as it is used by one of the
; merged def loop.

; The temp %mod1 is renamed in the merged def loops to %temp and %temp12.
; This is needed otherwise the definitions would collide when adjusting IV by
; mapped CE resulting in incorrect code.

; Incoming HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   %mod1 = i1  %  %n;
; |
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; |   |   |   + DO i4 = 0, 9, 1   <DO_LOOP>
; |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; |   |   |   |   |   %ld = (@C)[0][%mod1];
; |   |   |   |   |   (%A)[0][i1][i2][i3][i4][i5] = i1 + %ld;
; |   |   |   |   + END LOOP
; |   |   |   + END LOOP
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP
;
;
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   do.body:
; |
; |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>
; |   |   %mod = i2 + 1  %  %n;
; |   |
; |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; |   |   |   |   |   |   %t4 = (%A)[0][%mod][i3][i4][i6][i5];
; |   |   |   |   |   |   %t5 = (%A)[0][i2 + 1][i3][i4][i6][i5];
; |   |   |   |   |   |   (@B)[0][i2][i3][i4][i5][i6] = %t4 + %t5;
; |   |   |   |   |   + END LOOP
; |   |   |   |   + END LOOP
; |   |   |   + END LOOP
; |   |   + END LOOP
; |   + END LOOP
; |
; |   %3 = (%b)[0];
; |   if (%3 != 0)
; |   {
; |      <i1 = i1 + 1>
; |      goto do.body;
; |   }
; + END LOOP


; *** Region after HIR-CrossLoop Transformation with Array Contraction ***

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.body:
; CHECK: |
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>
; CHECK: |   |   %mod = i2 + 1  %  %n;
; CHECK: |   |   %temp12 = i2 + 1  %  %n;
; CHECK: |   |   %temp = %mod  %  %n;
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   %ld = (@C)[0][%temp12];
; CHECK: |   |   |   |   |   |   (%ContractedArray11)[0][i5][i6] = i2 + %ld + 1;
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   |
; CHECK: |   |   |   |
; CHECK: |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   %ld = (@C)[0][%temp];
; CHECK: |   |   |   |   |   |   (%ContractedArray)[0][i5][i6] = %ld + %mod;
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   |
; CHECK: |   |   |   |
; CHECK: |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   %t4 = (%ContractedArray)[0][i6][i5];
; CHECK: |   |   |   |   |   |   %t5 = (%ContractedArray11)[0][i6][i5];
; CHECK: |   |   |   |   |   |   (@B)[0][i2][i3][i4][i5][i6] = %t4 + %t5;
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %3 = (%b)[0];
; CHECK: |   if (%3 != 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.body;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100 x [100 x [100 x [10 x [10 x i32]]]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @shell(i32* nocapture readonly %b, i64 %n) local_unnamed_addr #0 {
entry:
  %A = alloca [100 x [100 x [100 x [10 x [10 x i32]]]]], align 16
  %0 = bitcast [100 x [100 x [100 x [10 x [10 x i32]]]]]* %A to i8*
  call void @llvm.lifetime.start.p0i8(i64 400000000, i8* nonnull %0) #2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv170 = phi i64 [ 0, %entry ], [ %indvars.iv.next171, %for.cond.cleanup3 ]
  %mod1 = srem i64 %indvars.iv170, %n
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv167 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next168, %for.cond.cleanup7 ]
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next171 = add nuw nsw i64 %indvars.iv170, 1
  %exitcond172 = icmp eq i64 %indvars.iv.next171, %n
  br i1 %exitcond172, label %do.body.preheader, label %for.cond1.preheader

do.body.preheader:                                ; preds = %for.cond.cleanup3
  br label %do.body

for.cond9.preheader:                              ; preds = %for.cond5.preheader, %for.cond.cleanup11
  %indvars.iv164 = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next165, %for.cond.cleanup11 ]
  br label %for.cond13.preheader

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next168 = add nuw nsw i64 %indvars.iv167, 1
  %exitcond169 = icmp eq i64 %indvars.iv.next168, 100
  br i1 %exitcond169, label %for.cond.cleanup3, label %for.cond5.preheader

for.cond13.preheader:                             ; preds = %for.cond9.preheader, %for.cond.cleanup15
  %indvars.iv161 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next162, %for.cond.cleanup15 ]
  br label %for.body16

for.cond.cleanup11:                               ; preds = %for.cond.cleanup15
  %indvars.iv.next165 = add nuw nsw i64 %indvars.iv164, 1
  %exitcond166 = icmp eq i64 %indvars.iv.next165, 100
  br i1 %exitcond166, label %for.cond.cleanup7, label %for.cond9.preheader

for.cond.cleanup15:                               ; preds = %for.body16
  %indvars.iv.next162 = add nuw nsw i64 %indvars.iv161, 1
  %exitcond163 = icmp eq i64 %indvars.iv.next162, 10
  br i1 %exitcond163, label %for.cond.cleanup11, label %for.cond13.preheader

for.body16:                                       ; preds = %for.cond13.preheader, %for.body16
  %indvars.iv157 = phi i64 [ 0, %for.cond13.preheader ], [ %indvars.iv.next158, %for.body16 ]
  %gep = getelementptr inbounds [100 x i64], [100 x i64]* @C, i64 0, i64 %mod1
  %ld = load i64, i64* %gep, align 4
  %1 = add nuw nsw i64 %indvars.iv170, %ld
  %arrayidx24 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], [100 x [100 x [100 x [10 x [10 x i32]]]]]* %A, i64 0, i64 %indvars.iv170, i64 %indvars.iv167, i64 %indvars.iv164, i64 %indvars.iv161, i64 %indvars.iv157, !intel-tbaa !2
  %2 = trunc i64 %1 to i32
  store i32 %2, i32* %arrayidx24, align 4, !tbaa !2
  %indvars.iv.next158 = add nuw nsw i64 %indvars.iv157, 1
  %exitcond160 = icmp eq i64 %indvars.iv.next158, 10
  br i1 %exitcond160, label %for.cond.cleanup15, label %for.body16

do.body:                                          ; preds = %do.body.preheader, %for.cond.cleanup40
  br label %for.cond43.preheader

for.cond43.preheader:                             ; preds = %do.body, %for.cond.cleanup45
  %indvars.iv154 = phi i64 [ 0, %do.body ], [ %indvars.iv.next155, %for.cond.cleanup45 ]
  %iv.add = add nsw i64 %indvars.iv154, 1
  %mod = srem i64 %iv.add, %n
  br label %for.cond48.preheader

for.cond.cleanup40:                               ; preds = %for.cond.cleanup45
  %3 = load i32, i32* %b, align 4, !tbaa !11
  %tobool = icmp eq i32 %3, 0
  br i1 %tobool, label %do.end, label %do.body

for.cond48.preheader:                             ; preds = %for.cond43.preheader, %for.cond.cleanup50
  %indvars.iv151 = phi i64 [ 0, %for.cond43.preheader ], [ %indvars.iv.next152, %for.cond.cleanup50 ]
  br label %for.cond53.preheader

for.cond.cleanup45:                               ; preds = %for.cond.cleanup50
  %indvars.iv.next155 = add nuw nsw i64 %indvars.iv154, 1
  %exitcond156 = icmp eq i64 %indvars.iv.next155, %n
  br i1 %exitcond156, label %for.cond.cleanup40, label %for.cond43.preheader

for.cond53.preheader:                             ; preds = %for.cond48.preheader, %for.cond.cleanup55
  %indvars.iv148 = phi i64 [ 0, %for.cond48.preheader ], [ %indvars.iv.next149, %for.cond.cleanup55 ]
  br label %for.cond58.preheader

for.cond.cleanup50:                               ; preds = %for.cond.cleanup55
  %indvars.iv.next152 = add nuw nsw i64 %indvars.iv151, 1
  %exitcond153 = icmp eq i64 %indvars.iv.next152, 100
  br i1 %exitcond153, label %for.cond.cleanup45, label %for.cond48.preheader

for.cond58.preheader:                             ; preds = %for.cond53.preheader, %for.cond.cleanup60
  %indvars.iv145 = phi i64 [ 0, %for.cond53.preheader ], [ %indvars.iv.next146, %for.cond.cleanup60 ]
  br label %for.body61

for.cond.cleanup55:                               ; preds = %for.cond.cleanup60
  %indvars.iv.next149 = add nuw nsw i64 %indvars.iv148, 1
  %exitcond150 = icmp eq i64 %indvars.iv.next149, 100
  br i1 %exitcond150, label %for.cond.cleanup50, label %for.cond53.preheader

for.cond.cleanup60:                               ; preds = %for.body61
  %indvars.iv.next146 = add nuw nsw i64 %indvars.iv145, 1
  %exitcond147 = icmp eq i64 %indvars.iv.next146, 10
  br i1 %exitcond147, label %for.cond.cleanup55, label %for.cond58.preheader

for.body61:                                       ; preds = %for.cond58.preheader, %for.body61
  %indvars.iv = phi i64 [ 0, %for.cond58.preheader ], [ %indvars.iv.next, %for.body61 ]
  %arrayidx71 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], [100 x [100 x [100 x [10 x [10 x i32]]]]]* %A, i64 0, i64 %mod, i64 %indvars.iv151, i64 %indvars.iv148, i64 %indvars.iv, i64 %indvars.iv145, !intel-tbaa !2
  %t4 = load i32, i32* %arrayidx71, align 4, !tbaa !2
  %arrayidx711 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], [100 x [100 x [100 x [10 x [10 x i32]]]]]* %A, i64 0, i64 %iv.add, i64 %indvars.iv151, i64 %indvars.iv148, i64 %indvars.iv, i64 %indvars.iv145, !intel-tbaa !2
  %t5 = load i32, i32* %arrayidx711, align 4, !tbaa !2
  %add72 = add nsw i32 %t4, %t5
  %arrayidx82 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], [100 x [100 x [100 x [10 x [10 x i32]]]]]* @B, i64 0, i64 %indvars.iv154, i64 %indvars.iv151, i64 %indvars.iv148, i64 %indvars.iv145, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add72, i32* %arrayidx82, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup60, label %for.body61

do.end:                                           ; preds = %for.cond.cleanup40
  call void @llvm.lifetime.end.p0i8(i64 400000000, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !8, i64 0}
!3 = !{!"array@_ZTSA100_A100_A100_A10_A10_i", !4, i64 0}
!4 = !{!"array@_ZTSA100_A100_A10_A10_i", !5, i64 0}
!5 = !{!"array@_ZTSA100_A10_A10_i", !6, i64 0}
!6 = !{!"array@_ZTSA10_A10_i", !7, i64 0}
!7 = !{!"array@_ZTSA10_i", !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!8, !8, i64 0}
