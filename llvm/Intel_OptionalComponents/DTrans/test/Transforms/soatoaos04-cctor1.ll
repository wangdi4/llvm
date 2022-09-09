; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,require<soatoaos-approx>,function(require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-malloc-functions="MemoryManagerImpl::allocate(unsigned long)"                         \
; RUN:        -dtrans-malloc-functions="XalanDummyMemoryManager::allocate(unsigned long)"                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                    \
; RUN:        -passes=soatoaos-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -dtrans-malloc-functions="MemoryManagerImpl::allocate(unsigned long)"                         \
; RUN:        -dtrans-malloc-functions="XalanDummyMemoryManager::allocate(unsigned long)"                   \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test checks allocation functions handling after devirtualization.
; The test is similar to soatoaos04-cctor.ll.
; 2 versions of allocation functions are processed, results of functions are merged with phi.
; Comparisons of pointers to functions from vtable of MemoryManager is a known side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.MemoryManager* }
%class.IC_Field = type opaque
%class.MemoryManager = type { i32 (...)** }
%class.MemoryManagerImpl = type { %class.MemoryManager }
%class.XalanDummyMemoryManager = type { %class.MemoryManager }

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)

; CHECK:      ; Classification: CCtor method
; CHECK-NEXT: ; Dump instructions needing update. Total = 11
define hidden void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf* nocapture %arg, %class.ValueVectorOf* nocapture readonly dereferenceable(32) %arg1) {
bb:
  %tmp = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 0
  %tmp2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 0
  %tmp3 = load i8, i8* %tmp2
  store i8 %tmp3, i8* %tmp
  %tmp4 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 1
  %tmp5 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 1
  %tmp6 = load i32, i32* %tmp5
  store i32 %tmp6, i32* %tmp4
  %tmp7 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 2
  %tmp8 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 2
  %tmp9 = load i32, i32* %tmp8
  store i32 %tmp9, i32* %tmp7
  %tmp10 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 3
; CHECK:      ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:   store %class.IC_Field** null, %class.IC_Field*** %tmp10
  store %class.IC_Field** null, %class.IC_Field*** %tmp10
  %tmp11 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 4
  %tmp12 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 4
  %tmp13 = load %class.MemoryManager*, %class.MemoryManager** %tmp12
  store %class.MemoryManager* %tmp13, %class.MemoryManager** %tmp11
  %tmp14 = zext i32 %tmp9 to i64
  %tmp15 = shl nuw nsw i64 %tmp14, 3
  %tmp16 = bitcast %class.MemoryManager* %tmp13 to i8* (i64)***
  %tmp17 = load i8* (i64)**, i8* (i64)*** %tmp16
  %tmp18 = bitcast i8* (i64)** %tmp17 to i8*
  %tmp19 = getelementptr inbounds i8* (i64)*, i8* (i64)** %tmp17, i64 2
  %tmp20 = load i8* (i64)*, i8* (i64)** %tmp19
  %tmp21 = bitcast i8* (i64)* %tmp20 to i8*
  %tmp22 = bitcast i8* (i64)* @"MemoryManagerImpl::allocate(unsigned long)" to i8*
  %tmp23 = icmp eq i8* %tmp21, %tmp22
; Known side-effect.
  br i1 %tmp23, label %bb24, label %bb26

bb24:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp25 = tail call i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64 %tmp15)
; CHECK-MOD:        %nsz = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT:   %tmp25 = tail call i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64 %nsz)
  %tmp25 = tail call i8* bitcast (i8* (i64)* @"MemoryManagerImpl::allocate(unsigned long)" to i8* (i64)*)(i64 %tmp15)
  br label %bb28

bb26:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp27 = tail call i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64 %tmp15)
; CHECK-MOD:      %nsz1 = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT: %tmp27 = tail call i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64 %nsz1)
  %tmp27 = tail call i8* bitcast (i8* (i64)* @"XalanDummyMemoryManager::allocate(unsigned long)" to i8* (i64)*)(i64 %tmp15)
  br label %bb28

bb28:                                             ; preds = %bb26, %bb24
  %tmp29 = phi i8* [ %tmp25, %bb24 ], [ %tmp27, %bb26 ]
  br label %bb30

bb30:                                             ; preds = %bb28
  %tmp31 = bitcast i8* %tmp29 to %class.IC_Field**
; CHECK:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:   store %class.IC_Field** %tmp31, %class.IC_Field*** %tmp10
  store %class.IC_Field** %tmp31, %class.IC_Field*** %tmp10
  %tmp32 = load i32, i32* %tmp7
  %tmp33 = zext i32 %tmp32 to i64
  %tmp34 = shl nuw nsw i64 %tmp33, 3
; CHECK:      ; MemInst: Memset of elements
; CHECK-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
  %tmp35 = load i32, i32* %tmp4
  %tmp36 = icmp eq i32 %tmp35, 0
  br i1 %tmp36, label %bb40, label %bb37

bb37:                                             ; preds = %bb30
  %tmp38 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 3
  %tmp39 = zext i32 %tmp35 to i64
  br label %bb41

bb40:                                             ; preds = %bb41, %bb30
  ret void

bb41:                                             ; preds = %bb41, %bb37
  %tmp42 = phi i64 [ 0, %bb37 ], [ %tmp50, %bb41 ]
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp43 = load %class.IC_Field**, %class.IC_Field*** %tmp38
  %tmp43 = load %class.IC_Field**, %class.IC_Field*** %tmp38
; CHECK:      ; MemInstGEP: Element load
; CHECK-NEXT:   %tmp44 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp43, i64 %tmp42
  %tmp44 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp43, i64 %tmp42
  %tmp45 = bitcast %class.IC_Field** %tmp44 to i64*
; CHECK:      ; MemInst: Element load
; CHECK-NEXT:   %tmp46 = load i64, i64* %tmp45
  %tmp46 = load i64, i64* %tmp45
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp47 = load %class.IC_Field**, %class.IC_Field*** %tmp10
  %tmp47 = load %class.IC_Field**, %class.IC_Field*** %tmp10
; CHECK:      ; MemInstGEP: Element copy
; CHECK-NEXT:   %tmp48 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp47, i64 %tmp42
  %tmp48 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp47, i64 %tmp42
  %tmp49 = bitcast %class.IC_Field** %tmp48 to i64*
; CHECK:      ; MemInst: Element copy
; CHECK-NEXT:   store i64 %tmp46, i64* %tmp49
  store i64 %tmp46, i64* %tmp49
  %tmp50 = add nuw nsw i64 %tmp42, 1
  %tmp51 = icmp ult i64 %tmp50, %tmp39
  br i1 %tmp51, label %bb41, label %bb40
}


declare hidden noalias nonnull i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64)
declare hidden noalias nonnull i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64)
