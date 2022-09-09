; This test verifies that MemManageTrans is not triggered since
; %"ReusableArenaAllocator" type is used in "Call" instruction in function
; "NonStringObjMemberFunction" like below.
;
; tail call void @foo(i8* null, %"ReusableArenaAllocator"* null)
;
; RUN: opt < %s -dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-memmanagetrans -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetrans -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: MemManageTrans transformation:
; CHECK:   Considering candidate: %XStringCachedAllocator
; CHECK:   Possible candidate structs:
; CHECK:       XStringCachedAllocator
; CHECK:   Analyzing Candidate ...
; CHECK:   Type unknown use: NonStringObjMemberFunction
; CHECK:   Failed: Types escaped

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

@_ZTSN11xercesc_2_713MemoryManagerE = internal constant [31 x i8] c"N11xercesc_2_713MemoryManagerE\00"

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0, %"MemoryManager"* nonnull align 8 dereferenceable(8) %1, i16 zeroext %2, i1 zeroext %3) unnamed_addr  align 2 {
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

; Function Attrs: norecurse nounwind readonly uwtable willreturn mustprogress
define internal nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nocapture nonnull readonly dereferenceable(40) %0) align 2 {
  ret %"MemoryManager"* null
}

; Function Attrs: uwtable
define internal %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nocapture nonnull dereferenceable(41) %0) unnamed_addr align 2 personality i32 (...)* @__gxx_personality_v0 {
  ret %"XStringCached"* null
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

define internal void @foo(i8* %a1, %"ReusableArenaAllocator"*) {
  ret void
}

define internal void @bar() align 2 {
  tail call void @NonStringObjMemberFunction()
  ret void
}

define internal void @NonStringObjMemberFunction() align 2 {
  tail call void @foo(i8* null, %"ReusableArenaAllocator"* null)
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

declare void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull, %"MemoryManager"* nonnull )

declare void @_ZN11xalanc_1_1013XStringCachedD2Ev(%"XStringCached"* nonnull)

declare void @__clang_call_terminate(i8*)

declare dso_local i32 @__gxx_personality_v0(...)
