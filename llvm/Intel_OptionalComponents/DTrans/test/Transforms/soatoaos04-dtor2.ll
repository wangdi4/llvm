; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,require<soatoaos-approx>,function(require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-free-functions="MemoryManagerImpl::deallocate(void*)"                                 \
; RUN:        -dtrans-free-functions="XalanDummyMemoryManager::deallocate(void*)"                           \
; RUN:        2>&1 | FileCheck %s
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test checks deallocation functions handling after devirtualization.
; The test is similar to soatoaos04-dtor.ll.
; 2 versions of deallocation functions are processed, although no processing of
; deallocation is needed.
; Comparisons of pointers to functions from vtable of MemoryManager is a known
; side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.MemoryManager* }
%class.IC_Field = type opaque
%class.MemoryManager = type { i32 (...)** }
%class.MemoryManagerImpl = type { %class.MemoryManager }
%class.XalanDummyMemoryManager = type { %class.MemoryManager }

; CHECK:      ; Classification: Dtor method
; CHECK-NEXT: ; Dump instructions needing update. Total = 1
define hidden void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf* nocapture readonly %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %tmp = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 4
  %tmp1 = load %class.MemoryManager*, %class.MemoryManager** %tmp
  %tmp2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 3
  %tmp3 = bitcast %class.IC_Field*** %tmp2 to i8**
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp4 = load i8*, i8** %tmp3
  %tmp4 = load i8*, i8** %tmp3
  %tmp5 = bitcast %class.MemoryManager* %tmp1 to void (i8*)***
  %tmp6 = load void (i8*)**, void (i8*)*** %tmp5
  %tmp7 = bitcast void (i8*)** %tmp6 to i8*
  %tmp8 = getelementptr inbounds void (i8*)*, void (i8*)** %tmp6, i64 3
  %tmp9 = load void (i8*)*, void (i8*)** %tmp8
  %tmp10 = bitcast void (i8*)* %tmp9 to i8*
  %tmp11 = bitcast void (i8*)* @"MemoryManagerImpl::deallocate(void*)" to i8*
  %tmp12 = icmp eq i8* %tmp10, %tmp11
; Known side-effect.
  br i1 %tmp12, label %bb13, label %bb14

bb13:                                             ; preds = %bb
  invoke void bitcast (void (i8*)* @"MemoryManagerImpl::deallocate(void*)" to void (i8*)*)(i8* %tmp4)
            to label %bb15 unwind label %terminate

bb14:                                             ; preds = %bb
  invoke void bitcast (void (i8*)* @"XalanDummyMemoryManager::deallocate(void*)" to void (i8*)*)(i8* %tmp4)
            to label %bb15 unwind label %terminate

bb15:                                             ; preds = %bb14, %bb13
  br label %bb16

bb16:                                             ; preds = %bb15
  ret void

terminate:                                        ; preds = %bb14, %bb13
  %lp = landingpad { i8*, i32 }
          catch i8* null
  %ext = extractvalue { i8*, i32 } %lp, 0
  tail call void @__clang_call_terminate(i8* %ext)
  unreachable
}

declare hidden void @"MemoryManagerImpl::deallocate(void*)"(i8*)
declare hidden void @"XalanDummyMemoryManager::deallocate(void*)"(i8* nocapture readnone)
declare dso_local void @__clang_call_terminate(i8*) unnamed_addr
declare i32 @__gxx_personality_v0(...)

