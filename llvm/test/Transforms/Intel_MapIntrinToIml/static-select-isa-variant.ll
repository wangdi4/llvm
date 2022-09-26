; Check to see that SVML function calls are dispatched to ISA-specific variants.

; RUN: opt -mtriple=i686-unknown-linux-gnu -enable-new-pm=0 -vector-library=SVML -mattr=+sse2 -iml-trans -S < %s | FileCheck %s -check-prefixes=CHECK,CHECK-X86
; RUN: opt -mtriple=x86_64-unknown-linux-gnu -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s -check-prefixes=CHECK,CHECK-X86_64
; RUN: opt -mtriple=i686-unknown-linux-gnu -enable-new-pm=0 -vector-library=SVML -mattr=+sse2 -iml-trans -enable-intel-advanced-opts -S < %s | FileCheck %s -check-prefixes=CHECK,CHECK-X86
; RUN: opt -mtriple=x86_64-unknown-linux-gnu -enable-new-pm=0 -vector-library=SVML -iml-trans -enable-intel-advanced-opts -S < %s | FileCheck %s -check-prefixes=CHECK,CHECK-X86_64

; CHECK-LABEL: @vector_foo_dynamic
; CHECK: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_dynamic(float* nocapture %varray) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_static_sse42
; CHECK: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_static_sse42(float* nocapture %varray) #1 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_static_avx
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4_g9(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_static_avx(float* nocapture %varray) #2 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_static_avx2
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4_s9(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_static_avx2(float* nocapture %varray) #3 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_static_avx512
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4_s9(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_static_avx512(float* nocapture %varray) #4 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; For AVX512 target with high ZMM, 512-bit SVML functions use high ZMM 512-bit variants, 256-bit and lower ones are dispatched to AVX2 variants.
; CHECK-LABEL: @vector_foo_static_avx512_high_zmm
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4_s9(
; CHECK-X86_64: call fast svml_cc <4 x float> @__svml_acosf4_l9(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_static_avx512_high_zmm(float* nocapture %varray) #5 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_512_static_avx512_high_zmm
; CHECK-X86: call fast svml_avx512_cc <16 x float> @__svml_acosf16_x0(
; CHECK-X86_64: call fast svml_avx512_cc <16 x float> @__svml_acosf16_z0(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_512_static_avx512_high_zmm(float* nocapture %varray) #5 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <16 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <16 x i32> %broadcast.splatinsert7, <16 x i32> undef, <16 x i32> zeroinitializer
  %induction9 = add <16 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %1 = sitofp <16 x i32> %induction9 to <16 x float>
  %2 = fmul fast <16 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <16 x float> @__svml_acosf16(<16 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <16 x float>*
  store <16 x float> %3, <16 x float>* %5, align 4
  %index.next = add i64 %index, 16
  %6 = icmp eq i64 %index.next, 1008
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_force_dynamic_avx
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_force_dynamic_avx(float* nocapture %varray) #2 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2) #7
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_force_dynamic_avx2
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_force_dynamic_avx2(float* nocapture %varray) #3 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2) #7
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_force_dynamic_avx512
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_force_dynamic_avx512(float* nocapture %varray) #4 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2) #7
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_foo_force_dynamic_avx512_high_zmm
; CHECK-X86: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK-X86_64: call fast svml_cc <4 x float> @__svml_acosf4(
; CHECK: ret

; Function Attrs: nounwind uwtable
define void @vector_foo_force_dynamic_avx512_high_zmm(float* nocapture %varray) #5 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert7 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat8 = shufflevector <4 x i32> %broadcast.splatinsert7, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction9 = add <4 x i32> %broadcast.splat8, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction9 to <4 x float>
  %2 = fmul fast <4 x float> %1, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %3 = call fast <4 x float> @__svml_acosf4(<4 x float> %2) #7
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @scalar_foo_dynamic
; CHECK: call fast svml_cc <1 x float> @__svml_acosf1(
; CHECK: ret
define <1 x float> @scalar_foo_dynamic(<1 x float> %src) #0 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src)
  ret <1 x float> %result
}

; CHECK-LABEL: @scalar_foo_static_sse42
; CHECK-X86: call fast svml_cc <1 x float> @__svml_acosf1_g9(
; CHECK-X86_64: call fast svml_cc <1 x float> @__svml_acosf1_e9(
; CHECK: ret
define <1 x float> @scalar_foo_static_sse42(<1 x float> %src) #2 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src)
  ret <1 x float> %result
}

; CHECK-LABEL: @scalar_foo_static_avx2
; CHECK-X86: call fast svml_cc <1 x float> @__svml_acosf1_s9(
; CHECK-X86_64: call fast svml_cc <1 x float> @__svml_acosf1_l9(
; CHECK: ret
define <1 x float> @scalar_foo_static_avx2(<1 x float> %src) #3 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src)
  ret <1 x float> %result
}

; CHECK-LABEL: @scalar_foo_static_avx512
; CHECK-X86: call fast svml_cc <1 x float> @__svml_acosf1_s9(
; CHECK-X86_64: call fast svml_cc <1 x float> @__svml_acosf1_l9(
; CHECK: ret
define <1 x float> @scalar_foo_static_avx512(<1 x float> %src) #4 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src)
  ret <1 x float> %result
}

; CHECK-LABEL: @scalar_foo_static_avx512_high_zmm
; CHECK-X86: call fast svml_cc <1 x float> @__svml_acosf1_s9(
; CHECK-X86_64: call fast svml_cc <1 x float> @__svml_acosf1_l9(
; CHECK: ret
define <1 x float> @scalar_foo_static_avx512_high_zmm(<1 x float> %src) #5 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src)
  ret <1 x float> %result
}

; CHECK-LABEL: @scalar_foo_force_dynamic_avx2
; CHECK: call fast svml_cc <1 x float> @__svml_acosf1(
; CHECK: ret
define <1 x float> @scalar_foo_force_dynamic_avx2(<1 x float> %src) #3 {
  %result = call fast svml_cc <1 x float> @__svml_acosf1(<1 x float> %src) #7
  ret <1 x float> %result
}

; Function Attrs: nounwind readnone
declare <16 x float> @__svml_acosf16(<16 x float>) #6
declare <4 x float> @__svml_acosf4(<4 x float>) #6
declare <1 x float> @__svml_acosf1(<1 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+rdrnd,+rdseed,+rtm,+sgx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+rdrnd,+rdseed,+rtm,+sgx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
attributes #7 = { "imf-force-dynamic"="true" }
