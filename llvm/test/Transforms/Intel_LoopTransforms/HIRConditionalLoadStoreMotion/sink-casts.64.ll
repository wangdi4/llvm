; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion is able to appropriately
; bitcast between matching values that support it and not choke on ones that
; don't when sinking.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      (%B)[i1] = (%A)[i1];
; CHECK:       |      (double*)(%B)[i1] = %Adouble.then;
; CHECK:       |      (i64**)(%B)[i1 + 2] = &((%Ai64ptr.then)[0]);
; CHECK:       |      (i32**)(%B)[i1 + 2] = &((%Ai32ptr.then)[0]);
; CHECK:       |      (i64 addrspace(1)**)(%B)[i1 + 3] = &((%Ai64as1ptr.then)[0]);
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      (%B)[i1] = (%A)[i1];
; CHECK:       |      (double*)(%B)[i1] = %Adouble.else;
; CHECK:       |      (i64**)(%B)[i1 + 2] = &((%Ai64ptr.else)[0]);
; CHECK:       |      (i32**)(%B)[i1 + 2] = &((%Ai32ptr.else)[0]);
; CHECK:       |      (i64 addrspace(1)**)(%B)[i1 + 3] = &((%Ai64as1ptr.else)[0]);
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %[[SUNK64:[A-za-z0-9_.]+]] = bitcast.i64.double(%{{[A-za-z0-9_.]+}});
; CHECK:       |      %[[SUNK64]] = %Adouble.then;
; CHECK:       |      %[[TSCASTPTR:[A-za-z0-9_.]+]] = &((%Ai64ptr.then)[0]);
; CHECK:       |      %[[SUNKPTR:[A-za-z0-9_.]+]] = bitcast.i64*.i32*(%[[TSCASTPTR]]);
; CHECK:       |      %[[SUNKPTR]] = &((%Ai32ptr.then)[0]);
; CHECK:       |      %[[SUNKAS1PTR:[A-za-z0-9_.]+]] = &((%Ai64as1ptr.then)[0]);
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %[[SUNK64]] = bitcast.i64.double(%{{[A-za-z0-9_.]+}});
; CHECK:       |      %[[SUNK64]] = %Adouble.else;
; CHECK:       |      %[[ESCASTPTR:[A-za-z0-9_.]+]] = &((%Ai64ptr.else)[0]);
; CHECK:       |      %[[SUNKPTR]] = bitcast.i64*.i32*(%[[ESCASTPTR]]);
; CHECK:       |      %[[SUNKPTR]] = &((%Ai32ptr.else)[0]);
; CHECK:       |      %[[SUNKAS1PTR]] = &((%Ai64as1ptr.else)[0]);
; CHECK:       |   }
; CHECK:       |   (double*)(%B)[i1] = %[[SUNK64]];
; CHECK:       |   (i32**)(%B)[i1 + 2] = %[[SUNKPTR]];
; CHECK:       |   (i64 addrspace(1)**)(%B)[i1 + 3] = %[[SUNKAS1PTR]];
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @casts(i64* %A, i64* %B, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %Ai = getelementptr inbounds i64, i64* %A, i64 %i
  %Adoublep = bitcast i64* %Ai to double*
  %Ai64ptrp = bitcast i64* %Ai to i64**
  %Ai32ptrp = bitcast i64* %Ai to i32**
  %Ai64as1ptrp = bitcast i64* %Ai to i64 addrspace(1)**
  %Bi0 = getelementptr inbounds i64, i64* %B, i64 %i
  %Bdoublep = bitcast i64* %Bi0 to double*
  %i2 = add nuw nsw i64 %i, 2
  %Bi2 = getelementptr inbounds i64, i64* %B, i64 %i2
  %Bi64ptrp = bitcast i64* %Bi2 to i64**
  %Bi32ptrp = bitcast i64* %Bi2 to i32**
  %i3 = add nuw nsw i64 %i, 3
  %Bi3 = getelementptr inbounds i64, i64* %B, i64 %i3
  %Bi64as1ptrp = bitcast i64* %Bi3 to i64 addrspace(1)**
  br i1 %which, label %then, label %else

then:
  %Ai64.then = load i64, i64* %Ai, align 8
  %Adouble.then = load double, double* %Adoublep, align 8
  %Ai64ptr.then = load i64*, i64** %Ai64ptrp, align 8
  %Ai32ptr.then = load i32*, i32** %Ai32ptrp, align 8
  %Ai64as1ptr.then = load i64 addrspace(1)*, i64 addrspace(1)** %Ai64as1ptrp, align 8
  store i64 %Ai64.then, i64* %Bi0, align 8
  store double %Adouble.then, double* %Bdoublep, align 8
  store i64* %Ai64ptr.then, i64** %Bi64ptrp, align 8
  store i32* %Ai32ptr.then, i32** %Bi32ptrp, align 8
  store i64 addrspace(1)* %Ai64as1ptr.then, i64 addrspace(1)** %Bi64as1ptrp, align 8
  br label %L1.latch

else:
  %Ai64.else = load i64, i64* %Ai, align 8
  %Adouble.else = load double, double* %Adoublep, align 8
  %Ai64ptr.else = load i64*, i64** %Ai64ptrp, align 8
  %Ai32ptr.else = load i32*, i32** %Ai32ptrp, align 8
  %Ai64as1ptr.else = load i64 addrspace(1)*, i64 addrspace(1)** %Ai64as1ptrp, align 8
  store i64 %Ai64.else, i64* %Bi0, align 8
  store double %Adouble.else, double* %Bdoublep, align 8
  store i64* %Ai64ptr.else, i64** %Bi64ptrp, align 8
  store i32* %Ai32ptr.else, i32** %Bi32ptrp, align 8
  store i64 addrspace(1)* %Ai64as1ptr.else, i64 addrspace(1)** %Bi64as1ptrp, align 8
  br label %L1.latch

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:
  ret void
}
