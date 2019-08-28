; This test checks that devirtualization with multiversioning won't be applied
; in functions that contain exception handling. It should generate the
; branch funnel intrinsic.

; RUN: opt -S -wholeprogramdevirt -wholeprogramdevirt-multiversion=true %s | FileCheck %s

; Check that the virtual call in %result was substituted with a call to
; the branch funnel
; CHECK: define i32 @test1(i8* %obj)
; CHECK: %2 = call i32 bitcast (void (i8*, ...)* @__typeid_type_id1_0_branch_funnel to i32 (i8*, i8*, i32)*)(i8* nest %1, i8* %obj, i32 1)


; Check that the wrapper for the branch funnel was created
; CHECK: define hidden void @__typeid_type_id1_0_branch_funnel(i8* nest %0, ...

; Check that the branch funnel call was created with with the virtual calls vfn_1
; and vfn_2
; CHECK: musttail call void (...) @llvm.icall.branch.funnel(i8* %0, i8* bitcast ([1 x i8*]* @vt_1 to i8*), i32 (i8*, i32)* @vfn_1, i8* bitcast ([1 x i8*]* @vt_2 to i8*), i32 (i8*, i32)* @vfn_2, ...)

; Check that the intrinsic for branch funnel was created
; CHECK: declare void @llvm.icall.branch.funnel(...)

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

; Virtual tables
@vt_1 = constant [1 x i8*] [i8* bitcast (i32 (i8*, i32)* @vfn_1 to i8*)], !type !0
@vt_2 = constant [1 x i8*] [i8* bitcast (i32 (i8*, i32)* @vfn_2 to i8*)], !type !0

declare i32 @__CxxFrameHandler3(...)

; Function vfn_1 has exception handling
define i32 @vfn_1(i8* %this, i32 %arg) personality i32 (...)* @__CxxFrameHandler3 {
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
declare i32 @vfn_2(i8* %this, i32 %arg)

define i32 @test1(i8* %obj) #0 {
  ; Load the virtual table
  %vtableptr = bitcast i8* %obj to [1 x i8*]**
  %vtable = load [1 x i8*]*, [1 x i8*]** %vtableptr
  %vtablei8 = bitcast [1 x i8*]* %vtable to i8*

  ; Match the virtual table with type_id1 (vfn_1 and vfn2)
  %p = call i1 @llvm.type.test(i8* %vtablei8, metadata !"type_id1")
  call void @llvm.assume(i1 %p)

  ; Load the virtual pointer
  %fptrptr = getelementptr [1 x i8*], [1 x i8*]* %vtable, i32 0, i32 0
  %fptr = load i8*, i8** %fptrptr
  %fptr_casted = bitcast i8* %fptr to i32 (i8*, i32)*

  ; Call to virtual function
  %result = call i32 %fptr_casted(i8* %obj, i32 1)
  ret i32 %result
}

declare i1 @llvm.type.test(i8*, metadata)
declare void @llvm.assume(i1)

!0 = !{i32 0, !"type_id1"}

attributes #0 = { "target-features"="+retpoline" }
attributes #1 = { nounwind }
