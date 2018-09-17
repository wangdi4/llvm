; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>)'            \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -disable-output                                           \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                              \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-approx-known-func="FieldValueMap::cleanUp()"                   \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=dtor                               \
; RUN:          -dtrans-soatoaos-array-dtor="ValueVectorOf<IC_Field*>::~ValueVectorOf()"            \
; RUN:          -dtrans-soatoaos-array-dtor="ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"   \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
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
; CHECK-TRANS: ; IR: analysed completely

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 10

; Some code related to fValues is inlined.
; void FieldValueMap::cleanUp() {
;   delete fFields;
;   delete fValidators;
;   delete fValues;
; }

%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }

declare i32 @__gxx_personality_v0(...)

declare hidden void @__clang_call_terminate(i8*)

define hidden void @"FieldValueMap::cleanUp()"(%class.FieldValueMap* %this) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
  %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-TRANS:      ; ArrayInst: Null check
; CHECK-TRANS-NEXT:   %isnull = icmp eq %class.ValueVectorOf.0* %tmp, null
  %isnull = icmp eq %class.ValueVectorOf.0* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0* %tmp)
  invoke void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0* %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %delete.notnull
; CHECK-TRANS:      ; ArrayInst: cast to call deallocate
; CHECK-TRANS-NEXT:   %tmp1 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  %tmp1 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  tail call void @"XMemory::operator delete(void*)"(i8* %tmp1)
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont, %entry
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp2 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
  %tmp2 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-TRANS:      ; ArrayInst: Null check
; CHECK-TRANS-NEXT:   %isnull3 = icmp eq %class.ValueVectorOf.1* %tmp2, null
  %isnull3 = icmp eq %class.ValueVectorOf.1* %tmp2, null
  br i1 %isnull3, label %delete.end8, label %delete.notnull4

delete.notnull4:                                  ; preds = %delete.end
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1* %tmp2)
  invoke void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1* %tmp2)
          to label %invoke.cont6 unwind label %lpad5

invoke.cont6:                                     ; preds = %delete.notnull4
; CHECK-TRANS:      ; ArrayInst: cast to call deallocate
; CHECK-TRANS-NEXT:   %tmp3 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
  %tmp3 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
  tail call void @"XMemory::operator delete(void*)"(i8* %tmp3)
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
; CHECK-TRANS:      ; ArrayInst: cast to call deallocate
; CHECK-TRANS-NEXT:   %tmp10 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  %tmp10 = getelementptr inbounds %class.ValueVectorOf.0, %class.ValueVectorOf.0* %tmp, i64 0, i32 0
  invoke void @"XMemory::operator delete(void*)"(i8* %tmp10)
          to label %eh.resume unwind label %terminate.lpad

lpad5:                                            ; preds = %delete.notnull4
  %tmp11 = landingpad { i8*, i32 }
          cleanup
  %tmp12 = extractvalue { i8*, i32 } %tmp11, 0
  %tmp13 = extractvalue { i8*, i32 } %tmp11, 1
; CHECK-TRANS:      ; ArrayInst: cast to call deallocate
; CHECK-TRANS-NEXT:   %tmp14 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
  %tmp14 = getelementptr inbounds %class.ValueVectorOf.1, %class.ValueVectorOf.1* %tmp2, i64 0, i32 0
  invoke void @"XMemory::operator delete(void*)"(i8* %tmp14)
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

declare hidden void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf.0*)

declare hidden void @"ValueVectorOf<DatatypeValidator*>::~ValueVectorOf()"(%class.ValueVectorOf.1*)

declare hidden void @"XMemory::operator delete(void*)"(i8*)

; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; XCHECK-DEP: Deps computed: 25, Queries: 52
