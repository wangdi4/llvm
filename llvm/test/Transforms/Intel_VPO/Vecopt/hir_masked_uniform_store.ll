; LLVM IR generated from testcase below using icx -O2 -mllvm -print-module-before-loopopt
; int main() {
;   unsigned k;
;   int i, j;
;   int ar[42] = {0}, yarrrr[42] = {0};
;   for (i = 1; i < 42; i++) {
;     ar[0]++;
;     for (j = 1; j < i; j++)
;       //#pragma omp simd
;       for (k = 1; k < j; k++)
;         ar[i] = yarrrr[k - 1];
;   }
;   return 0;
; }

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -vplan-force-vf=4 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MIXED
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-VPVAL

; Incoming HIR into the vectorizer is expected to look like:
;
;<0>          BEGIN REGION { }
;<46>               + DO i1 = 0, 40, 1   <DO_LOOP>
;<2>                |   %2 = (%ar)[0][0];
;<4>                |   (%ar)[0][0] = %2 + 1;
;<48>               |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;<47>               |
;<47>               |   + DO i2 = 0, i1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 40>
;<16>               |   |   if (i2 + 1 >u 1)
;<16>               |   |   {
;<24>               |   |      (%ar)[0][i1 + 1] = (%yarrrr)[0][i2 + -1];
;<16>               |   |   }
;<47>               |   + END LOOP
;<47>               |
;<49>               |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;<46>               + END LOOP
;<0>          END REGION
;
; The store to %ar array inside the i2 loop is to a loop invariant address.
; Current code generation does not generate efficient code for masked uniform
; stores. The loop is vectorized, but the inefficient scatter instruction is
; generated. This test checks for the corresponding HIR and LLVM-IR generated.
; HIR checks specifically test that the load from %yarrrr and store to %ar
; are under a mask. LLVM IR checks specifically test that a scatter gets
; generated.

; Check HIR
; CHECK: DO i2 = 0, 4 * {{%.*}} + -1, 4   <DO_LOOP>
; CHECK-MIXED-NEXT: [[Mask:%.*]] = i2 + <i32 0, i32 1, i32 2, i32 3> + 1 >u 1;
; CHECK-MIXED: [[Load:%.*]] = (<4 x i32>*)(%yarrrr)[0][i2 + <i32 0, i32 1, i32 2, i32 3> + -1]; Mask = @{[[Mask]]}
; CHECK-MIXED-NEXT: (<4 x i32>*)(%ar)[0][i1 + 1] = [[Load]]; Mask = @{[[Mask]]}
; CHECK-VPVAL: [[Mask:%.*]] = {{.*}} >u 1;
; CHECK-VPVAL: [[Load:%.*]] = (<4 x i32>*)(%yarrrr)[0][{{.*}}]; Mask = @{[[Mask]]}
; CHECK-VPVAL: (<4 x i32>*)(%ar)[0][{{.*}}] = [[Load]]; Mask = @{[[Mask]]}
; CHECK: END LOOP

; Check LLVM-IR
; CHECK: [[CmpInst:%.*]] = icmp ugt <4 x i32> {{%.*}}, <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT: store <4 x i1> [[CmpInst]], <4 x i1>* [[Mask:%.*]]
; CHECK-DAG: call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> {{%.*}}, <4 x i32*> {{%.*}}, i32 4, <4 x i1> [[MaskLoad:%.*]])
; CHECK-DAG: [[MaskLoad]] = load <4 x i1>, <4 x i1>* [[Mask]]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %ar = alloca [42 x i32], align 16
  %yarrrr = alloca [42 x i32], align 16
  %0 = bitcast [42 x i32]* %ar to i8*
  call void @llvm.lifetime.start.p0i8(i64 168, i8* nonnull %0) #2
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %0, i8 0, i64 168, i1 false)
  %1 = bitcast [42 x i32]* %yarrrr to i8*
  call void @llvm.lifetime.start.p0i8(i64 168, i8* nonnull %1) #2
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %1, i8 0, i64 168, i1 false)
  %arrayidx = getelementptr inbounds [42 x i32], [42 x i32]* %ar, i64 0, i64 0
  br label %for.body

for.body:                                         ; preds = %for.inc14, %entry
  %indvars.iv38 = phi i64 [ 1, %entry ], [ %indvars.iv.next39, %for.inc14 ]
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc14 ]
  %2 = load i32, i32* %arrayidx, align 16, !tbaa !2
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %arrayidx, align 16, !tbaa !2
  %cmp229 = icmp ugt i64 %indvars.iv38, 1
  br i1 %cmp229, label %for.cond4.preheader.lr.ph, label %for.inc14

for.cond4.preheader.lr.ph:                        ; preds = %for.body
  %arrayidx9 = getelementptr inbounds [42 x i32], [42 x i32]* %ar, i64 0, i64 %indvars.iv38
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc11, %for.cond4.preheader.lr.ph
  %indvars.iv33 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next34, %for.inc11 ]
  %j.030 = phi i32 [ 1, %for.cond4.preheader.lr.ph ], [ %inc12, %for.inc11 ]
  %cmp527 = icmp ugt i32 %j.030, 1
  br i1 %cmp527, label %for.body6.preheader, label %for.inc11

for.body6.preheader:                              ; preds = %for.cond4.preheader
  %3 = add nuw i64 %indvars.iv33, 4294967295
  %idxprom = and i64 %3, 4294967295
  %arrayidx7 = getelementptr inbounds [42 x i32], [42 x i32]* %yarrrr, i64 0, i64 %idxprom
  %4 = load i32, i32* %arrayidx7, align 4, !tbaa !2
  store i32 %4, i32* %arrayidx9, align 4, !tbaa !2
  br label %for.inc11

for.inc11:                                        ; preds = %for.body6.preheader, %for.cond4.preheader
  %inc12 = add nuw nsw i32 %j.030, 1
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond = icmp eq i64 %indvars.iv.next34, %indvars.iv
  br i1 %exitcond, label %for.inc14.loopexit, label %for.cond4.preheader

for.inc14.loopexit:                               ; preds = %for.inc11
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.body
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next, 41
  br i1 %exitcond40, label %for.end16, label %for.body

for.end16:                                        ; preds = %for.inc14
  call void @llvm.lifetime.end.p0i8(i64 168, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 168, i8* nonnull %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c878848243a118d2154deca4033daa03379bc6ac) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4c1a43e30478ae94635b556b6b0d3e9ac4f69336)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA42_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
