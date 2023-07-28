; RUN: opt -passes="nary-reassociate" -S %s | FileCheck %s
;
; 24715: Reassociate is skipping inttoptr conversion and creating binary
; expressions with mixed int, pointer types.

; CHECK-DAG: getelementptr inbounds double
; CHECK-DAG: ptrtoint ptr %0
; CHECK-DAG: add i64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local fastcc void @advection_mp_init_advection_() unnamed_addr #0 {
entry:
  br label %lab

lab:
  %0 = getelementptr inbounds double, ptr null, i64 undef
  %1 = ptrtoint ptr %0 to i64
  %2 = add i64 undef, %1
  %3 = add i64 %2, 0
  br i1 undef, label %lab, label %ex

ex:
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }

