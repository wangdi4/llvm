; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test checks that devirtualization with multiversioning will be applied
; for target functions that contain exception handling.

; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=wholeprogramdevirt -whole-program-visibility -wholeprogramdevirt-multiversion=true %s | FileCheck %s

; Check that the intrinsic for branch funnel wasn't created
; CHECK-NOT: declare void @llvm.icall.branch.funnel(...)

; Check that the virtual call %fptr_casted in @test1 is multiversioned into
; vfn_1 and vfn_2 calls. vfn_1 has exception handling code.
; CHECK: define i32 @test1(ptr %obj)
; CHECK-DAG: call i32 @vfn_1(ptr %obj, i32 1)
; CHECK-DAG: call i32 @vfn_2(ptr %obj, i32 1)


target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

; Virtual tables
@vt_1 = constant [1 x ptr] [ptr @vfn_1], !type !0
@vt_2 = constant [1 x ptr] [ptr @vfn_2], !type !0

declare i32 @__CxxFrameHandler3(...)

; Function vfn_1 has exception handling
define i32 @vfn_1(ptr %this, i32 %arg) personality ptr @__CxxFrameHandler3 {
entry:
  invoke void @try()
    to label %exit unwind label %catch.dispatch

catch.dispatch:
  %cs = catchswitch within none [label %catch.pad1, label %catch.pad2] unwind to caller

catch.pad1:
  %catchpad1 = catchpad within %cs [i32 1]
  call void @catch()
  catchret from %catchpad1 to label %exit

catch.pad2:
  %catchpad2 = catchpad within %cs [i32 2]
  unreachable

exit:
  ret i32 1
}

declare void @try()
declare void @catch() #1
declare i32 @vfn_2(ptr %this, i32 %arg)

define i32 @test1(ptr %obj) #0 {
  ; Load the virtual table
  %vtableptr = bitcast ptr %obj to ptr
  %vtable = load ptr, ptr %vtableptr
  %vtablei8 = bitcast ptr %vtable to ptr

  ; Match the virtual table with type_id1 (vfn_1 and vfn2)
  %p = call i1 @llvm.type.test(ptr %vtablei8, metadata !"type_id1")
  call void @llvm.assume(i1 %p)

  ; Load the virtual pointer
  %fptrptr = getelementptr [1 x ptr], ptr %vtable, i32 0, i32 0
  %fptr = load ptr, ptr %fptrptr
  %fptr_casted = bitcast ptr %fptr to ptr

  ; Call to virtual function
  %result = call i32 %fptr_casted(ptr %obj, i32 1)
  ret i32 %result
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)

!0 = !{i32 0, !"type_id1"}

attributes #0 = { "target-features"="+retpoline" }
attributes #1 = { nounwind }

; end INTEL_FEATURE_SW_DTRANS
