; RUN: opt -passes="instcombine" -S %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

define spir_func void @test() {
; CHECK-LABEL: @test(
; CHECK-COUNT-1: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)
