; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>'    \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                        \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                 \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                       \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyAlloc                                                        \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

; Test checks allocation functions handling after devirtualization.
; The test is similar to soatoaosop04-cctor.ll.
; 2 versions of allocation functions are processed, results of functions are merged with phi.
; Comparisons of pointers to functions from vtable of MemoryManager is a known side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
%class.bad_alloc = type { %class.exception }
%class.exception = type { ptr }
%class.IC_Field = type opaque
%class.MemoryManager = type { ptr }

; CHECK:      ; Classification: CCtor method
; CHECK-NEXT: ; Dump instructions needing update. Total = 11
define hidden void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(ptr nocapture "intel_dtrans_func_index"="1" %arg, ptr nocapture readonly dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !12 {
bb:
  %tmp = getelementptr inbounds %class.ValueVectorOf, ptr %arg, i64 0, i32 0
  %tmp2 = getelementptr inbounds %class.ValueVectorOf, ptr %arg1, i64 0, i32 0
  %tmp3 = load i8, ptr %tmp2, align 1
  store i8 %tmp3, ptr %tmp, align 1
  %tmp4 = getelementptr inbounds %class.ValueVectorOf, ptr %arg, i64 0, i32 1
  %tmp5 = getelementptr inbounds %class.ValueVectorOf, ptr %arg1, i64 0, i32 1
  %tmp6 = load i32, ptr %tmp5, align 4
  store i32 %tmp6, ptr %tmp4, align 4
  %tmp7 = getelementptr inbounds %class.ValueVectorOf, ptr %arg, i64 0, i32 2
  %tmp8 = getelementptr inbounds %class.ValueVectorOf, ptr %arg1, i64 0, i32 2
  %tmp9 = load i32, ptr %tmp8, align 4
  store i32 %tmp9, ptr %tmp7, align 4
  %tmp10 = getelementptr inbounds %class.ValueVectorOf, ptr %arg, i64 0, i32 3
; CHECK:      ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:   store ptr null, ptr %tmp10
  store ptr null, ptr %tmp10, align 8
  %tmp11 = getelementptr inbounds %class.ValueVectorOf, ptr %arg, i64 0, i32 4
  %tmp12 = getelementptr inbounds %class.ValueVectorOf, ptr %arg1, i64 0, i32 4
  %tmp13 = load ptr, ptr %tmp12, align 8
  store ptr %tmp13, ptr %tmp11, align 8
  %tmp14 = zext i32 %tmp9 to i64
  %tmp15 = shl nuw nsw i64 %tmp14, 3
  %tmp16 = bitcast ptr %tmp13 to ptr
  %tmp17 = load ptr, ptr %tmp16, align 8
  %tmp18 = bitcast ptr %tmp17 to ptr
  %tmp19 = getelementptr inbounds ptr, ptr %tmp17, i64 2
  %tmp20 = load ptr, ptr %tmp19, align 8
  %tmp21 = bitcast ptr %tmp20 to ptr
  %tmp22 = bitcast ptr @_ZN10MemManager8allocateEl to ptr
  %tmp23 = icmp eq ptr %tmp21, %tmp22
  br i1 %tmp23, label %bb24, label %bb26

bb24:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp25 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp13, i64 %tmp15)
; CHECK-MOD:        %nsz = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT:   %tmp25 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp13, i64 %nsz)
  %tmp25 = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp13, i64 %tmp15)
  br label %bb28

bb26:                                             ; preds = %bb
; CHECK:      ; BasePtrInst: Allocation call
; CHECK-NEXT:   %tmp27 = tail call ptr @dummyAlloc(ptr %tmp13, i64 %tmp15)
; CHECK-MOD:      %nsz1 = mul nuw i64 %tmp15, 2
; CHECK-MOD-NEXT: %tmp27 = tail call ptr @dummyAlloc(ptr %tmp13, i64 %nsz1)
  %tmp27 = tail call ptr @dummyAlloc(ptr %tmp13, i64 %tmp15)
  br label %bb28

bb28:                                             ; preds = %bb26, %bb24
  %tmp29 = phi ptr [ %tmp25, %bb24 ], [ %tmp27, %bb26 ]
  br label %bb30

bb30:                                             ; preds = %bb28
  %tmp31 = bitcast ptr %tmp29 to ptr
; CHECK:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:   store ptr %tmp31, ptr %tmp10
  store ptr %tmp31, ptr %tmp10, align 8
  %tmp32 = load i32, ptr %tmp7, align 4
  %tmp33 = zext i32 %tmp32 to i64
  %tmp34 = shl nuw nsw i64 %tmp33, 3
; CHECK:      ; MemInst: Memset of elements
; CHECK-NEXT:   tail call void @llvm.memset.p0.i64(ptr align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
  tail call void @llvm.memset.p0.i64(ptr align 8 %tmp29, i8 0, i64 %tmp34, i1 false)
  %tmp35 = load i32, ptr %tmp4, align 4
  %tmp36 = icmp eq i32 %tmp35, 0
  br i1 %tmp36, label %bb40, label %bb37

bb37:                                             ; preds = %bb30
  %tmp38 = getelementptr inbounds %class.ValueVectorOf, ptr %arg1, i64 0, i32 3
  %tmp39 = zext i32 %tmp35 to i64
  br label %bb41

bb40:                                             ; preds = %bb41, %bb30
  ret void

bb41:                                             ; preds = %bb41, %bb37
  %tmp42 = phi i64 [ 0, %bb37 ], [ %tmp50, %bb41 ]
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp43 = load ptr, ptr %tmp38
  %tmp43 = load ptr, ptr %tmp38, align 8
; CHECK:      ; MemInstGEP: Element load
; CHECK-NEXT:   %tmp44 = getelementptr inbounds ptr, ptr %tmp43, i64 %tmp42
  %tmp44 = getelementptr inbounds ptr, ptr %tmp43, i64 %tmp42
  %tmp45 = bitcast ptr %tmp44 to ptr
; CHECK:      ; MemInst: Element load
; CHECK-NEXT:   %tmp46 = load i64, ptr %tmp45
  %tmp46 = load i64, ptr %tmp45, align 8
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp47 = load ptr, ptr %tmp10
  %tmp47 = load ptr, ptr %tmp10, align 8
; CHECK:      ; MemInstGEP: Element copy
; CHECK-NEXT:   %tmp48 = getelementptr inbounds ptr, ptr %tmp47, i64 %tmp42
  %tmp48 = getelementptr inbounds ptr, ptr %tmp47, i64 %tmp42
  %tmp49 = bitcast ptr %tmp48 to ptr
; CHECK:      ; MemInst: Element copy
; CHECK-NEXT:   store i64 %tmp46, ptr %tmp49
  store i64 %tmp46, ptr %tmp49, align 8
  %tmp50 = add nuw nsw i64 %tmp42, 1
  %tmp51 = icmp ult i64 %tmp50, %tmp39
  br i1 %tmp51, label %bb41, label %bb40
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !14 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

define internal "intel_dtrans_func_index"="1" ptr @dummyAlloc(ptr "intel_dtrans_func_index"="2" %this, i64 %conv4) !intel.dtrans.func.type !16 {
entry:
  %call = tail call ptr @__cxa_allocate_exception(i64 8)
  %bc = bitcast ptr %call to ptr
  %gep = getelementptr inbounds %class.bad_alloc, ptr %bc, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr null, i64 0, inrange i32 0, i64 2), ptr %gep, align 8
  tail call void @__cxa_throw(ptr nonnull %call, ptr null, ptr null)
  unreachable
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !17 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

declare !intel.dtrans.func.type !18 dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64) local_unnamed_addr

declare !intel.dtrans.func.type !19 void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!intel.dtrans.types = !{!0, !1, !5, !9, !11}

!0 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!1 = !{!"S", %class.MemoryManager zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.MemoryManager zeroinitializer, i32 1}
!9 = !{!"S", %class.bad_alloc zeroinitializer, i32 1, !10}
!10 = !{%class.exception zeroinitializer, i32 0}
!11 = !{!"S", %class.exception zeroinitializer, i32 1, !2}
!12 = distinct !{!13, !13}
!13 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!14 = distinct !{!15, !8}
!15 = !{i8 0, i32 1}
!16 = distinct !{!15, !8}
!17 = distinct !{!15}
!18 = distinct !{!15}
!19 = distinct !{!15, !15, !15}
