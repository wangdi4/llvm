; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnPreVec %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'testModule'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @_Z5crossDv3_fS__test
; CHECK-NOT: @_Z5crossDv3_fS_
; CHECK: @_f_v._Z5crossDv3_fS_
; CHECK-NOT: @_Z5crossDv3_fS_
; CHECK: ret void
define void @_Z5crossDv3_fS__test(<3 x float>*, <3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z5crossDv3_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <<4 x float>> [#uses=1]
  %shuf_cast4 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %2, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast4, <3 x float>* %ret_ptr
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z5crossDv3_fS_(<4 x float>, <4 x float>)


; CHECK:@_Z5crossDv4_fS__test
; CHECK-NOT: @_Z5crossDv4_fS_
; CHECK: @_f_v._Z5crossDv4_fS_
; CHECK-NOT: @_Z5crossDv4_fS_
; CHECK: ret void
define void @_Z5crossDv4_fS__test(<4 x float>*, <4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z5crossDv4_fS_(<4 x float> %load_arg, <4 x float> %load_arg2) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %2, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z5crossDv4_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z13fast_distanceff_test
; CHECK-NOT: @_Z13fast_distanceff
; CHECK: @_f_v._Z13fast_distanceff
; CHECK-NOT: @_Z13fast_distanceff
; CHECK: ret void
define void @_Z13fast_distanceff_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @_Z13fast_distanceff(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z13fast_distanceff(float, float)


; CHECK: @_Z13fast_distanceDv2_fS__test
; CHECK-NOT: @_Z13fast_distanceDv2_fS_
; CHECK: @_f_v._Z13fast_distanceDv2_fS_
; CHECK-NOT: @_Z13fast_distanceDv2_fS_
; CHECK: ret void
define void @_Z13fast_distanceDv2_fS__test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z13fast_distanceDv2_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z13fast_distanceDv2_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z13fast_distanceDv3_fS__test
; CHECK-NOT: @_Z13fast_distanceDv3_fS_
; CHECK: @_f_v._Z13fast_distanceDv3_fS_
; CHECK-NOT: @_Z13fast_distanceDv3_fS_
; CHECK: ret void
define void @_Z13fast_distanceDv3_fS__test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z13fast_distanceDv3_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z13fast_distanceDv3_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z13fast_distanceDv4_fS__test
; CHECK-NOT: @_Z13fast_distanceDv4_fS_
; CHECK: @_f_v._Z13fast_distanceDv4_fS_
; CHECK-NOT: @_Z13fast_distanceDv4_fS_
; CHECK: ret void
define void @_Z13fast_distanceDv4_fS__test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @_Z13fast_distanceDv4_fS_(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z13fast_distanceDv4_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z8distanceff_test
; CHECK-NOT: @_Z8distanceff
; CHECK: @_f_v._Z8distanceff
; CHECK-NOT: @_Z8distanceff
; CHECK: ret void
define void @_Z8distanceff_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @_Z8distanceff(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z8distanceff(float, float)


; CHECK: @_Z6lengthf_test
; CHECK-NOT: @_Z6lengthf
; CHECK: @_f_v._Z6lengthf
; CHECK-NOT: @_Z6lengthf
; CHECK: ret void
define void @_Z6lengthf_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @_Z6lengthf(float %load_arg)  ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z6lengthf(float)


; CHECK: @_Z8distanceDv2_fS__test
; CHECK-NOT: @_Z8distanceDv2_fS_
; CHECK: @_f_v._Z8distanceDv2_fS_
; CHECK-NOT: @_Z8distanceDv2_fS_
; CHECK: ret void
define void @_Z8distanceDv2_fS__test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z8distanceDv2_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z8distanceDv2_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z6lengthDv2_f_test
; CHECK-NOT: @_Z6lengthDv2_f
; CHECK: @_f_v._Z6lengthDv2_f
; CHECK-NOT: @_Z6lengthDv2_f
; CHECK: ret void
define void @_Z6lengthDv2_f_test(<2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z6lengthDv2_f(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z6lengthDv2_f(<4 x float>)


; CHECK:  @_Z8distanceDv3_fS__test
; CHECK-NOT: @_Z8distanceDv3_fS_
; CHECK: @_f_v._Z8distanceDv3_fS_
; CHECK-NOT: @_Z8distanceDv3_fS_
; CHECK: ret void
define void @_Z8distanceDv3_fS__test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z8distanceDv3_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z8distanceDv3_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z6lengthDv3_f_test
; CHECK-NOT: @_Z6lengthDv3_f
; CHECK: @_f_v._Z6lengthDv3_f
; CHECK-NOT: @_Z6lengthDv3_f
; CHECK: ret void
define void @_Z6lengthDv3_f_test(<3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z6lengthDv3_f(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z6lengthDv3_f(<4 x float>)


; CHECK: @_Z8distanceDv4_fS__test
; CHECK-NOT: @_Z8distanceDv4_fS_
; CHECK: @_f_v._Z8distanceDv4_fS_
; CHECK-NOT: @_Z8distanceDv4_fS_
; CHECK: ret void
define void @_Z8distanceDv4_fS__test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @_Z8distanceDv4_fS_(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z8distanceDv4_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z6lengthDv4_f_test
; CHECK-NOT: @_Z6lengthDv4_f
; CHECK: @_f_v._Z6lengthDv4_f
; CHECK-NOT: @_Z6lengthDv4_f
; CHECK: ret void
define void @_Z6lengthDv4_f_test(<4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call float @_Z6lengthDv4_f(<4 x float> %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z6lengthDv4_f(<4 x float>)


; CHECK: @_Z3dotff_test
; CHECK-NOT: @_Z3dotff
; CHECK: @_f_v._Z3dotff
; CHECK-NOT: @_Z3dotff
; CHECK: ret void
define void @_Z3dotff_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @_Z3dotff(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z3dotff(float, float)


; CHECK: @_Z3dotDv2_fS__test
; CHECK-NOT: @_Z3dotDv2_fS_
; CHECK: @_f_v._Z3dotDv2_fS_
; CHECK-NOT: @_Z3dotDv2_fS_
; CHECK: ret void
define void @_Z3dotDv2_fS__test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z3dotDv2_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z3dotDv2_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z3dotDv3_fS__test
; CHECK-NOT: @_Z3dotDv3_fS_
; CHECK: @_f_v._Z3dotDv3_fS_
; CHECK-NOT: @_Z3dotDv3_fS_
; CHECK: ret void
define void @_Z3dotDv3_fS__test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z3dotDv3_fS_(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z3dotDv3_fS_(<4 x float>, <4 x float>)


; CHECK: @_Z3dotff4_test
; CHECK-NOT: @_Z3dotff4
; CHECK: @_f_v._Z3dotff4
; CHECK-NOT: @_Z3dotff4
; CHECK: ret void
define void @_Z3dotff4_test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @_Z3dotff4(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z3dotff4(<4 x float>, <4 x float>)


; CHECK: @_Z11fast_lengthf_test
; CHECK-NOT: @_Z11fast_lengthf
; CHECK: @_f_v._Z11fast_lengthf
; CHECK-NOT: @_Z11fast_lengthf
; CHECK: ret void
define void @_Z11fast_lengthf_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @_Z11fast_lengthf(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z11fast_lengthf(float)


; CHECK: @_Z11fast_lengthDv2_f_test
; CHECK-NOT: @_Z11fast_lengthDv2_f
; CHECK: @_f_v._Z11fast_lengthDv2_f
; CHECK-NOT: @_Z11fast_lengthDv2_f
; CHECK: ret void
define void @_Z11fast_lengthDv2_f_test(<2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z11fast_lengthDv2_f(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z11fast_lengthDv2_f(<4 x float>)


; CHECK: @_Z11fast_lengthDv3_f_test
; CHECK-NOT: @_Z11fast_lengthDv3_f
; CHECK: @_f_v._Z11fast_lengthDv3_f
; CHECK-NOT: @_Z11fast_lengthDv3_f
; CHECK: ret void
define void @_Z11fast_lengthDv3_f_test(<3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @_Z11fast_lengthDv3_f(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z11fast_lengthDv3_f(<4 x float>)


; CHECK: @_Z11fast_lengthDv4_f_test
; CHECK-NOT: @_Z11fast_lengthDv4_f
; CHECK: @_f_v._Z11fast_lengthDv4_f
; CHECK-NOT: @_Z11fast_lengthDv4_f
; CHECK: ret void
define void @_Z11fast_lengthDv4_f_test(<4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call float @_Z11fast_lengthDv4_f(<4 x float> %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z11fast_lengthDv4_f(<4 x float>)


; CHECK: @_Z14fast_normalizef_test
; CHECK-NOT: @_Z14fast_normalizef
; CHECK: @_f_v._Z14fast_normalizef
; CHECK-NOT: @_Z14fast_normalizef
; CHECK: ret void
define void @_Z14fast_normalizef_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @_Z14fast_normalizef(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z14fast_normalizef(float)


; CHECK: @_Z14fast_normalizeDv2_f_test
; CHECK-NOT: @_Z14fast_normalizeDv2_f
; CHECK: @_f_v._Z14fast_normalizeDv2_f
; CHECK-NOT: @_Z14fast_normalizeDv2_f
; CHECK: ret void
define void @_Z14fast_normalizeDv2_f_test(<2 x float>*, <2 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z14fast_normalizeDv2_f(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %ret_ptr = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  store <2 x float> %shuf_cast1, <2 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z14fast_normalizeDv2_f(<4 x float>)


; CHECK: @_Z14fast_normalizeDv3_f_test
; CHECK-NOT: @_Z14fast_normalizeDv3_f
; CHECK: @_f_v._Z14fast_normalizeDv3_f
; CHECK-NOT: @_Z14fast_normalizeDv3_f
; CHECK: ret void
define void @_Z14fast_normalizeDv3_f_test(<3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z14fast_normalizeDv3_f(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast1, <3 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z14fast_normalizeDv3_f(<4 x float>)


; CHECK: @_Z14fast_normalizef4_test
; CHECK-NOT: @_Z14fast_normalizef4
; CHECK: @_f_v._Z14fast_normalizef4
; CHECK-NOT: @_Z14fast_normalizef4
; CHECK: ret void
define void @_Z14fast_normalizef4_test(<4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z14fast_normalizef4(<4 x float> %load_arg) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z14fast_normalizef4(<4 x float>)


; CHECK: @_Z9normalizef_test
; CHECK-NOT: @_Z9normalizef
; CHECK: @_f_v._Z9normalizef
; CHECK-NOT: @_Z9normalizef
; CHECK: ret void
define void @_Z9normalizef_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @_Z9normalizef(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @_Z9normalizef(float)


; CHECK: @_Z9normalizeDv2_f_test
; CHECK-NOT: @_Z9normalizeDv2_f
; CHECK: @_f_v._Z9normalizeDv2_f
; CHECK-NOT: @_Z9normalizeDv2_f
; CHECK: ret void
define void @_Z9normalizeDv2_f_test(<2 x float>*, <2 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z9normalizeDv2_f(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %ret_ptr = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  store <2 x float> %shuf_cast1, <2 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z9normalizeDv2_f(<4 x float>)


; CHECK: @_Z9normalizeDv3_f_test
; CHECK-NOT: @_Z9normalizeDv3_f
; CHECK: @_f_v._Z9normalizeDv3_f
; CHECK-NOT: @_Z9normalizeDv3_f
; CHECK: ret void
define void @_Z9normalizeDv3_f_test(<3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z9normalizeDv3_f(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast1, <3 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z9normalizeDv3_f(<4 x float>)


; CHECK: @_Z9normalizeDv4_f_test
; CHECK-NOT: @_Z9normalizeDv4_f
; CHECK: @_f_v._Z9normalizeDv4_f
; CHECK-NOT: @_Z9normalizeDv4_f
; CHECK: ret void
define void @_Z9normalizeDv4_f_test(<4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %load_arg) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @_Z9normalizeDv4_f(<4 x float>)


; CHECK: @__ci_gamma_scalar_SPI_test
; CHECK-NOT: @__ci_gamma_scalar_SPI
; CHECK: @_f_v.__ci_gamma_scalar_SPI
; CHECK-NOT: @__ci_gamma_scalar_SPI
; CHECK: ret void
define void @__ci_gamma_scalar_SPI_test(<3 x float>*, float*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1          ; <<3 x float>> [#uses=1]
  %call = call <4 x float> @__ci_gamma_scalar_SPI(<4 x float> %shuf_cast, float %load_arg2) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %2, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast1, <3 x float>* %ret_ptr
  ret void
}
declare <4 x float> @__ci_gamma_scalar_SPI(<4 x float>, float %y) nounwind
