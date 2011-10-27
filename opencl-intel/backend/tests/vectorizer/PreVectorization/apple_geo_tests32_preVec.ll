; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnPreVec %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'testModule'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @__crossf3_test
; CHECK-NOT: @__crossf3
; CHECK: @_f_v.__crossf3
; CHECK-NOT: @__crossf3
; CHECK: ret void
define void @__crossf3_test(<3 x float>*, <3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__crossf3(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <<4 x float>> [#uses=1]
  %shuf_cast4 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %2, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast4, <3 x float>* %ret_ptr
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @__crossf3(<4 x float>, <4 x float>)


; CHECK:@__crossf4_test
; CHECK-NOT: @__crossf4
; CHECK: @_f_v.__crossf4
; CHECK-NOT: @__crossf4
; CHECK: ret void
define void @__crossf4_test(<4 x float>*, <4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__crossf4(<4 x float> %load_arg, <4 x float> %load_arg2) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %2, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__crossf4(<4 x float>, <4 x float>)


; CHECK: @__fast_distancef_test
; CHECK-NOT: @__fast_distancef
; CHECK: @_f_v.__fast_distancef
; CHECK-NOT: @__fast_distancef
; CHECK: ret void
define void @__fast_distancef_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @__fast_distancef(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_distancef(float, float)


; CHECK: @__fast_distancef2_test
; CHECK-NOT: @__fast_distancef2
; CHECK: @_f_v.__fast_distancef2
; CHECK-NOT: @__fast_distancef2
; CHECK: ret void
define void @__fast_distancef2_test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__fast_distancef2(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_distancef2(<4 x float>, <4 x float>)


; CHECK: @__fast_distancef3_test
; CHECK-NOT: @__fast_distancef3
; CHECK: @_f_v.__fast_distancef3
; CHECK-NOT: @__fast_distancef3
; CHECK: ret void
define void @__fast_distancef3_test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__fast_distancef3(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_distancef3(<4 x float>, <4 x float>)


; CHECK: @__fast_distancef4_test
; CHECK-NOT: @__fast_distancef4
; CHECK: @_f_v.__fast_distancef4
; CHECK-NOT: @__fast_distancef4
; CHECK: ret void
define void @__fast_distancef4_test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @__fast_distancef4(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_distancef4(<4 x float>, <4 x float>)


; CHECK: @__distancef_test
; CHECK-NOT: @__distancef
; CHECK: @_f_v.__distancef
; CHECK-NOT: @__distancef
; CHECK: ret void
define void @__distancef_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @__distancef(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__distancef(float, float)


; CHECK: @__lengthf_test
; CHECK-NOT: @__lengthf
; CHECK: @_f_v.__lengthf
; CHECK-NOT: @__lengthf
; CHECK: ret void
define void @__lengthf_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @__lengthf(float %load_arg)  ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__lengthf(float)


; CHECK: @__distancef2_test
; CHECK-NOT: @__distancef2
; CHECK: @_f_v.__distancef2
; CHECK-NOT: @__distancef2
; CHECK: ret void
define void @__distancef2_test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__distancef2(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__distancef2(<4 x float>, <4 x float>)


; CHECK: @__lengthf2_test
; CHECK-NOT: @__lengthf2
; CHECK: @_f_v.__lengthf2
; CHECK-NOT: @__lengthf2
; CHECK: ret void
define void @__lengthf2_test(<2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__lengthf2(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__lengthf2(<4 x float>)


; CHECK:  @__distancef3_test
; CHECK-NOT: @__distancef3
; CHECK: @_f_v.__distancef3
; CHECK-NOT: @__distancef3
; CHECK: ret void
define void @__distancef3_test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__distancef3(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__distancef3(<4 x float>, <4 x float>)


; CHECK: @__lengthf3_test
; CHECK-NOT: @__lengthf3
; CHECK: @_f_v.__lengthf3
; CHECK-NOT: @__lengthf3
; CHECK: ret void
define void @__lengthf3_test(<3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__lengthf3(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__lengthf3(<4 x float>)


; CHECK: @__distancef4_test
; CHECK-NOT: @__distancef4
; CHECK: @_f_v.__distancef4
; CHECK-NOT: @__distancef4
; CHECK: ret void
define void @__distancef4_test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @__distancef4(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__distancef4(<4 x float>, <4 x float>)


; CHECK: @__lengthf4_test
; CHECK-NOT: @__lengthf4
; CHECK: @_f_v.__lengthf4
; CHECK-NOT: @__lengthf4
; CHECK: ret void
define void @__lengthf4_test(<4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call float @__lengthf4(<4 x float> %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__lengthf4(<4 x float>)


; CHECK: @__dotf_test
; CHECK-NOT: @__dotf
; CHECK: @_f_v.__dotf
; CHECK-NOT: @__dotf
; CHECK: ret void
define void @__dotf_test(float*, float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %arg_ptr1 = getelementptr float* %1, i32 %gid   ; <float*> [#uses=1]
  %load_arg2 = load float* %arg_ptr1              ; <float> [#uses=1]
  %call = call float @__dotf(float %load_arg, float %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__dotf(float, float)


; CHECK: @__dotf2_test
; CHECK-NOT: @__dotf2
; CHECK: @_f_v.__dotf2
; CHECK-NOT: @__dotf2
; CHECK: ret void
define void @__dotf2_test(<2 x float>*, <2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg2 = load <2 x float>* %arg_ptr1        ; <<2 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <2 x float> %load_arg2, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__dotf2(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__dotf2(<4 x float>, <4 x float>)


; CHECK: @__dotf3_test
; CHECK-NOT: @__dotf3
; CHECK: @_f_v.__dotf3
; CHECK-NOT: @__dotf3
; CHECK: ret void
define void @__dotf3_test(<3 x float>*, <3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg2 = load <3 x float>* %arg_ptr1        ; <<3 x float>> [#uses=1]
  %shuf_cast3 = shufflevector <3 x float> %load_arg2, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__dotf3(<4 x float> %shuf_cast, <4 x float> %shuf_cast3) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__dotf3(<4 x float>, <4 x float>)


; CHECK: @__dotf4_test
; CHECK-NOT: @__dotf4
; CHECK: @_f_v.__dotf4
; CHECK-NOT: @__dotf4
; CHECK: ret void
define void @__dotf4_test(<4 x float>*, <4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=3]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %arg_ptr1 = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg2 = load <4 x float>* %arg_ptr1        ; <<4 x float>> [#uses=1]
  %call = call float @__dotf4(<4 x float> %load_arg, <4 x float> %load_arg2) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %2, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__dotf4(<4 x float>, <4 x float>)


; CHECK: @__fast_lengthf_test
; CHECK-NOT: @__fast_lengthf
; CHECK: @_f_v.__fast_lengthf
; CHECK-NOT: @__fast_lengthf
; CHECK: ret void
define void @__fast_lengthf_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @__fast_lengthf(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_lengthf(float)


; CHECK: @__fast_lengthf2_test
; CHECK-NOT: @__fast_lengthf2
; CHECK: @_f_v.__fast_lengthf2
; CHECK-NOT: @__fast_lengthf2
; CHECK: ret void
define void @__fast_lengthf2_test(<2 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__fast_lengthf2(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_lengthf2(<4 x float>)


; CHECK: @__fast_lengthf3_test
; CHECK-NOT: @__fast_lengthf3
; CHECK: @_f_v.__fast_lengthf3
; CHECK-NOT: @__fast_lengthf3
; CHECK: ret void
define void @__fast_lengthf3_test(<3 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call float @__fast_lengthf3(<4 x float> %shuf_cast) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_lengthf3(<4 x float>)


; CHECK: @__fast_lengthf4_test
; CHECK-NOT: @__fast_lengthf4
; CHECK: @_f_v.__fast_lengthf4
; CHECK-NOT: @__fast_lengthf4
; CHECK: ret void
define void @__fast_lengthf4_test(<4 x float>*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call float @__fast_lengthf4(<4 x float> %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_lengthf4(<4 x float>)


; CHECK: @__fast_normalizef_test
; CHECK-NOT: @__fast_normalizef
; CHECK: @_f_v.__fast_normalizef
; CHECK-NOT: @__fast_normalizef
; CHECK: ret void
define void @__fast_normalizef_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @__fast_normalizef(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__fast_normalizef(float)


; CHECK: @__fast_normalizef2_test
; CHECK-NOT: @__fast_normalizef2
; CHECK: @_f_v.__fast_normalizef2
; CHECK-NOT: @__fast_normalizef2
; CHECK: ret void
define void @__fast_normalizef2_test(<2 x float>*, <2 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__fast_normalizef2(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %ret_ptr = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  store <2 x float> %shuf_cast1, <2 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__fast_normalizef2(<4 x float>)


; CHECK: @__fast_normalizef3_test
; CHECK-NOT: @__fast_normalizef3
; CHECK: @_f_v.__fast_normalizef3
; CHECK-NOT: @__fast_normalizef3
; CHECK: ret void
define void @__fast_normalizef3_test(<3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__fast_normalizef3(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast1, <3 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__fast_normalizef3(<4 x float>)


; CHECK: @__fast_normalizef4_test
; CHECK-NOT: @__fast_normalizef4
; CHECK: @_f_v.__fast_normalizef4
; CHECK-NOT: @__fast_normalizef4
; CHECK: ret void
define void @__fast_normalizef4_test(<4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__fast_normalizef4(<4 x float> %load_arg) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__fast_normalizef4(<4 x float>)


; CHECK: @__normalizef_test
; CHECK-NOT: @__normalizef
; CHECK: @_f_v.__normalizef
; CHECK-NOT: @__normalizef
; CHECK: ret void
define void @__normalizef_test(float*, float*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr float* %0, i32 %gid    ; <float*> [#uses=1]
  %load_arg = load float* %arg_ptr                ; <float> [#uses=1]
  %call = call float @__normalizef(float %load_arg) ; <float> [#uses=1]
  %ret_ptr = getelementptr float* %1, i32 %gid    ; <float*> [#uses=1]
  store float %call, float* %ret_ptr
  ret void
}

declare float @__normalizef(float)


; CHECK: @__normalizef2_test
; CHECK-NOT: @__normalizef2
; CHECK: @_f_v.__normalizef2
; CHECK-NOT: @__normalizef2
; CHECK: ret void
define void @__normalizef2_test(<2 x float>*, <2 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <2 x float>* %0, i32 %gid ; <<2 x float>*> [#uses=1]
  %load_arg = load <2 x float>* %arg_ptr          ; <<2 x float>> [#uses=1]
  %shuf_cast = shufflevector <2 x float> %load_arg, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__normalizef2(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %ret_ptr = getelementptr <2 x float>* %1, i32 %gid ; <<2 x float>*> [#uses=1]
  store <2 x float> %shuf_cast1, <2 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__normalizef2(<4 x float>)


; CHECK: @__normalizef3_test
; CHECK-NOT: @__normalizef3
; CHECK: @_f_v.__normalizef3
; CHECK-NOT: @__normalizef3
; CHECK: ret void
define void @__normalizef3_test(<3 x float>*, <3 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <3 x float>* %0, i32 %gid ; <<3 x float>*> [#uses=1]
  %load_arg = load <3 x float>* %arg_ptr          ; <<3 x float>> [#uses=1]
  %shuf_cast = shufflevector <3 x float> %load_arg, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__normalizef3(<4 x float> %shuf_cast) ; <<4 x float>> [#uses=1]
  %shuf_cast1 = shufflevector <4 x float> %call, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %ret_ptr = getelementptr <3 x float>* %1, i32 %gid ; <<3 x float>*> [#uses=1]
  store <3 x float> %shuf_cast1, <3 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__normalizef3(<4 x float>)


; CHECK: @__normalizef4_test
; CHECK-NOT: @__normalizef4
; CHECK: @_f_v.__normalizef4
; CHECK-NOT: @__normalizef4
; CHECK: ret void
define void @__normalizef4_test(<4 x float>*, <4 x float>*) {
entry:
  %gid = call i32 @get_global_id(i32 0)           ; <i32> [#uses=2]
  %arg_ptr = getelementptr <4 x float>* %0, i32 %gid ; <<4 x float>*> [#uses=1]
  %load_arg = load <4 x float>* %arg_ptr          ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @__normalizef4(<4 x float> %load_arg) ; <<4 x float>> [#uses=1]
  %ret_ptr = getelementptr <4 x float>* %1, i32 %gid ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %ret_ptr
  ret void
}

declare <4 x float> @__normalizef4(<4 x float>)


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
