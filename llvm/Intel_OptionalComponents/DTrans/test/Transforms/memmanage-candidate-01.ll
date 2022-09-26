; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that %"XStringCachedAllocator" is considered as
; candidate for MemManageTrans.


; RUN: opt < %s -disable-output -dtrans-force-inline -pre-lto-inline-cost -debug-only=dtrans-memmanageinfo 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes='module(dtrans-force-inline)' -pre-lto-inline-cost -debug-only=dtrans-memmanageinfo 2>&1 | FileCheck %s


; CHECK: MemManageTrans considering candidate: %XStringCachedAllocator

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

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

declare dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"XStringCached"* nonnull dereferenceable(80), %"XPathExecutionContext::GetAndReleaseCachedString"* nonnull align 8 dereferenceable(16), %"MemoryManager"* nonnull align 8 dereferenceable(8)) unnamed_addr
declare %"MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"ArenaAllocator"* nonnull dereferenceable(40) %this)
declare void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(%"ArenaAllocator"* nonnull dereferenceable(40) %this, %"MemoryManager"* nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
declare %"XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this)
declare void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"ReusableArenaAllocator"* nonnull dereferenceable(41) %this, %"XStringCached"* %theObject)
