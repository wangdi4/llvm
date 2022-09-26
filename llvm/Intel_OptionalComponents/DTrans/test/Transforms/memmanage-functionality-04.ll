; This test verifies the following function is NOT recognized as AllocateBlock
; for MemManageTrans:
;
; _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; This function is not recognized as AllocateBlock since condition for
; BlockAvailable is "eq" instead of "ult" in "%33".

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: MemManageTrans transformation:
; CHECK:   Considering candidate: %XStringCachedAllocator
; CHECK:   Recognizing AllocateBlock Functionality _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK-NOT: Recognized AllocateBlock: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv

%"XStringCachedAllocator" = type { %"ReusableArenaAllocator" }
%"ReusableArenaAllocator" = type <{ %"ArenaAllocator", i8, [7 x i8] }>
%"ArenaAllocator" = type { i32 (...)**, i16, %"XalanList" }
%"XalanList" = type { %"MemoryManager"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%"XalanList<ReusableArenaBlock<XStringCached> *>::Node" = type { %"ReusableArenaBlock"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%"ReusableArenaBlock" = type <{ %"ArenaBlockBase", i16, i16, [4 x i8] }>
%"ArenaBlockBase" = type { %"XalanAllocator", i16, i16, %"XStringCached"* }
%"XalanAllocator" = type { %"MemoryManager"* }
%"XStringCached" = type { %"XStringBase", %"XPathExecutionContext::GetAndReleaseCachedString" }
%"XStringBase" = type { %"XObject", double, %"XObjectResultTreeFragProxy" }
%"XObject" = type { %"XalanReferenceCountedObject.base", i32, %"XObjectFactory"* }
%"XalanReferenceCountedObject.base" = type <{ i32 (...)**, i32 }>
%"XObjectFactory" = type opaque
%"XObjectResultTreeFragProxy" = type { %"XObjectResultTreeFragProxyBase", %"XObjectResultTreeFragProxyText" }
%"XObjectResultTreeFragProxyBase" = type { %"XalanDocumentFragment" }
%"XalanDocumentFragment" = type { %"XalanNode" }
%"XalanNode" = type { i32 (...)** }
%"XObjectResultTreeFragProxyText" = type { %"XalanText", %"XObject"*, %"MemoryManager"* }
%"XalanText" = type { %"XalanCharacterData" }
%"XalanCharacterData" = type { %"XalanNode" }
%"XPathExecutionContext::GetAndReleaseCachedString" = type { %"XPathExecutionContext"*, %"XalanDOMString"* }
%"XPathExecutionContext" = type { %"ExecutionContext", %"XObjectFactory"* }
%"ExecutionContext" = type { i32 (...)**, %"MemoryManager"* }
%"XalanDOMString" = type <{ %"XalanVector", i32, [4 x i8] }>
%"XalanVector" = type { %"MemoryManager"*, i64, i64, i16* }
%"MemoryManager" = type { i32 (...)** }
%"XalanListIteratorBase" = type { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%"class.std::reverse_iterator.1" = type { %"XalanListIteratorBase" }
%"DeleteFunctor" = type { %"MemoryManager"* }
%"XalanListIteratorBase.0" = type { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
%"class.std::reverse_iterator" = type { %"XalanListIteratorBase.0" }
%"XalanAllocationGuard" = type { %"MemoryManager"*, i8* }
%"ReusableArenaBlock<XStringCached>::NextBlock" = type { i16, i32 }
%"struct.std::less" = type { i8 }
%"XalanDestroyFunctor" = type { i8 }
%"DummyMemoryManager" = type { %"MemoryManager" }
%"MemoryManagerImpl" = type { %"MemoryManager" }
%"class.std::bad_alloc" = type { %"class.std::exception" }
%"class.std::exception" = type { i32 (...)** }
%"OutOfMemoryException" = type { i8 }

@_ZTSN11xercesc_2_713MemoryManagerE = internal constant [31 x i8] c"N11xercesc_2_713MemoryManagerE\00"

; Function Attrs: norecurse nounwind readonly uwtable willreturn mustprogress
define internal nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nocapture nonnull readonly dereferenceable(40) %arg) align 2 {
  %i = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %XalanList, %XalanList* %i, i64 0, i32 0
  %i2 = load %MemoryManager*, %MemoryManager** %i1, align 8
  ret %MemoryManager* %i2
}

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %arg, %"MemoryManager"* nonnull align 8 dereferenceable(8) %arg1, i16 zeroext %arg2, i1 zeroext %arg3) unnamed_addr  align 2 {
  %i = getelementptr inbounds %ReusableArenaAllocator, %ReusableArenaAllocator* %arg, i64 0, i32 0
  %i4 = getelementptr inbounds %ArenaAllocator, %ArenaAllocator* %i, i64 0, i32 1
  store i16 %arg2, i16* %i4, align 8
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
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%"ArenaAllocator"* nocapture nonnull dereferenceable(40) %0) unnamed_addr  align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nonnull dereferenceable(40) null)
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull dereferenceable(80) null)
  ret void
}

; Function Attrs: uwtable
define internal zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0, %"XStringCached"* %1) align 2 {
  ret i1 false
}

; Function Attrs: uwtable
define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nocapture nonnull dereferenceable(40) %0) unnamed_addr align 2 personality i32 (...)* @__gxx_personality_v0 {
  ret void
}

; Function Attrs: uwtable
define internal %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0) unnamed_addr align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %0, i64 0, i32 0
  %3 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %2, i64 0, i32 2
  %4 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 1
  %5 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %6 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %5, null
  br i1 %6, label %7, label %29

7:                                                ; preds = %1
  %8 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 0
  %9 = load %"MemoryManager"*, %"MemoryManager"** %8, align 8
  %10 = bitcast %"MemoryManager"* %9 to i8* (%"MemoryManager"*, i64)***
  %11 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %10, align 8
  %12 = bitcast i8* (%"MemoryManager"*, i64)** %11 to i8*
  %13 = tail call i1 @llvm.type.test(i8* %12, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %13)
  %14 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %11, i64 2
  %15 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %14, align 8
  %16 = bitcast i8* (%"MemoryManager"*, i64)* %15 to i8*
  %17 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %18 = icmp eq i8* %16, %17
  br i1 %18, label %19, label %21

19:                                               ; preds = %7
  %20 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %9, i64 24)
  br label %23

21:                                               ; preds = %7
  %22 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %9, i64 24)
  br label %23

23:                                               ; preds = %21, %19
  %24 = phi i8* [ %20, %19 ], [ %22, %21 ]
  br label %25

25:                                               ; preds = %23
  %26 = bitcast i8* %24 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %26, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %27 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %26, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %26, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %27, align 8
  %28 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %26, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %26, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %28, align 8
  br label %41

29:                                               ; preds = %1
  %30 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %5, i64 0, i32 2
  %31 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %30, align 8
  %32 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %31, %5
  br i1 %32, label %41, label %33

33:                                               ; preds = %29
  %34 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %31 to %"ArenaBlockBase"**
  %35 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %34, align 8
  %36 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %35, i64 0, i32 1
  %37 = load i16, i16* %36, align 8
  %38 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %35, i64 0, i32 2
  %39 = load i16, i16* %38, align 2
  %40 = icmp eq i16 %37, %39
  br i1 %40, label %187, label %41

41:                                               ; preds = %33, %29, %25
  %42 = tail call nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %2)
  %43 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %2, i64 0, i32 1
  %44 = load i16, i16* %43, align 8
  %45 = bitcast %"MemoryManager"* %42 to i8* (%"MemoryManager"*, i64)***
  %46 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %45, align 8
  %47 = bitcast i8* (%"MemoryManager"*, i64)** %46 to i8*
  %48 = tail call i1 @llvm.type.test(i8* %47, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %48)
  %49 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %46, i64 2
  %50 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %49, align 8
  %51 = bitcast i8* (%"MemoryManager"*, i64)* %50 to i8*
  %52 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %53 = icmp eq i8* %51, %52
  br i1 %53, label %54, label %56

54:                                               ; preds = %41
  %55 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i64 32)
  br label %58

56:                                               ; preds = %41
  %57 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i64 32)
  br label %58

58:                                               ; preds = %56, %54
  %59 = phi i8* [ %55, %54 ], [ %57, %56 ]
  br label %60

60:                                               ; preds = %58
  %61 = bitcast i8* %59 to %"ReusableArenaBlock"*
  %62 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %61, i64 0, i32 0
  %63 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %62, i64 0, i32 0
  %64 = getelementptr inbounds %"XalanAllocator", %"XalanAllocator"* %63, i64 0, i32 0
  store %"MemoryManager"* %42, %"MemoryManager"** %64, align 8
  %65 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %62, i64 0, i32 1
  store i16 0, i16* %65, align 8
  %66 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %62, i64 0, i32 2
  store i16 %44, i16* %66, align 2
  %67 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %45, align 8
  %68 = bitcast i8* (%"MemoryManager"*, i64)** %67 to i8*
  %69 = tail call i1 @llvm.type.test(i8* %68, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %69)
  %70 = zext i16 %44 to i64
  %71 = mul nuw nsw i64 %70, 80
  %72 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %67, i64 2
  %73 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %72, align 8
  %74 = bitcast i8* (%"MemoryManager"*, i64)* %73 to i8*
  %75 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %76 = icmp eq i8* %74, %75
  br i1 %76, label %77, label %79

77:                                               ; preds = %60
  %78 = invoke i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i64 %71)
          to label %81 unwind label %101

79:                                               ; preds = %60
  %80 = invoke i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i64 %71)
          to label %81 unwind label %101

81:                                               ; preds = %79, %77
  %82 = phi i8* [ %78, %77 ], [ %80, %79 ]
  br label %83

83:                                               ; preds = %81
  %84 = bitcast i8* %82 to %"XStringCached"*
  %85 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %62, i64 0, i32 3
  store %"XStringCached"* %84, %"XStringCached"** %85, align 8
  %86 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %61, i64 0, i32 1
  store i16 0, i16* %86, align 8
  %87 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %61, i64 0, i32 2
  store i16 0, i16* %87, align 2
  %88 = load i16, i16* %66, align 2
  %89 = icmp eq i16 %88, 0
  br i1 %89, label %119, label %90

90:                                               ; preds = %83
  %91 = zext i16 %88 to i64
  br label %92

92:                                               ; preds = %92, %90
  %93 = phi i64 [ 0, %90 ], [ %96, %92 ]
  %94 = getelementptr inbounds %"XStringCached", %"XStringCached"* %84, i64 %93
  %95 = bitcast %"XStringCached"* %94 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %96 = add nuw nsw i64 %93, 1
  %97 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %95, i64 0, i32 0
  %98 = trunc i64 %96 to i16
  store i16 %98, i16* %97, align 4
  %99 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %95, i64 0, i32 1
  store i32 -2228259, i32* %99, align 4
  %100 = icmp eq i64 %96, %91
  br i1 %100, label %119, label %92

101:                                              ; preds = %79, %77
  %102 = landingpad { i8*, i32 }
          cleanup
  %103 = bitcast %"MemoryManager"* %42 to void (%"MemoryManager"*, i8*)***
  %104 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %103, align 8
  %105 = bitcast void (%"MemoryManager"*, i8*)** %104 to i8*
  %106 = tail call i1 @llvm.type.test(i8* %105, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %106)
  %107 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %104, i64 3
  %108 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %107, align 8
  %109 = bitcast void (%"MemoryManager"*, i8*)* %108 to i8*
  %110 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %111 = icmp eq i8* %109, %110
  br i1 %111, label %112, label %113

112:                                              ; preds = %101
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i8* nonnull %59)
          to label %114 unwind label %115

113:                                              ; preds = %101
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %42, i8* nonnull %59)
          to label %114 unwind label %115

114:                                              ; preds = %113, %112
  br label %118

115:                                              ; preds = %113, %112
  %116 = landingpad { i8*, i32 }
          catch i8* null
  %117 = extractvalue { i8*, i32 } %116, 0
  tail call void @__clang_call_terminate(i8* %117)
  unreachable

118:                                              ; preds = %114
  resume { i8*, i32 } %102

119:                                              ; preds = %92, %83
  %120 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %121 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %120, null
  br i1 %121, label %125, label %122

122:                                              ; preds = %119
  %123 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %120, i64 0, i32 2
  %124 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %123, align 8
  br label %147

125:                                              ; preds = %119
  %126 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 0
  %127 = load %"MemoryManager"*, %"MemoryManager"** %126, align 8
  %128 = bitcast %"MemoryManager"* %127 to i8* (%"MemoryManager"*, i64)***
  %129 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %128, align 8
  %130 = bitcast i8* (%"MemoryManager"*, i64)** %129 to i8*
  %131 = tail call i1 @llvm.type.test(i8* %130, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %131)
  %132 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %129, i64 2
  %133 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %132, align 8
  %134 = bitcast i8* (%"MemoryManager"*, i64)* %133 to i8*
  %135 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %136 = icmp eq i8* %134, %135
  br i1 %136, label %137, label %139

137:                                              ; preds = %125
  %138 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %127, i64 24)
  br label %141

139:                                              ; preds = %125
  %140 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %127, i64 24)
  br label %141

141:                                              ; preds = %139, %137
  %142 = phi i8* [ %138, %137 ], [ %140, %139 ]
  br label %143

143:                                              ; preds = %141
  %144 = bitcast i8* %142 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %144, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %145 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %144, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %144, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %145, align 8
  %146 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %144, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %144, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %146, align 8
  br label %147

147:                                              ; preds = %143, %122
  %148 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %144, %143 ], [ %124, %122 ]
  %149 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 2
  %150 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %149, align 8
  %151 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %150, null
  br i1 %151, label %155, label %152

152:                                              ; preds = %147
  %153 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %150, i64 0, i32 2
  %154 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %153, align 8
  br label %175

155:                                              ; preds = %147
  %156 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 0
  %157 = load %"MemoryManager"*, %"MemoryManager"** %156, align 8
  %158 = bitcast %"MemoryManager"* %157 to i8* (%"MemoryManager"*, i64)***
  %159 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %158, align 8
  %160 = bitcast i8* (%"MemoryManager"*, i64)** %159 to i8*
  %161 = tail call i1 @llvm.type.test(i8* %160, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %161)
  %162 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %159, i64 2
  %163 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %162, align 8
  %164 = bitcast i8* (%"MemoryManager"*, i64)* %163 to i8*
  %165 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %166 = icmp eq i8* %164, %165
  br i1 %166, label %167, label %169

167:                                              ; preds = %155
  %168 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %157, i64 24)
  br label %171

169:                                              ; preds = %155
  %170 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %157, i64 24)
  br label %171

171:                                              ; preds = %169, %167
  %172 = phi i8* [ %168, %167 ], [ %170, %169 ]
  br label %173

173:                                              ; preds = %171
  %174 = bitcast i8* %172 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %175

175:                                              ; preds = %173, %152
  %176 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %150, %152 ], [ %174, %173 ]
  %177 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %154, %152 ], [ null, %173 ]
  %178 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %176, i64 0, i32 0
  store %"ReusableArenaBlock"* %61, %"ReusableArenaBlock"** %178, align 8
  %179 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %176, i64 0, i32 1
  %180 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %148, i64 0, i32 1
  %181 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %180, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %181, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %179, align 8
  %182 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %176, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %148, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %182, align 8
  %183 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %180, align 8
  %184 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %183, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %176, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %184, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %176, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %180, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %177, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %149, align 8
  %185 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %186 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %185, null
  br i1 %186, label %191, label %187

187:                                              ; preds = %175, %33
  %188 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %185, %175 ], [ %5, %33 ]
  %189 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %188, i64 0, i32 2
  %190 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %189, align 8
  br label %213

191:                                              ; preds = %175
  %192 = getelementptr inbounds %XalanList, %XalanList* %3, i64 0, i32 0
  %193 = load %"MemoryManager"*, %"MemoryManager"** %192, align 8
  %194 = bitcast %"MemoryManager"* %193 to i8* (%"MemoryManager"*, i64)***
  %195 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %194, align 8
  %196 = bitcast i8* (%"MemoryManager"*, i64)** %195 to i8*
  %197 = tail call i1 @llvm.type.test(i8* %196, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %197)
  %198 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %195, i64 2
  %199 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %198, align 8
  %200 = bitcast i8* (%"MemoryManager"*, i64)* %199 to i8*
  %201 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %202 = icmp eq i8* %200, %201
  br i1 %202, label %203, label %205

203:                                              ; preds = %191
  %204 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %193, i64 24)
  br label %207

205:                                              ; preds = %191
  %206 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %193, i64 24)
  br label %207

207:                                              ; preds = %205, %203
  %208 = phi i8* [ %204, %203 ], [ %206, %205 ]
  br label %209

209:                                              ; preds = %207
  %210 = bitcast i8* %208 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %210, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %4, align 8
  %211 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %210, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %210, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %211, align 8
  %212 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %210, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %210, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %212, align 8
  br label %213

213:                                              ; preds = %209, %187
  %214 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %210, %209 ], [ %190, %187 ]
  %215 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %214, i64 0, i32 0
  %216 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %215, align 8
  %217 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %216, i64 0, i32 0
  %218 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %217, i64 0, i32 1
  %219 = load i16, i16* %218, align 8
  %220 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %217, i64 0, i32 2
  %221 = load i16, i16* %220, align 2
  %222 = icmp eq i16 %219, %221
  br i1 %222, label %240, label %223

223:                                              ; preds = %213
  %224 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %216, i64 0, i32 1
  %225 = load i16, i16* %224, align 8
  %226 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %216, i64 0, i32 2
  %227 = load i16, i16* %226, align 2
  %228 = icmp eq i16 %225, %227
  %229 = zext i16 %225 to i64
  %230 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %217, i64 0, i32 3
  %231 = load %"XStringCached"*, %"XStringCached"** %230, align 8
  br i1 %228, label %234, label %232

232:                                              ; preds = %223
  %233 = getelementptr inbounds %"XStringCached", %"XStringCached"* %231, i64 %229
  br label %240

234:                                              ; preds = %223
  %235 = getelementptr inbounds %"XStringCached", %"XStringCached"* %231, i64 %229
  %236 = bitcast %"XStringCached"* %235 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %237 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %236, i64 0, i32 0
  %238 = load i16, i16* %237, align 4
  store i16 %238, i16* %226, align 2
  %239 = add i16 %219, 1
  store i16 %239, i16* %218, align 8
  br label %240

240:                                              ; preds = %234, %232, %213
  %241 = phi %"XStringCached"* [ null, %213 ], [ %233, %232 ], [ %235, %234 ]
  ret %"XStringCached"* %241
}

; Function Attrs: uwtable
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0, %"XStringCached"* nocapture readnone %1) unnamed_addr align 2 {
  ret void
}

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt.8894(%"XStringCachedAllocator"* nocapture nonnull dereferenceable(48) %0, %"MemoryManager"* nonnull align 8 dereferenceable(8) %1, i16 zeroext %2) unnamed_addr align 2 {
  %4 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %0, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %4, %"MemoryManager"* nonnull align 8 dereferenceable(8) %1, i16 zeroext %2, i1 zeroext false)
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev.8907(%"XStringCachedAllocator"* nocapture nonnull dereferenceable(48) %0) unnamed_addr align 2 {
  %2 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %0, i64 0, i32 0
  %3 = getelementptr %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %2, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%"ArenaAllocator"* nonnull dereferenceable(41) %3)
  ret void
}

; Function Attrs: uwtable mustprogress
define internal %"XStringCached"* @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE.8908(%"XStringCachedAllocator"* nocapture nonnull dereferenceable(48) %0, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %1) align 2 {
  %3 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %0, i64 0, i32 0
  %4 = tail call %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %3)
  %5 = getelementptr %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %3, i64 0, i32 0
  %6 = tail call nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %5)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull dereferenceable(80) %4, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %1, %"MemoryManager"* nonnull align 8 dereferenceable(8) %6)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %3, %"XStringCached"* nonnull %4)
  ret %"XStringCached"* %4
}

; Function Attrs: uwtable mustprogress
define internal zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE.8909(%"XStringCachedAllocator"* nocapture nonnull dereferenceable(48) %0, %"XStringCached"* %1) align 2 {
  %3 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %0, i64 0, i32 0
  %4 = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %3, %"XStringCached"* %1)
  ret i1 %4
}

; Function Attrs: uwtable mustprogress
define internal void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv.8911(%"XStringCachedAllocator"* nocapture nonnull dereferenceable(48) %0) align 2 {
  %2 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %0, i64 0, i32 0
  %3 = getelementptr %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %2, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nonnull dereferenceable(40) %3)
  ret void
}

; Function Attrs: nofree uwtable
define internal noalias nonnull i8* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(%"MemoryManagerImpl"* nocapture nonnull readnone dereferenceable(8) %0, i64 %1) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %3 = invoke noalias nonnull i8* @_Znwm(i64 %1)
          to label %11 unwind label %4

4:                                                ; preds = %2
  %5 = landingpad { i8*, i32 }
          catch i8* null
  %6 = extractvalue { i8*, i32 } %5, 0
  %7 = tail call i8* @__cxa_begin_catch(i8* %6)
  %8 = tail call i8* @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(i8* nonnull %8, i8* bitcast ({ i8*, i8*, i8* }* null to i8*), i8* bitcast (void (%"OutOfMemoryException"*)* null to i8*))
          to label %16 unwind label %9

9:                                                ; preds = %4
  %10 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %12 unwind label %13

11:                                               ; preds = %2
  ret i8* %3

12:                                               ; preds = %9
  resume { i8*, i32 } %10

13:                                               ; preds = %9
  %14 = landingpad { i8*, i32 }
          catch i8* null
  %15 = extractvalue { i8*, i32 } %14, 0
  tail call void @__clang_call_terminate(i8* %15)
  unreachable

16:                                               ; preds = %4
  unreachable
}

; Function Attrs: nounwind uwtable willreturn mustprogress
define internal void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(%"MemoryManagerImpl"* nocapture nonnull readnone dereferenceable(8) %0, i8* nocapture %1) unnamed_addr align 2 {
  tail call void @_ZdlPv(i8* %1)
  ret void
}

; Function Attrs: nofree noreturn uwtable
define internal noalias nonnull i8* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(%"DummyMemoryManager"* nocapture nonnull readnone dereferenceable(8) %0, i64 %1) unnamed_addr align 2 {
  %3 = tail call i8* @__cxa_allocate_exception(i64 8)
  %4 = bitcast i8* %3 to %"class.std::bad_alloc"*
  %5 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %4, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %5, align 8
  tail call void @__cxa_throw(i8* nonnull %3, i8* bitcast (i8** null to i8*), i8* bitcast (void (%"class.std::bad_alloc"*)* null to i8*))
  unreachable
}

; Function Attrs: nofree noreturn uwtable
define internal void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(%"DummyMemoryManager"* nocapture nonnull readnone dereferenceable(8) %0, i8* nocapture readnone %1) unnamed_addr align 2 {
  %3 = tail call i8* @__cxa_allocate_exception(i64 8)
  %4 = bitcast i8* %3 to %"class.std::bad_alloc"*
  %5 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %4, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %5, align 8
  tail call void @__cxa_throw(i8* nonnull %3, i8* bitcast (i8** null to i8*), i8* bitcast (void (%"class.std::bad_alloc"*)* null to i8*))
  unreachable
}


; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

declare void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull, %"MemoryManager"* nonnull ) #0

declare void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull) #1

declare void @__clang_call_terminate(i8*)

declare dso_local i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*)

declare i8* @_Znwm(i64)

declare i8* @__cxa_allocate_exception(i64)

declare void @llvm.assume(i1)

declare i1 @llvm.type.test(i8*, metadata)

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

declare void @__cxa_throw(i8* nonnull, i8*, i8*)

attributes #0 = { "intel-mempool-constructor" }
attributes #1 = { "intel-mempool-destructor" }
