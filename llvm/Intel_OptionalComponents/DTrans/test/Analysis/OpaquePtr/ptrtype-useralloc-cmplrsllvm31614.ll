; REQUIRES: asserts
; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -debug-only=dtrans-alloc-collector -passes=dtrans-ptrtypeanalyzertest -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test to ensure that special kinds of user allocation and free functions
; are checked on functions that were initially examined as a regular user
; allocation/free functions, but did not pass that test.

; CHECK-DAG: Analyzing for MallocWithStoredMMPtr _ZN6MemorynwEmPNS_13MemoryManagerE
; CHECK-DAG: Analyzing for FreeWithStoredMMPtr _ZN6MemorydlEPv

%class.MemManager = type { ptr }
%struct.Mem = type { ptr }
%struct.MemImpl = type { ptr, i32, i32, ptr }
%class.bad_alloc = type { %class.exception }
%class.exception = type { ptr }

@_ZTISt9bad_alloc = external dso_local constant ptr
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTISt9bad_alloc, ptr @_ZNSt9bad_allocD1Ev, ptr @_ZNSt9bad_allocD0Ev, ptr @_ZNKSt9bad_alloc4whatEv] }, !intel_dtrans_type !9

$__clang_call_terminate = comdat any
$_ZTS2Ex = comdat any
$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS2Ex }, comdat, !intel_dtrans_type !12

%struct.test01 = type { i64, i64 }
define void  @test01user() {
  %mm = alloca %class.MemManager
  %res = call ptr @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 16,  ptr %mm)
  %bc = bitcast ptr %res to ptr
  %f0 = getelementptr %struct.test01, ptr %bc, i64 0, i32 0
  store i64 0, ptr %f0
  call void @_ZN6MemorydlEPv(ptr %res)
  ret void
}

; This is recognized as user allocation function.
define "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr "intel_dtrans_func_index"="2" %this, i64 %size) personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) !intel.dtrans.func.type !15 {
entry:
  %call = invoke ptr @_Znwm(i64 %size)
          to label %if.then unwind label %lpad

lpad:                                             ; preds = %entry
  %lp1 = landingpad { ptr, i32 }
          catch ptr null
  %ext1 = extractvalue { ptr, i32 } %lp1, 0
  %call1 = tail call ptr @__cxa_begin_catch(ptr %ext1)
  %exception = tail call ptr @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(ptr nonnull %exception, ptr bitcast (ptr @_ZTI2Ex to ptr), ptr null)
          to label %unreachable unwind label %lpad2

lpad2:                                            ; preds = %lpad
  %lp2 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %lpad2
  %ext2 = extractvalue { ptr, i32 } %lp2, 0
  call void @_Unwind_Resume(ptr %ext2)
  unreachable

if.then:                                          ; preds = %entry
  ret ptr %call

terminate.lpad:                                   ; preds = %lpad2
  %lp3 = landingpad { ptr, i32 }
          catch ptr null
  %ext3 = extractvalue { ptr, i32 } %lp3, 0
  tail call void @__clang_call_terminate(ptr %ext3)
  unreachable

unreachable:                                      ; preds = %lpad
  unreachable
}

; This function should be checked as potentially being a MallocWithStoredMMPtr
; function because it contains a call to the function _ZN10MemManager8allocateEl
; which was recognized as a user allocation function.
define "intel_dtrans_func_index"="1" ptr @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 %size, ptr "intel_dtrans_func_index"="2" %mm) !intel.dtrans.func.type !18 {
entry:
  %new.size = add i64 %size, 8
  %bc1 = bitcast ptr %mm to ptr
  %ld1 = load ptr, ptr %bc1, align 8
  %gep1 = getelementptr inbounds ptr, ptr %ld1, i64 2
  %ld2 = load ptr, ptr %gep1, align 8
  %bc2 = bitcast ptr %ld2 to ptr
  %bc3 = bitcast ptr @_ZN10MemManager8allocateEl to ptr
  %cmp = icmp eq ptr %bc2, %bc3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                     ; preds = %entry
  %alloc1 = tail call ptr @_ZN10MemManager8allocateEl(ptr nonnull %mm, i64 %new.size)
  br label %merge

if.else:                                     ; preds = %entry
  ; Directly call 'new' to cause the function which had previously been
  ; checked as a user malloc function to be checked as a special candidate,
  ; which was a bug in CMPLRLLVM-31614.
  ; Calling 'new' here will cause the pattern to not match the special
  ; candidate code.
  %alloc2 = call ptr @_Znwm(i64 %size)
  br label %merge

merge:                                     ; preds = %if.else, %if.then
  %phi = phi ptr [ %alloc1, %if.then ], [ %alloc2, %if.else ]
  br label %return

return:                                     ; preds = %merge
  %bc4 = bitcast ptr %phi to ptr
  store ptr %mm, ptr %bc4, align 8
  %gep2 = getelementptr inbounds i8, ptr %phi, i64 8
  ret ptr %gep2
}

; This is recognized as user free function.
define void @_ZN10MemManager10deallocateEPv(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %p) !intel.dtrans.func.type !19 {
entry:
  tail call void @_ZdlPv(ptr %p)
  ret void
}

; This function should be checked as potentially being a FreeWithStoredMMPtr
; function because it contains a call to the function
; _ZN10MemManager10deallocateEPv which was recognized as a user free function.
define void @_ZN6MemorydlEPv(ptr "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !21 {
entry:
  %cmp1 = icmp eq ptr %ptr, null
  br i1 %cmp1, label %if.then, label %if.else

if.else:                                      ; preds = %entry
  %gep1 = getelementptr inbounds i8, ptr %ptr, i64 -8
  %bc1 = bitcast ptr %gep1 to ptr
  %ld1 = load ptr, ptr %bc1, align 8
  %bc2 = bitcast ptr %ld1 to ptr
  %ld2 = load ptr, ptr %bc2, align 8
  %gep2 = getelementptr inbounds ptr, ptr %ld2, i64 3
  %ld3 = load ptr, ptr %gep2, align 8
  %bc4 = bitcast ptr %ld3 to ptr
  %bc5 = bitcast ptr @_ZN10MemManager10deallocateEPv to ptr
  %cmp2 = icmp eq ptr %bc4, %bc5
  br i1 %cmp2, label %if.then.1, label %if.else.1

if.then.1:                                     ; preds = %if.else
  tail call void @_ZN10MemManager10deallocateEPv(ptr %ld1, ptr nonnull %gep1)
  br label %merge

if.else.1:                                     ; preds = %if.else
  ; Directly call 'delete' to cause the function which had previously been
  ; checked as a user free function to be checked as a special candidate,
  ; which was a bug in CMPLRLLVM-31614.
  ; Calling 'delete' here will cause the pattern to not match the special
  ; candidate code.
  tail call void @_ZdlPv(ptr %gep1)
  br label %merge

merge:                                         ; preds = %if.else.1, %if.then.1
  br label %next

next:                                          ; preds = %merge
  br label %if.then

if.then:                                      ; preds = %nex, %entry
  ret void
}

;comdat
define  void @__clang_call_terminate(ptr "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !22 {
  %call = tail call ptr @__cxa_begin_catch(ptr %ptr)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare !intel.dtrans.func.type !23 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare !intel.dtrans.func.type !24 void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !25 "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2")
declare !intel.dtrans.func.type !26 "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64)
declare !intel.dtrans.func.type !27 void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")
declare void @__cxa_end_catch()
declare  void @_ZSt9terminatev()
declare !intel.dtrans.func.type !28 void @_Unwind_Resume(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !30 void @_ZNSt9bad_allocD1Ev(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !31 "intel_dtrans_func_index"="1" ptr @_ZNKSt9bad_alloc4whatEv(ptr "intel_dtrans_func_index"="2")
declare !intel.dtrans.func.type !32 void @_ZNSt9bad_allocD0Ev(ptr "intel_dtrans_func_index"="1")

!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!5 = !{!4, i32 2}  ; i32 (...) **
!6 = !{%struct.Mem zeroinitializer, i32 1}  ; %struct.Mem*
!7 = !{i8 0, i32 2}  ; i8**
!8 = !{%class.exception zeroinitializer, i32 0}  ; %class.exception
!9 = !{!"L", i32 1, !10}  ; { [5 x i8*] }
!10 = !{!"A", i32 5, !11}  ; [5 x i8*]
!11 = !{i8 0, i32 1}  ; i8*
!12 = !{!"L", i32 2, !11, !11}  ; { i8*, i8* }
!13 = !{i64 0, i32 0}  ; i64
!14 = !{%class.MemManager zeroinitializer, i32 1}  ; %class.MemManager*
!15 = distinct !{!11, !14}
!16 = !{%struct.MemImpl zeroinitializer, i32 1}  ; %struct.MemImpl*
!17 = distinct !{!11, !16}
!18 = distinct !{!11, !14}
!19 = distinct !{!14, !11}
!20 = distinct !{!14, !11}
!21 = distinct !{!11}
!22 = distinct !{!11}
!23 = distinct !{!11}
!24 = distinct !{!11}
!25 = distinct !{!11, !11}
!26 = distinct !{!11}
!27 = distinct !{!11, !11, !11}
!28 = distinct !{!11}
!29 = !{%class.bad_alloc zeroinitializer, i32 1}  ; %class.bad_alloc*
!30 = distinct !{!29}
!31 = distinct !{!11, !29}
!32 = distinct !{!29}
!33 = !{!"S", %class.MemManager zeroinitializer, i32 1, !3} ; { i32 (...)** }
!34 = !{!"S", %struct.Mem zeroinitializer, i32 1, !5} ; { i32 (...) ** }
!35 = !{!"S", %struct.MemImpl zeroinitializer, i32 4, !6, !2, !2, !7} ; { %struct.Mem*, i32, i32, i8** }
!36 = !{!"S", %class.bad_alloc zeroinitializer, i32 1, !8} ; { %class.exception }
!37 = !{!"S", %class.exception zeroinitializer, i32 1, !3} ; { i32 (...)** }
!38 = !{!"S", %struct.test01 zeroinitializer, i32 2, !13, !13} ; { i64, i64 }

!intel.dtrans.types = !{!33, !34, !35, !36, !37, !38}
