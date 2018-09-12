; ModuleID = 'tsmall.c'
; LLVM IR generated from following test using clang -O1 -S -emit-llvm
; void foo(int **restrict ip1, int **restrict ip2)
; {
;   int index;
; 
;   for (index = 0; index <  1024; index++) {
;     *ip1++ = 0;
;     *ip2++ = 0;
;   }
; }
; 
; RUN: opt -default-vpo-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -print-after=VPODriverHIR -S < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR
;
; HIR Test.
; CHECK:      DO i1 = 0, 1023, 4
; CHECK-NEXT:    (<4 x i32*>*)(%ip1)[i1] = 0;
; CHECK-NEXT:    (<4 x i32*>*)(%ip2)[i1] = 0;
; CHECK-NEXT: END LOOP

; ModuleID = 'tsmall.c'
source_filename = "tsmall.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32** noalias nocapture %ip1, i32** noalias nocapture %ip2) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %index.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %ip2.addr.05 = phi i32** [ %ip2, %entry ], [ %incdec.ptr1, %for.body ]
  %ip1.addr.04 = phi i32** [ %ip1, %entry ], [ %incdec.ptr, %for.body ]
  %incdec.ptr = getelementptr inbounds i32*, i32** %ip1.addr.04, i64 1
  store i32* null, i32** %ip1.addr.04, align 8, !tbaa !1
  %incdec.ptr1 = getelementptr inbounds i32*, i32** %ip2.addr.05, i64 1
  store i32* null, i32** %ip2.addr.05, align 8, !tbaa !1
  %inc = add nuw nsw i32 %index.06, 1
  %exitcond = icmp eq i32 %inc, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12546)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
