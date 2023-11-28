; RUN: opt -passes=sycl-kernel-coerce-types -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -S %s -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define double @test(<2 x float>
; CHECK: call double @creal(double {{.*}}, double {{.*}})

define double @test(ptr byval({ float, float }) %z) {
entry:
  %call = tail call double @creal(ptr null)
  ret double 0.000000e+00
}

declare double @creal(ptr byval({ double, double }))

; DEBUGIFY-NOT: WARNING
