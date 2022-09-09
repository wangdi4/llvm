; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,require<soatoaos-approx>,function(require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-malloc-functions="MemoryManagerImpl::allocate(unsigned long)"                         \
; RUN:        -dtrans-malloc-functions="XalanDummyMemoryManager::allocate(unsigned long)"                   \
; RUN:        -dtrans-free-functions="MemoryManagerImpl::deallocate(void*)"                                 \
; RUN:        -dtrans-free-functions="XalanDummyMemoryManager::deallocate(void*)"                           \
; RUN:        2>&1 | FileCheck %s
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test checks allocation/deallocation functions handling after devirtualization.
; The test is similar to soatoaos04-realloc.ll.
; 2 versions of allocation functions are processed, results of functions are merged with phi.
; 2 versions of deallocation functions are processed, although no processing of deallocation is needed.
; Comparisons of pointers to functions from vtable of MemoryManager is a known side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.MemoryManager* }
%class.IC_Field = type opaque
%class.MemoryManager = type { i32 (...)** }

declare hidden noalias nonnull i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64) align 2

declare hidden void @"MemoryManagerImpl::deallocate(void*)"(i8*) align 2

; Classification: Realloc method
; Dump instructions needing update. Total = 9
define hidden void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%class.ValueVectorOf* nocapture %arg, i32 %arg1) align 2 {
bb:
  %tmp = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 1
  %tmp2 = load i32, i32* %tmp
  %tmp3 = add i32 %tmp2, 1
  %tmp4 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 2
  %tmp5 = load i32, i32* %tmp4
  %tmp6 = icmp ugt i32 %tmp3, %tmp5
  br i1 %tmp6, label %bb7, label %bb64

bb7:                                              ; preds = %bb
  %tmp8 = uitofp i32 %tmp2 to double
  %tmp9 = fmul fast double %tmp8, 1.250000e+00
  %tmp10 = fptoui double %tmp9 to i32
  %tmp11 = icmp ult i32 %tmp3, %tmp10
  %tmp12 = select i1 %tmp11, i32 %tmp10, i32 %tmp3
  %tmp13 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 4
  %tmp14 = load %class.MemoryManager*, %class.MemoryManager** %tmp13
  %tmp15 = zext i32 %tmp12 to i64
  %tmp16 = shl nuw nsw i64 %tmp15, 3
  %tmp17 = bitcast %class.MemoryManager* %tmp14 to i8* (%class.MemoryManager*, i64)***
  %tmp18 = load i8* (%class.MemoryManager*, i64)**, i8* (%class.MemoryManager*, i64)*** %tmp17
  %tmp19 = bitcast i8* (%class.MemoryManager*, i64)** %tmp18 to i8*
  %tmp20 = getelementptr inbounds i8* (%class.MemoryManager*, i64)*, i8* (%class.MemoryManager*, i64)** %tmp18, i64 2
  %tmp21 = load i8* (%class.MemoryManager*, i64)*, i8* (%class.MemoryManager*, i64)** %tmp20
  %tmp22 = bitcast i8* (%class.MemoryManager*, i64)* %tmp21 to i8*
  %tmp23 = bitcast i8* (i64)* @"MemoryManagerImpl::allocate(unsigned long)" to i8*
  %tmp24 = icmp eq i8* %tmp22, %tmp23
; Known side-effect.
  br i1 %tmp24, label %bb25, label %bb27

bb25:                                             ; preds = %bb7
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp26 = tail call i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64 %tmp16)
  %tmp26 = tail call i8* @"MemoryManagerImpl::allocate(unsigned long)"(i64 %tmp16)
  br label %bb29

bb27:                                             ; preds = %bb7
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp28 = tail call i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64 %tmp16)
  %tmp28 = tail call i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64 %tmp16)
  br label %bb29

bb29:                                             ; preds = %bb27, %bb25
  %tmp30 = phi i8* [ %tmp26, %bb25 ], [ %tmp28, %bb27 ]
  br label %bb31

bb31:                                             ; preds = %bb29
  %tmp32 = bitcast i8* %tmp30 to %class.IC_Field**
  %tmp33 = load i32, i32* %tmp
  %tmp34 = icmp eq i32 %tmp33, 0
  %tmp35 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 3
  br i1 %tmp34, label %bb38, label %bb36

bb36:                                             ; preds = %bb31
  %tmp37 = zext i32 %tmp33 to i64
  br label %bb54

bb38:                                             ; preds = %bb54, %bb31
  %tmp39 = load %class.MemoryManager*, %class.MemoryManager** %tmp13
  %tmp40 = bitcast %class.IC_Field*** %tmp35 to i8**
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp41 = load i8*, i8** %tmp40
  %tmp41 = load i8*, i8** %tmp40
  %tmp42 = bitcast %class.MemoryManager* %tmp39 to void (%class.MemoryManager*, i8*)***
  %tmp43 = load void (%class.MemoryManager*, i8*)**, void (%class.MemoryManager*, i8*)*** %tmp42
  %tmp44 = bitcast void (%class.MemoryManager*, i8*)** %tmp43 to i8*
  %tmp45 = getelementptr inbounds void (%class.MemoryManager*, i8*)*, void (%class.MemoryManager*, i8*)** %tmp43, i64 3
  %tmp46 = load void (%class.MemoryManager*, i8*)*, void (%class.MemoryManager*, i8*)** %tmp45
  %tmp47 = bitcast void (%class.MemoryManager*, i8*)* %tmp46 to i8*
  %tmp48 = bitcast void (i8*)* @"MemoryManagerImpl::deallocate(void*)" to i8*
  %tmp49 = icmp eq i8* %tmp47, %tmp48
; Known side-effect.
  br i1 %tmp49, label %bb50, label %bb51

bb50:                                             ; preds = %bb38
  tail call void @"MemoryManagerImpl::deallocate(void*)"(i8* %tmp41)
  br label %bb52

bb51:                                             ; preds = %bb38
  tail call void @"XalanDummyMemoryManager::deallocate(void*)"(i8* %tmp41)
  br label %bb52

bb52:                                             ; preds = %bb51, %bb50
  br label %bb53

bb53:                                             ; preds = %bb52
; CHECK:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:   store %class.IC_Field** %tmp32, %class.IC_Field*** %tmp35
  store %class.IC_Field** %tmp32, %class.IC_Field*** %tmp35
  store i32 %tmp12, i32* %tmp4
  br label %bb64

bb54:                                             ; preds = %bb54, %bb36
  %tmp55 = phi i64 [ 0, %bb36 ], [ %tmp62, %bb54 ]
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp56 = load %class.IC_Field**, %class.IC_Field*** %tmp35
  %tmp56 = load %class.IC_Field**, %class.IC_Field*** %tmp35
; CHECK:      ; MemInstGEP: Element load
; CHECK-NEXT:   %tmp57 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp56, i64 %tmp55
  %tmp57 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp56, i64 %tmp55
  %tmp58 = bitcast %class.IC_Field** %tmp57 to i64*
; CHECK:      ; MemInst: Element load
; CHECK-NEXT:   %tmp59 = load i64, i64* %tmp58
  %tmp59 = load i64, i64* %tmp58
; CHECK:      ; MemInstGEP: Element store to new mem
; CHECK-NEXT:   %tmp60 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp32, i64 %tmp55
  %tmp60 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp32, i64 %tmp55
  %tmp61 = bitcast %class.IC_Field** %tmp60 to i64*
; CHECK:      ; MemInst: Element store to new mem
; CHECK-NEXT:   store i64 %tmp59, i64* %tmp61
  store i64 %tmp59, i64* %tmp61
  %tmp62 = add nuw nsw i64 %tmp55, 1
  %tmp63 = icmp ult i64 %tmp62, %tmp37
  br i1 %tmp63, label %bb54, label %bb38

bb64:                                             ; preds = %bb53, %bb
  ret void
}

declare hidden noalias nonnull i8* @"XalanDummyMemoryManager::allocate(unsigned long)"(i64) align 2

declare hidden void @"XalanDummyMemoryManager::deallocate(void*)"(i8* nocapture readnone) align 2
