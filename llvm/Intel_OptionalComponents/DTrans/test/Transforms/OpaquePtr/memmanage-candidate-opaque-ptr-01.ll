; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(dtrans-force-inline-op)' -pre-lto-inline-cost -debug-only=dtrans-memmanageinfoop 2>&1 | FileCheck %s

; This test verifies that
; %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator"
; is considered as a candidate for MemManageTrans.

; This case is similar to memmanage-candidate-01.ll, but uses the opaque pointer IR
; produced during the -c compilation phase.

; CHECK: MemManageTrans considering candidate: %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" = type { %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" }
%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" = type { %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject", double, %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" }
%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" = type { %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base", i32, ptr }
%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" = type <{ ptr, i32 }>
%"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" = type { %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" }
%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" = type { %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" }
%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" = type { ptr }
%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" = type { %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText", ptr, ptr }
%"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" = type { %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" }
%"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" = type opaque
%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" = type { %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext", ptr }
%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" = type <{ %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }

;
; The original typed pointer structures are listed here for reference:
;
; %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" }
; %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }
; %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { i32 (...)**, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" }
; %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager"*, %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
; %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock"*, %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
; %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" = type { %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", i32, [4 x i8] }
; %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager"*, i64, i64, i16* }
; %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }
; %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", i16, i16, %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached"* }
; %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager"* }
; %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" = type { %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" }
; %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" = type { %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject", double, %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" }
; %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" = type { %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base", i32, %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory"* }
; %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" = type { i32 (...)**, i32 }
; %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" = type opaque
; %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" = type { %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" }
; %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" = type { %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" }
; %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
; %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" = type { i32 (...)** }
; %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" = type { %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText", %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject"*, %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager"* }
; %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" = type { %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" }
; %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
; %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" = type { %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext"*, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString"* }
; %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" = type { %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext", %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory"* }
; %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" = type { i32 (...)**, %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager"* }
; %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { i32 (...)** }


$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb = comdat any

@_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = external dso_local unnamed_addr constant { [8 x ptr] }, align 8, !type !0, !type !1, !type !2, !type !3, !type !4, !type !5, !type !6, !type !7, !type !8, !type !9, !intel_dtrans_type !10

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockCount) unnamed_addr #0 align 2 !intel.dtrans.func.type !81 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !82
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !31
  %theBlockCount.addr = alloca i16, align 2
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockCount, ptr %theBlockCount.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theManager.addr, align 8
  %1 = load i16, ptr %theBlockCount.addr, align 2
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator, ptr noundef nonnull align 8 dereferenceable(8) %0, i16 noundef zeroext %1, i1 noundef zeroext false)
  ret void
}

; Function Attrs: uwtable
define weak_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize, i1 noundef zeroext %destroyBlocks) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !83 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !84
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !31
  %theBlockSize.addr = alloca i16, align 2
  %destroyBlocks.addr = alloca i8, align 1
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  %frombool = zext i1 %destroyBlocks to i8
  store i8 %frombool, ptr %destroyBlocks.addr, align 1
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theManager.addr, align 8
  %1 = load i16, ptr %theBlockSize.addr, align 2
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(40) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0, i16 noundef zeroext %1)
  store ptr getelementptr inbounds ({ [8 x ptr] }, ptr @_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE, i32 0, inrange i32 0, i32 2), ptr %this1, align 8
  %m_destroyBlocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %this1, i32 0, i32 1
  %2 = load i8, ptr %destroyBlocks.addr, align 1, !range !85
  %tobool = trunc i8 %2 to i1
  %frombool2 = zext i1 %tobool to i8
  store i8 %frombool2, ptr %m_destroyBlocks, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %this, ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %theValue) #1 align 2 !intel.dtrans.func.type !86 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !82
  %theValue.addr = alloca ptr, align 8, !intel_dtrans_type !87
  %theBlock = alloca ptr, align 8, !intel_dtrans_type !69
  %theResult = alloca ptr, align 8, !intel_dtrans_type !69
  store ptr %this, ptr %this.addr, align 8
  store ptr %theValue, ptr %theValue.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %theBlock) #4
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %call = call noundef ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator)
  store ptr %call, ptr %theBlock, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %theResult) #4
  %0 = load ptr, ptr %theBlock, align 8
  %1 = load ptr, ptr %theValue.addr, align 8
  %m_allocator2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) %m_allocator2)
  call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(80) %0, ptr noundef nonnull align 8 dereferenceable(16) %1, ptr noundef nonnull align 8 dereferenceable(8) %call3)
  store ptr %0, ptr %theResult, align 8
  %m_allocator4 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %2 = load ptr, ptr %theBlock, align 8
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator4, ptr noundef %2)
  %3 = load ptr, ptr %theResult, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr %theResult) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %theBlock) #4
  ret ptr %3
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !88 dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="2") unnamed_addr #1 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !89 dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2") #1 align 2

declare !intel.dtrans.func.type !91 dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(80) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="2", ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3") unnamed_addr #3

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !92 dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #1 align 2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !93 dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2", i16 noundef zeroext) unnamed_addr #0 align 2

attributes #0 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!13, !14, !15, !16, !17}
!intel.dtrans.types = !{!18, !20, !24, !30, !33, !34, !36, !39, !43, !46, !47, !50, !52, !54, !55, !58, !60, !61, !64, !67, !70, !71, !73, !75, !76, !79}
!llvm.ident = !{!80}

!0 = !{i64 16, !"_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE"}
!1 = !{i64 32, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFPS1_vE.virtual"}
!2 = !{i64 40, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFvPS1_E.virtual"}
!3 = !{i64 48, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEKFbPKS1_E.virtual"}
!4 = !{i64 56, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFvvE.virtual"}
!5 = !{i64 16, !"_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE"}
!6 = !{i64 32, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFPS1_vE.virtual"}
!7 = !{i64 40, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFvPS1_E.virtual"}
!8 = !{i64 48, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEKFbPKS1_E.virtual"}
!9 = !{i64 56, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFvvE.virtual"}
!10 = !{!"L", i32 1, !11}
!11 = !{!"A", i32 8, !12}
!12 = !{i8 0, i32 1}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 1, !"Virtual Function Elim", i32 0}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{i32 1, !"ThinLTO", i32 0}
!17 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!18 = !{!"S", %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1, !19}
!19 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!20 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !21, !22, !23}
!21 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!22 = !{i8 0, i32 0}
!23 = !{!"A", i32 7, !22}
!24 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !25, !28, !29}
!25 = !{!26, i32 2}
!26 = !{!"F", i1 true, i32 0, !27}
!27 = !{i32 0, i32 0}
!28 = !{i16 0, i32 0}
!29 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!30 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !31, !32, !32}
!31 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!32 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 1}
!33 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !25}
!34 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 3, !35, !32, !32}
!35 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!36 = !{!"S", %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 2, !37, !38}
!37 = !{%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 0}
!38 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}
!39 = !{!"S", %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 3, !40, !41, !42}
!40 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 0}
!41 = !{double 0.000000e+00, i32 0}
!42 = !{%"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 0}
!43 = !{!"S", %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 3, !44, !27, !45}
!44 = !{%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 0}
!45 = !{%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 1}
!46 = !{!"S", %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 2, !25, !27}
!47 = !{!"S", %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 2, !48, !49}
!48 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}
!49 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 0}
!50 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !51}
!51 = !{%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 0}
!52 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1, !53}
!53 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 0}
!54 = !{!"S", %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 1, !25}
!55 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !56, !57, !31}
!56 = !{%"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 0}
!57 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 1}
!58 = !{!"S", %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 1, !59}
!59 = !{%"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 0}
!60 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 1, !53}
!61 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !62, !63}
!62 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 1}
!63 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!64 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !65, !28, !28, !66}
!65 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!66 = !{!"A", i32 4, !22}
!67 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !68, !28, !28, !69}
!68 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!69 = !{%"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 1}
!70 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !31}
!71 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 2, !72, !45}
!72 = !{%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 0}
!73 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 3, !74, !27, !66}
!74 = !{%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!75 = !{!"S", %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 2, !25, !31}
!76 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !31, !77, !77, !78}
!77 = !{i64 0, i32 0}
!78 = !{i16 0, i32 1}
!79 = !{!"S", %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 -1}
!80 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!81 = distinct !{!82, !31}
!82 = !{%"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1}
!83 = distinct !{!84, !31}
!84 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 1}
!85 = !{i8 0, i8 2}
!86 = distinct !{!69, !82, !87}
!87 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}
!88 = distinct !{!69, !84}
!89 = distinct !{!31, !90}
!90 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 1}
!91 = distinct !{!69, !87, !31}
!92 = distinct !{!84, !69}
!93 = distinct !{!90, !31}
