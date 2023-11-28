; Test that the stride is being applied correctly to struct field accesses.

; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4l32_foo
; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: ptr %alloca.s
; CHECK-SAME: QUAL.OMP.PRIVATE
; CHECK-SAME: ptr %s.addr
; CHECK: simd.loop.header:
; CHECK: %stride.bytes = mul i32 32, %index
; CHECK-NEXT: %load.s.gep = getelementptr i8, ptr %load.s, i32 %stride.bytes
; CHECK-NEXT: store ptr %load.s.gep, ptr %s.addr, align 8
; CHECK-NEXT: %0 = load ptr, ptr %s.addr, align 8
; CHECK-NEXT: %field1 = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
; CHECK-NEXT: %1 = load float, ptr %field1, align 8
; CHECK-NEXT: %2 = load ptr, ptr %s.addr, align 8
; CHECK-NEXT: %field5 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 4
; CHECK-NEXT: %3 = load float, ptr %field5, align 8
; CHECK-NEXT: %add = fadd float %1, %3

; ModuleID = 'struct_linear_ptr.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { float, i8, i32, i16, float, i64 }

; Function Attrs: nounwind uwtable
define float @foo(ptr %s) #0 {
entry:
  %s.addr = alloca ptr, align 8
  store ptr %s, ptr %s.addr, align 8
  %0 = load ptr, ptr %s.addr, align 8
  %field1 = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  %1 = load float, ptr %field1, align 8
  %2 = load ptr, ptr %s.addr, align 8
  %field5 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 4
  %3 = load float, ptr %field5, align 8
  %add = fadd float %1, %3
  ret float %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4l32_foo" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float "="false" }
