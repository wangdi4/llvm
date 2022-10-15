; REQUIRES: asserts
; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -debug-only=dtrans-alloc-collector -dtrans-ptrtypeanalyzertest -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -debug-only=dtrans-alloc-collector -passes=dtrans-ptrtypeanalyzertest -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test to ensure that special kinds of user allocation and free functions
; are checked on functions that were initially examined as a regular user
; allocation/free functions, but did not pass that test.

; CHECK-DAG: Analyzing for MallocWithStoredMMPtr _ZN6MemorynwEmPNS_13MemoryManagerE
; CHECK-DAG: Analyzing for FreeWithStoredMMPtr _ZN6MemorydlEPv

%class.MemManager = type { i32 (...)** }
%struct.Mem = type { i32 (...) ** }
%struct.MemImpl = type { %struct.Mem*, i32, i32, i8** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

@_ZTISt9bad_alloc = external dso_local constant i8*
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD0Ev to i8*), i8* bitcast (i8* (%class.bad_alloc*)* @_ZNKSt9bad_alloc4whatEv to i8*)] }, !intel_dtrans_type !9

$__clang_call_terminate = comdat any
$_ZTS2Ex = comdat any
$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, !intel_dtrans_type !12

%struct.test01 = type { i64, i64 }
define void  @test01user() {
  %mm = alloca %class.MemManager
  %res = call i8* @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 16,  %class.MemManager* %mm)
  %bc = bitcast i8* %res to %struct.test01*
  %f0 = getelementptr %struct.test01, %struct.test01* %bc, i64 0, i32 0
  store i64 0, i64* %f0
  call void @_ZN6MemorydlEPv(i8* %res)
  ret void
}

; This is recognized as user allocation function.
define "intel_dtrans_func_index"="1" i8* @_ZN10MemManager8allocateEl(%class.MemManager* "intel_dtrans_func_index"="2" %this, i64 %size) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !15 {
entry:
  %call = invoke i8* @_Znwm(i64 %size)
          to label %if.then unwind label %lpad

lpad:                                             ; preds = %entry
  %lp1 = landingpad { i8*, i32 }
          catch i8* null
  %ext1 = extractvalue { i8*, i32 } %lp1, 0
  %call1 = tail call i8* @__cxa_begin_catch(i8* %ext1)
  %exception = tail call i8* @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8* }* @_ZTI2Ex to i8*), i8* null)
          to label %unreachable unwind label %lpad2

lpad2:                                            ; preds = %lpad
  %lp2 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %lpad2
  %ext2 = extractvalue { i8*, i32 } %lp2, 0
  call void @_Unwind_Resume(i8* %ext2)
  unreachable

if.then:                                          ; preds = %entry
  ret i8* %call

terminate.lpad:                                   ; preds = %lpad2
  %lp3 = landingpad { i8*, i32 }
          catch i8* null
  %ext3 = extractvalue { i8*, i32 } %lp3, 0
  tail call void @__clang_call_terminate(i8* %ext3)
  unreachable

unreachable:                                      ; preds = %lpad
  unreachable
}

; This function should be checked as potentially being a MallocWithStoredMMPtr
; function because it contains a call to the function _ZN10MemManager8allocateEl
; which was recognized as a user allocation function.
define "intel_dtrans_func_index"="1" i8* @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 %size, %class.MemManager* "intel_dtrans_func_index"="2" %mm) !intel.dtrans.func.type !18 {
entry:
  %new.size = add i64 %size, 8
  %bc1 = bitcast %class.MemManager* %mm to i8* (%class.MemManager*, i64)***
  %ld1 = load i8* (%class.MemManager*, i64)**, i8* (%class.MemManager*, i64)*** %bc1, align 8
  %gep1 = getelementptr inbounds i8* (%class.MemManager*, i64)*, i8* (%class.MemManager*, i64)** %ld1, i64 2
  %ld2 = load i8* (%class.MemManager*, i64)*, i8* (%class.MemManager*, i64)** %gep1, align 8
  %bc2 = bitcast i8* (%class.MemManager*, i64)* %ld2 to i8*
  %bc3 = bitcast i8* (%class.MemManager*, i64)* @_ZN10MemManager8allocateEl to i8*
  %cmp = icmp eq i8* %bc2, %bc3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                     ; preds = %entry
  %alloc1 = tail call i8* bitcast (i8* (%class.MemManager*, i64)* @_ZN10MemManager8allocateEl to i8* (%class.MemManager*, i64)*)(%class.MemManager* nonnull %mm, i64 %new.size)
  br label %merge

if.else:                                     ; preds = %entry
  ; Directly call 'new' to cause the function which had previously been
  ; checked as a user malloc function to be checked as a special candidate,
  ; which was a bug in CMPLRLLVM-31614.
  ; Calling 'new' here will cause the pattern to not match the special
  ; candidate code.
  %alloc2 = call i8* @_Znwm(i64 %size)
  br label %merge

merge:                                     ; preds = %if.else, %if.then
  %phi = phi i8* [ %alloc1, %if.then ], [ %alloc2, %if.else ]
  br label %return

return:                                     ; preds = %merge
  %bc4 = bitcast i8* %phi to %class.MemManager**
  store %class.MemManager* %mm, %class.MemManager** %bc4, align 8
  %gep2 = getelementptr inbounds i8, i8* %phi, i64 8
  ret i8* %gep2
}

; This is recognized as user free function.
define void @_ZN10MemManager10deallocateEPv(%class.MemManager* "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) !intel.dtrans.func.type !19 {
entry:
  tail call void @_ZdlPv(i8* %p)
  ret void
}

; This function should be checked as potentially being a FreeWithStoredMMPtr
; function because it contains a call to the function
; _ZN10MemManager10deallocateEPv which was recognized as a user free function.
define void @_ZN6MemorydlEPv(i8* "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !21 {
entry:
  %cmp1 = icmp eq i8* %ptr, null
  br i1 %cmp1, label %if.then, label %if.else

if.else:                                      ; preds = %entry
  %gep1 = getelementptr inbounds i8, i8* %ptr, i64 -8
  %bc1 = bitcast i8* %gep1 to %class.MemManager**
  %ld1 = load %class.MemManager*, %class.MemManager** %bc1, align 8
  %bc2 = bitcast %class.MemManager* %ld1 to void (%class.MemManager*, i8*)***
  %ld2 = load void (%class.MemManager*, i8*)**, void (%class.MemManager*, i8*)*** %bc2, align 8
  %gep2 = getelementptr inbounds void (%class.MemManager*, i8*)*, void (%class.MemManager*, i8*)** %ld2, i64 3
  %ld3 = load void (%class.MemManager*, i8*)*, void (%class.MemManager*, i8*)** %gep2, align 8
  %bc4 = bitcast void (%class.MemManager*, i8*)* %ld3 to i8*
  %bc5 = bitcast void (%class.MemManager*, i8*)* @_ZN10MemManager10deallocateEPv to i8*
  %cmp2 = icmp eq i8* %bc4, %bc5
  br i1 %cmp2, label %if.then.1, label %if.else.1

if.then.1:                                     ; preds = %if.else
  tail call void bitcast (void (%class.MemManager*, i8*)* @_ZN10MemManager10deallocateEPv to void (%class.MemManager*, i8*)*)(%class.MemManager* %ld1, i8* nonnull %gep1)
  br label %merge

if.else.1:                                     ; preds = %if.else
  ; Directly call 'delete' to cause the function which had previously been
  ; checked as a user free function to be checked as a special candidate,
  ; which was a bug in CMPLRLLVM-31614.
  ; Calling 'delete' here will cause the pattern to not match the special
  ; candidate code.
  tail call void @_ZdlPv(i8* %gep1)
  br label %merge

merge:                                         ; preds = %if.else.1, %if.then.1
  br label %next

next:                                          ; preds = %merge
  br label %if.then

if.then:                                      ; preds = %nex, %entry
  ret void
}

;comdat
define  void @__clang_call_terminate(i8* "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !22 {
  %call = tail call i8* @__cxa_begin_catch(i8* %ptr)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare !intel.dtrans.func.type !23 "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
declare !intel.dtrans.func.type !24 void @_ZdlPv(i8* "intel_dtrans_func_index"="1")

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !25 "intel_dtrans_func_index"="1" i8* @__cxa_begin_catch(i8* "intel_dtrans_func_index"="2")
declare !intel.dtrans.func.type !26 "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64)
declare !intel.dtrans.func.type !27 void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")
declare void @__cxa_end_catch()
declare  void @_ZSt9terminatev()
declare !intel.dtrans.func.type !28 void @_Unwind_Resume(i8* "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !30 void @_ZNSt9bad_allocD1Ev(%class.bad_alloc* "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !31 "intel_dtrans_func_index"="1" i8* @_ZNKSt9bad_alloc4whatEv(%class.bad_alloc* "intel_dtrans_func_index"="2")
declare !intel.dtrans.func.type !32 void @_ZNSt9bad_allocD0Ev(%class.bad_alloc* "intel_dtrans_func_index"="1")

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
