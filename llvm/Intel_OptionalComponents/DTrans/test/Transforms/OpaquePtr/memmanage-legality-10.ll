; This test verifies that MemManageTrans is not triggered since
; the function @foo uses one of the candidate types, but @foo
; is not one of the interface or string object functions that
; were identified as expected users.
;
; RUN: opt < %s -opaque-pointers -passes=dtrans-memmanagetransop -dtrans-memmanageop-ignore-soa-heur -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetransop -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: MemManageTransOP transformation:
; CHECK:   Considering candidate: %XStringCachedAllocator
; CHECK:   Possible candidate structs:
; CHECK:       XStringCachedAllocator
; CHECK:   Analyzing Candidate ...
; CHECK:   Type unknown use in Function: foo
; CHECK:   Failed: Types escaped

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
%"DeleteFunctor" = type { ptr }
%"XalanAllocationGuard" = type { ptr, i8* }
%"ReusableArenaBlock<XStringCached>::NextBlock" = type { i16, i32 }
%"struct.std::less" = type { i8 }
%"XalanDestroyFunctor" = type { i8 }

@_ZTSN11xercesc_2_713MemoryManagerE = internal constant [31 x i8] c"N11xercesc_2_713MemoryManagerE\00"

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nocapture nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %0, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %1, i16 zeroext %2, i1 zeroext %3) unnamed_addr  align 2 !intel.dtrans.func.type !39 {
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr nocapture nonnull dereferenceable(40) "intel_dtrans_func_index"="1" %0) unnamed_addr  align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !41 {
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nonnull dereferenceable(40) null)
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr nonnull dereferenceable(80) null)
  ret void
}

; Function Attrs: uwtable
define internal zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr nocapture nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %0, ptr "intel_dtrans_func_index"="2" %1) align 2 !intel.dtrans.func.type !42 {
  ret i1 false
}

; Function Attrs: uwtable
define internal void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nocapture nonnull dereferenceable(40) "intel_dtrans_func_index"="1" %0) unnamed_addr align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !43 {
  ret void
}

; Function Attrs: norecurse nounwind readonly uwtable willreturn mustprogress
define internal nonnull align 8 dereferenceable(8)  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nocapture nonnull readonly dereferenceable(40) "intel_dtrans_func_index"="2" %0) align 2 !intel.dtrans.func.type !44 {
  ret ptr null
}

; Function Attrs: uwtable
define internal  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nocapture nonnull dereferenceable(41) "intel_dtrans_func_index"="2" %0) unnamed_addr align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !45 {
  ret ptr null
}

; Function Attrs: uwtable
define internal void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nocapture nonnull dereferenceable(41) "intel_dtrans_func_index"="1" %0, ptr nocapture readnone "intel_dtrans_func_index"="2" %1) unnamed_addr align 2 !intel.dtrans.func.type !46 {
  ret void
}

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt.8894(ptr nocapture nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %0, ptr nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %1, i16 zeroext %2) unnamed_addr align 2 !intel.dtrans.func.type !48 {
  %4 = getelementptr inbounds %"XStringCachedAllocator", ptr %0, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nonnull dereferenceable(41) %4, ptr nonnull align 8 dereferenceable(8) %1, i16 zeroext %2, i1 zeroext false)
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev.8907(ptr nocapture nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %0) unnamed_addr align 2 !intel.dtrans.func.type !49 {
  %2 = getelementptr inbounds %"XStringCachedAllocator", ptr %0, i64 0, i32 0
  %3 = getelementptr %"ReusableArenaAllocator", ptr %2, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr nonnull dereferenceable(41) %3)
  ret void
}

; Function Attrs: uwtable mustprogress
define internal  "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE.8908(ptr nocapture nonnull dereferenceable(48) "intel_dtrans_func_index"="2" %0, ptr nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %1) align 2 !intel.dtrans.func.type !51 {
  %3 = getelementptr inbounds %"XStringCachedAllocator", ptr %0, i64 0, i32 0
  %4 = tail call ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nonnull dereferenceable(41) %3)
  %5 = getelementptr %"ReusableArenaAllocator", ptr %3, i64 0, i32 0
  %6 = tail call nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nonnull dereferenceable(40) %5)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr nonnull dereferenceable(80) %4, ptr nonnull align 8 dereferenceable(16) %1, ptr nonnull align 8 dereferenceable(8) %6)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nonnull dereferenceable(41) %3, ptr nonnull %4)
  ret ptr %4
}

; Function Attrs: uwtable mustprogress
define internal zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE.8909(ptr nocapture nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %0, ptr "intel_dtrans_func_index"="2" %1) align 2 !intel.dtrans.func.type !52 {
  %3 = getelementptr inbounds %"XStringCachedAllocator", ptr %0, i64 0, i32 0
  %4 = tail call zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr nonnull dereferenceable(41) %3, ptr %1)
  ret i1 %4
}

; Function Attrs: uwtable mustprogress
define internal void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv.8911(ptr nocapture nonnull dereferenceable(48) "intel_dtrans_func_index"="1" %0) align 2 !intel.dtrans.func.type !53 {
  %2 = getelementptr inbounds %"XStringCachedAllocator", ptr %0, i64 0, i32 0
  %3 = getelementptr %"ReusableArenaAllocator", ptr %2, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nonnull dereferenceable(40) %3)
  ret void
}

define void @foo() {
  %local = alloca %"ReusableArenaAllocator"
  ret void
}

; This needs to be here so that @foo has users.
define void @doSomething() {
  call void @foo()
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare !intel.dtrans.func.type !54 void @llvm.memset.p0i8.i64(ptr nocapture "intel_dtrans_func_index"="1" writeonly, i8, i64, i1 immarg)

declare !intel.dtrans.func.type !55 void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr nonnull "intel_dtrans_func_index"="1", ptr nonnull "intel_dtrans_func_index"="2", ptr nonnull "intel_dtrans_func_index"="3" )

declare !intel.dtrans.func.type !56 void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr nonnull "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !57 void @__clang_call_terminate(ptr "intel_dtrans_func_index"="1")

declare dso_local i32 @__gxx_personality_v0(...)

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
!37 = !{i8 0, i32 1}  ; i8*
!38 = !{%"ReusableArenaAllocator" zeroinitializer, i32 1}  ; %"ReusableArenaAllocator"*
!39 = distinct !{!38, !10}
!40 = !{%"ArenaAllocator" zeroinitializer, i32 1}  ; %"ArenaAllocator"*
!41 = distinct !{!40}
!42 = distinct !{!38, !16}
!43 = distinct !{!40}
!44 = distinct !{!10, !40}
!45 = distinct !{!16, !38}
!46 = distinct !{!38, !16}
!47 = !{%"XStringCachedAllocator" zeroinitializer, i32 1}  ; %"XStringCachedAllocator"*
!48 = distinct !{!47, !10}
!49 = distinct !{!47}
!50 = !{%"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}  ; %"XPathExecutionContext::GetAndReleaseCachedString"*
!51 = distinct !{!16, !47, !50}
!52 = distinct !{!47, !16}
!53 = distinct !{!47}
!54 = distinct !{!37}
!55 = distinct !{!16, !50, !10}
!56 = distinct !{!16}
!57 = distinct !{!37}
!58 = !{!"S", %"XStringCachedAllocator" zeroinitializer, i32 1, !1} ; { %"ReusableArenaAllocator" }
!59 = !{!"S", %"ReusableArenaAllocator" zeroinitializer, i32 3, !2, !3, !4} ; <{ %"ArenaAllocator", i8, [7 x i8] }>
!60 = !{!"S", %"ArenaAllocator" zeroinitializer, i32 3, !7, !8, !9} ; { i32 (...)**, i16, %"XalanList" }
!61 = !{!"S", %"XalanList" zeroinitializer, i32 3, !10, !11, !11} ; { %"MemoryManager"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!62 = !{!"S", %"XalanList<ReusableArenaBlock<XStringCached> *>::Node" zeroinitializer, i32 3, !12, !11, !11} ; { %"ReusableArenaBlock"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"*, %"XalanList<ReusableArenaBlock<XStringCached> *>::Node"* }
!63 = !{!"S", %"ReusableArenaBlock" zeroinitializer, i32 4, !13, !8, !8, !14} ; <{ %"ArenaBlockBase", i16, i16, [4 x i8] }>
!64 = !{!"S", %"ArenaBlockBase" zeroinitializer, i32 4, !15, !8, !8, !16} ; { %"XalanAllocator", i16, i16, %"XStringCached"* }
!65 = !{!"S", %"XalanAllocator" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!66 = !{!"S", %"XStringCached" zeroinitializer, i32 2, !17, !18} ; { %"XStringBase", %"XPathExecutionContext::GetAndReleaseCachedString" }
!67 = !{!"S", %"XStringBase" zeroinitializer, i32 3, !19, !20, !21} ; { %"XObject", double, %"XObjectResultTreeFragProxy" }
!68 = !{!"S", %"XObject" zeroinitializer, i32 3, !22, !6, !23} ; { %"XalanReferenceCountedObject.base", i32, %"XObjectFactory"* }
!69 = !{!"S", %"XalanReferenceCountedObject.base" zeroinitializer, i32 2, !7, !6} ; <{ i32 (...)**, i32 }>
!70 = !{!"S", %"XObjectFactory" zeroinitializer, i32 0} ; opaque
!71 = !{!"S", %"XObjectResultTreeFragProxy" zeroinitializer, i32 2, !24, !25} ; { %"XObjectResultTreeFragProxyBase", %"XObjectResultTreeFragProxyText" }
!72 = !{!"S", %"XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !26} ; { %"XalanDocumentFragment" }
!73 = !{!"S", %"XalanDocumentFragment" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!74 = !{!"S", %"XalanNode" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!75 = !{!"S", %"XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !28, !29, !10} ; { %"XalanText", %"XObject"*, %"MemoryManager"* }
!76 = !{!"S", %"XalanText" zeroinitializer, i32 1, !30} ; { %"XalanCharacterData" }
!77 = !{!"S", %"XalanCharacterData" zeroinitializer, i32 1, !27} ; { %"XalanNode" }
!78 = !{!"S", %"XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !31, !32} ; { %"XPathExecutionContext"*, %"XalanDOMString"* }
!79 = !{!"S", %"XPathExecutionContext" zeroinitializer, i32 2, !33, !23} ; { %"ExecutionContext", %"XObjectFactory"* }
!80 = !{!"S", %"ExecutionContext" zeroinitializer, i32 2, !7, !10} ; { i32 (...)**, %"MemoryManager"* }
!81 = !{!"S", %"XalanDOMString" zeroinitializer, i32 3, !34, !6, !14} ; <{ %"XalanVector", i32, [4 x i8] }>
!82 = !{!"S", %"XalanVector" zeroinitializer, i32 4, !10, !35, !35, !36} ; { %"MemoryManager"*, i64, i64, i16* }
!83 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !7} ; { i32 (...)** }
!84 = !{!"S", %"DeleteFunctor" zeroinitializer, i32 1, !10} ; { %"MemoryManager"* }
!85 = !{!"S", %"XalanAllocationGuard" zeroinitializer, i32 2, !10, !37} ; { %"MemoryManager"*, i8* }
!86 = !{!"S", %"ReusableArenaBlock<XStringCached>::NextBlock" zeroinitializer, i32 2, !8, !6} ; { i16, i32 }
!87 = !{!"S", %"struct.std::less" zeroinitializer, i32 1, !3} ; { i8 }
!88 = !{!"S", %"XalanDestroyFunctor" zeroinitializer, i32 1, !3} ; { i8 }

!intel.dtrans.types = !{!58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88}

