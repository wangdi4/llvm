; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                           \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                       \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                      \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                           \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                       \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                      \
; RUN:        | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

; Test checks allocation functions handling after devirtualization.
; The test is similar to soatoaosop04-cctor.ll.
; 2 versions of allocation functions are processed, results of functions are merged with phi.
; Comparisons of pointers to functions from vtable of MemoryManager is a known side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.MemoryManager* }
%class.IC_Field = type opaque
%class.MemoryManager = type { i32 (...)** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)

; CHECK:      ; Classification: CCtor method
; CHECK-NEXT: ; Dump instructions needing update. Total = 11
; CHECK-OP:      ; Classification: CCtor method
; CHECK-OP-NEXT: ; Dump instructions needing update. Total = 11
define hidden void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf* nocapture "intel_dtrans_func_index"="1" %arg, %class.ValueVectorOf* nocapture readonly dereferenceable(32)  "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !15 {
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
; CHECK-OP:      ; BasePtrInst: Nullify base pointer
; CHECK-OP-NEXT:   store ptr null, ptr %tmp10
  store %class.IC_Field** null, %class.IC_Field*** %tmp10
  %tmp11 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 4
  %tmp12 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg1, i64 0, i32 4
  %tmp13 = load %class.MemoryManager*, %class.MemoryManager** %tmp12
  store %class.MemoryManager* %tmp13, %class.MemoryManager** %tmp11
  %tmp14 = zext i32 %tmp9 to i64
  %tmp15 = shl nuw nsw i64 %tmp14, 3
  %tmp16 = bitcast %class.MemoryManager* %tmp13 to i8* (%class.MemoryManager*, i64)***
  %tmp17 = load i8* (%class.MemoryManager*, i64)**, i8* (%class.MemoryManager*, i64)*** %tmp16
  %tmp18 = bitcast i8* (%class.MemoryManager*, i64)** %tmp17 to i8*
  %tmp19 = getelementptr inbounds i8* (%class.MemoryManager*, i64)*, i8* (%class.MemoryManager*, i64)** %tmp17, i64 2
  %tmp20 = load i8* (%class.MemoryManager*, i64)*, i8* (%class.MemoryManager*, i64)** %tmp19
  %tmp21 = bitcast i8* (%class.MemoryManager*, i64)* %tmp20 to i8*
  %tmp22 = bitcast i8* (%class.MemoryManager*, i64)* @"_ZN10MemManager8allocateEl" to i8*
  %tmp23 = icmp eq i8* %tmp21, %tmp22
; Known side-effect.
  br i1 %tmp23, label %bb24, label %bb26

bb24:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp25 = tail call i8* @_ZN10MemManager8allocateEl(%class.MemoryManager* %tmp13, i64 %tmp15)
; CHECK-MOD:        %nsz = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT:   %tmp25 = tail call i8* @_ZN10MemManager8allocateEl(%class.MemoryManager* %tmp13, i64 %nsz)
; CHECK-OP:      ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:   %tmp25 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp13, i64 %tmp15)
; CHECK-OP-MOD:        %nsz = mul nuw i64 %tmp15, 2
; CHECK-OP-MOD-NEXT:   %tmp25 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp13, i64 %nsz)
  %tmp25 = tail call i8* @_ZN10MemManager8allocateEl(%class.MemoryManager* %tmp13, i64 %tmp15)
  br label %bb28

bb26:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp27 = tail call i8* @dummyAlloc(%class.MemoryManager* %tmp13, i64 %tmp15)
; CHECK-MOD:      %nsz1 = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT: %tmp27 = tail call i8* @dummyAlloc(%class.MemoryManager* %tmp13, i64 %nsz1)
; CHECK-OP:      ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:   %tmp27 = tail call ptr @dummyAlloc(ptr %tmp13, i64 %tmp15)
; CHECK-OP-MOD:      %nsz1 = mul nuw i64 %tmp15, 2
; CHECK-OP-MOD-NEXT: %tmp27 = tail call ptr @dummyAlloc(ptr %tmp13, i64 %nsz1)
  %tmp27 = tail call i8* @dummyAlloc(%class.MemoryManager* %tmp13, i64 %tmp15)
  br label %bb28

bb28:                                             ; preds = %bb26, %bb24
  %tmp29 = phi i8* [ %tmp25, %bb24 ], [ %tmp27, %bb26 ]
  br label %bb30

bb30:                                             ; preds = %bb28
  %tmp31 = bitcast i8* %tmp29 to %class.IC_Field**
; CHECK:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:   store %class.IC_Field** %tmp31, %class.IC_Field*** %tmp10
; CHECK-OP:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-NEXT:   store ptr %tmp31, ptr %tmp10
  store %class.IC_Field** %tmp31, %class.IC_Field*** %tmp10
  %tmp32 = load i32, i32* %tmp7
  %tmp33 = zext i32 %tmp32 to i64
  %tmp34 = shl nuw nsw i64 %tmp33, 3
; CHECK:      ; MemInst: Memset of elements
; CHECK-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
; CHECK-OP:      ; MemInst: Memset of elements
; CHECK-OP-NEXT:   tail call void @llvm.memset.p0.i64(ptr align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
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
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp43 = load ptr, ptr %tmp38
  %tmp43 = load %class.IC_Field**, %class.IC_Field*** %tmp38
; CHECK:      ; MemInstGEP: Element load
; CHECK-NEXT:   %tmp44 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp43, i64 %tmp42
; CHECK-OP:      ; MemInstGEP: Element load
; CHECK-OP-NEXT:   %tmp44 = getelementptr inbounds ptr, ptr %tmp43, i64 %tmp42
  %tmp44 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp43, i64 %tmp42
  %tmp45 = bitcast %class.IC_Field** %tmp44 to i64*
; CHECK:      ; MemInst: Element load
; CHECK-NEXT:   %tmp46 = load i64, i64* %tmp45
; CHECK-OP:      ; MemInst: Element load
; CHECK-OP-NEXT:   %tmp46 = load i64, ptr %tmp45
  %tmp46 = load i64, i64* %tmp45
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp47 = load %class.IC_Field**, %class.IC_Field*** %tmp10
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp47 = load ptr, ptr %tmp10
  %tmp47 = load %class.IC_Field**, %class.IC_Field*** %tmp10
; CHECK:      ; MemInstGEP: Element copy
; CHECK-NEXT:   %tmp48 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp47, i64 %tmp42
; CHECK-OP:      ; MemInstGEP: Element copy
; CHECK-OP-NEXT:   %tmp48 = getelementptr inbounds ptr, ptr %tmp47, i64 %tmp42
  %tmp48 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp47, i64 %tmp42
  %tmp49 = bitcast %class.IC_Field** %tmp48 to i64*
; CHECK:      ; MemInst: Element copy
; CHECK-NEXT:   store i64 %tmp46, i64* %tmp49
; CHECK-OP:      ; MemInst: Element copy
; CHECK-OP-NEXT:   store i64 %tmp46, ptr %tmp49
  store i64 %tmp46, i64* %tmp49
  %tmp50 = add nuw nsw i64 %tmp42, 1
  %tmp51 = icmp ult i64 %tmp50, %tmp39
  br i1 %tmp51, label %bb41, label %bb40
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1"  i8* @_ZN10MemManager8allocateEl(%class.MemoryManager* "intel_dtrans_func_index"="2" nocapture readnone %this, i64 %size) align 2 !intel.dtrans.func.type !17 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

define internal "intel_dtrans_func_index"="1" i8* @dummyAlloc(%class.MemoryManager* "intel_dtrans_func_index"="2" %this, i64 %conv4) !intel.dtrans.func.type !18 {
entry:
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** null to i8*), i8* bitcast (void (%class.bad_alloc*)* null to i8*))
  unreachable
}

declare !intel.dtrans.func.type !19 "intel_dtrans_func_index"="1" i8* @malloc(i64) #1
declare !intel.dtrans.func.type !20 dso_local noalias nonnull "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64) local_unnamed_addr
declare !intel.dtrans.func.type !21 void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

attributes #0 = { argmemonly nofree nounwind willreturn writeonly }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!intel.dtrans.types = !{!3, !4, !8, !12, !14}

!1 = !{!"A", i32 5, !2}
!2 = !{i8 0, i32 1}
!3 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!4 = !{!"S", %class.MemoryManager zeroinitializer, i32 1, !5}
!5 = !{!6, i32 2}
!6 = !{!"F", i1 true, i32 0, !7}
!7 = !{i32 0, i32 0}
!8 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !9, !7, !7, !10, !11}
!9 = !{i8 0, i32 0}
!10 = !{%class.IC_Field zeroinitializer, i32 2}
!11 = !{%class.MemoryManager zeroinitializer, i32 1}
!12 = !{!"S", %class.bad_alloc zeroinitializer, i32 1, !13}
!13 = !{%class.exception zeroinitializer, i32 0}
!14 = !{!"S", %class.exception zeroinitializer, i32 1, !5}
!15 = distinct !{!16, !16}
!16 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!17 = distinct !{!2, !11}
!18 = distinct !{!2, !11}
!19 = distinct !{!2}
!20 = distinct !{!2}
!21 = distinct !{!2, !2, !2}
