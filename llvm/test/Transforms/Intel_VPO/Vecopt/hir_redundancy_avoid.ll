; C Source
; #include <stdlib.h>
;
; typedef unsigned char uint8_t;
;
; int name(uint8_t *pix1, int i_stride_pix1, uint8_t *pix2, int i_stride_pix2) {
;   int i_sum = 0;
;   for (int y = 0; y < 16; y++) {
;     for (int x = 0; x < 16; x++) {
;       i_sum += abs(pix1[x] - pix2[x]);
;     }
;     pix1 += i_stride_pix1;
;     pix2 += i_stride_pix2;
;   }
;   return i_sum;
; }
;
; The inner loop looks like the following before vectorization:
;
; 34>            + DO i1 = 0, 15, 1   <DO_LOOP>
; <36>            |   @llvm.intel.directive(!2);
; <37>            |   @llvm.intel.directive(!3);
; <35>            |
; <35>            |   + DO i2 = 0, 15, 1   <DO_LOOP>
; <6>             |   |   %24 = (%0)[sext.i32.i64(%1) * i1 + i2];
; <9>             |   |   %27 = (%2)[sext.i32.i64(%3) * i1 + i2];
; <14>            |   |   %32 = (zext.i8.i32(%24) + -1 * zext.i8.i32(%27) < 0) ? -1 * zext.i8.i32(%24) + zext.i8.i32(%27) : zext.i8.i32(%24) + -1 * zext.i8.i32(%27);
; <15>            |   |   %9 = %32  +  %9;
; <35>            |   + END LOOP
; <35>            |
; <39>            |   @llvm.intel.directive(!4);
; <38>            |   @llvm.intel.directive(!3);
;
; The generated vector code looks like:
;
; <34>            + DO i1 = 0, 15, 1   <DO_LOOP>
; <41>            |   %.vec = (<16 x i8>*)(%0)[sext.i32.i64(%1) * i1];
; <42>            |   %.vec2 = (<16 x i8>*)(%2)[sext.i32.i64(%3) * i1];
; <43>            |   %NBConv = zext.<16 x i8>.<16 x i32>(%.vec);
; <44>            |   %NBConv3 = zext.<16 x i8>.<16 x i32>(%.vec2);
; <45>            |   %NBConv4 = zext.<16 x i8>.<16 x i32>(%.vec);
; <46>            |   %NBConv5 = zext.<16 x i8>.<16 x i32>(%.vec2);
; <47>            |   %NBConv6 = zext.<16 x i8>.<16 x i32>(%.vec);
; <48>            |   %NBConv7 = zext.<16 x i8>.<16 x i32>(%.vec2);
; <49>            |   %.vec8 = (%NBConv + -1 * %NBConv3 < 0) ? -1 * %NBConv4 + %NBConv5 : %NBConv6 + -1 * %NBConv7;
; <61>            |   %RedOp = %.vec8  +  %RedOp;
; <34>            + END LOOP
;
; HLInsts <45> thru <48> are redundant. The test checks that we do not emit these redundant statements.
;
; ModuleID = 'sad2.c'
;
; TODO - We need to eliminate redundant VPInstructions to be able to do
; something similar here with VPValue base code generation. Testing only mixed
; CG mode for now. We should also try and preserve idioms(abs in this case)
; when possible. CMPLRLLVM-20305.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=16 -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir=0 -disable-output < %s 2>&1 | FileCheck %s
; CHECK: %[[WIDE_LOAD:.*]] = (<16 x i8>*)(%pix1){{.*}}
; CHECK: {{.*}} = zext.<16 x i8>.<16 x i32>(%[[WIDE_LOAD]]);
; CHECK-NOT: {{.*}} = zext.<16 x i8>.<16 x i32>(%[[WIDE_LOAD]]);
source_filename = "sad2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind readonly uwtable
define dso_local i32 @name(i8* nocapture readonly %pix1, i32 %i_stride_pix1, i8* nocapture readonly %pix2, i32 %i_stride_pix2) local_unnamed_addr #0 {
entry:
  %idx.ext = sext i32 %i_stride_pix1 to i64
  %idx.ext8 = sext i32 %i_stride_pix2 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %y.028 = phi i32 [ 0, %entry ], [ %inc11, %for.cond.cleanup3 ]
  %i_sum.027 = phi i32 [ 0, %entry ], [ %add, %for.cond.cleanup3 ]
  %pix1.addr.026 = phi i8* [ %pix1, %entry ], [ %add.ptr, %for.cond.cleanup3 ]
  %pix2.addr.025 = phi i8* [ %pix2, %entry ], [ %add.ptr9, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret i32 %add

for.cond.cleanup3:                                ; preds = %for.body4
  %add.ptr = getelementptr inbounds i8, i8* %pix1.addr.026, i64 %idx.ext
  %add.ptr9 = getelementptr inbounds i8, i8* %pix2.addr.025, i64 %idx.ext8
  %inc11 = add nuw nsw i32 %y.028, 1
  %exitcond29 = icmp eq i32 %inc11, 16
  br i1 %exitcond29, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %i_sum.123 = phi i32 [ %i_sum.027, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx = getelementptr inbounds i8, i8* %pix1.addr.026, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1, !tbaa !2
  %conv = zext i8 %0 to i32
  %arrayidx6 = getelementptr inbounds i8, i8* %pix2.addr.025, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx6, align 1, !tbaa !2
  %conv7 = zext i8 %1 to i32
  %sub = sub nsw i32 %conv, %conv7
  %2 = icmp slt i32 %sub, 0
  %neg = sub nsw i32 0, %sub
  %3 = select i1 %2, i32 %neg, i32 %sub
  %add = add nsw i32 %3, %i_sum.123
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 16
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { noinline norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 75251c447951a5a8c1526f5e9b69dfb5d68bce8e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4ad174c258e697af57c4f129236a52c0be7287a7)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
