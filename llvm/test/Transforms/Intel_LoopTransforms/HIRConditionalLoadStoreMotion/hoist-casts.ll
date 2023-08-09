; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion is able to appropriately
; bitcast between matching values that support it and not choke on ones that
; don't when hoisting.

; Print before:

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %Ai32.then = (i32*)(%A)[i1];
; CHECK:       |      %Adouble.then = (double*)(%A)[i1];
; CHECK:       |      %Afloat.then = (float*)(%A)[i1];
; CHECK:       |      %Ai64ptr.then = (ptr)(%A)[i1];
; CHECK:       |      %Ai32ptr.then = (ptr)(%A)[i1];
; CHECK:       |      %Ai64as1ptr.then = (ptr addrspace(1))(%A)[i1];
; CHECK:       |      (%B)[i1] = (%A)[i1];
; CHECK:       |      (i32*)(%B)[i1 + 1] = %Ai32.then;
; CHECK:       |      (double*)(%B)[i1] = %Adouble.then;
; CHECK:       |      (float*)(%B)[i1 + 1] = %Afloat.then;
; CHECK:       |      (ptr)(%B)[i1 + 2] = &((%Ai64ptr.then)[0]);
; CHECK:       |      (ptr)(%B)[i1 + 2] = &((%Ai32ptr.then)[0]);
; CHECK:       |      (ptr addrspace(1))(%B)[i1 + 3] = &((%Ai64as1ptr.then)[0]);
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %Ai32.else = (i32*)(%A)[i1];
; CHECK:       |      %Adouble.else = (double*)(%A)[i1];
; CHECK:       |      %Afloat.else = (float*)(%A)[i1];
; CHECK:       |      %Ai64ptr.else = (ptr)(%A)[i1];
; CHECK:       |      %Ai32ptr.else = (ptr)(%A)[i1];
; CHECK:       |      %Ai64as1ptr.else = (ptr addrspace(1))(%A)[i1];
; CHECK:       |      (%B)[i1] = (%A)[i1];
; CHECK:       |      (i32*)(%B)[i1 + 1] = %Ai32.else;
; CHECK:       |      (double*)(%B)[i1] = %Adouble.else;
; CHECK:       |      (float*)(%B)[i1 + 1] = %Afloat.else;
; CHECK:       |      (ptr)(%B)[i1 + 2] = &((%Ai64ptr.else)[0]);
; CHECK:       |      (ptr)(%B)[i1 + 2] = &((%Ai32ptr.else)[0]);
; CHECK:       |      (ptr addrspace(1))(%B)[i1 + 3] = &((%Ai64as1ptr.else)[0]);
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   %[[HOISTED32:[A-za-z0-9_.]+]] = (i32*)(%A)[i1];
; CHECK:       |   %[[HOISTED64:[A-za-z0-9_.]+]] = (double*)(%A)[i1];
; CHECK:       |   %[[HOISTEDPTR:[A-za-z0-9_.]+]] = (ptr)(%A)[i1];
; CHECK:       |   %[[HOISTEDAS1PTR:[A-za-z0-9_.]+]] = (ptr addrspace(1))(%A)[i1];
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %Ai32.then = %[[HOISTED32]];
; CHECK:       |      %Adouble.then = %[[HOISTED64]];
; CHECK:       |      %[[THCAST32:[A-za-z0-9_.]+]] = bitcast.i32.float(%[[HOISTED32]]);
; CHECK:       |      %Afloat.then = %[[THCAST32]];
; CHECK:       |      %Ai64ptr.then = %[[HOISTEDPTR]];
; CHECK:       |      %Ai32ptr.then = %[[HOISTEDPTR]];
; CHECK:       |      %Ai64as1ptr.then = %[[HOISTEDAS1PTR]];
; CHECK:       |      %[[THCAST64:[A-za-z0-9_.]+]] = bitcast.double.i64(%[[HOISTED64]]);
; CHECK:       |      {{[^ ]+}} = %[[THCAST64]];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %Ai32.else = %[[HOISTED32]];
; CHECK:       |      %Adouble.else = %[[HOISTED64]];
; CHECK:       |      %[[EHCAST32:[A-za-z0-9_.]+]] = bitcast.i32.float(%[[HOISTED32]]);
; CHECK:       |      %Afloat.else = %[[EHCAST32]];
; CHECK:       |      %Ai64ptr.else = %[[HOISTEDPTR]];
; CHECK:       |      %Ai32ptr.else = %[[HOISTEDPTR]];
; CHECK:       |      %Ai64as1ptr.else = %[[HOISTEDAS1PTR]];
; CHECK:       |      %[[EHCAST64:[A-za-z0-9_.]+]] = bitcast.double.i64(%[[HOISTED64]]);
; CHECK:       |      {{[^ ]+}} = %[[EHCAST64]];
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @casts(ptr %A, ptr %B, i1 %which) {
entry:
  br label %L1

L1:                                               ; preds = %L1.latch, %entry
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %Ai = getelementptr inbounds i64, ptr %A, i64 %i
  %Ai32p = bitcast ptr %Ai to ptr
  %Adoublep = bitcast ptr %Ai to ptr
  %Afloatp = bitcast ptr %Ai to ptr
  %Ai64ptrp = bitcast ptr %Ai to ptr
  %Ai32ptrp = bitcast ptr %Ai to ptr
  %Ai64as1ptrp = bitcast ptr %Ai to ptr
  %Bi0 = getelementptr inbounds i64, ptr %B, i64 %i
  %i1 = add nuw nsw i64 %i, 1
  %Bi1 = getelementptr inbounds i64, ptr %B, i64 %i1
  %Bi32p = bitcast ptr %Bi1 to ptr
  %Bdoublep = bitcast ptr %Bi0 to ptr
  %Bfloatp = bitcast ptr %Bi1 to ptr
  %i2 = add nuw nsw i64 %i, 2
  %Bi2 = getelementptr inbounds i64, ptr %B, i64 %i2
  %Bi64ptrp = bitcast ptr %Bi2 to ptr
  %Bi32ptrp = bitcast ptr %Bi2 to ptr
  %i3 = add nuw nsw i64 %i, 3
  %Bi3 = getelementptr inbounds i64, ptr %B, i64 %i3
  %Bi64as1ptrp = bitcast ptr %Bi3 to ptr
  br i1 %which, label %then, label %else

then:                                             ; preds = %L1
  %Ai64.then = load i64, ptr %Ai, align 8
  %Ai32.then = load i32, ptr %Ai32p, align 8
  %Adouble.then = load double, ptr %Adoublep, align 8
  %Afloat.then = load float, ptr %Afloatp, align 8
  %Ai64ptr.then = load ptr, ptr %Ai64ptrp, align 8
  %Ai32ptr.then = load ptr, ptr %Ai32ptrp, align 8
  %Ai64as1ptr.then = load ptr addrspace(1), ptr %Ai64as1ptrp, align 8
  store i64 %Ai64.then, ptr %Bi0, align 8
  store i32 %Ai32.then, ptr %Bi32p, align 8
  store double %Adouble.then, ptr %Bdoublep, align 8
  store float %Afloat.then, ptr %Bfloatp, align 8
  store ptr %Ai64ptr.then, ptr %Bi64ptrp, align 8
  store ptr %Ai32ptr.then, ptr %Bi32ptrp, align 8
  store ptr addrspace(1) %Ai64as1ptr.then, ptr %Bi64as1ptrp, align 8
  br label %L1.latch

else:                                             ; preds = %L1
  %Ai64.else = load i64, ptr %Ai, align 8
  %Ai32.else = load i32, ptr %Ai32p, align 8
  %Adouble.else = load double, ptr %Adoublep, align 8
  %Afloat.else = load float, ptr %Afloatp, align 8
  %Ai64ptr.else = load ptr, ptr %Ai64ptrp, align 8
  %Ai32ptr.else = load ptr, ptr %Ai32ptrp, align 8
  %Ai64as1ptr.else = load ptr addrspace(1), ptr %Ai64as1ptrp, align 8
  store i64 %Ai64.else, ptr %Bi0, align 8
  store i32 %Ai32.else, ptr %Bi32p, align 8
  store double %Adouble.else, ptr %Bdoublep, align 8
  store float %Afloat.else, ptr %Bfloatp, align 8
  store ptr %Ai64ptr.else, ptr %Bi64ptrp, align 8
  store ptr %Ai32ptr.else, ptr %Bi32ptrp, align 8
  store ptr addrspace(1) %Ai64as1ptr.else, ptr %Bi64as1ptrp, align 8
  br label %L1.latch

L1.latch:                                         ; preds = %else, %then
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:                                             ; preds = %L1.latch
  ret void
}
