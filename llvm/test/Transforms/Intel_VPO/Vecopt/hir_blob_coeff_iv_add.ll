; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-predicate -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


; Verify that test case compiles successfully.
; CanonExprUtils::add() was failing while trying to create blob coeff (3 + (3 * %0)) for i1.

; Incoming HIR for vectorizer-
; + DO i1 = 0, 8, 1   <DO_LOOP>
; |   if (i1 + 1 <u 9)
; |   {
; |      if (%0 == i1 + 1)
; |      {
; |         %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; |
; |         + DO i2 = 0, -1 * i1 + 7, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; |         |   %2 = (@b)[0][1];
; |         |   %3 = 3 * %0 * i1 + 3 * %0 * i2 + 3 * %0  +  3 * i1 + 3 * i2 + 3;
; |         |   (@d)[0] = 3 * ((-1 * %3) + %2);
; |         + END LOOP
; |
; |         @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; |      }
; |   }
; + END LOOP

; CHECK: |            + DO i2 = 0, {{.*}}, 4   <DO_LOOP>  <MAX_TC_EST = 2> <auto-vectorized> <nounroll> <novectorize>
; CHECK: |            |   %.unifload = (i32*)(@b)[0][1];
; CHECK: |            |   [[VEC_IV:%.*]] = 3 * %0  *  i2 + <i32 0, i32 1, i32 2, i32 3>;
; CHECK: |            |   [[SCAL_IV:%.*]] = 3 * %0  *  i2;
; CHECK: |            |   [[VEC1:%.*]] = (3 + (3 * %0)) * i1 + 3 * i2 + 3 * %0 + 3 * <i32 0, i32 1, i32 2, i32 3> + [[VEC_IV]] + 3  *  -1;
; CHECK: |            |   [[SCAL1:%.*]] = (3 + (3 * %0)) * i1 + 3 * i2 + 3 * %0 + [[SCAL_IV]] + 3  *  -1;
; CHECK: |            |   [[VEC2:%.*]] = %.unifload + [[VEC1]]  *  3;
; CHECK: |            |   [[SCAL2:%.*]] = %.unifload + [[SCAL1]]  *  3;
; CHECK: |            |   [[EXTRACT:%.*]] = extractelement [[VEC2]],  3;
; CHECK: |            |   (@d)[0] = [[EXTRACT]];
; CHECK: |            + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global i32 0, align 4
@d = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global [1 x i32] zeroinitializer, align 4

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  store i32 1, i32* @a, align 4, !tbaa !2
  %0 = load i32, i32* @c, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.end
  %storemerge26 = phi i32 [ 1, %entry ], [ %inc13, %for.end ]
  %1 = icmp ult i32 %storemerge26, 9
  br i1 %1, label %for.body5.lr.ph, label %for.end

for.body5.lr.ph:                                  ; preds = %for.body
  %cmp6 = icmp eq i32 %0, %storemerge26
  br label %for.body5

for.body5:                                        ; preds = %for.body5.lr.ph, %if.end
  %indvars.iv = phi i32 [ %storemerge26, %for.body5.lr.ph ], [ %indvars.iv.next, %if.end ]
  br i1 %cmp6, label %if.then, label %if.end

if.then:                                          ; preds = %for.body5
  %mul = mul nuw nsw i32 %indvars.iv, 3
  %2 = load i32, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @b, i64 1, i64 0), align 4, !tbaa !2
  %mul9 = mul nsw i32 %0, %mul
  %3 = add i32 %mul9, %mul
  %sub10 = sub i32 %2, %3
  %mul11 = mul nsw i32 %sub10, 3
  store i32 %mul11, i32* @d, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body5
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 9
  br i1 %exitcond, label %for.end.loopexit, label %for.body5

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %inc13 = add nuw nsw i32 %storemerge26, 1
  %exitcond27 = icmp eq i32 %inc13, 10
  br i1 %exitcond27, label %for.end14, label %for.body

for.end14:                                        ; preds = %for.end
  store i32 10, i32* @a, align 4, !tbaa !2
  ret i32 0
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
