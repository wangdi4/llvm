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
; RUN:          -dtrans-soatoaos-method-call-site-comparison=ctor                               \
; RUN:          -dtrans-soatoaos-array-ctor="ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"           \
; RUN:          -dtrans-soatoaos-array-ctor="ValueVectorOf<DatatypeValidator*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"  \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=append                             \
; RUN:          -dtrans-soatoaos-array-append="ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"                      \
; RUN:          -dtrans-soatoaos-array-append="ValueVectorOf<DatatypeValidator*>::addElement(DatatypeValidator* const&)"    \
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
; RUN:          -dtrans-soatoaos-method-call-site-comparison=ctor                               \
; RUN:          -dtrans-soatoaos-array-ctor="ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"           \
; RUN:          -dtrans-soatoaos-array-ctor="ValueVectorOf<DatatypeValidator*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"  \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=append                             \
; RUN:          -dtrans-soatoaos-array-append="ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"                      \
; RUN:          -dtrans-soatoaos-array-append="ValueVectorOf<DatatypeValidator*>::addElement(DatatypeValidator* const&)"    \
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
; CHECK-TRANS: ; Checking structure's method FieldValueMap::put(IC_Field*, DatatypeValidator*, unsigned short const*)
; CHECK-TRANS: ; IR: analysed completely

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 11

; Also checks that call sites to addElement methods can be merged. Same for ctor.

; Checks transformation. Types change and combined methods removed.

; inline void FieldValueMap::put(IC_Field *const key, DatatypeValidator *const dv,
;                                const XMLCh *const value) {
;   if (!fFields) {
;     fFields = new (fMemoryManager) ValueVectorOf<IC_Field *>(4, fMemoryManager);
;     fValidators = new (fMemoryManager)
;         ValueVectorOf<DatatypeValidator *>(4, fMemoryManager);
;     fValues =
;         new (fMemoryManager) RefArrayVectorOf<XMLCh>(4, true, fMemoryManager);
;   }
;   int keyIndex = indexOf(key);
;   if (keyIndex == -1) {
;     fFields->addElement(key);
;     fValidators->addElement(dv);
;     fValues->addElement(XMLString::replicate(value, fMemoryManager));
;   } else {
;     fValidators->setElementAt(dv, keyIndex);
;     fValues->setElementAt(XMLString::replicate(value, fMemoryManager),
;                           keyIndex);
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

@"vtable for BaseRefVectorOf<unsigned short>" = external hidden constant { [9 x i8*] }
@"vtable for RefArrayVectorOf<unsigned short>" = external hidden constant { [9 x i8*] }

declare i32 @__gxx_personality_v0(...)

declare hidden void @__clang_call_terminate(i8*)

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

declare hidden i32 @"FieldValueMap::indexOf(IC_Field const*) const"(%class.FieldValueMap*, %class.IC_Field*)

define hidden void @"FieldValueMap::put(IC_Field*, DatatypeValidator*, unsigned short const*)"(%class.FieldValueMap* %this, %class.IC_Field* %key, %class.DatatypeValidator* %dv, i16* %value) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %key.addr = alloca %class.IC_Field*
  %dv.addr = alloca %class.DatatypeValidator*
  store %class.IC_Field* %key, %class.IC_Field** %key.addr
  store %class.DatatypeValidator* %dv, %class.DatatypeValidator** %dv.addr
  %fFields = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-MOD:          %tmp = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields
  %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-MOD-NEXT:     %tobool = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp, null
  %tobool = icmp eq %class.ValueVectorOf.0* %tmp, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %fMemoryManager = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 3
  %tmp1 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; New array type has the same size as an old one.
; CHECK-MOD:          %call = tail call i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp1)
  %call = tail call i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp1)
  %tmp2 = bitcast i8* %call to %class.ValueVectorOf.0*
  %tmp3 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.0* %tmp2, i32 4, %class.XMLMsgLoader* %tmp3, i1 zeroext false)
; CHECK-MOD:          invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp2, i32 4, %class.XMLMsgLoader* %tmp3, i1 zeroext false)
  invoke void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.0* %tmp2, i32 4, %class.XMLMsgLoader* %tmp3, i1 zeroext false)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
; CHECK-DEP:      invoke.cont:                                      ; preds = %if.then
; CHECK-DEP-NEXT: ; GEP(Arg 0)
; CHECK-DEP-NEXT: ;     0
; CHECK-DEP-NEXT:   %tmp4 = bitcast %class.FieldValueMap* %this to i8**
; CHECK-MOD:        %tmp4 = bitcast %__SOA_class.FieldValueMap* %this to i8**
  %tmp4 = bitcast %class.FieldValueMap* %this to i8**
; CHECK-TRANS:      ; ArrayInst: Init ptr to array
; CHECK-TRANS-NEXT:   store i8* %call, i8** %tmp4
; CHECK-MOD-NEXT:     store i8* %call, i8** %tmp4
  store i8* %call, i8** %tmp4
  %tmp5 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-MOD-NOT:   XMemory::operator new
  %call6 = tail call i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 32, %class.XMLMsgLoader* %tmp5)
  %tmp6 = bitcast i8* %call6 to %class.ValueVectorOf.1*
  %tmp7 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   invoke void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.1* %tmp6, i32 4, %class.XMLMsgLoader* %tmp7, i1 zeroext false)
; CHECK-MOD:          br label %invoke.cont9
  invoke void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.1* %tmp6, i32 4, %class.XMLMsgLoader* %tmp7, i1 zeroext false)
          to label %invoke.cont9 unwind label %lpad8

; CHECK-MOD:        invoke.cont9:
invoke.cont9:                                     ; preds = %invoke.cont
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
  %tmp8 = bitcast %class.ValueVectorOf.1** %fValidators to i8**
; CHECK-TRANS:      ; ArrayInst: Init ptr to array
; CHECK-TRANS-NEXT:   store i8* %call6, i8** %tmp8
  store i8* %call6, i8** %tmp8
; Store removed.
; CHECK-MOD-NEXT:     %tmp9 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %tmp9 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %call12 = tail call i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64 40, %class.XMLMsgLoader* %tmp9)
  %tmp10 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %tmp11 = bitcast i8* %call12 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @"vtable for BaseRefVectorOf<unsigned short>", i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %tmp11
  %tmp12 = getelementptr inbounds i8, i8* %call12, i64 8
  store i8 1, i8* %tmp12
  %fCurCount.i.i = getelementptr inbounds i8, i8* %call12, i64 12
  %tmp13 = bitcast i8* %fCurCount.i.i to i32*
  store i32 0, i32* %tmp13
  %fMaxCount.i.i = getelementptr inbounds i8, i8* %call12, i64 16
  %tmp14 = bitcast i8* %fMaxCount.i.i to i32*
  store i32 4, i32* %tmp14
  %fElemList.i.i = getelementptr inbounds i8, i8* %call12, i64 24
  %tmp15 = bitcast i8* %fElemList.i.i to i16***
  store i16** null, i16*** %tmp15
  %fMemoryManager.i.i = getelementptr inbounds i8, i8* %call12, i64 32
  %tmp16 = bitcast i8* %fMemoryManager.i.i to %class.XMLMsgLoader**
  store %class.XMLMsgLoader* %tmp10, %class.XMLMsgLoader** %tmp16
  %tmp17 = bitcast %class.XMLMsgLoader* %tmp10 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i.i = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp17
  %vfn.i.i = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i.i, i64 2
  %tmp18 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i.i
  %call.i.i34 = invoke i8* %tmp18(%class.XMLMsgLoader* %tmp10, i64 32)
          to label %invoke.cont15 unwind label %lpad14

invoke.cont15:                                    ; preds = %invoke.cont9
  %tmp19 = bitcast i8* %fElemList.i.i to i8**
  store i8* %call.i.i34, i8** %tmp19
  tail call void @llvm.memset.p0i8.i64(i8* %call.i.i34, i8 0, i64 32, i1 false)
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [9 x i8*] }, { [9 x i8*] }* @"vtable for RefArrayVectorOf<unsigned short>", i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %tmp11
  %fValues = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp20 = bitcast %class.RefArrayVectorOf** %fValues to i8**
  store i8* %call12, i8** %tmp20
  %.pre = load %class.IC_Field*, %class.IC_Field** %key.addr
  br label %if.end

lpad:                                             ; preds = %if.then
  %tmp21 = landingpad { i8*, i32 }
          cleanup
  %tmp22 = extractvalue { i8*, i32 } %tmp21, 0
  %tmp23 = extractvalue { i8*, i32 } %tmp21, 1
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call, %class.XMLMsgLoader* %tmp1)
          to label %eh.resume unwind label %terminate.lpad

lpad8:                                            ; preds = %invoke.cont
  %tmp24 = landingpad { i8*, i32 }
          cleanup
  %tmp25 = extractvalue { i8*, i32 } %tmp24, 0
  %tmp26 = extractvalue { i8*, i32 } %tmp24, 1
; CHECK-MOD:  br label %eh.resume
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call6, %class.XMLMsgLoader* %tmp5)
          to label %eh.resume unwind label %terminate.lpad

lpad14:                                           ; preds = %invoke.cont9
  %tmp27 = landingpad { i8*, i32 }
          cleanup
  %tmp28 = extractvalue { i8*, i32 } %tmp27, 0
  %tmp29 = extractvalue { i8*, i32 } %tmp27, 1
  invoke void @"XMemory::operator delete(void*_ MemoryManager*)"(i8* %call12, %class.XMLMsgLoader* %tmp9)
          to label %eh.resume unwind label %terminate.lpad

if.end:                                           ; preds = %invoke.cont15, %entry
  %tmp30 = phi %class.IC_Field* [ %key, %entry ], [ %.pre, %invoke.cont15 ]
; CHECK-MOD:     %call17 = tail call i32 @"FieldValueMap::indexOf(IC_Field const*) const{{.*}}"(%__SOA_class.FieldValueMap* %this, %class.IC_Field* %tmp30)
  %call17 = tail call i32 @"FieldValueMap::indexOf(IC_Field const*) const"(%class.FieldValueMap* %this, %class.IC_Field* %tmp30)
  %cmp = icmp eq i32 %call17, -1
  br i1 %cmp, label %if.then18, label %if.else

if.then18:                                        ; preds = %if.end
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp31 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-MOD:          %tmp31 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields
  %tmp31 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   call void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"(%class.ValueVectorOf.0* %tmp31, %class.IC_Field** %key.addr)
  call void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"(%class.ValueVectorOf.0* %tmp31, %class.IC_Field** %key.addr)
; Call removed.
; CHECK-MOD-NEXT:     %fValidators20 = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 0
  %fValidators20 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp32 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators20
; CHECK-MOD-NEXT:     %tmp32 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fValidators20
  %tmp32 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators20
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   call void @"ValueVectorOf<DatatypeValidator*>::addElement(DatatypeValidator* const&)"(%class.ValueVectorOf.1* %tmp32, %class.DatatypeValidator** %dv.addr)
  call void @"ValueVectorOf<DatatypeValidator*>::addElement(DatatypeValidator* const&)"(%class.ValueVectorOf.1* %tmp32, %class.DatatypeValidator** %dv.addr)
; Call removed.
; CHECK-MOD-NEXT:     %fValues21 = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 2
  %fValues21 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp33 = bitcast %class.RefArrayVectorOf** %fValues21 to %class.BaseRefVectorOf**
  %tmp34 = load %class.BaseRefVectorOf*, %class.BaseRefVectorOf** %tmp33
  %fMemoryManager22 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 3
  %tmp35 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager22
; CHECK-MOD:        %tobool.i = icmp eq i16* %value, null
  %tobool.i = icmp eq i16* %value, null
; Replacement of 2 addElement at the end of basic block.
; CHECK-MOD-NEXT:   call void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp31, %class.IC_Field** %key.addr, %class.DatatypeValidator** %dv.addr)
  br i1 %tobool.i, label %"XMLString::replicate(unsigned short const*, MemoryManager*) [clone .exit]", label %lor.lhs.false.i.i

lor.lhs.false.i.i:                                ; preds = %if.then18
  %tmp36 = load i16, i16* %value
  %cmp1.i.i = icmp eq i16 %tmp36, 0
  br i1 %cmp1.i.i, label %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]", label %while.cond.i.i

while.cond.i.i:                                   ; preds = %while.cond.i.i, %lor.lhs.false.i.i
  %src.pn.i.i = phi i16* [ %pszTmp.0.i.i, %while.cond.i.i ], [ %value, %lor.lhs.false.i.i ]
  %pszTmp.0.i.i = getelementptr inbounds i16, i16* %src.pn.i.i, i64 1
  %tmp37 = load i16, i16* %pszTmp.0.i.i
  %tobool.i.i = icmp eq i16 %tmp37, 0
  br i1 %tobool.i.i, label %while.end.i.i, label %while.cond.i.i

while.end.i.i:                                    ; preds = %while.cond.i.i
  %sub.ptr.lhs.cast.i.i = ptrtoint i16* %pszTmp.0.i.i to i64
  %sub.ptr.rhs.cast.i.i = ptrtoint i16* %value to i64
  %sub.ptr.sub.i.i = sub i64 2, %sub.ptr.rhs.cast.i.i
  %tmp38 = add i64 %sub.ptr.sub.i.i, %sub.ptr.lhs.cast.i.i
  %phitmp12.i = and i64 %tmp38, 8589934590
  br label %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]"

"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]": ; preds = %while.end.i.i, %lor.lhs.false.i.i
  %retval.0.i.i = phi i64 [ %phitmp12.i, %while.end.i.i ], [ 2, %lor.lhs.false.i.i ]
  %tmp39 = bitcast %class.XMLMsgLoader* %tmp35 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp39
  %vfn.i = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i, i64 2
  %tmp40 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i
  %call1.i = call i8* %tmp40(%class.XMLMsgLoader* %tmp35, i64 %retval.0.i.i)
  %tmp41 = bitcast i8* %call1.i to i16*
  %tmp42 = bitcast i16* %value to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call1.i, i8* %tmp42, i64 %retval.0.i.i, i1 false)
  br label %"XMLString::replicate(unsigned short const*, MemoryManager*) [clone .exit]"

"XMLString::replicate(unsigned short const*, MemoryManager*) [clone .exit]": ; preds = %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]", %if.then18
  %ret.0.i = phi i16* [ %tmp41, %"XMLString::stringLen(unsigned short const*) [clone .exit] [clone .i]" ], [ null, %if.then18 ]
  %fCurCount.i.i35 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 2
  %tmp43 = load i32, i32* %fCurCount.i.i35
  %add.i.i = add i32 %tmp43, 1
  %fMaxCount.i.i36 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 3
  %tmp44 = load i32, i32* %fMaxCount.i.i36
  %cmp.i.i = icmp ugt i32 %add.i.i, %tmp44
  br i1 %cmp.i.i, label %if.end.i.i, label %entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i

entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i: ; preds = %"XMLString::replicate(unsigned short const*, MemoryManager*) [clone .exit]"
  %fElemList.phi.trans.insert.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 4
  %.pre.i = load i16**, i16*** %fElemList.phi.trans.insert.i
  br label %"BaseRefVectorOf<unsigned short>::addElement(unsigned short*) [clone .exit]"

if.end.i.i:                                       ; preds = %"XMLString::replicate(unsigned short const*, MemoryManager*) [clone .exit]"
  %div.i.i = lshr i32 %tmp44, 1
  %add4.i.i = add i32 %div.i.i, %tmp44
  %cmp5.i.i = icmp ult i32 %add.i.i, %add4.i.i
  %spec.select.i.i = select i1 %cmp5.i.i, i32 %add4.i.i, i32 %add.i.i
  %fMemoryManager.i.i37 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 5
  %tmp45 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i.i37
  %conv.i.i = zext i32 %spec.select.i.i to i64
  %mul.i.i = shl nuw nsw i64 %conv.i.i, 3
  %tmp46 = bitcast %class.XMLMsgLoader* %tmp45 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i.i38 = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp46
  %vfn.i.i39 = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i.i38, i64 2
  %tmp47 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i.i39
  %call.i.i = call i8* %tmp47(%class.XMLMsgLoader* %tmp45, i64 %mul.i.i)
  %tmp48 = bitcast i8* %call.i.i to i16**
  %tmp49 = load i32, i32* %fCurCount.i.i35
  %cmp1347.i.i = icmp eq i32 %tmp49, 0
  br i1 %cmp1347.i.i, label %for.cond16.preheader.i.i, label %for.body.lr.ph.i.i

for.body.lr.ph.i.i:                               ; preds = %if.end.i.i
  %fElemList.i.i40 = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 4
  %tmp50 = load i16**, i16*** %fElemList.i.i40
  %tmp51 = zext i32 %tmp49 to i64
  br label %for.body.i.i

for.cond16.preheader.i.i:                         ; preds = %for.body.i.i, %if.end.i.i
  %cmp1745.i.i = icmp ult i32 %tmp49, %spec.select.i.i
  br i1 %cmp1745.i.i, label %for.body18.preheader.i.i, label %for.end23.i.i

for.body18.preheader.i.i:                         ; preds = %for.cond16.preheader.i.i
  %tmp52 = zext i32 %tmp49 to i64
  %tmp53 = shl nuw nsw i64 %tmp52, 3
  %scevgep.i.i = getelementptr i8, i8* %call.i.i, i64 %tmp53
  %tmp54 = add i32 %spec.select.i.i, -1
  %tmp55 = sub i32 %tmp54, %tmp49
  %tmp56 = zext i32 %tmp55 to i64
  %tmp57 = shl nuw nsw i64 %tmp56, 3
  %tmp58 = add nuw nsw i64 %tmp57, 8
  call void @llvm.memset.p0i8.i64(i8* %scevgep.i.i, i8 0, i64 %tmp58, i1 false)
  br label %for.end23.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i
  %indvars.iv.i.i = phi i64 [ 0, %for.body.lr.ph.i.i ], [ %indvars.iv.next.i.i, %for.body.i.i ]
  %arrayidx.i.i = getelementptr inbounds i16*, i16** %tmp50, i64 %indvars.iv.i.i
  %tmp59 = bitcast i16** %arrayidx.i.i to i64*
  %tmp60 = load i64, i64* %tmp59
  %arrayidx15.i.i = getelementptr inbounds i16*, i16** %tmp48, i64 %indvars.iv.i.i
  %tmp61 = bitcast i16** %arrayidx15.i.i to i64*
  store i64 %tmp60, i64* %tmp61
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i.i, %tmp51
  br i1 %exitcond.i, label %for.cond16.preheader.i.i, label %for.body.i.i

for.end23.i.i:                                    ; preds = %for.body18.preheader.i.i, %for.cond16.preheader.i.i
  %tmp62 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i.i37
  %fElemList25.i.i = getelementptr inbounds %class.BaseRefVectorOf, %class.BaseRefVectorOf* %tmp34, i64 0, i32 4
  %tmp63 = bitcast i16*** %fElemList25.i.i to i8**
  %tmp64 = load i8*, i8** %tmp63
  %tmp65 = bitcast %class.XMLMsgLoader* %tmp62 to void (%class.XMLMsgLoader*, i8*)***
  %vtable26.i.i = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp65
  %vfn27.i.i = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vtable26.i.i, i64 3
  %tmp66 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vfn27.i.i
  call void %tmp66(%class.XMLMsgLoader* %tmp62, i8* %tmp64)
  store i8* %call.i.i, i8** %tmp63
  store i32 %spec.select.i.i, i32* %fMaxCount.i.i36
  %.pre3.i = load i32, i32* %fCurCount.i.i35
  %.pre4.i = add i32 %.pre3.i, 1
  br label %"BaseRefVectorOf<unsigned short>::addElement(unsigned short*) [clone .exit]"

"BaseRefVectorOf<unsigned short>::addElement(unsigned short*) [clone .exit]": ; preds = %for.end23.i.i, %entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i
  %inc.pre-phi.i = phi i32 [ %add.i.i, %entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i ], [ %.pre4.i, %for.end23.i.i ]
  %tmp67 = phi i32 [ %tmp43, %entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i ], [ %.pre3.i, %for.end23.i.i ]
  %tmp68 = phi i16** [ %.pre.i, %entry._ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.exit_crit_edge.i ], [ %tmp48, %for.end23.i.i ]
  %idxprom.i = zext i32 %tmp67 to i64
  %arrayidx.i = getelementptr inbounds i16*, i16** %tmp68, i64 %idxprom.i
  store i16* %ret.0.i, i16** %arrayidx.i
  store i32 %inc.pre-phi.i, i32* %fCurCount.i.i35
  br label %if.end28

if.else:                                          ; preds = %if.end
  %fValidators24 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp69 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators24
  %tmp69 = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators24
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   call void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int)"(%class.ValueVectorOf.1* %tmp69, %class.DatatypeValidator** %dv.addr, i32 %call17)
; CHECK-MOD:         call void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp69, %class.DatatypeValidator** %dv.addr, i32 %call17)
  call void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int)"(%class.ValueVectorOf.1* %tmp69, %class.DatatypeValidator** %dv.addr, i32 %call17)
  %fValues25 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp70 = load %class.RefArrayVectorOf*, %class.RefArrayVectorOf** %fValues25
  %fMemoryManager26 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 3
  %tmp71 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager26
  %tobool.i41 = icmp eq i16* %value, null
  br i1 %tobool.i41, label %_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE.exit59, label %lor.lhs.false.i.i43

lor.lhs.false.i.i43:                              ; preds = %if.else
  %tmp72 = load i16, i16* %value
  %cmp1.i.i42 = icmp eq i16 %tmp72, 0
  br i1 %cmp1.i.i42, label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit.i57, label %while.cond.i.i47

while.cond.i.i47:                                 ; preds = %while.cond.i.i47, %lor.lhs.false.i.i43
  %src.pn.i.i44 = phi i16* [ %pszTmp.0.i.i45, %while.cond.i.i47 ], [ %value, %lor.lhs.false.i.i43 ]
  %pszTmp.0.i.i45 = getelementptr inbounds i16, i16* %src.pn.i.i44, i64 1
  %tmp73 = load i16, i16* %pszTmp.0.i.i45
  %tobool.i.i46 = icmp eq i16 %tmp73, 0
  br i1 %tobool.i.i46, label %while.end.i.i52, label %while.cond.i.i47

while.end.i.i52:                                  ; preds = %while.cond.i.i47
  %sub.ptr.lhs.cast.i.i48 = ptrtoint i16* %pszTmp.0.i.i45 to i64
  %sub.ptr.rhs.cast.i.i49 = ptrtoint i16* %value to i64
  %sub.ptr.sub.i.i50 = sub i64 2, %sub.ptr.rhs.cast.i.i49
  %tmp74 = add i64 %sub.ptr.sub.i.i50, %sub.ptr.lhs.cast.i.i48
  %phitmp12.i51 = and i64 %tmp74, 8589934590
  br label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit.i57

_ZN11xercesc_2_79XMLString9stringLenEPKt.exit.i57: ; preds = %while.end.i.i52, %lor.lhs.false.i.i43
  %retval.0.i.i53 = phi i64 [ %phitmp12.i51, %while.end.i.i52 ], [ 2, %lor.lhs.false.i.i43 ]
  %tmp75 = bitcast %class.XMLMsgLoader* %tmp71 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable.i54 = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp75
  %vfn.i55 = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable.i54, i64 2
  %tmp76 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn.i55
  %call1.i56 = call i8* %tmp76(%class.XMLMsgLoader* %tmp71, i64 %retval.0.i.i53)
  %tmp77 = bitcast i8* %call1.i56 to i16*
  %tmp78 = bitcast i16* %value to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call1.i56, i8* %tmp78, i64 %retval.0.i.i53, i1 false)
  br label %_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE.exit59

_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE.exit59: ; preds = %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit.i57, %if.else
  %ret.0.i58 = phi i16* [ %tmp77, %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit.i57 ], [ null, %if.else ]
  %tmp79 = bitcast %class.RefArrayVectorOf* %tmp70 to void (%class.RefArrayVectorOf*, i16*, i32)***
  %vtable = load void (%class.RefArrayVectorOf*, i16*, i32)**, void (%class.RefArrayVectorOf*, i16*, i32)*** %tmp79
  %vfn = getelementptr inbounds void (%class.RefArrayVectorOf*, i16*, i32)*, void (%class.RefArrayVectorOf*, i16*, i32)** %vtable, i64 2
  %tmp80 = load void (%class.RefArrayVectorOf*, i16*, i32)*, void (%class.RefArrayVectorOf*, i16*, i32)** %vfn
  call void %tmp80(%class.RefArrayVectorOf* %tmp70, i16* %ret.0.i58, i32 %call17)
  br label %if.end28

if.end28:                                         ; preds = %_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE.exit59, %"BaseRefVectorOf<unsigned short>::addElement(unsigned short*) [clone .exit]"
  ret void

eh.resume:                                        ; preds = %lpad14, %lpad8, %lpad
  %ehselector.slot.0 = phi i32 [ %tmp29, %lpad14 ], [ %tmp26, %lpad8 ], [ %tmp23, %lpad ]
  %exn.slot.0 = phi i8* [ %tmp28, %lpad14 ], [ %tmp25, %lpad8 ], [ %tmp22, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val29 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val29

terminate.lpad:                                   ; preds = %lpad14, %lpad8, %lpad
  %tmp81 = landingpad { i8*, i32 }
          catch i8* null
  %tmp82 = extractvalue { i8*, i32 } %tmp81, 0
  tail call void @__clang_call_terminate(i8* %tmp82)
  unreachable
}

declare hidden void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.0*, i32, %class.XMLMsgLoader*, i1 zeroext)

declare hidden void @"ValueVectorOf<DatatypeValidator*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf.1*, i32, %class.XMLMsgLoader*, i1 zeroext)

declare hidden void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"(%class.ValueVectorOf.0*, %class.IC_Field**)

declare hidden void @"ValueVectorOf<DatatypeValidator*>::addElement(DatatypeValidator* const&)"(%class.ValueVectorOf.1*, %class.DatatypeValidator**)

declare hidden void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int)"(%class.ValueVectorOf.1*, %class.DatatypeValidator**, i32)

declare hidden i8* @"XMemory::operator new(unsigned long_ MemoryManager*)"(i64, %class.XMLMsgLoader*)

declare hidden void @"XMemory::operator delete(void*_ MemoryManager*)"(i8*, %class.XMLMsgLoader*)
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; XCHECK-DEP: Deps computed: 80, Queries: 270
