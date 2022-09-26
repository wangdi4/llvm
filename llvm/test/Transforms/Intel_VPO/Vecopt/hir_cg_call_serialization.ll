; Test VPlan HIR vectorizer codegen for call serialization.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s

; Input HIR:
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;
;       + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;       |   %0 = (%arr)[i1];
;       |   %unmasked_call = @bay(i1);
;       |   if (%0 != %unmasked_call)
;       |   {
;       |      %masked_call = @baz(i1);
;       |      (%arr)[i1] = %masked_call;
;       |      @baa(i1);
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; CHECK-LABEL:   BEGIN REGION { modified }
; CHECK-NEXT:          + DO i1 = 0, 99, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:          |   %.vec = (<2 x i32>*)(%arr)[i1];
; CHECK-NEXT:          |   %serial.temp = undef;
; CHECK-NEXT:          |   %bay = @bay(i1);
; CHECK-NEXT:          |   %serial.temp = insertelement %serial.temp,  %bay,  0;
; CHECK-NEXT:          |   %extract.1. = extractelement i1 + <i64 0, i64 1>,  1;
; CHECK-NEXT:          |   %bay2 = @bay(%extract.1.);
; CHECK-NEXT:          |   %serial.temp = insertelement %serial.temp,  %bay2,  1;
; CHECK-NEXT:          |   %.vec4 = %.vec != %serial.temp;
; CHECK-NEXT:          |   %serial.temp5 = undef;
; CHECK-NEXT:          |   %mask.0. = extractelement %.vec4,  0;
; CHECK-NEXT:          |   if (%mask.0. == 1)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      %baz = @baz(i1);
; CHECK-NEXT:          |      %serial.temp5 = insertelement %serial.temp5,  %baz,  0;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          |   %mask.1. = extractelement %.vec4,  1;
; CHECK-NEXT:          |   if (%mask.1. == 1)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      %extract.1.7 = extractelement i1 + <i64 0, i64 1>,  1;
; CHECK-NEXT:          |      %baz8 = @baz(%extract.1.7);
; CHECK-NEXT:          |      %serial.temp5 = insertelement %serial.temp5,  %baz8,  1;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          |   (<2 x i32>*)(%arr)[i1] = %serial.temp5, Mask = @{%.vec4};
; CHECK-NEXT:          |   %mask.0.10 = extractelement %.vec4,  0;
; CHECK-NEXT:          |   if (%mask.0.10 == 1)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      @baa(i1);
; CHECK-NEXT:          |   }
; CHECK-NEXT:          |   %mask.1.11 = extractelement %.vec4,  1;
; CHECK-NEXT:          |   if (%mask.1.11 == 1)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      %extract.1.12 = extractelement i1 + <i64 0, i64 1>,  1;
; CHECK-NEXT:          |      @baa(%extract.1.12);
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP
; CHECK:         END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture readonly %arr) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %unmasked_call = call i32 @bay(i64 %indvars.iv) #0
  %tobool = icmp eq i32 %0, %unmasked_call
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %1 = trunc i64 %indvars.iv to i32
  %masked_call = tail call fastcc i32 @baz(i32 %1) #0
  store i32 %masked_call, i32* %arrayidx, align 4
  tail call void @baa(i32 %1) #0
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @bay(i64)
declare i32 @baz(i32)
declare void @baa(i32)

attributes #0 = { nounwind }
