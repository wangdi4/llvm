; This test is same as memmanage-functionality-01.ll except the incoming
; PHI value is 1 (instead of %257) from %289 block for the following in _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_
;
;    %505 = phi i8 [ 1, %445 ], [ 1, %470 ], [ 1, %474 ], [ 1, %478 ], [ 1, %488 ], [ 1, %496 ], [ %257, %285 ], [ 1, %289 ], [ %257, %307 ]
;
;
; This test verifies that the below function is not recognized as
; DestroyObject.
; _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: MemManageTrans transformation:
; CHECK:   Considering candidate: %XStringCachedAllocator
; CHECK-NOT: Recognized DestroyObject: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

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
  %2 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %0, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ([8 x i8*], [8 x i8*]* null, i32 0, i64 2) to i32 (...)**), i32 (...)*** %2, align 8
  %3 = tail call i1 @llvm.type.test(i8* bitcast (i8** getelementptr inbounds ([8 x i8*], [8 x i8*]* null, i32 0, i64 2) to i8*), metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %3)
  invoke void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nonnull dereferenceable(40) %0)
          to label %4 unwind label %108

4:                                                ; preds = %1
  %5 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %0, i64 0, i32 2
  %6 = getelementptr inbounds %XalanList, %XalanList* %5, i64 0, i32 1
  %7 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %6, align 8
  %8 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %7, null
  br i1 %8, label %107, label %9

9:                                                ; preds = %4
  %10 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %7, i64 0, i32 2
  %11 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %10, align 8
  %12 = getelementptr inbounds %XalanList, %XalanList* %5, i64 0, i32 0
  br label %13

13:                                               ; preds = %58, %9
  %14 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %7, %9 ], [ %59, %58 ]
  %15 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %11, %9 ], [ %43, %58 ]
  %16 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %14, null
  br i1 %16, label %17, label %38

17:                                               ; preds = %13
  %18 = load %"MemoryManager"*, %"MemoryManager"** %12, align 8
  %19 = bitcast %"MemoryManager"* %18 to i8* (%"MemoryManager"*, i64)***
  %20 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %19, align 8
  %21 = bitcast i8* (%"MemoryManager"*, i64)** %20 to i8*
  %22 = tail call i1 @llvm.type.test(i8* %21, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %22)
  %23 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %20, i64 2
  %24 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %23, align 8
  %25 = bitcast i8* (%"MemoryManager"*, i64)* %24 to i8*
  %26 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %27 = icmp eq i8* %25, %26
  br i1 %27, label %28, label %30

28:                                               ; preds = %17
  %29 = invoke i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %18, i64 24)
          to label %32 unwind label %60

30:                                               ; preds = %17
  %31 = invoke i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %18, i64 24)
          to label %32 unwind label %60

32:                                               ; preds = %30, %28
  %33 = phi i8* [ %29, %28 ], [ %31, %30 ]
  br label %34

34:                                               ; preds = %32
  %35 = bitcast i8* %33 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %6, align 8
  %36 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %36, align 8
  %37 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %37, align 8
  br label %38

38:                                               ; preds = %34, %13
  %39 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %35, %34 ], [ %14, %13 ]
  %40 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %15, %39
  br i1 %40, label %64, label %41

41:                                               ; preds = %38
  %42 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %15, i64 0, i32 2
  %43 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %42, align 8
  %44 = load %"MemoryManager"*, %"MemoryManager"** %12, align 8
  %45 = bitcast %"MemoryManager"* %44 to void (%"MemoryManager"*, i8*)***
  %46 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %45, align 8
  %47 = bitcast void (%"MemoryManager"*, i8*)** %46 to i8*
  %48 = tail call i1 @llvm.type.test(i8* %47, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %48)
  %49 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %15 to i8*
  %50 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %46, i64 3
  %51 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %50, align 8
  %52 = bitcast void (%"MemoryManager"*, i8*)* %51 to i8*
  %53 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %54 = icmp eq i8* %52, %53
  br i1 %54, label %55, label %56

55:                                               ; preds = %41
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %44, i8* nonnull %49)
          to label %57 unwind label %62

56:                                               ; preds = %41
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %44, i8* nonnull %49)
          to label %57 unwind label %62

57:                                               ; preds = %56, %55
  br label %58

58:                                               ; preds = %57
  %59 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %6, align 8
  br label %13

60:                                               ; preds = %30, %28
  %61 = landingpad { i8*, i32 }
          catch i8* null
  br label %104

62:                                               ; preds = %56, %55
  %63 = landingpad { i8*, i32 }
          catch i8* null
  br label %104

64:                                               ; preds = %38
  %65 = getelementptr inbounds %XalanList, %XalanList* %5, i64 0, i32 2
  %66 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %65, align 8
  br label %67

67:                                               ; preds = %86, %64
  %68 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %66, %64 ], [ %75, %86 ]
  %69 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %68, null
  %70 = load %"MemoryManager"*, %"MemoryManager"** %12, align 8
  %71 = bitcast %"MemoryManager"* %70 to void (%"MemoryManager"*, i8*)***
  %72 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %71, align 8
  br i1 %69, label %89, label %73

73:                                               ; preds = %67
  %74 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %68, i64 0, i32 2
  %75 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %74, align 8
  %76 = bitcast void (%"MemoryManager"*, i8*)** %72 to i8*
  %77 = tail call i1 @llvm.type.test(i8* %76, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %77)
  %78 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %68 to i8*
  %79 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %72, i64 3
  %80 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %79, align 8
  %81 = bitcast void (%"MemoryManager"*, i8*)* %80 to i8*
  %82 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %83 = icmp eq i8* %81, %82
  br i1 %83, label %84, label %85

84:                                               ; preds = %73
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %70, i8* nonnull %78)
          to label %86 unwind label %87

85:                                               ; preds = %73
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %70, i8* nonnull %78)
          to label %86 unwind label %87

86:                                               ; preds = %85, %84
  br label %67

87:                                               ; preds = %85, %84
  %88 = landingpad { i8*, i32 }
          catch i8* null
  br label %104

89:                                               ; preds = %67
  %90 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %6 to i8**
  %91 = load i8*, i8** %90, align 8
  %92 = bitcast void (%"MemoryManager"*, i8*)** %72 to i8*
  %93 = tail call i1 @llvm.type.test(i8* %92, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %93)
  %94 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %72, i64 3
  %95 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %94, align 8
  %96 = bitcast void (%"MemoryManager"*, i8*)* %95 to i8*
  %97 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %98 = icmp eq i8* %96, %97
  br i1 %98, label %99, label %100

99:                                               ; preds = %89
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %70, i8* %91)
          to label %101 unwind label %102

100:                                              ; preds = %89
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %70, i8* %91)
          to label %101 unwind label %102

101:                                              ; preds = %100, %99
  br label %107

102:                                              ; preds = %100, %99
  %103 = landingpad { i8*, i32 }
          catch i8* null
  br label %104

104:                                              ; preds = %102, %87, %62, %60
  %105 = phi { i8*, i32 } [ %63, %62 ], [ %61, %60 ], [ %88, %87 ], [ %103, %102 ]
  %106 = extractvalue { i8*, i32 } %105, 0
  tail call void @__clang_call_terminate(i8* %106)
  unreachable

107:                                              ; preds = %101, %4
  ret void

108:                                              ; preds = %1
  %109 = landingpad { i8*, i32 }
          catch i8* null
  %110 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %0, i64 0, i32 2
  %111 = getelementptr inbounds %XalanList, %XalanList* %110, i64 0, i32 1
  %112 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %111, align 8
  %113 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %112, null
  br i1 %113, label %212, label %114

114:                                              ; preds = %108
  %115 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %112, i64 0, i32 2
  %116 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %115, align 8
  %117 = getelementptr inbounds %XalanList, %XalanList* %110, i64 0, i32 0
  br label %118

118:                                              ; preds = %163, %114
  %119 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %112, %114 ], [ %164, %163 ]
  %120 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %116, %114 ], [ %148, %163 ]
  %121 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %119, null
  br i1 %121, label %122, label %143

122:                                              ; preds = %118
  %123 = load %"MemoryManager"*, %"MemoryManager"** %117, align 8
  %124 = bitcast %"MemoryManager"* %123 to i8* (%"MemoryManager"*, i64)***
  %125 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %124, align 8
  %126 = bitcast i8* (%"MemoryManager"*, i64)** %125 to i8*
  %127 = tail call i1 @llvm.type.test(i8* %126, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %127)
  %128 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %125, i64 2
  %129 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %128, align 8
  %130 = bitcast i8* (%"MemoryManager"*, i64)* %129 to i8*
  %131 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %132 = icmp eq i8* %130, %131
  br i1 %132, label %133, label %135

133:                                              ; preds = %122
  %134 = invoke i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %123, i64 24)
          to label %137 unwind label %165

135:                                              ; preds = %122
  %136 = invoke i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %123, i64 24)
          to label %137 unwind label %165

137:                                              ; preds = %135, %133
  %138 = phi i8* [ %134, %133 ], [ %136, %135 ]
  br label %139

139:                                              ; preds = %137
  %140 = bitcast i8* %138 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %140, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %111, align 8
  %141 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %140, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %140, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %141, align 8
  %142 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %140, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %140, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %142, align 8
  br label %143

143:                                              ; preds = %139, %118
  %144 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %140, %139 ], [ %119, %118 ]
  %145 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %120, %144
  br i1 %145, label %169, label %146

146:                                              ; preds = %143
  %147 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %120, i64 0, i32 2
  %148 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %147, align 8
  %149 = load %"MemoryManager"*, %"MemoryManager"** %117, align 8
  %150 = bitcast %"MemoryManager"* %149 to void (%"MemoryManager"*, i8*)***
  %151 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %150, align 8
  %152 = bitcast void (%"MemoryManager"*, i8*)** %151 to i8*
  %153 = tail call i1 @llvm.type.test(i8* %152, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %153)
  %154 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %120 to i8*
  %155 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %151, i64 3
  %156 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %155, align 8
  %157 = bitcast void (%"MemoryManager"*, i8*)* %156 to i8*
  %158 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %159 = icmp eq i8* %157, %158
  br i1 %159, label %160, label %161

160:                                              ; preds = %146
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %149, i8* nonnull %154)
          to label %162 unwind label %167

161:                                              ; preds = %146
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %149, i8* nonnull %154)
          to label %162 unwind label %167

162:                                              ; preds = %161, %160
  br label %163

163:                                              ; preds = %162
  %164 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %111, align 8
  br label %118

165:                                              ; preds = %135, %133
  %166 = landingpad { i8*, i32 }
          catch i8* null
  br label %209

167:                                              ; preds = %161, %160
  %168 = landingpad { i8*, i32 }
          catch i8* null
  br label %209

169:                                              ; preds = %143
  %170 = getelementptr inbounds %XalanList, %XalanList* %110, i64 0, i32 2
  %171 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %170, align 8
  br label %172

172:                                              ; preds = %191, %169
  %173 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %171, %169 ], [ %180, %191 ]
  %174 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %173, null
  %175 = load %"MemoryManager"*, %"MemoryManager"** %117, align 8
  %176 = bitcast %"MemoryManager"* %175 to void (%"MemoryManager"*, i8*)***
  %177 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %176, align 8
  br i1 %174, label %194, label %178

178:                                              ; preds = %172
  %179 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %173, i64 0, i32 2
  %180 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %179, align 8
  %181 = bitcast void (%"MemoryManager"*, i8*)** %177 to i8*
  %182 = tail call i1 @llvm.type.test(i8* %181, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %182)
  %183 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %173 to i8*
  %184 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %177, i64 3
  %185 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %184, align 8
  %186 = bitcast void (%"MemoryManager"*, i8*)* %185 to i8*
  %187 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %188 = icmp eq i8* %186, %187
  br i1 %188, label %189, label %190

189:                                              ; preds = %178
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %175, i8* nonnull %183)
          to label %191 unwind label %192

190:                                              ; preds = %178
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %175, i8* nonnull %183)
          to label %191 unwind label %192

191:                                              ; preds = %190, %189
  br label %172

192:                                              ; preds = %190, %189
  %193 = landingpad { i8*, i32 }
          catch i8* null
  br label %209

194:                                              ; preds = %172
  %195 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %111 to i8**
  %196 = load i8*, i8** %195, align 8
  %197 = bitcast void (%"MemoryManager"*, i8*)** %177 to i8*
  %198 = tail call i1 @llvm.type.test(i8* %197, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %198)
  %199 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %177, i64 3
  %200 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %199, align 8
  %201 = bitcast void (%"MemoryManager"*, i8*)* %200 to i8*
  %202 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %203 = icmp eq i8* %201, %202
  br i1 %203, label %204, label %205


204:                                              ; preds = %194
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %175, i8* %196)
          to label %206 unwind label %207

205:                                              ; preds = %194
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %175, i8* %196)
          to label %206 unwind label %207

206:                                              ; preds = %205, %204
  br label %212

207:                                              ; preds = %205, %204
  %208 = landingpad { i8*, i32 }
          catch i8* null
  br label %209

209:                                              ; preds = %207, %192, %167, %165
  %210 = phi { i8*, i32 } [ %168, %167 ], [ %166, %165 ], [ %193, %192 ], [ %208, %207 ]
  %211 = extractvalue { i8*, i32 } %210, 0
  tail call void @__clang_call_terminate(i8* %211)
  unreachable

212:                                              ; preds = %206, %108
  %213 = extractvalue { i8*, i32 } %109, 0
  tail call void @__clang_call_terminate(i8* %213)
  unreachable
}

; Function Attrs: uwtable
define internal zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0, %"XStringCached"* %1) align 2 {
; identifyIteratorCheck will handle from first basicblock to %34.
  %3 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %0, i64 0, i32 0
  %4 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %3, i64 0, i32 2
  %5 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 1
  %6 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %7 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %6, null
  br i1 %7, label %8, label %30

8:                                                ; preds = %2
; identifyCreateListHead will handle code from this block to 26.
  %9 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %10 = load %"MemoryManager"*, %"MemoryManager"** %9, align 8
  %11 = bitcast %"MemoryManager"* %10 to i8* (%"MemoryManager"*, i64)***
  %12 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %11, align 8
  %13 = bitcast i8* (%"MemoryManager"*, i64)** %12 to i8*
  %14 = tail call i1 @llvm.type.test(i8* %13, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %14)
  %15 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %12, i64 2
  %16 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %15, align 8
  %17 = bitcast i8* (%"MemoryManager"*, i64)* %16 to i8*
  %18 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %19 = icmp eq i8* %17, %18
  br i1 %19, label %20, label %22

20:                                               ; preds = %8
  %21 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %10, i64 24)
  br label %24

22:                                               ; preds = %8
  %23 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %10, i64 24)
  br label %24

24:                                               ; preds = %22, %20
  %25 = phi i8* [ %21, %20 ], [ %23, %22 ]
  br label %26

26:                                               ; preds = %24
  %27 = bitcast i8* %25 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %27, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %28 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %27, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %27, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %28, align 8
  %29 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %27, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %27, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %29, align 8
  br label %508


30:                                               ; preds = %2
  %31 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %6, i64 0, i32 2
  %32 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %31, align 8
  %33 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %32, %6
  br i1 %33, label %508, label %34

34:                                               ; preds = %250, %30
; NotEmptyBB:
; identifyBlockAvailable will handle this block.
  %35 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %252, %250 ], [ %32, %30 ]
  %36 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35 to %"ArenaBlockBase"**
  %37 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %36, align 8
  %38 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %37, i64 0, i32 1
  %39 = load i16, i16* %38, align 8
  %40 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %37, i64 0, i32 2
  %41 = load i16, i16* %40, align 2
  %42 = icmp ult i16 %39, %41
  br i1 %42, label %43, label %254

43:                                               ; preds = %34
; BlockAvailableBB:
; identifyOwnsBlock will handle blocks 43 and 47.
  %44 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %37, i64 0, i32 3
  %45 = load %"XStringCached"*, %"XStringCached"** %44, align 8
  %46 = icmp ugt %"XStringCached"* %45, %1
  br i1 %46, label %250, label %47

47:                                               ; preds = %43
  %48 = zext i16 %41 to i64
  %49 = getelementptr inbounds %"XStringCached", %"XStringCached"* %45, i64 %48
  %50 = icmp ugt %"XStringCached"* %49, %1
  br i1 %50, label %51, label %250

51:                                               ; preds = %47
; OwnsBB
; identifyRABDestroyObject will handle from this block to 65.
  %52 = bitcast %"ArenaBlockBase"* %37 to %"ReusableArenaBlock"*
  %53 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 0
  %54 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %52, i64 0, i32 1
  %55 = load i16, i16* %54, align 8
  %56 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %52, i64 0, i32 2
  %57 = load i16, i16* %56, align 2
  %58 = icmp eq i16 %55, %57
  br i1 %58, label %65, label %59

59:                                               ; preds = %51
  %60 = zext i16 %55 to i64
  %61 = getelementptr inbounds %"XStringCached", %"XStringCached"* %45, i64 %60
  %62 = bitcast %"XStringCached"* %61 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %63 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %62, i64 0, i32 0
  store i16 %57, i16* %63, align 4
  %64 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %62, i64 0, i32 1
  store i32 -2228259, i32* %64, align 4
  store i16 %55, i16* %56, align 2
  br label %65


65:                                               ; preds = %59, %51
  %66 = bitcast %"XStringCached"* %1 to void (%"XStringCached"*)***
  %67 = load void (%"XStringCached"*)**, void (%"XStringCached"*)*** %66, align 8
  %68 = bitcast void (%"XStringCached"*)** %67 to i8*
  %69 = tail call i1 @llvm.type.test(i8* %68, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %69)
  %70 = load void (%"XStringCached"*)*, void (%"XStringCached"*)** %67, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull dereferenceable(80) %1)
  %71 = bitcast %"XStringCached"* %1 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %72 = load i16, i16* %54, align 8
  %73 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %71, i64 0, i32 0
  store i16 %72, i16* %73, align 4
  %74 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %71, i64 0, i32 1
  store i32 -2228259, i32* %74, align 4
  %75 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %52, i64 0, i32 0
  %76 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %75, i64 0, i32 3
  %77 = load %"XStringCached"*, %"XStringCached"** %76, align 8
  %78 = ptrtoint %"XStringCached"* %1 to i64
  %79 = ptrtoint %"XStringCached"* %77 to i64
  %80 = sub i64 %78, %79
  %81 = sdiv exact i64 %80, 80
  %82 = trunc i64 %81 to i16
  store i16 %82, i16* %56, align 2
  store i16 %82, i16* %54, align 8
  %83 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %75, i64 0, i32 1
  %84 = load i16, i16* %83, align 8
  %85 = add i16 %84, -1
  store i16 %85, i16* %83, align 8
; identifyGetListHead will handle from here to 109.
  %86 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %87 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %86, null
  br i1 %87, label %91, label %88

88:                                               ; preds = %65
  %89 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %86, i64 0, i32 2
  %90 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %89, align 8
  br label %113

91:                                               ; preds = %65
  %92 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %93 = load %"MemoryManager"*, %"MemoryManager"** %92, align 8
  %94 = bitcast %"MemoryManager"* %93 to i8* (%"MemoryManager"*, i64)***
  %95 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %94, align 8
  %96 = bitcast i8* (%"MemoryManager"*, i64)** %95 to i8*
  %97 = tail call i1 @llvm.type.test(i8* %96, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %97)
  %98 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %95, i64 2
  %99 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %98, align 8
  %100 = bitcast i8* (%"MemoryManager"*, i64)* %99 to i8*
  %101 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %102 = icmp eq i8* %100, %101
  br i1 %102, label %103, label %105


103:                                              ; preds = %91
  %104 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %93, i64 24)
  br label %107

105:                                              ; preds = %91
  %106 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %93, i64 24)
  br label %107

107:                                              ; preds = %105, %103
  %108 = phi i8* [ %104, %103 ], [ %106, %105 ]
  br label %109

109:                                              ; preds = %107
  %110 = bitcast i8* %108 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %110, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %111 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %110, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %110, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %111, align 8
  %112 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %110, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %110, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %112, align 8
  br label %113

113:                                              ; preds = %109, %88
  %114 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %110, %109 ], [ %90, %88 ]
  %115 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, %114
  br i1 %115, label %191, label %116

116:                                              ; preds = %113
; identifyMoveBlock will handle from 116 to 180
; identifyFreeNode will handle most of the code in this block.
  %117 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %53, align 8
  %118 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 2
  %119 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %118, align 8
  %120 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 1
  %121 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %120, align 8
  %122 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %121, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %119, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %122, align 8
  %123 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %118, align 8
  %124 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %123, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %121, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %124, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %120, align 8
  %125 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 2
  %126 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %125, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %126, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %118, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %125, align 8
  %127 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %128 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %127, null
  br i1 %128, label %132, label %129

129:                                              ; preds = %116
  %130 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %127, i64 0, i32 2
  %131 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %130, align 8
  br label %156

132:                                              ; preds = %116
  %133 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %134 = load %"MemoryManager"*, %"MemoryManager"** %133, align 8
  %135 = bitcast %"MemoryManager"* %134 to i8* (%"MemoryManager"*, i64)***
  %136 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %135, align 8
  %137 = bitcast i8* (%"MemoryManager"*, i64)** %136 to i8*
  %138 = tail call i1 @llvm.type.test(i8* %137, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %138)
  %139 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %136, i64 2
  %140 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %139, align 8
  %141 = bitcast i8* (%"MemoryManager"*, i64)* %140 to i8*
  %142 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %143 = icmp eq i8* %141, %142
  br i1 %143, label %144, label %146

144:                                              ; preds = %132
  %145 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %134, i64 24)
  br label %148

146:                                              ; preds = %132
  %147 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %134, i64 24)
  br label %148

148:                                              ; preds = %146, %144
  %149 = phi i8* [ %145, %144 ], [ %147, %146 ]
  br label %150

150:                                              ; preds = %148
  %151 = bitcast i8* %149 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %151, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %152 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %151, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %151, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %152, align 8
  %153 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %151, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %151, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %153, align 8
  %154 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %125, align 8
  %155 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %154, null
  br i1 %155, label %161, label %156

156:                                              ; preds = %150, %129
  %157 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %131, %129 ], [ %151, %150 ]
  %158 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %35, %129 ], [ %154, %150 ]
  %159 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %158, i64 0, i32 2
  %160 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %159, align 8
  br label %180

161:                                              ; preds = %150
  %162 = load %"MemoryManager"*, %"MemoryManager"** %133, align 8
  %163 = bitcast %"MemoryManager"* %162 to i8* (%"MemoryManager"*, i64)***
  %164 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %163, align 8
  %165 = bitcast i8* (%"MemoryManager"*, i64)** %164 to i8*
  %166 = tail call i1 @llvm.type.test(i8* %165, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %166)
  %167 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %164, i64 2
  %168 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %167, align 8
  %169 = bitcast i8* (%"MemoryManager"*, i64)* %168 to i8*
  %170 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %171 = icmp eq i8* %169, %170
  br i1 %171, label %172, label %174

172:                                              ; preds = %161
  %173 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %162, i64 24)
  br label %176

174:                                              ; preds = %161
  %175 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %162, i64 24)
  br label %176

176:                                              ; preds = %174, %172
  %177 = phi i8* [ %173, %172 ], [ %175, %174 ]
  br label %178

178:                                              ; preds = %176
  %179 = bitcast i8* %177 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %180

180:                                              ; preds = %178, %156
; identifyPushAtPos will handle most the code in this block.
  %181 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %157, %156 ], [ %151, %178 ]
  %182 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %158, %156 ], [ %179, %178 ]
  %183 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %160, %156 ], [ null, %178 ]
  %184 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %182, i64 0, i32 0
  store %"ReusableArenaBlock"* %117, %"ReusableArenaBlock"** %184, align 8
  %185 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %182, i64 0, i32 1
  %186 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %181, i64 0, i32 1
  %187 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %186, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %187, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %185, align 8
  %188 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %182, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %181, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %188, align 8
  %189 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %186, align 8
  %190 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %189, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %182, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %190, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %182, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %186, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %183, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %125, align 8
  br label %191

191:                                              ; preds = %180, %113
; identifyDestroyBlock will handle from this block to 242.
; CheckDestroyBlocksFlag will handle this block.
  %192 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %0, i64 0, i32 1
  %193 = load i8, i8* %192, align 8
  %194 = icmp eq i8 %193, 0
  br i1 %194, label %254, label %195

195:                                              ; preds = %191
; CheckListHead will handle from here to 220.
  %196 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %197 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %196, null
  br i1 %197, label %198, label %220

198:                                              ; preds = %195
  %199 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %200 = load %"MemoryManager"*, %"MemoryManager"** %199, align 8
  %201 = bitcast %"MemoryManager"* %200 to i8* (%"MemoryManager"*, i64)***
  %202 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %201, align 8
  %203 = bitcast i8* (%"MemoryManager"*, i64)** %202 to i8*
  %204 = tail call i1 @llvm.type.test(i8* %203, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %204)
  %205 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %202, i64 2
  %206 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %205, align 8
  %207 = bitcast i8* (%"MemoryManager"*, i64)* %206 to i8*
  %208 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %209 = icmp eq i8* %207, %208
  br i1 %209, label %210, label %212

210:                                              ; preds = %198
  %211 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %200, i64 24)
  br label %214

212:                                              ; preds = %198
  %213 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %200, i64 24)
  br label %214

214:                                              ; preds = %212, %210
  %215 = phi i8* [ %211, %210 ], [ %213, %212 ]
  br label %216

216:                                              ; preds = %214
  %217 = bitcast i8* %215 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %217, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %218 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %217, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %217, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %218, align 8
  %219 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %217, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %217, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %219, align 8
  br label %254

220:                                              ; preds = %195
  %221 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %196, i64 0, i32 2
  %222 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %221, align 8
  %223 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %222, %196
  br i1 %223, label %254, label %224

224:                                              ; preds = %220
; IsEmpty will handle this block.
  %225 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %222 to %"ArenaBlockBase"**
  %226 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %225, align 8
  %227 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %226, i64 0, i32 1
  %228 = load i16, i16* %227, align 8
  %229 = icmp eq i16 %228, 0
  br i1 %229, label %230, label %254

230:                                              ; preds = %224
; IsIterBlocksEnd will handle this block and 234.
  %231 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %222, i64 0, i32 2
  %232 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %231, align 8
  %233 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %232, %196
  br i1 %233, label %242, label %234

234:                                              ; preds = %230
; identifyBlockAvailable will handle this block.
  %235 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %232 to %"ArenaBlockBase"**
  %236 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %235, align 8
  %237 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %236, i64 0, i32 1
  %238 = load i16, i16* %237, align 8
  %239 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %236, i64 0, i32 2
  %240 = load i16, i16* %239, align 2
  %241 = icmp ult i16 %238, %240
  br i1 %241, label %242, label %254

242:                                              ; preds = %234, %230
; identifyFreeNode will handle this.
  %243 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %222, i64 0, i32 1
  %244 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %243, align 8
  %245 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 2
  %246 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %245, align 8
  %247 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %244, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %232, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %247, align 8
  %248 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %231, align 8
  %249 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %248, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %244, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %249, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %243, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %246, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %231, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %222, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %245, align 8
  br label %254

250:                                              ; preds = %47, %43
; DoesNotOwnBB
; CheckLoopLatch will handle this block.
  %251 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %35, i64 0, i32 2
  %252 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %251, align 8
  %253 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %252, %6
  br i1 %253, label %254, label %34

254:                                              ; preds = %250, %242, %234, %224, %220, %216, %191, %34
; LoopEndBB
; CollectLoopExitValues will handle this block.
  %255 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %35, %191 ], [ %35, %216 ], [ %35, %220 ], [ %35, %224 ], [ %35, %234 ], [ %35, %242 ], [ %6, %250 ], [ %35, %34 ]
  %256 = phi i1 [ false, %191 ], [ false, %216 ], [ false, %220 ], [ false, %224 ], [ false, %234 ], [ false, %242 ], [ true, %250 ], [ true, %34 ]
  %257 = phi i8 [ 1, %191 ], [ 1, %216 ], [ 1, %220 ], [ 1, %224 ], [ 1, %234 ], [ 1, %242 ], [ 0, %250 ], [ 0, %34 ]
  %258 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %259 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %258, null
  br i1 %259, label %263, label %260

260:                                              ; preds = %254
; CheckReverseIteratorLoop will handle from this to 367.
; identifyGetRBeginREnd will handle from here to 285.
  %261 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %258, i64 0, i32 2
  %262 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %261, align 8
  br label %285

263:                                              ; preds = %254
  %264 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %265 = load %"MemoryManager"*, %"MemoryManager"** %264, align 8
  %266 = bitcast %"MemoryManager"* %265 to i8* (%"MemoryManager"*, i64)***
  %267 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %266, align 8
  %268 = bitcast i8* (%"MemoryManager"*, i64)** %267 to i8*
  %269 = tail call i1 @llvm.type.test(i8* %268, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %269)
  %270 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %267, i64 2
  %271 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %270, align 8
  %272 = bitcast i8* (%"MemoryManager"*, i64)* %271 to i8*
  %273 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %274 = icmp eq i8* %272, %273
  br i1 %274, label %275, label %277

275:                                              ; preds = %263
  %276 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %265, i64 24)
  br label %279

277:                                              ; preds = %263
  %278 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %265, i64 24)
  br label %279

279:                                              ; preds = %277, %275
  %280 = phi i8* [ %276, %275 ], [ %278, %277 ]
  br label %281

281:                                              ; preds = %279
  %282 = bitcast i8* %280 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %282, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %283 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %282, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %282, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %283, align 8
  %284 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %282, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %282, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %284, align 8
  br label %285

285:                                              ; preds = %281, %260
  %286 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %282, %281 ], [ %258, %260 ]
  %287 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %282, %281 ], [ %262, %260 ]
  %288 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %255, i64 0, i32 0
  br i1 %256, label %289, label %504

289:                                              ; preds = %307, %285
  %290 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %294, %307 ], [ %286, %285 ]
  %291 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %290, %287
  br i1 %291, label %504, label %292

292:                                              ; preds = %289
; identifyOwnsBlock will handle this and 301.
  %293 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %290, i64 0, i32 1
  %294 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %293, align 8
  %295 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %294 to %"ArenaBlockBase"**
  %296 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %295, align 8
  %297 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %296, i64 0, i32 3
  %298 = load %"XStringCached"*, %"XStringCached"** %297, align 8
  %299 = icmp ugt %"XStringCached"* %298, %1
  %300 = bitcast %"ArenaBlockBase"* %296 to %"ReusableArenaBlock"*
  br i1 %299, label %307, label %301

301:                                              ; preds = %292
  %302 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %296, i64 0, i32 2
  %303 = load i16, i16* %302, align 2
  %304 = zext i16 %303 to i64
  %305 = getelementptr inbounds %"XStringCached", %"XStringCached"* %298, i64 %304
  %306 = icmp ugt %"XStringCached"* %305, %1
  br i1 %306, label %310, label %307

307:                                              ; preds = %301, %292
; Loop Latch for reverse iterator loop.
  %308 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %288, align 8
  %309 = icmp eq %"ReusableArenaBlock"* %308, %300
  br i1 %309, label %504, label %289

310:                                              ; preds = %301
; identifyRABDestroyObject will handle from here to 322.
  %311 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %300, i64 0, i32 1
  %312 = load i16, i16* %311, align 8
  %313 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %300, i64 0, i32 2
  %314 = load i16, i16* %313, align 2
  %315 = icmp eq i16 %312, %314
  br i1 %315, label %322, label %316

316:                                              ; preds = %310
  %317 = zext i16 %312 to i64
  %318 = getelementptr inbounds %"XStringCached", %"XStringCached"* %298, i64 %317
  %319 = bitcast %"XStringCached"* %318 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %320 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %319, i64 0, i32 0
  store i16 %314, i16* %320, align 4
  %321 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %319, i64 0, i32 1
  store i32 -2228259, i32* %321, align 4
  store i16 %312, i16* %313, align 2
  br label %322

322:                                              ; preds = %316, %310
  %323 = bitcast %"XStringCached"* %1 to void (%"XStringCached"*)***
  %324 = load void (%"XStringCached"*)**, void (%"XStringCached"*)*** %323, align 8
  %325 = bitcast void (%"XStringCached"*)** %324 to i8*
  %326 = tail call i1 @llvm.type.test(i8* %325, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %326)
  %327 = load void (%"XStringCached"*)*, void (%"XStringCached"*)** %324, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull dereferenceable(80) %1)
  %328 = bitcast %"XStringCached"* %1 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %329 = load i16, i16* %311, align 8
  %330 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %328, i64 0, i32 0
  store i16 %329, i16* %330, align 4
  %331 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %328, i64 0, i32 1
  store i32 -2228259, i32* %331, align 4
  %332 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %300, i64 0, i32 0
  %333 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %332, i64 0, i32 3
  %334 = load %"XStringCached"*, %"XStringCached"** %333, align 8
  %335 = ptrtoint %"XStringCached"* %1 to i64
  %336 = ptrtoint %"XStringCached"* %334 to i64
  %337 = sub i64 %335, %336
  %338 = sdiv exact i64 %337, 80
  %339 = trunc i64 %338 to i16
  store i16 %339, i16* %313, align 2
  store i16 %339, i16* %311, align 8
  %340 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %332, i64 0, i32 1
  %341 = load i16, i16* %340, align 8
  %342 = add i16 %341, -1
  store i16 %342, i16* %340, align 8
; identifyGetRBegin will handle from here to 367.
  %343 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %344 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %343, null
  br i1 %344, label %345, label %367

345:                                              ; preds = %322
  %346 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %347 = load %"MemoryManager"*, %"MemoryManager"** %346, align 8
  %348 = bitcast %"MemoryManager"* %347 to i8* (%"MemoryManager"*, i64)***
  %349 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %348, align 8
  %350 = bitcast i8* (%"MemoryManager"*, i64)** %349 to i8*
  %351 = tail call i1 @llvm.type.test(i8* %350, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %351)
  %352 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %349, i64 2
  %353 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %352, align 8
  %354 = bitcast i8* (%"MemoryManager"*, i64)* %353 to i8*
  %355 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %356 = icmp eq i8* %354, %355
  br i1 %356, label %357, label %359

357:                                              ; preds = %345
  %358 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %347, i64 24)
  br label %361

359:                                              ; preds = %345
  %360 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %347, i64 24)
  br label %361

361:                                              ; preds = %359, %357
  %362 = phi i8* [ %358, %357 ], [ %360, %359 ]
  br label %363

363:                                              ; preds = %361
  %364 = bitcast i8* %362 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %364, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %365 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %364, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %364, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %365, align 8
  %366 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %364, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %364, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %366, align 8
  br label %367

367:                                              ; preds = %363, %322
; IsIteratorBlocksBegin will handle this.
  %368 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %364, %363 ], [ %343, %322 ]
  %369 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %290, %368
  br i1 %369, label %445, label %370

370:                                              ; preds = %367
; identifyMoveBlock will handle from this block to 434.
; identifyFreeNode will handle this block.
  %371 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %288, align 8
  %372 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %255, i64 0, i32 2
  %373 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %372, align 8
  %374 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %255, i64 0, i32 1
  %375 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %374, align 8
  %376 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %375, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %373, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %376, align 8
  %377 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %372, align 8
  %378 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %377, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %375, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %378, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %374, align 8
  %379 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 2
  %380 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %379, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %380, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %372, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %255, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %379, align 8
  %381 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %382 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %381, null
  br i1 %382, label %386, label %383

383:                                              ; preds = %370
  %384 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %381, i64 0, i32 2
  %385 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %384, align 8
  br label %410

386:                                              ; preds = %370
  %387 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %388 = load %"MemoryManager"*, %"MemoryManager"** %387, align 8
  %389 = bitcast %"MemoryManager"* %388 to i8* (%"MemoryManager"*, i64)***
  %390 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %389, align 8
  %391 = bitcast i8* (%"MemoryManager"*, i64)** %390 to i8*
  %392 = tail call i1 @llvm.type.test(i8* %391, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %392)
  %393 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %390, i64 2
  %394 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %393, align 8
  %395 = bitcast i8* (%"MemoryManager"*, i64)* %394 to i8*
  %396 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %397 = icmp eq i8* %395, %396
  br i1 %397, label %398, label %400

398:                                              ; preds = %386
  %399 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %388, i64 24)
  br label %402

400:                                              ; preds = %386
  %401 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %388, i64 24)
  br label %402

402:                                              ; preds = %400, %398
  %403 = phi i8* [ %399, %398 ], [ %401, %400 ]
  br label %404

404:                                              ; preds = %402
  %405 = bitcast i8* %403 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %405, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %406 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %405, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %405, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %406, align 8
  %407 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %405, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %405, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %407, align 8
  %408 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %379, align 8
  %409 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %408, null
  br i1 %409, label %415, label %410

410:                                              ; preds = %404, %383
  %411 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %385, %383 ], [ %405, %404 ]
  %412 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %255, %383 ], [ %408, %404 ]
  %413 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %412, i64 0, i32 2
  %414 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %413, align 8
  br label %434

415:                                              ; preds = %404
  %416 = load %"MemoryManager"*, %"MemoryManager"** %387, align 8
  %417 = bitcast %"MemoryManager"* %416 to i8* (%"MemoryManager"*, i64)***
  %418 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %417, align 8
  %419 = bitcast i8* (%"MemoryManager"*, i64)** %418 to i8*
  %420 = tail call i1 @llvm.type.test(i8* %419, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %420)
  %421 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %418, i64 2
  %422 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %421, align 8
  %423 = bitcast i8* (%"MemoryManager"*, i64)* %422 to i8*
  %424 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %425 = icmp eq i8* %423, %424
  br i1 %425, label %426, label %428

426:                                              ; preds = %415
  %427 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %416, i64 24)
  br label %430

428:                                              ; preds = %415
  %429 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %416, i64 24)
  br label %430

430:                                              ; preds = %428, %426
  %431 = phi i8* [ %427, %426 ], [ %429, %428 ]
  br label %432

432:                                              ; preds = %430
  %433 = bitcast i8* %431 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %434

434:                                              ; preds = %432, %410
; identifyPushAtPos will handle this block.
  %435 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %411, %410 ], [ %405, %432 ]
  %436 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %412, %410 ], [ %433, %432 ]
  %437 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %414, %410 ], [ null, %432 ]
  %438 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %436, i64 0, i32 0
  store %"ReusableArenaBlock"* %371, %"ReusableArenaBlock"** %438, align 8
  %439 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %436, i64 0, i32 1
  %440 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %435, i64 0, i32 1
  %441 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %440, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %441, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %439, align 8
  %442 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %436, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %435, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %442, align 8
  %443 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %440, align 8
  %444 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %443, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %436, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %444, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %436, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %440, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %437, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %379, align 8
  br label %445

445:                                              ; preds = %434, %367
; identifyDestroyBlock will handle from this block to 498.
; CheckDestroyBlocksFlag will handle this block.
  %446 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %0, i64 0, i32 1
  %447 = load i8, i8* %446, align 8
  %448 = icmp eq i8 %447, 0
  br i1 %448, label %504, label %449

449:                                              ; preds = %445
; CheckListHead will handle from here to 474.
  %450 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %451 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %450, null
  br i1 %451, label %452, label %474

452:                                              ; preds = %449
  %453 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %454 = load %"MemoryManager"*, %"MemoryManager"** %453, align 8
  %455 = bitcast %"MemoryManager"* %454 to i8* (%"MemoryManager"*, i64)***
  %456 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %455, align 8
  %457 = bitcast i8* (%"MemoryManager"*, i64)** %456 to i8*
  %458 = tail call i1 @llvm.type.test(i8* %457, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %458)
  %459 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %456, i64 2
  %460 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %459, align 8
  %461 = bitcast i8* (%"MemoryManager"*, i64)* %460 to i8*
  %462 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %463 = icmp eq i8* %461, %462
  br i1 %463, label %464, label %466

464:                                              ; preds = %452
  %465 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %454, i64 24)
  br label %468

466:                                              ; preds = %452
  %467 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %454, i64 24)
  br label %468

468:                                              ; preds = %466, %464
  %469 = phi i8* [ %465, %464 ], [ %467, %466 ]
  br label %470

470:                                              ; preds = %468
  %471 = bitcast i8* %469 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %471, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %472 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %471, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %471, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %472, align 8
  %473 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %471, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %471, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %473, align 8
  br label %504

474:                                              ; preds = %449
  %475 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %450, i64 0, i32 2
  %476 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %475, align 8
  %477 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %476, %450
  br i1 %477, label %504, label %478

478:                                              ; preds = %474
; IsEmpty will handle this.
  %479 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %476 to %"ArenaBlockBase"**
  %480 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %479, align 8
  %481 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %480, i64 0, i32 1
  %482 = load i16, i16* %481, align 8
  %483 = icmp eq i16 %482, 0
  br i1 %483, label %484, label %504

484:                                              ; preds = %478
; IsIterBlocksEnd will handle this block and 488.
  %485 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %476, i64 0, i32 2
  %486 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %485, align 8
  %487 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %486, %450
  br i1 %487, label %496, label %488

488:                                              ; preds = %484
; identifyBlockAvailable will handle this.
  %489 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %486 to %"ArenaBlockBase"**
  %490 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %489, align 8
  %491 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %490, i64 0, i32 1
  %492 = load i16, i16* %491, align 8
  %493 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %490, i64 0, i32 2
  %494 = load i16, i16* %493, align 2
  %495 = icmp ult i16 %492, %494
  br i1 %495, label %496, label %504

496:                                              ; preds = %488, %484
; identifyFreeNode will handle this block.
  %497 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %476, i64 0, i32 1
  %498 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %497, align 8
  %499 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 2
  %500 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %499, align 8
  %501 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %498, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %486, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %501, align 8
  %502 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %485, align 8
  %503 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %502, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %498, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %503, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %497, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %500, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %485, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %476, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %499, align 8
  br label %504

504:                                              ; preds = %496, %488, %478, %474, %470, %445, %307, %289, %285
; RLoopEnd
; CheckReverseIteratorLoop will handle this.
  %505 = phi i8 [ 1, %445 ], [ 1, %470 ], [ 1, %474 ], [ 1, %478 ], [ 1, %488 ], [ 1, %496 ], [ %257, %285 ], [ 1, %289 ], [ %257, %307 ]
  %506 = and i8 %505, 1
  %507 = icmp ne i8 %506, 0
  br label %508

508:                                              ; preds = %504, %30, %26
; RetBB
  %509 = phi i1 [ %507, %504 ], [ false, %30 ], [ false, %26 ]
  ret i1 %509
}

; Function Attrs: uwtable
define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nocapture nonnull dereferenceable(40) %0) unnamed_addr align 2 personality i32 (...)* @__gxx_personality_v0 {
  %2 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %0, i64 0, i32 2
  %3 = getelementptr inbounds %XalanList, %XalanList* %2, i64 0, i32 1
  %4 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %3, align 8
  %5 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %4, null
  br i1 %5, label %6, label %28

6:                                                ; preds = %1
  %7 = getelementptr inbounds %XalanList, %XalanList* %2, i64 0, i32 0
  %8 = load %"MemoryManager"*, %"MemoryManager"** %7, align 8
  %9 = bitcast %"MemoryManager"* %8 to i8* (%"MemoryManager"*, i64)***
  %10 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %9, align 8
  %11 = bitcast i8* (%"MemoryManager"*, i64)** %10 to i8*
  %12 = tail call i1 @llvm.type.test(i8* %11, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %12)
  %13 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %10, i64 2
  %14 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %13, align 8
  %15 = bitcast i8* (%"MemoryManager"*, i64)* %14 to i8*
  %16 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %17 = icmp eq i8* %15, %16
  br i1 %17, label %18, label %20

18:                                               ; preds = %6
  %19 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %8, i64 24)
  br label %22

20:                                               ; preds = %6
  %21 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %8, i64 24)
  br label %22

22:                                               ; preds = %20, %18
  %23 = phi i8* [ %19, %18 ], [ %21, %20 ]
  br label %24

24:                                               ; preds = %22
  %25 = bitcast i8* %23 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %3, align 8
  %26 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %25, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %26, align 8
  %27 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %25, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %25, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %27, align 8
  br label %152

28:                                               ; preds = %1
  %29 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %4, i64 0, i32 2
  %30 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %29, align 8
  %31 = getelementptr inbounds %XalanList, %XalanList* %2, i64 0, i32 0
  %32 = load %"MemoryManager"*, %"MemoryManager"** %31, align 8
  %33 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, %4
  br i1 %33, label %128, label %34

34:                                               ; preds = %28
  %35 = bitcast %"MemoryManager"* %32 to void (%"MemoryManager"*, i8*)***
  br label %36

36:                                               ; preds = %122, %34
  %37 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %30, %34 ], [ %124, %122 ]
  %38 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %37, i64 0, i32 0
  %39 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %38, align 8
  %40 = icmp eq %"ReusableArenaBlock"* %39, null
  br i1 %40, label %122, label %41

41:                                               ; preds = %36
  %42 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %39, i64 0, i32 0
  %43 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %42, i64 0, i32 1
  %44 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %42, i64 0, i32 2
  %45 = load i16, i16* %44, align 2
  %46 = icmp eq i16 %45, 0
  br i1 %46, label %55, label %47

47:                                               ; preds = %41
  %48 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %42, i64 0, i32 3
  br label %49

49:                                               ; preds = %102, %47
  %50 = phi i64 [ 0, %47 ], [ %106, %102 ]
  %51 = phi i16 [ %45, %47 ], [ %104, %102 ]
  %52 = phi i16 [ 0, %47 ], [ %105, %102 ]
  %53 = load i16, i16* %43, align 8
  %54 = icmp ult i16 %52, %53
  br i1 %54, label %79, label %55

55:                                               ; preds = %102, %49, %41
  %56 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %42, i64 0, i32 3
  %57 = load %"XStringCached"*, %"XStringCached"** %56, align 8
  %58 = icmp eq %"XStringCached"* %57, null
  br i1 %58, label %108, label %59

59:                                               ; preds = %55
  %60 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %42, i64 0, i32 0
  %61 = getelementptr inbounds %"XalanAllocator", %"XalanAllocator"* %60, i64 0, i32 0
  %62 = load %"MemoryManager"*, %"MemoryManager"** %61, align 8
  %63 = bitcast %"XStringCached"* %57 to i8*
  %64 = bitcast %"MemoryManager"* %62 to void (%"MemoryManager"*, i8*)***
  %65 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %64, align 8
  %66 = bitcast void (%"MemoryManager"*, i8*)** %65 to i8*
  %67 = tail call i1 @llvm.type.test(i8* %66, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %67)
  %68 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %65, i64 3
  %69 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %68, align 8
  %70 = bitcast void (%"MemoryManager"*, i8*)* %69 to i8*
  %71 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %72 = icmp eq i8* %70, %71
  br i1 %72, label %73, label %74

73:                                               ; preds = %59
  invoke void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %62, i8* nonnull %63)
          to label %75 unwind label %76

74:                                               ; preds = %59
  invoke void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %62, i8* nonnull %63)
          to label %75 unwind label %76

75:                                               ; preds = %74, %73
  br label %108

76:                                               ; preds = %74, %73
  %77 = landingpad { i8*, i32 }
          catch i8* null
  %78 = extractvalue { i8*, i32 } %77, 0
  tail call void @__clang_call_terminate(i8* %78)
  unreachable

79:                                               ; preds = %49
  %80 = load %"XStringCached"*, %"XStringCached"** %48, align 8
  %81 = getelementptr inbounds %"XStringCached", %"XStringCached"* %80, i64 %50
  %82 = bitcast %"XStringCached"* %81 to %"ReusableArenaBlock<XStringCached>::NextBlock"*
  %83 = zext i16 %51 to i64
  %84 = icmp ult i64 %50, %83
  br i1 %84, label %85, label %93

85:                                               ; preds = %79
  %86 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %82, i64 0, i32 1
  %87 = load i32, i32* %86, align 4
  %88 = icmp eq i32 %87, -2228259
  br i1 %88, label %89, label %93

89:                                               ; preds = %85
  %90 = getelementptr inbounds %"ReusableArenaBlock<XStringCached>::NextBlock", %"ReusableArenaBlock<XStringCached>::NextBlock"* %82, i64 0, i32 0
  %91 = load i16, i16* %90, align 4
  %92 = icmp ugt i16 %91, %51
  br i1 %92, label %93, label %102

93:                                               ; preds = %89, %85, %79
  %94 = bitcast %"XStringCached"* %81 to void (%"XStringCached"*)***
  %95 = load void (%"XStringCached"*)**, void (%"XStringCached"*)*** %94, align 8
  %96 = bitcast void (%"XStringCached"*)** %95 to i8*
  %97 = tail call i1 @llvm.type.test(i8* %96, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  tail call void @llvm.assume(i1 %97)
  %98 = load void (%"XStringCached"*)*, void (%"XStringCached"*)** %95, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull dereferenceable(80) %81)
  %99 = add nuw i16 %52, 1
  %100 = load i16, i16* %44, align 2
  %101 = zext i16 %100 to i64
  br label %102

102:                                              ; preds = %93, %89
  %103 = phi i64 [ %101, %93 ], [ %83, %89 ]
  %104 = phi i16 [ %100, %93 ], [ %51, %89 ]
  %105 = phi i16 [ %99, %93 ], [ %52, %89 ]
  %106 = add nuw nsw i64 %50, 1
  %107 = icmp ult i64 %106, %103
  br i1 %107, label %49, label %55

108:                                              ; preds = %75, %55
  %109 = bitcast %"ReusableArenaBlock"* %39 to i8*
  %110 = load void (%"MemoryManager"*, i8*)**, void (%"MemoryManager"*, i8*)*** %35, align 8
  %111 = bitcast void (%"MemoryManager"*, i8*)** %110 to i8*
  %112 = tail call i1 @llvm.type.test(i8* %111, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %112)
  %113 = getelementptr inbounds void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %110, i64 3
  %114 = load void (%"MemoryManager"*, i8*)*, void (%"MemoryManager"*, i8*)** %113, align 8
  %115 = bitcast void (%"MemoryManager"*, i8*)* %114 to i8*
  %116 = bitcast void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to i8*
  %117 = icmp eq i8* %115, %116
  br i1 %117, label %118, label %119

118:                                              ; preds = %108
  tail call void bitcast (void (%"MemoryManagerImpl"*, i8*)* @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %32, i8* nonnull %109)
  br label %120

119:                                              ; preds = %108
  tail call void bitcast (void (%"DummyMemoryManager"*, i8*)* @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv to void (%"MemoryManager"*, i8*)*)(%"MemoryManager"* nonnull dereferenceable(8) %32, i8* nonnull %109)
  br label %120

120:                                              ; preds = %119, %118
  br label %121

121:                                              ; preds = %120
  br label %122

122:                                              ; preds = %121, %36
  %123 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %37, i64 0, i32 2
  %124 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %123, align 8
  %125 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %124, %4
  br i1 %125, label %126, label %36

126:                                              ; preds = %122
  %127 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %3, align 8
  br label %128

128:                                              ; preds = %126, %28
  %129 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %127, %126 ], [ %30, %28 ]
  %130 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %129, null
  br i1 %130, label %131, label %152

131:                                              ; preds = %128
  %132 = load %"MemoryManager"*, %"MemoryManager"** %31, align 8
  %133 = bitcast %"MemoryManager"* %132 to i8* (%"MemoryManager"*, i64)***
  %134 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %133, align 8
  %135 = bitcast i8* (%"MemoryManager"*, i64)** %134 to i8*
  %136 = tail call i1 @llvm.type.test(i8* %135, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %136)
  %137 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %134, i64 2
  %138 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %137, align 8
  %139 = bitcast i8* (%"MemoryManager"*, i64)* %138 to i8*
  %140 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %141 = icmp eq i8* %139, %140
  br i1 %141, label %142, label %144

142:                                              ; preds = %131
  %143 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %132, i64 24)
  br label %146

144:                                              ; preds = %131
  %145 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %132, i64 24)
  br label %146

146:                                              ; preds = %144, %142
  %147 = phi i8* [ %143, %142 ], [ %145, %144 ]
  br label %148

148:                                              ; preds = %146
  %149 = bitcast i8* %147 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %3, align 8
  %150 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %150, align 8
  %151 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %151, align 8
  br label %172

152:                                              ; preds = %128, %24
  %153 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %25, %24 ], [ %129, %128 ]
  %154 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %153, i64 0, i32 2
  %155 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %154, align 8
  %156 = getelementptr inbounds %XalanList, %XalanList* %2, i64 0, i32 2
  %157 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %155, %153
  br i1 %157, label %172, label %158

158:                                              ; preds = %152
  %159 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %156, align 8
  br label %160

160:                                              ; preds = %160, %158
  %161 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %159, %158 ], [ %162, %160 ]
  %162 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %155, %158 ], [ %164, %160 ]
  %163 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %162, i64 0, i32 2
  %164 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %163, align 8
  %165 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %162, i64 0, i32 1
  %166 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %165, align 8
  %167 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %166, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %164, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %167, align 8
  %168 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %163, align 8
  %169 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %168, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %166, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %169, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %165, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %161, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %163, align 8
  %170 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %164, %153
  br i1 %170, label %171, label %160

171:                                              ; preds = %160
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %162, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %156, align 8
  br label %172

172:                                              ; preds = %171, %152, %148
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
  %40 = icmp ult i16 %37, %39
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
  %3 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %0, i64 0, i32 0
  %4 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %3, i64 0, i32 2
  %5 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 1
  %6 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %7 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %6, null
  br i1 %7, label %11, label %8

8:                                                ; preds = %2
  %9 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %6, i64 0, i32 2
  %10 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %9, align 8
  br label %33

11:                                               ; preds = %2
  %12 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %13 = load %"MemoryManager"*, %"MemoryManager"** %12, align 8
  %14 = bitcast %"MemoryManager"* %13 to i8* (%"MemoryManager"*, i64)***
  %15 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %14, align 8
  %16 = bitcast i8* (%"MemoryManager"*, i64)** %15 to i8*
  %17 = tail call i1 @llvm.type.test(i8* %16, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %17)
  %18 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %15, i64 2
  %19 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %18, align 8
  %20 = bitcast i8* (%"MemoryManager"*, i64)* %19 to i8*
  %21 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %22 = icmp eq i8* %20, %21
  br i1 %22, label %23, label %25

23:                                               ; preds = %11
  %24 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %13, i64 24)
  br label %27

25:                                               ; preds = %11
  %26 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %13, i64 24)
  br label %27

27:                                               ; preds = %25, %23
  %28 = phi i8* [ %24, %23 ], [ %26, %25 ]
  br label %29

29:                                               ; preds = %27
  %30 = bitcast i8* %28 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %31 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %31, align 8
  %32 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %30, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %32, align 8
  br label %33

33:                                               ; preds = %29, %8
  %34 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %30, %29 ], [ %10, %8 ]
  %35 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %34, i64 0, i32 0
  %36 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %35, align 8
  %37 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %36, i64 0, i32 2
  %38 = load i16, i16* %37, align 2
  %39 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %36, i64 0, i32 1
  store i16 %38, i16* %39, align 8
  %40 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %34 to %"ArenaBlockBase"**
  %41 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %40, align 8
  %42 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %41, i64 0, i32 1
  %43 = load i16, i16* %42, align 8
  %44 = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %41, i64 0, i32 2
  %45 = load i16, i16* %44, align 2
  %46 = icmp ult i16 %43, %45
  br i1 %46, label %151, label %47

47:                                               ; preds = %33
  %48 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %49 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %48, null
  br i1 %49, label %57, label %50

50:                                               ; preds = %47
  %51 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %48, i64 0, i32 2
  %52 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %51, align 8
  %53 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %52, i64 0, i32 2
  %54 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %53, align 8
  %55 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %52, i64 0, i32 1
  %56 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %55, align 8
  br label %79

57:                                               ; preds = %47
  %58 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %59 = load %"MemoryManager"*, %"MemoryManager"** %58, align 8
  %60 = bitcast %"MemoryManager"* %59 to i8* (%"MemoryManager"*, i64)***
  %61 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %60, align 8
  %62 = bitcast i8* (%"MemoryManager"*, i64)** %61 to i8*
  %63 = tail call i1 @llvm.type.test(i8* %62, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %63)
  %64 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %61, i64 2
  %65 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %64, align 8
  %66 = bitcast i8* (%"MemoryManager"*, i64)* %65 to i8*
  %67 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %68 = icmp eq i8* %66, %67
  br i1 %68, label %69, label %71

69:                                               ; preds = %57
  %70 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %59, i64 24)
  br label %73

71:                                               ; preds = %57
  %72 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %59, i64 24)
  br label %73

73:                                               ; preds = %71, %69
  %74 = phi i8* [ %70, %69 ], [ %72, %71 ]
  br label %75

75:                                               ; preds = %73
  %76 = bitcast i8* %74 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %76, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %77 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %76, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %76, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %77, align 8
  %78 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %76, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %76, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %78, align 8
  br label %79

79:                                               ; preds = %75, %50
  %80 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %56, %50 ], [ %76, %75 ]
  %81 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %54, %50 ], [ %76, %75 ]
  %82 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %52, %50 ], [ %76, %75 ]
  %83 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %82, i64 0, i32 0
  %84 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %83, align 8
  %85 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %82, i64 0, i32 2
  %86 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %82, i64 0, i32 1
  %87 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %80, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %81, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %87, align 8
  %88 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %85, align 8
  %89 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %88, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %80, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %89, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* null, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %86, align 8
  %90 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 2
  %91 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %90, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %91, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %85, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %82, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %90, align 8
  %92 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %93 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %92, null
  br i1 %93, label %94, label %140

94:                                               ; preds = %79
  %95 = getelementptr inbounds %XalanList, %XalanList* %4, i64 0, i32 0
  %96 = load %"MemoryManager"*, %"MemoryManager"** %95, align 8
  %97 = bitcast %"MemoryManager"* %96 to i8* (%"MemoryManager"*, i64)***
  %98 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %97, align 8
  %99 = bitcast i8* (%"MemoryManager"*, i64)** %98 to i8*
  %100 = tail call i1 @llvm.type.test(i8* %99, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %100)
  %101 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %98, i64 2
  %102 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %101, align 8
  %103 = bitcast i8* (%"MemoryManager"*, i64)* %102 to i8*
  %104 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %105 = icmp eq i8* %103, %104
  br i1 %105, label %106, label %108

106:                                              ; preds = %94
  %107 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %96, i64 24)
  br label %110

108:                                              ; preds = %94
  %109 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %96, i64 24)
  br label %110

110:                                              ; preds = %108, %106
  %111 = phi i8* [ %107, %106 ], [ %109, %108 ]
  br label %112

112:                                              ; preds = %110
  %113 = bitcast i8* %111 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %113, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %5, align 8
  %114 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %113, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %113, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %114, align 8
  %115 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %113, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %113, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %115, align 8
  %116 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %90, align 8
  %117 = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %116, null
  br i1 %117, label %121, label %118

118:                                              ; preds = %112
  %119 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %116, i64 0, i32 2
  %120 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %119, align 8
  br label %140

121:                                              ; preds = %112
  %122 = load %"MemoryManager"*, %"MemoryManager"** %95, align 8
  %123 = bitcast %"MemoryManager"* %122 to i8* (%"MemoryManager"*, i64)***
  %124 = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %123, align 8
  %125 = bitcast i8* (%"MemoryManager"*, i64)** %124 to i8*
  %126 = tail call i1 @llvm.type.test(i8* %125, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %126)
  %127 = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %124, i64 2
  %128 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %127, align 8
  %129 = bitcast i8* (%"MemoryManager"*, i64)* %128 to i8*
  %130 = bitcast i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8*
  %131 = icmp eq i8* %129, %130
  br i1 %131, label %132, label %134

132:                                              ; preds = %121
  %133 = tail call i8* bitcast (i8* (%"MemoryManagerImpl"*, i64)* @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %122, i64 24)
  br label %136

134:                                              ; preds = %121
  %135 = tail call i8* bitcast (i8* (%"DummyMemoryManager"*, i64)* @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm to i8* (%"MemoryManager"*, i64)*)(%"MemoryManager"* nonnull dereferenceable(8) %122, i64 24)
  br label %136

136:                                              ; preds = %134, %132
  %137 = phi i8* [ %133, %132 ], [ %135, %134 ]
  br label %138

138:                                              ; preds = %136
  %139 = bitcast i8* %137 to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  br label %140

140:                                              ; preds = %138, %118, %79
  %141 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %113, %138 ], [ %113, %118 ], [ %92, %79 ]
  %142 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %139, %138 ], [ %116, %118 ], [ %82, %79 ]
  %143 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ null, %138 ], [ %120, %118 ], [ %91, %79 ]
  %144 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %142, i64 0, i32 0
  store %"ReusableArenaBlock"* %84, %"ReusableArenaBlock"** %144, align 8
  %145 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %142, i64 0, i32 1
  %146 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %141, i64 0, i32 1
  %147 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %146, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %147, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %145, align 8
  %148 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %142, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %141, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %148, align 8
  %149 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %146, align 8
  %150 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %149, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %142, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %150, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %142, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %146, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %143, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %90, align 8
  br label %151

151:                                              ; preds = %140, %33
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

declare void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull, %"MemoryManager"* nonnull )

declare void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull)

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
