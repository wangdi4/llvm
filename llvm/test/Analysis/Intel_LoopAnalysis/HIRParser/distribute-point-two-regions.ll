; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; The test is a couple of sibling loops where the first has a <distribute_point>.
; Check that two regions are recognized.

; CHECK: BEGIN REGION
; CHECK: <distribute_point>
; CHECK: END REGION

; CHECK: BEGIN REGION
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv36 = phi i64 [ 0, %entry ], [ %indvars.iv.next37, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %indvars.iv36
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @C, i64 0, i64 %indvars.iv36
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd float %0, %1
  %arrayidx4 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv36
  store float %add, float* %arrayidx4, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %arrayidx7 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv.next37
  %3 = load float, float* %arrayidx7, align 4
  %4 = trunc i64 %indvars.iv36 to i32
  %conv = sitofp i32 %4 to float
  %add8 = fadd float %3, %conv
  %arrayidx10 = getelementptr inbounds [100 x float], [100 x float]* @D, i64 0, i64 %indvars.iv36
  store float %add8, float* %arrayidx10, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %exitcond38 = icmp eq i64 %indvars.iv.next37, 100
  br i1 %exitcond38, label %for.body16.preheader, label %for.body

for.body16.preheader:                             ; preds = %for.body
  br label %for.body16

for.cond.cleanup15:                               ; preds = %for.body16
  ret void

for.body16:                                       ; preds = %for.body16.preheader, %for.body16
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body16 ], [ 0, %for.body16.preheader ]
  %arrayidx18 = getelementptr inbounds [100 x float], [100 x float]* @D, i64 0, i64 %indvars.iv
  %5 = load float, float* %arrayidx18, align 4
  %inc19 = fadd float %5, 1.000000e+00
  store float %inc19, float* %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.cond.cleanup15, label %for.body16
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

