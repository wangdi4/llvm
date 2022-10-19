; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyDeallocateEPv                                   \
; RUN:        2>&1 | FileCheck %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyDeallocateEPv                                   \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; REQUIRES: asserts

; Test checks allocation/deallocation functions handling after devirtualization.
; The test is similar to soatoaos04-realloc.ll.
; 2 versions of allocation functions are processed, results of functions are merged with phi.
; 2 versions of deallocation functions are processed, although no processing of deallocation is needed.
; Comparisons of pointers to functions from vtable of XMLMsgLoader is a known side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

; Classification: Realloc method
; Dump instructions needing update. Total = 9
define hidden void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%class.ValueVectorOf* nocapture "intel_dtrans_func_index"="1" %arg, i32 %arg1) align 2 !intel.dtrans.func.type !12 {
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
  %tmp14 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %tmp13
  %tmp15 = zext i32 %tmp12 to i64
  %tmp16 = shl nuw nsw i64 %tmp15, 3
  %tmp17 = bitcast %class.XMLMsgLoader* %tmp14 to i8* (%class.XMLMsgLoader*, i64)***
  %tmp18 = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp17
  %tmp19 = bitcast i8* (%class.XMLMsgLoader*, i64)** %tmp18 to i8*
  %tmp20 = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %tmp18, i64 2
  %tmp21 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %tmp20
  %tmp22 = bitcast i8* (%class.XMLMsgLoader*, i64)* %tmp21 to i8*
  %tmp23 = bitcast i8* (%class.XMLMsgLoader*, i64)* @_ZN10MemManager8allocateEl to i8*
  %tmp24 = icmp eq i8* %tmp22, %tmp23
; Known side-effect.
  br i1 %tmp24, label %bb25, label %bb27

bb25:                                             ; preds = %bb7
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp26 = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp14, i64 %tmp16)
; CHECK-OP:      ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:   %tmp26 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp14, i64 %tmp16)
  %tmp26 = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp14, i64 %tmp16)
  br label %bb29

bb27:                                             ; preds = %bb7
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp28 = tail call i8* @dummyAlloc(%class.XMLMsgLoader* %tmp14, i64 %tmp16)
; CHECK-OP:      ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:   %tmp28 = tail call ptr @dummyAlloc(ptr %tmp14, i64 %tmp16)
  %tmp28 = tail call i8* @dummyAlloc(%class.XMLMsgLoader* %tmp14, i64 %tmp16)
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
  %tmp39 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %tmp13
  %tmp40 = bitcast %class.IC_Field*** %tmp35 to i8**
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp41 = load i8*, i8** %tmp40
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp41 = load ptr, ptr %tmp40
  %tmp41 = load i8*, i8** %tmp40
  %tmp42 = bitcast %class.XMLMsgLoader* %tmp39 to void (%class.XMLMsgLoader*, i8*)***
  %tmp43 = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp42
  %tmp44 = bitcast void (%class.XMLMsgLoader*, i8*)** %tmp43 to i8*
  %tmp45 = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %tmp43, i64 3
  %tmp46 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %tmp45
  %tmp47 = bitcast void (%class.XMLMsgLoader*, i8*)* %tmp46 to i8*
  %tmp48 = bitcast void (%class.XMLMsgLoader*, i8*)* @_ZN10MemManager10deallocateEPv to i8*
  %tmp49 = icmp eq i8* %tmp47, %tmp48
; Known side-effect.
  br i1 %tmp49, label %bb50, label %bb51

bb50:                                             ; preds = %bb38
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* %tmp39, i8* %tmp41)
  br label %bb52

bb51:                                             ; preds = %bb38
  tail call void @dummyDeallocateEPv(%class.XMLMsgLoader* %tmp39, i8* %tmp41)
  br label %bb52

bb52:                                             ; preds = %bb51, %bb50
  br label %bb53

bb53:                                             ; preds = %bb52
; CHECK:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:   store %class.IC_Field** %tmp32, %class.IC_Field*** %tmp35
; CHECK-OP:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-NEXT:   store ptr %tmp32, ptr %tmp35
  store %class.IC_Field** %tmp32, %class.IC_Field*** %tmp35
  store i32 %tmp12, i32* %tmp4
  br label %bb64

bb54:                                             ; preds = %bb54, %bb36
  %tmp55 = phi i64 [ 0, %bb36 ], [ %tmp62, %bb54 ]
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp56 = load %class.IC_Field**, %class.IC_Field*** %tmp35
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp56 = load ptr, ptr %tmp35
  %tmp56 = load %class.IC_Field**, %class.IC_Field*** %tmp35
; CHECK:      ; MemInstGEP: Element load
; CHECK-NEXT:   %tmp57 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp56, i64 %tmp55
; CHECK-OP:      ; MemInstGEP: Element load
; CHECK-OP-NEXT:   %tmp57 = getelementptr inbounds ptr, ptr %tmp56, i64 %tmp55
  %tmp57 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp56, i64 %tmp55
  %tmp58 = bitcast %class.IC_Field** %tmp57 to i64*
; CHECK:      ; MemInst: Element load
; CHECK-NEXT:   %tmp59 = load i64, i64* %tmp58
; CHECK-OP:      ; MemInst: Element load
; CHECK-OP-NEXT:   %tmp59 = load i64, ptr %tmp58
  %tmp59 = load i64, i64* %tmp58
; CHECK:      ; MemInstGEP: Element store to new mem
; CHECK-NEXT:   %tmp60 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp32, i64 %tmp55
; CHECK-OP:      ; MemInstGEP: Element store to new mem
; CHECK-OP-NEXT:   %tmp60 = getelementptr inbounds ptr, ptr %tmp32, i64 %tmp55
  %tmp60 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp32, i64 %tmp55
  %tmp61 = bitcast %class.IC_Field** %tmp60 to i64*
; CHECK:      ; MemInst: Element store to new mem
; CHECK-NEXT:   store i64 %tmp59, i64* %tmp61
; CHECK-OP:      ; MemInst: Element store to new mem
; CHECK-OP-NEXT:   store i64 %tmp59, ptr %tmp61
  store i64 %tmp59, i64* %tmp61
  %tmp62 = add nuw nsw i64 %tmp55, 1
  %tmp63 = icmp ult i64 %tmp62, %tmp37
  br i1 %tmp63, label %bb54, label %bb38

bb64:                                             ; preds = %bb53, %bb
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !14 {
entry:
  tail call void @free(i8* %p)
  ret void
}

define void @dummyDeallocateEPv(%class.XMLMsgLoader* "intel_dtrans_func_index"="1" %0, i8* "intel_dtrans_func_index"="2" %1) !intel.dtrans.func.type !16 {
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** null, i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* null, i8* null)
  unreachable
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !17 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

define internal "intel_dtrans_func_index"="1" i8* @dummyAlloc(%class.XMLMsgLoader* "intel_dtrans_func_index"="2" %this, i64 %conv4) !intel.dtrans.func.type !18 {
entry:
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* null, i8* null)
  unreachable
}

declare !intel.dtrans.func.type !19 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !20 void @free(i8* "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !21 dso_local noalias nonnull "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64) local_unnamed_addr
declare !intel.dtrans.func.type !22 void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!0, !1, !5, !9, !11}

!0 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!1 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!9 = !{!"S", %class.bad_alloc zeroinitializer, i32 1, !10}
!10 = !{%class.exception zeroinitializer, i32 0}
!11 = !{!"S", %class.exception zeroinitializer, i32 1, !2}
!12 = distinct !{!13}
!13 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!14 = distinct !{!8, !15}
!15 = !{i8 0, i32 1}
!16 = distinct !{!8, !15}
!17 = distinct !{!15, !8}
!18 = distinct !{!15, !8}
!19 = distinct !{!15}
!20 = distinct !{!15}
!21 = distinct !{!15}
!22 = distinct !{!15, !15, !15}
