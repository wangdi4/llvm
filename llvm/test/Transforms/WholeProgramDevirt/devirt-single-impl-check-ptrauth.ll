; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
<<<<<<< HEAD
; Require SW DTrans to use %intel_devirt_options

; RUN: opt -S -passes=wholeprogramdevirt,verify -whole-program-visibility -pass-remarks=wholeprogramdevirt %intel_devirt_options %s 2>&1 | FileCheck %s ;INTEL
=======
; Require SW DTrans to use wholeprogramdevirt-multiversion=false option

; RUN: opt -S -passes=wholeprogramdevirt,verify -whole-program-visibility -pass-remarks=wholeprogramdevirt -wholeprogramdevirt-multiversion=false %s 2>&1 | FileCheck %s ;INTEL
>>>>>>> 2487df3fd9175703fa6c42a7e532ed62efd2d27f

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: remark: <unknown>:0:0: single-impl: devirtualized a call to vf
; CHECK: remark: <unknown>:0:0: devirtualized vf
; CHECK-NOT: devirtualized

@vt1 = constant [1 x ptr] [ptr @vf], !type !0
@vt2 = constant [1 x ptr] [ptr @vf], !type !0

define void @vf(ptr %this) {
  ret void
}

; CHECK: define void @call
define void @call(ptr %obj) {
  %vtable = load ptr, ptr %obj
  %pair = call {ptr, i1} @llvm.type.checked.load(ptr %vtable, i32 0, metadata !"typeid")
  %fptr = extractvalue {ptr, i1} %pair, 0
  %p = extractvalue {ptr, i1} %pair, 1
  ; CHECK: br i1 true,
  br i1 %p, label %cont, label %trap

cont:
  ; CHECK: call void @vf(
  call void %fptr(ptr %obj) [ "ptrauth"(i32 5, i64 120) ]
  ret void

trap:
  call void @llvm.trap()
  unreachable
}

declare {ptr, i1} @llvm.type.checked.load(ptr, i32, metadata)
declare void @llvm.trap()

!0 = !{i32 0, !"typeid"}

; end INTEL_FEATURE_SW_DTRANS
