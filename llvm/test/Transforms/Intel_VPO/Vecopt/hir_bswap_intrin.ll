; LLVM IR generated from testcase below using: icx -S -emit-llvm -Qoption,c,-fveclib=SVML -openmp -restrict -ffast-math -O2
;
; void gl_swap2(unsigned short *p, unsigned int n) {
;   register unsigned int i;
; 
;   for (i=0;i<n;i++) {
;     p[i] = (p[i] >> 8) | ((p[i] << 8) & 0xff00);
;   }
; }

; Test vectorization of intrinsics, specifically bswap in this case.

; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -print-after=VPODriverHIR -S  < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK: call <8 x i16> @llvm.bswap.v8i16

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @gl_swap2(i16* nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp15 = icmp eq i32 %n, 0
  br i1 %cmp15, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i16, i16* %p, i64 %indvars.iv
  %0 = load i16, i16* %arrayidx, align 2, !tbaa !1
  %rev = tail call i16 @llvm.bswap.i16(i16 %0)
  store i16 %rev, i16* %arrayidx, align 2, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind readnone
declare i16 @llvm.bswap.i16(i16) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!1 = !{!2, !2, i64 0}
!2 = !{!"short", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
