; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

define void @sample_test(<4 x i8> %x, <4 x i8> %y) {
entry:
; CHECK-COUNT-2: shufflevector <4 x i8> %x, <4 x i8> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-COUNT-2: shufflevector <4 x i8> %x, <4 x i8> %y, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %concatVectors = alloca <8 x i8>, align 8
  store <8 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7>, ptr %concatVectors, align 8
  %mask = load <8 x i8>, ptr %concatVectors, align 8
  %call1 = call <8 x i8> @_Z20__ocl_helper_shuffleDv4_cDv8_h(<4 x i8> noundef %x, <8 x i8> noundef %mask)
  %call2 = call <8 x i8> @_Z20__ocl_helper_shuffleDv4_cDv8_h(<4 x i8> noundef %x, <8 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7>)
  %call3 = call <8 x i8> @_Z21__ocl_helper_shuffle2Dv4_cS_Dv8_h(<4 x i8> noundef %x, <4 x i8> noundef %y, <8 x i8> noundef %mask)
  %call4 = call <8 x i8> @_Z21__ocl_helper_shuffle2Dv4_cS_Dv8_h(<4 x i8> noundef %x, <4 x i8> noundef %y, <8 x i8> <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7>)
  ret void
}

declare <8 x i8> @_Z20__ocl_helper_shuffleDv4_cDv8_h(<4 x i8> noundef, <8 x i8> noundef)
declare <8 x i8> @_Z21__ocl_helper_shuffle2Dv4_cS_Dv8_h(<4 x i8> noundef, <4 x i8> noundef, <8 x i8> noundef)

; DEBUGIFY-NOT: WARNING
