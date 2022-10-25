; This test verifies that "DummyAlloc" is treated as allocation function and 
; "Bad casting" safety issue is not set for %struct.test01b.

; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Call graph: enclosing type: struct.test01a
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test01b

%struct.test01a = type { i32, ptr }
%struct.test01b = type { i32 }
%struct.test01c = type { ptr }

define void @foo(ptr "intel_dtrans_func_index"="1" %arg1, ptr "intel_dtrans_func_index"="1" %arg2) personality ptr null !intel.dtrans.func.type !3 {
bbb:
 %i = getelementptr inbounds %struct.test01a, ptr %arg1, i64 0, i32 1
 br i1 undef, label %bb0, label %bb1

bb0:
  %i1 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef 32)
          to label %bb4 unwind label %bb3

bb1:
  %i2 = invoke ptr @DummyAlloc(ptr %arg2, i64 32)
          to label %bb4 unwind label %bb3

bb4:                                               ; preds = %bb0, %bb1
  %ph = phi ptr [ %i1, %bb0 ], [ %i2, %bb1 ]
  br label %bb2

bb2:                                              ; preds = %bb4
  store ptr %ph, ptr %i, align 8
  br label %common.ret

bb3:                                              ; preds = %bb
  %i4 = landingpad { ptr, i32 }
          cleanup
  br label %common.ret

common.ret:                                       ; preds = %bb3, %bb2
  ret void
}

define "intel_dtrans_func_index"="1" ptr @DummyAlloc(ptr "intel_dtrans_func_index"="2" %arg, i64 %arg1) !intel.dtrans.func.type !7 {
bb:
  %c = tail call ptr @__cxa_allocate_exception(i64 8)
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr null, i64 0, inrange i32 0, i64 2), ptr %c, align 8
  tail call void @__cxa_throw(ptr nonnull %c, ptr null, ptr null)
  unreachable
}

declare !intel.dtrans.func.type !8 dso_local noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare !intel.dtrans.func.type !8 dso_local noalias "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64)
declare !intel.dtrans.func.type !9 dso_local void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")


!intel.dtrans.types = !{!4, !5, !10}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01b zeroinitializer, i32 1}
!3 = distinct !{!13, !6}
!4 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test02b* }
!5 = !{!"S", %struct.test01b zeroinitializer, i32 1, !1} ; { i32 }
!6 = !{i8 0, i32 1}
!7 = distinct !{!6, !11}
!8 =  distinct !{!6}
!9 = distinct !{!6, !6, !6}
!10 = !{!"S", %struct.test01c zeroinitializer, i32 1, !6} ; { i8* }
!11 = !{%struct.test01c zeroinitializer, i32 1}
!12 =  distinct !{!11}
!13 = !{%struct.test01a zeroinitializer, i32 1}
