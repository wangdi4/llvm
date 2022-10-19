; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                               \
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
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                            \
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
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                              \
; RUN:          -disable-output -debug-only=dtrans-soatoaosop-deps                                  \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                              \
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
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                           \
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
; RUN:       | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; CHECK-OP-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP
; CHECK-OP-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::cleanUp()
; CHECK-TRANS: ; IR: has only expected side-effects
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::cleanUp()
; CHECK-OP-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 4
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 4

; Also checks that call sites to dtor can be combined.

; Checks transformation. Types change and combined methods removed.

; Some code related to fValues is inlined.
; void FieldValueMap::cleanUp() {
;   delete fFields;
;   delete fValidators;
;   delete fValues;
; }

%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }

declare i32 @__gxx_personality_v0(...)

declare hidden void @__clang_call_terminate(i8*)

define hidden void @"FieldValueMap::cleanUp()"(%class.FieldValueMap* "intel_dtrans_func_index"="1" %this) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !25 {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT: %tmp = load ptr, ptr %fFields
; CHECK-MOD:          %tmp = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields
; CHECK-OP-MOD:       %tmp = load ptr, ptr %fFields
  %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-MOD-NEXT:     %isnull = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp, null
; CHECK-OP-MOD-NEXT:  %isnull = icmp eq ptr %tmp, null
  %isnull = icmp eq %class.ValueVectorOf.0* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0* %tmp)
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(ptr %tmp)
; CHECK-MOD:          invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf(){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp)
; CHECK-OP-MOD:       invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf(){{.*}}"(ptr %tmp)
  invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0* %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %delete.notnull
; CHECK-MOD: %tmp1 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, %__SOA_AR_class.ValueVectorOf.0* %tmp, i64 0, i32 0
; CHECK-OP-MOD: %tmp1 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  %tmp1 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  tail call void @free(i8* %tmp1)
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont, %entry
; CHECK-MOD:          %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 0
; CHECK-OP-MOD:       %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 0
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp2 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT: %tmp2 = load ptr, ptr %fValidators
; CHECK-MOD-NEXT:     %tmp2 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fValidators
; CHECK-OP-MOD-NEXT:  %tmp2 = load ptr, ptr %fValidators
  %tmp2 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-MOD-NEXT:     %isnull3 = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp2, null
; CHECK-OP-MOD-NEXT:  %isnull3 = icmp eq ptr %tmp2, null
  %isnull3 = icmp eq %class.ValueVectorOf.1* %tmp2, null
  br i1 %isnull3, label %delete.end8, label %delete.notnull4

delete.notnull4:                                  ; preds = %delete.end
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1* %tmp2)
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT: invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(ptr %tmp2)
; CHECK-MOD:          br label %invoke.cont6
; CHECK-OP-MOD:          br label %invoke.cont6
  invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1* %tmp2)
          to label %invoke.cont6 unwind label %lpad5

; CHECK-MOD:        invoke.cont6:
; CHECK-OP-MOD:        invoke.cont6:
invoke.cont6:                                     ; preds = %delete.notnull4
  %tmp3 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
  tail call void @free(i8* %tmp3)
; %tmp3 and call are deleted.
; CHECK-MOD-NEXT:     br label %delete.end8
; CHECK-OP-MOD-NEXT:     br label %delete.end8
  br label %delete.end8

delete.end8:                                      ; preds = %invoke.cont6, %delete.end
  %fValues = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp4 = load %class.RefArrayVectorOf*, %class.RefArrayVectorOf** %fValues
  %isnull9 = icmp eq %class.RefArrayVectorOf* %tmp4, null
  br i1 %isnull9, label %delete.end11, label %delete.notnull10

delete.notnull10:                                 ; preds = %delete.end8
  %tmp5 = bitcast %class.RefArrayVectorOf* %tmp4 to void (%class.RefArrayVectorOf*)***
  %vtable = load void (%class.RefArrayVectorOf*)**, void (%class.RefArrayVectorOf*)*** %tmp5
  %vfn = getelementptr inbounds void (%class.RefArrayVectorOf*)*, void (%class.RefArrayVectorOf*)** %vtable, i64 1
  %tmp6 = load void (%class.RefArrayVectorOf*)*, void (%class.RefArrayVectorOf*)** %vfn
  tail call void %tmp6(%class.RefArrayVectorOf* %tmp4)
  br label %delete.end11

delete.end11:                                     ; preds = %delete.notnull10, %delete.end8
  ret void

lpad:                                             ; preds = %delete.notnull
  %tmp7 = landingpad { i8*, i32 }
          cleanup
  %tmp8 = extractvalue { i8*, i32 } %tmp7, 0
  %tmp9 = extractvalue { i8*, i32 } %tmp7, 1
; CHECK-MOD:    %tmp10 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, %__SOA_AR_class.ValueVectorOf.0* %tmp, i64 0, i32 0
; CHECK-OP-MOD:    %tmp10 = getelementptr inbounds %__SOA_AR_class.ValueVectorOf.0, ptr %tmp, i64 0, i32 0
  %tmp10 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %tmp10)
          to label %eh.resume unwind label %terminate.lpad

lpad5:                                            ; preds = %delete.notnull4
  %tmp11 = landingpad { i8*, i32 }
          cleanup
  %tmp12 = extractvalue { i8*, i32 } %tmp11, 0
; CHECK-MOD:        %tmp13 = extractvalue { i8*, i32 } %tmp11, 1
; CHECK-OP-MOD:     %tmp13 = extractvalue { ptr, i32 } %tmp11, 1
  %tmp13 = extractvalue { i8*, i32 } %tmp11, 1
  %tmp14 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
; %tmp14 and invoke are deleted
; CHECK-MOD-NEXT:   br label %eh.resume
; CHECK-OP-MOD-NEXT:   br label %eh.resume
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %tmp14)
          to label %eh.resume unwind label %terminate.lpad

eh.resume:                                        ; preds = %lpad5, %lpad
  %exn.slot.0 = phi i8* [ %tmp12, %lpad5 ], [ %tmp8, %lpad ]
  %ehselector.slot.0 = phi i32 [ %tmp13, %lpad5 ], [ %tmp9, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val12 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val12

terminate.lpad:                                   ; preds = %lpad5, %lpad
  %tmp15 = landingpad { i8*, i32 }
          catch i8* null
  %tmp16 = extractvalue { i8*, i32 } %tmp15, 0
  tail call void @__clang_call_terminate(i8* %tmp16)
  unreachable
}

declare !intel.dtrans.func.type !26 hidden void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0* "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !27 hidden void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1* "intel_dtrans_func_index"="1")

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !28 {
entry:
  tail call void @free(i8* %p)
  ret void
}


declare !intel.dtrans.func.type !30 hidden void @free(i8* "intel_dtrans_func_index"="1") #0
attributes #0 = { allockind("free") "alloc-family"="malloc" }

; CHECK-TRANS: ; Seen dtor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; CHECK-OP-TRANS: ; Seen dtor.
; CHECK-OP-TRANS: ; Array call sites analysis result: required call sites can be merged
; XCHECK-DEP: Deps computed: 25, Queries: 52

!intel.dtrans.types = !{ !1, !2, !3, !7, !14, !16, !18, !20}

!1 = !{!"S", %"class.DatatypeValidator" zeroinitializer, i32 -1}
!2 = !{!"S", %"class.IC_Field" zeroinitializer, i32 -1}
!3 = !{!"S", %"class.XMLMsgLoader" zeroinitializer, i32 1, !4}
!4 = !{!5, i32 2}
!5 = !{!"F", i1 true, i32 0, !6}
!6 = !{i32 0, i32 0}
!7 = !{!"S", %"class.BaseRefVectorOf" zeroinitializer, i32 6, !9, !12, !11, !11, !8, !13}
!8 = !{i16 0, i32 2}
!9 = !{!10, i32 2}
!10 = !{!"F", i1 true, i32 0, !11}
!11 = !{i32 0, i32 0}
!12 = !{i8 0, i32 0}
!13 = !{%"class.XMLMsgLoader" zeroinitializer, i32 1}
!14 = !{!"S", %"class.RefArrayVectorOf" zeroinitializer, i32 1, !15}
!15 = !{%"class.BaseRefVectorOf" zeroinitializer, i32 0}
!16 = !{!"S", %"class.ValueVectorOf.1" zeroinitializer, i32 5, !12, !11, !11, !17, !13}
!17 = !{%"class.DatatypeValidator" zeroinitializer, i32 2}
!18 = !{!"S", %"class.ValueVectorOf.0" zeroinitializer, i32 5, !12, !11, !11, !19, !13}
!19 = !{%"class.IC_Field" zeroinitializer, i32 2}
!20 = !{!"S", %"class.FieldValueMap" zeroinitializer, i32 4, !21, !22, !23, !13}
!21 = !{%"class.ValueVectorOf.0" zeroinitializer, i32 1}
!22 = !{%"class.ValueVectorOf.1" zeroinitializer, i32 1}
!23 = !{%"class.RefArrayVectorOf" zeroinitializer, i32 1}
!24 = !{%"class.FieldValueMap" zeroinitializer, i32 1}
!25 = distinct !{!24}
!26 = distinct !{!21}
!27 = distinct !{!22}
!28 = distinct !{!13, !29}
!29 = !{i8 0, i32 1}
!30 = distinct !{!29}
