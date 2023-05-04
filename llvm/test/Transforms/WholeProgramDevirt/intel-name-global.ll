; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -S -passes=wholeprogramdevirt -whole-program-visibility %intel_devirt_options %s 2>&1 | FileCheck %s

; This test is to check that the virtual function constant variables are not
; changed to unnamed variables in the case where aliases get introduced to
; reference them by the devirtualization pass.

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: @0 = private constant
; CHECK: [[VT1DATA:@_[^ ]*]] = private constant { [8 x i8], [3 x ptr], [0 x i8] } { [8 x i8] zeroinitializer, [3 x ptr] [ptr @vf0i1, ptr @vf1i1, ptr @vf1i32], [0 x i8] zeroinitializer }, section "vt1sec"
@vt1 = constant [3 x ptr] [ptr @vf0i1, ptr @vf1i1, ptr @vf1i32], section "vt1sec", !type !0

; CHECK-NOT: @1 = private constant
; CHECK: [[VT2DATA:@_[^ ]*]] = private constant { [8 x i8], [3 x ptr], [0 x i8] } { [8 x i8] c"\00\00\00\00\00\00\00\01", [3 x ptr] [ptr @vf1i1, ptr @vf0i1, ptr @vf2i32], [0 x i8] zeroinitializer }
@vt2 = constant [3 x ptr] [ptr @vf1i1, ptr @vf0i1, ptr @vf2i32], !type !0

; CHECK-NOT: @2 = private constant
; CHECK: [[VT3DATA:@_[^ ]*]] = private constant { [1 x i8], [3 x ptr], [0 x i8] } { [1 x i8] zeroinitializer, [3 x ptr] [ptr @vf0i1, ptr @vf1i1, ptr @vf3i32], [0 x i8] zeroinitializer }
@vt3 = constant [3 x ptr] [ptr @vf0i1, ptr @vf1i1, ptr @vf3i32], align 1, !type !0

; CHECK-NOT: @3 = private constant
; CHECK: [[VT4DATA:@_[^ ]*]] = private constant { [16 x i8], [3 x ptr], [0 x i8] } { [16 x i8] c"\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\01", [3 x ptr] [ptr @vf1i1, ptr @vf0i1, ptr @vf4i32], [0 x i8] zeroinitializer }
@vt4 = constant [3 x ptr] [ptr @vf1i1, ptr @vf0i1, ptr @vf4i32], align 16, !type !0

; CHECK: @vt1 = alias [3 x ptr], getelementptr inbounds ({ [8 x i8], [3 x ptr], [0 x i8] }, ptr @__Devirt, i32 0, i32 1)
; CHECK: @vt2 = alias [3 x ptr], getelementptr inbounds ({ [8 x i8], [3 x ptr], [0 x i8] }, ptr @__Devirt.1, i32 0, i32 1)
; CHECK: @vt3 = alias [3 x ptr], getelementptr inbounds ({ [1 x i8], [3 x ptr], [0 x i8] }, ptr @__Devirt.2, i32 0, i32 1)
; CHECK: @vt4 = alias [3 x ptr], getelementptr inbounds ({ [16 x i8], [3 x ptr], [0 x i8] }, ptr @__Devirt.3, i32 0, i32 1)

define i1 @vf0i1(ptr %this) readnone {
  ret i1 0
}

define i1 @vf1i1(ptr %this) readnone {
  ret i1 1
}

define i32 @vf1i32(ptr %this) readnone {
  ret i32 1
}

define i32 @vf2i32(ptr %this) readnone {
  ret i32 2
}

define i32 @vf3i32(ptr %this) readnone {
  ret i32 3
}

define i32 @vf4i32(ptr %this) readnone {
  ret i32 4
}

define i1 @call1(ptr %obj) {
  %vtableptr = bitcast ptr %obj to ptr
  %vtable = load ptr, ptr %vtableptr, align 8
  %vtablei8 = bitcast ptr %vtable to ptr
  %p = call i1 @llvm.type.test(ptr %vtablei8, metadata !"typeid")
  call void @llvm.assume(i1 %p)
  %fptrptr = getelementptr [3 x ptr], ptr %vtable, i32 0, i32 0
  %fptr = load ptr, ptr %fptrptr, align 8
  %fptr_casted = bitcast ptr %fptr to ptr
  %result = call i1 %fptr_casted(ptr %obj)
  ret i1 %result
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)
declare void @__cxa_pure_virtual()

!0 = !{i32 0, !"typeid"}

; end INTEL_FEATURE_SW_DTRANS
