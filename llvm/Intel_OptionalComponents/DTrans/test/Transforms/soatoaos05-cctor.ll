; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>)'            \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>)'            \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP-WF %s
; RUN: opt < %s -whole-program-assume -disable-output                                           \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                              \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=cctor                              \
; RUN:          -dtrans-soatoaos-array-cctor="ValueVectorOf<DatatypeValidator*>::ValueVectorOf(ValueVectorOf<DatatypeValidator*> const&)" \
; RUN:          -dtrans-soatoaos-array-cctor="ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"                   \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                        \
; RUN:          -passes=soatoaos-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=cctor                              \
; RUN:          -dtrans-soatoaos-array-cctor="ValueVectorOf<DatatypeValidator*>::ValueVectorOf(ValueVectorOf<DatatypeValidator*> const&)" \
; RUN:          -dtrans-soatoaos-array-cctor="ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"                   \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-WF-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap(FieldValueMap const&)
; CHECK-TRANS: ; IR: analysed completely

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 10

; Also checks that call sites to cctor can be combined.

; Checks transformation. Types change and combined methods removed.

; Some code not related to arrays is inlined.
; FieldValueMap::FieldValueMap(const FieldValueMap &other)
;     : XMemory(other), fFields(0), fValidators(0), fValues(0),
;       fMemoryManager(other.fMemoryManager) {
;   if (other.fFields) {
;     CleanupType cleanup(this, &FieldValueMap::cleanUp);
;     try {
;       unsigned int valuesSize = other.fValues->size();
;       fFields =
;           new (fMemoryManager) ValueVectorOf<IC_Field *>(*(other.fFields));
;       fValidators = new (fMemoryManager)
;           ValueVectorOf<DatatypeValidator *>(*(other.fValidators));
;       fValues = new (fMemoryManager) RefArrayVectorOf<XMLCh>(
;           other.fFields->curCapacity(), true, fMemoryManager);
;       for (unsigned int i = 0; i < valuesSize; i++) {
;         fValues->addElement(
;             XMLString::replicate(other.fValues->elementAt(i), fMemoryManager));
;       }
;     } catch (const OutOfMemoryException &) {
;       cleanup.release();
;       throw;
;     }
;     cleanup.release();
;   }
; }


%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }
%class.XMLException = type opaque

@"typeinfo for OutOfMemoryException" = external hidden constant { i8*, i8*, i8* }
@"typeinfo for ArrayIndexOutOfBoundsException" = external hidden constant { i8*, i8*, i8* }
@"vtable for ArrayIndexOutOfBoundsException" = external hidden constant { [6 x i8*] }
@"vtable for BaseRefVectorOf<unsigned short>" = external hidden constant { [9 x i8*] }
@"vtable for RefArrayVectorOf<unsigned short>" = external hidden constant { [9 x i8*] }
@.str.941 = external hidden constant [33 x i8]

declare i32 @__gxx_personality_v0(...)

declare i32 @llvm.eh.typeid.for(i8*)

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_rethrow()

declare void @__cxa_end_catch()

declare hidden void @__clang_call_terminate(i8*)

declare noalias i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_free_exception(i8*)

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

declare void @llvm.dbg.value(metadata, metadata, metadata)

; CHECK-MOD:       @"FieldValueMap::FieldValueMap(FieldValueMap const&){{.*}}"(%__SOA_class.FieldValueMap* %this, %__SOA_class.FieldValueMap* %other) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
define hidden void @"FieldValueMap::FieldValueMap(FieldValueMap const&)"(%class.FieldValueMap* %this, %class.FieldValueMap* %other) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
; CHECK-MOD:    call void @llvm.dbg.value(metadata %__SOA_class.FieldValueMap* %this
  call void @llvm.dbg.value(metadata %class.FieldValueMap* %this, metadata !13, metadata !DIExpression()), !dbg !14
; next getelementptr is removed
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-MOD-NEXT:   %fValues = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 2
  %fValues = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %fMemoryManager = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 3
  %fMemoryManager2 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %other, i64 0, i32 3
; CHECK-DEP:        %fMemoryManager2 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %other, i64 0, i32 3
; CHECK-DEP-NEXT: ; GEP(Arg 1)
; CHECK-DEP-NEXT: ;     3
; CHECK-DEP-NEXT:   %tmp = bitcast %class.XMLMsgLoader** %fMemoryManager2 to i64*
  %tmp = bitcast %class.XMLMsgLoader** %fMemoryManager2 to i64*
; Require special processing in 'soatoaos-struct-methods'.
; CHECK-DEP:      ; Func(Arg 0)
; CHECK-DEP-NEXT:   %tmp1 = bitcast %class.FieldValueMap* %this to i8*
; CHECK-MOD:        %tmp1 = bitcast %__SOA_class.FieldValueMap* %this to i8*
  %tmp1 = bitcast %class.FieldValueMap* %this to i8*
; CHECK-TRANS:      ; ArrayInst: Nullptr with memset of array
; CHECK-TRANS-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* %tmp1, i8 0, i64 24, i1 false)
; Layout of struct did not change.
; CHECK-MOD-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* %tmp1, i8 0, i64 24, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* %tmp1, i8 0, i64 24, i1 false)
  %tmp2 = load i64, i64* %tmp
  %tmp3 = bitcast %class.XMLMsgLoader** %fMemoryManager to i64*
; CHECK-DEP:      ; Store(Load(GEP(Arg 1)
; CHECK-DEP-NEXT: ;                3))
; CHECK-DEP-NEXT: ;      (GEP(Arg 0)
; CHECK-DEP-NEXT: ;           3)
; CHECK-DEP-NEXT:   store i64 %tmp2, i64* %tmp3
  store i64 %tmp2, i64* %tmp3
  %fFields3 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %other, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp4 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
; CHECK-MOD:          %tmp4 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields3
  %tmp4 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
; CHECK-MOD-NEXT:     %tobool = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp4, null
  %tobool = icmp eq %class.ValueVectorOf.0* %tmp4, null
; checking special case of ignored inttoptr.
; CHECK-DEP:        %tobool = icmp eq %class.ValueVectorOf.0* %tmp4, null
; CHECK-DEP-NEXT: ; Load(GEP(Arg 1)
; CHECK-DEP-NEXT: ;          3)
; CHECK-DEP-NEXT:   %tmp5 = inttoptr i64 %tmp2 to %class.XMLMsgLoader*
  %tmp5 = inttoptr i64 %tmp2 to %class.XMLMsgLoader*
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %fValues4 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %other, i64 0, i32 2
  %tmp6 = bitcast %class.RefArrayVectorOf** %fValues4 to %class.BaseRefVectorOf**
  %tmp7 = load %class.BaseRefVectorOf*, %class.BaseRefVectorOf** %tmp6
  %fCurCount.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp7, i64 0, i32 2
  %tmp8 = load i32, i32* %fCurCount.i
; New array type has the same size as an old one.
; CHECK-MOD:          %call7 = invoke i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp5)
  %call7 = invoke i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp5)
          to label %invoke.cont6 unwind label %lpad

invoke.cont6:                                     ; preds = %if.then
  %tmp9 = bitcast i8* %call7 to %class.ValueVectorOf.0*
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp10 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
; CHECK-MOD:          %tmp10 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields3
  %tmp10 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf.0* %tmp9, %class.ValueVectorOf.0* %tmp10)
; CHECK-MOD:          invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp9, %__SOA_AR_class.ValueVectorOf.0* %tmp10)
  invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf.0* %tmp9, %class.ValueVectorOf.0* %tmp10)
          to label %invoke.cont10 unwind label %lpad9

invoke.cont10:                                    ; preds = %invoke.cont6

; checking special case BitCast as GEP
; CHECK-DEP:      ; GEP(Arg 0)
; CHECK-DEP-NEXT: ;     0
; CHECK-DEP-NEXT:   %tmp11 = bitcast %class.FieldValueMap* %this to i8**
; CHECK-MOD:        %tmp11 = bitcast %__SOA_class.FieldValueMap* %this to i8**
  %tmp11 = bitcast %class.FieldValueMap* %this to i8**
; CHECK-DEP-NEXT: ; Store(Alloc size(Const)
; CHECK-DEP-NEXT: ;                 (Func(Load(GEP(Arg 1)
; CHECK-DEP-NEXT: ;                                3))))
; CHECK-DEP-NEXT: ;      (GEP(Arg 0)
; CHECK-DEP-NEXT: ;           0)
; CHECK-TRANS:      ; ArrayInst: Init ptr to array
; CHECK-TRANS-NEXT:   store i8* %call7, i8** %tmp11
; CHECK-MOD:          store i8* %call7, i8** %tmp11
  store i8* %call7, i8** %tmp11
  %tmp12 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; Call removed
; CHECK-MOD-NEXT:     br label %invoke.cont14
  %call15 = invoke i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp12)
          to label %invoke.cont14 unwind label %lpad

invoke.cont14:                                    ; preds = %invoke.cont10
  %tmp13 = bitcast i8* %call15 to %class.ValueVectorOf.1*
  %fValidators16 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %other, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp14 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators16
  %tmp14 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators16
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(ValueVectorOf<DatatypeValidator*> const&)"(%class.ValueVectorOf.1* %tmp13, %class.ValueVectorOf.1* %tmp14)
; Call removed
; CHECK-MOD:          br label %invoke.cont18
  invoke void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(ValueVectorOf<DatatypeValidator*> const&)"(%class.ValueVectorOf.1* %tmp13, %class.ValueVectorOf.1* %tmp14)
          to label %invoke.cont18 unwind label %lpad17

; CHECK-MOD:      invoke.cont18:
invoke.cont18:                                    ; preds = %invoke.cont14
; CHECK-DEP:      ; GEP(Arg 0)
; CHECK-DEP-NEXT: ;     1
; CHECK-DEP-NEXT:   %tmp15 = bitcast %class.ValueVectorOf.1** %fValidators to i8**
; bitcast is removed
  %tmp15 = bitcast %class.ValueVectorOf.1** %fValidators to i8**
; CHECK-TRANS:      ; ArrayInst: Init ptr to array
; CHECK-TRANS-NEXT:   store i8* %call15, i8** %tmp15
  store i8* %call15, i8** %tmp15
; Store removed
; CHECK-MOD-NEXT:     %tmp16 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %tmp16 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %call23 = invoke i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 40, %class.XMLMsgLoader* %tmp16)
          to label %invoke.cont22 unwind label %lpad

invoke.cont22:                                    ; preds = %invoke.cont18
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp17 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
  %tmp17 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields3
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call27 = tail call i32 @"ValueVectorOf<IC_Field*>::curCapacity() const"(%class.ValueVectorOf.0* %tmp17)
  %call27 = tail call i32 @"ValueVectorOf<IC_Field*>::curCapacity() const"(%class.ValueVectorOf.0* %tmp17)
  %tmp18 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %tmp19 = bitcast i8* %call23 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @"vtable for BaseRefVectorOf<unsigned short>", i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %tmp19
  %tmp20 = getelementptr inbounds i8, i8* %call23, i64 8
  store i8 1, i8* %tmp20
  %fCurCount.i.i = getelementptr inbounds i8, i8* %call23, i64 12
  %tmp21 = bitcast i8* %fCurCount.i.i to i32*
  store i32 0, i32* %tmp21
  %fMaxCount.i.i = getelementptr inbounds i8, i8* %call23, i64 16
  %tmp22 = bitcast i8* %fMaxCount.i.i to i32*
  store i32 %call27, i32* %tmp22
  %fElemList.i.i = getelementptr inbounds i8, i8* %call23, i64 24
  %tmp23 = bitcast i8* %fElemList.i.i to i16***
  store i16** null, i16*** %tmp23
  %fMemoryManager.i.i = getelementptr inbounds i8, i8* %call23, i64 32
  %tmp24 = bitcast i8* %fMemoryManager.i.i to %class.XMLMsgLoader**
  store %class.XMLMsgLoader* %tmp18, %class.XMLMsgLoader** %tmp24
  %conv.i.i = zext i32 %call27 to i64
  %mul.i.i = shl nuw nsw i64 %conv.i.i, 3
  %tmp25 = bitcast %class.XMLMsgLoader* %tmp18 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i.i = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp25
  %vfn.i.i = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i.i, i64 2
  %tmp26 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i.i
  %call.i.i71 = invoke i8* %tmp26(%class.XMLMsgLoader* %tmp18, i64 %mul.i.i)
          to label %call.i.i.noexc unwind label %lpad25

call.i.i.noexc:                                   ; preds = %invoke.cont22
  %tmp27 = bitcast i8* %fElemList.i.i to i8**
  store i8* %call.i.i71, i8** %tmp27
  %cmp11.i.i = icmp eq i32 %call27, 0
  br i1 %cmp11.i.i, label %invoke.cont29, label %for.body.lr.ph.i.i

for.body.lr.ph.i.i:                               ; preds = %call.i.i.noexc
  tail call void @llvm.memset.p0i8.i64(i8* %call.i.i71, i8 0, i64 %mul.i.i, i1 false)
  br label %invoke.cont29

invoke.cont29:                                    ; preds = %for.body.lr.ph.i.i, %call.i.i.noexc
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @"vtable for RefArrayVectorOf<unsigned short>", i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %tmp19
  %tmp28 = bitcast %class.RefArrayVectorOf** %fValues to i8**
  store i8* %call23, i8** %tmp28
  %cmp131 = icmp eq i32 %tmp8, 0
  br i1 %cmp131, label %if.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %invoke.cont29
  %tmp29 = bitcast %class.RefArrayVectorOf** %fValues to %class.BaseRefVectorOf**
  %tmp30 = zext i32 %tmp8 to i64
  br label %for.body

lpad:                                             ; preds = %invoke.cont18, %invoke.cont10, %if.then
  %tmp31 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  %tmp32 = extractvalue { i8*, i32 } %tmp31, 0
  %tmp33 = extractvalue { i8*, i32 } %tmp31, 1
  br label %ehcleanup

lpad9:                                            ; preds = %invoke.cont6
  %tmp34 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  %tmp35 = extractvalue { i8*, i32 } %tmp34, 0
  %tmp36 = extractvalue { i8*, i32 } %tmp34, 1
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call7, %class.XMLMsgLoader* %tmp5)
          to label %ehcleanup unwind label %terminate.lpad

lpad17:                                           ; preds = %invoke.cont14
  %tmp37 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  %tmp38 = extractvalue { i8*, i32 } %tmp37, 0
  %tmp39 = extractvalue { i8*, i32 } %tmp37, 1
; CHECK-MOD:      br label %ehcleanup
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call15, %class.XMLMsgLoader* %tmp12)
          to label %ehcleanup unwind label %terminate.lpad

lpad25:                                           ; preds = %invoke.cont22
  %tmp40 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  %tmp41 = extractvalue { i8*, i32 } %tmp40, 0
  %tmp42 = extractvalue { i8*, i32 } %tmp40, 1
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call23, %class.XMLMsgLoader* %tmp16)
          to label %ehcleanup unwind label %terminate.lpad

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %tmp43 = load %class.BaseRefVectorOf*, %class.BaseRefVectorOf** %tmp29
  %tmp44 = load %class.BaseRefVectorOf*, %class.BaseRefVectorOf** %tmp6
  %fCurCount.i102 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp44, i64 0, i32 2
  %tmp45 = load i32, i32* %fCurCount.i102
  %tmp46 = zext i32 %tmp45 to i64
  %cmp.i103 = icmp ult i64 %indvars.iv, %tmp46
  br i1 %cmp.i103, label %invoke.cont35, label %if.then.i104

if.then.i104:                                     ; preds = %for.body
  %exception.i = tail call i8* @__cxa_allocate_exception(i64 48)
  %fMemoryManager.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp44, i64 0, i32 5
  %tmp47 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i
  %tmp48 = bitcast i8* %exception.i to %class.XMLException*
  invoke void @"XMLException::XMLException(char const*, unsigned int, MemoryManager*)"(%class.XMLException* %tmp48, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.941, i64 0, i64 0), i32 249, %class.XMLMsgLoader* %tmp47)
          to label %.noexc112 unwind label %lpad.i

.noexc112:                                        ; preds = %if.then.i104
  %tmp49 = bitcast i8* %exception.i to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @"vtable for ArrayIndexOutOfBoundsException", i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %tmp49
  invoke void @"XMLException::loadExceptText(XMLExcepts::Codes)"(%class.XMLException* %tmp48, i32 116)
          to label %invoke.cont.i unwind label %lpad.i111

lpad.i111:                                        ; preds = %.noexc112
  %tmp50 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  invoke void @"XMLException::~XMLException()"(%class.XMLException* %tmp48)
          to label %lpad.i.body unwind label %terminate.lpad.i

terminate.lpad.i:                                 ; preds = %lpad.i111
  %tmp51 = landingpad { i8*, i32 }
          catch i8* null
  %tmp52 = extractvalue { i8*, i32 } %tmp51, 0
  tail call void @__clang_call_terminate(i8* %tmp52)
  unreachable

invoke.cont.i:                                    ; preds = %.noexc112
  invoke void @__cxa_throw(i8* %exception.i, i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for ArrayIndexOutOfBoundsException" to i8*), i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
          to label %.noexc107 unwind label %lpad34.loopexit.split-lp

.noexc107:                                        ; preds = %invoke.cont.i
  unreachable

lpad.i:                                           ; preds = %if.then.i104
  %tmp53 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  br label %lpad.i.body

lpad.i.body:                                      ; preds = %lpad.i, %lpad.i111
  %eh.lpad-body113 = phi { i8*, i32 } [ %tmp53, %lpad.i ], [ %tmp50, %lpad.i111 ]
  tail call void @__cxa_free_exception(i8* %exception.i)
  br label %lpad34.body

invoke.cont35:                                    ; preds = %for.body
  %fElemList.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp44, i64 0, i32 4
  %tmp54 = load i16**, i16*** %fElemList.i
  %arrayidx.i106 = getelementptr inbounds i16*, i16** %tmp54, i64 %indvars.iv
  %tmp55 = load i16*, i16** %arrayidx.i106
  %tmp56 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %tobool.i = icmp eq i16* %tmp55, null
  br i1 %tobool.i, label %invoke.cont38, label %lor.lhs.false.i.i

lor.lhs.false.i.i:                                ; preds = %invoke.cont35
  %tmp57 = load i16, i16* %tmp55
  %cmp1.i.i = icmp eq i16 %tmp57, 0
  br i1 %cmp1.i.i, label %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]", label %while.cond.i.i

while.cond.i.i:                                   ; preds = %while.cond.i.i, %lor.lhs.false.i.i
  %src.pn.i.i = phi i16* [ %pszTmp.0.i.i, %while.cond.i.i ], [ %tmp55, %lor.lhs.false.i.i ]
  %pszTmp.0.i.i = getelementptr inbounds i16, i16* %src.pn.i.i, i64 1
  %tmp58 = load i16, i16* %pszTmp.0.i.i
  %tobool.i.i = icmp eq i16 %tmp58, 0
  br i1 %tobool.i.i, label %while.end.i.i, label %while.cond.i.i

while.end.i.i:                                    ; preds = %while.cond.i.i
  %sub.ptr.lhs.cast.i.i = ptrtoint i16* %pszTmp.0.i.i to i64
  %sub.ptr.rhs.cast.i.i = ptrtoint i16* %tmp55 to i64
  %sub.ptr.sub.i.i = sub i64 2, %sub.ptr.rhs.cast.i.i
  %tmp59 = add i64 %sub.ptr.sub.i.i, %sub.ptr.lhs.cast.i.i
  %phitmp12.i = and i64 %tmp59, 8589934590
  br label %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]"

"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]": ; preds = %while.end.i.i, %lor.lhs.false.i.i
  %retval.0.i.i = phi i64 [ %phitmp12.i, %while.end.i.i ], [ 2, %lor.lhs.false.i.i ]
  %tmp60 = bitcast %class.XMLMsgLoader* %tmp56 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i108 = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp60
  %vfn.i = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i108, i64 2
  %tmp61 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i
  %call1.i109 = invoke i8* %tmp61(%class.XMLMsgLoader* %tmp56, i64 %retval.0.i.i)
          to label %call1.i.noexc unwind label %lpad34.loopexit

call1.i.noexc:                                    ; preds = %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]"
  %tmp62 = bitcast i8* %call1.i109 to i16*
  %tmp63 = bitcast i16* %tmp55 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call1.i109, i8* %tmp63, i64 %retval.0.i.i, i1 false)
  br label %invoke.cont38

invoke.cont38:                                    ; preds = %call1.i.noexc, %invoke.cont35
  %ret.0.i = phi i16* [ %tmp62, %call1.i.noexc ], [ null, %invoke.cont35 ]
  %fCurCount.i.i72 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 2
  %tmp64 = load i32, i32* %fCurCount.i.i72
  %add.i.i = add i32 %tmp64, 1
  %fMaxCount.i.i73 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 3
  %tmp65 = load i32, i32* %fMaxCount.i.i73
  %cmp.i.i = icmp ugt i32 %add.i.i, %tmp65
  br i1 %cmp.i.i, label %if.end.i.i, label %"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i"

"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i": ; preds = %invoke.cont38
  %fElemList.phi.trans.insert.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 4
  %.pre.i = load i16**, i16*** %fElemList.phi.trans.insert.i
  br label %for.inc

if.end.i.i:                                       ; preds = %invoke.cont38
  %div.i.i = lshr i32 %tmp65, 1
  %add4.i.i = add i32 %div.i.i, %tmp65
  %cmp5.i.i = icmp ult i32 %add.i.i, %add4.i.i
  %spec.select.i.i = select i1 %cmp5.i.i, i32 %add4.i.i, i32 %add.i.i
  %fMemoryManager.i.i74 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 5
  %tmp66 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i.i74
  %conv.i.i75 = zext i32 %spec.select.i.i to i64
  %mul.i.i76 = shl nuw nsw i64 %conv.i.i75, 3
  %tmp67 = bitcast %class.XMLMsgLoader* %tmp66 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i.i77 = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp67
  %vfn.i.i78 = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i.i77, i64 2
  %tmp68 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i.i78
  %call.i.i82 = invoke i8* %tmp68(%class.XMLMsgLoader* %tmp66, i64 %mul.i.i76)
          to label %call.i.i.noexc81 unwind label %lpad34.loopexit

call.i.i.noexc81:                                 ; preds = %if.end.i.i
  %tmp69 = bitcast i8* %call.i.i82 to i16**
  %tmp70 = load i32, i32* %fCurCount.i.i72
  %cmp1347.i.i = icmp eq i32 %tmp70, 0
  br i1 %cmp1347.i.i, label %for.cond16.preheader.i.i, label %for.body.lr.ph.i.i80

for.body.lr.ph.i.i80:                             ; preds = %call.i.i.noexc81
  %fElemList.i.i79 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 4
  %tmp71 = load i16**, i16*** %fElemList.i.i79
  %tmp72 = zext i32 %tmp70 to i64
  br label %for.body.i.i

for.cond16.preheader.i.i:                         ; preds = %for.body.i.i, %call.i.i.noexc81
  %cmp1745.i.i = icmp ult i32 %tmp70, %spec.select.i.i
  br i1 %cmp1745.i.i, label %for.body18.preheader.i.i, label %for.end23.i.i

for.body18.preheader.i.i:                         ; preds = %for.cond16.preheader.i.i
  %tmp73 = zext i32 %tmp70 to i64
  %tmp74 = shl nuw nsw i64 %tmp73, 3
  %scevgep.i.i = getelementptr i8, i8* %call.i.i82, i64 %tmp74
  %tmp75 = add i32 %spec.select.i.i, -1
  %tmp76 = sub i32 %tmp75, %tmp70
  %tmp77 = zext i32 %tmp76 to i64
  %tmp78 = shl nuw nsw i64 %tmp77, 3
  %tmp79 = add nuw nsw i64 %tmp78, 8
  tail call void @llvm.memset.p0i8.i64(i8* %scevgep.i.i, i8 0, i64 %tmp79, i1 false)
  br label %for.end23.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i80
  %indvars.iv.i.i = phi i64 [ 0, %for.body.lr.ph.i.i80 ], [ %indvars.iv.next.i.i, %for.body.i.i ]
  %arrayidx.i.i = getelementptr inbounds i16*, i16** %tmp71, i64 %indvars.iv.i.i
  %tmp80 = bitcast i16** %arrayidx.i.i to i64*
  %tmp81 = load i64, i64* %tmp80
  %arrayidx15.i.i = getelementptr inbounds i16*, i16** %tmp69, i64 %indvars.iv.i.i
  %tmp82 = bitcast i16** %arrayidx15.i.i to i64*
  store i64 %tmp81, i64* %tmp82
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i.i, %tmp72
  br i1 %exitcond.i, label %for.cond16.preheader.i.i, label %for.body.i.i

for.end23.i.i:                                    ; preds = %for.body18.preheader.i.i, %for.cond16.preheader.i.i
  %tmp83 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i.i74
  %fElemList25.i.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp43, i64 0, i32 4
  %tmp84 = bitcast i16*** %fElemList25.i.i to i8**
  %tmp85 = load i8*, i8** %tmp84
  %tmp86 = bitcast %class.XMLMsgLoader* %tmp83 to void (%class.XMLMsgLoader*, i8*)***
  %vtable26.i.i = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp86
  %vfn27.i.i = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vtable26.i.i, i64 3
  %tmp87 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vfn27.i.i
  invoke void %tmp87(%class.XMLMsgLoader* %tmp83, i8* %tmp85)
          to label %.noexc83 unwind label %lpad34.loopexit

.noexc83:                                         ; preds = %for.end23.i.i
  store i8* %call.i.i82, i8** %tmp84
  store i32 %spec.select.i.i, i32* %fMaxCount.i.i73
  %.pre3.i = load i32, i32* %fCurCount.i.i72
  %.pre4.i = add i32 %.pre3.i, 1
  br label %for.inc

for.inc:                                          ; preds = %.noexc83, %"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i"
  %inc.pre-phi.i = phi i32 [ %add.i.i, %"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i" ], [ %.pre4.i, %.noexc83 ]
  %tmp88 = phi i32 [ %tmp64, %"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i" ], [ %.pre3.i, %.noexc83 ]
  %tmp89 = phi i16** [ %.pre.i, %"entry.BaseRefVectorOf<unsigned short>::ensureExtraCapacity(unsigned int).exit_crit_edge.i" ], [ %tmp69, %.noexc83 ]
  %idxprom.i = zext i32 %tmp88 to i64
  %arrayidx.i = getelementptr inbounds i16*, i16** %tmp89, i64 %idxprom.i
  store i16* %ret.0.i, i16** %arrayidx.i
  store i32 %inc.pre-phi.i, i32* %fCurCount.i.i72
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, %tmp30
  br i1 %cmp, label %for.body, label %if.end

lpad34.loopexit:                                  ; preds = %for.end23.i.i, %if.end.i.i, %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]"
  %lpad.loopexit = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  br label %lpad34.body

lpad34.loopexit.split-lp:                         ; preds = %invoke.cont.i
  %lpad.loopexit.split-lp = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*)
  br label %lpad34.body

lpad34.body:                                      ; preds = %lpad34.loopexit.split-lp, %lpad34.loopexit, %lpad.i.body
  %eh.lpad-body = phi { i8*, i32 } [ %eh.lpad-body113, %lpad.i.body ], [ %lpad.loopexit, %lpad34.loopexit ], [ %lpad.loopexit.split-lp, %lpad34.loopexit.split-lp ]
  %tmp90 = extractvalue { i8*, i32 } %eh.lpad-body, 0
  %tmp91 = extractvalue { i8*, i32 } %eh.lpad-body, 1
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad34.body, %lpad25, %lpad17, %lpad9, %lpad
  %ehselector.slot.0 = phi i32 [ %tmp91, %lpad34.body ], [ %tmp42, %lpad25 ], [ %tmp33, %lpad ], [ %tmp39, %lpad17 ], [ %tmp36, %lpad9 ]
  %exn.slot.0 = phi i8* [ %tmp90, %lpad34.body ], [ %tmp41, %lpad25 ], [ %tmp32, %lpad ], [ %tmp38, %lpad17 ], [ %tmp35, %lpad9 ]
  %tmp92 = tail call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for OutOfMemoryException" to i8*))
  %matches = icmp eq i32 %ehselector.slot.0, %tmp92
  br i1 %matches, label %catch, label %ehcleanup49

catch:                                            ; preds = %ehcleanup
  %tmp93 = tail call i8* @__cxa_begin_catch(i8* %exn.slot.0)
  invoke void @__cxa_rethrow()
          to label %unreachable unwind label %lpad42

lpad42:                                           ; preds = %catch
  %tmp94 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %ehcleanup49.thread unwind label %terminate.lpad

ehcleanup49.thread:                               ; preds = %lpad42
  %tmp95 = extractvalue { i8*, i32 } %tmp94, 1
  %tmp96 = extractvalue { i8*, i32 } %tmp94, 0
  br label %invoke.cont50

ehcleanup49:                                      ; preds = %ehcleanup
; CHECK-MOD:  %cmp.i = icmp eq %__SOA_class.FieldValueMap* %this, null
  %cmp.i = icmp eq %class.FieldValueMap* %this, null
  br i1 %cmp.i, label %invoke.cont50, label %memptr.end.i

memptr.end.i:                                     ; preds = %ehcleanup49
; CHECK-MOD:  invoke void @"FieldValueMap::cleanUp(){{.*}}"(%__SOA_class.FieldValueMap* %this)
  invoke void @"FieldValueMap::cleanUp()"(%class.FieldValueMap* %this)
          to label %invoke.cont50 unwind label %terminate.lpad

invoke.cont50:                                    ; preds = %memptr.end.i, %ehcleanup49, %ehcleanup49.thread
  %exn.slot.1129 = phi i8* [ %tmp96, %ehcleanup49.thread ], [ %exn.slot.0, %ehcleanup49 ], [ %exn.slot.0, %memptr.end.i ]
  %ehselector.slot.1128 = phi i32 [ %tmp95, %ehcleanup49.thread ], [ %ehselector.slot.0, %ehcleanup49 ], [ %ehselector.slot.0, %memptr.end.i ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.1129, 0
  %lpad.val54 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.1128, 1
  resume { i8*, i32 } %lpad.val54

if.end:                                           ; preds = %for.inc, %invoke.cont29, %entry
  ret void

terminate.lpad:                                   ; preds = %memptr.end.i, %lpad42, %lpad25, %lpad17, %lpad9
  %tmp97 = landingpad { i8*, i32 }
          catch i8* null
  %tmp98 = extractvalue { i8*, i32 } %tmp97, 0
  tail call void @__clang_call_terminate(i8* %tmp98)
  unreachable

unreachable:                                      ; preds = %catch
  unreachable
}

declare hidden void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf.0*, %class.ValueVectorOf.0*)

declare hidden void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(ValueVectorOf<DatatypeValidator*> const&)"(%class.ValueVectorOf.1*, %class.ValueVectorOf.1*)

declare hidden i32 @"ValueVectorOf<IC_Field*>::curCapacity() const"(%class.ValueVectorOf.0*)

declare hidden void @"FieldValueMap::cleanUp()"(%class.FieldValueMap*)

declare hidden void @"XMLException::~XMLException()"(%class.XMLException*)

declare hidden void @"XMLException::XMLException(char const*, unsigned int, MemoryManager*)"(%class.XMLException*, i8*, i32, %class.XMLMsgLoader*)

declare hidden void @"XMLException::loadExceptText(XMLExcepts::Codes)"(%class.XMLException*, i32)

declare hidden i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64, %class.XMLMsgLoader*)

declare hidden void @"XMemory::operator delete(void*_ MemoryManager*)"(i8*, %class.XMLMsgLoader*)

; CHECK-TRANS: ; Seen nullptr init with memset.
; CHECK-TRANS: ; Seen cctor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; XCHECK-DEP: Deps computed: 101, Queries: 298

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
; int(void*) type.
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DILocalVariable(name: "na", arg: 1, scope: !8, file: !1, line: 1, type: !12)
!14 = !DILocation(line: 1, column: 1, scope: !8)
!15 = !DILocation(line: 1, column: 1, scope: !8)
!16 = !DILocation(line: 1, column: 1, scope: !8)
