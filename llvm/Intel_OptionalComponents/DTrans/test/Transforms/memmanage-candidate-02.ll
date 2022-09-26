; This test verifies that calls in member functions of XStringCachedAllocator
; are not inlined and other calls in member functions of ReusableArenaAllocator
; are inlined.

; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -S -dtrans-force-inline -inline -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes='module(dtrans-force-inline),cgscc(inline)' -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt
; CHECK: tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb

; CHECK: define dso_local %XStringCached* @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE
; CHECK: %call = tail call %XStringCached* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK: %call3 = tail call nonnull align 8 dereferenceable(8) %MemoryManager* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK: tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE
; CHECK: tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_

; "*getMemoryManager" is not inlined here because it is considered as
; Interface function.
; CHECK: define linkonce_odr dso_local %XStringCached* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK-NOT: tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv
; CHECK: tail call nonnull align 8 dereferenceable(8) %MemoryManager* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK-NOT: tail call %"ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt
; CHECK-NOT: call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_
; CHECK-NOT: call %"XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv

; CHECK: define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_
; CHECK-NOT: tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv
; CHECK-NOT: tail call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_
; CHECK-NOT: tail call zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv
; CHECK-NOT: tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv
; CHECK-NOT: call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_

; CHECK: define dso_local zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE
; CHECK: %call = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

; CHECK: define linkonce_odr dso_local zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_
; CHECK-NOT: call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv

; CHECK: define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv
; CHECK-NOT: tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv



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

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(%"XStringCachedAllocator"* nonnull dereferenceable(48) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockCount) unnamed_addr align 2 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %m_allocator, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockCount, i1 zeroext false)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize, i1 zeroext %destroyBlocks) unnamed_addr align 2 {
entry:
  %frombool = zext i1 %destroyBlocks to i8
  %0 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(%"ArenaAllocator"* nonnull dereferenceable(40) %0, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
  %1 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %1, align 8
  %m_destroyBlocks = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 1
  store i8 %frombool, i8* %m_destroyBlocks, align 8
  ret void
}

; Function Attrs: uwtable
define dso_local %"XStringCached"* @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(%"XStringCachedAllocator"* nonnull dereferenceable(48) %this, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %theValue) local_unnamed_addr align 2 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %this, i64 0, i32 0
  %call = tail call %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %m_allocator)
  %0 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %this, i64 0, i32 0, i32 0
  %call3 = tail call nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %0)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull dereferenceable(80) %call, %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16) %theValue, %"MemoryManager"* nonnull align 8 dereferenceable(8) %call3)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %m_allocator, %"XStringCached"* nonnull %call)
  ret %"XStringCached"* %call
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn

; Function Attrs: uwtable
define linkonce_odr dso_local %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this) unnamed_addr align 2 {
entry:
  %ref.tmp = alloca %"ReusableArenaBlock"*, align 8
  %0 = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0
  %m_blocks = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0, i32 2
  %call = tail call zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %call3 = tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  %1 = bitcast %"ReusableArenaBlock"** %call3 to %"ArenaBlockBase"**
  %2 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %1, align 8
  %call4 = tail call zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"ArenaBlockBase"* nonnull dereferenceable(24) %2)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false, %entry
  %3 = bitcast %"ReusableArenaBlock"** %ref.tmp to i8*
  %call6 = tail call nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %0)
  %m_blockSize = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0, i32 1
  %4 = load i16, i16* %m_blockSize, align 8
  %call7 = tail call %"ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(%"MemoryManager"* nonnull align 8 dereferenceable(8) %call6, i16 zeroext %4)
  store %"ReusableArenaBlock"* %call7, %"ReusableArenaBlock"** %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"XalanList"* nonnull dereferenceable(24) %m_blocks, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %ref.tmp)
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %call9 = call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  %5 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %call9, align 8
  %call10 = call %"XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(%"ReusableArenaBlock"* nonnull dereferenceable(28) %5)
  ret %"XStringCached"* %call10
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %this) local_unnamed_addr align 2 {
entry:
  %m_blocks = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %this, i64 0, i32 2
  %call = tail call nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  ret %"MemoryManager"* %call
}

declare dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull dereferenceable(80), %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16), %"MemoryManager"* nonnull align 8 dereferenceable(8)) unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this, %"XStringCached"* %theObject) unnamed_addr align 2 {
entry:
  %fullBlock = alloca %"ReusableArenaBlock"*, align 8
  %m_blocks = getelementptr inbounds %"ReusableArenaAllocator", %"ReusableArenaAllocator"* %this, i64 0, i32 0, i32 2
  %call = tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  %0 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %call, align 8
  tail call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(%"ReusableArenaBlock"* nonnull dereferenceable(28) %0, %"XStringCached"* %theObject)
  %call3 = tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  %1 = bitcast %"ReusableArenaBlock"** %call3 to %"ArenaBlockBase"**
  %2 = load %"ArenaBlockBase"*, %"ArenaBlockBase"** %1, align 8
  %call4 = tail call zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"ArenaBlockBase"* nonnull dereferenceable(24) %2)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %3 = bitcast %"ReusableArenaBlock"** %fullBlock to i8*
  %call6 = tail call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  %4 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %call6, align 8
  store %"ReusableArenaBlock"* %4, %"ReusableArenaBlock"** %fullBlock, align 8
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(%"XalanList"* nonnull dereferenceable(24) %m_blocks)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(%"XalanList"* nonnull dereferenceable(24) %m_blocks, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %fullBlock)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn

; Function Attrs: uwtable
define dso_local zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE(%"XStringCachedAllocator"* nonnull dereferenceable(48) %this, %"XStringCached"* %theString) local_unnamed_addr align 2 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %this, i64 0, i32 0
  %call = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %m_allocator, %"XStringCached"* %theString)
  ret i1 %call
}

; Function Attrs: uwtable
define linkonce_odr dso_local zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this, %"XStringCached"* %theObject) local_unnamed_addr align 2 {
entry:
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this)
  ret i1 false
}

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv(%"XStringCachedAllocator"* nonnull dereferenceable(48) %this) local_unnamed_addr align 2 {
entry:
  %0 = getelementptr inbounds %"XStringCachedAllocator", %"XStringCachedAllocator"* %this, i64 0, i32 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nonnull dereferenceable(40) %0)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"ArenaAllocator"* nonnull dereferenceable(40) %this) unnamed_addr align 2 {
entry:
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(%"ArenaAllocator"* nonnull dereferenceable(40) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 {
entry:
  %0 = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %this, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* null, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0, align 8
  %m_blockSize = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %this, i64 0, i32 1
  store i16 %theBlockSize, i16* %m_blockSize, align 8
  %m_blocks = getelementptr inbounds %"ArenaAllocator", %"ArenaAllocator"* %this, i64 0, i32 2
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"XalanList"* nonnull dereferenceable(24) %m_blocks, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"XalanList"* nonnull dereferenceable(24) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager) unnamed_addr align 2 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 0
  store %"MemoryManager"* %theManager, %"MemoryManager"** %m_memoryManager, align 8
  %m_listHead = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 1
  %0 = bitcast %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_listHead to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(16) %0, i8 0, i64 16, i1 false)
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8* %0) local_unnamed_addr {
  %2 = tail call i8* @__cxa_begin_catch(i8* %0)
  tail call void @_ZSt9terminatev()
  unreachable
}

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanListIteratorBase"* noalias sret(%"XalanListIteratorBase") align 8 %agg.result, %"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this)
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, i64 0, i32 2
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next, align 8
  tail call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase"* nonnull dereferenceable(8) %agg.result, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %0)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"XalanListIteratorBase"* noalias sret(%"XalanListIteratorBase") align 8 %agg.result, %"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this)
  tail call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase"* nonnull dereferenceable(8) %agg.result, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %this) local_unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", %"XalanListIteratorBase"* %this, i64 0, i32 0
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %m_listHead = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 1
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_listHead, align 8
  %cmp = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"XalanList"* nonnull dereferenceable(24) %this, i64 1)
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_listHead, align 8
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next, align 8
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %1 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %call, %if.then ], [ %0, %entry ]
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase"* nonnull dereferenceable(8) %this, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %node) unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", %"XalanListIteratorBase"* %this, i64 0, i32 0
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %node, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"XalanList"* nonnull dereferenceable(24) %this, i64 %size) local_unnamed_addr align 2 {
entry:
  %mul = mul i64 %size, 24
  %m_memoryManager = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 0
  %0 = load %"MemoryManager"*, %"MemoryManager"** %m_memoryManager, align 8
  %1 = bitcast %"MemoryManager"* %0 to i8* (%"MemoryManager"*, i64)***
  %vtable = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %1, align 8
  %vfn = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vtable, i64 2
  %2 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vfn, align 8
  %call = tail call i8* %2(%"MemoryManager"* nonnull dereferenceable(8) %0, i64 %mul)
  %3 = bitcast i8* %call to %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %3
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %ref.tmp = alloca %"XalanListIteratorBase.0", align 8
  %ref.tmp2 = alloca %"XalanListIteratorBase.0", align 8
  %0 = bitcast %"XalanListIteratorBase.0"* %ref.tmp to i8*
  %call = tail call %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanList"* nonnull dereferenceable(24) %this)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %ref.tmp, i64 0, i32 0
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %coerce.dive, align 8
  %1 = bitcast %"XalanListIteratorBase.0"* %ref.tmp2 to i8*
  %call3 = tail call %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"XalanList"* nonnull dereferenceable(24) %this)
  %coerce.dive4 = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %ref.tmp2, i64 0, i32 0
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call3, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %coerce.dive4, align 8
  %call5 = call zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %ref.tmp, %"XalanListIteratorBase.0"* nonnull align 8 dereferenceable(8) %ref.tmp2)
  ret i1 %call5
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"ArenaBlockBase"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %m_objectCount = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 1
  %0 = load i16, i16* %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 2
  %1 = load i16, i16* %m_blockSize, align 2
  %cmp = icmp ult i16 %0, %1
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(%"XalanList"* nonnull dereferenceable(24) %this, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data) local_unnamed_addr align 2 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"XalanListIteratorBase"* nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, %"XalanList"* nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"XalanList"* nonnull dereferenceable(24) %this, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data, %"XalanListIteratorBase"* nonnull %agg.tmp)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(%"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize) local_unnamed_addr align 2 {
entry:
  %theBlockSize.addr = alloca i16, align 2
  %theInstance = alloca %"ReusableArenaBlock"*, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %0 = bitcast %"ReusableArenaBlock"** %theInstance to i8*
  %call = call %"ReusableArenaBlock"* @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(%"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %theInstance, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16* nonnull align 2 dereferenceable(2) %theBlockSize.addr)
  ret %"ReusableArenaBlock"* %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local %"XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(%"ReusableArenaBlock"* nonnull dereferenceable(28) %this) local_unnamed_addr align 2 {
entry:
  ret %"XStringCached"* null
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %retval = alloca %"XalanListIteratorBase.0", align 8
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this)
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, i64 0, i32 2
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %retval, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %0)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %retval, i64 0, i32 0
  %1 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %this, %"XalanListIteratorBase.0"* nonnull align 8 dereferenceable(8) %theRhs) local_unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %this, i64 0, i32 0
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  %currentNode2 = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %theRhs, i64 0, i32 0
  %1 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode2, align 8
  %cmp = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, %1
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %retval = alloca %"XalanListIteratorBase.0", align 8
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %retval, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %call)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %retval, i64 0, i32 0
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"XalanList"* nonnull dereferenceable(24) %this)
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %this, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %node) unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %this, i64 0, i32 0
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %node, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %this) local_unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", %"XalanListIteratorBase"* %this, i64 0, i32 0
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  %value = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, i64 0, i32 0
  ret %"ReusableArenaBlock"** %value
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"XalanList"* nonnull dereferenceable(24) %this, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data, %"XalanListIteratorBase"* %pos) local_unnamed_addr align 2 {
entry:
  %m_freeListHeadPtr = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 2
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  %cmp.not = icmp eq %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, null
  br i1 %cmp.not, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, i64 0, i32 2
  %1 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %call = tail call %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"XalanList"* nonnull dereferenceable(24) %this, i64 1)
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %newNode.0 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %0, %if.then ], [ %call, %if.else ]
  %nextFreeNode.0 = phi %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* [ %1, %if.then ], [ null, %if.else ]
  %value = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0, i64 0, i32 0
  %m_memoryManager = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 0
  %2 = load %"MemoryManager"*, %"MemoryManager"** %m_memoryManager, align 8
  %call6 = tail call %"ReusableArenaBlock"** @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(%"ReusableArenaBlock"** %value, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data, %"MemoryManager"* nonnull align 8 dereferenceable(8) %2)
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0, i64 0, i32 1
  %call7 = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %pos)
  %prev8 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call7, i64 0, i32 1
  %3 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev8, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %3, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev, align 8
  %next9 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0, i64 0, i32 2
  %call10 = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %pos)
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call10, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next9, align 8
  %call11 = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %pos)
  %prev12 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call11, i64 0, i32 1
  %4 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev12, align 8
  %next13 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %4, i64 0, i32 2
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %next13, align 8
  %call14 = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %pos)
  %prev15 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %call14, i64 0, i32 1
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev15, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %nextFreeNode.0, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %newNode.0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local %"ReusableArenaBlock"** @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(%"ReusableArenaBlock"** %address, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %theRhs, %"MemoryManager"* nonnull align 8 dereferenceable(8) %0) local_unnamed_addr align 2 {
entry:
  %1 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %theRhs, align 8
  store %"ReusableArenaBlock"* %1, %"ReusableArenaBlock"** %address, align 8
  ret %"ReusableArenaBlock"** %address
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"ReusableArenaBlock"* @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(%"MemoryManager"* nonnull align 8 dereferenceable(8) %theMemoryManager, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %theInstance, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theParam1, i16* nonnull align 2 dereferenceable(2) %theParam2) local_unnamed_addr personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %theGuard = alloca %"XalanAllocationGuard", align 8
  %0 = bitcast %"XalanAllocationGuard"* %theGuard to i8*
  call void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(%"XalanAllocationGuard"* nonnull dereferenceable(16) %theGuard, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theMemoryManager, i64 32)
  %call = call i8* @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(%"XalanAllocationGuard"* nonnull dereferenceable(16) %theGuard)
  %1 = bitcast i8* %call to %"ReusableArenaBlock"*
  %2 = load i16, i16* %theParam2, align 2
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"ReusableArenaBlock"* nonnull dereferenceable(28) %1, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theParam1, i16 zeroext %2)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %entry
  %3 = bitcast %"ReusableArenaBlock"** %theInstance to i8**
  store i8* %call, i8** %3, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(%"XalanAllocationGuard"* nonnull dereferenceable(16) %theGuard)
  %4 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %theInstance, align 8
  ret %"ReusableArenaBlock"* %4

lpad:                                             ; preds = %entry
  %5 = landingpad { i8*, i32 }
          cleanup
  resume { i8*, i32 } %5
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(%"XalanAllocationGuard"* nonnull dereferenceable(16) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theMemoryManager, i64 %theSize) unnamed_addr align 2 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocationGuard", %"XalanAllocationGuard"* %this, i64 0, i32 0
  store %"MemoryManager"* %theMemoryManager, %"MemoryManager"** %m_memoryManager, align 8
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", %"XalanAllocationGuard"* %this, i64 0, i32 1
  %0 = bitcast %"MemoryManager"* %theMemoryManager to i8* (%"MemoryManager"*, i64)***
  %vtable = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %0, align 8
  %vfn = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vtable, i64 2
  %1 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vfn, align 8
  %call = tail call i8* %1(%"MemoryManager"* nonnull dereferenceable(8) %theMemoryManager, i64 %theSize)
  store i8* %call, i8** %m_pointer, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local i8* @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(%"XalanAllocationGuard"* nonnull dereferenceable(16) %this) local_unnamed_addr align 2 {
entry:
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", %"XalanAllocationGuard"* %this, i64 0, i32 1
  %0 = load i8*, i8** %m_pointer, align 8
  ret i8* %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"ReusableArenaBlock"* nonnull dereferenceable(28) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 {
entry:
  %0 = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"ArenaBlockBase"* nonnull dereferenceable(24) %0, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
  %m_firstFreeBlock = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %this, i64 0, i32 1
  store i16 0, i16* %m_firstFreeBlock, align 8
  %m_nextFreeBlock = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %this, i64 0, i32 2
  store i16 0, i16* %m_nextFreeBlock, align 2
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(%"XalanAllocationGuard"* nonnull dereferenceable(16) %this) local_unnamed_addr align 2 {
entry:
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", %"XalanAllocationGuard"* %this, i64 0, i32 1
  store i8* null, i8** %m_pointer, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"ArenaBlockBase"* nonnull dereferenceable(24) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %m_allocator = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(%"XalanAllocator"* nonnull dereferenceable(8) %m_allocator, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager)
  %m_objectCount = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 1
  store i16 0, i16* %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 2
  store i16 %theBlockSize, i16* %m_blockSize, align 2
  %conv = zext i16 %theBlockSize to i64
  %call = invoke %"XStringCached"* @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(%"XalanAllocator"* nonnull dereferenceable(8) %m_allocator, i64 %conv, i8* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %m_objectBlock = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 3
  store %"XStringCached"* %call, %"XStringCached"** %m_objectBlock, align 8
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(%"XalanAllocator"* nonnull dereferenceable(8) %m_allocator)
  resume { i8*, i32 } %0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(%"XalanAllocator"* nonnull dereferenceable(8) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager) unnamed_addr align 2 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocator", %"XalanAllocator"* %this, i64 0, i32 0
  store %"MemoryManager"* %theManager, %"MemoryManager"** %m_memoryManager, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local %"XStringCached"* @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(%"XalanAllocator"* nonnull dereferenceable(8) %this, i64 %size, i8* %0) local_unnamed_addr align 2 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocator", %"XalanAllocator"* %this, i64 0, i32 0
  %1 = load %"MemoryManager"*, %"MemoryManager"** %m_memoryManager, align 8
  %mul = mul i64 %size, 80
  %2 = bitcast %"MemoryManager"* %1 to i8* (%"MemoryManager"*, i64)***
  %vtable = load i8* (%"MemoryManager"*, i64)**, i8* (%"MemoryManager"*, i64)*** %2, align 8
  %vfn = getelementptr inbounds i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vtable, i64 2
  %3 = load i8* (%"MemoryManager"*, i64)*, i8* (%"MemoryManager"*, i64)** %vfn, align 8
  %call = tail call i8* %3(%"MemoryManager"* nonnull dereferenceable(8) %1, i64 %mul)
  %4 = bitcast i8* %call to %"XStringCached"*
  ret %"XStringCached"* %4
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(%"XalanAllocator"* nonnull dereferenceable(8) %this) unnamed_addr align 2 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(%"ReusableArenaBlock"* nonnull dereferenceable(28) %this, %"XStringCached"* %0) local_unnamed_addr align 2 {
entry:
  %m_nextFreeBlock = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %this, i64 0, i32 2
  %1 = load i16, i16* %m_nextFreeBlock, align 2
  %m_firstFreeBlock = getelementptr inbounds %"ReusableArenaBlock", %"ReusableArenaBlock"* %this, i64 0, i32 1
  store i16 %1, i16* %m_firstFreeBlock, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"XalanListIteratorBase.0"* nonnull dereferenceable(8) %this) local_unnamed_addr align 2 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", %"XalanListIteratorBase.0"* %this, i64 0, i32 0
  %0 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %0, i64 0, i32 1
  %1 = load %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %prev, align 8
  store %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %1, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"** %currentNode, align 8
  ret %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(%"ArenaBlockBase"* nonnull dereferenceable(24) %this, %"XStringCached"* %theObject, i16 zeroext %rightBoundary) local_unnamed_addr align 2 {
entry:
  %functor = alloca %"struct.std::less", align 1
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 2
  %0 = load i16, i16* %m_blockSize, align 2
  %1 = getelementptr inbounds %"struct.std::less", %"struct.std::less"* %functor, i64 0, i32 0
  %m_objectBlock = getelementptr inbounds %"ArenaBlockBase", %"ArenaBlockBase"* %this, i64 0, i32 3
  %2 = load %"XStringCached"*, %"XStringCached"** %m_objectBlock, align 8
  %call = call zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* nonnull dereferenceable(1) %functor, %"XStringCached"* %theObject, %"XStringCached"* %2)
  br i1 %call, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %cmp = icmp ult i16 %0, %rightBoundary
  %spec.select = select i1 %cmp, i16 %0, i16 %rightBoundary
  %3 = load %"XStringCached"*, %"XStringCached"** %m_objectBlock, align 8
  %idx.ext = zext i16 %spec.select to i64
  %add.ptr = getelementptr inbounds %"XStringCached", %"XStringCached"* %3, i64 %idx.ext
  %call8 = call zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* nonnull dereferenceable(1) %functor, %"XStringCached"* %theObject, %"XStringCached"* %add.ptr)
  br i1 %call8, label %cleanup, label %if.else

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %cleanup

cleanup:                                          ; preds = %land.lhs.true, %if.else
  %retval.0 = phi i1 [ false, %if.else ], [ true, %land.lhs.true ]
  ret i1 %retval.0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* nonnull dereferenceable(1) %this, %"XStringCached"* %__x, %"XStringCached"* %__y) local_unnamed_addr align 2 {
entry:
  %cmp = icmp ult %"XStringCached"* %__x, %__y
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %ref.tmp = alloca %"XalanListIteratorBase", align 8
  %0 = bitcast %"XalanListIteratorBase"* %ref.tmp to i8*
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanListIteratorBase"* nonnull sret(%"XalanListIteratorBase") align 8 %ref.tmp, %"XalanList"* nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(8) %"ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %ref.tmp)
  ret %"ReusableArenaBlock"** %call
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"XalanList"* nonnull dereferenceable(24) %this, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data) local_unnamed_addr align 2 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanListIteratorBase"* nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, %"XalanList"* nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"XalanList"* nonnull dereferenceable(24) %this, %"ReusableArenaBlock"** nonnull align 8 dereferenceable(8) %data, %"XalanListIteratorBase"* nonnull %agg.tmp)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) %"MemoryManager"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanList", %"XalanList"* %this, i64 0, i32 0
  %0 = load %"MemoryManager"*, %"MemoryManager"** %m_memoryManager, align 8
  ret %"MemoryManager"* %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(%"XalanList"* nonnull dereferenceable(24) %this) local_unnamed_addr align 2 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"XalanListIteratorBase"* nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, %"XalanList"* nonnull dereferenceable(24) %this)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"XalanList"* nonnull dereferenceable(24) %this, %"XalanListIteratorBase"* nonnull %agg.tmp)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"XalanList"* nonnull dereferenceable(24) %this, %"XalanListIteratorBase"* %pos) local_unnamed_addr align 2 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"XalanListIteratorBase"* nonnull dereferenceable(8) %pos)
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(%"XalanList"* nonnull dereferenceable(24) %this, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(%"XalanList"* nonnull dereferenceable(24) %this, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* nonnull align 8 dereferenceable(24) %node) local_unnamed_addr align 2 {
entry:
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this) local_unnamed_addr align 2 {
entry:
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
