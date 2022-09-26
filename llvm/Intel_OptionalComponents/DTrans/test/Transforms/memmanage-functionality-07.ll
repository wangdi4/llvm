; This test verifies that all functions are recognized for MemManageTrans
; when the functions have windows specific EH.
; This test is similar to memmanage-functionality-06.ll except for the
; following windows specific things.
;
; 1. ReusableArenaAllocator type: Unused field is removed.
;
; 2. Constructor: returns "this" object instead of void.
;    Constant value (i.e 10) instead of "%arg2" is saved to BlockSize field.
;
; 3. Windows EH  code
;
; This test verifies the following functionalities are recognized for
; MemManageTrans:
;
; getMemManager: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
;
; Constructor: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb
;
; AllocateBlock: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
;
; CommitAllocation: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_
;
; Destructor: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev
;
; Recognized Reset: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv
;
; Recognized DestroyObject: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

;
; RUN: opt < %s -dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; CHECK: MemManageTrans transformation:
; CHECK:   Considering candidate: %XStringCachedAllocator
; CHECK: Recognized GetMemManager: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK: Recognized Constructor: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb
; CHECK: Recognized AllocateBlock: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK: Recognized CommitAllocation: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_
; CHECK: Recognized Destructor: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev
; CHECK: Recognized Reset: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv
; CHECK: Recognized DestroyObject: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_
;

source_filename = "memmanage-functionality-07.ll"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%MemoryManager = type { i32 (...)** }
%ArenaAllocator = type { i32 (...)**, i16, %XalanList }
%XalanList = type { %MemoryManager*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%"XalanList<ReusableArenaBlock<XStringCached> *>::Node" = type { %ReusableArenaBlock*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%ReusableArenaBlock = type <{ %ArenaBlockBase, i16, i16, [4 x i8] }>
%ArenaBlockBase = type { %XalanAllocator, i16, i16, %XStringCached* }
%XalanAllocator = type { %MemoryManager* }
%XStringCached = type { %XStringBase, %"XPathExecutionContext::GetAndReleaseCachedString" }
%XStringBase = type { %XObject, double, %XObjectResultTreeFragProxy }
%XObject = type { %XalanReferenceCountedObject.base, i32, %XObjectFactory* }
%XalanReferenceCountedObject.base = type <{ i32 (...)**, i32 }>
%XObjectFactory = type opaque
%XObjectResultTreeFragProxy = type { %XObjectResultTreeFragProxyBase, %XObjectResultTreeFragProxyText }
%XObjectResultTreeFragProxyBase = type { %XalanDocumentFragment }
%XalanDocumentFragment = type { %XalanNode }
%XalanNode = type { i32 (...)** }
%XObjectResultTreeFragProxyText = type { %XalanText, %XObject*, %MemoryManager* }
%XalanText = type { %XalanCharacterData }
%XalanCharacterData = type { %XalanNode }
%"XPathExecutionContext::GetAndReleaseCachedString" = type { %XPathExecutionContext*, %XalanDOMString* }
%XPathExecutionContext = type { %ExecutionContext, %XObjectFactory* }
%ExecutionContext = type { i32 (...)**, %MemoryManager* }
%XalanDOMString = type <{ %XalanVector, i32, [4 x i8] }>
%XalanVector = type { %MemoryManager*, i64, i64, i16* }
%ReusableArenaAllocator = type <{ %ArenaAllocator, i8 }>
%"ReusableArenaBlock<XStringCached>::NextBlock" = type { i16, i32 }
%XStringCachedAllocator = type { %ReusableArenaAllocator }
%MemoryManagerImpl = type { %MemoryManager }
%DummyMemoryManager = type { %MemoryManager }
%"class.std::bad_alloc" = type { %"class.std::exception" }
%"class.std::exception" = type { i32 (...)**, %struct.__std_exception_data }
%struct.__std_exception_data = type { i8*, i8 }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

@_ZTSN11xercesc_2_713MemoryManagerE = internal constant [31 x i8] c"N11xercesc_2_713MemoryManagerE\00"

define internal nonnull align 8 dereferenceable(8) %MemoryManager* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%ArenaAllocator* nocapture nonnull readonly dereferenceable(40) %arg) align 2 {
bb:
  %i = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 0
  %i2 = load %MemoryManager*, %MemoryManager** %i1, align 8
  ret %MemoryManager* %i2
}

define internal %ReusableArenaAllocator* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%ReusableArenaAllocator* nocapture nonnull dereferenceable(41) %arg, %MemoryManager* nonnull align 8 dereferenceable(8) %arg1, i16 zeroext %arg2, i1 zeroext %arg3) unnamed_addr align 2 {
bb:
  %i = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0
  %i4 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 1
  store i16 10, i16* %i4, align 8
  %i5 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 2
  %i6 = getelementptr inbounds %XalanList, %XalanList* %i5, i64 0, i32 0
  store %MemoryManager* %arg1, %MemoryManager** %i6, align 8
  %i7 = getelementptr inbounds %XalanList, %XalanList* %i5, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i7, align 8
  %i8 = getelementptr inbounds %XalanList, %XalanList* %i5, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i8, align 8
  %i9 = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ([8 x i8*], [8 x i8*]* null, i32 0, i64 2) to i32 (...)**), i32 (...)*** %i9, align 8
  %i10 = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 1
  store i8 0, i8* %i10, align 8
  ret %ReusableArenaAllocator* %arg
}

define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%ArenaAllocator* nocapture nonnull dereferenceable(40) %arg) personality i32 (...)* @__CxxFrameHandler3 {
bb:
  %i = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ([8 x i8*], [8 x i8*]* null, i32 0, i64 2) to i32 (...)**), i32 (...)*** %i, align 8
  %i1 = tail call i1 @llvm.type.test(i8* bitcast (i8** getelementptr inbounds ([8 x i8*], [8 x i8*]* null, i32 0, i64 2) to i8*), metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i1)
  invoke void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%ArenaAllocator* nonnull dereferenceable(40) %arg)
          to label %bb2 unwind label %bb106

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 2
  %i4 = getelementptr inbounds %XalanList, %XalanList* %i3, i64 0, i32 1
  %i5 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i4, align 8
  %i6 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i5, null
  br i1 %i6, label %bb105, label %bb7

bb7:                                              ; preds = %bb2
  %i8 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i5, i64 0, i32 2
  %i9 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i8, align 8
  %i10 = getelementptr inbounds %XalanList, %XalanList* %i3, i64 0, i32 0
  br label %bb11

bb11:                                             ; preds = %bb55, %bb7
  %i12 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i9, %bb7 ], [ %i41, %bb55 ]
  %j12 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i4, align 8
  %i14 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %j12, null
  br i1 %i14, label %bb15, label %bb36

bb15:                                             ; preds = %bb11
  %i16 = load %MemoryManager*, %MemoryManager** %i10, align 8
  %i17 = bitcast %MemoryManager* %i16 to i8* (%MemoryManager*, i64)***
  %i18 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i17, align 8
  %i19 = bitcast i8* (%MemoryManager*, i64)** %i18 to i8*
  %i20 = tail call i1 @llvm.type.test(i8* %i19, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i20)
  %i21 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i18, i64 2
  %i22 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i21, align 8
  %i23 = bitcast i8* (%MemoryManager*, i64)* %i22 to i8*
  %i24 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i25 = icmp eq i8* %i23, %i24
  br i1 %i25, label %bb26, label %bb28

bb26:                                             ; preds = %bb15
  %i27 = invoke i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i16, i64 24)
          to label %bb30 unwind label %bb102

bb28:                                             ; preds = %bb15
  %i29 = invoke i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i16, i64 24)
          to label %bb30 unwind label %bb102

bb30:                                             ; preds = %bb28, %bb26
  %i31 = phi i8* [ %i27, %bb26 ], [ %i29, %bb28 ]
  br label %bb32

bb32:                                             ; preds = %bb30
  %i33 = bitcast i8* %i31 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i4, align 8
  %i34 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i34, align 8
  %i35 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i35, align 8
  br label %bb36

bb36:                                             ; preds = %bb32, %bb11
  %i37 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i33, %bb32 ], [ %j12, %bb11 ]
  %i38 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i12, %i37
  br i1 %i38, label %bb62, label %bb39

bb39:                                             ; preds = %bb36
  %i40 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i12, i64 0, i32 2
  %i41 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i40, align 8
  %i42 = load %MemoryManager*, %MemoryManager** %i10, align 8
  %i43 = bitcast %MemoryManager* %i42 to void (%MemoryManager*, i8*)***
  %i44 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i43, align 8
  %i45 = bitcast void (%MemoryManager*, i8*)** %i44 to i8*
  %i46 = tail call i1 @llvm.type.test(i8* %i45, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i46)
  %i47 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i12 to i8*
  %i48 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i44, i64 3
  %i49 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i48, align 8
  %i50 = bitcast void (%MemoryManager*, i8*)* %i49 to i8*
  %i51 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i52 = icmp eq i8* %i50, %i51
  br i1 %i52, label %bb53, label %bb54

bb53:                                             ; preds = %bb39
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i42, i8* nonnull %i47)
          to label %bb55 unwind label %bb102

bb54:                                             ; preds = %bb39
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i42, i8* nonnull %i47)
          to label %bb55 unwind label %bb102

bb55:                                             ; preds = %bb54, %bb53
  br label %bb11

bb62:                                             ; preds = %bb36
  %i63 = getelementptr inbounds %XalanList, %XalanList* %i3, i64 0, i32 2
  %i64 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i63, align 8
  br label %bb65

bb65:                                             ; preds = %bb84, %bb62
  %i66 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i64, %bb62 ], [ %i73, %bb84 ]
  %i67 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i66, null
  %i68 = load %MemoryManager*, %MemoryManager** %i10, align 8
  %i69 = bitcast %MemoryManager* %i68 to void (%MemoryManager*, i8*)***
  %i70 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i69, align 8
  br i1 %i67, label %bb87, label %bb71

bb71:                                             ; preds = %bb65
  %i72 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i66, i64 0, i32 2
  %i73 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i72, align 8
  %i74 = bitcast void (%MemoryManager*, i8*)** %i70 to i8*
  %i75 = tail call i1 @llvm.type.test(i8* %i74, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i75)
  %i76 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i66 to i8*
  %i77 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i70, i64 3
  %i78 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i77, align 8
  %i79 = bitcast void (%MemoryManager*, i8*)* %i78 to i8*
  %i80 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i81 = icmp eq i8* %i79, %i80
  br i1 %i81, label %bb82, label %bb83

bb82:                                             ; preds = %bb71
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i68, i8* nonnull %i76)
          to label %bb84 unwind label %bb102

bb83:                                             ; preds = %bb71
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i68, i8* nonnull %i76)
          to label %bb84 unwind label %bb102

bb84:                                             ; preds = %bb83, %bb82
  br label %bb65

bb87:                                             ; preds = %bb65
  %i88 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i4 to i8**
  %i89 = load i8*, i8** %i88, align 8
  %i90 = bitcast void (%MemoryManager*, i8*)** %i70 to i8*
  %i91 = tail call i1 @llvm.type.test(i8* %i90, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i91)
  %i92 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i70, i64 3
  %i93 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i92, align 8
  %i94 = bitcast void (%MemoryManager*, i8*)* %i93 to i8*
  %i95 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i96 = icmp eq i8* %i94, %i95
  br i1 %i96, label %bb97, label %bb98

bb97:                                             ; preds = %bb87
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i68, i8* %i89)
          to label %bb99 unwind label %bb102

bb98:                                             ; preds = %bb87
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i68, i8* %i89)
          to label %bb99 unwind label %bb102

bb99:                                             ; preds = %bb98, %bb97
  br label %bb105

bb102:                                            ; preds = %bb98, %bb97, %bb83, %bb82, %bb85, %bb53, %bb54, %bb58, %bb26, %bb28
  %i103 = cleanuppad within none []
  call void @__std_terminate() [ "funclet"(token %i103) ]
  unreachable

bb105:                                            ; preds = %bb99, %bb2
  ret void

bb106:                                            ; preds = %bb
  %i107 = cleanuppad within none []
  %i108 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 2
  %i109 = getelementptr inbounds %XalanList, %XalanList* %i108, i64 0, i32 1
  %i110 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i109, align 8
  %i111 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i110, null
  br i1 %i111, label %bb210, label %bb112

bb112:                                            ; preds = %bb106
  %i113 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i110, i64 0, i32 2
  %i114 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i113, align 8
  %i115 = getelementptr inbounds %XalanList, %XalanList* %i108, i64 0, i32 0
  br label %bb116

bb116:                                            ; preds = %bb161, %bb112
  %i117 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i110, %bb112 ], [ %i162, %bb161 ]
  %i118 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i114, %bb112 ], [ %i146, %bb161 ]
  %i119 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i117, null
  br i1 %i119, label %bb120, label %bb141

bb120:                                            ; preds = %bb116
  %i121 = load %MemoryManager*, %MemoryManager** %i115, align 8
  %i122 = bitcast %MemoryManager* %i121 to i8* (%MemoryManager*, i64)***
  %i123 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i122, align 8
  %i124 = bitcast i8* (%MemoryManager*, i64)** %i123 to i8*
  %i125 = tail call i1 @llvm.type.test(i8* %i124, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i125)
  %i126 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i123, i64 2
  %i127 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i126, align 8
  %i128 = bitcast i8* (%MemoryManager*, i64)* %i127 to i8*
  %i129 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i130 = icmp eq i8* %i128, %i129
  br i1 %i130, label %bb131, label %bb133

bb131:                                            ; preds = %bb120
  %i132 = invoke i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i121, i64 24) [ "funclet"(token %i107) ]
          to label %bb135 unwind label %bb207

bb133:                                            ; preds = %bb120
  %i134 = invoke i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i121, i64 24) [ "funclet"(token %i107) ]
          to label %bb135 unwind label %bb207

bb135:                                            ; preds = %bb133, %bb131
  %i136 = phi i8* [ %i132, %bb131 ], [ %i134, %bb133 ]
  br label %bb137

bb137:                                            ; preds = %bb135
  %i138 = bitcast i8* %i136 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i138, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i109, align 8
  %i139 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i138, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i138, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i139, align 8
  %i140 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i138, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i138, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i140, align 8
  br label %bb141

bb141:                                            ; preds = %bb137, %bb116
  %i142 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i138, %bb137 ], [ %i117, %bb116 ]
  %i143 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i118, %i142
  br i1 %i143, label %bb167, label %bb144

bb144:                                            ; preds = %bb141
  %i145 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i118, i64 0, i32 2
  %i146 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i145, align 8
  %i147 = load %MemoryManager*, %MemoryManager** %i115, align 8
  %i148 = bitcast %MemoryManager* %i147 to void (%MemoryManager*, i8*)***
  %i149 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i148, align 8
  %i150 = bitcast void (%MemoryManager*, i8*)** %i149 to i8*
  %i151 = tail call i1 @llvm.type.test(i8* %i150, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i151)
  %i152 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i118 to i8*
  %i153 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i149, i64 3
  %i154 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i153, align 8
  %i155 = bitcast void (%MemoryManager*, i8*)* %i154 to i8*
  %i156 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i157 = icmp eq i8* %i155, %i156
  br i1 %i157, label %bb158, label %bb159

bb158:                                            ; preds = %bb144
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i147, i8* nonnull %i152) [ "funclet"(token %i107) ]
          to label %bb160 unwind label %bb207

bb159:                                            ; preds = %bb144
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i147, i8* nonnull %i152) [ "funclet"(token %i107) ]
          to label %bb160 unwind label %bb207

bb160:                                            ; preds = %bb159, %bb158
  br label %bb161

bb161:                                            ; preds = %bb160
  %i162 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i109, align 8
  br label %bb116

bb167:                                            ; preds = %bb141
  %i168 = getelementptr inbounds %XalanList, %XalanList* %i108, i64 0, i32 2
  %i169 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i168, align 8
  br label %bb170

bb170:                                            ; preds = %bb189, %bb167
  %i171 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i169, %bb167 ], [ %i178, %bb189 ]
  %i172 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i171, null
  %i173 = load %MemoryManager*, %MemoryManager** %i115, align 8
  %i174 = bitcast %MemoryManager* %i173 to void (%MemoryManager*, i8*)***
  %i175 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i174, align 8
  br i1 %i172, label %bb192, label %bb176

bb176:                                            ; preds = %bb170
  %i177 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i171, i64 0, i32 2
  %i178 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i177, align 8
  %i179 = bitcast void (%MemoryManager*, i8*)** %i175 to i8*
  %i180 = tail call i1 @llvm.type.test(i8* %i179, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i180)
  %i181 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i171 to i8*
  %i182 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i175, i64 3
  %i183 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i182, align 8
  %i184 = bitcast void (%MemoryManager*, i8*)* %i183 to i8*
  %i185 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i186 = icmp eq i8* %i184, %i185
  br i1 %i186, label %bb187, label %bb188

bb187:                                            ; preds = %bb176
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i173, i8* nonnull %i181) [ "funclet"(token %i107) ]
          to label %bb189 unwind label %bb207

bb188:                                            ; preds = %bb176
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i173, i8* nonnull %i181) [ "funclet"(token %i107) ]
          to label %bb189 unwind label %bb207

bb189:                                            ; preds = %bb188, %bb187
  br label %bb170

bb192:                                            ; preds = %bb170
  %i193 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i109 to i8**
  %i194 = load i8*, i8** %i193, align 8
  %i195 = bitcast void (%MemoryManager*, i8*)** %i175 to i8*
  %i196 = tail call i1 @llvm.type.test(i8* %i195, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i196)
  %i197 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i175, i64 3
  %i198 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i197, align 8
  %i199 = bitcast void (%MemoryManager*, i8*)* %i198 to i8*
  %i200 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i201 = icmp eq i8* %i199, %i200
  br i1 %i201, label %bb202, label %bb203

bb202:                                            ; preds = %bb192
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i173, i8* %i194) [ "funclet"(token %i107) ]
          to label %bb204 unwind label %bb207

bb203:                                            ; preds = %bb192
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i173, i8* %i194) [ "funclet"(token %i107) ]
          to label %bb204 unwind label %bb207

bb204:                                            ; preds = %bb203, %bb202
  br label %bb210

bb207:                                            ; preds = %bb203, %bb202, %bb190, %bb165, %bb163
  %i209 = cleanuppad within %i107 []
  call void @__std_terminate() [ "funclet"(token %i209) ]
  unreachable

bb210:                                            ; preds = %bb204, %bb106
  call void @__std_terminate() [ "funclet"(token %i107) ]
  unreachable
}

define internal zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%ReusableArenaAllocator* nocapture nonnull dereferenceable(41) %arg, %XStringCached* %arg1) align 2 {
bb:
  %i = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0
  %i2 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 2
  %i3 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 1
  %i4 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i5 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i4, null
  br i1 %i5, label %bb6, label %bb28

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i8 = load %MemoryManager*, %MemoryManager** %i7, align 8
  %i9 = bitcast %MemoryManager* %i8 to i8* (%MemoryManager*, i64)***
  %i10 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i9, align 8
  %i11 = bitcast i8* (%MemoryManager*, i64)** %i10 to i8*
  %i12 = tail call i1 @llvm.type.test(i8* %i11, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i12)
  %i13 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i10, i64 2
  %i14 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i13, align 8
  %i15 = bitcast i8* (%MemoryManager*, i64)* %i14 to i8*
  %i16 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i17 = icmp eq i8* %i15, %i16
  br i1 %i17, label %bb18, label %bb20

bb18:                                             ; preds = %bb6
  %i19 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i8, i64 24)
  br label %bb22

bb20:                                             ; preds = %bb6
  %i21 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i8, i64 24)
  br label %bb22

bb22:                                             ; preds = %bb20, %bb18
  %i23 = phi i8* [ %i19, %bb18 ], [ %i21, %bb20 ]
  br label %bb24

bb24:                                             ; preds = %bb22
  %i25 = bitcast i8* %i23 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i26 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i25, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i26, align 8
  %i27 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i25, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i27, align 8
  br label %bb506

bb28:                                             ; preds = %bb
  %i29 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i4, i64 0, i32 2
  %i30 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i29, align 8
  %i31 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i30, %i4
  br i1 %i31, label %bb506, label %bb32

bb32:                                             ; preds = %bb248, %bb28
  %i33 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i250, %bb248 ], [ %i30, %bb28 ]
  %i34 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33 to %ArenaBlockBase**
  %i35 = load %ArenaBlockBase*, %ArenaBlockBase** %i34, align 8
  %i36 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i35, i64 0, i32 1
  %i37 = load i16, i16* %i36, align 8
  %i38 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i35, i64 0, i32 2
  %i39 = load i16, i16* %i38, align 2
  %i40 = icmp ult i16 %i37, %i39
  br i1 %i40, label %bb41, label %bb252

bb41:                                             ; preds = %bb32
  %i42 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i35, i64 0, i32 3
  %i43 = load %XStringCached*, %XStringCached** %i42, align 8
  %i44 = icmp ugt %XStringCached* %i43, %arg1
  br i1 %i44, label %bb248, label %bb45

bb45:                                             ; preds = %bb41
  %i46 = zext i16 %i39 to i64
  %i47 = getelementptr inbounds %XStringCached, %XStringCached* %i43, i64 %i46
  %i48 = icmp ugt %XStringCached* %i47, %arg1
  br i1 %i48, label %bb49, label %bb248

bb49:                                             ; preds = %bb45
  %i50 = bitcast %ArenaBlockBase* %i35 to %ReusableArenaBlock*
  %i51 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 0
  %i52 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i50, i64 0, i32 1
  %i53 = load i16, i16* %i52, align 8
  %i54 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i50, i64 0, i32 2
  %i55 = load i16, i16* %i54, align 2
  %i56 = icmp eq i16 %i53, %i55
  br i1 %i56, label %bb63, label %bb57

bb57:                                             ; preds = %bb49
  %i58 = zext i16 %i53 to i64
  %i59 = getelementptr inbounds %XStringCached, %XStringCached* %i43, i64 %i58
  %i60 = bitcast %XStringCached* %i59 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i61 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i60, i64 0, i32 0
  store i16 %i55, i16* %i61, align 4
  %i62 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i60, i64 0, i32 1
  store i32 -2228259, i32* %i62, align 4
  store i16 %i53, i16* %i54, align 2
  br label %bb63

bb63:                                             ; preds = %bb57, %bb49
  %i64 = bitcast %XStringCached* %arg1 to void (%XStringCached*)***
  %i65 = load void (%XStringCached*)**, void (%XStringCached*)*** %i64, align 8
  %i66 = bitcast void (%XStringCached*)** %i65 to i8*
  %i67 = tail call i1 @llvm.type.test(i8* %i66, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %i67)
  %i68 = load void (%XStringCached*)*, void (%XStringCached*)** %i65, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%XStringCached* nonnull dereferenceable(80) %arg1)
  %i69 = bitcast %XStringCached* %arg1 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i70 = load i16, i16* %i52, align 8
  %i71 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i69, i64 0, i32 0
  store i16 %i70, i16* %i71, align 4
  %i72 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i69, i64 0, i32 1
  store i32 -2228259, i32* %i72, align 4
  %i73 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i50, i64 0, i32 0
  %i74 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i73, i64 0, i32 3
  %i75 = load %XStringCached*, %XStringCached** %i74, align 8
  %i76 = ptrtoint %XStringCached* %arg1 to i64
  %i77 = ptrtoint %XStringCached* %i75 to i64
  %i78 = sub i64 %i76, %i77
  %i79 = sdiv exact i64 %i78, 80
  %i80 = trunc i64 %i79 to i16
  store i16 %i80, i16* %i54, align 2
  store i16 %i80, i16* %i52, align 8
  %i81 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i73, i64 0, i32 1
  %i82 = load i16, i16* %i81, align 8
  %i83 = add i16 %i82, -1
  store i16 %i83, i16* %i81, align 8
  %i84 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i85 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i84, null
  br i1 %i85, label %bb89, label %bb86

bb86:                                             ; preds = %bb63
  %i87 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i84, i64 0, i32 2
  %i88 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i87, align 8
  br label %bb111

bb89:                                             ; preds = %bb63
  %i90 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i91 = load %MemoryManager*, %MemoryManager** %i90, align 8
  %i92 = bitcast %MemoryManager* %i91 to i8* (%MemoryManager*, i64)***
  %i93 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i92, align 8
  %i94 = bitcast i8* (%MemoryManager*, i64)** %i93 to i8*
  %i95 = tail call i1 @llvm.type.test(i8* %i94, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i95)
  %i96 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i93, i64 2
  %i97 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i96, align 8
  %i98 = bitcast i8* (%MemoryManager*, i64)* %i97 to i8*
  %i99 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i100 = icmp eq i8* %i98, %i99
  br i1 %i100, label %bb101, label %bb103

bb101:                                            ; preds = %bb89
  %i102 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i91, i64 24)
  br label %bb105

bb103:                                            ; preds = %bb89
  %i104 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i91, i64 24)
  br label %bb105

bb105:                                            ; preds = %bb103, %bb101
  %i106 = phi i8* [ %i102, %bb101 ], [ %i104, %bb103 ]
  br label %bb107

bb107:                                            ; preds = %bb105
  %i108 = bitcast i8* %i106 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i108, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i109 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i108, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i108, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i109, align 8
  %i110 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i108, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i108, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i110, align 8
  br label %bb111

bb111:                                            ; preds = %bb107, %bb86
  %i112 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i108, %bb107 ], [ %i88, %bb86 ]
  %i113 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, %i112
  br i1 %i113, label %bb189, label %bb114

bb114:                                            ; preds = %bb111
  %i115 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i51, align 8
  %i116 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 2
  %i117 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i116, align 8
  %i118 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 1
  %i119 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i118, align 8
  %i120 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i119, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i117, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i120, align 8
  %i121 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i116, align 8
  %i122 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i121, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i119, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i122, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i118, align 8
  %i123 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 2
  %i124 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i123, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i124, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i116, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i123, align 8
  %i125 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i126 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i125, null
  br i1 %i126, label %bb130, label %bb127

bb127:                                            ; preds = %bb114
  %i128 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i125, i64 0, i32 2
  %i129 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i128, align 8
  br label %bb154

bb130:                                            ; preds = %bb114
  %i131 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i132 = load %MemoryManager*, %MemoryManager** %i131, align 8
  %i133 = bitcast %MemoryManager* %i132 to i8* (%MemoryManager*, i64)***
  %i134 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i133, align 8
  %i135 = bitcast i8* (%MemoryManager*, i64)** %i134 to i8*
  %i136 = tail call i1 @llvm.type.test(i8* %i135, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i136)
  %i137 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i134, i64 2
  %i138 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i137, align 8
  %i139 = bitcast i8* (%MemoryManager*, i64)* %i138 to i8*
  %i140 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i141 = icmp eq i8* %i139, %i140
  br i1 %i141, label %bb142, label %bb144

bb142:                                            ; preds = %bb130
  %i143 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i132, i64 24)
  br label %bb146

bb144:                                            ; preds = %bb130
  %i145 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i132, i64 24)
  br label %bb146

bb146:                                            ; preds = %bb144, %bb142
  %i147 = phi i8* [ %i143, %bb142 ], [ %i145, %bb144 ]
  br label %bb148

bb148:                                            ; preds = %bb146
  %i149 = bitcast i8* %i147 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i150 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i149, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i150, align 8
  %i151 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i149, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i151, align 8
  %i152 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i123, align 8
  %i153 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i152, null
  br i1 %i153, label %bb159, label %bb154

bb154:                                            ; preds = %bb148, %bb127
  %i155 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i129, %bb127 ], [ %i149, %bb148 ]
  %i156 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i33, %bb127 ], [ %i152, %bb148 ]
  %i157 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i156, i64 0, i32 2
  %i158 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i157, align 8
  br label %bb178

bb159:                                            ; preds = %bb148
  %i160 = load %MemoryManager*, %MemoryManager** %i131, align 8
  %i161 = bitcast %MemoryManager* %i160 to i8* (%MemoryManager*, i64)***
  %i162 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i161, align 8
  %i163 = bitcast i8* (%MemoryManager*, i64)** %i162 to i8*
  %i164 = tail call i1 @llvm.type.test(i8* %i163, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i164)
  %i165 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i162, i64 2
  %i166 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i165, align 8
  %i167 = bitcast i8* (%MemoryManager*, i64)* %i166 to i8*
  %i168 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i169 = icmp eq i8* %i167, %i168
  br i1 %i169, label %bb170, label %bb172

bb170:                                            ; preds = %bb159
  %i171 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i160, i64 24)
  br label %bb174

bb172:                                            ; preds = %bb159
  %i173 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i160, i64 24)
  br label %bb174

bb174:                                            ; preds = %bb172, %bb170
  %i175 = phi i8* [ %i171, %bb170 ], [ %i173, %bb172 ]
  br label %bb176

bb176:                                            ; preds = %bb174
  %i177 = bitcast i8* %i175 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %bb178

bb178:                                            ; preds = %bb176, %bb154
  %i179 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i155, %bb154 ], [ %i149, %bb176 ]
  %i180 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i156, %bb154 ], [ %i177, %bb176 ]
  %i181 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i158, %bb154 ], [ null, %bb176 ]
  %i182 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i180, i64 0, i32 0
  store %ReusableArenaBlock* %i115, %ReusableArenaBlock** %i182, align 8
  %i183 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i180, i64 0, i32 1
  %i184 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i179, i64 0, i32 1
  %i185 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i184, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i185, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i183, align 8
  %i186 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i180, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i179, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i186, align 8
  %i187 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i184, align 8
  %i188 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i187, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i180, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i188, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i180, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i184, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i181, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i123, align 8
  br label %bb189

bb189:                                            ; preds = %bb178, %bb111
  %i190 = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 1
  %i191 = load i8, i8* %i190, align 8
  %i192 = icmp eq i8 %i191, 0
  br i1 %i192, label %bb252, label %bb193

bb193:                                            ; preds = %bb189
  %i194 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i195 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i194, null
  br i1 %i195, label %bb196, label %bb218

bb196:                                            ; preds = %bb193
  %i197 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i198 = load %MemoryManager*, %MemoryManager** %i197, align 8
  %i199 = bitcast %MemoryManager* %i198 to i8* (%MemoryManager*, i64)***
  %i200 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i199, align 8
  %i201 = bitcast i8* (%MemoryManager*, i64)** %i200 to i8*
  %i202 = tail call i1 @llvm.type.test(i8* %i201, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i202)
  %i203 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i200, i64 2
  %i204 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i203, align 8
  %i205 = bitcast i8* (%MemoryManager*, i64)* %i204 to i8*
  %i206 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i207 = icmp eq i8* %i205, %i206
  br i1 %i207, label %bb208, label %bb210

bb208:                                            ; preds = %bb196
  %i209 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i198, i64 24)
  br label %bb212

bb210:                                            ; preds = %bb196
  %i211 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i198, i64 24)
  br label %bb212

bb212:                                            ; preds = %bb210, %bb208
  %i213 = phi i8* [ %i209, %bb208 ], [ %i211, %bb210 ]
  br label %bb214

bb214:                                            ; preds = %bb212
  %i215 = bitcast i8* %i213 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i215, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i216 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i215, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i215, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i216, align 8
  %i217 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i215, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i215, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i217, align 8
  br label %bb502

bb218:                                            ; preds = %bb193
  %i219 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i194, i64 0, i32 2
  %i220 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i219, align 8
  %i221 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i220, %i194
  br i1 %i221, label %bb252, label %bb222

bb222:                                            ; preds = %bb218
  %i223 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i220 to %ArenaBlockBase**
  %i224 = load %ArenaBlockBase*, %ArenaBlockBase** %i223, align 8
  %i225 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i224, i64 0, i32 1
  %i226 = load i16, i16* %i225, align 8
  %i227 = icmp eq i16 %i226, 0
  br i1 %i227, label %bb228, label %bb252

bb228:                                            ; preds = %bb222
  %i229 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i220, i64 0, i32 2
  %i230 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i229, align 8
  %i231 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i230, %i194
  br i1 %i231, label %bb240, label %bb232

bb232:                                            ; preds = %bb228
  %i233 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i230 to %ArenaBlockBase**
  %i234 = load %ArenaBlockBase*, %ArenaBlockBase** %i233, align 8
  %i235 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i234, i64 0, i32 1
  %i236 = load i16, i16* %i235, align 8
  %i237 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i234, i64 0, i32 2
  %i238 = load i16, i16* %i237, align 2
  %i239 = icmp ult i16 %i236, %i238
  br i1 %i239, label %bb240, label %bb252

bb240:                                            ; preds = %bb232, %bb228
  %i241 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i220, i64 0, i32 1
  %i242 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i241, align 8
  %i243 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 2
  %i244 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i243, align 8
  %i245 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i242, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i230, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i245, align 8
  %i246 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i229, align 8
  %i247 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i246, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i242, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i247, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i241, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i244, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i229, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i220, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i243, align 8
  br label %bb252

bb248:                                            ; preds = %bb45, %bb41
  %i249 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 2
  %i250 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i249, align 8
  %i251 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i250, %i4
  br i1 %i251, label %bb252, label %bb32

bb252:                                            ; preds = %bb248, %bb240, %bb232, %bb222, %bb218, %bb189, %bb32
  %i253 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i33, %bb189 ], [ %i33, %bb218 ], [ %i33, %bb222 ], [ %i33, %bb232 ], [ %i33, %bb240 ], [ %i4, %bb248 ], [ %i33, %bb32 ]
  %i254 = phi i1 [ false, %bb189 ], [ false, %bb218 ], [ false, %bb222 ], [ false, %bb232 ], [ false, %bb240 ], [ true, %bb248 ], [ true, %bb32 ]
  %i255 = phi i8 [ 1, %bb189 ], [ 1, %bb218 ], [ 1, %bb222 ], [ 1, %bb232 ], [ 1, %bb240 ], [ 0, %bb248 ], [ 0, %bb32 ]
  %i256 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i257 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i256, null
  br i1 %i257, label %bb261, label %bb258

bb258:                                            ; preds = %bb252
  %i259 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i256, i64 0, i32 2
  %i260 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i259, align 8
  br i1 %i254, label %bb283, label %bb502

bb261:                                            ; preds = %bb252
  %i262 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i263 = load %MemoryManager*, %MemoryManager** %i262, align 8
  %i264 = bitcast %MemoryManager* %i263 to i8* (%MemoryManager*, i64)***
  %i265 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i264, align 8
  %i266 = bitcast i8* (%MemoryManager*, i64)** %i265 to i8*
  %i267 = tail call i1 @llvm.type.test(i8* %i266, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i267)
  %i268 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i265, i64 2
  %i269 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i268, align 8
  %i270 = bitcast i8* (%MemoryManager*, i64)* %i269 to i8*
  %i271 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i272 = icmp eq i8* %i270, %i271
  br i1 %i272, label %bb273, label %bb275

bb273:                                            ; preds = %bb261
  %i274 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i263, i64 24)
  br label %bb277

bb275:                                            ; preds = %bb261
  %i276 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i263, i64 24)
  br label %bb277

bb277:                                            ; preds = %bb275, %bb273
  %i278 = phi i8* [ %i274, %bb273 ], [ %i276, %bb275 ]
  br label %bb279

bb279:                                            ; preds = %bb277
  %i280 = bitcast i8* %i278 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i280, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i281 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i280, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i280, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i281, align 8
  %i282 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i280, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i280, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i282, align 8
  br i1 %i254, label %bb283, label %bb502

bb283:                                            ; preds = %bb279, %bb258
  %i284 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i280, %bb279 ], [ %i256, %bb258 ]
  %i285 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i280, %bb279 ], [ %i260, %bb258 ]
  %i286 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i253, i64 0, i32 0
  br label %bb287

bb287:                                            ; preds = %bb305, %bb283
  %i288 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i292, %bb305 ], [ %i284, %bb283 ]
  %i289 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i288, %i285
  br i1 %i289, label %bb502, label %bb290

bb290:                                            ; preds = %bb287
  %i291 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i288, i64 0, i32 1
  %i292 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i291, align 8
  %i293 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i292 to %ArenaBlockBase**
  %i294 = load %ArenaBlockBase*, %ArenaBlockBase** %i293, align 8
  %i295 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i294, i64 0, i32 3
  %i296 = load %XStringCached*, %XStringCached** %i295, align 8
  %i297 = icmp ugt %XStringCached* %i296, %arg1
  %i298 = bitcast %ArenaBlockBase* %i294 to %ReusableArenaBlock*
  br i1 %i297, label %bb305, label %bb299

bb299:                                            ; preds = %bb290
  %i300 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i294, i64 0, i32 2
  %i301 = load i16, i16* %i300, align 2
  %i302 = zext i16 %i301 to i64
  %i303 = getelementptr inbounds %XStringCached, %XStringCached* %i296, i64 %i302
  %i304 = icmp ugt %XStringCached* %i303, %arg1
  br i1 %i304, label %bb308, label %bb305

bb305:                                            ; preds = %bb299, %bb290
  %i306 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i286, align 8
  %i307 = icmp eq %ReusableArenaBlock* %i306, %i298
  br i1 %i307, label %bb502, label %bb287

bb308:                                            ; preds = %bb299
  %i309 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i298, i64 0, i32 1
  %i310 = load i16, i16* %i309, align 8
  %i311 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i298, i64 0, i32 2
  %i312 = load i16, i16* %i311, align 2
  %i313 = icmp eq i16 %i310, %i312
  br i1 %i313, label %bb320, label %bb314

bb314:                                            ; preds = %bb308
  %i315 = zext i16 %i310 to i64
  %i316 = getelementptr inbounds %XStringCached, %XStringCached* %i296, i64 %i315
  %i317 = bitcast %XStringCached* %i316 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i318 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i317, i64 0, i32 0
  store i16 %i312, i16* %i318, align 4
  %i319 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i317, i64 0, i32 1
  store i32 -2228259, i32* %i319, align 4
  store i16 %i310, i16* %i311, align 2
  br label %bb320

bb320:                                            ; preds = %bb314, %bb308
  %i321 = bitcast %XStringCached* %arg1 to void (%XStringCached*)***
  %i322 = load void (%XStringCached*)**, void (%XStringCached*)*** %i321, align 8
  %i323 = bitcast void (%XStringCached*)** %i322 to i8*
  %i324 = tail call i1 @llvm.type.test(i8* %i323, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %i324)
  %i325 = load void (%XStringCached*)*, void (%XStringCached*)** %i322, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%XStringCached* nonnull dereferenceable(80) %arg1)
  %i326 = bitcast %XStringCached* %arg1 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i327 = load i16, i16* %i309, align 8
  %i328 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i326, i64 0, i32 0
  store i16 %i327, i16* %i328, align 4
  %i329 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i326, i64 0, i32 1
  store i32 -2228259, i32* %i329, align 4
  %i330 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i298, i64 0, i32 0
  %i331 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i330, i64 0, i32 3
  %i332 = load %XStringCached*, %XStringCached** %i331, align 8
  %i333 = ptrtoint %XStringCached* %arg1 to i64
  %i334 = ptrtoint %XStringCached* %i332 to i64
  %i335 = sub i64 %i333, %i334
  %i336 = sdiv exact i64 %i335, 80
  %i337 = trunc i64 %i336 to i16
  store i16 %i337, i16* %i311, align 2
  store i16 %i337, i16* %i309, align 8
  %i338 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i330, i64 0, i32 1
  %i339 = load i16, i16* %i338, align 8
  %i340 = add i16 %i339, -1
  store i16 %i340, i16* %i338, align 8
  %i341 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i342 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i341, null
  br i1 %i342, label %bb343, label %bb365

bb343:                                            ; preds = %bb320
  %i344 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i345 = load %MemoryManager*, %MemoryManager** %i344, align 8
  %i346 = bitcast %MemoryManager* %i345 to i8* (%MemoryManager*, i64)***
  %i347 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i346, align 8
  %i348 = bitcast i8* (%MemoryManager*, i64)** %i347 to i8*
  %i349 = tail call i1 @llvm.type.test(i8* %i348, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i349)
  %i350 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i347, i64 2
  %i351 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i350, align 8
  %i352 = bitcast i8* (%MemoryManager*, i64)* %i351 to i8*
  %i353 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i354 = icmp eq i8* %i352, %i353
  br i1 %i354, label %bb355, label %bb357

bb355:                                            ; preds = %bb343
  %i356 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i345, i64 24)
  br label %bb359

bb357:                                            ; preds = %bb343
  %i358 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i345, i64 24)
  br label %bb359

bb359:                                            ; preds = %bb357, %bb355
  %i360 = phi i8* [ %i356, %bb355 ], [ %i358, %bb357 ]
  br label %bb361

bb361:                                            ; preds = %bb359
  %i362 = bitcast i8* %i360 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i362, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i363 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i362, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i362, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i363, align 8
  %i364 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i362, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i362, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i364, align 8
  br label %bb365

bb365:                                            ; preds = %bb361, %bb320
  %i366 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i362, %bb361 ], [ %i341, %bb320 ]
  %i367 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i288, %i366
  br i1 %i367, label %bb443, label %bb368

bb368:                                            ; preds = %bb365
  %i369 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i286, align 8
  %i370 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i253, i64 0, i32 2
  %i371 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i370, align 8
  %i372 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i253, i64 0, i32 1
  %i373 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i372, align 8
  %i374 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i373, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i371, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i374, align 8
  %i375 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i370, align 8
  %i376 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i375, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i373, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i376, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i372, align 8
  %i377 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 2
  %i378 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i377, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i378, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i370, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i253, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i377, align 8
  %i379 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i380 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i379, null
  br i1 %i380, label %bb384, label %bb381

bb381:                                            ; preds = %bb368
  %i382 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i379, i64 0, i32 2
  %i383 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i382, align 8
  br label %bb408

bb384:                                            ; preds = %bb368
  %i385 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i386 = load %MemoryManager*, %MemoryManager** %i385, align 8
  %i387 = bitcast %MemoryManager* %i386 to i8* (%MemoryManager*, i64)***
  %i388 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i387, align 8
  %i389 = bitcast i8* (%MemoryManager*, i64)** %i388 to i8*
  %i390 = tail call i1 @llvm.type.test(i8* %i389, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i390)
  %i391 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i388, i64 2
  %i392 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i391, align 8
  %i393 = bitcast i8* (%MemoryManager*, i64)* %i392 to i8*
  %i394 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i395 = icmp eq i8* %i393, %i394
  br i1 %i395, label %bb396, label %bb398

bb396:                                            ; preds = %bb384
  %i397 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i386, i64 24)
  br label %bb400

bb398:                                            ; preds = %bb384
  %i399 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i386, i64 24)
  br label %bb400

bb400:                                            ; preds = %bb398, %bb396
  %i401 = phi i8* [ %i397, %bb396 ], [ %i399, %bb398 ]
  br label %bb402

bb402:                                            ; preds = %bb400
  %i403 = bitcast i8* %i401 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i403, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i404 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i403, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i403, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i404, align 8
  %i405 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i403, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i403, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i405, align 8
  %i406 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i377, align 8
  %i407 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i406, null
  br i1 %i407, label %bb413, label %bb408

bb408:                                            ; preds = %bb402, %bb381
  %i409 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i383, %bb381 ], [ %i403, %bb402 ]
  %i410 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i253, %bb381 ], [ %i406, %bb402 ]
  %i411 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i410, i64 0, i32 2
  %i412 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i411, align 8
  br label %bb432

bb413:                                            ; preds = %bb402
  %i414 = load %MemoryManager*, %MemoryManager** %i385, align 8
  %i415 = bitcast %MemoryManager* %i414 to i8* (%MemoryManager*, i64)***
  %i416 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i415, align 8
  %i417 = bitcast i8* (%MemoryManager*, i64)** %i416 to i8*
  %i418 = tail call i1 @llvm.type.test(i8* %i417, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i418)
  %i419 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i416, i64 2
  %i420 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i419, align 8
  %i421 = bitcast i8* (%MemoryManager*, i64)* %i420 to i8*
  %i422 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i423 = icmp eq i8* %i421, %i422
  br i1 %i423, label %bb424, label %bb426

bb424:                                            ; preds = %bb413
  %i425 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i414, i64 24)
  br label %bb428

bb426:                                            ; preds = %bb413
  %i427 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i414, i64 24)
  br label %bb428

bb428:                                            ; preds = %bb426, %bb424
  %i429 = phi i8* [ %i425, %bb424 ], [ %i427, %bb426 ]
  br label %bb430

bb430:                                            ; preds = %bb428
  %i431 = bitcast i8* %i429 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %bb432

bb432:                                            ; preds = %bb430, %bb408
  %i433 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i409, %bb408 ], [ %i403, %bb430 ]
  %i434 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i410, %bb408 ], [ %i431, %bb430 ]
  %i435 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i412, %bb408 ], [ null, %bb430 ]
  %i436 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i434, i64 0, i32 0
  store %ReusableArenaBlock* %i369, %ReusableArenaBlock** %i436, align 8
  %i437 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i434, i64 0, i32 1
  %i438 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i433, i64 0, i32 1
  %i439 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i438, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i439, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i437, align 8
  %i440 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i434, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i433, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i440, align 8
  %i441 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i438, align 8
  %i442 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i441, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i434, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i442, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i434, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i438, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i435, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i377, align 8
  br label %bb443

bb443:                                            ; preds = %bb432, %bb365
  %i444 = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 1
  %i445 = load i8, i8* %i444, align 8
  %i446 = icmp eq i8 %i445, 0
  br i1 %i446, label %bb502, label %bb447

bb447:                                            ; preds = %bb443
  %i448 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i449 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i448, null
  br i1 %i449, label %bb450, label %bb472

bb450:                                            ; preds = %bb447
  %i451 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i452 = load %MemoryManager*, %MemoryManager** %i451, align 8
  %i453 = bitcast %MemoryManager* %i452 to i8* (%MemoryManager*, i64)***
  %i454 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i453, align 8
  %i455 = bitcast i8* (%MemoryManager*, i64)** %i454 to i8*
  %i456 = tail call i1 @llvm.type.test(i8* %i455, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i456)
  %i457 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i454, i64 2
  %i458 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i457, align 8
  %i459 = bitcast i8* (%MemoryManager*, i64)* %i458 to i8*
  %i460 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i461 = icmp eq i8* %i459, %i460
  br i1 %i461, label %bb462, label %bb464

bb462:                                            ; preds = %bb450
  %i463 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i452, i64 24)
  br label %bb466

bb464:                                            ; preds = %bb450
  %i465 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i452, i64 24)
  br label %bb466

bb466:                                            ; preds = %bb464, %bb462
  %i467 = phi i8* [ %i463, %bb462 ], [ %i465, %bb464 ]
  br label %bb468

bb468:                                            ; preds = %bb466
  %i469 = bitcast i8* %i467 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i469, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i470 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i469, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i469, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i470, align 8
  %i471 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i469, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i469, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i471, align 8
  br label %bb502

bb472:                                            ; preds = %bb447
  %i473 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i448, i64 0, i32 2
  %i474 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i473, align 8
  %i475 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i474, %i448
  br i1 %i475, label %bb502, label %bb476

bb476:                                            ; preds = %bb472
  %i477 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i474 to %ArenaBlockBase**
  %i478 = load %ArenaBlockBase*, %ArenaBlockBase** %i477, align 8
  %i479 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i478, i64 0, i32 1
  %i480 = load i16, i16* %i479, align 8
  %i481 = icmp eq i16 %i480, 0
  br i1 %i481, label %bb482, label %bb502

bb482:                                            ; preds = %bb476
  %i483 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i474, i64 0, i32 2
  %i484 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i483, align 8
  %i485 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i484, %i448
  br i1 %i485, label %bb494, label %bb486

bb486:                                            ; preds = %bb482
  %i487 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i484 to %ArenaBlockBase**
  %i488 = load %ArenaBlockBase*, %ArenaBlockBase** %i487, align 8
  %i489 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i488, i64 0, i32 1
  %i490 = load i16, i16* %i489, align 8
  %i491 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i488, i64 0, i32 2
  %i492 = load i16, i16* %i491, align 2
  %i493 = icmp ult i16 %i490, %i492
  br i1 %i493, label %bb494, label %bb502

bb494:                                            ; preds = %bb486, %bb482
  %i495 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i474, i64 0, i32 1
  %i496 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i495, align 8
  %i497 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 2
  %i498 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i497, align 8
  %i499 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i496, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i484, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i499, align 8
  %i500 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i483, align 8
  %i501 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i500, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i496, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i501, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i495, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i498, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i483, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i474, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i497, align 8
  br label %bb502

bb502:                                            ; preds = %bb494, %bb486, %bb476, %bb472, %bb468, %bb443, %bb305, %bb287, %bb279, %bb258, %bb214
  %i503 = phi i8 [ 1, %bb443 ], [ 1, %bb468 ], [ 1, %bb472 ], [ 1, %bb476 ], [ 1, %bb486 ], [ 1, %bb494 ], [ %i255, %bb279 ], [ %i255, %bb258 ], [ %i255, %bb287 ], [ %i255, %bb305 ], [ 1, %bb214 ]
  %i504 = and i8 %i503, 1
  %i505 = icmp ne i8 %i504, 0
  br label %bb506

bb506:                                            ; preds = %bb502, %bb28, %bb24
  %i507 = phi i1 [ %i505, %bb502 ], [ false, %bb28 ], [ false, %bb24 ]
  ret i1 %i507
}

define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%ArenaAllocator* nocapture nonnull dereferenceable(40) %arg) personality i32 (...)* @__CxxFrameHandler3 {
bb:
  %i = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 1
  %i2 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i1, align 8
  %i3 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i2, null
  br i1 %i3, label %bb4, label %bb26

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 0
  %i6 = load %MemoryManager*, %MemoryManager** %i5, align 8
  %i7 = bitcast %MemoryManager* %i6 to i8* (%MemoryManager*, i64)***
  %i8 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i7, align 8
  %i9 = bitcast i8* (%MemoryManager*, i64)** %i8 to i8*
  %i10 = tail call i1 @llvm.type.test(i8* %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i8, i64 2
  %i12 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i11, align 8
  %i13 = bitcast i8* (%MemoryManager*, i64)* %i12 to i8*
  %i14 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i15 = icmp eq i8* %i13, %i14
  br i1 %i15, label %bb16, label %bb18

bb16:                                             ; preds = %bb4
  %i17 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i6, i64 24)
  br label %bb20

bb18:                                             ; preds = %bb4
  %i19 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i6, i64 24)
  br label %bb20

bb20:                                             ; preds = %bb18, %bb16
  %i21 = phi i8* [ %i17, %bb16 ], [ %i19, %bb18 ]
  br label %bb22

bb22:                                             ; preds = %bb20
  %i23 = bitcast i8* %i21 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i23, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i1, align 8
  %i24 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i23, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i23, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i24, align 8
  %i25 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i23, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i23, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i25, align 8
  br label %bb150

bb26:                                             ; preds = %bb
  %i27 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i2, i64 0, i32 2
  %i28 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i27, align 8
  %i29 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 0
  %i30 = load %MemoryManager*, %MemoryManager** %i29, align 8
  %i33 = bitcast %MemoryManager* %i30 to void (%MemoryManager*, i8*)***
  %i31 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, %i2
  br i1 %i31, label %bb150, label %bb34

bb34:                                             ; preds = %bb120, %bb26
  %i35 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i28, %bb26 ], [ %i122, %bb120 ]
  %i36 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i35, i64 0, i32 0
  %i37 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i36, align 8
  %i38 = icmp eq %ReusableArenaBlock* %i37, null
  br i1 %i38, label %bb120, label %bb39

bb39:                                             ; preds = %bb34
  %i40 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i37, i64 0, i32 0
  %i41 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i40, i64 0, i32 1
  %i42 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i40, i64 0, i32 2
  %i43 = load i16, i16* %i42, align 2
  %i44 = icmp eq i16 %i43, 0
  br i1 %i44, label %bb53, label %bb45

bb45:                                             ; preds = %bb39
  %i46 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i40, i64 0, i32 3
  br label %bb47

bb47:                                             ; preds = %bb100, %bb45
  %i48 = phi i64 [ 0, %bb45 ], [ %i104, %bb100 ]
  %i49 = phi i16 [ %i43, %bb45 ], [ %i102, %bb100 ]
  %i50 = phi i16 [ 0, %bb45 ], [ %i103, %bb100 ]
  %i51 = load i16, i16* %i41, align 8
  %i52 = icmp ult i16 %i50, %i51
  br i1 %i52, label %bb77, label %bb53

bb53:                                             ; preds = %bb100, %bb47, %bb39
  %i54 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i40, i64 0, i32 3
  %i55 = load %XStringCached*, %XStringCached** %i54, align 8
  %i56 = icmp eq %XStringCached* %i55, null
  br i1 %i56, label %bb106, label %bb57

bb57:                                             ; preds = %bb53
  %i58 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i40, i64 0, i32 0
  %i59 = getelementptr inbounds %XalanAllocator, %XalanAllocator* %i58, i64 0, i32 0
  %i60 = load %MemoryManager*, %MemoryManager** %i59, align 8
  %i61 = bitcast %XStringCached* %i55 to i8*
  %i62 = bitcast %MemoryManager* %i60 to void (%MemoryManager*, i8*)***
  %i63 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i62, align 8
  %i64 = bitcast void (%MemoryManager*, i8*)** %i63 to i8*
  %i65 = tail call i1 @llvm.type.test(i8* %i64, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i65)
  %i66 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i63, i64 3
  %i67 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i66, align 8
  %i68 = bitcast void (%MemoryManager*, i8*)* %i67 to i8*
  %i69 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i70 = icmp eq i8* %i68, %i69
  br i1 %i70, label %bb71, label %bb72

bb71:                                             ; preds = %bb57
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i60, i8* nonnull %i61)
          to label %bb73 unwind label %bb74

bb72:                                             ; preds = %bb57
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i60, i8* nonnull %i61)
          to label %bb73 unwind label %bb74

bb73:                                             ; preds = %bb72, %bb71
  br label %bb106

bb74:                                             ; preds = %bb72, %bb71
  %i75 = cleanuppad within none []
  call void @__std_terminate() [ "funclet"(token %i75) ]
  unreachable

bb77:                                             ; preds = %bb47
  %i78 = load %XStringCached*, %XStringCached** %i46, align 8
  %i79 = getelementptr inbounds %XStringCached, %XStringCached* %i78, i64 %i48
  %i80 = bitcast %XStringCached* %i79 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i81 = zext i16 %i49 to i64
  %i82 = icmp ult i64 %i48, %i81
  br i1 %i82, label %bb83, label %bb91

bb83:                                             ; preds = %bb77
  %i84 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i80, i64 0, i32 1
  %i85 = load i32, i32* %i84, align 4
  %i86 = icmp eq i32 %i85, -2228259
  br i1 %i86, label %bb87, label %bb91

bb87:                                             ; preds = %bb83
  %i88 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i80, i64 0, i32 0
  %i89 = load i16, i16* %i88, align 4
  %i90 = icmp ugt i16 %i89, %i49
  br i1 %i90, label %bb91, label %bb100

bb91:                                             ; preds = %bb87, %bb83, %bb77
  %i92 = bitcast %XStringCached* %i79 to void (%XStringCached*)***
  %i93 = load void (%XStringCached*)**, void (%XStringCached*)*** %i92, align 8
  %i94 = bitcast void (%XStringCached*)** %i93 to i8*
  %i95 = tail call i1 @llvm.type.test(i8* %i94, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %i95)
  %i96 = load void (%XStringCached*)*, void (%XStringCached*)** %i93, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%XStringCached* nonnull dereferenceable(80) %i79)
  %i97 = add nuw i16 %i50, 1
  %i98 = load i16, i16* %i42, align 2
  %i99 = zext i16 %i98 to i64
  br label %bb100

bb100:                                            ; preds = %bb91, %bb87
  %i101 = phi i64 [ %i99, %bb91 ], [ %i81, %bb87 ]
  %i102 = phi i16 [ %i98, %bb91 ], [ %i49, %bb87 ]
  %i103 = phi i16 [ %i97, %bb91 ], [ %i50, %bb87 ]
  %i104 = add nuw nsw i64 %i48, 1
  %i105 = icmp ult i64 %i104, %i101
  br i1 %i105, label %bb47, label %bb53

bb106:                                            ; preds = %bb73, %bb53
  %i107 = bitcast %ReusableArenaBlock* %i37 to i8*
  %i108 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i33, align 8
  %i109 = bitcast void (%MemoryManager*, i8*)** %i108 to i8*
  %i110 = tail call i1 @llvm.type.test(i8* %i109, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i110)
  %i111 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i108, i64 3
  %i112 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i111, align 8
  %i113 = bitcast void (%MemoryManager*, i8*)* %i112 to i8*
  %i114 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i115 = icmp eq i8* %i113, %i114
  br i1 %i115, label %bb116, label %bb117

bb116:                                            ; preds = %bb106
  tail call void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i30, i8* nonnull %i107)
  br label %bb118

bb117:                                            ; preds = %bb106
  tail call void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i30, i8* nonnull %i107)
  br label %bb118

bb118:                                            ; preds = %bb117, %bb116
  br label %bb119

bb119:                                            ; preds = %bb118
  br label %bb120

bb120:                                            ; preds = %bb119, %bb34
  %i121 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i35, i64 0, i32 2
  %i122 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i121, align 8
  %i123 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i122, %i2
  br i1 %i123, label %bb124, label %bb34

bb124:                                            ; preds = %bb120
  %i125 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i1, align 8
  %i128 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i125, null
  br i1 %i128, label %bb129, label %bb150

bb129:                                            ; preds = %bb126
  %i130 = load %MemoryManager*, %MemoryManager** %i29, align 8
  %i131 = bitcast %MemoryManager* %i130 to i8* (%MemoryManager*, i64)***
  %i132 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i131, align 8
  %i133 = bitcast i8* (%MemoryManager*, i64)** %i132 to i8*
  %i134 = tail call i1 @llvm.type.test(i8* %i133, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i134)
  %i135 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i132, i64 2
  %i136 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i135, align 8
  %i137 = bitcast i8* (%MemoryManager*, i64)* %i136 to i8*
  %i138 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i139 = icmp eq i8* %i137, %i138
  br i1 %i139, label %bb140, label %bb142

bb140:                                            ; preds = %bb129
  %i141 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i130, i64 24)
  br label %bb144

bb142:                                            ; preds = %bb129
  %i143 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i130, i64 24)
  br label %bb144

bb144:                                            ; preds = %bb142, %bb140
  %i145 = phi i8* [ %i141, %bb140 ], [ %i143, %bb142 ]
  br label %bb146

bb146:                                            ; preds = %bb144
  %i147 = bitcast i8* %i145 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i147, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i1, align 8
  %i148 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i147, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i147, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i148, align 8
  %i149 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i147, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i147, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i149, align 8
  br label %bb170

bb150:                                            ; preds = %bb126, %bb22, %bb26
  %i151 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i23, %bb22 ], [ %i125, %bb124 ], [ %i2 , %bb26 ]
  %i152 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i151, i64 0, i32 2
  %i153 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i152, align 8
  %i154 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 2
  %i155 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i153, %i151
  br i1 %i155, label %bb170, label %bb156

bb156:                                            ; preds = %bb150
  %i157 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i154, align 8
  br label %bb158

bb158:                                            ; preds = %bb158, %bb156
  %i159 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i157, %bb156 ], [ %i160, %bb158 ]
  %i160 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i153, %bb156 ], [ %i162, %bb158 ]
  %i161 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i160, i64 0, i32 2
  %i162 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i161, align 8
  %i163 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i160, i64 0, i32 1
  %i164 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i163, align 8
  %i165 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i164, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i162, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i165, align 8
  %i166 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i161, align 8
  %i167 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i166, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i164, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i167, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i163, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i159, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i161, align 8
  %i168 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i162, %i151
  br i1 %i168, label %bb169, label %bb158

bb169:                                            ; preds = %bb158
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i160, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i154, align 8
  br label %bb170

bb170:                                            ; preds = %bb169, %bb150, %bb146
  ret void
}

define internal %XStringCached* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%ReusableArenaAllocator* nocapture nonnull dereferenceable(41) %arg) personality i32 (...)* @__CxxFrameHandler3 {
bb:
  %i = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0
  %i1 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 2
  %i2 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 1
  %i3 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i4 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i3, null
  br i1 %i4, label %bb5, label %bb27

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 0
  %i7 = load %MemoryManager*, %MemoryManager** %i6, align 8
  %i8 = bitcast %MemoryManager* %i7 to i8* (%MemoryManager*, i64)***
  %i9 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i8, align 8
  %i10 = bitcast i8* (%MemoryManager*, i64)** %i9 to i8*
  %i11 = tail call i1 @llvm.type.test(i8* %i10, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i11)
  %i12 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i9, i64 2
  %i13 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i12, align 8
  %i14 = bitcast i8* (%MemoryManager*, i64)* %i13 to i8*
  %i15 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i16 = icmp eq i8* %i14, %i15
  br i1 %i16, label %bb17, label %bb19

bb17:                                             ; preds = %bb5
  %i18 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i7, i64 24)
  br label %bb21

bb19:                                             ; preds = %bb5
  %i20 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i7, i64 24)
  br label %bb21

bb21:                                             ; preds = %bb19, %bb17
  %i22 = phi i8* [ %i18, %bb17 ], [ %i20, %bb19 ]
  br label %bb23

bb23:                                             ; preds = %bb21
  %i24 = bitcast i8* %i22 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i24, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i25 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i24, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i24, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i25, align 8
  %i26 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i24, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i24, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i26, align 8
  br label %bb39

bb27:                                             ; preds = %bb
  %i28 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i3, i64 0, i32 2
  %i29 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i28, align 8
  %i30 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i29, %i3
  br i1 %i30, label %bb39, label %bb31

bb31:                                             ; preds = %bb27
  %i32 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i29 to %ArenaBlockBase**
  %i33 = load %ArenaBlockBase*, %ArenaBlockBase** %i32, align 8
  %i34 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i33, i64 0, i32 1
  %i35 = load i16, i16* %i34, align 8
  %i36 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i33, i64 0, i32 2
  %i37 = load i16, i16* %i36, align 2
  %i38 = icmp ult i16 %i35, %i37
  br i1 %i38, label %bb185, label %bb39

bb39:                                             ; preds = %bb31, %bb27, %bb23
  %i40 = tail call nonnull align 8 dereferenceable(8) %MemoryManager* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%ArenaAllocator* nonnull dereferenceable(40) %i)
  %i41 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 1
  %i42 = load i16, i16* %i41, align 8
  %i43 = bitcast %MemoryManager* %i40 to i8* (%MemoryManager*, i64)***
  %i44 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i43, align 8
  %i45 = bitcast i8* (%MemoryManager*, i64)** %i44 to i8*
  %i46 = tail call i1 @llvm.type.test(i8* %i45, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i46)
  %i47 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i44, i64 2
  %i48 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i47, align 8
  %i49 = bitcast i8* (%MemoryManager*, i64)* %i48 to i8*
  %i50 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i51 = icmp eq i8* %i49, %i50
  br i1 %i51, label %bb52, label %bb54

bb52:                                             ; preds = %bb39
  %i53 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i64 32)
  br label %bb56

bb54:                                             ; preds = %bb39
  %i55 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i64 32)
  br label %bb56

bb56:                                             ; preds = %bb54, %bb52
  %i57 = phi i8* [ %i53, %bb52 ], [ %i55, %bb54 ]
  br label %bb58

bb58:                                             ; preds = %bb56
  %i59 = bitcast i8* %i57 to %ReusableArenaBlock*
  %i60 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i59, i64 0, i32 0
  %i61 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i60, i64 0, i32 0
  %i62 = getelementptr inbounds %XalanAllocator, %XalanAllocator* %i61, i64 0, i32 0
  store %MemoryManager* %i40, %MemoryManager** %i62, align 8
  %i63 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i60, i64 0, i32 1
  store i16 0, i16* %i63, align 8
  %i64 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i60, i64 0, i32 2
  store i16 %i42, i16* %i64, align 2
  %i65 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i43, align 8
  %i66 = bitcast i8* (%MemoryManager*, i64)** %i65 to i8*
  %i67 = tail call i1 @llvm.type.test(i8* %i66, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i67)
  %i68 = zext i16 %i42 to i64
  %i69 = mul nuw nsw i64 %i68, 80
  %i70 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i65, i64 2
  %i71 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i70, align 8
  %i72 = bitcast i8* (%MemoryManager*, i64)* %i71 to i8*
  %i73 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i74 = icmp eq i8* %i72, %i73
  br i1 %i74, label %bb75, label %bb77

bb75:                                             ; preds = %bb58
  %i76 = invoke i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i64 %i69)
          to label %bb79 unwind label %bb99

bb77:                                             ; preds = %bb58
  %i78 = invoke i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i64 %i69)
          to label %bb79 unwind label %bb99

bb79:                                             ; preds = %bb77, %bb75
  %i80 = phi i8* [ %i76, %bb75 ], [ %i78, %bb77 ]
  br label %bb81

bb81:                                             ; preds = %bb79
  %i82 = bitcast i8* %i80 to %XStringCached*
  %i83 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i60, i64 0, i32 3
  store %XStringCached* %i82, %XStringCached** %i83, align 8
  %i84 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i59, i64 0, i32 1
  store i16 0, i16* %i84, align 8
  %i85 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i59, i64 0, i32 2
  store i16 0, i16* %i85, align 2
  %i86 = load i16, i16* %i64, align 2
  %i87 = icmp eq i16 %i86, 0
  br i1 %i87, label %bb117, label %bb88

bb88:                                             ; preds = %bb81
  %i89 = zext i16 %i86 to i64
  br label %bb90

bb90:                                             ; preds = %bb90, %bb88
  %i91 = phi i64 [ 0, %bb88 ], [ %i94, %bb90 ]
  %i92 = getelementptr inbounds %XStringCached, %XStringCached* %i82, i64 %i91
  %i93 = bitcast %XStringCached* %i92 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i94 = add nuw nsw i64 %i91, 1
  %i95 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i93, i64 0, i32 0
  %i96 = trunc i64 %i94 to i16
  store i16 %i96, i16* %i95, align 4
  %i97 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i93, i64 0, i32 1
  store i32 -2228259, i32* %i97, align 4
  %i98 = icmp eq i64 %i94, %i89
  br i1 %i98, label %bb117, label %bb90

bb99:                                             ; preds = %bb77, %bb75
  %i100 = cleanuppad within none []
  %i101 = bitcast %MemoryManager* %i40 to void (%MemoryManager*, i8*)***
  %i102 = load void (%MemoryManager*, i8*)**, void (%MemoryManager*, i8*)*** %i101, align 8
  %i103 = bitcast void (%MemoryManager*, i8*)** %i102 to i8*
  %i104 = tail call i1 @llvm.type.test(i8* %i103, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i104)
  %i105 = getelementptr inbounds void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i102, i64 3
  %i106 = load void (%MemoryManager*, i8*)*, void (%MemoryManager*, i8*)** %i105, align 8
  %i107 = bitcast void (%MemoryManager*, i8*)* %i106 to i8*
  %i108 = bitcast void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %i109 = icmp eq i8* %i107, %i108
  br i1 %i109, label %bb110, label %bb111

bb110:                                            ; preds = %bb99
  invoke void bitcast (void (%MemoryManagerImpl*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i8* nonnull %i57) [ "funclet"(token %i100) ]
          to label %bb112 unwind label %bb113

bb111:                                            ; preds = %bb99
  invoke void bitcast (void (%DummyMemoryManager*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%MemoryManager*, i8*)*)(%MemoryManager* nonnull dereferenceable(8) %i40, i8* nonnull %i57) [ "funclet"(token %i100) ]
          to label %bb112 unwind label %bb113

bb112:                                            ; preds = %bb111, %bb110
  br label %bb116

bb113:                                            ; preds = %bb111, %bb110
  %i114 = cleanuppad within %i100 []
  call void @__std_terminate() [ "funclet"(token %i114) ]
  unreachable

bb116:                                            ; preds = %bb112
  cleanupret from %i100 unwind to caller

bb117:                                            ; preds = %bb90, %bb81
  %i118 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i119 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i118, null
  br i1 %i119, label %bb123, label %bb120

bb120:                                            ; preds = %bb117
  %i121 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i118, i64 0, i32 2
  %i122 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i121, align 8
  br label %bb145

bb123:                                            ; preds = %bb117
  %i124 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 0
  %i125 = load %MemoryManager*, %MemoryManager** %i124, align 8
  %i126 = bitcast %MemoryManager* %i125 to i8* (%MemoryManager*, i64)***
  %i127 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i126, align 8
  %i128 = bitcast i8* (%MemoryManager*, i64)** %i127 to i8*
  %i129 = tail call i1 @llvm.type.test(i8* %i128, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i129)
  %i130 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i127, i64 2
  %i131 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i130, align 8
  %i132 = bitcast i8* (%MemoryManager*, i64)* %i131 to i8*
  %i133 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i134 = icmp eq i8* %i132, %i133
  br i1 %i134, label %bb135, label %bb137

bb135:                                            ; preds = %bb123
  %i136 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i125, i64 24)
  br label %bb139

bb137:                                            ; preds = %bb123
  %i138 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i125, i64 24)
  br label %bb139

bb139:                                            ; preds = %bb137, %bb135
  %i140 = phi i8* [ %i136, %bb135 ], [ %i138, %bb137 ]
  br label %bb141

bb141:                                            ; preds = %bb139
  %i142 = bitcast i8* %i140 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i142, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i143 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i142, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i142, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i143, align 8
  %i144 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i142, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i142, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i144, align 8
  br label %bb145

bb145:                                            ; preds = %bb141, %bb120
  %i146 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i142, %bb141 ], [ %i122, %bb120 ]
  %i147 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 2
  %i148 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i147, align 8
  %i149 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i148, null
  br i1 %i149, label %bb153, label %bb150

bb150:                                            ; preds = %bb145
  %i151 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i148, i64 0, i32 2
  %i152 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i151, align 8
  br label %bb173

bb153:                                            ; preds = %bb145
  %i154 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 0
  %i155 = load %MemoryManager*, %MemoryManager** %i154, align 8
  %i156 = bitcast %MemoryManager* %i155 to i8* (%MemoryManager*, i64)***
  %i157 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i156, align 8
  %i158 = bitcast i8* (%MemoryManager*, i64)** %i157 to i8*
  %i159 = tail call i1 @llvm.type.test(i8* %i158, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i159)
  %i160 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i157, i64 2
  %i161 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i160, align 8
  %i162 = bitcast i8* (%MemoryManager*, i64)* %i161 to i8*
  %i163 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i164 = icmp eq i8* %i162, %i163
  br i1 %i164, label %bb165, label %bb167

bb165:                                            ; preds = %bb153
  %i166 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i155, i64 24)
  br label %bb169

bb167:                                            ; preds = %bb153
  %i168 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i155, i64 24)
  br label %bb169

bb169:                                            ; preds = %bb167, %bb165
  %i170 = phi i8* [ %i166, %bb165 ], [ %i168, %bb167 ]
  br label %bb171

bb171:                                            ; preds = %bb169
  %i172 = bitcast i8* %i170 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %bb173

bb173:                                            ; preds = %bb171, %bb150
  %i174 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i148, %bb150 ], [ %i172, %bb171 ]
  %i175 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i152, %bb150 ], [ null, %bb171 ]
  %i176 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i174, i64 0, i32 0
  store %ReusableArenaBlock* %i59, %ReusableArenaBlock** %i176, align 8
  %i177 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i174, i64 0, i32 1
  %i178 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i146, i64 0, i32 1
  %i179 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i178, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i179, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i177, align 8
  %i180 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i174, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i146, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i180, align 8
  %i181 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i178, align 8
  %i182 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i181, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i174, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i182, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i174, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i178, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i175, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i147, align 8
  %i183 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i184 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i183, null
  br i1 %i184, label %bb189, label %bb185

bb185:                                            ; preds = %bb173, %bb31
  %i186 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i183, %bb173 ], [ %i3, %bb31 ]
  %i187 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i186, i64 0, i32 2
  %i188 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i187, align 8
  br label %bb211

bb189:                                            ; preds = %bb173
  %i190 = getelementptr inbounds %XalanList, %XalanList* %i1, i64 0, i32 0
  %i191 = load %MemoryManager*, %MemoryManager** %i190, align 8
  %i192 = bitcast %MemoryManager* %i191 to i8* (%MemoryManager*, i64)***
  %i193 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i192, align 8
  %i194 = bitcast i8* (%MemoryManager*, i64)** %i193 to i8*
  %i195 = tail call i1 @llvm.type.test(i8* %i194, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i195)
  %i196 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i193, i64 2
  %i197 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i196, align 8
  %i198 = bitcast i8* (%MemoryManager*, i64)* %i197 to i8*
  %i199 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i200 = icmp eq i8* %i198, %i199
  br i1 %i200, label %bb201, label %bb203

bb201:                                            ; preds = %bb189
  %i202 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i191, i64 24)
  br label %bb205

bb203:                                            ; preds = %bb189
  %i204 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i191, i64 24)
  br label %bb205

bb205:                                            ; preds = %bb203, %bb201
  %i206 = phi i8* [ %i202, %bb201 ], [ %i204, %bb203 ]
  br label %bb207

bb207:                                            ; preds = %bb205
  %i208 = bitcast i8* %i206 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i208, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i2, align 8
  %i209 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i208, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i208, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i209, align 8
  %i210 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i208, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i208, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i210, align 8
  br label %bb211

bb211:                                            ; preds = %bb207, %bb185
  %i212 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i208, %bb207 ], [ %i188, %bb185 ]
  %i213 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i212, i64 0, i32 0
  %i214 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i213, align 8
  %i215 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i214, i64 0, i32 0
  %i216 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i215, i64 0, i32 1
  %i217 = load i16, i16* %i216, align 8
  %i218 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i215, i64 0, i32 2
  %i219 = load i16, i16* %i218, align 2
  %i220 = icmp eq i16 %i217, %i219
  br i1 %i220, label %bb238, label %bb221

bb221:                                            ; preds = %bb211
  %i222 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i214, i64 0, i32 1
  %i223 = load i16, i16* %i222, align 8
  %i224 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i214, i64 0, i32 2
  %i225 = load i16, i16* %i224, align 2
  %i226 = icmp eq i16 %i223, %i225
  %i227 = zext i16 %i223 to i64
  %i228 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i215, i64 0, i32 3
  %i229 = load %XStringCached*, %XStringCached** %i228, align 8
  br i1 %i226, label %bb232, label %bb230

bb230:                                            ; preds = %bb221
  %i231 = getelementptr inbounds %XStringCached, %XStringCached* %i229, i64 %i227
  br label %bb238

bb232:                                            ; preds = %bb221
  %i233 = getelementptr inbounds %XStringCached, %XStringCached* %i229, i64 %i227
  %i234 = bitcast %XStringCached* %i233 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %i235 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %i234, i64 0, i32 0
  %i236 = load i16, i16* %i235, align 4
  store i16 %i236, i16* %i224, align 2
  %i237 = add i16 %i217, 1
  store i16 %i237, i16* %i216, align 8
  br label %bb238

bb238:                                            ; preds = %bb232, %bb230, %bb211
  %i239 = phi %XStringCached* [ null, %bb211 ], [ %i231, %bb230 ], [ %i233, %bb232 ]
  ret %XStringCached* %i239
}

; IsOptimizedCommitAllocation handles this routine which has optimized IR.
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%ReusableArenaAllocator* %arg, %XStringCached* nocapture readnone %arg1) {
; identifyGetBeginEnd handles from this block to bb31.
bb:
  %i = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0
  %i2 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 2
  %i3 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 1
  %i4 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i5 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i4, null
  br i1 %i5, label %bb9, label %bb6

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i4, i64 0, i32 2
  %i8 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i7, align 8
  br label %bb31

bb9:                                              ; preds = %bb
  %i10 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 0
  %i11 = load %MemoryManager*, %MemoryManager** %i10, align 8
  %i12 = bitcast %MemoryManager* %i11 to i8* (%MemoryManager*, i64)***
  %i13 = load i8* (%MemoryManager*, i64)**, i8* (%MemoryManager*, i64)*** %i12, align 8
  %i14 = bitcast i8* (%MemoryManager*, i64)** %i13 to i8*
  %i15 = tail call i1 @llvm.type.test(i8* %i14, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i15)
  %i16 = getelementptr inbounds i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i13, i64 2
  %i17 = load i8* (%MemoryManager*, i64)*, i8* (%MemoryManager*, i64)** %i16, align 8
  %i18 = bitcast i8* (%MemoryManager*, i64)* %i17 to i8*
  %i19 = bitcast i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %i20 = icmp eq i8* %i18, %i19
  br i1 %i20, label %bb21, label %bb23

bb21:                                             ; preds = %bb9
  %i22 = tail call i8* bitcast (i8* (%MemoryManagerImpl*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i11, i64 24)
  br label %bb25

bb23:                                             ; preds = %bb9
  %i24 = tail call i8* bitcast (i8* (%DummyMemoryManager*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%MemoryManager*, i64)*)(%MemoryManager* nonnull dereferenceable(8) %i11, i64 24)
  br label %bb25

bb25:                                             ; preds = %bb23, %bb21
  %i26 = phi i8* [ %i22, %bb21 ], [ %i24, %bb23 ]
  br label %bb27

bb27:                                             ; preds = %bb25
  %i28 = bitcast i8* %i26 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i3, align 8
  %i29 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i29, align 8
  %i30 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i28, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i30, align 8
  br label %bb31

; identifyBlockAvailable handles this block.
; This block also has IR for BlockCommit.
bb31:                                             ; preds = %bb27, %bb6
  %i32 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i28, %bb27 ], [ %i4, %bb6 ]
  %i33 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %i28, %bb27 ], [ %i8, %bb6 ]
  %i34 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i33, i64 0, i32 0
  %i35 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i34, align 8
  %i36 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i35, i64 0, i32 2
  %i37 = load i16, i16* %i36, align 2
  %i38 = getelementptr inbounds %ReusableArenaBlock, %ReusableArenaBlock* %i35, i64 0, i32 1
  store i16 %i37, i16* %i38, align 8
  %i39 = getelementptr %ReusableArenaBlock, %ReusableArenaBlock* %i35, i64 0, i32 0
  %i40 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i39, i64 0, i32 1
  %i41 = load i16, i16* %i40, align 8
  %i42 = getelementptr inbounds %ArenaBlockBase, %ArenaBlockBase* %i39, i64 0, i32 2
  %i43 = load i16, i16* %i42, align 2
  %i44 = icmp ult i16 %i41, %i43
  br i1 %i44, label %bb63, label %bb45

; This block has both PopFront and PushBack.
; identifyPopFrontPushBack will handle this block.
bb45:                                             ; preds = %bb31
  %i46 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i32, i64 0, i32 2
  %i47 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i46, align 8
  %i48 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i47, i64 0, i32 2
  %i49 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i48, align 8
  %i50 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i47, i64 0, i32 1
  %i51 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i50, align 8
  %i52 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i47, i64 0, i32 0
  %i53 = load %ReusableArenaBlock*, %ReusableArenaBlock** %i52, align 8
  %i54 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i51, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i49, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i54, align 8
  %i55 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i48, align 8
  %i56 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i55, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i51, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i56, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i50, align 8
  %i57 = getelementptr inbounds %XalanList, %XalanList* %i2, i64 0, i32 2
  %i58 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i57, align 8
  store %ReusableArenaBlock* %i53, %ReusableArenaBlock** %i52, align 8
  %i59 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i32, i64 0, i32 1
  %i60 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i59, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i60, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i50, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i32, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i48, align 8
  %i61 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i59, align 8
  %i62 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i61, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i47, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i62, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i47, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i59, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %i58, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %i57, align 8
  br label %bb63

bb63:                                             ; preds = %bb45, %bb31
  ret void
}

define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt.8894(%XStringCachedAllocator* nocapture nonnull dereferenceable(48) %arg, %MemoryManager* nonnull align 8 dereferenceable(8) %arg1, i16 zeroext %arg2) unnamed_addr align 2 {
bb:
  %i = getelementptr inbounds %XStringCachedAllocator, %XStringCachedAllocator* %arg, i64 0, i32 0
  %i2 = tail call %ReusableArenaAllocator* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%ReusableArenaAllocator* nonnull dereferenceable(41) %i, %MemoryManager* nonnull align 8 dereferenceable(8) %arg1, i16 zeroext %arg2, i1 zeroext false)
  ret void
}

define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev.8907(%XStringCachedAllocator* nocapture nonnull dereferenceable(48) %arg) unnamed_addr align 2 {
bb:
  %i = getelementptr inbounds %XStringCachedAllocator, %XStringCachedAllocator* %arg, i64 0, i32 0
  %i1 = getelementptr %ReusableArenaAllocator, %ReusableArenaAllocator* %i, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%ArenaAllocator* nonnull dereferenceable(41) %i1)
  ret void
}

define internal %XStringCached* @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE.8908(%XStringCachedAllocator* nocapture nonnull dereferenceable(48) %arg, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %arg1) align 2 {
bb:
  %i = getelementptr inbounds %XStringCachedAllocator, %XStringCachedAllocator* %arg, i64 0, i32 0
  %i2 = tail call %XStringCached* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%ReusableArenaAllocator* nonnull dereferenceable(41) %i)
  %i3 = getelementptr %ReusableArenaAllocator, %ReusableArenaAllocator* %i, i64 0, i32 0
  %i4 = tail call nonnull align 8 dereferenceable(8) %MemoryManager* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%ArenaAllocator* nonnull dereferenceable(40) %i3)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%XStringCached* nonnull dereferenceable(80) %i2, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %arg1, %MemoryManager* nonnull align 8 dereferenceable(8) %i4)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%ReusableArenaAllocator* nonnull dereferenceable(41) %i, %XStringCached* nonnull %i2)
  ret %XStringCached* %i2
}

define internal zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE.8909(%XStringCachedAllocator* nocapture nonnull dereferenceable(48) %arg, %XStringCached* %arg1) align 2 {
bb:
  %i = getelementptr inbounds %XStringCachedAllocator, %XStringCachedAllocator* %arg, i64 0, i32 0
  %i2 = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%ReusableArenaAllocator* nonnull dereferenceable(41) %i, %XStringCached* %arg1)
  ret i1 %i2
}

define internal void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv.8911(%XStringCachedAllocator* nocapture nonnull dereferenceable(48) %arg) align 2 {
bb:
  %i = getelementptr inbounds %XStringCachedAllocator, %XStringCachedAllocator* %arg, i64 0, i32 0
  %i1 = getelementptr %ReusableArenaAllocator, %ReusableArenaAllocator* %i, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%ArenaAllocator* nonnull dereferenceable(40) %i1)
  ret void
}

define internal noalias nonnull i8* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(%MemoryManagerImpl* nocapture nonnull readnone dereferenceable(8) %0, i64 %1) align 2 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
  %3 = alloca i8, align 1
  %4 = invoke noalias nonnull i8* @"??2@YAPEAX_K@Z"(i64 %1) #121
          to label %10 unwind label %5

5:                                                ; preds = %2
  %6 = catchswitch within none [label %7] unwind to caller

7:                                                ; preds = %5
  %8 = catchpad within %6 [i8* null, i32 64, i8* null]
  %9 = getelementptr inbounds i8, i8* %3, i64 0
  call void @_CxxThrowException(i8* nonnull %9, %eh.ThrowInfo* null) [ "funclet"(token %8) ]
  unreachable

10:                                               ; preds = %2
  ret i8* %4
}

define internal void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(%MemoryManagerImpl* nocapture nonnull readnone dereferenceable(8) %arg, i8* nocapture %arg1) unnamed_addr align 2 {
bb:
  tail call void @"??3@YAXPEAX@Z"(i8* %arg1)
  ret void
}

define internal noalias nonnull i8* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(%DummyMemoryManager* nocapture nonnull readnone dereferenceable(8) %0, i64 %1) unnamed_addr align 2 {
  %a = alloca %"class.std::bad_alloc", align 8
  %g1 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 1, i32 1
  %bc1 = bitcast i8* %g1 to i64*
  store i64 0, i64* %bc1, align 8
  %g3 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 1, i32 0
  store i8* getelementptr inbounds ([15 x i8], [15 x i8]* null, i64 0, i64 0), i8** %g3, align 8
  %g2 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** null to i32 (...)**), i32 (...)*** %g2, align 8
  %bc2 = bitcast %"class.std::bad_alloc"* %a to i8*
  call void @_CxxThrowException(i8* %bc2, %eh.ThrowInfo* null)
  unreachable
}

define internal void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(%DummyMemoryManager* nocapture nonnull readnone dereferenceable(8) %0, i8* nocapture readnone %1) unnamed_addr align 2 {
bb:
  %2 = alloca %"class.std::bad_alloc", align 8
  %3 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %2, i64 0, i32 0
  %4 = getelementptr inbounds %"class.std::exception", %"class.std::exception"* %3, i64 0, i32 1
  %5 = bitcast %struct.__std_exception_data* %4 to i8*
  %6 = getelementptr inbounds i8, i8* %5, i64 8
  %7 = bitcast i8* %6 to i64*
  store i64 0, i64* %7, align 8
  %8 = getelementptr inbounds %struct.__std_exception_data, %struct.__std_exception_data* %4, i64 0, i32 0
  store i8* getelementptr inbounds ([15 x i8], [15 x i8]* null, i64 0, i64 0), i8** %8, align 8
  %9 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %2, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** null to i32 (...)**), i32 (...)*** %9, align 8
  %10 = bitcast %"class.std::bad_alloc"* %2 to i8*
  call void @_CxxThrowException(i8* nonnull %10, %eh.ThrowInfo* null)
  unreachable
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

declare void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%XStringCached* nonnull, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull, %MemoryManager* nonnull) #0

declare void @_ZN11xalanc_1_1013XStringCachedD2Ev(%XStringCached* nonnull) #1

declare void @__std_terminate()

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef)

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i1 @llvm.type.test(i8*, metadata)

declare dso_local i32 @__CxxFrameHandler3(...)

declare i8* @"??2@YAPEAX_K@Z"(i64)
declare void @"??3@YAXPEAX@Z"(i8*)

declare void @_CxxThrowException(i8*, %eh.ThrowInfo*)

attributes #0 = { "intel-mempool-constructor" }
attributes #1 = { "intel-mempool-destructor" }
