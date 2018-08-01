; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-runtime-dd -scoped-noalias -hir-loop-reversal -hir-vec-dir-insert -VPODriverHIR -print-after=VPODriverHIR -hir-cg 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; Verify that we successfully generate code for this case. An assert was 
; triggered during codgen because we were trying to convert the pointer blob in
; the subscript (src) to a vector type (<4 * i32>) instead of vector's scalar
; type (i32).


; Input HIR-

; + DO i1 = 0, -1 * %src + umax((1 + %src), (%len + %src)) + -1, 1   <DO_LOOP>
; |   (%dest)[-1 * i1 + %len + -1] = (%src)[i1];
; + END LOOP


; The loop just before CG-

; CHECK: + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>
; CHECK: |   (<4 x i8>*)(%dest)[i1 + %src + %len + -1 * umax((1 + %src), (%len + %src))] = (<4 x i8>*)(%src)[-1 * i1 + -1 * %src + umax((1 + %src), (%len + %src)) + <i32 0, i32 -1, i32 -2, i32 -3> + -1];
; CHECK: + END LOOP


target datalayout = "e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"

define internal i8* @memcpy_76543210(i8* returned %dest, i8* readonly %src, i32 %len) #5 {
entry:
  %add.ptr1 = getelementptr inbounds i8, i8* %src, i32 %len
  %cmp10 = icmp sgt i32 %len, 0
  br i1 %cmp10, label %while.body.preheader, label %while.end

while.body.preheader:                             ; preds = %entry
  %add.ptr = getelementptr inbounds i8, i8* %dest, i32 %len
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %act_dest.012 = phi i8* [ %incdec.ptr2, %while.body ], [ %add.ptr, %while.body.preheader ]
  %act_src.011 = phi i8* [ %incdec.ptr, %while.body ], [ %src, %while.body.preheader ]
  %incdec.ptr = getelementptr inbounds i8, i8* %act_src.011, i32 1
  %0 = load i8, i8* %act_src.011, align 1
  %incdec.ptr2 = getelementptr inbounds i8, i8* %act_dest.012, i32 -1
  store i8 %0, i8* %incdec.ptr2, align 1
  %cmp = icmp ult i8* %incdec.ptr, %add.ptr1
  br i1 %cmp, label %while.body, label %while.end.loopexit

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret i8* %dest
}

