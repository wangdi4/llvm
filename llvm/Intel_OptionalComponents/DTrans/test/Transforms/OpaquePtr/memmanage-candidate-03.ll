; RUN: opt < %s -opaque-pointers -S -passes='lto-pre-link<O3>' -enable-npm-dtrans -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -inline-report=0xe807 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that inlining for the DTrans memory management optimization
; occurs on the compile step an LTO compilation.

; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(
; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(
; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(
; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(

; CHECK: Begin Inlining Report
; CHECK: INLINE: {{.*}}inline to enable DTrans
; CHECK: End Inlining Report

%"XStringCachedAllocator" = type { %"ReusableArenaAllocator" }
%"ReusableArenaAllocator" = type <{ %"ArenaAllocator", i8, [7 x i8] }>
%"ArenaAllocator" = type { ptr, i16, %"XalanList" }
%"XalanList" = type { ptr, ptr, ptr }
%"XalanList<ReusableArenaBlock<XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"ReusableArenaBlock" = type <{ %"ArenaBlockBase", i16, i16, [4 x i8] }>
%"ArenaBlockBase" = type { %"XalanAllocator", i16, i16, ptr }
%"XalanAllocator" = type { ptr }
%"XStringCached" = type { %"XStringBase", %"XPathExecutionContext::GetAndReleaseCachedString" }
%"XStringBase" = type { %"XObject", double, %"XObjectResultTreeFragProxy" }
%"XObject" = type { %"XalanReferenceCountedObject.base", i32, ptr }
%"XalanReferenceCountedObject.base" = type <{ ptr, i32 }>
%"XObjectFactory" = type opaque
%"XObjectResultTreeFragProxy" = type { %"XObjectResultTreeFragProxyBase", %"XObjectResultTreeFragProxyText" }
%"XObjectResultTreeFragProxyBase" = type { %"XalanDocumentFragment" }
%"XalanDocumentFragment" = type { %"XalanNode" }
%"XalanNode" = type { ptr }
%"XObjectResultTreeFragProxyText" = type { %"XalanText", ptr, ptr }
%"XalanText" = type { %"XalanCharacterData" }
%"XalanCharacterData" = type { %"XalanNode" }
%"XPathExecutionContext::GetAndReleaseCachedString" = type { ptr, %"XalanDOMString"* }
%"XPathExecutionContext" = type { %"ExecutionContext", ptr }
%"ExecutionContext" = type { ptr, ptr }
%"XalanDOMString" = type <{ %"XalanVector", i32, [4 x i8] }>
%"XalanVector" = type { ptr, i64, i64, ptr }
%"MemoryManager" = type { ptr }
%"XalanListIteratorBase" = type { ptr }
%"class.std::reverse_iterator.1" = type { %"XalanListIteratorBase" }
%"DeleteFunctor" = type { ptr }
%"XalanListIteratorBase.0" = type { ptr }
%"class.std::reverse_iterator" = type { %"XalanListIteratorBase.0" }
%"XalanAllocationGuard" = type { ptr, i8* }
%"ReusableArenaBlock<XStringCached>::NextBlock" = type { i16, i32 }
%"struct.std::less" = type { i8 }
%"XalanDestroyFunctor" = type { i8 }

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockCount) unnamed_addr align 2 !intel.dtrans.func.type !41 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nonnull dereferenceable(41) %m_allocator, ptr nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockCount, i1 zeroext false)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockSize, i1 zeroext %destroyBlocks) unnamed_addr align 2 !intel.dtrans.func.type !43 {
entry:
  %frombool = zext i1 %destroyBlocks to i8
  %0 = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(40) %0, ptr nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
  %1 = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0, i32 0
  store ptr bitcast (ptr getelementptr inbounds ({ [8 x ptr] }, ptr null, i64 0, inrange i32 0, i64 2) to ptr), ptr %1, align 8
  %m_destroyBlocks = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 1
  store i8 %frombool, ptr %m_destroyBlocks, align 8
  ret void
}

; Function Attrs: uwtable
define dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(ptr nonnull dereferenceable(48) "intel_dtrans_func_index"="2" %this, ptr nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %theValue) local_unnamed_addr align 2 !intel.dtrans.func.type !45 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0
  %call = tail call ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nonnull dereferenceable(41) %m_allocator)
  %0 = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0, i32 0
  %call3 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nonnull dereferenceable(40) %0)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(80) %call, ptr nonnull align 8 dereferenceable(16) %theValue, ptr nonnull align 8 dereferenceable(8) %call3)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nonnull dereferenceable(41) %m_allocator, ptr nonnull %call)
  ret ptr %call
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nonnull dereferenceable(41) "intel_dtrans_func_index"="2" %this) unnamed_addr align 2 !intel.dtrans.func.type !46 {
entry:
  %ref.tmp = alloca ptr, align 8, !intel_dtrans_type !12
  %0 = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0
  %m_blocks = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0, i32 2
  %call = tail call zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr nonnull dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %call3 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  %1 = bitcast ptr %call3 to ptr
  %2 = load ptr, ptr %1, align 8
  %call4 = tail call zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr nonnull dereferenceable(24) %2)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false, %entry
  %3 = bitcast ptr %ref.tmp to i8*
  %call6 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nonnull dereferenceable(40) %0)
  %m_blockSize = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0, i32 1
  %4 = load i16, ptr %m_blockSize, align 8
  %call7 = tail call ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(ptr nonnull align 8 dereferenceable(8) %call6, i16 zeroext %4)
  store ptr %call7, ptr %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr nonnull dereferenceable(24) %m_blocks, ptr nonnull align 8 dereferenceable(8) %ref.tmp)
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %call9 = call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  %5 = load ptr, ptr %call9, align 8
  %call10 = call ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(ptr nonnull dereferenceable(28) %5)
  ret ptr %call10
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nonnull dereferenceable(40) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !48 {
entry:
  %m_blocks = getelementptr inbounds %"ArenaAllocator", ptr %this, i64 0, i32 2
  %call = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(ptr nonnull dereferenceable(24) %m_blocks)
  ret ptr %call
}

declare !intel.dtrans.func.type !49 dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(80) "intel_dtrans_func_index"="1", ptr nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="2", ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3") unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %theObject) unnamed_addr align 2 !intel.dtrans.func.type !50 {
entry:
  %fullBlock = alloca ptr, align 8, !intel_dtrans_type !12
  %m_blocks = getelementptr inbounds %"ReusableArenaAllocator", ptr %this, i64 0, i32 0, i32 2
  %call = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  %0 = load ptr, ptr %call, align 8
  tail call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(ptr nonnull dereferenceable(28) %0, ptr %theObject)
  %call3 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  %1 = bitcast ptr %call3 to ptr
  %2 = load ptr, ptr %1, align 8
  %call4 = tail call zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr nonnull dereferenceable(24) %2)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %3 = bitcast ptr %fullBlock to i8*
  %call6 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  %4 = load ptr, ptr %call6, align 8
  store ptr %4, ptr %fullBlock, align 8
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(ptr nonnull dereferenceable(24) %m_blocks)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(ptr nonnull dereferenceable(24) %m_blocks, ptr nonnull align 8 dereferenceable(8) %fullBlock)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn

; Function Attrs: uwtable
define dso_local zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE(ptr nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %theString) local_unnamed_addr align 2 !intel.dtrans.func.type !51 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0
  %call = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr nonnull dereferenceable(41) %m_allocator, ptr %theString)
  ret i1 %call
}

; Function Attrs: uwtable
define linkonce_odr dso_local zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %theObject) local_unnamed_addr align 2 !intel.dtrans.func.type !52 {
entry:
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(ptr nonnull dereferenceable(41) %this)
  ret i1 false
}

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv(ptr nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !53 {
entry:
  %0 = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nonnull dereferenceable(40) %0)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nonnull dereferenceable(40) "intel_dtrans_func_index"="1" %this) unnamed_addr align 2 !intel.dtrans.func.type !54 {
entry:
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(40) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 !intel.dtrans.func.type !55 {
entry:
  %0 = getelementptr inbounds %"ArenaAllocator", ptr %this, i64 0, i32 0
  store ptr bitcast (ptr getelementptr inbounds ({ [8 x ptr] }, ptr null, i64 0, inrange i32 0, i64 2) to ptr), ptr %0, align 8
  %m_blockSize = getelementptr inbounds %"ArenaAllocator", ptr %this, i64 0, i32 1
  store i16 %theBlockSize, ptr %m_blockSize, align 8
  %m_blocks = getelementptr inbounds %"ArenaAllocator", ptr %this, i64 0, i32 2
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(24) %m_blocks, ptr nonnull align 8 dereferenceable(8) %theManager)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager) unnamed_addr align 2 !intel.dtrans.func.type !57 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 0
  store ptr %theManager, ptr %m_memoryManager, align 8
  %m_listHead = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 1
  %0 = bitcast ptr %m_listHead to i8*
  call void @llvm.memset.p0i8.i64(ptr nonnull align 8 dereferenceable(16) %0, i8 0, i64 16, i1 false)
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr "intel_dtrans_func_index"="1" %0) local_unnamed_addr !intel.dtrans.func.type !58 {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0)
  tail call void @_ZSt9terminatev()
  unreachable
}

; Function Attrs: nofree
declare !intel.dtrans.func.type !59 dso_local  "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2") local_unnamed_addr

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noalias sret(%"XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !61 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) %this)
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call, i64 0, i32 2
  %0 = load ptr, ptr %next, align 8
  tail call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) %agg.result, ptr nonnull align 8 dereferenceable(24) %0)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noalias sret(%"XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !62 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) %this)
  tail call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) %agg.result, ptr nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !63 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  ret ptr %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !64 {
entry:
  %m_listHead = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 1
  %0 = load ptr, ptr %m_listHead, align 8
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr nonnull dereferenceable(24) %this, i64 1)
  store ptr %call, ptr %m_listHead, align 8
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call, i64 0, i32 2
  store ptr %call, ptr %next, align 8
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call, i64 0, i32 1
  store ptr %call, ptr %prev, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %1 = phi ptr [ %call, %if.then ], [ %0, %entry ]
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(24) %node) unnamed_addr align 2 !intel.dtrans.func.type !65 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", ptr %this, i64 0, i32 0
  store ptr %node, ptr %currentNode, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this, i64 %size) local_unnamed_addr align 2 !intel.dtrans.func.type !66 {
entry:
  %mul = mul i64 %size, 24
  %m_memoryManager = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %m_memoryManager, align 8
  %1 = bitcast ptr %0 to i8* (ptr, i64)***
  %vtable = load ptr (ptr, i64)**, i8* (ptr, i64)*** %1, align 8
  %vfn = getelementptr inbounds ptr (ptr, i64)*, i8* (ptr, i64)** %vtable, i64 2
  %2 = load ptr (ptr, i64)*, i8* (ptr, i64)** %vfn, align 8
  %call = tail call ptr %2(ptr nonnull dereferenceable(8) %0, i64 %mul), !intel_dtrans_type !67
  %3 = bitcast ptr %call to ptr
  ret ptr %3
}

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !71 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1") local_unnamed_addr

; Function Attrs: uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !72 {
entry:
  %ref.tmp = alloca %"XalanListIteratorBase.0", align 8
  %ref.tmp2 = alloca %"XalanListIteratorBase.0", align 8
  %0 = bitcast ptr %ref.tmp to i8*
  %call = tail call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr nonnull dereferenceable(24) %this)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", ptr %ref.tmp, i64 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  %1 = bitcast ptr %ref.tmp2 to i8*
  %call3 = tail call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr nonnull dereferenceable(24) %this)
  %coerce.dive4 = getelementptr inbounds %"XalanListIteratorBase.0", ptr %ref.tmp2, i64 0, i32 0
  store ptr %call3, ptr %coerce.dive4, align 8
  %call5 = call zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr nonnull dereferenceable(8) %ref.tmp, ptr nonnull align 8 dereferenceable(8) %ref.tmp2)
  ret i1 %call5
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !74 {
entry:
  %m_objectCount = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 1
  %0 = load i16, ptr %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %cmp = icmp ult i16 %0, %1
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %data) local_unnamed_addr align 2 !intel.dtrans.func.type !76 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, ptr nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr nonnull dereferenceable(24) %this, ptr nonnull align 8 dereferenceable(8) %data, ptr nonnull %agg.tmp)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockSize) local_unnamed_addr align 2 !intel.dtrans.func.type !77 {
entry:
  %theBlockSize.addr = alloca i16, align 2
  %theInstance = alloca ptr, align 8, !intel_dtrans_type !12
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  %0 = bitcast ptr %theInstance to ptr
  %call = call ptr @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(ptr nonnull align 8 dereferenceable(8) %theManager, ptr nonnull align 8 dereferenceable(8) %theInstance, ptr nonnull align 8 dereferenceable(8) %theManager, ptr nonnull align 2 dereferenceable(2) %theBlockSize.addr)
  ret ptr %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(ptr nonnull dereferenceable(28) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !78 {
entry:
  ret ptr null
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !79 {
entry:
  %retval = alloca %"XalanListIteratorBase.0", align 8
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) %this)
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call, i64 0, i32 2
  %0 = load ptr, ptr %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) %retval, ptr nonnull align 8 dereferenceable(24) %0)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", ptr %retval, i64 0, i32 0
  %1 = load ptr, ptr %coerce.dive, align 8
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) local_unnamed_addr align 2 !intel.dtrans.func.type !81 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %currentNode2 = getelementptr inbounds %"XalanListIteratorBase.0", ptr %theRhs, i64 0, i32 0
  %1 = load ptr, ptr %currentNode2, align 8
  %cmp = icmp eq ptr %0, %1
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !82 {
entry:
  %retval = alloca %"XalanListIteratorBase.0", align 8
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) %this)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) %retval, ptr nonnull align 8 dereferenceable(24) %call)
  %coerce.dive = getelementptr inbounds %"XalanListIteratorBase.0", ptr %retval, i64 0, i32 0
  %0 = load ptr, ptr %coerce.dive, align 8
  ret ptr %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24)  "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !83 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr nonnull dereferenceable(24) %this)
  ret ptr %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(24) %node) unnamed_addr align 2 !intel.dtrans.func.type !84 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", ptr %this, i64 0, i32 0
  store ptr %node, ptr %currentNode, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8)  "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !85 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %value = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %0, i64 0, i32 0
  ret ptr %value
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(24)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %data, ptr "intel_dtrans_func_index"="4" %pos) local_unnamed_addr align 2 !intel.dtrans.func.type !86 {
entry:
  %m_freeListHeadPtr = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 2
  %0 = load ptr, ptr %m_freeListHeadPtr, align 8
  %cmp.not = icmp eq ptr %0, null
  br i1 %cmp.not, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %next = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %0, i64 0, i32 2
  %1 = load ptr, ptr %next, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %call = tail call ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr nonnull dereferenceable(24) %this, i64 1)
  store ptr %call, ptr %m_freeListHeadPtr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %newNode.0 = phi ptr [ %0, %if.then ], [ %call, %if.else ]
  %nextFreeNode.0 = phi ptr [ %1, %if.then ], [ null, %if.else ]
  %value = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %newNode.0, i64 0, i32 0
  %m_memoryManager = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 0
  %2 = load ptr, ptr %m_memoryManager, align 8
  %call6 = tail call ptr @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(ptr %value, ptr nonnull align 8 dereferenceable(8) %data, ptr nonnull align 8 dereferenceable(8) %2)
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %newNode.0, i64 0, i32 1
  %call7 = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) %pos)
  %prev8 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call7, i64 0, i32 1
  %3 = load ptr, ptr %prev8, align 8
  store ptr %3, ptr %prev, align 8
  %next9 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %newNode.0, i64 0, i32 2
  %call10 = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) %pos)
  store ptr %call10, ptr %next9, align 8
  %call11 = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) %pos)
  %prev12 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call11, i64 0, i32 1
  %4 = load ptr, ptr %prev12, align 8
  %next13 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %4, i64 0, i32 2
  store ptr %newNode.0, ptr %next13, align 8
  %call14 = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) %pos)
  %prev15 = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %call14, i64 0, i32 1
  store ptr %newNode.0, ptr %prev15, align 8
  store ptr %nextFreeNode.0, ptr %m_freeListHeadPtr, align 8
  ret ptr %newNode.0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(ptr "intel_dtrans_func_index"="2" %address, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theRhs, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="4" %0) local_unnamed_addr align 2 !intel.dtrans.func.type !87 {
entry:
  %1 = load ptr, ptr %theRhs, align 8
  store ptr %1, ptr %address, align 8
  ret ptr %address
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theMemoryManager, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theInstance, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="4" %theParam1, ptr nonnull align 2 dereferenceable(2) "intel_dtrans_func_index"="5" %theParam2) local_unnamed_addr personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) !intel.dtrans.func.type !88 {
entry:
  %theGuard = alloca %"XalanAllocationGuard", align 8
  %0 = bitcast ptr %theGuard to i8*
  call void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(ptr nonnull dereferenceable(16) %theGuard, ptr nonnull align 8 dereferenceable(8) %theMemoryManager, i64 32)
  %call = call ptr @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(ptr nonnull dereferenceable(16) %theGuard)
  %1 = bitcast ptr %call to ptr
  %2 = load i16, ptr %theParam2, align 2
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(28) %1, ptr nonnull align 8 dereferenceable(8) %theParam1, i16 zeroext %2)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %entry
  %3 = bitcast ptr %theInstance to ptr
  store ptr %call, ptr %3, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(ptr nonnull dereferenceable(16) %theGuard)
  %4 = load ptr, ptr %theInstance, align 8
  ret ptr %4

lpad:                                             ; preds = %entry
  %5 = landingpad { ptr, i32 }
          cleanup
  resume { ptr, i32 } %5
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(ptr nonnull dereferenceable(16) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theMemoryManager, i64 %theSize) unnamed_addr align 2 !intel.dtrans.func.type !90 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocationGuard", ptr %this, i64 0, i32 0
  store ptr %theMemoryManager, ptr %m_memoryManager, align 8
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", ptr %this, i64 0, i32 1
  %0 = bitcast ptr %theMemoryManager to ptr (ptr, i64)***
  %vtable = load ptr (ptr, i64)**, i8* (ptr, i64)*** %0, align 8
  %vfn = getelementptr inbounds ptr (ptr, i64)*, i8* (ptr, i64)** %vtable, i64 2
  %1 = load ptr (ptr, i64)*, i8* (ptr, i64)** %vfn, align 8
  %call = tail call ptr %1(ptr nonnull dereferenceable(8) %theMemoryManager, i64 %theSize), !intel_dtrans_type !67
  store ptr %call, ptr %m_pointer, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(ptr nonnull dereferenceable(16) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !91 {
entry:
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", ptr %this, i64 0, i32 1
  %0 = load ptr, ptr %m_pointer, align 8
  ret ptr %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 !intel.dtrans.func.type !92 {
entry:
  %0 = getelementptr inbounds %"ReusableArenaBlock", ptr %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(24) %0, ptr nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
  %m_firstFreeBlock = getelementptr inbounds %"ReusableArenaBlock", ptr %this, i64 0, i32 1
  store i16 0, ptr %m_firstFreeBlock, align 8
  %m_nextFreeBlock = getelementptr inbounds %"ReusableArenaBlock", ptr %this, i64 0, i32 2
  store i16 0, ptr %m_nextFreeBlock, align 2
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(ptr nonnull dereferenceable(16) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !93 {
entry:
  %m_pointer = getelementptr inbounds %"XalanAllocationGuard", ptr %this, i64 0, i32 1
  store ptr null, ptr %m_pointer, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 zeroext %theBlockSize) unnamed_addr align 2 personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) !intel.dtrans.func.type !94 {
entry:
  %m_allocator = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(8) %m_allocator, ptr nonnull align 8 dereferenceable(8) %theManager)
  %m_objectCount = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 1
  store i16 0, ptr %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 2
  store i16 %theBlockSize, ptr %m_blockSize, align 2
  %conv = zext i16 %theBlockSize to i64
  %call = invoke ptr @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(ptr nonnull dereferenceable(8) %m_allocator, i64 %conv, ptr null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %m_objectBlock = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 3
  store ptr %call, ptr %m_objectBlock, align 8
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(ptr nonnull dereferenceable(8) %m_allocator)
  resume { ptr, i32 } %0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager) unnamed_addr align 2 !intel.dtrans.func.type !96 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocator", ptr %this, i64 0, i32 0
  store ptr %theManager, ptr %m_memoryManager, align 8
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="2" %this, i64 %size, ptr "intel_dtrans_func_index"="3" %0) local_unnamed_addr align 2 !intel.dtrans.func.type !97 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanAllocator", ptr %this, i64 0, i32 0
  %1 = load ptr, ptr %m_memoryManager, align 8
  %mul = mul i64 %size, 80
  %2 = bitcast ptr %1 to i8* (ptr, i64)***
  %vtable = load ptr (ptr, i64)**, i8* (ptr, i64)*** %2, align 8
  %vfn = getelementptr inbounds ptr (ptr, i64)*, i8* (ptr, i64)** %vtable, i64 2
  %3 = load ptr (ptr, i64)*, i8* (ptr, i64)** %vfn, align 8
  %call = tail call ptr %3(ptr nonnull dereferenceable(8) %1, i64 %mul), !intel_dtrans_type !67
  %4 = bitcast ptr %call to ptr
  ret ptr %4
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr align 2 !intel.dtrans.func.type !98 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(ptr nonnull dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %0) local_unnamed_addr align 2 !intel.dtrans.func.type !99 {
entry:
  %m_nextFreeBlock = getelementptr inbounds %"ReusableArenaBlock", ptr %this, i64 0, i32 2
  %1 = load i16, ptr %m_nextFreeBlock, align 2
  %m_firstFreeBlock = getelementptr inbounds %"ReusableArenaBlock", ptr %this, i64 0, i32 1
  store i16 %1, ptr %m_firstFreeBlock, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr nonnull dereferenceable(8) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !100 {
entry:
  %currentNode = getelementptr inbounds %"XalanListIteratorBase.0", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %prev = getelementptr inbounds %"XalanList<ReusableArenaBlock<XStringCached> *>::Node", ptr %0, i64 0, i32 1
  %1 = load ptr, ptr %prev, align 8
  store ptr %1, ptr %currentNode, align 8
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %theObject, i16 zeroext %rightBoundary) local_unnamed_addr align 2 !intel.dtrans.func.type !101 {
entry:
  %functor = alloca %"struct.std::less", align 1
  %m_blockSize = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 2
  %0 = load i16, ptr %m_blockSize, align 2
  %1 = getelementptr inbounds %"struct.std::less", ptr %functor, i64 0, i32 0
  %m_objectBlock = getelementptr inbounds %"ArenaBlockBase", ptr %this, i64 0, i32 3
  %2 = load ptr, ptr %m_objectBlock, align 8
  %call = call zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr nonnull dereferenceable(1) %functor, ptr %theObject, ptr %2)
  br i1 %call, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %cmp = icmp ult i16 %0, %rightBoundary
  %spec.select = select i1 %cmp, i16 %0, i16 %rightBoundary
  %3 = load ptr, ptr %m_objectBlock, align 8
  %idx.ext = zext i16 %spec.select to i64
  %add.ptr = getelementptr inbounds %"XStringCached", ptr %3, i64 %idx.ext
  %call8 = call zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr nonnull dereferenceable(1) %functor, ptr %theObject, ptr %add.ptr)
  br i1 %call8, label %cleanup, label %if.else

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %cleanup

cleanup:                                          ; preds = %land.lhs.true, %if.else
  %retval.0 = phi i1 [ false, %if.else ], [ true, %land.lhs.true ]
  ret i1 %retval.0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr nonnull dereferenceable(1) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %__x, ptr "intel_dtrans_func_index"="3" %__y) local_unnamed_addr align 2 !intel.dtrans.func.type !103 {
entry:
  %cmp = icmp ult ptr %__x, %__y
  ret i1 %cmp
}

; Function Attrs: uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !104 {
entry:
  %ref.tmp = alloca %"XalanListIteratorBase", align 8
  %0 = bitcast ptr %ref.tmp to i8*
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr nonnull sret(%"XalanListIteratorBase") align 8 %ref.tmp, ptr nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr nonnull dereferenceable(8) %ref.tmp)
  ret ptr %call
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %data) local_unnamed_addr align 2 !intel.dtrans.func.type !105 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, ptr nonnull dereferenceable(24) %this)
  %call = call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr nonnull dereferenceable(24) %this, ptr nonnull align 8 dereferenceable(8) %data, ptr nonnull %agg.tmp)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="2" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !106 {
entry:
  %m_memoryManager = getelementptr inbounds %"XalanList", ptr %this, i64 0, i32 0
  %0 = load ptr, ptr %m_memoryManager, align 8
  ret ptr %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !107 {
entry:
  %agg.tmp = alloca %"XalanListIteratorBase", align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr nonnull sret(%"XalanListIteratorBase") align 8 %agg.tmp, ptr nonnull dereferenceable(24) %this)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr nonnull dereferenceable(24) %this, ptr nonnull %agg.tmp)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %pos) local_unnamed_addr align 2 !intel.dtrans.func.type !108 {
entry:
  %call = tail call nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr nonnull dereferenceable(8) %pos)
  tail call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(ptr nonnull dereferenceable(24) %this, ptr nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(ptr nonnull dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(24) %node) local_unnamed_addr align 2 !intel.dtrans.func.type !109 {
entry:
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(ptr nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %this) local_unnamed_addr align 2 !intel.dtrans.func.type !110 {
entry:
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare !intel.dtrans.func.type !111 void @llvm.memset.p0i8.i64(ptr nocapture "intel_dtrans_func_index"="1" writeonly, i8, i64, i1 immarg)

!1 = !{%"ReusableArenaAllocator" zeroinitializer, i32 0}  ; %"ReusableArenaAllocator"
!2 = !{%"ArenaAllocator" zeroinitializer, i32 0}  ; %"ArenaAllocator"
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"A", i32 7, !3}  ; [7 x i8]
!5 = !{!"F", i1 true, i32 0, !6}  ; i32 (...)
!6 = !{i32 0, i32 0}  ; i32
!7 = !{!5, i32 2}  ; i32 (...)**
!8 = !{i16 0, i32 0}  ; i16
!9 = !{%"XalanList" zeroinitializer, i32 0}  ; %"XalanList"
!10 = !{%"MemoryManager" zeroinitializer, i32 1}  ; %"MemoryManager"*
!11 = !{%"XalanList<ReusableArenaBlock<XStringCached> *>::Node" zeroinitializer, i32 1}  ; %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*
!12 = !{%"ReusableArenaBlock" zeroinitializer, i32 1}  ; %"ReusableArenaBlock"*
!13 = !{%"ArenaBlockBase" zeroinitializer, i32 0}  ; %"ArenaBlockBase"
!14 = !{!"A", i32 4, !3}  ; [4 x i8]
!15 = !{%"XalanAllocator" zeroinitializer, i32 0}  ; %"XalanAllocator"
!16 = !{%"XStringCached" zeroinitializer, i32 1}  ; %"XStringCached"*
!17 = !{%"XStringBase" zeroinitializer, i32 0}  ; %"XStringBase"
!18 = !{%"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}  ; %"XPathExecutionContext::GetAndReleaseCachedString"
!19 = !{%"XObject" zeroinitializer, i32 0}  ; %"XObject"
!20 = !{double 0.0e+00, i32 0}  ; double
!21 = !{%"XObjectResultTreeFragProxy" zeroinitializer, i32 0}  ; %"XObjectResultTreeFragProxy"
!22 = !{%"XalanReferenceCountedObject.base" zeroinitializer, i32 0}  ; %"XalanReferenceCountedObject.base"
!23 = !{%"XObjectFactory" zeroinitializer, i32 1}  ; %"XObjectFactory"*
!24 = !{%"XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}  ; %"XObjectResultTreeFragProxyBase"
!25 = !{%"XObjectResultTreeFragProxyText" zeroinitializer, i32 0}  ; %"XObjectResultTreeFragProxyText"
!26 = !{%"XalanDocumentFragment" zeroinitializer, i32 0}  ; %"XalanDocumentFragment"
!27 = !{%"XalanNode" zeroinitializer, i32 0}  ; %"XalanNode"
!28 = !{%"XalanText" zeroinitializer, i32 0}  ; %"XalanText"
!29 = !{%"XObject" zeroinitializer, i32 1}  ; %"XObject"*
!30 = !{%"XalanCharacterData" zeroinitializer, i32 0}  ; %"XalanCharacterData"
!31 = !{%"XPathExecutionContext" zeroinitializer, i32 1}  ; %"XPathExecutionContext"*
!32 = !{%"XalanDOMString" zeroinitializer, i32 1}  ; %"XalanDOMString"*
!33 = !{%"ExecutionContext" zeroinitializer, i32 0}  ; %"ExecutionContext"
!34 = !{%"XalanVector" zeroinitializer, i32 0}  ; %"XalanVector"
!35 = !{i64 0, i32 0}  ; i64
!36 = !{i16 0, i32 1}  ; i16*
!37 = !{%"XalanListIteratorBase" zeroinitializer, i32 0}  ; %"XalanListIteratorBase"
!38 = !{%"XalanListIteratorBase.0" zeroinitializer, i32 0}  ; %"XalanListIteratorBase.0"
!39 = !{i8 0, i32 1}  ; i8*
!40 = !{%"XStringCachedAllocator" zeroinitializer, i32 1}  ; %"XStringCachedAllocator"*
!41 = distinct !{!40, !10}
!42 = !{%"ReusableArenaAllocator" zeroinitializer, i32 1}  ; %"ReusableArenaAllocator"*
!43 = distinct !{!42, !10}
!44 = !{%"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}  ; %"XPathExecutionContext::GetAndReleaseCachedString"*
!45 = distinct !{!16, !40, !44}
!46 = distinct !{!16, !42}
!47 = !{%"ArenaAllocator" zeroinitializer, i32 1}  ; %"ArenaAllocator"*
!48 = distinct !{!10, !47}
!49 = distinct !{!16, !44, !10}
!50 = distinct !{!42, !16}
!51 = distinct !{!40, !16}
!52 = distinct !{!42, !16}
!53 = distinct !{!40}
!54 = distinct !{!47}
!55 = distinct !{!47, !10}
!56 = !{%"XalanList" zeroinitializer, i32 1}  ; %"XalanList"*
!57 = distinct !{!56, !10}
!58 = distinct !{!39}
!59 = distinct !{!39, !39}
!60 = !{%"XalanListIteratorBase" zeroinitializer, i32 1}  ; %"XalanListIteratorBase"*
!61 = distinct !{!60, !56}
!62 = distinct !{!60, !56}
!63 = distinct !{!11, !60}
!64 = distinct !{!11, !56}
!65 = distinct !{!60}
!66 = distinct !{!11, !56}
!67 = !{!"F", i1 false, i32 2, !39, !68, !35}  ; i8* (%"MemoryManager"* nonnull dereferenceable(8), i64)
!68 = !{!"F", i1 false, i32 1, !69, !70}  ; %"MemoryManager"* nonnull dereferenceable(8)
!69 = !{}  ;  nonnull dereferenceable
!70 = !{}  ; 8
!71 = distinct !{!39}
!72 = distinct !{!56}
!73 = !{%"ArenaBlockBase" zeroinitializer, i32 1}  ; %"ArenaBlockBase"*
!74 = distinct !{!73}
!75 = !{%"ReusableArenaBlock" zeroinitializer, i32 2}  ; %"ReusableArenaBlock"**
!76 = distinct !{!56, !75}
!77 = distinct !{!12, !10}
!78 = distinct !{!16, !12}
!79 = distinct !{!11, !56}
!80 = !{%"XalanListIteratorBase.0" zeroinitializer, i32 1}  ; %"XalanListIteratorBase.0"*
!81 = distinct !{!80, !80}
!82 = distinct !{!11, !56}
!83 = distinct !{!11, !56}
!84 = distinct !{!80}
!85 = distinct !{!75, !60}
!86 = distinct !{!11, !56, !75, !60}
!87 = distinct !{!75, !75, !75, !10}
!88 = distinct !{!12, !10, !75, !10, !36}
!89 = !{%"XalanAllocationGuard" zeroinitializer, i32 1}  ; %"XalanAllocationGuard"*
!90 = distinct !{!89, !10}
!91 = distinct !{!39, !89}
!92 = distinct !{!12, !10}
!93 = distinct !{!89}
!94 = distinct !{!73, !10}
!95 = !{%"XalanAllocator" zeroinitializer, i32 1}  ; %"XalanAllocator"*
!96 = distinct !{!95, !10}
!97 = distinct !{!16, !95, !39}
!98 = distinct !{!95}
!99 = distinct !{!12, !16}
!100 = distinct !{!11, !80}
!101 = distinct !{!73, !16}
!102 = !{%"struct.std::less" zeroinitializer, i32 1}  ; %"struct.std::less"*
!103 = distinct !{!102, !16, !16}
!104 = distinct !{!75, !56}
!105 = distinct !{!56, !75}
!106 = distinct !{!10, !56}
!107 = distinct !{!56}
!108 = distinct !{!56, !60}
!109 = distinct !{!56}
!110 = distinct !{!42}
!111 = distinct !{!39}
!112 = !{!"S", %"XStringCachedAllocator" zeroinitializer, i32 1, !1} ; { %"ReusableArenaAllocator" }
!113 = !{!"S", %"ReusableArenaAllocator" zeroinitializer, i32 3, !2, !3, !4} ; <{ %"ArenaAllocator", i8, [7 x i8] }>
!114 = !{!"S", %"ArenaAllocator" zeroinitializer, i32 3, !7, !8, !9} ; { i32 (...)**, i16, %"XalanList" }
!115 = !{!"S", %"XalanList" zeroinitializer, i32 3, !10, !11, !11} ; { %"MemoryManager"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!116 = !{!"S", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node" zeroinitializer, i32 3, !12, !11, !11} ; { %"ReusableArenaBlock"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!117 = !{!"S", %"ReusableArenaBlock" zeroinitializer, i32 4, !13, !8, !8, !14} ; <{ %"ArenaBlockBase", i16, i16, [4 x i8] }>
!118 = !{!"S", %"ArenaBlockBase" zeroinitializer, i32 4, !15, !8, !8, !16} ; { %"XalanAllocator", i16, i16, %"XStringCached"* }
!119 = !{!"S", %"XalanAllocator" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!120 = !{!"S", %"XStringCached" zeroinitializer, i32 2, !17, !18} ; { %"XStringBase", %"XPathExecutionContext::GetAndReleaseCachedString" }
!121 = !{!"S", %"XStringBase" zeroinitializer, i32 3, !19, !20, !21} ; { %"XObject", double, %"XObjectResultTreeFragProxy" }
!122 = !{!"S", %"XObject" zeroinitializer, i32 3, !22, !6, !23} ; { %"XalanReferenceCountedObject.base", i32, %"XObjectFactory"* }
!123 = !{!"S", %"XalanReferenceCountedObject.base" zeroinitializer, i32 2, !7, !6} ; <{ i32 (...)**, i32 }>
!124 = !{!"S", %"XObjectFactory" zeroinitializer, i32 0} ; opaque
!125 = !{!"S", %"XObjectResultTreeFragProxy" zeroinitializer, i32 2, !24, !25} ; { %"XObjectResultTreeFragProxyBase", %"XObjectResultTreeFragProxyText" }
!126 = !{!"S", %"XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !26} ; { %"XalanDocumentFragment" }
!127 = !{!"S", %"XalanDocumentFragment" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!128 = !{!"S", %"XalanNode" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!129 = !{!"S", %"XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !28, !29, !10} ; { %"XalanText", %"XObject"*, %"MemoryManager"* }
!130 = !{!"S", %"XalanText" zeroinitializer, i32 1, !30} ; { %"XalanCharacterData" }
!131 = !{!"S", %"XalanCharacterData" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!132 = !{!"S", %"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !31, !32} ; { %"XPathExecutionContext"*, %"XalanDOMString"* }
!133 = !{!"S", %"XPathExecutionContext" zeroinitializer, i32 2, !33, !23} ; { %"ExecutionContext", %"XObjectFactory"* }
!134 = !{!"S", %"ExecutionContext" zeroinitializer, i32 2, !7, !10} ; { i32 (...)**, %"MemoryManager"* }
!135 = !{!"S", %"XalanDOMString" zeroinitializer, i32 3, !34, !6, !14} ; <{ %"XalanVector", i32, [4 x i8] }>
!136 = !{!"S", %"XalanVector" zeroinitializer, i32 4, !10, !35, !35, !36} ; { %"MemoryManager"*, i64, i64, i16* }
!137 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!138 = !{!"S", %"XalanListIteratorBase" zeroinitializer, i32 1, !11} ; { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!139 = !{!"S", %"class.std::reverse_iterator.1" zeroinitializer, i32 1, !37} ; { %"XalanListIteratorBase" }
!140 = !{!"S", %"DeleteFunctor" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!141 = !{!"S", %"XalanListIteratorBase.0" zeroinitializer, i32 1, !11} ; { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!142 = !{!"S", %"class.std::reverse_iterator" zeroinitializer, i32 1, !38} ; { %"XalanListIteratorBase.0" }
!143 = !{!"S", %"XalanAllocationGuard" zeroinitializer, i32 2, !10, !39} ; { %"MemoryManager"*, i8* }
!144 = !{!"S", %"ReusableArenaBlock<XStringCached>::NextBlock" zeroinitializer, i32 2, !8, !6} ; { i16, i32 }
!145 = !{!"S", %"struct.std::less" zeroinitializer, i32 1, !3} ; { i8 }
!146 = !{!"S", %"XalanDestroyFunctor" zeroinitializer, i32 1, !3} ; { i8 }

!intel.dtrans.types = !{!112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146}

