; REQUIRES: asserts
; RUN: opt %s -passes='lto<O3>' -whole-program-assume -print-anders-alias-queries 2>&1 | FileCheck %s

; This test is checking that Andersen's AA is used in aa-pipeline at "lto<O3>".
; Just checking dump of Andersen's query.

; CHECK-LABEL: Alias_Begin
; CHECK-NEXT: Loc 1:
; CHECK-NEXT: Loc 2:
; CHECK-DAG: Node {{.*}}: foo:a
; CHECK-DAG: Node {{.*}}: foo:b
; CHECK-NEXT: Result: NoAlias
; CHECK-NEXT:  Alias_End


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr

; Function Attrs: noinline norecurse nounwind uwtable
define internal i32 @foo(float* nocapture %a, float* nocapture readonly %b, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %sub = add nsw i32 %n, -1
  %idxprom3 = sext i32 %sub to i64
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %idxprom3
  %0 = load float, float* %arrayidx4, align 4
  %conv5 = fptosi float %0 to i32
  ret i32 %conv5

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to float
  %add = fadd fast float %1, %conv
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %add, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr {
entry:
  %conv.i = sext i32 %argc to i64
  %mul.i = shl nsw i64 %conv.i, 2
  %call.i = tail call noalias i8* @malloc(i64 %mul.i)
  %0 = bitcast i8* %call.i to float*
  %call.i13 = tail call noalias i8* @malloc(i64 %mul.i)
  %1 = bitcast i8* %call.i13 to float*
  %call2 = tail call i32 @foo(float* %0, float* %1, i32 %argc)
  %call4 = tail call i32 @foo(float* null, float* null, i32 %argc)
  %add = add nsw i32 %call4, %call2
  ret i32 %add
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "loopopt-pipeline"="full" }
