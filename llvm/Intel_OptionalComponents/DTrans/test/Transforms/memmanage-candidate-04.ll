; This test verifies that calls in member functions of XStringCachedAllocator
; are not inlined and other calls in member functions of ReusableArenaAllocator
; are inlined.

; The forms of the functions here are taken from the new pass manager IR
; at the time of the dtrans-force-inline transformation.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -dtrans-force-inline -inline -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes='module(dtrans-force-inline),cgscc(inline)' -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define {{.*}} @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt
; CHECK: call {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb

; CHECK: define {{.*}} @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev

; CHECK: define {{.*}} @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE
; CHECK: call {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK: call {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_

; CHECK: define {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv
; CHECK-NOT: call {{.*}} @ _ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv
; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_

; CHECK: define {{.*}} @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE
; CHECK: call {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

; CHECK: define {{.*}} @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv

; CHECK: define {{.*}} @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev

; CHECK: define {{.*}} @_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv
; CHECK-NOT: call {{.*}} @ _ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv
; CHECK-NOT: call {{.*}} @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv
; CHECK-NOT: call {{.*}} @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_

; CHECK: define {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev

; CHECK: define {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv
; CHECK: call {{.*}} @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.xalanc_1_10::XStringCachedAllocator" = type { %"class.xalanc_1_10::ReusableArenaAllocator" }
%"class.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class.xalanc_1_10::ArenaAllocator" = type { i32 (...)**, i16, %"class.xalanc_1_10::XalanList" }
%"class.xalanc_1_10::XalanList" = type { %"class.xercesc_2_7::MemoryManager"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
%"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { %"class.xalanc_1_10::ReusableArenaBlock"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
%"class.xalanc_1_10::ReusableArenaBlock" = type <{ %"class.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class.xalanc_1_10::ArenaBlockBase" = type { %"class.xalanc_1_10::XalanAllocator", i16, i16, %"class.xalanc_1_10::XStringCached"* }
%"class.xalanc_1_10::XalanAllocator" = type { %"class.xercesc_2_7::MemoryManager"* }
%"class.xalanc_1_10::XStringCached" = type { %"class.xalanc_1_10::XStringBase", %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" }
%"class.xalanc_1_10::XStringBase" = type { %"class.xalanc_1_10::XObject", double, %"class.xalanc_1_10::XObjectResultTreeFragProxy" }
%"class.xalanc_1_10::XObject" = type { %"class.xalanc_1_10::XalanReferenceCountedObject.base", i32, %"class.xalanc_1_10::XObjectFactory"* }
%"class.xalanc_1_10::XalanReferenceCountedObject.base" = type <{ i32 (...)**, i32 }>
%"class.xalanc_1_10::XObjectFactory" = type opaque
%"class.xalanc_1_10::XObjectResultTreeFragProxy" = type { %"class.xalanc_1_10::XObjectResultTreeFragProxyBase", %"class.xalanc_1_10::XObjectResultTreeFragProxyText" }
%"class.xalanc_1_10::XObjectResultTreeFragProxyBase" = type { %"class.xalanc_1_10::XalanDocumentFragment" }
%"class.xalanc_1_10::XalanDocumentFragment" = type { %"class.xalanc_1_10::XalanNode" }
%"class.xalanc_1_10::XalanNode" = type { i32 (...)** }
%"class.xalanc_1_10::XObjectResultTreeFragProxyText" = type { %"class.xalanc_1_10::XalanText", %"class.xalanc_1_10::XObject"*, %"class.xercesc_2_7::MemoryManager"* }
%"class.xalanc_1_10::XalanText" = type { %"class.xalanc_1_10::XalanCharacterData" }
%"class.xalanc_1_10::XalanCharacterData" = type { %"class.xalanc_1_10::XalanNode" }
%"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" = type { %"class.xalanc_1_10::XPathExecutionContext"*, %"class.xalanc_1_10::XalanDOMString"* }
%"class.xalanc_1_10::XPathExecutionContext" = type { %"class.xalanc_1_10::ExecutionContext", %"class.xalanc_1_10::XObjectFactory"* }
%"class.xalanc_1_10::ExecutionContext" = type { i32 (...)**, %"class.xercesc_2_7::MemoryManager"* }
%"class.xalanc_1_10::XalanDOMString" = type <{ %"class.xalanc_1_10::XalanVector", i32, [4 x i8] }>
%"class.xalanc_1_10::XalanVector" = type { %"class.xercesc_2_7::MemoryManager"*, i64, i64, i16* }
%"class.xercesc_2_7::MemoryManager" = type { i32 (...)** }
%"struct.xalanc_1_10::XalanListIteratorBase" = type { %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
%"class.std::reverse_iterator.1" = type { %"struct.xalanc_1_10::XalanListIteratorBase" }
%"struct.xalanc_1_10::DeleteFunctor" = type { %"class.xercesc_2_7::MemoryManager"* }
%"struct.xalanc_1_10::XalanListIteratorBase.0" = type { %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* }
%"class.std::reverse_iterator" = type { %"struct.xalanc_1_10::XalanListIteratorBase.0" }
%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" = type { i16, i32 }
%"class.xalanc_1_10::XalanAllocationGuard" = type { %"class.xercesc_2_7::MemoryManager"*, i8* }
%"struct.xalanc_1_10::XalanCompileErrorBoolean" = type { [1 x i8] }
%"struct.std::iterator" = type { i8 }
%"struct.std::less" = type { i8 }
%"struct.std::iterator.2" = type { i8 }
%"struct.std::unary_function" = type { i8 }
%"struct.xalanc_1_10::XalanDestroyFunctor" = type { i8 }

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_ = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_ = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev = comdat any

$_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_ = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16commitAllocationEPS1_ = comdat any

$_ZNK11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE10ownsObjectEPKS1_ = comdat any

$__clang_call_terminate = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEED2Ev = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_ = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11destroyNodeERNS5_4NodeE = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_ = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_ = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv = comdat any

$_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_ = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_ = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_ = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_ = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE = comdat any

$_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_ = comdat any

$_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm = comdat any

$_ZNK11xalanc_1_1020XalanAllocationGuard3getEv = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt = comdat any

$_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv = comdat any

$_ZN11xalanc_1_1020XalanAllocationGuardD2Ev = comdat any

$_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et = comdat any

$_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev = comdat any

$_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv = comdat any

$_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev = comdat any

$_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE10deallocateEPS1_m = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_ = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv = comdat any

$_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv = comdat any

$_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_ = comdat any

$_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv = comdat any

$_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE10ownsObjectEPKS1_ = comdat any

$_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv = comdat any

$_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_ = comdat any

$_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_ = comdat any

$_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv = comdat any

$_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPKv = comdat any

$_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_ = comdat any

$_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock10isValidForEt = comdat any

$_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t = comdat any

$_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_ = comdat any

$_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_ = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_ = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_ = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv = comdat any

$_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_ = comdat any

$_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv = comdat any

$_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv = comdat any

$_ZN11xalanc_1_1012XalanDestroyINS_13XStringCachedEEEvRT_ = comdat any

$_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE = comdat any

$_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE7isEmptyEv = comdat any

$_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_ = comdat any

$_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_ = comdat any

$_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv = comdat any

$_ZSt8for_eachIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEENS0_13DeleteFunctorIS5_EEET0_T_SF_SE_ = comdat any

$_ZN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5clearEv = comdat any

$_ZNK11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_ = comdat any

$_ZN11xalanc_1_1023makeXalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_19XalanDestroyFunctorIT_EEPKS5_ = comdat any

$_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_RN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPS3_RN11xercesc_2_713MemoryManagerE = comdat any

$_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclERS3_ = comdat any

$_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtED2Ev = comdat any

$_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = comdat any

$_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = comdat any

$_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = comdat any

$_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = comdat any

$_ZTIN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = comdat any

$_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = comdat any

@_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local unnamed_addr constant { [8 x i8*] } { [8 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev to i8*), i8* bitcast (void (%"class.xalanc_1_10::ReusableArenaAllocator"*)* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev to i8*), i8* bitcast (%"class.xalanc_1_10::XStringCached"* (%"class.xalanc_1_10::ReusableArenaAllocator"*)* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv to i8*), i8* bitcast (void (%"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::XStringCached"*)* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_ to i8*), i8* bitcast (i1 (%"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::XStringCached"*)* @_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_ to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv to i8*)] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local constant [61 x i8] c"N11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local constant [83 x i8] c"N11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE\00", comdat, align 1
@_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([83 x i8], [83 x i8]* @_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, i32 0, i32 0) }, comdat, align 8
@_ZTIN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([61 x i8], [61 x i8]* @_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE to i8*) }, comdat, align 8
@_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local unnamed_addr constant { [8 x i8*] } { [8 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev to i8*), i8* bitcast (%"class.xalanc_1_10::XStringCached"* (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::XStringCached"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16commitAllocationEPS1_ to i8*), i8* bitcast (i1 (%"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::XStringCached"*)* @_ZNK11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE10ownsObjectEPKS1_ to i8*), i8* bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv to i8*)] }, comdat, align 8

@_ZN11xalanc_1_1022XStringCachedAllocatorC1ERN11xercesc_2_713MemoryManagerEt = dso_local unnamed_addr alias void (%"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xercesc_2_7::MemoryManager"*, i16), void (%"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xercesc_2_7::MemoryManager"*, i16)* @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt
@_ZN11xalanc_1_1022XStringCachedAllocatorD1Ev = dso_local unnamed_addr alias void (%"class.xalanc_1_10::XStringCachedAllocator"*), void (%"class.xalanc_1_10::XStringCachedAllocator"*)* @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::XStringCachedAllocator"* noundef nonnull align 8 dereferenceable(48) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockCount) unnamed_addr #0 align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XStringCachedAllocator"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockCount.addr = alloca i16, align 2
  store %"class.xalanc_1_10::XStringCachedAllocator"* %this, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockCount, i16* %theBlockCount.addr, align 2
  %this1 = load %"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %i1 = load i16, i16* %theBlockCount.addr, align 2
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %m_allocator, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i, i16 noundef zeroext %i1, i1 noundef zeroext false)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockSize, i1 noundef zeroext %destroyBlocks) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockSize.addr = alloca i16, align 2
  %destroyBlocks.addr = alloca i8, align 1
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %frombool = zext i1 %destroyBlocks to i8
  store i8 %frombool, i8* %destroyBlocks.addr, align 1
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %i2 = load i16, i16* %theBlockSize.addr, align 2
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %i, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, i16 noundef zeroext %i2)
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* @_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %i3, align 8
  %m_destroyBlocks = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaAllocator", %"class.xalanc_1_10::ReusableArenaAllocator"* %this1, i32 0, i32 1
  %i4 = load i8, i8* %destroyBlocks.addr, align 1
  %tobool = trunc i8 %i4 to i1
  %frombool2 = zext i1 %tobool to i8
  store i8 %frombool2, i8* %m_destroyBlocks, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev(%"class.xalanc_1_10::XStringCachedAllocator"* noundef nonnull align 8 dereferenceable(48) %this) unnamed_addr #1 align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XStringCachedAllocator"*, align 8
  store %"class.xalanc_1_10::XStringCachedAllocator"* %this, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  call void bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev to void (%"class.xalanc_1_10::ReusableArenaAllocator"*)*)(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %m_allocator) #6
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ArenaAllocator"* %this1 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %i, align 8
  %i1 = bitcast %"class.xalanc_1_10::ArenaAllocator"* %this1 to void (%"class.xalanc_1_10::ArenaAllocator"*)***
  %vtable = load void (%"class.xalanc_1_10::ArenaAllocator"*)**, void (%"class.xalanc_1_10::ArenaAllocator"*)*** %i1, align 8
  %i2 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i3 = bitcast void (%"class.xalanc_1_10::ArenaAllocator"*)** %vtable to i8*
  %i4 = call i1 @llvm.type.test(i8* %i3, metadata !"_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE")
  call void @llvm.assume(i1 %i4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%"class.xalanc_1_10::ArenaAllocator"*)*, void (%"class.xalanc_1_10::ArenaAllocator"*)** %vtable, i64 5
  %i5 = load void (%"class.xalanc_1_10::ArenaAllocator"*)*, void (%"class.xalanc_1_10::ArenaAllocator"*)** %vfn, align 8
  invoke void %i5(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this1)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEED2Ev(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks) #6
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %i6 = landingpad { i8*, i32 }
          catch i8* null
  %i7 = extractvalue { i8*, i32 } %i6, 0
  call void @__clang_call_terminate(i8* %i7) #18
  unreachable
}

; Function Attrs: mustprogress uwtable
define dso_local noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(%"class.xalanc_1_10::XStringCachedAllocator"* noundef nonnull align 8 dereferenceable(48) %this, %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"* noundef nonnull align 8 dereferenceable(16) %theValue) #2 align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XStringCachedAllocator"*, align 8
  %theValue.addr = alloca %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"*, align 8
  %theBlock = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %theResult = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::XStringCachedAllocator"* %this, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"* %theValue, %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"** %theValue.addr, align 8
  %this1 = load %"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::XStringCached"** %theBlock to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %call = call noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %m_allocator)
  store %"class.xalanc_1_10::XStringCached"* %call, %"class.xalanc_1_10::XStringCached"** %theBlock, align 8
  %i1 = bitcast %"class.xalanc_1_10::XStringCached"** %theResult to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %i2 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theBlock, align 8
  %i3 = bitcast %"class.xalanc_1_10::XStringCached"* %i2 to i8*
  %i4 = bitcast i8* %i3 to %"class.xalanc_1_10::XStringCached"*
  %i5 = load %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"*, %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"** %theValue.addr, align 8
  %m_allocator2 = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %i6 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %m_allocator2 to %"class.xalanc_1_10::ArenaAllocator"*
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %i6)
  call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80) %i4, %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"* noundef nonnull align 8 dereferenceable(16) %i5, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %call3)
  store %"class.xalanc_1_10::XStringCached"* %i4, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  %m_allocator4 = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %i7 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theBlock, align 8
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %m_allocator4, %"class.xalanc_1_10::XStringCached"* noundef %i7)
  %i8 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  %i9 = bitcast %"class.xalanc_1_10::XStringCached"** %theResult to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i9) #6
  %i10 = bitcast %"class.xalanc_1_10::XStringCached"** %theBlock to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i10) #6
  ret %"class.xalanc_1_10::XStringCached"* %i8
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %ref.tmp = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %i1 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i1, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %i2 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call3, align 8
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i2 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i3)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false, %entry
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks5 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i4, i32 0, i32 2
  %i5 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i5) #6
  %i6 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %call6 = call noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %i6)
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i7, i32 0, i32 1
  %i8 = load i16, i16* %m_blockSize, align 8
  %call7 = call noundef %"class.xalanc_1_10::ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %call6, i16 noundef zeroext %i8)
  store %"class.xalanc_1_10::ReusableArenaBlock"* %call7, %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks5, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i9 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i9) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %i10 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks8 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i10, i32 0, i32 2
  %call9 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks8)
  %i11 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call9, align 8
  %call10 = call noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i11)
  ret %"class.xalanc_1_10::XStringCached"* %call10
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  ret %"class.xercesc_2_7::MemoryManager"* %call
}

declare dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80), %"class.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString"* noundef nonnull align 8 dereferenceable(16), %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8)) unnamed_addr #4

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %fullBlock = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %i1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call, align 8
  %i2 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i1, %"class.xalanc_1_10::XStringCached"* noundef %i2)
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i3, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %i4 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call3, align 8
  %i5 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i4 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i5)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %i6 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %fullBlock to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i6) #6
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks5 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i7, i32 0, i32 2
  %call6 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks5)
  %i8 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call6, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %i8, %"class.xalanc_1_10::ReusableArenaBlock"** %fullBlock, align 8
  %i9 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks7 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i9, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks7)
  %i10 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks8 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i10, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks8, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %fullBlock)
  %i11 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %fullBlock to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i11) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: mustprogress uwtable
define dso_local noundef zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE(%"class.xalanc_1_10::XStringCachedAllocator"* noundef nonnull align 8 dereferenceable(48) %this, %"class.xalanc_1_10::XStringCached"* noundef %theString) #2 align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XStringCachedAllocator"*, align 8
  %theString.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::XStringCachedAllocator"* %this, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theString, %"class.xalanc_1_10::XStringCached"** %theString.addr, align 8
  %this1 = load %"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theString.addr, align 8
  %call = call noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %m_allocator, %"class.xalanc_1_10::XStringCached"* noundef %i)
  ret i1 %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) #2 comdat align 2 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %bResult = alloca i8, align 1
  %cleanup.dest.slot = alloca i32, align 4
  %iTerator = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %iEnd = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %block = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %rIterator = alloca %"class.std::reverse_iterator.1", align 8
  %rEnd = alloca %"class.std::reverse_iterator.1", align 8
  %ref.tmp34 = alloca %"class.std::reverse_iterator.1", align 8
  %block38 = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %agg.tmp41 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %bResult) #6
  store i8 0, i8* %bResult, align 1
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i1 = load i8, i8* %bResult, align 1
  %tobool = trunc i8 %i1 to i1
  store i1 %tobool, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end:                                           ; preds = %entry
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %iTerator to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i3, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %iTerator, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %iEnd to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i4) #6
  %i5 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks3 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i5, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %iEnd, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks3)
  br label %while.cond

while.cond:                                       ; preds = %if.end21, %if.end
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iEnd)
  br i1 %call4, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %call5 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i6 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call5, align 8
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i6 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call6 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i7)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %i8 = phi i1 [ false, %while.cond ], [ %call6, %land.rhs ]
  br i1 %i8, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %call7 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i9 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call7, align 8
  %i10 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i9 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i11 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %call8 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i10, %"class.xalanc_1_10::XStringCached"* noundef %i11)
  %conv = zext i1 %call8 to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then9, label %if.end21

if.then9:                                         ; preds = %while.body
  %call10 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i12 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call10, align 8
  %i13 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i12, %"class.xalanc_1_10::XStringCached"* noundef %i13)
  %i14 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i14) #6
  %i15 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks11 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i15, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks11)
  %call12 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i16 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i16) #6
  br i1 %call12, label %if.then13, label %if.end17

if.then13:                                        ; preds = %if.then9
  %i17 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %block to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i17) #6
  %call14 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i18 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call14, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %i18, %"class.xalanc_1_10::ReusableArenaBlock"** %block, align 8
  %i19 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks15 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i19, i32 0, i32 2
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks15, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  %i20 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks16 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i20, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks16, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %block)
  %i21 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %block to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i21) #6
  br label %if.end17

if.end17:                                         ; preds = %if.then13, %if.then9
  %m_destroyBlocks = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaAllocator", %"class.xalanc_1_10::ReusableArenaAllocator"* %this1, i32 0, i32 1
  %i22 = load i8, i8* %m_destroyBlocks, align 8
  %tobool18 = trunc i8 %i22 to i1
  br i1 %tobool18, label %if.then19, label %if.end20

if.then19:                                        ; preds = %if.end17
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this1)
  br label %if.end20

if.end20:                                         ; preds = %if.then19, %if.end17
  store i8 1, i8* %bResult, align 1
  br label %while.end

if.end21:                                         ; preds = %while.body
  %i23 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i23) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i24 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i24) #6
  br label %while.cond

while.end:                                        ; preds = %if.end20, %land.end
  %i25 = bitcast %"class.std::reverse_iterator.1"* %rIterator to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i25) #6
  %i26 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks22 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i26, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator.1"* sret(%"class.std::reverse_iterator.1") align 8 %rIterator, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks22)
  %i27 = bitcast %"class.std::reverse_iterator.1"* %rEnd to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i27) #6
  %i28 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks23 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i28, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(%"class.std::reverse_iterator.1"* sret(%"class.std::reverse_iterator.1") align 8 %rEnd, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks23)
  br label %while.cond24

while.cond24:                                     ; preds = %if.end54, %while.end
  %i29 = load i8, i8* %bResult, align 1
  %tobool25 = trunc i8 %i29 to i1
  br i1 %tobool25, label %land.end28, label %land.rhs26

land.rhs26:                                       ; preds = %while.cond24
  %call27 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rEnd)
  br label %land.end28

land.end28:                                       ; preds = %land.rhs26, %while.cond24
  %i30 = phi i1 [ false, %while.cond24 ], [ %call27, %land.rhs26 ]
  br i1 %i30, label %while.body29, label %while.end55

while.body29:                                     ; preds = %land.end28
  %call30 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  %i31 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call30, align 8
  %i32 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i31 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i33 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %call31 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i32, %"class.xalanc_1_10::XStringCached"* noundef %i33)
  br i1 %call31, label %if.then32, label %if.end48

if.then32:                                        ; preds = %while.body29
  %call33 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  %i34 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call33, align 8
  %i35 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i34, %"class.xalanc_1_10::XStringCached"* noundef %i35)
  %i36 = bitcast %"class.std::reverse_iterator.1"* %ref.tmp34 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i36) #6
  %i37 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks35 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i37, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator.1"* sret(%"class.std::reverse_iterator.1") align 8 %ref.tmp34, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks35)
  %call36 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %ref.tmp34)
  %i38 = bitcast %"class.std::reverse_iterator.1"* %ref.tmp34 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i38) #6
  br i1 %call36, label %if.then37, label %if.end43

if.then37:                                        ; preds = %if.then32
  %i39 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %block38 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i39) #6
  %call39 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i40 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call39, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %i40, %"class.xalanc_1_10::ReusableArenaBlock"** %block38, align 8
  %i41 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks40 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i41, i32 0, i32 2
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.tmp41, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks40, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp41)
  %i42 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks42 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i42, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks42, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %block38)
  %i43 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %block38 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i43) #6
  br label %if.end43

if.end43:                                         ; preds = %if.then37, %if.then32
  %m_destroyBlocks44 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaAllocator", %"class.xalanc_1_10::ReusableArenaAllocator"* %this1, i32 0, i32 1
  %i44 = load i8, i8* %m_destroyBlocks44, align 8
  %tobool45 = trunc i8 %i44 to i1
  br i1 %tobool45, label %if.then46, label %if.end47

if.then46:                                        ; preds = %if.end43
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this1)
  br label %if.end47

if.end47:                                         ; preds = %if.then46, %if.end43
  store i8 1, i8* %bResult, align 1
  br label %while.end55

if.end48:                                         ; preds = %while.body29
  %call49 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  %i45 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call49, align 8
  %call50 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i46 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call50, align 8
  %cmp51 = icmp eq %"class.xalanc_1_10::ReusableArenaBlock"* %i45, %i46
  br i1 %cmp51, label %if.then52, label %if.else

if.then52:                                        ; preds = %if.end48
  br label %while.end55

if.else:                                          ; preds = %if.end48
  %call53 = call noundef nonnull align 8 dereferenceable(8) %"class.std::reverse_iterator.1"* @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  br label %if.end54

if.end54:                                         ; preds = %if.else
  br label %while.cond24

while.end55:                                      ; preds = %if.then52, %if.end47, %land.end28
  %i47 = load i8, i8* %bResult, align 1
  %tobool56 = trunc i8 %i47 to i1
  store i1 %tobool56, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  %i48 = bitcast %"class.std::reverse_iterator.1"* %rEnd to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i48) #6
  %i49 = bitcast %"class.std::reverse_iterator.1"* %rIterator to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i49) #6
  %i50 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %iEnd to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i50) #6
  %i51 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %iTerator to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i51) #6
  br label %cleanup

cleanup:                                          ; preds = %while.end55, %if.then
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %bResult) #6
  %i52 = load i1, i1* %retval, align 1
  ret i1 %i52
}

; Function Attrs: mustprogress uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv(%"class.xalanc_1_10::XStringCachedAllocator"* noundef nonnull align 8 dereferenceable(48) %this) #2 align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XStringCachedAllocator"*, align 8
  store %"class.xalanc_1_10::XStringCachedAllocator"* %this, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XStringCachedAllocator"*, %"class.xalanc_1_10::XStringCachedAllocator"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::XStringCachedAllocator", %"class.xalanc_1_10::XStringCachedAllocator"* %this1, i32 0, i32 0
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %m_allocator to %"class.xalanc_1_10::ArenaAllocator"*
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %i)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %agg.tmp2 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %agg.tmp4 = alloca %"struct.xalanc_1_10::DeleteFunctor", align 8
  %coerce = alloca %"struct.xalanc_1_10::DeleteFunctor", align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %m_blocks3 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp2, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks3)
  %m_blocks5 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks5)
  call void @_ZN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::DeleteFunctor"* noundef nonnull align 8 dereferenceable(8) %agg.tmp4, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %call)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %agg.tmp4, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %coerce.dive, align 8
  %call6 = call %"class.xercesc_2_7::MemoryManager"* @_ZSt8for_eachIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEENS0_13DeleteFunctorIS5_EEET0_T_SF_SE_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp2, %"class.xercesc_2_7::MemoryManager"* %i)
  %coerce.dive7 = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %coerce, i32 0, i32 0
  store %"class.xercesc_2_7::MemoryManager"* %call6, %"class.xercesc_2_7::MemoryManager"** %coerce.dive7, align 8
  %m_blocks8 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5clearEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks8)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockSize.addr = alloca i16, align 2
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ArenaAllocator"* %this1 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [8 x i8*] }, { [8 x i8*] }* @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %i, align 8
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 1
  %i1 = load i16, i16* %theBlockSize.addr, align 2
  store i16 %i1, i16* %m_blockSize, align 8
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %i2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i2)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  call void bitcast (void (%"class.xalanc_1_10::ArenaAllocator"*)* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev to void (%"class.xalanc_1_10::ReusableArenaAllocator"*)*)(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this1) #6
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to i8*
  call void @_ZdlPv(i8* noundef %i) #19
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) unnamed_addr #2 comdat align 2 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %iTerator = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %iEnd = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %cleanup.dest.slot = alloca i32, align 4
  %coerce = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %rIterator = alloca %"class.std::reverse_iterator", align 8
  %rEnd = alloca %"class.std::reverse_iterator", align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i1 false, i1* %retval, align 1
  br label %return

if.end:                                           ; preds = %entry
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iTerator to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %i2 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i2, i32 0, i32 2
  %call3 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iTerator, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call3, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iEnd to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i3) #6
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks4 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i4, i32 0, i32 2
  %call5 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks4)
  %coerce.dive6 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iEnd, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call5, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive6, align 8
  br label %while.cond

while.cond:                                       ; preds = %if.end13, %if.end
  %call7 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iEnd)
  br i1 %call7, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %call8 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i5 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call8, align 8
  %i6 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i5 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call9 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i6)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %i7 = phi i1 [ false, %while.cond ], [ %call9, %land.rhs ]
  br i1 %i7, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %call10 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i8 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call10, align 8
  %i9 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i8 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i10 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %call11 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i9, %"class.xalanc_1_10::XStringCached"* noundef %i10)
  br i1 %call11, label %if.then12, label %if.end13

if.then12:                                        ; preds = %while.body
  store i1 true, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup32

if.end13:                                         ; preds = %while.body
  %call14 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %coerce.dive15 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %coerce, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call14, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive15, align 8
  br label %while.cond

while.end:                                        ; preds = %land.end
  %i11 = bitcast %"class.std::reverse_iterator"* %rIterator to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i11) #6
  %i12 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks16 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i12, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator"* sret(%"class.std::reverse_iterator") align 8 %rIterator, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks16)
  %i13 = bitcast %"class.std::reverse_iterator"* %rEnd to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i13) #6
  %i14 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks17 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i14, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(%"class.std::reverse_iterator"* sret(%"class.std::reverse_iterator") align 8 %rEnd, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks17)
  br label %while.cond18

while.cond18:                                     ; preds = %if.end29, %while.end
  %call19 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %rIterator, %"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %rEnd)
  br i1 %call19, label %while.body20, label %while.end30

while.body20:                                     ; preds = %while.cond18
  %call21 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  %i15 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call21, align 8
  %i16 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i15 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i17 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %call22 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i16, %"class.xalanc_1_10::XStringCached"* noundef %i17)
  br i1 %call22, label %if.then23, label %if.end24

if.then23:                                        ; preds = %while.body20
  store i1 true, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end24:                                         ; preds = %while.body20
  %call25 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i18 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call25, align 8
  %call26 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  %i19 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call26, align 8
  %cmp = icmp eq %"class.xalanc_1_10::ReusableArenaBlock"* %i18, %i19
  br i1 %cmp, label %if.then27, label %if.else

if.then27:                                        ; preds = %if.end24
  br label %while.end30

if.else:                                          ; preds = %if.end24
  %call28 = call noundef nonnull align 8 dereferenceable(8) %"class.std::reverse_iterator"* @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %rIterator)
  br label %if.end29

if.end29:                                         ; preds = %if.else
  br label %while.cond18

while.end30:                                      ; preds = %if.then27, %while.cond18
  store i1 false, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %while.end30, %if.then23
  %i20 = bitcast %"class.std::reverse_iterator"* %rEnd to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i20) #6
  %i21 = bitcast %"class.std::reverse_iterator"* %rIterator to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i21) #6
  br label %cleanup32

cleanup32:                                        ; preds = %cleanup, %if.then12
  %i22 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iEnd to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i22) #6
  %i23 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iTerator to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i23) #6
  br label %return

return:                                           ; preds = %cleanup32, %if.then
  %i24 = load i1, i1* %retval, align 1
  ret i1 %i24
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %i, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %m_listHead = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this1) #6
  %i = bitcast %"class.xalanc_1_10::ArenaAllocator"* %this1 to i8*
  call void @_ZdlPv(i8* noundef %i) #19
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  %ref.tmp = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %conv = zext i1 %call to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call3, align 8
  %i1 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i to %"class.xalanc_1_10::ArenaBlockBase"*
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i1)
  %conv5 = zext i1 %call4 to i32
  %cmp6 = icmp eq i32 %conv5, 0
  br i1 %cmp6, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %entry
  %m_blocks7 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %i2 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %call8 = call noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this1)
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 1
  %i3 = load i16, i16* %m_blockSize, align 8
  %call9 = call noundef %"class.xalanc_1_10::ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %call8, i16 noundef zeroext %i3)
  store %"class.xalanc_1_10::ReusableArenaBlock"* %call9, %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks7, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %m_blocks10 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call11 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks10)
  %i5 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call11, align 8
  %call12 = call noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i5)
  ret %"class.xalanc_1_10::XStringCached"* %call12
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16commitAllocationEPS1_(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call, align 8
  %i1 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i, %"class.xalanc_1_10::XStringCached"* noundef %i1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE10ownsObjectEPKS1_(%"class.xalanc_1_10::ArenaAllocator"* noundef nonnull align 8 dereferenceable(40) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaAllocator"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %fResult = alloca i8, align 1
  %theEnd = alloca %"class.std::reverse_iterator", align 8
  %i = alloca %"class.std::reverse_iterator", align 8
  store %"class.xalanc_1_10::ArenaAllocator"* %this, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaAllocator"*, %"class.xalanc_1_10::ArenaAllocator"** %this.addr, align 8
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %fResult) #6
  store i8 0, i8* %fResult, align 1
  %i1 = bitcast %"class.std::reverse_iterator"* %theEnd to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(%"class.std::reverse_iterator"* sret(%"class.std::reverse_iterator") align 8 %theEnd, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %i2 = bitcast %"class.std::reverse_iterator"* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator"* sret(%"class.std::reverse_iterator") align 8 %i, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %call = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i, %"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %theEnd)
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i)
  %i3 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call3, align 8
  %i4 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE10ownsObjectEPKS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i3, %"class.xalanc_1_10::XStringCached"* noundef %i4)
  %conv = zext i1 %call4 to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  store i8 1, i8* %fResult, align 1
  br label %while.end

if.else:                                          ; preds = %while.body
  %call5 = call noundef nonnull align 8 dereferenceable(8) %"class.std::reverse_iterator"* @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i)
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %while.cond

while.end:                                        ; preds = %if.then, %while.cond
  %i5 = load i8, i8* %fResult, align 1
  %tobool = trunc i8 %i5 to i1
  %i6 = bitcast %"class.std::reverse_iterator"* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i6) #6
  %i7 = bitcast %"class.std::reverse_iterator"* %theEnd to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i7) #6
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %fResult) #6
  ret i1 %tobool
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #6

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(i8*, metadata) #7

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #8

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8* %arg) #9 comdat {
bb:
  %i = call i8* @__cxa_begin_catch(i8* %arg) #6
  call void @_ZSt9terminatev() #18
  unreachable
}

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8*) #10

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() #11

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEED2Ev(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %pos = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp4 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %freeNode = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  %nextNode = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %m_listHead = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead, align 8
  %cmp = icmp ne %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %pos to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %pos, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %if.then
  br label %while.cond

while.cond:                                       ; preds = %invoke.cont8, %invoke.cont
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
          to label %invoke.cont2 unwind label %terminate.lpad

invoke.cont2:                                     ; preds = %while.cond
  %call = invoke noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %invoke.cont2
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i3) #6
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %invoke.cont3
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i4) #6
  invoke void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp4, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos, i32 noundef 0)
          to label %invoke.cont5 unwind label %terminate.lpad

invoke.cont5:                                     ; preds = %while.body
  %call7 = invoke noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp4)
          to label %invoke.cont6 unwind label %terminate.lpad

invoke.cont6:                                     ; preds = %invoke.cont5
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11destroyNodeERNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %call7)
          to label %invoke.cont8 unwind label %terminate.lpad

invoke.cont8:                                     ; preds = %invoke.cont6
  %i5 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i5) #6
  br label %while.cond

while.end:                                        ; preds = %invoke.cont3
  %i6 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i6) #6
  %m_freeListHeadPtr = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i7 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i7, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode, align 8
  br label %while.cond9

while.cond9:                                      ; preds = %invoke.cont12, %while.end
  %i8 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode, align 8
  %cmp10 = icmp ne %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i8, null
  br i1 %cmp10, label %while.body11, label %while.end13

while.body11:                                     ; preds = %while.cond9
  %i9 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextNode to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i9) #6
  %i10 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i10, i32 0, i32 2
  %i11 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i11, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextNode, align 8
  %i12 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode, align 8
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef %i12)
          to label %invoke.cont12 unwind label %terminate.lpad

invoke.cont12:                                    ; preds = %while.body11
  %i13 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextNode, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i13, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode, align 8
  %i14 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextNode to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i14) #6
  br label %while.cond9

while.end13:                                      ; preds = %while.cond9
  %m_listHead14 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i15 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead14, align 8
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef %i15)
          to label %invoke.cont15 unwind label %terminate.lpad

invoke.cont15:                                    ; preds = %while.end13
  %i16 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %freeNode to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i16) #6
  %i17 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %pos to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i17) #6
  br label %if.end

if.end:                                           ; preds = %invoke.cont15, %entry
  ret void

terminate.lpad:                                   ; preds = %while.end13, %while.body11, %invoke.cont6, %invoke.cont5, %while.body, %invoke.cont2, %while.cond, %if.then
  %i18 = landingpad { i8*, i32 }
          catch i8* null
  %i19 = extractvalue { i8*, i32 } %i18, 0
  call void @__clang_call_terminate(i8* %i19) #18
  unreachable
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, i32 0, i32 2
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %i1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %theRhs) #2 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %i = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this1, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %i)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11destroyNodeERNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %node) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %node.addr = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %node, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef %i1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this, i32 noundef %arg) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %.addr = alloca i32, align 4
  %origNode = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  store i32 %arg, i32* %.addr, align 4
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %i1 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %origNode to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %origNode, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i3 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i3, i32 0, i32 2
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  %currentNode3 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode3, align 8
  %i5 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %origNode, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %i5)
  %i6 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %origNode to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i6) #6
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef %pointer) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %pointer.addr = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %pointer, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %pointer.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %pointer.addr, align 8
  %i2 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1 to i8*
  %i3 = bitcast %"class.xercesc_2_7::MemoryManager"* %i to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %i3, align 8
  %i4 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i5 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %i6 = call i1 @llvm.type.test(i8* %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %i7 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  call void %i7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i, i8* noundef %i2)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %m_listHead = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead, align 8
  %cmp = icmp eq %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %i
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call noundef %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, i64 noundef 1)
  %m_listHead2 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead2, align 8
  %m_listHead3 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead3, align 8
  %m_listHead4 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead4, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  %m_listHead5 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i3 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead5, align 8
  %m_listHead6 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead6, align 8
  %prev = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i3, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %m_listHead7 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 1
  %i5 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_listHead7, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i5
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %node) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %node.addr = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %node, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, i64 noundef %size) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %size.addr = alloca i64, align 8
  %theBytesNeeded = alloca i64, align 8
  %pointer = alloca i8*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store i64 %size, i64* %size.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast i64* %theBytesNeeded to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %i1 = load i64, i64* %size.addr, align 8
  %mul = mul i64 %i1, 24
  store i64 %mul, i64* %theBytesNeeded, align 8
  %i2 = bitcast i8** %pointer to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 0
  %i3 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %i4 = load i64, i64* %theBytesNeeded, align 8
  %i5 = bitcast %"class.xercesc_2_7::MemoryManager"* %i3 to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %i5, align 8
  %i6 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i6, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i7 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %i8 = call i1 @llvm.type.test(i8* %i7, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i8)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %i9 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = call noundef i8* %i9(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i3, i64 noundef %i4)
  store i8* %call, i8** %pointer, align 8
  %i10 = load i8*, i8** %pointer, align 8
  %i11 = bitcast i8* %i10 to %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*
  %i12 = bitcast i8** %pointer to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i12) #6
  %i13 = bitcast i64* %theBytesNeeded to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i13) #6
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i11
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %theRhs) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %i1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %i1, i32 0, i32 0
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  %cmp = icmp eq %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, %i2
  ret i1 %cmp
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef) #13

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp2 = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %call3 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive4 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp2, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call3, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive4, align 8
  %call5 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  %conv = zext i1 %call5 to i32
  %cmp = icmp ne i32 %conv, 0
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i3) #6
  ret i1 %cmp
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp2 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp2, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i3) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %call
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 1
  %i = load i16, i16* %m_objectCount, align 8
  %conv = zext i16 %i to i32
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %m_blockSize, align 2
  %conv2 = zext i16 %i1 to i32
  %cmp = icmp slt i32 %conv, %conv2
  %i2 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %data) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %data.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %data, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %i, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::ReusableArenaBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockSize) #2 comdat align 2 {
entry:
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockSize.addr = alloca i16, align 2
  %theInstance = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %i = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %theInstance to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %i2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %call = call noundef %"class.xalanc_1_10::ReusableArenaBlock"* @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %theInstance, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i2, i16* noundef nonnull align 2 dereferenceable(2) %theBlockSize.addr)
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %theInstance to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i3) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"* %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this) #2 comdat align 2 {
entry:
  %retval = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theResult = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i, i32 0, i32 1
  %i1 = load i16, i16* %m_objectCount, align 8
  %conv = zext i16 %i1 to i32
  %i2 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i2, i32 0, i32 2
  %i3 = load i16, i16* %m_blockSize, align 2
  %conv2 = zext i16 %i3 to i32
  %cmp = icmp eq i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store %"class.xalanc_1_10::XStringCached"* null, %"class.xalanc_1_10::XStringCached"** %retval, align 8
  br label %return

if.else:                                          ; preds = %entry
  %i4 = bitcast %"class.xalanc_1_10::XStringCached"** %theResult to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i4) #6
  store %"class.xalanc_1_10::XStringCached"* null, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  %m_firstFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i5 = load i16, i16* %m_firstFreeBlock, align 8
  %conv3 = zext i16 %i5 to i32
  %m_nextFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  %i6 = load i16, i16* %m_nextFreeBlock, align 2
  %conv4 = zext i16 %i6 to i32
  %cmp5 = icmp ne i32 %conv3, %conv4
  br i1 %cmp5, label %if.then6, label %if.else9

if.then6:                                         ; preds = %if.else
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i7, i32 0, i32 3
  %i8 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %m_firstFreeBlock7 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i9 = load i16, i16* %m_firstFreeBlock7, align 8
  %conv8 = zext i16 %i9 to i32
  %idx.ext = sext i32 %conv8 to i64
  %add.ptr = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i8, i64 %idx.ext
  store %"class.xalanc_1_10::XStringCached"* %add.ptr, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  br label %if.end

if.else9:                                         ; preds = %if.else
  %i10 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock10 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i10, i32 0, i32 3
  %i11 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock10, align 8
  %m_firstFreeBlock11 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i12 = load i16, i16* %m_firstFreeBlock11, align 8
  %conv12 = zext i16 %i12 to i32
  %idx.ext13 = sext i32 %conv12 to i64
  %add.ptr14 = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i11, i64 %idx.ext13
  store %"class.xalanc_1_10::XStringCached"* %add.ptr14, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  %i13 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  %i14 = bitcast %"class.xalanc_1_10::XStringCached"* %i13 to i8*
  %call = call noundef %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(i8* noundef %i14)
  %next = getelementptr inbounds %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %call, i32 0, i32 0
  %i15 = load i16, i16* %next, align 4
  %m_nextFreeBlock15 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  store i16 %i15, i16* %m_nextFreeBlock15, align 2
  %i16 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectCount16 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i16, i32 0, i32 1
  %i17 = load i16, i16* %m_objectCount16, align 8
  %inc = add i16 %i17, 1
  store i16 %inc, i16* %m_objectCount16, align 8
  br label %if.end

if.end:                                           ; preds = %if.else9, %if.then6
  %i18 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theResult, align 8
  store %"class.xalanc_1_10::XStringCached"* %i18, %"class.xalanc_1_10::XStringCached"** %retval, align 8
  %i19 = bitcast %"class.xalanc_1_10::XStringCached"** %theResult to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i19) #6
  br label %return

return:                                           ; preds = %if.end, %if.then
  %i20 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %retval, align 8
  ret %"class.xalanc_1_10::XStringCached"* %i20
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %retval = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, i32 0, i32 2
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %retval, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %i)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %theRhs) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %i1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %i1, i32 0, i32 0
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  %cmp = icmp eq %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, %i2
  ret i1 %cmp
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %retval = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %retval, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %call)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %node) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  %node.addr = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %node, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %prev = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, i32 0, i32 1
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this1)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %value = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, i32 0, i32 0
  %i1 = call %"class.xalanc_1_10::ReusableArenaBlock"** @"llvm.intel.fakeload.p0p0s_class.xalanc_1_10::ReusableArenaBlocks"(%"class.xalanc_1_10::ReusableArenaBlock"** %value, metadata !95) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %i1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %theRhs) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %i, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare %"class.xalanc_1_10::ReusableArenaBlock"** @"llvm.intel.fakeload.p0p0s_class.xalanc_1_10::ReusableArenaBlocks"(%"class.xalanc_1_10::ReusableArenaBlock"**, metadata) #14

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %data, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %pos) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %data.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %newNode = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  %nextFreeNode = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %data, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %i1 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextFreeNode to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextFreeNode, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  %cmp = icmp ne %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, null
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %m_freeListHeadPtr2 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i3 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr2, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i3, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %m_freeListHeadPtr3 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr3, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4, i32 0, i32 2
  %i5 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i5, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextFreeNode, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %call = call noundef %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, i64 noundef 1)
  %m_freeListHeadPtr4 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr4, align 8
  %m_freeListHeadPtr5 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i6 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr5, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i6, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i7 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %value = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i7, i32 0, i32 0
  %i8 = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 0
  %i9 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %call6 = call noundef %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::ReusableArenaBlock"** noundef %value, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %i8, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i9)
  %i10 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %prev = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i10, i32 0, i32 1
  %i11 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev to i8*
  %i12 = bitcast i8* %i11 to %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"**
  %call7 = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos)
  %prev8 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call7, i32 0, i32 1
  %i13 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev8, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i13, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %i12, align 8
  %i14 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %next9 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i14, i32 0, i32 2
  %i15 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next9 to i8*
  %i16 = bitcast i8* %i15 to %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"**
  %call10 = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos)
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call10, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %i16, align 8
  %i17 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %call11 = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos)
  %prev12 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call11, i32 0, i32 1
  %i18 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev12, align 8
  %next13 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i18, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i17, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next13, align 8
  %i19 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %call14 = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos)
  %prev15 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call14, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i19, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev15, align 8
  %i20 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextFreeNode, align 8
  %m_freeListHeadPtr16 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i20, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr16, align 8
  %i21 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode, align 8
  %i22 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %nextFreeNode to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i22) #6
  %i23 = bitcast %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %newNode to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i23) #6
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i21
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::ReusableArenaBlock"** noundef %address, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %theRhs, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %arg) #12 comdat align 2 {
entry:
  %address.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %theRhs.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %address, %"class.xalanc_1_10::ReusableArenaBlock"*** %address.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %theRhs, %"class.xalanc_1_10::ReusableArenaBlock"*** %theRhs.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %arg, %"class.xercesc_2_7::MemoryManager"** %.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %address.addr, align 8
  %i1 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"** %i to i8*
  %i2 = bitcast i8* %i1 to %"class.xalanc_1_10::ReusableArenaBlock"**
  %i3 = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %theRhs.addr, align 8
  %i4 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %i3, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %i4, %"class.xalanc_1_10::ReusableArenaBlock"** %i2, align 8
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %i2
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::ReusableArenaBlock"* @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theMemoryManager, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %theInstance, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theParam1, i16* noundef nonnull align 2 dereferenceable(2) %theParam2) #2 comdat personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %theMemoryManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theInstance.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %theParam1.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theParam2.addr = alloca i16*, align 8
  %theGuard = alloca %"class.xalanc_1_10::XalanAllocationGuard", align 8
  %exn.slot = alloca i8*, align 8
  %ehselector.slot = alloca i32, align 4
  store %"class.xercesc_2_7::MemoryManager"* %theMemoryManager, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %theInstance, %"class.xalanc_1_10::ReusableArenaBlock"*** %theInstance.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theParam1, %"class.xercesc_2_7::MemoryManager"** %theParam1.addr, align 8
  store i16* %theParam2, i16** %theParam2.addr, align 8
  %i = bitcast %"class.xalanc_1_10::XalanAllocationGuard"* %theGuard to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %i) #6
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %theGuard, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, i64 noundef 32)
  %call = invoke noundef i8* @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %theGuard)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %i2 = bitcast i8* %call to %"class.xalanc_1_10::ReusableArenaBlock"*
  %i3 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theParam1.addr, align 8
  %i4 = load i16*, i16** %theParam2.addr, align 8
  %i5 = load i16, i16* %i4, align 2
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i2, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i3, i16 noundef zeroext %i5)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %i6 = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %theInstance.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %i2, %"class.xalanc_1_10::ReusableArenaBlock"** %i6, align 8
  invoke void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %theGuard)
          to label %invoke.cont2 unwind label %lpad

invoke.cont2:                                     ; preds = %invoke.cont1
  %i7 = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %theInstance.addr, align 8
  %i8 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %i7, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %theGuard) #6
  %i9 = bitcast %"class.xalanc_1_10::XalanAllocationGuard"* %theGuard to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %i9) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"* %i8

lpad:                                             ; preds = %invoke.cont1, %invoke.cont, %entry
  %i10 = landingpad { i8*, i32 }
          cleanup
  %i11 = extractvalue { i8*, i32 } %i10, 0
  store i8* %i11, i8** %exn.slot, align 8
  %i12 = extractvalue { i8*, i32 } %i10, 1
  store i32 %i12, i32* %ehselector.slot, align 4
  call void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %theGuard) #6
  %i13 = bitcast %"class.xalanc_1_10::XalanAllocationGuard"* %theGuard to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %i13) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  %sel = load i32, i32* %ehselector.slot, align 4
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn, 0
  %lpad.val3 = insertvalue { i8*, i32 } %lpad.val, i32 %sel, 1
  resume { i8*, i32 } %lpad.val3
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theMemoryManager, i64 noundef %theSize) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocationGuard"*, align 8
  %theMemoryManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theSize.addr = alloca i64, align 8
  store %"class.xalanc_1_10::XalanAllocationGuard"* %this, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theMemoryManager, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  store i64 %theSize, i64* %theSize.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocationGuard"*, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %i, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %m_pointer = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 1
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  %i2 = load i64, i64* %theSize.addr, align 8
  %i3 = bitcast %"class.xercesc_2_7::MemoryManager"* %i1 to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %i3, align 8
  %i4 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i5 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %i6 = call i1 @llvm.type.test(i8* %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %i7 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = call noundef i8* %i7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, i64 noundef %i2)
  store i8* %call, i8** %m_pointer, align 8
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i8* @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocationGuard"*, align 8
  store %"class.xalanc_1_10::XalanAllocationGuard"* %this, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocationGuard"*, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 1
  %i = load i8*, i8** %m_pointer, align 8
  ret i8* %i
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockSize.addr = alloca i16, align 2
  %agg.tmp.ensured = alloca %"struct.xalanc_1_10::XalanCompileErrorBoolean", align 1
  %i = alloca i16, align 2
  %exn.slot = alloca i8*, align 8
  %ehselector.slot = alloca i32, align 4
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %i1 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %i3 = load i16, i16* %theBlockSize.addr, align 2
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i1, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i2, i16 noundef zeroext %i3)
  %m_firstFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  store i16 0, i16* %m_firstFreeBlock, align 8
  %m_nextFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  store i16 0, i16* %m_nextFreeBlock, align 2
  %i4 = bitcast %"struct.xalanc_1_10::XalanCompileErrorBoolean"* %agg.tmp.ensured to i8*
  call void @llvm.memset.p0i8.i64(i8* align 1 %i4, i8 0, i64 1, i1 false)
  %i5 = bitcast i16* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* %i5) #6
  store i16 0, i16* %i, align 2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i6 = load i16, i16* %i, align 2
  %conv = zext i16 %i6 to i32
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i7, i32 0, i32 2
  %i8 = load i16, i16* %m_blockSize, align 2
  %conv2 = zext i16 %i8 to i32
  %cmp = icmp slt i32 %conv, %conv2
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %i9 = bitcast i16* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 2, i8* %i9) #6
  br label %for.end

for.body:                                         ; preds = %for.cond
  %i10 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i10, i32 0, i32 3
  %i11 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %i12 = load i16, i16* %i, align 2
  %idxprom = zext i16 %i12 to i64
  %arrayidx = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i11, i64 %idxprom
  %i13 = bitcast %"class.xalanc_1_10::XStringCached"* %arrayidx to i8*
  %i14 = bitcast i8* %i13 to %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*
  %i15 = load i16, i16* %i, align 2
  %conv3 = zext i16 %i15 to i32
  %add = add nsw i32 %conv3, 1
  %conv4 = trunc i32 %add to i16
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %i14, i16 noundef zeroext %conv4)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %invoke.cont
  %i16 = load i16, i16* %i, align 2
  %inc = add i16 %i16, 1
  store i16 %inc, i16* %i, align 2
  br label %for.cond

lpad:                                             ; preds = %for.body
  %i17 = landingpad { i8*, i32 }
          cleanup
  %i18 = extractvalue { i8*, i32 } %i17, 0
  store i8* %i18, i8** %exn.slot, align 8
  %i19 = extractvalue { i8*, i32 } %i17, 1
  store i32 %i19, i32* %ehselector.slot, align 4
  %i20 = bitcast i16* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 2, i8* %i20) #6
  %i21 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i21) #6
  br label %eh.resume

for.end:                                          ; preds = %for.cond.cleanup
  ret void

eh.resume:                                        ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  %sel = load i32, i32* %ehselector.slot, align 4
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn, 0
  %lpad.val5 = insertvalue { i8*, i32 } %lpad.val, i32 %sel, 1
  resume { i8*, i32 } %lpad.val5
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocationGuard"*, align 8
  store %"class.xalanc_1_10::XalanAllocationGuard"* %this, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocationGuard"*, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 1
  store i8* null, i8** %m_pointer, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(%"class.xalanc_1_10::XalanAllocationGuard"* noundef nonnull align 8 dereferenceable(16) %this) unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocationGuard"*, align 8
  store %"class.xalanc_1_10::XalanAllocationGuard"* %this, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocationGuard"*, %"class.xalanc_1_10::XalanAllocationGuard"** %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 1
  %i = load i8*, i8** %m_pointer, align 8
  %cmp = icmp ne i8* %i, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 0
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %m_pointer2 = getelementptr inbounds %"class.xalanc_1_10::XalanAllocationGuard", %"class.xalanc_1_10::XalanAllocationGuard"* %this1, i32 0, i32 1
  %i2 = load i8*, i8** %m_pointer2, align 8
  %i3 = bitcast %"class.xercesc_2_7::MemoryManager"* %i1 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %i3, align 8
  %i4 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %i5 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %i6 = call i1 @llvm.type.test(i8* %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %i7 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  invoke void %i7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, i8* noundef %i2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  br label %if.end

if.end:                                           ; preds = %invoke.cont, %entry
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %i8 = landingpad { i8*, i32 }
          catch i8* null
  %i9 = extractvalue { i8*, i32 } %i8, 0
  call void @__clang_call_terminate(i8* %i9) #18
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  %theBlockSize.addr = alloca i16, align 2
  %exn.slot = alloca i8*, align 8
  %ehselector.slot = alloca i32, align 4
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store i16 %theBlockSize, i16* %theBlockSize.addr, align 2
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %m_allocator, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i)
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 1
  store i16 0, i16* %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %theBlockSize.addr, align 2
  store i16 %i1, i16* %m_blockSize, align 2
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 3
  %m_allocator2 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 0
  %m_blockSize3 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i2 = load i16, i16* %m_blockSize3, align 2
  %conv = zext i16 %i2 to i64
  %call = invoke noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %m_allocator2, i64 noundef %conv, i8* noundef null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  store %"class.xalanc_1_10::XStringCached"* %call, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  ret void

lpad:                                             ; preds = %entry
  %i3 = landingpad { i8*, i32 }
          cleanup
  %i4 = extractvalue { i8*, i32 } %i3, 0
  store i8* %i4, i8** %exn.slot, align 8
  %i5 = extractvalue { i8*, i32 } %i3, 1
  store i32 %i5, i32* %ehselector.slot, align 4
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %m_allocator) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  %sel = load i32, i32* %ehselector.slot, align 4
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn, 0
  %lpad.val4 = insertvalue { i8*, i32 } %lpad.val, i32 %sel, 1
  resume { i8*, i32 } %lpad.val4
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #15

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %this, i16 noundef zeroext %_next) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, align 8
  %_next.addr = alloca i16, align 2
  store %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %this.addr, align 8
  store i16 %_next, i16* %_next.addr, align 2
  %this1 = load %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %this.addr, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this1, i32 0, i32 0
  %i = load i16, i16* %_next.addr, align 2
  store i16 %i, i16* %next, align 4
  %verificationStamp = getelementptr inbounds %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this1, i32 0, i32 1
  store i32 -2228259, i32* %verificationStamp, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this) unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 0
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 3
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %m_blockSize, align 2
  %conv = zext i16 %i1 to i64
  invoke void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE10deallocateEPS1_m(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %m_allocator, %"class.xalanc_1_10::XStringCached"* noundef %i, i64 noundef %conv)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  %m_allocator2 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %m_allocator2) #6
  ret void

terminate.lpad:                                   ; preds = %entry
  %i2 = landingpad { i8*, i32 }
          catch i8* null
  %i3 = extractvalue { i8*, i32 } %i2, 0
  call void @__clang_call_terminate(i8* %i3) #18
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocator"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"class.xalanc_1_10::XalanAllocator"* %this, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocator"*, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanAllocator", %"class.xalanc_1_10::XalanAllocator"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %i, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.xalanc_1_10::XStringCached"* @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %this, i64 noundef %size, i8* noundef %arg) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocator"*, align 8
  %size.addr = alloca i64, align 8
  %.addr = alloca i8*, align 8
  store %"class.xalanc_1_10::XalanAllocator"* %this, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  store i64 %size, i64* %size.addr, align 8
  store i8* %arg, i8** %.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocator"*, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanAllocator", %"class.xalanc_1_10::XalanAllocator"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %i1 = load i64, i64* %size.addr, align 8
  %mul = mul i64 %i1, 80
  %i2 = bitcast %"class.xercesc_2_7::MemoryManager"* %i to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %i2, align 8
  %i3 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i4 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %i5 = call i1 @llvm.type.test(i8* %i4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %i6 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = call noundef i8* %i6(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i, i64 noundef %mul)
  %i7 = bitcast i8* %call to %"class.xalanc_1_10::XStringCached"*
  ret %"class.xalanc_1_10::XStringCached"* %i7
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocator"*, align 8
  store %"class.xalanc_1_10::XalanAllocator"* %this, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocator"*, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE10deallocateEPS1_m(%"class.xalanc_1_10::XalanAllocator"* noundef nonnull align 8 dereferenceable(8) %this, %"class.xalanc_1_10::XStringCached"* noundef %p, i64 noundef %arg) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanAllocator"*, align 8
  %p.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %.addr = alloca i64, align 8
  store %"class.xalanc_1_10::XalanAllocator"* %this, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %p, %"class.xalanc_1_10::XStringCached"** %p.addr, align 8
  store i64 %arg, i64* %.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanAllocator"*, %"class.xalanc_1_10::XalanAllocator"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %p.addr, align 8
  %cmp = icmp eq %"class.xalanc_1_10::XStringCached"* %i, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanAllocator", %"class.xalanc_1_10::XalanAllocator"* %this1, i32 0, i32 0
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  %i2 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %p.addr, align 8
  %i3 = bitcast %"class.xalanc_1_10::XStringCached"* %i2 to i8*
  %i4 = bitcast %"class.xercesc_2_7::MemoryManager"* %i1 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %i4, align 8
  %i5 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i5, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.end
  %i6 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %i7 = call i1 @llvm.type.test(i8* %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i7)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.end
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %i8 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  call void %i8(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1, i8* noundef %i3)
  br label %return

return:                                           ; preds = %whpr.continue, %if.then
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(i8* noundef %thePointer) #12 comdat align 2 {
entry:
  %thePointer.addr = alloca i8*, align 8
  store i8* %thePointer, i8** %thePointer.addr, align 8
  %i = load i8*, i8** %thePointer.addr, align 8
  %i1 = bitcast i8* %i to %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*
  ret %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %i1
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this, %"class.xalanc_1_10::XStringCached"* noundef %arg) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %arg, %"class.xalanc_1_10::XStringCached"** %.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %m_nextFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  %i = load i16, i16* %m_nextFreeBlock, align 2
  %m_firstFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  store i16 %i, i16* %m_firstFreeBlock, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(%"class.std::reverse_iterator"* noalias sret(%"class.std::reverse_iterator") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %i = bitcast %"class.std::reverse_iterator"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %agg.tmp, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %coerce.dive2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %agg.tmp, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive2, align 8
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator"* noalias sret(%"class.std::reverse_iterator") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %i = bitcast %"class.std::reverse_iterator"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %agg.tmp, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %coerce.dive2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %agg.tmp, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive2, align 8
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %__x, %"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %__y) #16 comdat {
entry:
  %__x.addr = alloca %"class.std::reverse_iterator"*, align 8
  %__y.addr = alloca %"class.std::reverse_iterator"*, align 8
  store %"class.std::reverse_iterator"* %__x, %"class.std::reverse_iterator"** %__x.addr, align 8
  store %"class.std::reverse_iterator"* %__y, %"class.std::reverse_iterator"** %__y.addr, align 8
  %i = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %__x.addr, align 8
  %i1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %__y.addr, align 8
  %call = call noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i, %"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i1)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.std::reverse_iterator"*, align 8
  %__tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  store %"class.std::reverse_iterator"* %this, %"class.std::reverse_iterator"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %__tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %current = getelementptr inbounds %"class.std::reverse_iterator", %"class.std::reverse_iterator"* %this1, i32 0, i32 0
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %__tmp to i8*
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %current to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i1, i8* align 8 %i2, i64 8, i1 false)
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i3) #6
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %__tmp)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %call2 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  %i5 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %__tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i5) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %call2
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE10ownsObjectEPKS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %i1 = bitcast %"class.xalanc_1_10::XStringCached"* %i to i8*
  %call = call noundef %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPKv(i8* noundef %i1)
  %call2 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this1, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef %call)
  ret i1 %call2
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.std::reverse_iterator"* @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.std::reverse_iterator"*, align 8
  %coerce = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  store %"class.std::reverse_iterator"* %this, %"class.std::reverse_iterator"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %this.addr, align 8
  %current = getelementptr inbounds %"class.std::reverse_iterator", %"class.std::reverse_iterator"* %this1, i32 0, i32 0
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %current)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %coerce, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"class.std::reverse_iterator"* %this1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %__x.coerce) unnamed_addr #5 comdat align 2 {
entry:
  %__x = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"class.std::reverse_iterator"*, align 8
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %__x, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %__x.coerce, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  store %"class.std::reverse_iterator"* %this, %"class.std::reverse_iterator"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %this.addr, align 8
  %i = bitcast %"class.std::reverse_iterator"* %this1 to %"struct.std::iterator"*
  %current = getelementptr inbounds %"class.std::reverse_iterator", %"class.std::reverse_iterator"* %this1, i32 0, i32 0
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %current to i8*
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %__x to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i1, i8* align 8 %i2, i64 8, i1 false)
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #17

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %__x, %"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %__y) #16 comdat {
entry:
  %__x.addr = alloca %"class.std::reverse_iterator"*, align 8
  %__y.addr = alloca %"class.std::reverse_iterator"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp1 = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  store %"class.std::reverse_iterator"* %__x, %"class.std::reverse_iterator"** %__x.addr, align 8
  store %"class.std::reverse_iterator"* %__y, %"class.std::reverse_iterator"** %__y.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %i1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %__x.addr, align 8
  %call = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i1)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %i3 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %__y.addr, align 8
  %call2 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %i3)
  %coerce.dive3 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call2, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp1)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  %i5 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i5) #6
  ret i1 %call4
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"class.std::reverse_iterator"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %retval = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"class.std::reverse_iterator"*, align 8
  store %"class.std::reverse_iterator"* %this, %"class.std::reverse_iterator"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator"*, %"class.std::reverse_iterator"** %this.addr, align 8
  %current = getelementptr inbounds %"class.std::reverse_iterator", %"class.std::reverse_iterator"* %this1, i32 0, i32 0
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval to i8*
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %current to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i, i8* align 8 %i1, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval, i32 0, i32 0
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %retval = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %prev = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, i32 0, i32 1
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval to i8*
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i2, i8* align 8 %i3, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval, i32 0, i32 0
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %value = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, i32 0, i32 0
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %value
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef %block) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %block.addr = alloca %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  store %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %block, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %block.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %i1 = load %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %block.addr, align 8
  %i2 = bitcast %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %i1 to %"class.xalanc_1_10::XStringCached"*
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i, %"class.xalanc_1_10::XStringCached"* noundef %i2)
  br i1 %call, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %i3 = load %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %block.addr, align 8
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i4, i32 0, i32 2
  %i5 = load i16, i16* %m_blockSize, align 2
  %call2 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock10isValidForEt(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %i3, i16 noundef zeroext %i5)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %i6 = phi i1 [ false, %entry ], [ %call2, %land.rhs ]
  %lnot = xor i1 %i6, true
  ret i1 %lnot
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPKv(i8* noundef %thePointer) #12 comdat align 2 {
entry:
  %thePointer.addr = alloca i8*, align 8
  store i8* %thePointer, i8** %thePointer.addr, align 8
  %i = load i8*, i8** %thePointer.addr, align 8
  %i1 = bitcast i8* %i to %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*
  ret %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %i1
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %m_blockSize, align 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this1, %"class.xalanc_1_10::XStringCached"* noundef %i, i16 noundef zeroext %i1)
  ret i1 %call
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock10isValidForEt(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %this, i16 noundef zeroext %rightBorder) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, align 8
  %rightBorder.addr = alloca i16, align 2
  store %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %this.addr, align 8
  store i16 %rightBorder, i16* %rightBorder.addr, align 2
  %this1 = load %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %this.addr, align 8
  %verificationStamp = getelementptr inbounds %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this1, i32 0, i32 1
  %i = load i32, i32* %verificationStamp, align 4
  %cmp = icmp eq i32 %i, -2228259
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %next = getelementptr inbounds %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %this1, i32 0, i32 0
  %i1 = load i16, i16* %next, align 4
  %conv = zext i16 %i1 to i32
  %i2 = load i16, i16* %rightBorder.addr, align 2
  %conv2 = zext i16 %i2 to i32
  %cmp3 = icmp sle i32 %conv, %conv2
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %i3 = phi i1 [ false, %entry ], [ %cmp3, %land.rhs ]
  %i4 = zext i1 %i3 to i64
  %cond = select i1 %i3, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject, i16 noundef zeroext %rightBoundary) #12 comdat align 2 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %rightBoundary.addr = alloca i16, align 2
  %functor = alloca %"struct.std::less", align 1
  %cleanup.dest.slot = alloca i32, align 4
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  store i16 %rightBoundary, i16* %rightBoundary.addr, align 2
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %i = load i16, i16* %rightBoundary.addr, align 2
  %conv = zext i16 %i to i32
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %m_blockSize, align 2
  %conv2 = zext i16 %i1 to i32
  %cmp = icmp sgt i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %m_blockSize3 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 2
  %i2 = load i16, i16* %m_blockSize3, align 2
  store i16 %i2, i16* %rightBoundary.addr, align 2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %i3 = bitcast %"struct.std::less"* %functor to i8*
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %i3) #6
  %i4 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 3
  %i5 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %call = call noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* noundef nonnull align 1 dereferenceable(1) %functor, %"class.xalanc_1_10::XStringCached"* noundef %i4, %"class.xalanc_1_10::XStringCached"* noundef %i5) #6
  %conv4 = zext i1 %call to i32
  %cmp5 = icmp eq i32 %conv4, 0
  br i1 %cmp5, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %if.end
  %i6 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %m_objectBlock6 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 3
  %i7 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock6, align 8
  %i8 = load i16, i16* %rightBoundary.addr, align 2
  %conv7 = zext i16 %i8 to i32
  %idx.ext = sext i32 %conv7 to i64
  %add.ptr = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i7, i64 %idx.ext
  %call8 = call noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* noundef nonnull align 1 dereferenceable(1) %functor, %"class.xalanc_1_10::XStringCached"* noundef %i6, %"class.xalanc_1_10::XStringCached"* noundef %add.ptr) #6
  %conv9 = zext i1 %call8 to i32
  %cmp10 = icmp eq i32 %conv9, 1
  br i1 %cmp10, label %if.then11, label %if.else

if.then11:                                        ; preds = %land.lhs.true
  store i1 true, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.else:                                          ; preds = %land.lhs.true, %if.end
  store i1 false, i1* %retval, align 1
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then11
  %i9 = bitcast %"struct.std::less"* %functor to i8*
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %i9) #6
  %i10 = load i1, i1* %retval, align 1
  ret i1 %i10
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(%"struct.std::less"* noundef nonnull align 1 dereferenceable(1) %this, %"class.xalanc_1_10::XStringCached"* noundef %__x, %"class.xalanc_1_10::XStringCached"* noundef %__y) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.std::less"*, align 8
  %__x.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %__y.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"struct.std::less"* %this, %"struct.std::less"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %__x, %"class.xalanc_1_10::XStringCached"** %__x.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %__y, %"class.xalanc_1_10::XStringCached"** %__y.addr, align 8
  %this1 = load %"struct.std::less"*, %"struct.std::less"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %__x.addr, align 8
  %i1 = ptrtoint %"class.xalanc_1_10::XStringCached"* %i to i64
  %i2 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %__y.addr, align 8
  %i3 = ptrtoint %"class.xalanc_1_10::XStringCached"* %i2 to i64
  %cmp = icmp ult i64 %i1, %i3
  ret i1 %cmp
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %theRhs) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %i = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %theRhs.addr, align 8
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this1, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %i)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this) #12 comdat align 2 {
entry:
  %retval = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, i32 0, i32 2
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval to i8*
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i2, i8* align 8 %i3, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %retval, i32 0, i32 0
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  ret %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i1) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %data) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %data.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"**, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"** %data, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"**, %"class.xalanc_1_10::ReusableArenaBlock"*** %data.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"class.xalanc_1_10::ReusableArenaBlock"** noundef nonnull align 8 dereferenceable(8) %i, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xercesc_2_7::MemoryManager"* @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 0
  %i = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  ret %"class.xercesc_2_7::MemoryManager"* %i
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %pos) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %node) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %node.addr = alloca %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %node, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i, i32 0, i32 2
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %prev = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, i32 0, i32 1
  %i3 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev, align 8
  %next2 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i3, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next2, align 8
  %i4 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %prev3 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i4, i32 0, i32 1
  %i5 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev3, align 8
  %i6 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %next4 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i6, i32 0, i32 2
  %i7 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next4, align 8
  %prev5 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i7, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i5, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev5, align 8
  %i8 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %i9 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %prev6 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i9, i32 0, i32 1
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* null, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %prev6, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  %i10 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr, align 8
  %i11 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %next7 = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i11, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i10, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next7, align 8
  %i12 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %node.addr, align 8
  %m_freeListHeadPtr8 = getelementptr inbounds %"class.xalanc_1_10::XalanList", %"class.xalanc_1_10::XalanList"* %this1, i32 0, i32 2
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i12, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %m_freeListHeadPtr8, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this, %"class.xalanc_1_10::XStringCached"* noundef %theObject) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theObject.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  %p = alloca i8*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  store %"class.xalanc_1_10::XStringCached"* %theObject, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %m_firstFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i = load i16, i16* %m_firstFreeBlock, align 8
  %conv = zext i16 %i to i32
  %m_nextFreeBlock = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  %i1 = load i16, i16* %m_nextFreeBlock, align 2
  %conv2 = zext i16 %i1 to i32
  %cmp = icmp ne i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i2 = bitcast i8** %p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i3, i32 0, i32 3
  %i4 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %m_firstFreeBlock3 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i5 = load i16, i16* %m_firstFreeBlock3, align 8
  %conv4 = zext i16 %i5 to i32
  %idx.ext = sext i32 %conv4 to i64
  %add.ptr = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i4, i64 %idx.ext
  %i6 = bitcast %"class.xalanc_1_10::XStringCached"* %add.ptr to i8*
  store i8* %i6, i8** %p, align 8
  %i7 = load i8*, i8** %p, align 8
  %i8 = bitcast i8* %i7 to %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*
  %m_nextFreeBlock5 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  %i9 = load i16, i16* %m_nextFreeBlock5, align 2
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %i8, i16 noundef zeroext %i9)
  %m_firstFreeBlock6 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i10 = load i16, i16* %m_firstFreeBlock6, align 8
  %m_nextFreeBlock7 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  store i16 %i10, i16* %m_nextFreeBlock7, align 2
  %i11 = bitcast i8** %p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i11) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %i12 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  call void @_ZN11xalanc_1_1012XalanDestroyINS_13XStringCachedEEEvRT_(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80) %i12)
  %i13 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %i14 = bitcast %"class.xalanc_1_10::XStringCached"* %i13 to i8*
  %i15 = bitcast i8* %i14 to %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*
  %m_firstFreeBlock8 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  %i16 = load i16, i16* %m_firstFreeBlock8, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef nonnull align 4 dereferenceable(8) %i15, i16 noundef zeroext %i16)
  %i17 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theObject.addr, align 8
  %i18 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock9 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i18, i32 0, i32 3
  %i19 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock9, align 8
  %sub.ptr.lhs.cast = ptrtoint %"class.xalanc_1_10::XStringCached"* %i17 to i64
  %sub.ptr.rhs.cast = ptrtoint %"class.xalanc_1_10::XStringCached"* %i19 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 80
  %conv10 = trunc i64 %sub.ptr.div to i16
  %m_nextFreeBlock11 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 2
  store i16 %conv10, i16* %m_nextFreeBlock11, align 2
  %m_firstFreeBlock12 = getelementptr inbounds %"class.xalanc_1_10::ReusableArenaBlock", %"class.xalanc_1_10::ReusableArenaBlock"* %this1, i32 0, i32 1
  store i16 %conv10, i16* %m_firstFreeBlock12, align 8
  %i20 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i20, i32 0, i32 1
  %i21 = load i16, i16* %m_objectCount, align 8
  %dec = add i16 %i21, -1
  store i16 %dec, i16* %m_objectCount, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(%"class.xalanc_1_10::ReusableArenaAllocator"* noundef nonnull align 8 dereferenceable(41) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaAllocator"*, align 8
  %iTerator = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %coerce = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp7 = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0", align 8
  %ref.tmp8 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::ReusableArenaAllocator"* %this, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaAllocator"*, %"class.xalanc_1_10::ReusableArenaAllocator"** %this.addr, align 8
  %i = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %conv = zext i1 %call to i32
  %cmp = icmp eq i32 %conv, 0
  br i1 %cmp, label %if.then, label %if.end16

if.then:                                          ; preds = %entry
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iTerator to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks2 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i3, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  %call3 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i5 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call3, align 8
  %i6 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i5 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE7isEmptyEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i6)
  br i1 %call4, label %if.then5, label %if.end15

if.then5:                                         ; preds = %if.then
  %call6 = call %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %coerce, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %call6, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %coerce.dive, align 8
  %i7 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i7) #6
  %i8 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i8) #6
  %i9 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks9 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i9, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp8, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks9)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp7, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp8)
  %call10 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator, %"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %ref.tmp7)
  br i1 %call10, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %if.then5
  %call11 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %iTerator)
  %i10 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call11, align 8
  %i11 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i10 to %"class.xalanc_1_10::ArenaBlockBase"*
  %call12 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i11)
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %if.then5
  %i12 = phi i1 [ true, %if.then5 ], [ %call12, %lor.rhs ]
  %i13 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp8 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i13) #6
  %i14 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %ref.tmp7 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i14) #6
  br i1 %i12, label %if.then13, label %if.end

if.then13:                                        ; preds = %lor.end
  %i15 = bitcast %"class.xalanc_1_10::ReusableArenaAllocator"* %this1 to %"class.xalanc_1_10::ArenaAllocator"*
  %m_blocks14 = getelementptr inbounds %"class.xalanc_1_10::ArenaAllocator", %"class.xalanc_1_10::ArenaAllocator"* %i15, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %m_blocks14)
  br label %if.end

if.end:                                           ; preds = %if.then13, %lor.end
  br label %if.end15

if.end15:                                         ; preds = %if.end, %if.then
  %i16 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase.0"* %iTerator to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i16) #6
  br label %if.end16

if.end16:                                         ; preds = %if.end15, %entry
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %this, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  %next = getelementptr inbounds %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, i32 0, i32 2
  %i2 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %next, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %this1, i32 0, i32 0
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i2, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %this1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(%"class.std::reverse_iterator.1"* noalias sret(%"class.std::reverse_iterator.1") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %i = bitcast %"class.std::reverse_iterator.1"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(%"class.std::reverse_iterator.1"* noalias sret(%"class.std::reverse_iterator.1") align 8 %agg.result, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %agg.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %i = bitcast %"class.std::reverse_iterator.1"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %agg.tmp)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %__x, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %__y) #16 comdat {
entry:
  %__x.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %__y.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  store %"class.std::reverse_iterator.1"* %__x, %"class.std::reverse_iterator.1"** %__x.addr, align 8
  store %"class.std::reverse_iterator.1"* %__y, %"class.std::reverse_iterator.1"** %__y.addr, align 8
  %i = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %__x.addr, align 8
  %i1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %__y.addr, align 8
  %call = call noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %i, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %i1)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %__tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.std::reverse_iterator.1"* %this, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %__tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %current = getelementptr inbounds %"class.std::reverse_iterator.1", %"class.std::reverse_iterator.1"* %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %current)
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__tmp)
  %call = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i2) #6
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %__tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i3) #6
  ret %"class.xalanc_1_10::ReusableArenaBlock"** %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) %"class.std::reverse_iterator.1"* @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.std::reverse_iterator.1"* %this, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %current = getelementptr inbounds %"class.std::reverse_iterator.1", %"class.std::reverse_iterator.1"* %this1, i32 0, i32 0
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %current)
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i1) #6
  ret %"class.std::reverse_iterator.1"* %this1
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1012XalanDestroyINS_13XStringCachedEEEvRT_(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80) %theArg) #12 comdat {
entry:
  %theArg.addr = alloca %"class.xalanc_1_10::XStringCached"*, align 8
  store %"class.xalanc_1_10::XStringCached"* %theArg, %"class.xalanc_1_10::XStringCached"** %theArg.addr, align 8
  %i = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %theArg.addr, align 8
  %i1 = bitcast %"class.xalanc_1_10::XStringCached"* %i to void (%"class.xalanc_1_10::XStringCached"*)***
  %vtable = load void (%"class.xalanc_1_10::XStringCached"*)**, void (%"class.xalanc_1_10::XStringCached"*)*** %i1, align 8
  %i2 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i3 = bitcast void (%"class.xalanc_1_10::XStringCached"*)** %vtable to i8*
  %i4 = call i1 @llvm.type.test(i8* %i3, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  call void @llvm.assume(i1 %i4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%"class.xalanc_1_10::XStringCached"*)*, void (%"class.xalanc_1_10::XStringCached"*)** %vtable, i64 0
  %i5 = load void (%"class.xalanc_1_10::XStringCached"*)*, void (%"class.xalanc_1_10::XStringCached"*)** %vfn, align 8
  call void %i5(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80) %i) #6
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(%"struct.xalanc_1_10::XalanListIteratorBase.0"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %theRhs) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase.0"*, align 8
  %theRhs.addr = alloca %"struct.xalanc_1_10::XalanListIteratorBase"*, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  store %"struct.xalanc_1_10::XalanListIteratorBase"* %theRhs, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanListIteratorBase.0"*, %"struct.xalanc_1_10::XalanListIteratorBase.0"** %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase.0", %"struct.xalanc_1_10::XalanListIteratorBase.0"* %this1, i32 0, i32 0
  %i = load %"struct.xalanc_1_10::XalanListIteratorBase"*, %"struct.xalanc_1_10::XalanListIteratorBase"** %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct.xalanc_1_10::XalanListIteratorBase", %"struct.xalanc_1_10::XalanListIteratorBase"* %i, i32 0, i32 0
  %i1 = load %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode2, align 8
  store %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* %i1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"** %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE7isEmptyEv(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %this) #12 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ArenaBlockBase"*, align 8
  store %"class.xalanc_1_10::ArenaBlockBase"* %this, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ArenaBlockBase"*, %"class.xalanc_1_10::ArenaBlockBase"** %this.addr, align 8
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %this1, i32 0, i32 1
  %i = load i16, i16* %m_objectCount, align 8
  %conv = zext i16 %i to i32
  %cmp = icmp eq i32 %conv, 0
  %i1 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %this, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %__x) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  store %"class.std::reverse_iterator.1"* %this, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %i = bitcast %"class.std::reverse_iterator.1"* %this1 to %"struct.std::iterator.2"*
  %current = getelementptr inbounds %"class.std::reverse_iterator.1", %"class.std::reverse_iterator.1"* %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %current, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__x)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(%"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %__x, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %__y) #16 comdat {
entry:
  %__x.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %__y.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp1 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.std::reverse_iterator.1"* %__x, %"class.std::reverse_iterator.1"** %__x.addr, align 8
  store %"class.std::reverse_iterator.1"* %__y, %"class.std::reverse_iterator.1"** %__y.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  %i1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %__x.addr, align 8
  call void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %i1)
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i2) #6
  %i3 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %__y.addr, align 8
  call void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp1, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %i3)
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp1)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  %i5 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i5) #6
  ret i1 %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noalias sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %agg.result, %"class.std::reverse_iterator.1"* noundef nonnull align 8 dereferenceable(8) %this) #2 comdat align 2 {
entry:
  %result.ptr = alloca i8*, align 8
  %this.addr = alloca %"class.std::reverse_iterator.1"*, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %agg.result to i8*
  store i8* %i, i8** %result.ptr, align 8
  store %"class.std::reverse_iterator.1"* %this, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %this1 = load %"class.std::reverse_iterator.1"*, %"class.std::reverse_iterator.1"** %this.addr, align 8
  %current = getelementptr inbounds %"class.std::reverse_iterator.1", %"class.std::reverse_iterator.1"* %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %agg.result, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %current)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local %"class.xercesc_2_7::MemoryManager"* @_ZSt8for_eachIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEENS0_13DeleteFunctorIS5_EEET0_T_SF_SE_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef %__first, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef %__last, %"class.xercesc_2_7::MemoryManager"* %__f.coerce) #2 comdat {
entry:
  %retval = alloca %"struct.xalanc_1_10::DeleteFunctor", align 8
  %__f = alloca %"struct.xalanc_1_10::DeleteFunctor", align 8
  %tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %coerce.dive = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %__f, i32 0, i32 0
  store %"class.xercesc_2_7::MemoryManager"* %__f.coerce, %"class.xercesc_2_7::MemoryManager"** %coerce.dive, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__first, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__last)
  br i1 %call, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call1 = call noundef nonnull align 8 dereferenceable(8) %"class.xalanc_1_10::ReusableArenaBlock"** @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__first)
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %call1, align 8
  call void @_ZNK11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_(%"struct.xalanc_1_10::DeleteFunctor"* noundef nonnull align 8 dereferenceable(8) %__f, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %i)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %__first)
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i2) #6
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %i3 = bitcast %"struct.xalanc_1_10::DeleteFunctor"* %retval to i8*
  %i4 = bitcast %"struct.xalanc_1_10::DeleteFunctor"* %__f to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i3, i8* align 8 %i4, i64 8, i1 false)
  %coerce.dive2 = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %retval, i32 0, i32 0
  %i5 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %coerce.dive2, align 8
  ret %"class.xercesc_2_7::MemoryManager"* %i5
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::DeleteFunctor"* noundef nonnull align 8 dereferenceable(8) %this, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theManager) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::DeleteFunctor"*, align 8
  %theManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"struct.xalanc_1_10::DeleteFunctor"* %this, %"struct.xalanc_1_10::DeleteFunctor"** %this.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theManager, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  %this1 = load %"struct.xalanc_1_10::DeleteFunctor"*, %"struct.xalanc_1_10::DeleteFunctor"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::DeleteFunctor"* %this1 to %"struct.std::unary_function"*
  %m_memoryManager = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %this1, i32 0, i32 0
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theManager.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %i1, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5clearEv(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.xalanc_1_10::XalanList"*, align 8
  %pos = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp2 = alloca %"struct.xalanc_1_10::XalanListIteratorBase", align 8
  store %"class.xalanc_1_10::XalanList"* %this, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::XalanList"*, %"class.xalanc_1_10::XalanList"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %pos to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %pos, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i1 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i1) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, %"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  %i2 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i2) #6
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %i3 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i3) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(%"struct.xalanc_1_10::XalanListIteratorBase"* sret(%"struct.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp2, %"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %pos, i32 noundef 0)
  %call3 = call noundef nonnull align 8 dereferenceable(24) %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(%"struct.xalanc_1_10::XalanListIteratorBase"* noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(%"class.xalanc_1_10::XalanList"* noundef nonnull align 8 dereferenceable(24) %this1, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node"* noundef nonnull align 8 dereferenceable(24) %call3)
  %i4 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %ref.tmp2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i4) #6
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %i5 = bitcast %"struct.xalanc_1_10::XalanListIteratorBase"* %pos to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i5) #6
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_(%"struct.xalanc_1_10::DeleteFunctor"* noundef nonnull align 8 dereferenceable(8) %this, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %thePointer) #2 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::DeleteFunctor"*, align 8
  %thePointer.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %ref.tmp = alloca %"struct.xalanc_1_10::XalanDestroyFunctor", align 1
  %undef.agg.tmp = alloca %"struct.xalanc_1_10::XalanDestroyFunctor", align 1
  store %"struct.xalanc_1_10::DeleteFunctor"* %this, %"struct.xalanc_1_10::DeleteFunctor"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %thePointer, %"class.xalanc_1_10::ReusableArenaBlock"** %thePointer.addr, align 8
  %this1 = load %"struct.xalanc_1_10::DeleteFunctor"*, %"struct.xalanc_1_10::DeleteFunctor"** %this.addr, align 8
  %i = bitcast %"struct.xalanc_1_10::XalanDestroyFunctor"* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %i) #6
  %i1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %thePointer.addr, align 8
  call void @_ZN11xalanc_1_1023makeXalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_19XalanDestroyFunctorIT_EEPKS5_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef %i1)
  %i2 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %thePointer.addr, align 8
  %m_memoryManager = getelementptr inbounds %"struct.xalanc_1_10::DeleteFunctor", %"struct.xalanc_1_10::DeleteFunctor"* %this1, i32 0, i32 0
  %i3 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %m_memoryManager, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_RN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %ref.tmp, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %i2, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i3)
  %i4 = bitcast %"struct.xalanc_1_10::XalanDestroyFunctor"* %ref.tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %i4) #6
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1023makeXalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_19XalanDestroyFunctorIT_EEPKS5_(%"class.xalanc_1_10::ReusableArenaBlock"* noundef %arg) #12 comdat {
entry:
  %.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %arg, %"class.xalanc_1_10::ReusableArenaBlock"** %.addr, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_RN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %this, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %theArg, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theMemoryManager) #2 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanDestroyFunctor"*, align 8
  %theArg.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theMemoryManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"struct.xalanc_1_10::XalanDestroyFunctor"* %this, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %theArg, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theMemoryManager, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanDestroyFunctor"*, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  %i1 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPS3_RN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %this1, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %i, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPS3_RN11xercesc_2_713MemoryManagerE(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %this, %"class.xalanc_1_10::ReusableArenaBlock"* noundef %theArg, %"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %theMemoryManager) #2 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanDestroyFunctor"*, align 8
  %theArg.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %theMemoryManager.addr = alloca %"class.xercesc_2_7::MemoryManager"*, align 8
  store %"struct.xalanc_1_10::XalanDestroyFunctor"* %this, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %theArg, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  store %"class.xercesc_2_7::MemoryManager"* %theMemoryManager, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanDestroyFunctor"*, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  %cmp = icmp ne %"class.xalanc_1_10::ReusableArenaBlock"* %i, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclERS3_(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %this1, %"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i1)
  %i2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %theMemoryManager.addr, align 8
  %i3 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %i3 to i8*
  %i5 = bitcast %"class.xercesc_2_7::MemoryManager"* %i2 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %i5, align 8
  %i6 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i6, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %i7 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %i8 = call i1 @llvm.type.test(i8* %i7, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i8)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %i9 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  call void %i9(%"class.xercesc_2_7::MemoryManager"* noundef nonnull align 8 dereferenceable(8) %i2, i8* noundef %i4)
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %entry
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclERS3_(%"struct.xalanc_1_10::XalanDestroyFunctor"* noundef nonnull align 1 dereferenceable(1) %this, %"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %theArg) #12 comdat align 2 {
entry:
  %this.addr = alloca %"struct.xalanc_1_10::XalanDestroyFunctor"*, align 8
  %theArg.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  store %"struct.xalanc_1_10::XalanDestroyFunctor"* %this, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %theArg, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  %this1 = load %"struct.xalanc_1_10::XalanDestroyFunctor"*, %"struct.xalanc_1_10::XalanDestroyFunctor"** %this.addr, align 8
  %i = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %theArg.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtED2Ev(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %i) #6
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtED2Ev(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this) unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %this.addr = alloca %"class.xalanc_1_10::ReusableArenaBlock"*, align 8
  %removedObjects = alloca i16, align 2
  %i = alloca i16, align 2
  %pStruct = alloca %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, align 8
  store %"class.xalanc_1_10::ReusableArenaBlock"* %this, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %this1 = load %"class.xalanc_1_10::ReusableArenaBlock"*, %"class.xalanc_1_10::ReusableArenaBlock"** %this.addr, align 8
  %i1 = bitcast i16* %removedObjects to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* %i1) #6
  store i16 0, i16* %removedObjects, align 2
  %i2 = bitcast i16* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* %i2) #6
  store i16 0, i16* %i, align 2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i3 = load i16, i16* %i, align 2
  %conv = zext i16 %i3 to i32
  %i4 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_blockSize = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i4, i32 0, i32 2
  %i5 = load i16, i16* %m_blockSize, align 2
  %conv2 = zext i16 %i5 to i32
  %cmp = icmp slt i32 %conv, %conv2
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %for.cond
  %i6 = load i16, i16* %removedObjects, align 2
  %conv3 = zext i16 %i6 to i32
  %i7 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectCount = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i7, i32 0, i32 1
  %i8 = load i16, i16* %m_objectCount, align 8
  %conv4 = zext i16 %i8 to i32
  %cmp5 = icmp slt i32 %conv3, %conv4
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.cond
  %i9 = phi i1 [ false, %for.cond ], [ %cmp5, %land.rhs ]
  br i1 %i9, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %land.end
  %i10 = bitcast i16* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 2, i8* %i10) #6
  br label %for.end

for.body:                                         ; preds = %land.end
  %i11 = bitcast %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %pStruct to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %i11) #6
  %i12 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i12, i32 0, i32 3
  %i13 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock, align 8
  %i14 = load i16, i16* %i, align 2
  %idxprom = zext i16 %i14 to i64
  %arrayidx = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i13, i64 %idxprom
  %i15 = bitcast %"class.xalanc_1_10::XStringCached"* %arrayidx to i8*
  %call = call noundef %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(i8* noundef %i15)
  store %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* %call, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %pStruct, align 8
  %i16 = load %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"*, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %pStruct, align 8
  %call6 = invoke noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(%"class.xalanc_1_10::ReusableArenaBlock"* noundef nonnull align 8 dereferenceable(28) %this1, %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"* noundef %i16)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %for.body
  br i1 %call6, label %if.then, label %if.end

if.then:                                          ; preds = %invoke.cont
  %i17 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  %m_objectBlock7 = getelementptr inbounds %"class.xalanc_1_10::ArenaBlockBase", %"class.xalanc_1_10::ArenaBlockBase"* %i17, i32 0, i32 3
  %i18 = load %"class.xalanc_1_10::XStringCached"*, %"class.xalanc_1_10::XStringCached"** %m_objectBlock7, align 8
  %i19 = load i16, i16* %i, align 2
  %idxprom8 = zext i16 %i19 to i64
  %arrayidx9 = getelementptr inbounds %"class.xalanc_1_10::XStringCached", %"class.xalanc_1_10::XStringCached"* %i18, i64 %idxprom8
  %i20 = bitcast %"class.xalanc_1_10::XStringCached"* %arrayidx9 to void (%"class.xalanc_1_10::XStringCached"*)***
  %vtable = load void (%"class.xalanc_1_10::XStringCached"*)**, void (%"class.xalanc_1_10::XStringCached"*)*** %i20, align 8
  %i21 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i21, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %i22 = bitcast void (%"class.xalanc_1_10::XStringCached"*)** %vtable to i8*
  %i23 = call i1 @llvm.type.test(i8* %i22, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  call void @llvm.assume(i1 %i23)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds void (%"class.xalanc_1_10::XStringCached"*)*, void (%"class.xalanc_1_10::XStringCached"*)** %vtable, i64 0
  %i24 = load void (%"class.xalanc_1_10::XStringCached"*)*, void (%"class.xalanc_1_10::XStringCached"*)** %vfn, align 8
  call void %i24(%"class.xalanc_1_10::XStringCached"* noundef nonnull align 8 dereferenceable(80) %arrayidx9) #6
  %i25 = load i16, i16* %removedObjects, align 2
  %inc = add i16 %i25, 1
  store i16 %inc, i16* %removedObjects, align 2
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %invoke.cont
  %i26 = bitcast %"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock"** %pStruct to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %i26) #6
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %i27 = load i16, i16* %i, align 2
  %inc10 = add i16 %i27, 1
  store i16 %inc10, i16* %i, align 2
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %i28 = bitcast i16* %removedObjects to i8*
  call void @llvm.lifetime.end.p0i8(i64 2, i8* %i28) #6
  %i29 = bitcast %"class.xalanc_1_10::ReusableArenaBlock"* %this1 to %"class.xalanc_1_10::ArenaBlockBase"*
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(%"class.xalanc_1_10::ArenaBlockBase"* noundef nonnull align 8 dereferenceable(24) %i29) #6
  ret void

terminate.lpad:                                   ; preds = %for.body
  %i30 = landingpad { i8*, i32 }
          catch i8* null
  %i31 = extractvalue { i8*, i32 } %i30, 0
  call void @__clang_call_terminate(i8* %i31) #18
  unreachable
}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C++ TBAA"}
!41 = !{!"pointer@_ZTSPN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE", !18, i64 0}
!45 = !{!"pointer@_ZTSPN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE", !18, i64 0}
!61 = !{!"struct@_ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE", !45, i64 0, !41, i64 8, !41, i64 16}
!95 = !{!61, !45, i64 0}
