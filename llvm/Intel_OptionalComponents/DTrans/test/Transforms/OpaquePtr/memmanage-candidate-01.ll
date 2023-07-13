; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(dtrans-force-inline-op)' -pre-lto-inline-cost -debug-only=dtrans-memmanageinfoop 2>&1 | FileCheck %s

; This test verifies that %"XStringCachedAllocator" is considered as a
; candidate for MemManageTrans.

; CHECK: MemManageTrans considering candidate: %XStringCachedAllocator

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

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
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(ptr "intel_dtrans_func_index"="1" nonnull dereferenceable(48) %this, ptr "intel_dtrans_func_index"="2" nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockCount) unnamed_addr align 2 !intel.dtrans.func.type !41 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nonnull dereferenceable(41) %m_allocator, ptr nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockCount, i1 zeroext false)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr "intel_dtrans_func_index"="1" nonnull dereferenceable(41) %this, ptr "intel_dtrans_func_index"="2" nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize, i1 zeroext %destroyBlocks) unnamed_addr align 2 !intel.dtrans.func.type !43 {
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
define dso_local "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(ptr "intel_dtrans_func_index"="2" nonnull dereferenceable(48) %this, ptr "intel_dtrans_func_index"="3" nonnull align 8 dereferenceable(16) %theValue) local_unnamed_addr align 2 !intel.dtrans.func.type !45 {
entry:
  %m_allocator = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0
  %call = tail call ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nonnull dereferenceable(41) %m_allocator)
  %0 = getelementptr inbounds %"XStringCachedAllocator", ptr %this, i64 0, i32 0, i32 0
  %call3 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nonnull dereferenceable(40) %0)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(80) %call, ptr nonnull align 8 dereferenceable(16) %theValue, ptr nonnull align 8 dereferenceable(8) %call3)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nonnull dereferenceable(41) %m_allocator, ptr nonnull %call)
  ret ptr %call
}

declare !intel.dtrans.func.type !46 dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr "intel_dtrans_func_index"="1" nonnull dereferenceable(80), ptr "intel_dtrans_func_index"="2" nonnull align 8 dereferenceable(16), ptr "intel_dtrans_func_index"="3" nonnull align 8 dereferenceable(8)) unnamed_addr
declare !intel.dtrans.func.type !48 "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr "intel_dtrans_func_index"="2" nonnull dereferenceable(40) %this)
declare !intel.dtrans.func.type !49 void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr "intel_dtrans_func_index"="1" nonnull dereferenceable(40) %this, ptr "intel_dtrans_func_index"="2" nonnull align 8 dereferenceable(8) %theManager, i16 zeroext %theBlockSize)
declare !intel.dtrans.func.type !50 "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr "intel_dtrans_func_index"="2" nonnull dereferenceable(41) %this)
declare !intel.dtrans.func.type !51 void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr "intel_dtrans_func_index"="1" nonnull dereferenceable(41) %this, ptr "intel_dtrans_func_index"="2" %theObject)

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
!46 = distinct !{!16, !44, !10}
!47 = !{%"ArenaAllocator" zeroinitializer, i32 1}  ; %"ArenaAllocator"*
!48 = distinct !{!10, !47}
!49 = distinct !{!47, !10}
!50 = distinct !{!16, !42}
!51 = distinct !{!42, !16}
!52 = !{!"S", %"XStringCachedAllocator" zeroinitializer, i32 1, !1} ; { %"ReusableArenaAllocator" }
!53 = !{!"S", %"ReusableArenaAllocator" zeroinitializer, i32 3, !2, !3, !4} ; <{ %"ArenaAllocator", i8, [7 x i8] }>
!54 = !{!"S", %"ArenaAllocator" zeroinitializer, i32 3, !7, !8, !9} ; { i32 (...)**, i16, %"XalanList" }
!55 = !{!"S", %"XalanList" zeroinitializer, i32 3, !10, !11, !11} ; { %"MemoryManager"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!56 = !{!"S", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node" zeroinitializer, i32 3, !12, !11, !11} ; { %"ReusableArenaBlock"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!57 = !{!"S", %"ReusableArenaBlock" zeroinitializer, i32 4, !13, !8, !8, !14} ; <{ %"ArenaBlockBase", i16, i16, [4 x i8] }>
!58 = !{!"S", %"ArenaBlockBase" zeroinitializer, i32 4, !15, !8, !8, !16} ; { %"XalanAllocator", i16, i16, %"XStringCached"* }
!59 = !{!"S", %"XalanAllocator" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!60 = !{!"S", %"XStringCached" zeroinitializer, i32 2, !17, !18} ; { %"XStringBase", %"XPathExecutionContext::GetAndReleaseCachedString" }
!61 = !{!"S", %"XStringBase" zeroinitializer, i32 3, !19, !20, !21} ; { %"XObject", double, %"XObjectResultTreeFragProxy" }
!62 = !{!"S", %"XObject" zeroinitializer, i32 3, !22, !6, !23} ; { %"XalanReferenceCountedObject.base", i32, %"XObjectFactory"* }
!63 = !{!"S", %"XalanReferenceCountedObject.base" zeroinitializer, i32 2, !7, !6} ; <{ i32 (...)**, i32 }>
!64 = !{!"S", %"XObjectFactory" zeroinitializer, i32 0} ; opaque
!65 = !{!"S", %"XObjectResultTreeFragProxy" zeroinitializer, i32 2, !24, !25} ; { %"XObjectResultTreeFragProxyBase", %"XObjectResultTreeFragProxyText" }
!66 = !{!"S", %"XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !26} ; { %"XalanDocumentFragment" }
!67 = !{!"S", %"XalanDocumentFragment" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!68 = !{!"S", %"XalanNode" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!69 = !{!"S", %"XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !28, !29, !10} ; { %"XalanText", %"XObject"*, %"MemoryManager"* }
!70 = !{!"S", %"XalanText" zeroinitializer, i32 1, !30} ; { %"XalanCharacterData" }
!71 = !{!"S", %"XalanCharacterData" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!72 = !{!"S", %"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !31, !32} ; { %"XPathExecutionContext"*, %"XalanDOMString"* }
!73 = !{!"S", %"XPathExecutionContext" zeroinitializer, i32 2, !33, !23} ; { %"ExecutionContext", %"XObjectFactory"* }
!74 = !{!"S", %"ExecutionContext" zeroinitializer, i32 2, !7, !10} ; { i32 (...)**, %"MemoryManager"* }
!75 = !{!"S", %"XalanDOMString" zeroinitializer, i32 3, !34, !6, !14} ; <{ %"XalanVector", i32, [4 x i8] }>
!76 = !{!"S", %"XalanVector" zeroinitializer, i32 4, !10, !35, !35, !36} ; { %"MemoryManager"*, i64, i64, i16* }
!77 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!78 = !{!"S", %"XalanListIteratorBase" zeroinitializer, i32 1, !11} ; { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!79 = !{!"S", %"class.std::reverse_iterator.1" zeroinitializer, i32 1, !37} ; { %"XalanListIteratorBase" }
!80 = !{!"S", %"DeleteFunctor" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!81 = !{!"S", %"XalanListIteratorBase.0" zeroinitializer, i32 1, !11} ; { %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!82 = !{!"S", %"class.std::reverse_iterator" zeroinitializer, i32 1, !38} ; { %"XalanListIteratorBase.0" }
!83 = !{!"S", %"XalanAllocationGuard" zeroinitializer, i32 2, !10, !39} ; { %"MemoryManager"*, i8* }
!84 = !{!"S", %"ReusableArenaBlock<XStringCached>::NextBlock" zeroinitializer, i32 2, !8, !6} ; { i16, i32 }
!85 = !{!"S", %"struct.std::less" zeroinitializer, i32 1, !3} ; { i8 }
!86 = !{!"S", %"XalanDestroyFunctor" zeroinitializer, i32 1, !3} ; { i8 }

!intel.dtrans.types = !{!52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86}
