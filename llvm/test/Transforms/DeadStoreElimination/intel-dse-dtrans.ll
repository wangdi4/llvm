; RUN: opt -passes=dse -S %s | FileCheck %s

; CMPLRLLVM-49478:
; This test verifies that the below store instruction is removed by DSE pass
; when routine is marked with "noinline-dtrans" attribute.
;
; "store ptr %i12, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert"

; CHECK: @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_
; CHECK-NOT: store ptr %i12, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
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
%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" = type opaque
%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" = type { %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext", ptr }
%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" = type <{ %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_ = comdat any

; Function Attrs: uwtable
define weak_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !66 {
entry:
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this, i64 0, i32 2, !intel-tbaa !68
  %m_listHead.i.i.i = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %m_blocks, i64 0, i32 1, !intel-tbaa !76
  %i = load ptr, ptr %m_listHead.i.i.i, align 8, !tbaa !76, !noalias !77
  %cmp.i.i.i = icmp eq ptr %i, null
  br i1 %cmp.i.i.i, label %if.then.i.i.i, label %entry._ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv.exit_crit_edge.i.i

entry._ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv.exit_crit_edge.i.i: ; preds = %entry
  %next.phi.trans.insert.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i, i64 0, i32 2
  %.pre.i.i = load ptr, ptr %next.phi.trans.insert.i.i, align 8, !tbaa !78, !noalias !77
  br label %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv.exit24

if.then.i.i.i:                                    ; preds = %entry
  %m_memoryManager.i.i.i.i = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %m_blocks, i64 0, i32 0, !intel-tbaa !81
  %i1 = load ptr, ptr %m_memoryManager.i.i.i.i, align 8, !tbaa !81, !noalias !82
  %vtable.i.i.i.i = load ptr, ptr %i1, align 8, !tbaa !85, !noalias !82
  %i2 = tail call i1 @llvm.intel.wholeprogramsafe(), !noalias !82
  br i1 %i2, label %whpr.wrap.i.i.i.i, label %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i

whpr.wrap.i.i.i.i:                                ; preds = %if.then.i.i.i
  %i3 = tail call i1 @llvm.public.type.test(ptr %vtable.i.i.i.i, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  br label %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i

_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i: ; preds = %whpr.wrap.i.i.i.i, %if.then.i.i.i
  %vfn.i.i.i.i = getelementptr inbounds ptr, ptr %vtable.i.i.i.i, i64 2
  %i4 = load ptr, ptr %vfn.i.i.i.i, align 8, !noalias !82
  %call.i.i.i.i = tail call noundef ptr %i4(ptr noundef nonnull align 8 dereferenceable(8) %i1, i64 noundef 24), !noalias !82, !intel_dtrans_type !87
  store ptr %call.i.i.i.i, ptr %m_listHead.i.i.i, align 8, !tbaa !76, !noalias !82
  %next.i.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call.i.i.i.i, i64 0, i32 2, !intel-tbaa !78
  store ptr %call.i.i.i.i, ptr %next.i.i.i, align 8, !tbaa !78, !noalias !82
  %prev.i.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call.i.i.i.i, i64 0, i32 1, !intel-tbaa !89
  store ptr %call.i.i.i.i, ptr %prev.i.i.i, align 8, !tbaa !89, !noalias !82
  br label %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv.exit24

_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv.exit24: ; preds = %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i, %entry._ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv.exit_crit_edge.i.i
  %i5 = phi ptr [ %call.i.i.i.i, %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i ], [ %i, %entry._ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv.exit_crit_edge.i.i ]
  %.pre.i.i13 = phi ptr [ %call.i.i.i.i, %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm.exit.i.i.i ], [ %.pre.i.i, %entry._ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv.exit_crit_edge.i.i ]
  %value.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %.pre.i.i13, i64 0, i32 0, !intel-tbaa !90
  %i6 = load ptr, ptr %value.i.i, align 8, !tbaa !90
  %m_nextFreeBlock.i = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i6, i64 0, i32 2, !intel-tbaa !91
  %i7 = load i16, ptr %m_nextFreeBlock.i, align 2, !tbaa !91
  %m_firstFreeBlock.i = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i6, i64 0, i32 1, !intel-tbaa !97
  store i16 %i7, ptr %m_firstFreeBlock.i, align 8, !tbaa !97
  %m_objectCount.i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i6, i64 0, i32 1, !intel-tbaa !98
  %i8 = load i16, ptr %m_objectCount.i, align 8, !tbaa !98
  %m_blockSize.i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i6, i64 0, i32 2, !intel-tbaa !99
  %i9 = load i16, ptr %m_blockSize.i, align 2, !tbaa !99
  %cmp.i = icmp ult i16 %i8, %i9
  br i1 %cmp.i, label %if.end, label %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_.exit

_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_.exit: ; preds = %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv.exit24
  %next.phi.trans.insert.i.i28 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i5, i64 0, i32 2
  %.pre.i.i29 = load ptr, ptr %next.phi.trans.insert.i.i28, align 8, !tbaa !78, !noalias !77
  %next.i.i3.phi.trans.insert.i.phi.trans.insert = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %.pre.i.i29, i64 0, i32 2
  %.pre.i.pre = load ptr, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !78
  %prev.i.i4.phi.trans.insert.i.phi.trans.insert = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %.pre.i.i29, i64 0, i32 1
  %.pre5.i.pre = load ptr, ptr %prev.i.i4.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !89
  %value.i.i30 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %.pre.i.i29, i64 0, i32 0, !intel-tbaa !90
  %i10 = load ptr, ptr %value.i.i30, align 8, !tbaa !90
  %next2.i.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %.pre5.i.pre, i64 0, i32 2, !intel-tbaa !78
  store ptr %.pre.i.pre, ptr %next2.i.i.i, align 8, !tbaa !78
  %i11 = load ptr, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !78
  %prev5.i.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i11, i64 0, i32 1, !intel-tbaa !89
  store ptr %.pre5.i.pre, ptr %prev5.i.i.i, align 8, !tbaa !89
  store ptr null, ptr %prev.i.i4.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !89
  %m_freeListHeadPtr.i.i.i = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %m_blocks, i64 0, i32 2, !intel-tbaa !100
  %i12 = load ptr, ptr %m_freeListHeadPtr.i.i.i, align 8, !tbaa !100
  store ptr %i12, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !78
  store ptr %i10, ptr %value.i.i30, align 8, !tbaa !101
  %prev8.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i5, i64 0, i32 1, !intel-tbaa !89
  %i13 = load ptr, ptr %prev8.i.i, align 8, !tbaa !89
  store ptr %i13, ptr %prev.i.i4.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !102
  store ptr %i5, ptr %next.i.i3.phi.trans.insert.i.phi.trans.insert, align 8, !tbaa !102
  %i14 = load ptr, ptr %prev8.i.i, align 8, !tbaa !89
  %next13.i.i = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i14, i64 0, i32 2, !intel-tbaa !78
  store ptr %.pre.i.i29, ptr %next13.i.i, align 8, !tbaa !78
  store ptr %.pre.i.i29, ptr %prev8.i.i, align 8, !tbaa !89
  store ptr %i12, ptr %m_freeListHeadPtr.i.i.i, align 8, !tbaa !100
  br label %if.end

if.end:                                           ; preds = %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_.exit, %_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv.exit24
  ret void
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.public.type.test(ptr, metadata) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #3

attributes #0 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !9, !15, !18, !19, !21, !24, !28, !31, !32, !35, !37, !39, !40, !43, !45, !46, !49, !52, !55, !56, !58, !60, !61, !64}
!llvm.ident = !{!65}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !6, !7, !8}
!6 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!7 = !{i8 0, i32 0}
!8 = !{!"A", i32 7, !7}
!9 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !10, !13, !14}
!10 = !{!11, i32 2}
!11 = !{!"F", i1 true, i32 0, !12}
!12 = !{i32 0, i32 0}
!13 = !{i16 0, i32 0}
!14 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!15 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !16, !17, !17}
!16 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!17 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 1}
!18 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !10}
!19 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 3, !20, !17, !17}
!20 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!21 = !{!"S", %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 2, !22, !23}
!22 = !{%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 0}
!23 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}
!24 = !{!"S", %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 3, !25, !26, !27}
!25 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 0}
!26 = !{double 0.000000e+00, i32 0}
!27 = !{%"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 0}
!28 = !{!"S", %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 3, !29, !12, !30}
!29 = !{%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 0}
!30 = !{%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 1}
!31 = !{!"S", %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 2, !10, !12}
!32 = !{!"S", %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 2, !33, !34}
!33 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}
!34 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 0}
!35 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !36}
!36 = !{%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 0}
!37 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1, !38}
!38 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 0}
!39 = !{!"S", %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 1, !10}
!40 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !41, !42, !16}
!41 = !{%"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 0}
!42 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 1}
!43 = !{!"S", %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 1, !44}
!44 = !{%"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 0}
!45 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 1, !38}
!46 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !47, !48}
!47 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 1}
!48 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!49 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !50, !13, !13, !51}
!50 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!51 = !{!"A", i32 4, !7}
!52 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !53, !13, !13, !54}
!53 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!54 = !{%"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 1}
!55 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !16}
!56 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 2, !57, !30}
!57 = !{%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 0}
!58 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 3, !59, !12, !51}
!59 = !{%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!60 = !{!"S", %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 2, !10, !16}
!61 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !16, !62, !62, !63}
!62 = !{i64 0, i32 0}
!63 = !{i16 0, i32 1}
!64 = !{!"S", %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 -1}
!65 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)"}
!66 = distinct !{!67, !54}
!67 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 1}
!68 = !{!69, !73, i64 16}
!69 = !{!"struct@_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE", !70, i64 8, !73, i64 16}
!70 = !{!"short", !71, i64 0}
!71 = !{!"omnipotent char", !72, i64 0}
!72 = !{!"Simple C++ TBAA"}
!73 = !{!"struct@_ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE", !74, i64 0, !75, i64 8, !75, i64 16}
!74 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !71, i64 0}
!75 = !{!"pointer@_ZTSPN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE", !71, i64 0}
!76 = !{!73, !75, i64 8}
!77 = !{}
!78 = !{!79, !75, i64 16}
!79 = !{!"struct@_ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE", !80, i64 0, !75, i64 8, !75, i64 16}
!80 = !{!"pointer@_ZTSPN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE", !71, i64 0}
!81 = !{!73, !74, i64 0}
!82 = !{!83}
!83 = distinct !{!83, !84, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!84 = distinct !{!84, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!85 = !{!86, !86, i64 0}
!86 = !{!"vtable pointer", !72, i64 0}
!87 = !{!"F", i1 false, i32 2, !88, !16, !62}
!88 = !{i8 0, i32 1}
!89 = !{!79, !75, i64 8}
!90 = !{!79, !80, i64 0}
!91 = !{!92, !70, i64 26}
!92 = !{!"struct@_ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE", !93, i64 0, !70, i64 24, !70, i64 26}
!93 = !{!"struct@_ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE", !94, i64 0, !70, i64 8, !70, i64 10, !96, i64 16}
!94 = !{!"struct@_ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE", !95, i64 0}
!95 = !{!"any pointer", !71, i64 0}
!96 = !{!"pointer@_ZTSPN11xalanc_1_1013XStringCachedE", !71, i64 0}
!97 = !{!92, !70, i64 24}
!98 = !{!93, !70, i64 8}
!99 = !{!93, !70, i64 10}
!100 = !{!73, !75, i64 16}
!101 = !{!80, !80, i64 0}
!102 = !{!75, !75, i64 0}
