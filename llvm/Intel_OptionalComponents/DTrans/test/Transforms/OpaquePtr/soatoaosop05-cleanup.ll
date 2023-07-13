; RUN: opt < %s -whole-program-assume -intel-libirc-allowed                        \
; RUN:          -disable-output -debug-only=dtrans-soatoaosop-deps                                  \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output        \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                              \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                        \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                                 \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                                 \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                   \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                 \
; RUN:          -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                      \
; RUN:          -dtrans-soatoaosop-array-dtor="ValueVectorOf<IC_Field*>::~ValueVectorOf()"          \
; RUN:          -dtrans-soatoaosop-array-dtor="ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()" \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                     \
; RUN:          -passes=soatoaosop-struct-methods-transform                                         \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                        \
; RUN:          -dtransop-optbase-process-function-declaration                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                                 \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                                 \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                   \
; RUN:          -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                      \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                 \
; RUN:          -dtrans-soatoaosop-array-dtor="ValueVectorOf<IC_Field*>::~ValueVectorOf()"          \
; RUN:          -dtrans-soatoaosop-array-dtor="ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()" \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::cleanUp()
; CHECK-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 4

; Also checks that call sites to dtor can be combined.

; Checks transformation. Types change and combined methods removed.

; Some code related to fValues is inlined.
; void FieldValueMap::cleanUp() {
;   delete fFields;
;   delete fValidators;
;   delete fValues;
; }

; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-MOD-NOT: ValueVectorOf.1
%class.FieldValueMap = type { ptr, ptr, ptr, ptr }
%class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
%class.ValueVectorOf.1 = type { i8, i32, i32, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { ptr, i8, i32, i32, ptr, ptr }

declare i32 @__gxx_personality_v0(...)

declare hidden void @__clang_call_terminate(ptr)

define hidden void @"FieldValueMap::cleanUp()"(ptr "intel_dtrans_func_index"="1" %this) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !20 {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT: %tmp = load ptr, ptr %fFields
; CHECK-MOD:       %tmp = load ptr, ptr %fFields
  %tmp = load ptr, ptr %fFields, align 8
; CHECK-MOD-NEXT:  %isnull = icmp eq ptr %tmp, null
  %isnull = icmp eq ptr %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(ptr %tmp)
; CHECK-MOD:       invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf(){{.*}}"(ptr %tmp)
  invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(ptr %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %delete.notnull
; CHECK-MOD: %tmp1 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  %tmp1 = getelementptr inbounds %class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  tail call void @free(ptr %tmp1)
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont, %entry
; CHECK-MOD:       %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 0
  %fValidators = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT: %tmp2 = load ptr, ptr %fValidators
; CHECK-MOD-NEXT:  %tmp2 = load ptr, ptr %fValidators
  %tmp2 = load ptr, ptr %fValidators, align 8
; CHECK-MOD-NEXT:  %isnull3 = icmp eq ptr %tmp2, null
  %isnull3 = icmp eq ptr %tmp2, null
  br i1 %isnull3, label %delete.end8, label %delete.notnull4

delete.notnull4:                                  ; preds = %delete.end
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT: invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(ptr %tmp2)
; CHECK-MOD:          br label %invoke.cont6
  invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(ptr %tmp2)
          to label %invoke.cont6 unwind label %lpad5

; CHECK-MOD:        invoke.cont6:
invoke.cont6:                                     ; preds = %delete.notnull4
  %tmp3 = getelementptr inbounds %class.ValueVectorOf.1, ptr %tmp2, i64 0, i32 0
  tail call void @free(ptr %tmp3)
; %tmp3 and call are deleted.
; CHECK-MOD-NEXT:     br label %delete.end8
  br label %delete.end8

delete.end8:                                      ; preds = %invoke.cont6, %delete.end
  %fValues = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 2
  %tmp4 = load ptr, ptr %fValues, align 8
  %isnull9 = icmp eq ptr %tmp4, null
  br i1 %isnull9, label %delete.end11, label %delete.notnull10

delete.notnull10:                                 ; preds = %delete.end8
  %tmp5 = bitcast ptr %tmp4 to ptr
  %vtable = load ptr, ptr %tmp5, align 8
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 1
  %tmp6 = load ptr, ptr %vfn, align 8
  tail call void %tmp6(ptr %tmp4)
  br label %delete.end11

delete.end11:                                     ; preds = %delete.notnull10, %delete.end8
  ret void

lpad:                                             ; preds = %delete.notnull
  %tmp7 = landingpad { ptr, i32 }
          cleanup
  %tmp8 = extractvalue { ptr, i32 } %tmp7, 0
  %tmp9 = extractvalue { ptr, i32 } %tmp7, 1
; CHECK-MOD:    %tmp10 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  %tmp10 = getelementptr inbounds %class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  invoke void @_ZN10MemManager10deallocateEPv(ptr null, ptr %tmp10)
          to label %eh.resume unwind label %terminate.lpad

lpad5:                                            ; preds = %delete.notnull4
  %tmp11 = landingpad { ptr, i32 }
          cleanup
  %tmp12 = extractvalue { ptr, i32 } %tmp11, 0
; CHECK-MOD:     %tmp13 = extractvalue { ptr, i32 } %tmp11, 1
  %tmp13 = extractvalue { ptr, i32 } %tmp11, 1
  %tmp14 = getelementptr inbounds %class.ValueVectorOf.1, ptr %tmp2, i64 0, i32 0
; %tmp14 and invoke are deleted
; CHECK-MOD-NEXT:   br label %eh.resume
  invoke void @_ZN10MemManager10deallocateEPv(ptr null, ptr %tmp14)
          to label %eh.resume unwind label %terminate.lpad

eh.resume:                                        ; preds = %lpad5, %lpad
  %exn.slot.0 = phi ptr [ %tmp12, %lpad5 ], [ %tmp8, %lpad ]
  %ehselector.slot.0 = phi i32 [ %tmp13, %lpad5 ], [ %tmp9, %lpad ]
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn.slot.0, 0
  %lpad.val12 = insertvalue { ptr, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { ptr, i32 } %lpad.val12

terminate.lpad:                                   ; preds = %lpad5, %lpad
  %tmp15 = landingpad { ptr, i32 }
          catch ptr null
  %tmp16 = extractvalue { ptr, i32 } %tmp15, 0
  tail call void @__clang_call_terminate(ptr %tmp16)
  unreachable
}

declare !intel.dtrans.func.type !22 hidden void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(ptr "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !23 hidden void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(ptr "intel_dtrans_func_index"="1")

define dso_local void @_ZN10MemManager10deallocateEPv(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !24 {
entry:
  tail call void @free(ptr %p)
  ret void
}

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !26 hidden void @free(ptr "intel_dtrans_func_index"="1") #0

attributes #0 = { allockind("free") "alloc-family"="malloc" }

; CHECK-TRANS: ; Seen dtor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; XCHECK-DEP: Deps computed: 25, Queries: 52

!intel.dtrans.types = !{!0, !1, !2, !6, !10, !12, !14, !16}

!0 = !{!"S", %class.DatatypeValidator zeroinitializer, i32 -1}
!1 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!2 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !3}
!3 = !{!4, i32 2}
!4 = !{!"F", i1 true, i32 0, !5}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %class.BaseRefVectorOf zeroinitializer, i32 6, !3, !7, !5, !5, !8, !9}
!7 = !{i8 0, i32 0}
!8 = !{i16 0, i32 2}
!9 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!10 = !{!"S", %class.RefArrayVectorOf zeroinitializer, i32 1, !11}
!11 = !{%class.BaseRefVectorOf zeroinitializer, i32 0}
!12 = !{!"S", %class.ValueVectorOf.1 zeroinitializer, i32 5, !7, !5, !5, !13, !9}
!13 = !{%class.DatatypeValidator zeroinitializer, i32 2}
!14 = !{!"S", %class.ValueVectorOf.0 zeroinitializer, i32 5, !7, !5, !5, !15, !9}
!15 = !{%class.IC_Field zeroinitializer, i32 2}
!16 = !{!"S", %class.FieldValueMap zeroinitializer, i32 4, !17, !18, !19, !9}
!17 = !{%class.ValueVectorOf.0 zeroinitializer, i32 1}
!18 = !{%class.ValueVectorOf.1 zeroinitializer, i32 1}
!19 = !{%class.RefArrayVectorOf zeroinitializer, i32 1}
!20 = distinct !{!21}
!21 = !{%class.FieldValueMap zeroinitializer, i32 1}
!22 = distinct !{!17}
!23 = distinct !{!18}
!24 = distinct !{!9, !25}
!25 = !{i8 0, i32 1}
!26 = distinct !{!25}
