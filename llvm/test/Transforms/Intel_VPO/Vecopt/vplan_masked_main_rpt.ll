;
; Check the opt-report correctly attached to main maksed loop.
;
; RUN: opt -passes="vplan-vec,intel-ir-optreport-emitter" -intel-opt-report=high -vplan-enable-masked-main-loop -vplan-masked-main-cost-threshold=0 -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: LOOP BEGIN
;CHECK-NEXT:    remark #15301: SIMD LOOP WAS VECTORIZED
;CHECK-NEXT:    remark #15305: vectorization support: vector length 4
;CHECK-NEXT:    remark #15475: --- begin vector loop cost summary ---
;CHECK-NEXT:    remark #15476: scalar cost:
;CHECK-NEXT:    remark #15477: vector cost:
;CHECK-NEXT:    remark #15478: estimated potential speedup:
;CHECK-NEXT:    remark #15309: vectorization support: normalized vectorization overhead
;CHECK-NEXT:    remark #15570: using scalar loop trip count: 3
;CHECK-NEXT:    remark #15488: --- end vector loop cost summary ---
;CHECK-NEXT:    remark #15447: --- begin vector loop memory reference summary ---
;CHECK-NEXT:    remark #15457: masked unaligned unit stride stores: 1
;CHECK-NEXT:    remark #15474: --- end vector loop memory reference summary ---
;CHECK-NEXT: LOOP END

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(ptr %arr1) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4)]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr1, i32 %indvars.iv
  store i32 %indvars.iv, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 3
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !5

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.vectorize.ignore_profitability"}
!7 = !{!"llvm.loop.vectorize.enable", i1 true}
