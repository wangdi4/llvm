; RUN: opt -passes=dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(float %x, i64* %y) nounwind {
  %tmp = fptosi float %x to i64
  store i64 %tmp, i64* %y
  ret void
}

; CHECK: call i64 @_Z12convert_longf(float %x)

; DEBUGIFY-NOT: WARNING
