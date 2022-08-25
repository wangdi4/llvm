; Check VecClone skips widening uniform parameters of <n x Ty> types.

; RUN: opt -vec-clone -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @bar(<2 x i8>)

define i32 @foo(<2 x i8> %a, <3 x i32> %b) #0 {
entry:
  call void @bar(<2 x i8> %a)
  %ext = extractelement <3 x i32> %b, i32 0
  ret i32 %ext
}

attributes #0 = { "vector-variants"="_ZGVbM4uu_foo,_ZGVbN4uu_foo" }

; CHECK-LABEL: <4 x i32> @_ZGVbM4uu_foo(<2 x i8> %a, <3 x i32> %b, <4 x i32> %mask)
; CHECK:      simd.begin.region:
; CHECK-NEXT:   %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME:   "DIR.OMP.SIMD"
; CHECK-SAME:   "QUAL.OMP.SIMDLEN"
; CHECK-SAME:   "QUAL.OMP.UNIFORM"(<2 x i8>* %alloca.a)
; CHECK-SAME:   "QUAL.OMP.UNIFORM"(<3 x i32>* %alloca.b)
; CHECK:      simd.loop.preheader:
; CHECK-NOT:    %mask
; CHECK:      simd.loop.header:
; CHECK:        call void @bar(<2 x i8> %load.a)
; CHECK:        %ext = extractelement <3 x i32> %load.b, i32 0

; CHECK-LABEL: <4 x i32> @_ZGVbN4uu_foo(<2 x i8> %a, <3 x i32> %b)
; CHECK:      simd.begin.region:
; CHECK-NEXT:   %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME:   "DIR.OMP.SIMD"
; CHECK-SAME:   "QUAL.OMP.SIMDLEN"
; CHECK-SAME:   "QUAL.OMP.UNIFORM"(<2 x i8>* %alloca.a)
; CHECK-SAME:   "QUAL.OMP.UNIFORM"(<3 x i32>* %alloca.b)
; CHECK:      simd.loop.header:
; CHECK:        call void @bar(<2 x i8> %load.a)
; CHECK:        %ext = extractelement <3 x i32> %load.b, i32 0
