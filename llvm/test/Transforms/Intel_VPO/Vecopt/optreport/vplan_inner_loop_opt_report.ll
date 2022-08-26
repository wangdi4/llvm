; Test to check the functionality of vectorization opt-report for
; outer loopnest with nested inner loop.

; RUN: opt -vplan-vec -vplan-force-vf=2 -intel-opt-report=high -intel-ir-optreport-emitter < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefixes=CHECK,IR
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -intel-opt-report=high -hir-optreport-emitter < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefixes=CHECK,HIR

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 2
; CHECK-NEXT:     remark #15475: --- begin vector loop cost summary ---
; IR-NEXT:        remark #15476: scalar cost: 7.000000
; IR-NEXT:        remark #15477: vector cost: 5.000000
; IR-NEXT:        remark #15478: estimated potential speedup: 1.390625
; IR-NEXT:        remark #15309: vectorization support: normalized vectorization overhead 0.000000
; HIR-NEXT:       remark #15476: scalar cost: 8.000000
; HIR-NEXT:       remark #15477: vector cost: 6.000000
; HIR-NEXT:       remark #15478: estimated potential speedup: 1.328125
; HIR-NEXT:       remark #15309: vectorization support: normalized vectorization overhead 0.156250
; CHECK-NEXT:     remark #15482: vectorized math library calls: 0
; CHECK-NEXT:     remark #15484: vector function calls: 0
; CHECK-NEXT:     remark #15485: serialized function calls: 1
; IR-NEXT:        remark #15558: Call to function 'serial_call_1' was serialized due to no suitable vector variants were found.
; CHECK-NEXT:     remark #15488: --- end vector loop cost summary ---

; CHECK:          LOOP BEGIN
; CHECK-NEXT:         remark #15475: --- begin vector loop cost summary ---
; CHECK-NEXT:         remark #15482: vectorized math library calls: 1
; CHECK-NEXT:         remark #15484: vector function calls: 0
; CHECK-NEXT:         remark #15485: serialized function calls: 0
; CHECK-NEXT:         remark #15488: --- end vector loop cost summary ---
; CHECK:          LOOP END
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-EMPTY:
; CHECK-NEXT:     LOOP BEGIN
; CHECK-NEXT:     LOOP END
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================



; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @serial_call_1() nounwind
declare double @llvm.sqrt.f64(double %val) #1

define void @test(i64 %n, i64* %arr, i64* %arr1) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  call void @serial_call_1()
  %inner.cond = icmp eq i64 %indvars.iv, 42
  br i1 %inner.cond, label %for.body2.preheader, label %for.latch

for.body2.preheader:                              ; preds = %for.body
  br label %for.body2

for.body2:                                        ; preds = %for.body2.preheader, %for.body2
  %indvars.iv2 = phi i64 [ %indvars.iv.next2, %for.body2 ], [ 0, %for.body2.preheader ]
  %d = sitofp i64 %indvars.iv2 to double
  %sqrt = call double @llvm.sqrt.f64(double %d)
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch.loopexit, label %for.body2

for.latch.loopexit:                               ; preds = %for.body2
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}

attributes #1 = { nounwind readnone "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
