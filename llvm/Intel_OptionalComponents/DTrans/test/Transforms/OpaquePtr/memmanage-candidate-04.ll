; This test verifies that calls in member functions of XStringCachedAllocator
; are not inlined and other calls in member functions of ReusableArenaAllocator
; are inlined.

; The forms of the functions here are taken from the new pass manager IR
; at the time of the dtrans-force-inline-op transformation when using
; opaque pointers.

; RUN: opt < %s -opaque-pointers -S -passes='module(dtrans-force-inline-op),cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost 2>&1 | FileCheck %s

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
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" = type { %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" }
%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor" = type { ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" = type { %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" = type { i16, i32 }
%"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard" = type { ptr, ptr }
%"struct._ZTSN11xalanc_1_1024XalanCompileErrorBooleanILb1EEE.xalanc_1_10::XalanCompileErrorBoolean" = type { [1 x i8] }
%"struct._ZTSSt4lessIPKN11xalanc_1_1013XStringCachedEE.std::less" = type { i8 }
%"struct._ZTSN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanDestroyFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" = type opaque
%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" = type { %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext", ptr }
%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" = type <{ %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject" = type <{ ptr, i32, [4 x i8] }>

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

@_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local unnamed_addr constant { [8 x ptr] } { [8 x ptr] [ptr null, ptr @_ZTIN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev, ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev, ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv, ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_, ptr @_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3, !type !4, !type !5, !type !6, !type !7, !type !8, !type !9, !intel_dtrans_type !10
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local constant [61 x i8] c"N11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local constant [83 x i8] c"N11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE\00", comdat, align 1
@_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE }, comdat, align 8, !intel_dtrans_type !13
@_ZTIN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE, ptr @_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE }, comdat, align 8, !intel_dtrans_type !14
@_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE = linkonce_odr dso_local unnamed_addr constant { [8 x ptr] } { [8 x ptr] [ptr null, ptr @_ZTIN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16commitAllocationEPS1_, ptr @_ZNK11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE10ownsObjectEPKS1_, ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3, !type !4, !intel_dtrans_type !10

@_ZN11xalanc_1_1022XStringCachedAllocatorC1ERN11xercesc_2_713MemoryManagerEt = dso_local unnamed_addr alias void (ptr, ptr, i16), ptr @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt
@_ZN11xalanc_1_1022XStringCachedAllocatorD1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev

; Function Attrs: uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockCount) unnamed_addr #0 align 2 !intel.dtrans.func.type !97 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !98
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
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
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize, i1 noundef zeroext %destroyBlocks) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !99 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
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
  %2 = load i8, ptr %destroyBlocks.addr, align 1, !range !101
  %tobool = trunc i8 %2 to i1
  %frombool2 = zext i1 %tobool to i8
  store i8 %frombool2, ptr %m_destroyBlocks, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 align 2 !intel.dtrans.func.type !102 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !98
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator) #6
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !103 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  store ptr getelementptr inbounds ({ [8 x ptr] }, ptr @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, i32 0, inrange i32 0, i32 2), ptr %this1, align 8
  %vtable = load ptr, ptr %this1, align 8
  %0 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %0, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %1 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE")
  call void @llvm.assume(i1 %1)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 5
  %2 = load ptr, ptr %vfn, align 8
  invoke void %2(ptr noundef nonnull align 8 dereferenceable(40) %this1)
          to label %invoke.cont unwind label %terminate.lpad, !intel_dtrans_type !105

invoke.cont:                                      ; preds = %whpr.continue
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks) #6
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %3 = landingpad { ptr, i32 }
          catch ptr null
  %4 = extractvalue { ptr, i32 } %3, 0
  call void @__clang_call_terminate(ptr %4) #17
  unreachable
}

; Function Attrs: mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %this, ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %theValue) #2 align 2 !intel.dtrans.func.type !107 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !98
  %theValue.addr = alloca ptr, align 8, !intel_dtrans_type !108
  %theBlock = alloca ptr, align 8, !intel_dtrans_type !71
  %theResult = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %theValue, ptr %theValue.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %theBlock) #6
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %call = call noundef ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator)
  store ptr %call, ptr %theBlock, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %theResult) #6
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
  call void @llvm.lifetime.end.p0(i64 8, ptr %theResult) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %theBlock) #6
  ret ptr %3
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg %0, ptr nocapture %1) #3

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="2" %this) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !109 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %ref.tmp = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %0 = load ptr, ptr %call3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %0)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false, %entry
  %m_blocks5 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %call6 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) %this1)
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 1
  %1 = load i16, ptr %m_blockSize, align 8
  %call7 = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(8) %call6, i16 noundef zeroext %1)
  store ptr %call7, ptr %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks5, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %m_blocks8 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call9 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks8)
  %2 = load ptr, ptr %call9, align 8
  %call10 = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(28) %2)
  ret ptr %call10
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !110 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  ret ptr %call
}

declare !intel.dtrans.func.type !111 dso_local void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %0, ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="2" %1, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %2) unnamed_addr #4

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !112 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %fullBlock = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %0 = load ptr, ptr %call, align 8
  %1 = load ptr, ptr %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(28) %0, ptr noundef %1)
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %2 = load ptr, ptr %call3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %2)
  br i1 %call4, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %fullBlock) #6
  %m_blocks5 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call6 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks5)
  %3 = load ptr, ptr %call6, align 8
  store ptr %3, ptr %fullBlock, align 8
  %m_blocks7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks7)
  %m_blocks8 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks8, ptr noundef nonnull align 8 dereferenceable(8) %fullBlock)
  call void @llvm.lifetime.end.p0(i64 8, ptr %fullBlock) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg %0, ptr nocapture %1) #3

; Function Attrs: mustprogress uwtable
define dso_local noundef zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theString) #2 align 2 !intel.dtrans.func.type !113 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !98
  %theString.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %theString, ptr %theString.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theString.addr, align 8
  %call = call noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(41) %m_allocator, ptr noundef %0)
  ret i1 %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) #2 comdat align 2 !intel.dtrans.func.type !114 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %bResult = alloca i8, align 1
  %cleanup.dest.slot = alloca i32, align 4
  %iTerator = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %iEnd = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %block = alloca ptr, align 8, !intel_dtrans_type !37
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %rIterator = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  %rEnd = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  %ref.tmp34 = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  %block38 = alloca ptr, align 8, !intel_dtrans_type !37
  %agg.tmp41 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 1, ptr %bResult) #6
  store i8 0, ptr %bResult, align 1
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = load i8, ptr %bResult, align 1, !range !101
  %tobool = trunc i8 %0 to i1
  store i1 %tobool, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

if.end:                                           ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %iTerator) #6
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %iTerator, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  call void @llvm.lifetime.start.p0(i64 8, ptr %iEnd) #6
  %m_blocks3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %iEnd, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks3)
  br label %while.cond

while.cond:                                       ; preds = %if.end21, %if.end
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %iTerator, ptr noundef nonnull align 8 dereferenceable(8) %iEnd)
  br i1 %call4, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %call5 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %1 = load ptr, ptr %call5, align 8
  %call6 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %1)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %2 = phi i1 [ false, %while.cond ], [ %call6, %land.rhs ]
  br i1 %2, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %call7 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %3 = load ptr, ptr %call7, align 8
  %4 = load ptr, ptr %theObject.addr, align 8
  %call8 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef %4)
  %conv = zext i1 %call8 to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then9, label %if.end21

if.then9:                                         ; preds = %while.body
  %call10 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %5 = load ptr, ptr %call10, align 8
  %6 = load ptr, ptr %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(28) %5, ptr noundef %6)
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %m_blocks11 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks11)
  %call12 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %iTerator, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  br i1 %call12, label %if.then13, label %if.end17

if.then13:                                        ; preds = %if.then9
  call void @llvm.lifetime.start.p0(i64 8, ptr %block) #6
  %call14 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %7 = load ptr, ptr %call14, align 8
  store ptr %7, ptr %block, align 8
  %m_blocks15 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %agg.tmp, ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks15, ptr noundef %agg.tmp)
  %m_blocks16 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks16, ptr noundef nonnull align 8 dereferenceable(8) %block)
  call void @llvm.lifetime.end.p0(i64 8, ptr %block) #6
  br label %if.end17

if.end17:                                         ; preds = %if.then13, %if.then9
  %m_destroyBlocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %this1, i32 0, i32 1
  %8 = load i8, ptr %m_destroyBlocks, align 8, !range !101
  %tobool18 = trunc i8 %8 to i1
  br i1 %tobool18, label %if.then19, label %if.end20

if.then19:                                        ; preds = %if.end17
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(ptr noundef nonnull align 8 dereferenceable(41) %this1)
  br label %if.end20

if.end20:                                         ; preds = %if.then19, %if.end17
  store i8 1, ptr %bResult, align 1
  br label %while.end

if.end21:                                         ; preds = %while.body
  call void @llvm.lifetime.start.p0(i64 8, ptr %tmp) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  call void @llvm.lifetime.end.p0(i64 8, ptr %tmp) #6
  br label %while.cond, !llvm.loop !115

while.end:                                        ; preds = %if.end20, %land.end
  call void @llvm.lifetime.start.p0(i64 8, ptr %rIterator) #6
  %m_blocks22 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %rIterator, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks22)
  call void @llvm.lifetime.start.p0(i64 8, ptr %rEnd) #6
  %m_blocks23 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %rEnd, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks23)
  br label %while.cond24

while.cond24:                                     ; preds = %if.end54, %while.end
  %9 = load i8, ptr %bResult, align 1, !range !101
  %tobool25 = trunc i8 %9 to i1
  br i1 %tobool25, label %land.end28, label %land.rhs26

land.rhs26:                                       ; preds = %while.cond24
  %call27 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %rIterator, ptr noundef nonnull align 8 dereferenceable(8) %rEnd)
  br label %land.end28

land.end28:                                       ; preds = %land.rhs26, %while.cond24
  %10 = phi i1 [ false, %while.cond24 ], [ %call27, %land.rhs26 ]
  br i1 %10, label %while.body29, label %while.end55

while.body29:                                     ; preds = %land.end28
  %call30 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  %11 = load ptr, ptr %call30, align 8
  %12 = load ptr, ptr %theObject.addr, align 8
  %call31 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) %11, ptr noundef %12)
  br i1 %call31, label %if.then32, label %if.end48

if.then32:                                        ; preds = %while.body29
  %call33 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  %13 = load ptr, ptr %call33, align 8
  %14 = load ptr, ptr %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(28) %13, ptr noundef %14)
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp34) #6
  %m_blocks35 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %ref.tmp34, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks35)
  %call36 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %rIterator, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp34)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp34) #6
  br i1 %call36, label %if.then37, label %if.end43

if.then37:                                        ; preds = %if.then32
  call void @llvm.lifetime.start.p0(i64 8, ptr %block38) #6
  %call39 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %15 = load ptr, ptr %call39, align 8
  store ptr %15, ptr %block38, align 8
  %m_blocks40 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %agg.tmp41, ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks40, ptr noundef %agg.tmp41)
  %m_blocks42 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks42, ptr noundef nonnull align 8 dereferenceable(8) %block38)
  call void @llvm.lifetime.end.p0(i64 8, ptr %block38) #6
  br label %if.end43

if.end43:                                         ; preds = %if.then37, %if.then32
  %m_destroyBlocks44 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %this1, i32 0, i32 1
  %16 = load i8, ptr %m_destroyBlocks44, align 8, !range !101
  %tobool45 = trunc i8 %16 to i1
  br i1 %tobool45, label %if.then46, label %if.end47

if.then46:                                        ; preds = %if.end43
  call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(ptr noundef nonnull align 8 dereferenceable(41) %this1)
  br label %if.end47

if.end47:                                         ; preds = %if.then46, %if.end43
  store i8 1, ptr %bResult, align 1
  br label %while.end55

if.end48:                                         ; preds = %while.body29
  %call49 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  %17 = load ptr, ptr %call49, align 8
  %call50 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %18 = load ptr, ptr %call50, align 8
  %cmp51 = icmp eq ptr %17, %18
  br i1 %cmp51, label %if.then52, label %if.else

if.then52:                                        ; preds = %if.end48
  br label %while.end55

if.else:                                          ; preds = %if.end48
  %call53 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  br label %if.end54

if.end54:                                         ; preds = %if.else
  br label %while.cond24, !llvm.loop !117

while.end55:                                      ; preds = %if.then52, %if.end47, %land.end28
  %19 = load i8, ptr %bResult, align 1, !range !101
  %tobool56 = trunc i8 %19 to i1
  store i1 %tobool56, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  call void @llvm.lifetime.end.p0(i64 8, ptr %rEnd) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %rIterator) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %iEnd) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %iTerator) #6
  br label %cleanup

cleanup:                                          ; preds = %while.end55, %if.then
  call void @llvm.lifetime.end.p0(i64 1, ptr %bResult) #6
  %20 = load i1, ptr %retval, align 1
  ret i1 %20
}

; Function Attrs: mustprogress uwtable
define dso_local void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv(ptr noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %this) #2 align 2 !intel.dtrans.func.type !118 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !98
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr noundef nonnull align 8 dereferenceable(40) %m_allocator)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !119 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %agg.tmp2 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %agg.tmp4 = alloca %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", align 8
  %coerce = alloca %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %m_blocks3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp2, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks3)
  %m_blocks5 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks5)
  call void @_ZN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(8) %agg.tmp4, ptr noundef nonnull align 8 dereferenceable(8) %call)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %agg.tmp4, i32 0, i32 0
  %0 = load ptr, ptr %coerce.dive, align 8
  %call6 = call ptr @_ZSt8for_eachIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEENS0_13DeleteFunctorIS5_EEET0_T_SF_SE_(ptr noundef %agg.tmp, ptr noundef %agg.tmp2, ptr %0)
  %coerce.dive7 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %coerce, i32 0, i32 0
  store ptr %call6, ptr %coerce.dive7, align 8
  %m_blocks8 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5clearEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks8)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !120 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theBlockSize.addr = alloca i16, align 2
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  store ptr getelementptr inbounds ({ [8 x ptr] }, ptr @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE, i32 0, inrange i32 0, i32 2), ptr %this1, align 8
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 1
  %0 = load i16, ptr %theBlockSize.addr, align 2
  store i16 %0, ptr %m_blockSize, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %1 = load ptr, ptr %theManager.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks, ptr noundef nonnull align 8 dereferenceable(8) %1)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEED0Ev(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 !intel.dtrans.func.type !121 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr noundef nonnull align 8 dereferenceable(41) %this1) #6
  call void @_ZdlPv(ptr noundef %this1) #18
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE10ownsObjectEPKS1_(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !122 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %iTerator = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %iEnd = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %cleanup.dest.slot = alloca i32, align 4
  %coerce = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %rIterator = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  %rEnd = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i1 false, ptr %retval, align 1
  br label %return

if.end:                                           ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %iTerator) #6
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call3 = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %iTerator, i32 0, i32 0
  store ptr %call3, ptr %coerce.dive, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %iEnd) #6
  %m_blocks4 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call5 = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks4)
  %coerce.dive6 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %iEnd, i32 0, i32 0
  store ptr %call5, ptr %coerce.dive6, align 8
  br label %while.cond

while.cond:                                       ; preds = %if.end13, %if.end
  %call7 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %iTerator, ptr noundef nonnull align 8 dereferenceable(8) %iEnd)
  br i1 %call7, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %call8 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %0 = load ptr, ptr %call8, align 8
  %call9 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %0)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %1 = phi i1 [ false, %while.cond ], [ %call9, %land.rhs ]
  br i1 %1, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %call10 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %2 = load ptr, ptr %call10, align 8
  %3 = load ptr, ptr %theObject.addr, align 8
  %call11 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef %3)
  br i1 %call11, label %if.then12, label %if.end13

if.then12:                                        ; preds = %while.body
  store i1 true, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup32

if.end13:                                         ; preds = %while.body
  %call14 = call ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %coerce.dive15 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %coerce, i32 0, i32 0
  store ptr %call14, ptr %coerce.dive15, align 8
  br label %while.cond, !llvm.loop !123

while.end:                                        ; preds = %land.end
  call void @llvm.lifetime.start.p0(i64 8, ptr %rIterator) #6
  %m_blocks16 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %rIterator, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks16)
  call void @llvm.lifetime.start.p0(i64 8, ptr %rEnd) #6
  %m_blocks17 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %rEnd, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks17)
  br label %while.cond18

while.cond18:                                     ; preds = %if.end29, %while.end
  %call19 = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %rIterator, ptr noundef nonnull align 8 dereferenceable(8) %rEnd)
  br i1 %call19, label %while.body20, label %while.end30

while.body20:                                     ; preds = %while.cond18
  %call21 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  %4 = load ptr, ptr %call21, align 8
  %5 = load ptr, ptr %theObject.addr, align 8
  %call22 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) %4, ptr noundef %5)
  br i1 %call22, label %if.then23, label %if.end24

if.then23:                                        ; preds = %while.body20
  store i1 true, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

if.end24:                                         ; preds = %while.body20
  %call25 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %6 = load ptr, ptr %call25, align 8
  %call26 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  %7 = load ptr, ptr %call26, align 8
  %cmp = icmp eq ptr %6, %7
  br i1 %cmp, label %if.then27, label %if.else

if.then27:                                        ; preds = %if.end24
  br label %while.end30

if.else:                                          ; preds = %if.end24
  %call28 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %rIterator)
  br label %if.end29

if.end29:                                         ; preds = %if.else
  br label %while.cond18, !llvm.loop !124

while.end30:                                      ; preds = %if.then27, %while.cond18
  store i1 false, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %while.end30, %if.then23
  call void @llvm.lifetime.end.p0(i64 8, ptr %rEnd) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %rIterator) #6
  br label %cleanup32

cleanup32:                                        ; preds = %cleanup, %if.then12
  call void @llvm.lifetime.end.p0(i64 8, ptr %iEnd) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %iTerator) #6
  br label %return

return:                                           ; preds = %cleanup32, %if.then
  %8 = load i1, ptr %retval, align 1
  ret i1 %8
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !125 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theManager.addr, align 8
  store ptr %0, ptr %m_memoryManager, align 8
  %m_listHead = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  store ptr null, ptr %m_listHead, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  store ptr null, ptr %m_freeListHeadPtr, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED0Ev(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 !intel.dtrans.func.type !127 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr noundef nonnull align 8 dereferenceable(40) %this1) #6
  call void @_ZdlPv(ptr noundef %this1) #18
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %this) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !128 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  %ref.tmp = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %conv = zext i1 %call to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  %0 = load ptr, ptr %call3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %0)
  %conv5 = zext i1 %call4 to i32
  %cmp6 = icmp eq i32 %conv5, 0
  br i1 %cmp6, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %entry
  %m_blocks7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %call8 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) %this1)
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 1
  %1 = load i16, ptr %m_blockSize, align 8
  %call9 = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(8) %call8, i16 noundef zeroext %1)
  store ptr %call9, ptr %ref.tmp, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks7, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %m_blocks10 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call11 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks10)
  %2 = load ptr, ptr %call11, align 8
  %call12 = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(28) %2)
  ret ptr %call12
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !129 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %0 = load ptr, ptr %call, align 8
  %1 = load ptr, ptr %theObject.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(28) %0, ptr noundef %1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE10ownsObjectEPKS1_(ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !130 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !104
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %fResult = alloca i8, align 1
  %theEnd = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  %i = alloca %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 1, ptr %fResult) #6
  store i8 0, ptr %fResult, align 1
  call void @llvm.lifetime.start.p0(i64 8, ptr %theEnd) #6
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %theEnd, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  call void @llvm.lifetime.start.p0(i64 8, ptr %i) #6
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 %i, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %call = call noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %i, ptr noundef nonnull align 8 dereferenceable(8) %theEnd)
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %i)
  %0 = load ptr, ptr %call3, align 8
  %1 = load ptr, ptr %theObject.addr, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE10ownsObjectEPKS1_(ptr noundef nonnull align 8 dereferenceable(28) %0, ptr noundef %1)
  %conv = zext i1 %call4 to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  store i8 1, ptr %fResult, align 1
  br label %while.end

if.else:                                          ; preds = %while.body
  %call5 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %i)
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %while.cond, !llvm.loop !131

while.end:                                        ; preds = %if.then, %while.cond
  %2 = load i8, ptr %fResult, align 1, !range !101
  %tobool = trunc i8 %2 to i1
  call void @llvm.lifetime.end.p0(i64 8, ptr %i) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %theEnd) #6
  call void @llvm.lifetime.end.p0(i64 1, ptr %fResult) #6
  ret i1 %tobool
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #6

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr %0, metadata %1) #7

; Function Attrs: inaccessiblememonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef %0) #8

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) #9 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #6
  call void @_ZSt9terminatev() #17
  unreachable
}

; Function Attrs: nofree
declare !intel.dtrans.func.type !132 dso_local "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2" %0) #10

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() #11

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !133 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %pos = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp4 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %freeNode = alloca ptr, align 8, !intel_dtrans_type !34
  %nextNode = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_listHead = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %0 = load ptr, ptr %m_listHead, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %pos) #6
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %pos, ptr noundef nonnull align 8 dereferenceable(24) %this1)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %if.then
  br label %while.cond

while.cond:                                       ; preds = %invoke.cont8, %invoke.cont
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
          to label %invoke.cont2 unwind label %terminate.lpad

invoke.cont2:                                     ; preds = %while.cond
  %call = invoke noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %pos, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %invoke.cont2
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %invoke.cont3
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp4) #6
  invoke void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp4, ptr noundef nonnull align 8 dereferenceable(8) %pos, i32 noundef 0)
          to label %invoke.cont5 unwind label %terminate.lpad

invoke.cont5:                                     ; preds = %while.body
  %call7 = invoke noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp4)
          to label %invoke.cont6 unwind label %terminate.lpad

invoke.cont6:                                     ; preds = %invoke.cont5
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11destroyNodeERNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(24) %call7)
          to label %invoke.cont8 unwind label %terminate.lpad

invoke.cont8:                                     ; preds = %invoke.cont6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp4) #6
  br label %while.cond, !llvm.loop !134

while.end:                                        ; preds = %invoke.cont3
  call void @llvm.lifetime.start.p0(i64 8, ptr %freeNode) #6
  %m_freeListHeadPtr = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %1 = load ptr, ptr %m_freeListHeadPtr, align 8
  store ptr %1, ptr %freeNode, align 8
  br label %while.cond9

while.cond9:                                      ; preds = %invoke.cont12, %while.end
  %2 = load ptr, ptr %freeNode, align 8
  %cmp10 = icmp ne ptr %2, null
  br i1 %cmp10, label %while.body11, label %while.end13

while.body11:                                     ; preds = %while.cond9
  call void @llvm.lifetime.start.p0(i64 8, ptr %nextNode) #6
  %3 = load ptr, ptr %freeNode, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %3, i32 0, i32 2
  %4 = load ptr, ptr %next, align 8
  store ptr %4, ptr %nextNode, align 8
  %5 = load ptr, ptr %freeNode, align 8
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %5)
          to label %invoke.cont12 unwind label %terminate.lpad

invoke.cont12:                                    ; preds = %while.body11
  %6 = load ptr, ptr %nextNode, align 8
  store ptr %6, ptr %freeNode, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr %nextNode) #6
  br label %while.cond9, !llvm.loop !135

while.end13:                                      ; preds = %while.cond9
  %m_listHead14 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %7 = load ptr, ptr %m_listHead14, align 8
  invoke void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %7)
          to label %invoke.cont15 unwind label %terminate.lpad

invoke.cont15:                                    ; preds = %while.end13
  call void @llvm.lifetime.end.p0(i64 8, ptr %freeNode) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %pos) #6
  br label %if.end

if.end:                                           ; preds = %invoke.cont15, %entry
  ret void

terminate.lpad:                                   ; preds = %while.end13, %while.body11, %invoke.cont6, %invoke.cont5, %while.body, %invoke.cont2, %while.cond, %if.then
  %8 = landingpad { ptr, i32 }
          catch ptr null
  %9 = extractvalue { ptr, i32 } %8, 0
  call void @__clang_call_terminate(ptr %9) #17
  unreachable
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !136 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call, i32 0, i32 2
  %0 = load ptr, ptr %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(24) %0)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) #2 comdat align 2 !intel.dtrans.func.type !138 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theRhs.addr, align 8
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !139 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11destroyNodeERNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %node) #2 comdat align 2 !intel.dtrans.func.type !140 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %node.addr = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %node, ptr %node.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %node.addr, align 8
  %1 = load ptr, ptr %node.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this, i32 noundef %0) #2 comdat align 2 !intel.dtrans.func.type !141 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  %.addr = alloca i32, align 4
  %origNode = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  store i32 %0, ptr %.addr, align 4
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %origNode) #6
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %1 = load ptr, ptr %currentNode, align 8
  store ptr %1, ptr %origNode, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %2 = load ptr, ptr %currentNode2, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %2, i32 0, i32 2
  %3 = load ptr, ptr %next, align 8
  %currentNode3 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  store ptr %3, ptr %currentNode3, align 8
  %4 = load ptr, ptr %origNode, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(24) %4)
  call void @llvm.lifetime.end.p0(i64 8, ptr %origNode) #6
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !142 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  ret ptr %0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10deallocateEPNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %pointer) #2 comdat align 2 !intel.dtrans.func.type !143 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %pointer.addr = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %pointer, ptr %pointer.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %m_memoryManager, align 8
  %1 = load ptr, ptr %pointer.addr, align 8
  %vtable = load ptr, ptr %0, align 8
  %2 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %3 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %3)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %4 = load ptr, ptr %vfn, align 8
  call void %4(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef %1), !intel_dtrans_type !144
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !145 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_listHead = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %0 = load ptr, ptr %m_listHead, align 8
  %cmp = icmp eq ptr null, %0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call noundef ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %this1, i64 noundef 1)
  %m_listHead2 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  store ptr %call, ptr %m_listHead2, align 8
  %m_listHead3 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %1 = load ptr, ptr %m_listHead3, align 8
  %m_listHead4 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %2 = load ptr, ptr %m_listHead4, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %2, i32 0, i32 2
  store ptr %1, ptr %next, align 8
  %m_listHead5 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %3 = load ptr, ptr %m_listHead5, align 8
  %m_listHead6 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %4 = load ptr, ptr %m_listHead6, align 8
  %prev = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %4, i32 0, i32 1
  store ptr %3, ptr %prev, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %m_listHead7 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 1
  %5 = load ptr, ptr %m_listHead7, align 8
  ret ptr %5
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %node) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !146 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  %node.addr = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %node, ptr %node.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %node.addr, align 8
  store ptr %0, ptr %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this, i64 noundef %size) #2 comdat align 2 !intel.dtrans.func.type !147 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %size.addr = alloca i64, align 8
  %theBytesNeeded = alloca i64, align 8
  %pointer = alloca ptr, align 8, !intel_dtrans_type !12
  store ptr %this, ptr %this.addr, align 8
  store i64 %size, ptr %size.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %theBytesNeeded) #6
  %0 = load i64, ptr %size.addr, align 8
  %mul = mul i64 %0, 24
  store i64 %mul, ptr %theBytesNeeded, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %pointer) #6
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 0
  %1 = load ptr, ptr %m_memoryManager, align 8
  %2 = load i64, ptr %theBytesNeeded, align 8
  %vtable = load ptr, ptr %1, align 8
  %3 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %4 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %5 = load ptr, ptr %vfn, align 8
  %call = call noundef ptr %5(ptr noundef nonnull align 8 dereferenceable(8) %1, i64 noundef %2), !intel_dtrans_type !148
  store ptr %call, ptr %pointer, align 8
  %6 = load ptr, ptr %pointer, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr %pointer) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %theBytesNeeded) #6
  ret ptr %6
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) #12 comdat align 2 !intel.dtrans.func.type !149 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %1 = load ptr, ptr %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %1, i32 0, i32 0
  %2 = load ptr, ptr %currentNode2, align 8
  %cmp = icmp eq ptr %0, %2
  ret i1 %cmp
}

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !150 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1" %0) #13

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) #2 comdat align 2 !intel.dtrans.func.type !151 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp2 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %call = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %ref.tmp, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp2) #6
  %call3 = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive4 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %ref.tmp2, i32 0, i32 0
  store ptr %call3, ptr %coerce.dive4, align 8
  %call5 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  %conv = zext i1 %call5 to i32
  %cmp = icmp ne i32 %conv, 0
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp2) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  ret i1 %cmp
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4backEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !152 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp2 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp2) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp2, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp2) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  ret ptr %call
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) #12 comdat align 2 !intel.dtrans.func.type !154 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %0 = load i16, ptr %m_objectCount, align 8
  %conv = zext i16 %0 to i32
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %conv2 = zext i16 %1 to i32
  %cmp = icmp slt i32 %conv, %conv2
  %2 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9push_backERKS4_(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %data) #2 comdat align 2 !intel.dtrans.func.type !156 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %data.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %data, ptr %data.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %data.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE6createERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize) #2 comdat align 2 !intel.dtrans.func.type !157 {
entry:
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theBlockSize.addr = alloca i16, align 2
  %theInstance = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  call void @llvm.lifetime.start.p0(i64 8, ptr %theInstance) #6
  %0 = load ptr, ptr %theManager.addr, align 8
  %1 = load ptr, ptr %theManager.addr, align 8
  %call = call noundef ptr @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %theInstance, ptr noundef nonnull align 8 dereferenceable(8) %1, ptr noundef nonnull align 2 dereferenceable(2) %theBlockSize.addr)
  call void @llvm.lifetime.end.p0(i64 8, ptr %theInstance) #6
  ret ptr %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !158 {
entry:
  %retval = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theResult = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %0 = load i16, ptr %m_objectCount, align 8
  %conv = zext i16 %0 to i32
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %conv2 = zext i16 %1 to i32
  %cmp = icmp eq i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr null, ptr %retval, align 8
  br label %return

if.else:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %theResult) #6
  store ptr null, ptr %theResult, align 8
  %m_firstFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %2 = load i16, ptr %m_firstFreeBlock, align 8
  %conv3 = zext i16 %2 to i32
  %m_nextFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  %3 = load i16, ptr %m_nextFreeBlock, align 2
  %conv4 = zext i16 %3 to i32
  %cmp5 = icmp ne i32 %conv3, %conv4
  br i1 %cmp5, label %if.then6, label %if.else9

if.then6:                                         ; preds = %if.else
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %4 = load ptr, ptr %m_objectBlock, align 8
  %m_firstFreeBlock7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %5 = load i16, ptr %m_firstFreeBlock7, align 8
  %conv8 = zext i16 %5 to i32
  %idx.ext = sext i32 %conv8 to i64
  %add.ptr = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %4, i64 %idx.ext
  store ptr %add.ptr, ptr %theResult, align 8
  br label %if.end

if.else9:                                         ; preds = %if.else
  %m_objectBlock10 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %6 = load ptr, ptr %m_objectBlock10, align 8
  %m_firstFreeBlock11 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %7 = load i16, ptr %m_firstFreeBlock11, align 8
  %conv12 = zext i16 %7 to i32
  %idx.ext13 = sext i32 %conv12 to i64
  %add.ptr14 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %6, i64 %idx.ext13
  store ptr %add.ptr14, ptr %theResult, align 8
  %8 = load ptr, ptr %theResult, align 8
  %call = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(ptr noundef %8)
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %call, i32 0, i32 0
  %9 = load i16, ptr %next, align 4
  %m_nextFreeBlock15 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  store i16 %9, ptr %m_nextFreeBlock15, align 2
  %m_objectCount16 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %10 = load i16, ptr %m_objectCount16, align 8
  %inc = add i16 %10, 1
  store i16 %inc, ptr %m_objectCount16, align 8
  br label %if.end

if.end:                                           ; preds = %if.else9, %if.then6
  %11 = load ptr, ptr %theResult, align 8
  store ptr %11, ptr %retval, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr %theResult) #6
  br label %return

return:                                           ; preds = %if.end, %if.then
  %12 = load ptr, ptr %retval, align 8
  ret ptr %12
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !159 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call, i32 0, i32 2
  %0 = load ptr, ptr %next, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) %retval, ptr noundef nonnull align 8 dereferenceable(24) %0)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %retval, i32 0, i32 0
  %1 = load ptr, ptr %coerce.dive, align 8
  ret ptr %1
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) #12 comdat align 2 !intel.dtrans.func.type !160 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !161
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %1 = load ptr, ptr %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %1, i32 0, i32 0
  %2 = load ptr, ptr %currentNode2, align 8
  %cmp = icmp eq ptr %0, %2
  ret i1 %cmp
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !162 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) %retval, ptr noundef nonnull align 8 dereferenceable(24) %call)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %retval, i32 0, i32 0
  %0 = load ptr, ptr %coerce.dive, align 8
  ret ptr %0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !163 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE11getListHeadEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  ret ptr %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERS9_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %node) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !164 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  %node.addr = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %node, ptr %node.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %node.addr, align 8
  store ptr %0, ptr %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !165 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %prev = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 1
  %1 = load ptr, ptr %prev, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  store ptr %1, ptr %currentNode2, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(8) %this1)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !166 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %value = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 0
  ret ptr %value
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !167 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %currentNode2, align 8
  store ptr %1, ptr %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %data, ptr noundef "intel_dtrans_func_index"="4" %pos) #2 comdat align 2 !intel.dtrans.func.type !168 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %data.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %newNode = alloca ptr, align 8, !intel_dtrans_type !34
  %nextFreeNode = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %data, ptr %data.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %newNode) #6
  store ptr null, ptr %newNode, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %nextFreeNode) #6
  store ptr null, ptr %nextFreeNode, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %0 = load ptr, ptr %m_freeListHeadPtr, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %m_freeListHeadPtr2 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %1 = load ptr, ptr %m_freeListHeadPtr2, align 8
  store ptr %1, ptr %newNode, align 8
  %m_freeListHeadPtr3 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %2 = load ptr, ptr %m_freeListHeadPtr3, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %2, i32 0, i32 2
  %3 = load ptr, ptr %next, align 8
  store ptr %3, ptr %nextFreeNode, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %call = call noundef ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %this1, i64 noundef 1)
  %m_freeListHeadPtr4 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  store ptr %call, ptr %m_freeListHeadPtr4, align 8
  %m_freeListHeadPtr5 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %4 = load ptr, ptr %m_freeListHeadPtr5, align 8
  store ptr %4, ptr %newNode, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %5 = load ptr, ptr %newNode, align 8
  %value = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %5, i32 0, i32 0
  %6 = load ptr, ptr %data.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 0
  %7 = load ptr, ptr %m_memoryManager, align 8
  %call6 = call noundef ptr @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(ptr noundef %value, ptr noundef nonnull align 8 dereferenceable(8) %6, ptr noundef nonnull align 8 dereferenceable(8) %7)
  %8 = load ptr, ptr %newNode, align 8
  %prev = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %8, i32 0, i32 1
  %call7 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %pos)
  %prev8 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call7, i32 0, i32 1
  %9 = load ptr, ptr %prev8, align 8
  store ptr %9, ptr %prev, align 8
  %10 = load ptr, ptr %newNode, align 8
  %next9 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %10, i32 0, i32 2
  %call10 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %pos)
  store ptr %call10, ptr %next9, align 8
  %11 = load ptr, ptr %newNode, align 8
  %call11 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %pos)
  %prev12 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call11, i32 0, i32 1
  %12 = load ptr, ptr %prev12, align 8
  %next13 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %12, i32 0, i32 2
  store ptr %11, ptr %next13, align 8
  %13 = load ptr, ptr %newNode, align 8
  %call14 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %pos)
  %prev15 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %call14, i32 0, i32 1
  store ptr %13, ptr %prev15, align 8
  %14 = load ptr, ptr %nextFreeNode, align 8
  %m_freeListHeadPtr16 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  store ptr %14, ptr %m_freeListHeadPtr16, align 8
  %15 = load ptr, ptr %newNode, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr %nextFreeNode) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %newNode) #6
  ret ptr %15
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1028ConstructWithNoMemoryManagerIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9constructEPS4_RKS4_RN11xercesc_2_713MemoryManagerE(ptr noundef "intel_dtrans_func_index"="2" %address, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theRhs, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="4" %0) #12 comdat align 2 !intel.dtrans.func.type !169 {
entry:
  %address.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %address, ptr %address.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %address.addr, align 8
  %2 = load ptr, ptr %theRhs.addr, align 8
  %3 = load ptr, ptr %2, align 8
  store ptr %3, ptr %1, align 8
  ret ptr %1
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014XalanConstructINS_18ReusableArenaBlockINS_13XStringCachedEtEEN11xercesc_2_713MemoryManagerEtEEPT_RS5_RS7_RT0_RKT1_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theMemoryManager, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theInstance, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="4" %theParam1, ptr noundef nonnull align 2 dereferenceable(2) "intel_dtrans_func_index"="5" %theParam2) #2 comdat personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !170 {
entry:
  %theMemoryManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theInstance.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %theParam1.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theParam2.addr = alloca ptr, align 8, !intel_dtrans_type !95
  %theGuard = alloca %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", align 8
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %theMemoryManager, ptr %theMemoryManager.addr, align 8
  store ptr %theInstance, ptr %theInstance.addr, align 8
  store ptr %theParam1, ptr %theParam1.addr, align 8
  store ptr %theParam2, ptr %theParam2.addr, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr %theGuard) #6
  %0 = load ptr, ptr %theMemoryManager.addr, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(ptr noundef nonnull align 8 dereferenceable(16) %theGuard, ptr noundef nonnull align 8 dereferenceable(8) %0, i64 noundef 32)
  %call = invoke noundef ptr @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(ptr noundef nonnull align 8 dereferenceable(16) %theGuard)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %1 = load ptr, ptr %theParam1.addr, align 8
  %2 = load ptr, ptr %theParam2.addr, align 8
  %3 = load i16, ptr %2, align 2
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(28) %call, ptr noundef nonnull align 8 dereferenceable(8) %1, i16 noundef zeroext %3)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %4 = load ptr, ptr %theInstance.addr, align 8
  store ptr %call, ptr %4, align 8
  invoke void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) %theGuard)
          to label %invoke.cont2 unwind label %lpad

invoke.cont2:                                     ; preds = %invoke.cont1
  %5 = load ptr, ptr %theInstance.addr, align 8
  %6 = load ptr, ptr %5, align 8
  call void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(ptr noundef nonnull align 8 dereferenceable(16) %theGuard) #6
  call void @llvm.lifetime.end.p0(i64 16, ptr %theGuard) #6
  ret ptr %6

lpad:                                             ; preds = %invoke.cont1, %invoke.cont, %entry
  %7 = landingpad { ptr, i32 }
          cleanup
  %8 = extractvalue { ptr, i32 } %7, 0
  store ptr %8, ptr %exn.slot, align 8
  %9 = extractvalue { ptr, i32 } %7, 1
  store i32 %9, ptr %ehselector.slot, align 4
  call void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(ptr noundef nonnull align 8 dereferenceable(16) %theGuard) #6
  call void @llvm.lifetime.end.p0(i64 16, ptr %theGuard) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val3 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val3
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardC2ERN11xercesc_2_713MemoryManagerEm(ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theMemoryManager, i64 noundef %theSize) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !171 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !172
  %theMemoryManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theSize.addr = alloca i64, align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %theMemoryManager, ptr %theMemoryManager.addr, align 8
  store i64 %theSize, ptr %theSize.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theMemoryManager.addr, align 8
  store ptr %0, ptr %m_memoryManager, align 8
  %m_pointer = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 1
  %1 = load ptr, ptr %theMemoryManager.addr, align 8
  %2 = load i64, ptr %theSize.addr, align 8
  %vtable = load ptr, ptr %1, align 8
  %3 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %4 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %5 = load ptr, ptr %vfn, align 8
  %call = call noundef ptr %5(ptr noundef nonnull align 8 dereferenceable(8) %1, i64 noundef %2), !intel_dtrans_type !148
  store ptr %call, ptr %m_pointer, align 8
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_1020XalanAllocationGuard3getEv(ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !173 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !172
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 1
  %0 = load ptr, ptr %m_pointer, align 8
  ret ptr %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !174 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theBlockSize.addr = alloca i16, align 2
  %agg.tmp.ensured = alloca %"struct._ZTSN11xalanc_1_1024XalanCompileErrorBooleanILb1EEE.xalanc_1_10::XalanCompileErrorBoolean", align 1
  %i = alloca i16, align 2
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theManager.addr, align 8
  %1 = load i16, ptr %theBlockSize.addr, align 2
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0, i16 noundef zeroext %1)
  %m_firstFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  store i16 0, ptr %m_firstFreeBlock, align 8
  %m_nextFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  store i16 0, ptr %m_nextFreeBlock, align 2
  call void @llvm.memset.p0.i64(ptr align 1 %agg.tmp.ensured, i8 0, i64 1, i1 false)
  call void @llvm.lifetime.start.p0(i64 2, ptr %i) #6
  store i16 0, ptr %i, align 2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i16, ptr %i, align 2
  %conv = zext i16 %2 to i32
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %3 = load i16, ptr %m_blockSize, align 2
  %conv2 = zext i16 %3 to i32
  %cmp = icmp slt i32 %conv, %conv2
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 2, ptr %i) #6
  br label %for.end

for.body:                                         ; preds = %for.cond
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %4 = load ptr, ptr %m_objectBlock, align 8
  %5 = load i16, ptr %i, align 2
  %idxprom = zext i16 %5 to i64
  %arrayidx = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %4, i64 %idxprom
  %6 = load i16, ptr %i, align 2
  %conv3 = zext i16 %6 to i32
  %add = add nsw i32 %conv3, 1
  %conv4 = trunc i32 %add to i16
  invoke void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(ptr noundef nonnull align 4 dereferenceable(8) %arrayidx, i16 noundef zeroext %conv4)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %invoke.cont
  %7 = load i16, ptr %i, align 2
  %inc = add i16 %7, 1
  store i16 %inc, ptr %i, align 2
  br label %for.cond, !llvm.loop !175

lpad:                                             ; preds = %for.body
  %8 = landingpad { ptr, i32 }
          cleanup
  %9 = extractvalue { ptr, i32 } %8, 0
  store ptr %9, ptr %exn.slot, align 8
  %10 = extractvalue { ptr, i32 } %8, 1
  store i32 %10, ptr %ehselector.slot, align 4
  call void @llvm.lifetime.end.p0(i64 2, ptr %i) #6
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %this1) #6
  br label %eh.resume

for.end:                                          ; preds = %for.cond.cleanup
  ret void

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val5 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val5
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuard7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %this) #12 comdat align 2 !intel.dtrans.func.type !176 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !172
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 1
  store ptr null, ptr %m_pointer, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1020XalanAllocationGuardD2Ev(ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !177 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !172
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_pointer = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 1
  %0 = load ptr, ptr %m_pointer, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 0
  %1 = load ptr, ptr %m_memoryManager, align 8
  %m_pointer2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard", ptr %this1, i32 0, i32 1
  %2 = load ptr, ptr %m_pointer2, align 8
  %vtable = load ptr, ptr %1, align 8
  %3 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %4 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %5 = load ptr, ptr %vfn, align 8
  invoke void %5(ptr noundef nonnull align 8 dereferenceable(8) %1, ptr noundef %2)
          to label %invoke.cont unwind label %terminate.lpad, !intel_dtrans_type !144

invoke.cont:                                      ; preds = %whpr.continue
  br label %if.end

if.end:                                           ; preds = %invoke.cont, %entry
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %6 = landingpad { ptr, i32 }
          catch ptr null
  %7 = extractvalue { ptr, i32 } %6, 0
  call void @__clang_call_terminate(ptr %7) #17
  unreachable
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEC2ERN11xercesc_2_713MemoryManagerEt(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager, i16 noundef zeroext %theBlockSize) unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !178 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  %theBlockSize.addr = alloca i16, align 2
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  store i16 %theBlockSize, ptr %theBlockSize.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theManager.addr, align 8
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(8) %m_allocator, ptr noundef nonnull align 8 dereferenceable(8) %0)
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  store i16 0, ptr %m_objectCount, align 8
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %theBlockSize.addr, align 2
  store i16 %1, ptr %m_blockSize, align 2
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %m_allocator2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 0
  %m_blockSize3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %2 = load i16, ptr %m_blockSize3, align 2
  %conv = zext i16 %2 to i64
  %call = invoke noundef ptr @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(ptr noundef nonnull align 8 dereferenceable(8) %m_allocator2, i64 noundef %conv, ptr noundef null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  store ptr %call, ptr %m_objectBlock, align 8
  ret void

lpad:                                             ; preds = %entry
  %3 = landingpad { ptr, i32 }
          cleanup
  %4 = extractvalue { ptr, i32 } %3, 0
  store ptr %4, ptr %exn.slot, align 8
  %5 = extractvalue { ptr, i32 } %3, 1
  store i32 %5, ptr %ehselector.slot, align 4
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(ptr noundef nonnull align 8 dereferenceable(8) %m_allocator) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val4 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val4
}

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #14

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(ptr noundef nonnull align 4 dereferenceable(8) "intel_dtrans_func_index"="1" %this, i16 noundef zeroext %_next) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !179 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !180
  %_next.addr = alloca i16, align 2
  store ptr %this, ptr %this.addr, align 8
  store i16 %_next, ptr %_next.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %this1, i32 0, i32 0
  %0 = load i16, ptr %_next.addr, align 2
  store i16 %0, ptr %next, align 4
  %verificationStamp = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %this1, i32 0, i32 1
  store i32 -2228259, ptr %verificationStamp, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !181 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_allocator = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 0
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %0 = load ptr, ptr %m_objectBlock, align 8
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %conv = zext i16 %1 to i64
  invoke void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE10deallocateEPS1_m(ptr noundef nonnull align 8 dereferenceable(8) %m_allocator, ptr noundef %0, i64 noundef %conv)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  %m_allocator2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(ptr noundef nonnull align 8 dereferenceable(8) %m_allocator2) #6
  ret void

terminate.lpad:                                   ; preds = %entry
  %2 = landingpad { ptr, i32 }
          catch ptr null
  %3 = extractvalue { ptr, i32 } %2, 0
  call void @__clang_call_terminate(ptr %3) #17
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !182 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !183
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theManager.addr, align 8
  store ptr %0, ptr %m_memoryManager, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE8allocateEmPKv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this, i64 noundef %size, ptr noundef "intel_dtrans_func_index"="3" %0) #2 comdat align 2 !intel.dtrans.func.type !184 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !183
  %size.addr = alloca i64, align 8
  %.addr = alloca ptr, align 8, !intel_dtrans_type !12
  store ptr %this, ptr %this.addr, align 8
  store i64 %size, ptr %size.addr, align 8
  store ptr %0, ptr %.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", ptr %this1, i32 0, i32 0
  %1 = load ptr, ptr %m_memoryManager, align 8
  %2 = load i64, ptr %size.addr, align 8
  %mul = mul i64 %2, 80
  %vtable = load ptr, ptr %1, align 8
  %3 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %4 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %5 = load ptr, ptr %vfn, align 8
  %call = call noundef ptr %5(ptr noundef nonnull align 8 dereferenceable(8) %1, i64 noundef %mul), !intel_dtrans_type !148
  ret ptr %call
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEED2Ev(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 !intel.dtrans.func.type !185 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !183
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEE10deallocateEPS1_m(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %p, i64 noundef %0) #2 comdat align 2 !intel.dtrans.func.type !186 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !183
  %p.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %.addr = alloca i64, align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %p, ptr %p.addr, align 8
  store i64 %0, ptr %.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %1 = load ptr, ptr %p.addr, align 8
  %cmp = icmp eq ptr %1, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", ptr %this1, i32 0, i32 0
  %2 = load ptr, ptr %m_memoryManager, align 8
  %3 = load ptr, ptr %p.addr, align 8
  %vtable = load ptr, ptr %2, align 8
  %4 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.end
  %5 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.end
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %6 = load ptr, ptr %vfn, align 8
  call void %6(ptr noundef nonnull align 8 dereferenceable(8) %2, ptr noundef %3), !intel_dtrans_type !144
  br label %return

return:                                           ; preds = %whpr.continue, %if.then
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(ptr noundef "intel_dtrans_func_index"="2" %thePointer) #12 comdat align 2 !intel.dtrans.func.type !187 {
entry:
  %thePointer.addr = alloca ptr, align 8, !intel_dtrans_type !12
  store ptr %thePointer, ptr %thePointer.addr, align 8
  %0 = load ptr, ptr %thePointer.addr, align 8
  ret ptr %0
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %0) #12 comdat align 2 !intel.dtrans.func.type !188 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %0, ptr %.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_nextFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_nextFreeBlock, align 2
  %m_firstFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  store i16 %1, ptr %m_firstFreeBlock, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(ptr noalias sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !189 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %agg.tmp, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  %coerce.dive2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %agg.tmp, i32 0, i32 0
  %0 = load ptr, ptr %coerce.dive2, align 8
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr %0)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr noalias sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !191 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call ptr @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %agg.tmp, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  %coerce.dive2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %agg.tmp, i32 0, i32 0
  %0 = load ptr, ptr %coerce.dive2, align 8
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr %0)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %__x, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %__y) #15 comdat !intel.dtrans.func.type !192 {
entry:
  %__x.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %__y.addr = alloca ptr, align 8, !intel_dtrans_type !190
  store ptr %__x, ptr %__x.addr, align 8
  store ptr %__y, ptr %__y.addr, align 8
  %0 = load ptr, ptr %__x.addr, align 8
  %1 = load ptr, ptr %__y.addr, align 8
  %call = call noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %1)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !193 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %__tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %__tmp) #6
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %__tmp, ptr align 8 %current, i64 8, i1 false)
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %call = call ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr noundef nonnull align 8 dereferenceable(8) %__tmp)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %ref.tmp, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  %call2 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %__tmp) #6
  ret ptr %call2
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE10ownsObjectEPKS1_(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) #2 comdat align 2 !intel.dtrans.func.type !194 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theObject.addr, align 8
  %call = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPKv(ptr noundef %0)
  %call2 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(ptr noundef nonnull align 8 dereferenceable(28) %this1, ptr noundef %call)
  ret i1 %call2
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !195 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %coerce = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  %call = call ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr noundef nonnull align 8 dereferenceable(8) %current)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %coerce, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  ret ptr %this1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %__x.coerce) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !196 {
entry:
  %__x = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %__x, i32 0, i32 0
  store ptr %__x.coerce, ptr %coerce.dive, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %current, ptr align 8 %__x, i64 8, i1 false)
  ret void
}

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly %0, ptr noalias nocapture readonly %1, i64 %2, i1 immarg %3) #16

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %__x, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %__y) #15 comdat !intel.dtrans.func.type !197 {
entry:
  %__x.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %__y.addr = alloca ptr, align 8, !intel_dtrans_type !190
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp1 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %__x, ptr %__x.addr, align 8
  store ptr %__y, ptr %__y.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %0 = load ptr, ptr %__x.addr, align 8
  %call = call ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) %0)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %ref.tmp, i32 0, i32 0
  store ptr %call, ptr %coerce.dive, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp1) #6
  %1 = load ptr, ptr %__y.addr, align 8
  %call2 = call ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) %1)
  %coerce.dive3 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %ref.tmp1, i32 0, i32 0
  store ptr %call2, ptr %coerce.dive3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp1)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp1) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  ret i1 %call4
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !198 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !190
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %retval, ptr align 8 %current, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %retval, i32 0, i32 0
  %0 = load ptr, ptr %coerce.dive, align 8
  ret ptr %0
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !199 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %prev = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 1
  %1 = load ptr, ptr %prev, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  store ptr %1, ptr %currentNode2, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %retval, ptr align 8 %this1, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %retval, i32 0, i32 0
  %2 = load ptr, ptr %coerce.dive, align 8
  ret ptr %2
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !200 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %value = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 0
  ret ptr %value
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %block) #2 comdat align 2 !intel.dtrans.func.type !201 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %block.addr = alloca ptr, align 8, !intel_dtrans_type !180
  store ptr %this, ptr %this.addr, align 8
  store ptr %block, ptr %block.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %block.addr, align 8
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %0)
  br i1 %call, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %1 = load ptr, ptr %block.addr, align 8
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %2 = load i16, ptr %m_blockSize, align 2
  %call2 = call noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock10isValidForEt(ptr noundef nonnull align 4 dereferenceable(8) %1, i16 noundef zeroext %2)
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %3 = phi i1 [ false, %entry ], [ %call2, %land.rhs ]
  %lnot = xor i1 %3, true
  ret i1 %lnot
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPKv(ptr noundef "intel_dtrans_func_index"="2" %thePointer) #12 comdat align 2 !intel.dtrans.func.type !202 {
entry:
  %thePointer.addr = alloca ptr, align 8, !intel_dtrans_type !12
  store ptr %thePointer, ptr %thePointer.addr, align 8
  %0 = load ptr, ptr %thePointer.addr, align 8
  ret ptr %0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE9ownsBlockEPKS1_(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) #2 comdat align 2 !intel.dtrans.func.type !203 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theObject.addr, align 8
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %0, i16 noundef zeroext %1)
  ret i1 %call
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock10isValidForEt(ptr noundef nonnull align 4 dereferenceable(8) "intel_dtrans_func_index"="1" %this, i16 noundef zeroext %rightBorder) #12 comdat align 2 !intel.dtrans.func.type !204 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !180
  %rightBorder.addr = alloca i16, align 2
  store ptr %this, ptr %this.addr, align 8
  store i16 %rightBorder, ptr %rightBorder.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %verificationStamp = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %this1, i32 0, i32 1
  %0 = load i32, ptr %verificationStamp, align 4
  %cmp = icmp eq i32 %0, -2228259
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %this1, i32 0, i32 0
  %1 = load i16, ptr %next, align 4
  %conv = zext i16 %1 to i32
  %2 = load i16, ptr %rightBorder.addr, align 2
  %conv2 = zext i16 %2 to i32
  %cmp3 = icmp sle i32 %conv, %conv2
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %3 = phi i1 [ false, %entry ], [ %cmp3, %land.rhs ]
  %4 = zext i1 %3 to i64
  %cond = select i1 %3, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE11isInBordersEPKS1_t(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject, i16 noundef zeroext %rightBoundary) #12 comdat align 2 !intel.dtrans.func.type !205 {
entry:
  %retval = alloca i1, align 1
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %rightBoundary.addr = alloca i16, align 2
  %functor = alloca %"struct._ZTSSt4lessIPKN11xalanc_1_1013XStringCachedEE.std::less", align 1
  %cleanup.dest.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  store i16 %rightBoundary, ptr %rightBoundary.addr, align 2
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i16, ptr %rightBoundary.addr, align 2
  %conv = zext i16 %0 to i32
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %conv2 = zext i16 %1 to i32
  %cmp = icmp sgt i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %m_blockSize3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %2 = load i16, ptr %m_blockSize3, align 2
  store i16 %2, ptr %rightBoundary.addr, align 2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @llvm.lifetime.start.p0(i64 1, ptr %functor) #6
  %3 = load ptr, ptr %theObject.addr, align 8
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %4 = load ptr, ptr %m_objectBlock, align 8
  %call = call noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr noundef nonnull align 1 dereferenceable(1) %functor, ptr noundef %3, ptr noundef %4) #6
  %conv4 = zext i1 %call to i32
  %cmp5 = icmp eq i32 %conv4, 0
  br i1 %cmp5, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %if.end
  %5 = load ptr, ptr %theObject.addr, align 8
  %m_objectBlock6 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %6 = load ptr, ptr %m_objectBlock6, align 8
  %7 = load i16, ptr %rightBoundary.addr, align 2
  %conv7 = zext i16 %7 to i32
  %idx.ext = sext i32 %conv7 to i64
  %add.ptr = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %6, i64 %idx.ext
  %call8 = call noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr noundef nonnull align 1 dereferenceable(1) %functor, ptr noundef %5, ptr noundef %add.ptr) #6
  %conv9 = zext i1 %call8 to i32
  %cmp10 = icmp eq i32 %conv9, 1
  br i1 %cmp10, label %if.then11, label %if.else

if.then11:                                        ; preds = %land.lhs.true
  store i1 true, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

if.else:                                          ; preds = %land.lhs.true, %if.end
  store i1 false, ptr %retval, align 1
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then11
  call void @llvm.lifetime.end.p0(i64 1, ptr %functor) #6
  %8 = load i1, ptr %retval, align 1
  ret i1 %8
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNKSt4lessIPKN11xalanc_1_1013XStringCachedEEclES3_S3_(ptr noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %__x, ptr noundef "intel_dtrans_func_index"="3" %__y) #12 comdat align 2 !intel.dtrans.func.type !206 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !207
  %__x.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %__y.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %this, ptr %this.addr, align 8
  store ptr %__x, ptr %__x.addr, align 8
  store ptr %__y, ptr %__y.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %__x.addr, align 8
  %1 = ptrtoint ptr %0 to i64
  %2 = load ptr, ptr %__y.addr, align 8
  %3 = ptrtoint ptr %2 to i64
  %cmp = icmp ult i64 %1, %3
  ret i1 %cmp
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) #12 comdat align 2 !intel.dtrans.func.type !208 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !161
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theRhs.addr, align 8
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !209 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 2
  %1 = load ptr, ptr %next, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  store ptr %1, ptr %currentNode2, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %retval, ptr align 8 %this1, i64 8, i1 false)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %retval, i32 0, i32 0
  %2 = load ptr, ptr %coerce.dive, align 8
  ret ptr %2
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5frontEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !210 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  ret ptr %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE10push_frontERKS4_(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %data) #2 comdat align 2 !intel.dtrans.func.type !211 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %data.addr = alloca ptr, align 8, !intel_dtrans_type !153
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %data, ptr %data.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %data.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE13constructNodeERKS4_NS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #12 comdat align 2 !intel.dtrans.func.type !212 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %m_memoryManager, align 8
  ret ptr %0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) #2 comdat align 2 !intel.dtrans.func.type !213 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5eraseENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsIS4_EENS5_4NodeEEE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %pos) #2 comdat align 2 !intel.dtrans.func.type !214 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %pos)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(24) %call)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %node) #12 comdat align 2 !intel.dtrans.func.type !215 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %node.addr = alloca ptr, align 8, !intel_dtrans_type !34
  store ptr %this, ptr %this.addr, align 8
  store ptr %node, ptr %node.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %node.addr, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 2
  %1 = load ptr, ptr %next, align 8
  %2 = load ptr, ptr %node.addr, align 8
  %prev = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %2, i32 0, i32 1
  %3 = load ptr, ptr %prev, align 8
  %next2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %3, i32 0, i32 2
  store ptr %1, ptr %next2, align 8
  %4 = load ptr, ptr %node.addr, align 8
  %prev3 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %4, i32 0, i32 1
  %5 = load ptr, ptr %prev3, align 8
  %6 = load ptr, ptr %node.addr, align 8
  %next4 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %6, i32 0, i32 2
  %7 = load ptr, ptr %next4, align 8
  %prev5 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %7, i32 0, i32 1
  store ptr %5, ptr %prev5, align 8
  %8 = load ptr, ptr %node.addr, align 8
  %9 = load ptr, ptr %node.addr, align 8
  %prev6 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %9, i32 0, i32 1
  store ptr null, ptr %prev6, align 8
  %m_freeListHeadPtr = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  %10 = load ptr, ptr %m_freeListHeadPtr, align 8
  %11 = load ptr, ptr %node.addr, align 8
  %next7 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %11, i32 0, i32 2
  store ptr %10, ptr %next7, align 8
  %12 = load ptr, ptr %node.addr, align 8
  %m_freeListHeadPtr8 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %this1, i32 0, i32 2
  store ptr %12, ptr %m_freeListHeadPtr8, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theObject) #2 comdat align 2 !intel.dtrans.func.type !216 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theObject.addr = alloca ptr, align 8, !intel_dtrans_type !71
  %p = alloca ptr, align 8, !intel_dtrans_type !12
  store ptr %this, ptr %this.addr, align 8
  store ptr %theObject, ptr %theObject.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_firstFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %0 = load i16, ptr %m_firstFreeBlock, align 8
  %conv = zext i16 %0 to i32
  %m_nextFreeBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_nextFreeBlock, align 2
  %conv2 = zext i16 %1 to i32
  %cmp = icmp ne i32 %conv, %conv2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %p) #6
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %2 = load ptr, ptr %m_objectBlock, align 8
  %m_firstFreeBlock3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %3 = load i16, ptr %m_firstFreeBlock3, align 8
  %conv4 = zext i16 %3 to i32
  %idx.ext = sext i32 %conv4 to i64
  %add.ptr = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %2, i64 %idx.ext
  store ptr %add.ptr, ptr %p, align 8
  %4 = load ptr, ptr %p, align 8
  %m_nextFreeBlock5 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  %5 = load i16, ptr %m_nextFreeBlock5, align 2
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(ptr noundef nonnull align 4 dereferenceable(8) %4, i16 noundef zeroext %5)
  %m_firstFreeBlock6 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %6 = load i16, ptr %m_firstFreeBlock6, align 8
  %m_nextFreeBlock7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  store i16 %6, ptr %m_nextFreeBlock7, align 2
  call void @llvm.lifetime.end.p0(i64 8, ptr %p) #6
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load ptr, ptr %theObject.addr, align 8
  call void @_ZN11xalanc_1_1012XalanDestroyINS_13XStringCachedEEEvRT_(ptr noundef nonnull align 8 dereferenceable(80) %7)
  %8 = load ptr, ptr %theObject.addr, align 8
  %m_firstFreeBlock8 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  %9 = load i16, ptr %m_firstFreeBlock8, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockC2Et(ptr noundef nonnull align 4 dereferenceable(8) %8, i16 noundef zeroext %9)
  %10 = load ptr, ptr %theObject.addr, align 8
  %m_objectBlock9 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %11 = load ptr, ptr %m_objectBlock9, align 8
  %sub.ptr.lhs.cast = ptrtoint ptr %10 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %11 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 80
  %conv10 = trunc i64 %sub.ptr.div to i16
  %m_nextFreeBlock11 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 2
  store i16 %conv10, ptr %m_nextFreeBlock11, align 2
  %m_firstFreeBlock12 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %this1, i32 0, i32 1
  store i16 %conv10, ptr %m_firstFreeBlock12, align 8
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %12 = load i16, ptr %m_objectCount, align 8
  %dec = add i16 %12, -1
  store i16 %dec, ptr %m_objectCount, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE12destroyBlockEv(ptr noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %this) #2 comdat align 2 !intel.dtrans.func.type !217 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !100
  %iTerator = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %coerce = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp7 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp8 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_blocks = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5emptyEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks)
  %conv = zext i1 %call to i32
  %cmp = icmp eq i32 %conv, 0
  br i1 %cmp, label %if.then, label %if.end16

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %iTerator) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %m_blocks2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks2)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(ptr noundef nonnull align 8 dereferenceable(8) %iTerator, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  %call3 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %0 = load ptr, ptr %call3, align 8
  %call4 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE7isEmptyEv(ptr noundef nonnull align 8 dereferenceable(24) %0)
  br i1 %call4, label %if.then5, label %if.end15

if.then5:                                         ; preds = %if.then
  %call6 = call ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %coerce, i32 0, i32 0
  store ptr %call6, ptr %coerce.dive, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp7) #6
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp8) #6
  %m_blocks9 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp8, ptr noundef nonnull align 8 dereferenceable(24) %m_blocks9)
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp7, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp8)
  %call10 = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %iTerator, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp7)
  br i1 %call10, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %if.then5
  %call11 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %iTerator)
  %1 = load ptr, ptr %call11, align 8
  %call12 = call noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE14blockAvailableEv(ptr noundef nonnull align 8 dereferenceable(24) %1)
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %if.then5
  %2 = phi i1 [ true, %if.then5 ], [ %call12, %lor.rhs ]
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp8) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp7) #6
  br i1 %2, label %if.then13, label %if.end

if.then13:                                        ; preds = %lor.end
  %m_blocks14 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %this1, i32 0, i32 2
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE9pop_frontEv(ptr noundef nonnull align 8 dereferenceable(24) %m_blocks14)
  br label %if.end

if.end:                                           ; preds = %if.then13, %lor.end
  br label %if.end15

if.end15:                                         ; preds = %if.end, %if.then
  call void @llvm.lifetime.end.p0(i64 8, ptr %iTerator) #6
  br label %if.end16

if.end16:                                         ; preds = %if.end15, %entry
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !218 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %currentNode, align 8
  %next = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %0, i32 0, i32 2
  %1 = load ptr, ptr %next, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  store ptr %1, ptr %currentNode2, align 8
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(8) %this1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv(ptr noalias sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !219 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef %agg.tmp)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv(ptr noalias sret(%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !221 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %agg.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %agg.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  call void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef %agg.tmp)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZStneIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %__x, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %__y) #15 comdat !intel.dtrans.func.type !222 {
entry:
  %__x.addr = alloca ptr, align 8, !intel_dtrans_type !220
  %__y.addr = alloca ptr, align 8, !intel_dtrans_type !220
  store ptr %__x, ptr %__x.addr, align 8
  store ptr %__y, ptr %__y.addr, align 8
  %0 = load ptr, ptr %__x.addr, align 8
  %1 = load ptr, ptr %__y.addr, align 8
  %call = call noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %1)
  %lnot = xor i1 %call, true
  ret i1 %lnot
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !223 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !220
  %__tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %__tmp) #6
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %__tmp, ptr noundef nonnull align 8 dereferenceable(8) %current)
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %__tmp)
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %__tmp) #6
  ret ptr %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEppEv(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !224 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !220
  %tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @llvm.lifetime.start.p0(i64 8, ptr %tmp) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, ptr noundef nonnull align 8 dereferenceable(8) %current)
  call void @llvm.lifetime.end.p0(i64 8, ptr %tmp) #6
  ret ptr %this1
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1012XalanDestroyINS_13XStringCachedEEEvRT_(ptr noundef nonnull align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %theArg) #12 comdat !intel.dtrans.func.type !225 {
entry:
  %theArg.addr = alloca ptr, align 8, !intel_dtrans_type !71
  store ptr %theArg, ptr %theArg.addr, align 8
  %0 = load ptr, ptr %theArg.addr, align 8
  %vtable = load ptr, ptr %0, align 8
  %1 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %1, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %2 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  call void @llvm.assume(i1 %2)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 0
  %3 = load ptr, ptr %vfn, align 8
  call void %3(ptr noundef nonnull align 8 dereferenceable(80) %0) #6, !intel_dtrans_type !226
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKNS0_INS_23XalanListIteratorTraitsIS5_EES9_EE(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theRhs) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !227 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !161
  %theRhs.addr = alloca ptr, align 8, !intel_dtrans_type !137
  store ptr %this, ptr %this.addr, align 8
  store ptr %theRhs, ptr %theRhs.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %currentNode = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theRhs.addr, align 8
  %currentNode2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %currentNode2, align 8
  store ptr %1, ptr %currentNode, align 8
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtE7isEmptyEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) #12 comdat align 2 !intel.dtrans.func.type !228 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !155
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %0 = load i16, ptr %m_objectCount, align 8
  %conv = zext i16 %0 to i32
  %cmp = icmp eq i32 %conv, 0
  %1 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i1 true, i1 false
  ret i1 %cond
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZNSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEC2ESB_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %__x) unnamed_addr #0 comdat align 2 !intel.dtrans.func.type !229 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !220
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %current, ptr noundef nonnull align 8 dereferenceable(8) %__x)
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZSteqIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEEbRKSt16reverse_iteratorIT_ESG_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %__x, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %__y) #15 comdat !intel.dtrans.func.type !230 {
entry:
  %__x.addr = alloca ptr, align 8, !intel_dtrans_type !220
  %__y.addr = alloca ptr, align 8, !intel_dtrans_type !220
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp1 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %__x, ptr %__x.addr, align 8
  store ptr %__y, ptr %__y.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  %0 = load ptr, ptr %__x.addr, align 8
  call void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %0)
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp1) #6
  %1 = load ptr, ptr %__y.addr, align 8
  call void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp1, ptr noundef nonnull align 8 dereferenceable(8) %1)
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEeqERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp1)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp1) #6
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  ret i1 %call
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNKSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE4baseEv(ptr noalias sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 "intel_dtrans_func_index"="1" %agg.result, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %this) #2 comdat align 2 !intel.dtrans.func.type !231 {
entry:
  %result.ptr = alloca ptr, align 8
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !220
  store ptr %agg.result, ptr %result.ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %current = getelementptr inbounds %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator", ptr %this1, i32 0, i32 0
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEC2ERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %agg.result, ptr noundef nonnull align 8 dereferenceable(8) %current)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZSt8for_eachIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEENS0_13DeleteFunctorIS5_EEET0_T_SF_SE_(ptr noundef "intel_dtrans_func_index"="2" %__first, ptr noundef "intel_dtrans_func_index"="3" %__last, ptr "intel_dtrans_func_index"="4" %__f.coerce) #2 comdat !intel.dtrans.func.type !232 {
entry:
  %retval = alloca %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", align 8
  %__f = alloca %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", align 8
  %tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %coerce.dive = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %__f, i32 0, i32 0
  store ptr %__f.coerce, ptr %coerce.dive, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %__first, ptr noundef nonnull align 8 dereferenceable(8) %__last)
  br i1 %call, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call1 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %__first)
  %0 = load ptr, ptr %call1, align 8
  call void @_ZNK11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_(ptr noundef nonnull align 8 dereferenceable(8) %__f, ptr noundef %0)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  call void @llvm.lifetime.start.p0(i64 8, ptr %tmp) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %tmp, ptr noundef nonnull align 8 dereferenceable(8) %__first)
  call void @llvm.lifetime.end.p0(i64 8, ptr %tmp) #6
  br label %for.cond, !llvm.loop !233

for.end:                                          ; preds = %for.cond
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %retval, ptr align 8 %__f, i64 8, i1 false)
  %coerce.dive2 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %retval, i32 0, i32 0
  %1 = load ptr, ptr %coerce.dive2, align 8
  ret ptr %1
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %theManager) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !234 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !235
  %theManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %this, ptr %this.addr, align 8
  store ptr %theManager, ptr %theManager.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %m_memoryManager = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %theManager.addr, align 8
  store ptr %0, ptr %m_memoryManager, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5clearEv(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %this) #2 comdat align 2 !intel.dtrans.func.type !236 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !126
  %pos = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  %ref.tmp2 = alloca %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase", align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %pos) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %pos, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp) #6
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp, ptr noundef nonnull align 8 dereferenceable(24) %this1)
  %call = call noundef zeroext i1 @_ZNK11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEneERKSA_(ptr noundef nonnull align 8 dereferenceable(8) %pos, ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp) #6
  br i1 %call, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @llvm.lifetime.start.p0(i64 8, ptr %ref.tmp2) #6
  call void @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi(ptr sret(%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase") align 8 %ref.tmp2, ptr noundef nonnull align 8 dereferenceable(8) %pos, i32 noundef 0)
  %call3 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEE4nodeEv(ptr noundef nonnull align 8 dereferenceable(8) %ref.tmp2)
  call void @_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE8freeNodeERNS5_4NodeE(ptr noundef nonnull align 8 dereferenceable(24) %this1, ptr noundef nonnull align 8 dereferenceable(24) %call3)
  call void @llvm.lifetime.end.p0(i64 8, ptr %ref.tmp2) #6
  br label %while.cond, !llvm.loop !237

while.end:                                        ; preds = %while.cond
  call void @llvm.lifetime.end.p0(i64 8, ptr %pos) #6
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNK11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %thePointer) #2 comdat align 2 !intel.dtrans.func.type !238 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !235
  %thePointer.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %ref.tmp = alloca %"struct._ZTSN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanDestroyFunctor", align 1
  %undef.agg.tmp = alloca %"struct._ZTSN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanDestroyFunctor", align 1
  store ptr %this, ptr %this.addr, align 8
  store ptr %thePointer, ptr %thePointer.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 1, ptr %ref.tmp) #6
  %0 = load ptr, ptr %thePointer.addr, align 8
  call void @_ZN11xalanc_1_1023makeXalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_19XalanDestroyFunctorIT_EEPKS5_(ptr noundef %0)
  %1 = load ptr, ptr %thePointer.addr, align 8
  %m_memoryManager = getelementptr inbounds %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor", ptr %this1, i32 0, i32 0
  %2 = load ptr, ptr %m_memoryManager, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_RN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp, ptr noundef %1, ptr noundef nonnull align 8 dereferenceable(8) %2)
  call void @llvm.lifetime.end.p0(i64 1, ptr %ref.tmp) #6
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1023makeXalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_19XalanDestroyFunctorIT_EEPKS5_(ptr noundef "intel_dtrans_func_index"="1" %0) #12 comdat !intel.dtrans.func.type !239 {
entry:
  %.addr = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPKS3_RN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theArg, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theMemoryManager) #2 comdat align 2 !intel.dtrans.func.type !240 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !241
  %theArg.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theMemoryManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %this, ptr %this.addr, align 8
  store ptr %theArg, ptr %theArg.addr, align 8
  store ptr %theMemoryManager, ptr %theMemoryManager.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theArg.addr, align 8
  %1 = load ptr, ptr %theMemoryManager.addr, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPS3_RN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 1 dereferenceable(1) %this1, ptr noundef %0, ptr noundef nonnull align 8 dereferenceable(8) %1)
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclEPS3_RN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %this, ptr noundef "intel_dtrans_func_index"="2" %theArg, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %theMemoryManager) #2 comdat align 2 !intel.dtrans.func.type !242 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !241
  %theArg.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %theMemoryManager.addr = alloca ptr, align 8, !intel_dtrans_type !33
  store ptr %this, ptr %this.addr, align 8
  store ptr %theArg, ptr %theArg.addr, align 8
  store ptr %theMemoryManager, ptr %theMemoryManager.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theArg.addr, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr %theArg.addr, align 8
  call void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclERS3_(ptr noundef nonnull align 1 dereferenceable(1) %this1, ptr noundef nonnull align 8 dereferenceable(28) %1)
  %2 = load ptr, ptr %theMemoryManager.addr, align 8
  %3 = load ptr, ptr %theArg.addr, align 8
  %vtable = load ptr, ptr %2, align 8
  %4 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %5 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %6 = load ptr, ptr %vfn, align 8
  call void %6(ptr noundef nonnull align 8 dereferenceable(8) %2, ptr noundef %3), !intel_dtrans_type !144
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %entry
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEclERS3_(ptr noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1" %this, ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="2" %theArg) #12 comdat align 2 !intel.dtrans.func.type !243 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !241
  %theArg.addr = alloca ptr, align 8, !intel_dtrans_type !37
  store ptr %this, ptr %this.addr, align 8
  store ptr %theArg, ptr %theArg.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %theArg.addr, align 8
  call void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtED2Ev(ptr noundef nonnull align 8 dereferenceable(28) %0) #6
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtED2Ev(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !244 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !37
  %removedObjects = alloca i16, align 2
  %i = alloca i16, align 2
  %pStruct = alloca ptr, align 8, !intel_dtrans_type !180
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 2, ptr %removedObjects) #6
  store i16 0, ptr %removedObjects, align 2
  call void @llvm.lifetime.start.p0(i64 2, ptr %i) #6
  store i16 0, ptr %i, align 2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i16, ptr %i, align 2
  %conv = zext i16 %0 to i32
  %m_blockSize = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 2
  %1 = load i16, ptr %m_blockSize, align 2
  %conv2 = zext i16 %1 to i32
  %cmp = icmp slt i32 %conv, %conv2
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %for.cond
  %2 = load i16, ptr %removedObjects, align 2
  %conv3 = zext i16 %2 to i32
  %m_objectCount = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 1
  %3 = load i16, ptr %m_objectCount, align 8
  %conv4 = zext i16 %3 to i32
  %cmp5 = icmp slt i32 %conv3, %conv4
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.cond
  %4 = phi i1 [ false, %for.cond ], [ %cmp5, %land.rhs ]
  br i1 %4, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %land.end
  call void @llvm.lifetime.end.p0(i64 2, ptr %i) #6
  br label %for.end

for.body:                                         ; preds = %land.end
  call void @llvm.lifetime.start.p0(i64 8, ptr %pStruct) #6
  %m_objectBlock = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %5 = load ptr, ptr %m_objectBlock, align 8
  %6 = load i16, ptr %i, align 2
  %idxprom = zext i16 %6 to i64
  %arrayidx = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %5, i64 %idxprom
  %call = call noundef ptr @_ZN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlock4castEPv(ptr noundef %arrayidx)
  store ptr %call, ptr %pStruct, align 8
  %7 = load ptr, ptr %pStruct, align 8
  %call6 = invoke noundef zeroext i1 @_ZNK11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE15isOccupiedBlockEPKNS2_9NextBlockE(ptr noundef nonnull align 8 dereferenceable(28) %this1, ptr noundef %7)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %for.body
  br i1 %call6, label %if.then, label %if.end

if.then:                                          ; preds = %invoke.cont
  %m_objectBlock7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %this1, i32 0, i32 3
  %8 = load ptr, ptr %m_objectBlock7, align 8
  %9 = load i16, ptr %i, align 2
  %idxprom8 = zext i16 %9 to i64
  %arrayidx9 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %8, i64 %idxprom8
  %vtable = load ptr, ptr %arrayidx9, align 8
  %10 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %10, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %11 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTSN11xalanc_1_1013XStringCachedE")
  call void @llvm.assume(i1 %11)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 0
  %12 = load ptr, ptr %vfn, align 8
  call void %12(ptr noundef nonnull align 8 dereferenceable(80) %arrayidx9) #6, !intel_dtrans_type !226
  %13 = load i16, ptr %removedObjects, align 2
  %inc = add i16 %13, 1
  store i16 %inc, ptr %removedObjects, align 2
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %invoke.cont
  call void @llvm.lifetime.end.p0(i64 8, ptr %pStruct) #6
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %14 = load i16, ptr %i, align 2
  %inc10 = add i16 %14, 1
  store i16 %inc10, ptr %i, align 2
  br label %for.cond, !llvm.loop !245

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.lifetime.end.p0(i64 2, ptr %removedObjects) #6
  call void @_ZN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %this1) #6
  ret void

terminate.lpad:                                   ; preds = %for.body
  %15 = landingpad { ptr, i32 }
          catch ptr null
  %16 = extractvalue { ptr, i32 } %15, 0
  call void @__clang_call_terminate(ptr %16) #17
  unreachable
}

attributes #0 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #4 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nounwind }
attributes #7 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #8 = { inaccessiblememonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #9 = { noinline noreturn nounwind }
attributes #10 = { nofree }
attributes #11 = { nofree noreturn nounwind }
attributes #12 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #15 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #17 = { noreturn nounwind }
attributes #18 = { builtin nounwind }

!llvm.module.flags = !{!15, !16, !17, !18, !19}
!intel.dtrans.types = !{!20, !22, !26, !32, !35, !36, !38, !41, !45, !48, !49, !52, !54, !56, !57, !60, !62, !63, !66, !69, !72, !73, !74, !75, !76, !78, !79, !81, !82, !84, !85, !86, !87, !88, !90, !92, !93}
!llvm.ident = !{!96}

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
!13 = !{!"L", i32 2, !12, !12}
!14 = !{!"L", i32 3, !12, !12, !12}
!15 = !{i32 1, !"wchar_size", i32 4}
!16 = !{i32 1, !"Virtual Function Elim", i32 0}
!17 = !{i32 7, !"uwtable", i32 2}
!18 = !{i32 1, !"ThinLTO", i32 0}
!19 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!20 = !{!"S", %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1, !21}
!21 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!22 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !23, !24, !25}
!23 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!24 = !{i8 0, i32 0}
!25 = !{!"A", i32 7, !24}
!26 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !27, !30, !31}
!27 = !{!28, i32 2}
!28 = !{!"F", i1 true, i32 0, !29}
!29 = !{i32 0, i32 0}
!30 = !{i16 0, i32 0}
!31 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!32 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !33, !34, !34}
!33 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!34 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 1}
!35 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !27}
!36 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 3, !37, !34, !34}
!37 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!38 = !{!"S", %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 2, !39, !40}
!39 = !{%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 0}
!40 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}
!41 = !{!"S", %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 3, !42, !43, !44}
!42 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 0}
!43 = !{double 0.000000e+00, i32 0}
!44 = !{%"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 0}
!45 = !{!"S", %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 3, !46, !29, !47}
!46 = !{%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 0}
!47 = !{%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 1}
!48 = !{!"S", %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 2, !27, !29}
!49 = !{!"S", %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 2, !50, !51}
!50 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}
!51 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 0}
!52 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !53}
!53 = !{%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 0}
!54 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1, !55}
!55 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 0}
!56 = !{!"S", %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 1, !27}
!57 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !58, !59, !33}
!58 = !{%"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 0}
!59 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 1}
!60 = !{!"S", %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 1, !61}
!61 = !{%"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 0}
!62 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 1, !55}
!63 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !64, !65}
!64 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 1}
!65 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!66 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !67, !30, !30, !68}
!67 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!68 = !{!"A", i32 4, !24}
!69 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !70, !30, !30, !71}
!70 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!71 = !{%"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 1}
!72 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !33}
!73 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !34}
!74 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !34}
!75 = !{!"S", %"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard" zeroinitializer, i32 2, !33, !12}
!76 = !{!"S", %"struct._ZTSN11xalanc_1_1024XalanCompileErrorBooleanILb1EEE.xalanc_1_10::XalanCompileErrorBoolean" zeroinitializer, i32 1, !77}
!77 = !{!"A", i32 1, !24}
!78 = !{!"S", %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" zeroinitializer, i32 2, !30, !29}
!79 = !{!"S", %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" zeroinitializer, i32 1, !80}
!80 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 0}
!81 = !{!"S", %"struct._ZTSSt4lessIPKN11xalanc_1_1013XStringCachedEE.std::less" zeroinitializer, i32 1, !24}
!82 = !{!"S", %"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" zeroinitializer, i32 1, !83}
!83 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 0}
!84 = !{!"S", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !33}
!85 = !{!"S", %"struct._ZTSN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanDestroyFunctor" zeroinitializer, i32 1, !24}
!86 = !{!"S", %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject" zeroinitializer, i32 3, !27, !29, !68}
!87 = !{!"S", %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 -1}
!88 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 2, !89, !47}
!89 = !{%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 0}
!90 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 3, !91, !29, !68}
!91 = !{%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!92 = !{!"S", %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 2, !27, !33}
!93 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !33, !94, !94, !95}
!94 = !{i64 0, i32 0}
!95 = !{i16 0, i32 1}
!96 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!97 = distinct !{!98, !33}
!98 = !{%"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1}
!99 = distinct !{!100, !33}
!100 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 1}
!101 = !{i8 0, i8 2}
!102 = distinct !{!98}
!103 = distinct !{!104}
!104 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 1}
!105 = !{!"F", i1 false, i32 1, !106, !104}
!106 = !{!"void", i32 0}
!107 = distinct !{!71, !98, !108}
!108 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}
!109 = distinct !{!71, !100}
!110 = distinct !{!33, !104}
!111 = distinct !{!71, !108, !33}
!112 = distinct !{!100, !71}
!113 = distinct !{!98, !71}
!114 = distinct !{!100, !71}
!115 = distinct !{!115, !116}
!116 = !{!"llvm.loop.mustprogress"}
!117 = distinct !{!117, !116}
!118 = distinct !{!98}
!119 = distinct !{!104}
!120 = distinct !{!104, !33}
!121 = distinct !{!100}
!122 = distinct !{!100, !71}
!123 = distinct !{!123, !116}
!124 = distinct !{!124, !116}
!125 = distinct !{!126, !33}
!126 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 1}
!127 = distinct !{!104}
!128 = distinct !{!71, !104}
!129 = distinct !{!104, !71}
!130 = distinct !{!104, !71}
!131 = distinct !{!131, !116}
!132 = distinct !{!12, !12}
!133 = distinct !{!126}
!134 = distinct !{!134, !116}
!135 = distinct !{!135, !116}
!136 = distinct !{!137, !126}
!137 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!138 = distinct !{!137, !137}
!139 = distinct !{!137, !126}
!140 = distinct !{!126, !34}
!141 = distinct !{!137, !137}
!142 = distinct !{!34, !137}
!143 = distinct !{!126, !34}
!144 = !{!"F", i1 false, i32 2, !106, !33, !12}
!145 = distinct !{!34, !126}
!146 = distinct !{!137, !34}
!147 = distinct !{!34, !126}
!148 = !{!"F", i1 false, i32 2, !12, !33, !94}
!149 = distinct !{!137, !137}
!150 = distinct !{!12}
!151 = distinct !{!126}
!152 = distinct !{!153, !126}
!153 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 2}
!154 = distinct !{!155}
!155 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 1}
!156 = distinct !{!126, !153}
!157 = distinct !{!37, !33}
!158 = distinct !{!71, !37}
!159 = distinct !{!34, !126}
!160 = distinct !{!161, !161}
!161 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_28XalanListConstIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!162 = distinct !{!34, !126}
!163 = distinct !{!34, !126}
!164 = distinct !{!161, !34}
!165 = distinct !{!137, !137}
!166 = distinct !{!153, !137}
!167 = distinct !{!137, !137}
!168 = distinct !{!34, !126, !153, !137}
!169 = distinct !{!153, !153, !153, !33}
!170 = distinct !{!37, !33, !153, !33, !95}
!171 = distinct !{!172, !33}
!172 = !{%"class._ZTSN11xalanc_1_1020XalanAllocationGuardE.xalanc_1_10::XalanAllocationGuard" zeroinitializer, i32 1}
!173 = distinct !{!12, !172}
!174 = distinct !{!37, !33}
!175 = distinct !{!175, !116}
!176 = distinct !{!172}
!177 = distinct !{!172}
!178 = distinct !{!155, !33}
!179 = distinct !{!180}
!180 = !{%"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" zeroinitializer, i32 1}
!181 = distinct !{!155}
!182 = distinct !{!183, !33}
!183 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1}
!184 = distinct !{!71, !183, !12}
!185 = distinct !{!183}
!186 = distinct !{!183, !71}
!187 = distinct !{!180, !12}
!188 = distinct !{!37, !71}
!189 = distinct !{!190, !126}
!190 = !{%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_28XalanListConstIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" zeroinitializer, i32 1}
!191 = distinct !{!190, !126}
!192 = distinct !{!190, !190}
!193 = distinct !{!153, !190}
!194 = distinct !{!37, !71}
!195 = distinct !{!190, !190}
!196 = distinct !{!190, !34}
!197 = distinct !{!190, !190}
!198 = distinct !{!34, !190}
!199 = distinct !{!34, !161}
!200 = distinct !{!153, !161}
!201 = distinct !{!37, !180}
!202 = distinct !{!180, !12}
!203 = distinct !{!155, !71}
!204 = distinct !{!180}
!205 = distinct !{!155, !71}
!206 = distinct !{!207, !71, !71}
!207 = !{%"struct._ZTSSt4lessIPKN11xalanc_1_1013XStringCachedEE.std::less" zeroinitializer, i32 1}
!208 = distinct !{!161, !161}
!209 = distinct !{!34, !161}
!210 = distinct !{!153, !126}
!211 = distinct !{!126, !153}
!212 = distinct !{!33, !126}
!213 = distinct !{!126}
!214 = distinct !{!126, !137}
!215 = distinct !{!126, !34}
!216 = distinct !{!37, !71}
!217 = distinct !{!100}
!218 = distinct !{!137, !137}
!219 = distinct !{!220, !126}
!220 = !{%"class._ZTSSt16reverse_iteratorIN11xalanc_1_1021XalanListIteratorBaseINS0_23XalanListIteratorTraitsIPNS0_18ReusableArenaBlockINS0_13XStringCachedEtEEEENS0_9XalanListIS6_E4NodeEEEE.std::reverse_iterator" zeroinitializer, i32 1}
!221 = distinct !{!220, !126}
!222 = distinct !{!220, !220}
!223 = distinct !{!153, !220}
!224 = distinct !{!220, !220}
!225 = distinct !{!71}
!226 = !{!"F", i1 false, i32 1, !106, !71}
!227 = distinct !{!161, !137}
!228 = distinct !{!155}
!229 = distinct !{!220, !137}
!230 = distinct !{!220, !220}
!231 = distinct !{!137, !220}
!232 = distinct !{!33, !137, !137, !33}
!233 = distinct !{!233, !116}
!234 = distinct !{!235, !33}
!235 = !{%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1}
!236 = distinct !{!126}
!237 = distinct !{!237, !116}
!238 = distinct !{!235, !37}
!239 = distinct !{!37}
!240 = distinct !{!241, !37, !33}
!241 = !{%"struct._ZTSN11xalanc_1_1019XalanDestroyFunctorINS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanDestroyFunctor" zeroinitializer, i32 1}
!242 = distinct !{!241, !37, !33}
!243 = distinct !{!241, !37}
!244 = distinct !{!37}
!245 = distinct !{!245, !116}
