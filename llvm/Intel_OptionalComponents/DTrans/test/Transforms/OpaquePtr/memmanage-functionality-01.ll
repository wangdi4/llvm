; This test verifies the following functionalities are recognized for
; MemManageTransOP:
;
; getMemManager    : _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; Constructor      : _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb
; AllocateBlock    : _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CommitAllocation : _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_
; Destructor       : _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev
; Reset            : _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv
; DestroyObject    : _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

; RUN: opt < %s -opaque-pointers -passes=dtrans-memmanagetransop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-memmanagetransop -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK: MemManageTransOP transformation:
; CHECK:   Considering candidate: %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator"
; CHECK: Recognized GetMemManager: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv
; CHECK: Recognized Constructor: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb
; CHECK: Recognized AllocateBlock: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv
; CHECK: Recognized CommitAllocation: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_

; The following will be enabled as CHECKs as more functionality is implemented
; TODO: Recognized Destructor: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev
; TODO: Recognized Reset: _ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv
; TODO: Recognized DestroyObject: _ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

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
%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" = type { %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext", ptr }
%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" = type <{ %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" = type { i16, i32 }
%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" = type { i8 }
%"class._ZTSN11xalanc_1_103AVTE.xalanc_1_10::AVT" = type { ptr, ptr, i64, ptr, i32, ptr }
%"class._ZTSN11xalanc_1_107AVTPartE.xalanc_1_10::AVTPart" = type { ptr }
%"class._ZTSN11xalanc_1_1024XPathConstructionContextE.xalanc_1_10::XPathConstructionContext" = type { ptr, ptr }
%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" = type { ptr }
%"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver" = type { ptr }
%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_105XPathE.xalanc_1_10::XPath" = type <{ %"class._ZTSN11xalanc_1_1015XPathExpressionE.xalanc_1_10::XPathExpression", ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1015XPathExpressionE.xalanc_1_10::XPathExpression" = type { %"class._ZTSN11xalanc_1_1011XalanVectorIiNS_31MemoryManagedConstructionTraitsIiEEEE.xalanc_1_10::XalanVector", i32, %"class._ZTSN11xalanc_1_1011XalanVectorINS_6XTokenENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", i32, ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIdNS_31MemoryManagedConstructionTraitsIdEEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIiNS_31MemoryManagedConstructionTraitsIiEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_6XTokenENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIdNS_31MemoryManagedConstructionTraitsIdEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_106XTokenE.xalanc_1_10::XToken" = type <{ %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject", ptr, double, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1017AttributeListImplE.xalanc_1_10::AttributeListImpl" = type { %"class._ZTSN11xercesc_2_713AttributeListE.xercesc_2_7::AttributeList", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_20AttributeVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_20AttributeVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xercesc_2_713AttributeListE.xercesc_2_7::AttributeList" = type { ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_20AttributeVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1020AttributeVectorEntryE.xalanc_1_10::AttributeVectorEntry" = type { ptr, %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1013CountersTableE.xalanc_1_10::CountersTable" = type { %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_107CounterE.xalanc_1_10::Counter" = type { i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", ptr, ptr }
%"class._ZTSN11xalanc_1_1010ElemNumberE.xalanc_1_10::ElemNumber" = type { %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base", ptr, ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, i64 }
%"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base" = type <{ %"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver", ptr, %"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler", i32, [4 x i8], ptr, ptr, ptr, %union._ZTSN11xalanc_1_1019ElemTemplateElementUt_E.anon, %"class._ZTSN11xalanc_1_1019ElemTemplateElement12LocatorProxyE.xalanc_1_10::ElemTemplateElement::LocatorProxy", i16 }>
%"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler" = type { %"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler9NamespaceENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler17NamespaceExtendedENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler9NamespaceENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler17NamespaceExtendedENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1028DOMStringPointerHashFunctionE.xalanc_1_10::DOMStringPointerHashFunction", %"struct._ZTSN11xalanc_1_1013pointer_equalINS_14XalanDOMStringEEE.xalanc_1_10::pointer_equal", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"struct._ZTSN11xalanc_1_1028DOMStringPointerHashFunctionE.xalanc_1_10::DOMStringPointerHashFunction" = type { i8 }
%"struct._ZTSN11xalanc_1_1013pointer_equalINS_14XalanDOMStringEEE.xalanc_1_10::pointer_equal" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%union._ZTSN11xalanc_1_1019ElemTemplateElementUt_E.anon = type { ptr }
%"class._ZTSN11xalanc_1_1019ElemTemplateElement12LocatorProxyE.xalanc_1_10::ElemTemplateElement::LocatorProxy" = type { %"class._ZTSN11xalanc_1_1012XalanLocatorE.xalanc_1_10::XalanLocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1012XalanLocatorE.xalanc_1_10::XalanLocator" = type { %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" }
%"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet" = type { %"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver", ptr, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14KeyDeclarationENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanSpaceNodeTesterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_10StylesheetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64, %"class._ZTSN11xalanc_1_1010XalanDequeINS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanDeque", %"class._ZTSN11xalanc_1_1010XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanDeque", i8, %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", ptr, %"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", double, %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", %"struct._ZTSN11xalanc_1_1016XalanMapIteratorINS_27XalanMapConstIteratorTraitsISt4pairIKNS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS8_EEEEEEENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIS3_SB_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISK_E4NodeEEEEE.xalanc_1_10::XalanMapIterator", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", %"struct._ZTSN11xalanc_1_1016XalanMapIteratorINS_27XalanMapConstIteratorTraitsISt4pairIKNS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS8_EEEEEEENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIS3_SB_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISK_E4NodeEEEEE.xalanc_1_10::XalanMapIterator", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17ElemDecimalFormatENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_14KeyDeclarationENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanSpaceNodeTesterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_10StylesheetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1010XalanDequeINS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEENS_31MemoryManagedConstructionTraitsIS8_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEENS_31MemoryManagedConstructionTraitsIS8_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEENS_31MemoryManagedConstructionTraitsIS8_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1010XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS_31MemoryManagedConstructionTraitsIS5_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS_31MemoryManagedConstructionTraitsIS5_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS_31MemoryManagedConstructionTraitsIS5_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction" = type { i8 }
%"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1024XalanHashMemberReferenceINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberReference", %"struct._ZTSSt8equal_toIN11xalanc_1_1010XalanQNameEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"struct._ZTSN11xalanc_1_1024XalanHashMemberReferenceINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberReference" = type { i8 }
%"struct._ZTSSt8equal_toIN11xalanc_1_1010XalanQNameEE.std::equal_to" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1016XalanMapIteratorINS_27XalanMapConstIteratorTraitsISt4pairIKNS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS8_EEEEEEENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIS3_SB_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISK_E4NodeEEEEE.xalanc_1_10::XalanMapIterator" = type { %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISE_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISE_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17ElemDecimalFormatENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement" = type <{ %"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver", ptr, %"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler", i32, [4 x i8], ptr, ptr, ptr, %union._ZTSN11xalanc_1_1019ElemTemplateElementUt_E.anon, %"class._ZTSN11xalanc_1_1019ElemTemplateElement12LocatorProxyE.xalanc_1_10::ElemTemplateElement::LocatorProxy", i16, [6 x i8] }>
%"class._ZTSN11xalanc_1_1017NamespacesHandler9NamespaceE.xalanc_1_10::NamespacesHandler::Namespace" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1017NamespacesHandler17NamespaceExtendedE.xalanc_1_10::NamespacesHandler::NamespaceExtended" = type { %"class._ZTSN11xalanc_1_1017NamespacesHandler9NamespaceE.xalanc_1_10::NamespacesHandler::Namespace", ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1026StylesheetExecutionContextE.xalanc_1_10::StylesheetExecutionContext" = type { %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES5_NS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xalanc_1_1013XalanDocumentE.xalanc_1_10::XalanDocument" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"class._ZTSN11xalanc_1_1012XalanElementE.xalanc_1_10::XalanElement" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"class._ZTSN11xalanc_1_109XalanAttrE.xalanc_1_10::XalanAttr" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"class._ZTSN11xalanc_1_1012XalanCommentE.xalanc_1_10::XalanComment" = type { %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" }
%"class._ZTSN11xalanc_1_1026XalanProcessingInstructionE.xalanc_1_10::XalanProcessingInstruction" = type { %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" }
%"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener" = type { %"class._ZTSN11xercesc_2_715DocumentHandlerE.xercesc_2_7::DocumentHandler", ptr, i32, i32 }
%"class._ZTSN11xercesc_2_715DocumentHandlerE.xercesc_2_7::DocumentHandler" = type { ptr }
%"class._ZTSN11xalanc_1_1017XalanNamedNodeMapE.xalanc_1_10::XalanNamedNodeMap" = type { ptr }
%"class._ZTSN11xalanc_1_1017XalanOutputStreamE.xalanc_1_10::XalanOutputStream" = type { ptr, i32, ptr, i32, %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i8, i8, %"class._ZTSN11xalanc_1_1011XalanVectorIcNS_31MemoryManagedConstructionTraitsIcEEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIcNS_31MemoryManagedConstructionTraitsIcEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1021XalanOutputTranscoderE.xalanc_1_10::XalanOutputTranscoder" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1020DOMStringPrintWriterE.xalanc_1_10::DOMStringPrintWriter" = type { %"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter", ptr }
%"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter" = type { %"class._ZTSN11xalanc_1_106WriterE.xalanc_1_10::Writer", i8, ptr }
%"class._ZTSN11xalanc_1_106WriterE.xalanc_1_10::Writer" = type { ptr }
%"class._ZTSN11xalanc_1_1010DOMSupportE.xalanc_1_10::DOMSupport" = type { ptr }
%"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool" = type { ptr, %"class._ZTSN11xalanc_1_1023XalanDOMStringAllocatorE.xalanc_1_10::XalanDOMStringAllocator", i64, %"class._ZTSN11xalanc_1_1023XalanDOMStringHashTableE.xalanc_1_10::XalanDOMStringHashTable" }
%"class._ZTSN11xalanc_1_1023XalanDOMStringAllocatorE.xalanc_1_10::XalanDOMStringAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1023XalanDOMStringHashTableE.xalanc_1_10::XalanDOMStringHashTable" = type <{ i64, i64, %"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEEE.xalanc_1_10::XalanMemMgrAutoPtrArray", i64, i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEEE.xalanc_1_10::XalanMemMgrAutoPtrArray" = type { %"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEE22MemMgrAutoPtrArrayDataE.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" }
%"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEE22MemMgrAutoPtrArrayDataE.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" = type { ptr, ptr, i64 }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1017XalanQNameByValueE.xalanc_1_10::XalanQNameByValue" = type { %"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName" = type { ptr }
%"class._ZTSN11xalanc_1_1014StylesheetRootE.xalanc_1_10::StylesheetRoot" = type { %"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i32, [4 x i8], %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i8, [7 x i8], %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i32, %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_10XalanQNameENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", i8, %"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", ptr, ptr, ptr, i8, i8, i32, i8, i64, %"class._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_10XalanQNameENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1022XalanHashMemberPointerINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberPointer", %"struct._ZTSN11xalanc_1_1013pointer_equalINS_10XalanQNameEEE.xalanc_1_10::pointer_equal", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEENS_32ConstructWithMemoryManagerTraitsISM_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"struct._ZTSN11xalanc_1_1022XalanHashMemberPointerINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberPointer" = type { i8 }
%"struct._ZTSN11xalanc_1_1013pointer_equalINS_10XalanQNameEEE.xalanc_1_10::pointer_equal" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEENS_32ConstructWithMemoryManagerTraitsISM_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1012ElemTemplateE.xalanc_1_10::ElemTemplate" = type { %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base", ptr, ptr, ptr, double }
%"class._ZTSN11xalanc_1_1014KeyDeclarationE.xalanc_1_10::KeyDeclaration" = type { ptr, ptr, ptr, ptr, i64, i64 }
%"class._ZTSN11xalanc_1_1020XalanSpaceNodeTesterE.xalanc_1_10::XalanSpaceNodeTester" = type { %"class._ZTSN11xalanc_1_105XPath10NodeTesterE.xalanc_1_10::XPath::NodeTester", i32, i32 }
%"class._ZTSN11xalanc_1_105XPath10NodeTesterE.xalanc_1_10::XPath::NodeTester" = type { ptr, ptr, ptr, { i64, i64 }, { i64, i64 } }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1012ElemVariableE.xalanc_1_10::ElemVariable" = type { %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base", ptr, ptr, i8, %"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr", ptr }
%"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr" = type { ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1021XalanMatchPatternDataE.xalanc_1_10::XalanMatchPatternData" = type <{ ptr, i64, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1017ElemDecimalFormatE.xalanc_1_10::ElemDecimalFormat" = type { %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base", ptr, ptr, ptr, ptr, %"class._ZTSN11xalanc_1_1025XalanDecimalFormatSymbolsE.xalanc_1_10::XalanDecimalFormatSymbols" }
%"class._ZTSN11xalanc_1_1025XalanDecimalFormatSymbolsE.xalanc_1_10::XalanDecimalFormatSymbols" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i16, i16, i16, [2 x i8], %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i16, i16, [4 x i8], %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i16, i16, i16, i16 }
%"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase" = type { ptr }
%"class._ZTSN11xalanc_1_109NameSpaceE.xalanc_1_10::NameSpace" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEPNS0_18ExtensionNSHandlerEE.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1018ExtensionNSHandlerE.xalanc_1_10::ExtensionNSHandler" = type <{ %"class._ZTSN11xalanc_1_1024ExtensionFunctionHandlerE.xalanc_1_10::ExtensionFunctionHandler.base", [7 x i8], %"class._ZTSN11xalanc_1_108XalanSetINS_14XalanDOMStringEEE.xalanc_1_10::XalanSet", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1024ExtensionFunctionHandlerE.xalanc_1_10::ExtensionFunctionHandler.base" = type <{ ptr, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", ptr, %"class._ZTSN11xalanc_1_108XalanSetINS_14XalanDOMStringEEE.xalanc_1_10::XalanSet", i8 }>
%"class._ZTSN11xalanc_1_108XalanSetINS_14XalanDOMStringEEE.xalanc_1_10::XalanSet" = type { %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISF_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xalanc_1_1016ElemAttributeSetE.xalanc_1_10::ElemAttributeSet" = type { %"class._ZTSN11xalanc_1_107ElemUseE.xalanc_1_10::ElemUse", ptr }
%"class._ZTSN11xalanc_1_107ElemUseE.xalanc_1_10::ElemUse" = type { %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base", ptr, i64 }
%"class._ZTSN11xalanc_1_1018MutableNodeRefListE.xalanc_1_10::MutableNodeRefList" = type <{ %"class._ZTSN11xalanc_1_1011NodeRefListE.xalanc_1_10::NodeRefList", i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011NodeRefListE.xalanc_1_10::NodeRefList" = type { %"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1010NodeSorterE.xalanc_1_10::NodeSorter" = type { %"class._ZTSN11xalanc_1_1011XalanVectorINS0_IdNS_31MemoryManagedConstructionTraitsIdEEEENS1_IS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_11NodeSortKeyENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_10NodeSorter11VectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_IdNS_31MemoryManagedConstructionTraitsIdEEEENS1_IS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_11NodeSortKeyENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_10NodeSorter11VectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011NodeSortKeyE.xalanc_1_10::NodeSortKey" = type { ptr, ptr, i8, i8, i32, ptr, ptr }
%"struct._ZTSN11xalanc_1_1010NodeSorter11VectorEntryE.xalanc_1_10::NodeSorter::VectorEntry" = type <{ ptr, i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1021XalanQNameByReferenceE.xalanc_1_10::XalanQNameByReference" = type { %"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName", ptr, ptr }
%"struct._ZTSN11xalanc_1_1026StylesheetExecutionContext22UseAttributeSetIndexesE.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" = type { i32, i32 }
%"class._ZTSN11xalanc_1_1015XPathEnvSupportE.xalanc_1_10::XPathEnvSupport" = type { ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEbE.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i8, [7 x i8] }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSN11xalanc_1_1027hash_null_terminated_arraysItEE.xalanc_1_10::hash_null_terminated_arrays" = type { i8 }
%"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1021FormatterToSourceTreeE.xalanc_1_10::FormatterToSourceTree" = type { %"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener", ptr, ptr, ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_22XalanSourceTreeElementENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_22XalanSourceTreeElementENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1023XalanSourceTreeDocumentE.xalanc_1_10::XalanSourceTreeDocument" = type { %"class._ZTSN11xalanc_1_1013XalanDocumentE.xalanc_1_10::XalanDocument", ptr, ptr, %"class._ZTSN11xalanc_1_1033XalanSourceTreeAttributeAllocatorE.xalanc_1_10::XalanSourceTreeAttributeAllocator", %"class._ZTSN11xalanc_1_1035XalanSourceTreeAttributeNSAllocatorE.xalanc_1_10::XalanSourceTreeAttributeNSAllocator", %"class._ZTSN11xalanc_1_1031XalanSourceTreeCommentAllocatorE.xalanc_1_10::XalanSourceTreeCommentAllocator", %"class._ZTSN11xalanc_1_1032XalanSourceTreeElementAAllocatorE.xalanc_1_10::XalanSourceTreeElementAAllocator", %"class._ZTSN11xalanc_1_1034XalanSourceTreeElementANSAllocatorE.xalanc_1_10::XalanSourceTreeElementANSAllocator", %"class._ZTSN11xalanc_1_1033XalanSourceTreeElementNAAllocatorE.xalanc_1_10::XalanSourceTreeElementNAAllocator", %"class._ZTSN11xalanc_1_1035XalanSourceTreeElementNANSAllocatorE.xalanc_1_10::XalanSourceTreeElementNANSAllocator", %"class._ZTSN11xalanc_1_1045XalanSourceTreeProcessingInstructionAllocatorE.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator", %"class._ZTSN11xalanc_1_1028XalanSourceTreeTextAllocatorE.xalanc_1_10::XalanSourceTreeTextAllocator", %"class._ZTSN11xalanc_1_1031XalanSourceTreeTextIWSAllocatorE.xalanc_1_10::XalanSourceTreeTextIWSAllocator", %"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool", %"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool", %"class._ZTSN11xalanc_1_1019XalanArrayAllocatorIPNS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanArrayAllocator", i64, i8, %"class._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEEE.xalanc_1_10::XalanMap", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", %"class._ZTSN11xalanc_1_1023XalanDOMStringAllocatorE.xalanc_1_10::XalanDOMStringAllocator", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"class._ZTSN11xalanc_1_1033XalanSourceTreeAttributeAllocatorE.xalanc_1_10::XalanSourceTreeAttributeAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeAttrENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeAttrENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1035XalanSourceTreeAttributeNSAllocatorE.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_21XalanSourceTreeAttrNSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_21XalanSourceTreeAttrNSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1031XalanSourceTreeCommentAllocatorE.xalanc_1_10::XalanSourceTreeCommentAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeCommentENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeCommentENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1032XalanSourceTreeElementAAllocatorE.xalanc_1_10::XalanSourceTreeElementAAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeElementAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeElementAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1034XalanSourceTreeElementANSAllocatorE.xalanc_1_10::XalanSourceTreeElementANSAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_25XalanSourceTreeElementANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_25XalanSourceTreeElementANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1033XalanSourceTreeElementNAAllocatorE.xalanc_1_10::XalanSourceTreeElementNAAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_24XalanSourceTreeElementNAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_24XalanSourceTreeElementNAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1035XalanSourceTreeElementNANSAllocatorE.xalanc_1_10::XalanSourceTreeElementNANSAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_26XalanSourceTreeElementNANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_26XalanSourceTreeElementNANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1045XalanSourceTreeProcessingInstructionAllocatorE.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_36XalanSourceTreeProcessingInstructionENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_36XalanSourceTreeProcessingInstructionENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1028XalanSourceTreeTextAllocatorE.xalanc_1_10::XalanSourceTreeTextAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeTextENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeTextENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1031XalanSourceTreeTextIWSAllocatorE.xalanc_1_10::XalanSourceTreeTextIWSAllocator" = type { %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeTextIWSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" }
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeTextIWSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1019XalanArrayAllocatorIPNS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanArrayAllocator" = type { %"class._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEEE.xalanc_1_10::XalanList", i64, ptr }
%"class._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1027hash_null_terminated_arraysItEE.xalanc_1_10::hash_null_terminated_arrays", %"struct._ZTSN11xalanc_1_1028equal_null_terminated_arraysItEE.xalanc_1_10::equal_null_terminated_arrays", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"struct._ZTSN11xalanc_1_1028equal_null_terminated_arraysItEE.xalanc_1_10::equal_null_terminated_arrays" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1031XalanSourceTreeDocumentFragmentE.xalanc_1_10::XalanSourceTreeDocumentFragment" = type { %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment", ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" = type { %"class._ZTSN11xalanc_1_1012XalanElementE.xalanc_1_10::XalanElement", ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64 }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSSt4pairImPN11xalanc_1_1011XalanVectorIPNS0_19XalanSourceTreeAttrENS0_31MemoryManagedConstructionTraitsIS3_EEEEE.std::pair" = type { i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEE4NodeE.xalanc_1_10::XalanList<std::pair<unsigned long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" = type { %"struct._ZTSSt4pairImPN11xalanc_1_1011XalanVectorIPNS0_19XalanSourceTreeAttrENS0_31MemoryManagedConstructionTraitsIS3_EEEEE.std::pair", ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEE5EntryE.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEE5EntryE.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1019XalanSourceTreeTextE.xalanc_1_10::XalanSourceTreeText" = type { %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText", ptr, ptr, ptr, ptr, i64 }
%"class._ZTSN11xalanc_1_1022XalanSourceTreeCommentE.xalanc_1_10::XalanSourceTreeComment" = type { %"class._ZTSN11xalanc_1_1012XalanCommentE.xalanc_1_10::XalanComment", ptr, ptr, ptr, ptr, ptr, i64 }
%"class._ZTSN11xalanc_1_1036XalanSourceTreeProcessingInstructionE.xalanc_1_10::XalanSourceTreeProcessingInstruction" = type { %"class._ZTSN11xalanc_1_1026XalanProcessingInstructionE.xalanc_1_10::XalanProcessingInstruction", ptr, ptr, ptr, ptr, ptr, ptr, i64 }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_21XalanSourceTreeAttrNSEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_21XalanSourceTreeAttrNSEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeCommentEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeCommentEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeElementAEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeElementAEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_25XalanSourceTreeElementANSEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_25XalanSourceTreeElementANSEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_24XalanSourceTreeElementNAEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_24XalanSourceTreeElementNAEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_26XalanSourceTreeElementNANSEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_26XalanSourceTreeElementNANSEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_36XalanSourceTreeProcessingInstructionEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_36XalanSourceTreeProcessingInstructionEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeTextEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeTextEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlock" = type { %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlockBase" }
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeTextIWSEEE.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeTextIWSEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr" = type { %"class._ZTSN11xalanc_1_109XalanAttrE.xalanc_1_10::XalanAttr", ptr, ptr, ptr, i64 }
%"class._ZTSN11xalanc_1_1021XalanSourceTreeAttrNSE.xalanc_1_10::XalanSourceTreeAttrNS" = type { %"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr", ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1023XalanSourceTreeElementAE.xalanc_1_10::XalanSourceTreeElementA" = type { %"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement", %"class._ZTSN11xalanc_1_1017XalanNamedNodeMapE.xalanc_1_10::XalanNamedNodeMap", ptr, i64 }
%"class._ZTSN11xalanc_1_1025XalanSourceTreeElementANSE.xalanc_1_10::XalanSourceTreeElementANS" = type { %"class._ZTSN11xalanc_1_1023XalanSourceTreeElementAE.xalanc_1_10::XalanSourceTreeElementA", ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1024XalanSourceTreeElementNAE.xalanc_1_10::XalanSourceTreeElementNA" = type { %"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" }
%"class._ZTSN11xalanc_1_1026XalanSourceTreeElementNANSE.xalanc_1_10::XalanSourceTreeElementNANS" = type { %"class._ZTSN11xalanc_1_1024XalanSourceTreeElementNAE.xalanc_1_10::XalanSourceTreeElementNA", ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1022XalanSourceTreeTextIWSE.xalanc_1_10::XalanSourceTreeTextIWS" = type { %"class._ZTSN11xalanc_1_1019XalanSourceTreeTextE.xalanc_1_10::XalanSourceTreeText" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_10XObjectPtrENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XPathProcessorE.xalanc_1_10::XPathProcessor" = type { ptr }
%"class._ZTSN11xalanc_1_1031XPathConstructionContextDefaultE.xalanc_1_10::XPathConstructionContextDefault" = type { %"class._ZTSN11xalanc_1_1024XPathConstructionContextE.xalanc_1_10::XPathConstructionContext", %"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool", %"class._ZTSN11xalanc_1_1019XalanDOMStringCacheE.xalanc_1_10::XalanDOMStringCache" }
%"class._ZTSN11xalanc_1_1019XalanDOMStringCacheE.xalanc_1_10::XalanDOMStringCache" = type { %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i32, [4 x i8], %"class._ZTSN11xalanc_1_1031XalanDOMStringReusableAllocatorE.xalanc_1_10::XalanDOMStringReusableAllocator" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1031XalanDOMStringReusableAllocatorE.xalanc_1_10::XalanDOMStringReusableAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_14XalanDOMStringEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_108KeyTableE.xalanc_1_10::KeyTable" = type { ptr, ptr, %"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEEE.xalanc_1_10::XalanMap" }
%"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1024XalanHashMemberReferenceINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberReference", %"struct._ZTSSt8equal_toIN11xalanc_1_1010XalanQNameEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceENS0_8XalanMapINS0_14XalanDOMStringENS0_18MutableNodeRefListENS0_17XalanMapKeyTraitsIS4_EEEEE.std::pair" = type { %"class._ZTSN11xalanc_1_1021XalanQNameByReferenceE.xalanc_1_10::XalanQNameByReference", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEENS_32ConstructWithMemoryManagerTraitsISH_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEENS_32ConstructWithMemoryManagerTraitsISH_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_18MutableNodeRefListEE.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1018MutableNodeRefListE.xalanc_1_10::MutableNodeRefList" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS8_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS2_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEEENS6_IS3_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" }
%"struct._ZTSSt4pairIKPKN11xalanc_1_1014XalanDOMStringES3_E.std::pair" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1018OutputContextStackE.xalanc_1_10::OutputContextStack" = type { %"class._ZTSN11xalanc_1_1010XalanDequeINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_18OutputContextStack13OutputContextEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator", i64 }
%"class._ZTSN11xalanc_1_1010XalanDequeINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_18OutputContextStack13OutputContextEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1018OutputContextStack13OutputContextE.xalanc_1_10::OutputContextStack::OutputContext" = type <{ ptr, %"class._ZTSN11xalanc_1_1017AttributeListImplE.xalanc_1_10::AttributeListImpl", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i8, i8, [6 x i8] }>
%"class._ZTSN11xalanc_1_1015ProblemListenerE.xalanc_1_10::ProblemListener" = type { ptr }
%"class._ZTSN11xalanc_1_1022ProblemListenerDefaultE.xalanc_1_10::ProblemListenerDefault" = type { %"class._ZTSN11xalanc_1_1015ProblemListenerE.xalanc_1_10::ProblemListener", ptr, ptr }
%"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceEPKNS0_12ElemTemplateEE.std::pair" = type { %"class._ZTSN11xalanc_1_1021XalanQNameByReferenceE.xalanc_1_10::XalanQNameByReference", ptr }
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_11XalanVectorIPKNS0_21XalanMatchPatternDataENS0_31MemoryManagedConstructionTraitsIS6_EEEEE.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_11TopLevelArgENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011TopLevelArgE.xalanc_1_10::TopLevelArg" = type { %"class._ZTSN11xalanc_1_1017XalanQNameByValueE.xalanc_1_10::XalanQNameByValue", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr" }
%"class._ZTSN11xalanc_1_1014XSLTEngineImplE.xalanc_1_10::XSLTEngineImpl" = type { %"class._ZTSN11xalanc_1_1013XSLTProcessorE.xalanc_1_10::XSLTProcessor", %"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", ptr, ptr, %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKN11xercesc_2_77LocatorENS_31MemoryManagedConstructionTraitsIS4_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1022ProblemListenerDefaultE.xalanc_1_10::ProblemListenerDefault", ptr, ptr, i8, i8, ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_13TraceListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64, %"class._ZTSN11xalanc_1_1011XalanVectorINS_11TopLevelArgENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", ptr, ptr, ptr, ptr, %"class._ZTSN11xalanc_1_1018OutputContextStackE.xalanc_1_10::OutputContextStack", %"class._ZTSN11xalanc_1_1020XalanNamespacesStackE.xalanc_1_10::XalanNamespacesStack", %"class._ZTSN11xalanc_1_1017AttributeListImplE.xalanc_1_10::AttributeListImpl", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", i8, %"class._ZTSN11xalanc_1_1031XPathConstructionContextDefaultE.xalanc_1_10::XPathConstructionContextDefault" }
%"class._ZTSN11xalanc_1_1013XSLTProcessorE.xalanc_1_10::XSLTProcessor" = type { ptr }
%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" = type { %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" }
%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" = type { %"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1014XPathProcessorEE.std::pair" }
%"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1014XPathProcessorEE.std::pair" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKN11xercesc_2_77LocatorENS_31MemoryManagedConstructionTraitsIS4_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_13TraceListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1020XalanNamespacesStackE.xalanc_1_10::XalanNamespacesStack" = type { %"class._ZTSN11xalanc_1_1010XalanDequeINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_20XalanNamespacesStack25XalanNamespacesStackEntryEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_20XalanNamespacesStack25XalanNamespacesStackEntryEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1010XalanDequeINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_20XalanNamespacesStack25XalanNamespacesStackEntryEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class._ZTSN11xalanc_1_1012XPathFactoryE.xalanc_1_10::XPathFactory" = type { ptr }
%"class._ZTSN11xalanc_1_1016XMLParserLiaisonE.xalanc_1_10::XMLParserLiaison" = type { ptr }
%"class._ZTSN11xalanc_1_1013TraceListenerE.xalanc_1_10::TraceListener" = type { ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1020XalanNamespacesStack25XalanNamespacesStackEntryE.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" = type { %"class._ZTSN11xalanc_1_1010XalanDequeINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanDeque", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_14XalanNamespaceEEENS_10XalanDequeIS2_NS_31MemoryManagedConstructionTraitsIS2_EEEEEE.xalanc_1_10::XalanDequeIterator" }
%"class._ZTSN11xalanc_1_1010XalanDequeINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS5_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS5_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS5_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_14XalanNamespaceEEENS_10XalanDequeIS2_NS_31MemoryManagedConstructionTraitsIS2_EEEEEE.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1022XalanCollationServices23CollationCompareFunctorE.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefaultE.xalanc_1_10::StylesheetExecutionContextDefault" = type <{ %"class._ZTSN11xalanc_1_1026StylesheetExecutionContextE.xalanc_1_10::StylesheetExecutionContext", %"class._ZTSN11xalanc_1_1028XPathExecutionContextDefaultE.xalanc_1_10::XPathExecutionContextDefault", ptr, ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17FormatterListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_11PrintWriterENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17XalanOutputStreamENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", ptr, ptr, %"class._ZTSN11xalanc_1_1014VariablesStackE.xalanc_1_10::VariablesStack", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap", %"class._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap", %"class._ZTSN11xalanc_1_1013CountersTableE.xalanc_1_10::CountersTable", %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr", ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemTemplateENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", i32, [4 x i8], %"class._ZTSN11xalanc_1_1024XResultTreeFragAllocatorE.xalanc_1_10::XResultTreeFragAllocator", %"class._ZTSN11xalanc_1_1040XalanSourceTreeDocumentFragmentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator", %"class._ZTSN11xalanc_1_1032XalanSourceTreeDocumentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentAllocator", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_10XalanQNameENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIiNS_31MemoryManagedConstructionTraitsIiEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_10XObjectPtrENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache", %"class._ZTSN11xalanc_1_1011XalanVectorINS_33StylesheetExecutionContextDefault16NodesToTransformENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_14XalanDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS2_EENS_13DeleteFunctorIS2_EENS_24DefaultCacheResetFunctorIS2_EEEE.xalanc_1_10::XalanObjectStackCache", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_21FormatterToSourceTreeENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorINS_26StylesheetExecutionContext22UseAttributeSetIndexesENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1010NodeSorterE.xalanc_1_10::NodeSorter", i8, [3 x i8], i32, i32, i8, [3 x i8] }>
%"class._ZTSN11xalanc_1_1028XPathExecutionContextDefaultE.xalanc_1_10::XPathExecutionContextDefault" = type { %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext", ptr, ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_15NodeRefListBaseENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", ptr, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1016XalanObjectCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_22ClearCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectCache", %"class._ZTSN11xalanc_1_1019XalanDOMStringCacheE.xalanc_1_10::XalanDOMStringCache", %"struct._ZTSN11xalanc_1_1028XPathExecutionContextDefault28ContextNodeListPositionCacheE.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache", %"class._ZTSN11xalanc_1_1017XalanQNameByValueE.xalanc_1_10::XalanQNameByValue" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_15NodeRefListBaseENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1016XalanObjectCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_22ClearCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectCache" = type { %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DeleteFunctor", %"class._ZTSN11xalanc_1_1022ClearCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::ClearCacheResetFunctor", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_18MutableNodeRefListENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1022ClearCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::ClearCacheResetFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_18MutableNodeRefListENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1028XPathExecutionContextDefault28ContextNodeListPositionCacheE.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" = type <{ ptr, i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17FormatterListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_11PrintWriterENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17XalanOutputStreamENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014VariablesStackE.xalanc_1_10::VariablesStack" = type { %"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack10StackEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64, i8, i64, %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack10StackEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" = type { %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" = type { %"class._ZTSN11xalanc_1_1011XalanHasherIPKNS_9XalanNodeEEE.xalanc_1_10::XalanHasher", %"struct._ZTSSt8equal_toIPKN11xalanc_1_109XalanNodeEE.std::equal_to", ptr, float, i64, i64, %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector", i64, i64 }
%"class._ZTSN11xalanc_1_1011XalanHasherIPKNS_9XalanNodeEEE.xalanc_1_10::XalanHasher" = type { i8 }
%"struct._ZTSSt8equal_toIPKN11xalanc_1_109XalanNodeEE.std::equal_to" = type { i8 }
%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" = type { %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" }
%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" = type { %"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1023XalanSourceTreeDocumentEE.std::pair" }
%"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1023XalanSourceTreeDocumentEE.std::pair" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemTemplateENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1024XResultTreeFragAllocatorE.xalanc_1_10::XResultTreeFragAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_15XResultTreeFragENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_15XResultTreeFragENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1040XalanSourceTreeDocumentFragmentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_31XalanSourceTreeDocumentFragmentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_31XalanSourceTreeDocumentFragmentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1032XalanSourceTreeDocumentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentAllocator" = type { %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::ReusableArenaAllocator" }
%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeDocumentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeDocumentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEEE.xalanc_1_10::XalanList" }
%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEEE.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" = type { %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DeleteFunctor", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheResetFunctor", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_18MutableNodeRefListENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64 }
%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_33StylesheetExecutionContextDefault16NodesToTransformENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_14XalanDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" = type { %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DeleteFunctor", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64 }
%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS2_EENS_13DeleteFunctorIS2_EENS_24DefaultCacheResetFunctorIS2_EEEE.xalanc_1_10::XalanObjectStackCache" = type { %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DeleteFunctor", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector", i64 }
%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_21FormatterToSourceTreeENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" = type { %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DeleteFunctor", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheResetFunctor", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_21FormatterToSourceTreeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector", i64 }
%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_21FormatterToSourceTreeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_26StylesheetExecutionContext22UseAttributeSetIndexesENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault19FormatNumberFunctorE.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" = type { ptr }
%"class._ZTSN11xalanc_1_1014VariablesStack10StackEntryE.xalanc_1_10::VariablesStack::StackEntry" = type { i32, ptr, %"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr", ptr, ptr }
%"struct._ZTSN11xalanc_1_1014VariablesStack17ParamsVectorEntryE.xalanc_1_10::VariablesStack::ParamsVectorEntry" = type { ptr, %"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr", ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES_IPKNS0_5XPathElEE.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"struct._ZTSSt4pairIPKN11xalanc_1_105XPathElE.std::pair" }
%"struct._ZTSSt4pairIPKN11xalanc_1_105XPathElE.std::pair" = type { ptr, i64 }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISC_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" = type { %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry", ptr, ptr }
%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" = type <{ ptr, i8, [7 x i8] }>
%"struct._ZTSSt4pairIKPKN11xalanc_1_109XalanNodeEPNS0_8KeyTableEE.std::pair" = type { ptr, ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_15XResultTreeFragEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_15XResultTreeFragEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_15XResultTreeFragEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" = type { ptr, ptr, ptr }
%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault16NodesToTransformE.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" = type <{ ptr, i32, [4 x i8] }>
%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault24FormatterToTextDOMStringE.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" = type { %"class._ZTSN11xalanc_1_1015FormatterToTextE.xalanc_1_10::FormatterToText.base", %"class._ZTSN11xalanc_1_1020DOMStringPrintWriterE.xalanc_1_10::DOMStringPrintWriter" }
%"class._ZTSN11xalanc_1_1015FormatterToTextE.xalanc_1_10::FormatterToText.base" = type <{ %"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener", ptr, i16, [6 x i8], %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", i8, i8, i8, [5 x i8], ptr, i32 }>
%"class._ZTSN11xalanc_1_1015XResultTreeFragE.xalanc_1_10::XResultTreeFrag" = type { %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject", ptr, ptr, ptr, %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", double }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ReusableArenaBlock" = type <{ %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ArenaBlockBase" = type { %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct._ZTSSt4pairIKPKN11xalanc_1_1010XalanQNameENS0_11XalanVectorIPNS0_16ElemAttributeSetENS0_31MemoryManagedConstructionTraitsIS7_EEEEE.std::pair" = type { ptr, %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class._ZTSN11xalanc_1_1014XalanNamespaceE.xalanc_1_10::XalanNamespace" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES1_E.std::pair" = type { %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" }
%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" }
%"class._ZTSSt9bad_alloc.std::bad_alloc" = type { %"class._ZTSSt9exception.std::exception" }
%"class._ZTSSt9exception.std::exception" = type { ptr }
%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct._ZTSSt4pairIKPKtPN11xalanc_1_1022XalanSourceTreeElementEE.std::pair" = type { ptr, ptr }

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_ = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv = comdat any

$_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv = comdat any

$_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_ = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv = comdat any

@_ZTISt9bad_alloc = external dso_local constant ptr
@_ZTVSt9bad_alloc = external dso_local unnamed_addr constant { [5 x ptr] }, align 8, !type !0, !type !1, !type !2, !type !3, !intel_dtrans_type !4
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !7
@_ZTVN11xalanc_1_1013XStringCachedE.0 = external hidden constant [19 x ptr], !type !8, !type !9, !type !10, !type !11, !type !12, !type !13, !type !14, !type !15, !type !16, !type !17, !type !18, !type !19, !type !20, !type !21, !type !22, !type !23, !type !24, !type !25, !type !26, !type !27, !type !28, !type !29, !type !30, !type !31, !type !32, !type !33, !type !34, !type !35, !type !36, !type !37, !type !38, !type !39, !type !40, !type !41, !type !42, !type !43, !type !44, !type !45, !type !46, !type !47, !type !48, !type !49, !type !50, !type !51, !type !52, !type !53, !type !54, !type !55, !type !56, !type !57, !type !58, !type !59, !type !60, !type !61, !type !62, !type !63, !type !64, !type !65, !type !66, !type !67, !type !68, !type !69, !type !70, !type !71, !intel_dtrans_type !72
@_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.0 = external hidden constant [8 x ptr], !type !73, !type !74, !type !75, !type !76, !type !77, !type !78, !type !79, !type !80, !type !81, !type !82, !intel_dtrans_type !83
@_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.0 = external hidden constant [8 x ptr], !type !73, !type !74, !type !75, !type !76, !type !77, !intel_dtrans_type !83

@_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE = hidden unnamed_addr alias void (ptr, ptr, ptr), ptr @_ZN11xalanc_1_1013XStringCachedC2ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree
declare !intel.dtrans.func.type !1062 dso_local "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2") local_unnamed_addr #0

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #0

; Function Attrs: nofree noinline noreturn nounwind
declare hidden void @__clang_call_terminate(ptr) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #2

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #3

; Function Attrs: nofree
declare !intel.dtrans.func.type !1063 dso_local noalias "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64) local_unnamed_addr #0

; Function Attrs: nofree noreturn
declare !intel.dtrans.func.type !1064 dso_local void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3") local_unnamed_addr #4

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !1065 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !1066 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare !intel.dtrans.func.type !1067 hidden void @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev(ptr nocapture noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1") unnamed_addr #7 align 2

; Function Attrs: uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1) unnamed_addr #8 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1069 !_Intel.Devirt.Target !1071 {
bb:
  %i = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %arg1) #20
          to label %bb9 unwind label %bb2

bb2:                                              ; preds = %bb
  %i3 = landingpad { ptr, i32 }
          catch ptr null
  %i4 = extractvalue { ptr, i32 } %i3, 0
  %i5 = tail call ptr @__cxa_begin_catch(ptr %i4) #21
  %i6 = tail call ptr @__cxa_allocate_exception(i64 1) #21
  invoke void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #22
          to label %bb14 unwind label %bb7

bb7:                                              ; preds = %bb2
  %i8 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb10 unwind label %bb11

bb9:                                              ; preds = %bb
  ret ptr %i

bb10:                                             ; preds = %bb7
  resume { ptr, i32 } %i8

bb11:                                             ; preds = %bb7
  %i12 = landingpad { ptr, i32 }
          catch ptr null
  %i13 = extractvalue { ptr, i32 } %i12, 0
  tail call void @__clang_call_terminate(ptr %i13) #23
  unreachable

bb14:                                             ; preds = %bb2
  unreachable
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) unnamed_addr #9 align 2 !intel.dtrans.func.type !1072 !_Intel.Devirt.Target !1071 {
bb:
  tail call void @_ZdlPv(ptr noundef %arg1) #21
  ret void
}

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1073 hidden noundef zeroext i1 @_ZN11xalanc_1_1033StylesheetExecutionContextDefault19releaseCachedStringERNS_14XalanDOMStringE(ptr noundef nonnull align 8 dereferenceable(2069) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(36) "intel_dtrans_func_index"="2") unnamed_addr #10 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1075 hidden noundef zeroext i1 @_ZN11xalanc_1_1028XPathExecutionContextDefault19releaseCachedStringERNS_14XalanDOMStringE(ptr noundef nonnull align 8 dereferenceable(432) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(36) "intel_dtrans_func_index"="2") unnamed_addr #10 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1077 hidden void @_ZN11xalanc_1_1011XStringBaseC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(64) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1079 hidden void @_ZN11xalanc_1_1011XStringBaseD2Ev(ptr noundef nonnull align 8 dereferenceable(64) "intel_dtrans_func_index"="1") unnamed_addr #12 align 2

; Function Attrs: uwtable
define hidden void @_ZN11xalanc_1_1013XStringCachedC2ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="2" %arg1, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %arg2) unnamed_addr #11 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1080 {
bb:
  tail call void @_ZN11xalanc_1_1011XStringBaseC2ERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(64) %arg, ptr noundef nonnull align 8 dereferenceable(8) %arg2)
  %i = getelementptr %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([19 x ptr], ptr @_ZTVN11xalanc_1_1013XStringCachedE.0, i32 0, i64 2), ptr %i, align 8
  %i3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg, i64 0, i32 1
  %i4 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %i3, i64 0, i32 0
  %i5 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %arg1, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8
  store ptr %i6, ptr %i4, align 8
  %i7 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %i3, i64 0, i32 1
  %i8 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %arg1, i64 0, i32 1
  %i9 = load ptr, ptr %i8, align 8
  store ptr %i9, ptr %i7, align 8
  store ptr null, ptr %i8, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr noundef nonnull align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg) unnamed_addr #12 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1082 {
bb:
  %i = getelementptr %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([19 x ptr], ptr @_ZTVN11xalanc_1_1013XStringCachedE.0, i32 0, i64 2), ptr %i, align 8
  %i1 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg, i64 0, i32 1
  %i2 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %i1, i64 0, i32 1
  %i3 = load ptr, ptr %i2, align 8
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb24, label %bb5

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %i1, i64 0, i32 0
  %i7 = load ptr, ptr %i6, align 8
  %i8 = getelementptr %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext", ptr %i7, i64 0, i32 0, i32 0
  %i9 = load ptr, ptr %i8, align 8
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xalanc_1_1021XPathExecutionContextE") #21
  tail call void @llvm.assume(i1 %i10) #21
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 24
  %i12 = load ptr, ptr %i11, align 8
  %i13 = bitcast ptr %i12 to ptr
  %i14 = bitcast ptr @_ZN11xalanc_1_1033StylesheetExecutionContextDefault19releaseCachedStringERNS_14XalanDOMStringE to ptr
  %i15 = icmp eq ptr %i13, %i14
  br i1 %i15, label %bb16, label %bb18

bb16:                                             ; preds = %bb5
  %i17 = invoke noundef zeroext i1 @_ZN11xalanc_1_1033StylesheetExecutionContextDefault19releaseCachedStringERNS_14XalanDOMStringE(ptr noundef nonnull align 8 dereferenceable(24) %i7, ptr noundef nonnull align 8 dereferenceable(36) %i3)
          to label %bb20 unwind label %bb21, !intel_dtrans_type !1083

bb18:                                             ; preds = %bb5
  %i19 = invoke noundef zeroext i1 @_ZN11xalanc_1_1028XPathExecutionContextDefault19releaseCachedStringERNS_14XalanDOMStringE(ptr noundef nonnull align 8 dereferenceable(24) %i7, ptr noundef nonnull align 8 dereferenceable(36) %i3)
          to label %bb20 unwind label %bb21, !intel_dtrans_type !1083

bb20:                                             ; preds = %bb18, %bb16
  br label %bb24

bb21:                                             ; preds = %bb18, %bb16
  %i22 = landingpad { ptr, i32 }
          catch ptr null
  %i23 = extractvalue { ptr, i32 } %i22, 0
  tail call void @__clang_call_terminate(ptr %i23) #23
  unreachable

bb24:                                             ; preds = %bb20, %bb
  tail call void @_ZN11xalanc_1_1011XStringBaseD2Ev(ptr noundef nonnull align 8 dereferenceable(64) %arg) #21
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
define hidden void @_ZN11xalanc_1_1022XStringCachedAllocatorC2ERN11xercesc_2_713MemoryManagerEt(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg1, i16 noundef zeroext %arg2) unnamed_addr #13 align 2 !intel.dtrans.func.type !1085 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %arg, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr noundef nonnull align 8 dereferenceable(41) %i, ptr noundef nonnull align 8 dereferenceable(8) %arg1, i16 noundef zeroext %arg2, i1 noundef zeroext false)
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
define hidden void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEC2ERN11xercesc_2_713MemoryManagerEtb(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %arg, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg1, i16 noundef zeroext %arg2, i1 noundef zeroext %arg3) unnamed_addr #13 comdat align 2 !intel.dtrans.func.type !1087 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 1
  store i16 %arg2, ptr %i, align 8
  %i4 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i5 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i4, i64 0, i32 0
  store ptr %arg1, ptr %i5, align 8
  %i6 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i4, i64 0, i32 1
  store ptr null, ptr %i6, align 8
  %i7 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i4, i64 0, i32 2
  store ptr null, ptr %i7, align 8
  %i8 = getelementptr %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([8 x ptr], ptr @_ZTVN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.0, i32 0, i64 2), ptr %i8, align 8
  %i9 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %arg, i64 0, i32 1
  store i8 0, ptr %i9, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg) unnamed_addr #14 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1089 {
bb:
  %i = getelementptr %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 0
  store ptr getelementptr inbounds ([8 x ptr], ptr @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.0, i32 0, i64 2), ptr %i, align 8
  %i1 = tail call i1 @llvm.type.test(ptr getelementptr inbounds ([8 x ptr], ptr @_ZTVN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.0, i32 0, i64 2), metadata !"_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE")
  tail call void @llvm.assume(i1 %i1)
  invoke void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr noundef nonnull align 8 dereferenceable(40) %arg)
          to label %bb2 unwind label %bb94, !intel_dtrans_type !1091

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i4 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i3, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, null
  br i1 %i6, label %bb93, label %bb7

bb7:                                              ; preds = %bb2
  %i8 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i5, i64 0, i32 2
  %i9 = load ptr, ptr %i8, align 8, !noalias !1093
  %i10 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i3, i64 0, i32 0
  br label %bb11

bb11:                                             ; preds = %bb51, %bb7
  %i12 = phi ptr [ %i9, %bb7 ], [ %i39, %bb51 ]
  %i13 = load ptr, ptr %i4, align 8, !noalias !1096
  %i14 = icmp eq ptr %i13, null
  br i1 %i14, label %bb15, label %bb34

bb15:                                             ; preds = %bb11
  %i16 = load ptr, ptr %i10, align 8, !noalias !1096
  %i17 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i16, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 8, !noalias !1096
  %i19 = tail call i1 @llvm.type.test(ptr %i18, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i19) #21
  %i20 = getelementptr inbounds ptr, ptr %i18, i64 2
  %i21 = load ptr, ptr %i20, align 8, !noalias !1096
  %i22 = bitcast ptr %i21 to ptr
  %i23 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i24 = icmp eq ptr %i22, %i23
  br i1 %i24, label %bb25, label %bb27

bb25:                                             ; preds = %bb15
  %i26 = invoke noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i16, i64 noundef 24)
          to label %bb29 unwind label %bb86, !intel_dtrans_type !1099

bb27:                                             ; preds = %bb15
  %i28 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i16, i64 noundef 24)
          to label %bb29 unwind label %bb86, !intel_dtrans_type !1099

bb29:                                             ; preds = %bb27, %bb25
  %i30 = phi ptr [ %i26, %bb25 ], [ %i28, %bb27 ]
  br label %bb31

bb31:                                             ; preds = %bb29
  store ptr %i30, ptr %i4, align 8, !noalias !1096
  %i32 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 2
  store ptr %i30, ptr %i32, align 8, !noalias !1096
  %i33 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 1
  store ptr %i30, ptr %i33, align 8, !noalias !1096
  br label %bb34

bb34:                                             ; preds = %bb31, %bb11
  %i35 = phi ptr [ %i30, %bb31 ], [ %i13, %bb11 ]
  %i36 = icmp eq ptr %i12, %i35
  br i1 %i36, label %bb52, label %bb37

bb37:                                             ; preds = %bb34
  %i38 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i12, i64 0, i32 2
  %i39 = load ptr, ptr %i38, align 8, !noalias !1100
  %i40 = load ptr, ptr %i10, align 8
  %i41 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i40, i64 0, i32 0
  %i42 = load ptr, ptr %i41, align 8
  %i43 = tail call i1 @llvm.type.test(ptr %i42, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i43) #21
  %i44 = getelementptr inbounds ptr, ptr %i42, i64 3
  %i45 = load ptr, ptr %i44, align 8
  %i46 = bitcast ptr %i45 to ptr
  %i47 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i48 = icmp eq ptr %i46, %i47
  br i1 %i48, label %bb49, label %bb50

bb49:                                             ; preds = %bb37
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i40, ptr noundef nonnull %i12)
          to label %bb51 unwind label %bb86, !llvm.loop !1103, !intel_dtrans_type !1105

bb50:                                             ; preds = %bb37
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i40, ptr noundef nonnull %i12)
          to label %bb51 unwind label %bb86, !llvm.loop !1103, !intel_dtrans_type !1105

bb51:                                             ; preds = %bb50, %bb49
  br label %bb11

bb52:                                             ; preds = %bb34
  %i53 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i3, i64 0, i32 2
  %i54 = load ptr, ptr %i53, align 8
  br label %bb55

bb55:                                             ; preds = %bb72, %bb52
  %i56 = phi ptr [ %i54, %bb52 ], [ %i63, %bb72 ]
  %i57 = icmp eq ptr %i56, null
  %i58 = load ptr, ptr %i10, align 8
  %i59 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i58, i64 0, i32 0
  %i60 = load ptr, ptr %i59, align 8
  br i1 %i57, label %bb73, label %bb61

bb61:                                             ; preds = %bb55
  %i62 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i56, i64 0, i32 2
  %i63 = load ptr, ptr %i62, align 8
  %i64 = tail call i1 @llvm.type.test(ptr %i60, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i64) #21
  %i65 = getelementptr inbounds ptr, ptr %i60, i64 3
  %i66 = load ptr, ptr %i65, align 8
  %i67 = bitcast ptr %i66 to ptr
  %i68 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i69 = icmp eq ptr %i67, %i68
  br i1 %i69, label %bb70, label %bb71

bb70:                                             ; preds = %bb61
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i58, ptr noundef nonnull %i56)
          to label %bb72 unwind label %bb84, !intel_dtrans_type !1105

bb71:                                             ; preds = %bb61
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i58, ptr noundef nonnull %i56)
          to label %bb72 unwind label %bb84, !intel_dtrans_type !1105

bb72:                                             ; preds = %bb71, %bb70
  br label %bb55

bb73:                                             ; preds = %bb55
  %i74 = load ptr, ptr %i4, align 8
  %i75 = tail call i1 @llvm.type.test(ptr %i60, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i75) #21
  %i76 = getelementptr inbounds ptr, ptr %i60, i64 3
  %i77 = load ptr, ptr %i76, align 8
  %i78 = bitcast ptr %i77 to ptr
  %i79 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i80 = icmp eq ptr %i78, %i79
  br i1 %i80, label %bb81, label %bb82

bb81:                                             ; preds = %bb73
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i58, ptr noundef %i74)
          to label %bb83 unwind label %bb88, !intel_dtrans_type !1105

bb82:                                             ; preds = %bb73
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i58, ptr noundef %i74)
          to label %bb83 unwind label %bb88, !intel_dtrans_type !1105

bb83:                                             ; preds = %bb82, %bb81
  br label %bb93

bb84:                                             ; preds = %bb71, %bb70
  %i85 = landingpad { ptr, i32 }
          catch ptr null
  br label %bb90

bb86:                                             ; preds = %bb50, %bb49, %bb27, %bb25
  %i87 = landingpad { ptr, i32 }
          catch ptr null
  br label %bb90

bb88:                                             ; preds = %bb82, %bb81
  %i89 = landingpad { ptr, i32 }
          catch ptr null
  br label %bb90

bb90:                                             ; preds = %bb88, %bb86, %bb84
  %i91 = phi { ptr, i32 } [ %i85, %bb84 ], [ %i87, %bb86 ], [ %i89, %bb88 ]
  %i92 = extractvalue { ptr, i32 } %i91, 0
  tail call void @__clang_call_terminate(ptr %i92) #23
  unreachable

bb93:                                             ; preds = %bb83, %bb2
  ret void

bb94:                                             ; preds = %bb
  %i95 = landingpad { ptr, i32 }
          catch ptr null
  %i96 = extractvalue { ptr, i32 } %i95, 0
  tail call void @__clang_call_terminate(ptr %i96) #23
  unreachable
}

; Function Attrs: uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr nocapture noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="2" %arg) unnamed_addr #15 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1106 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 1
  %i2 = load ptr, ptr %i1, align 8
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb4, label %bb24

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8
  %i7 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i6, i64 0, i32 0
  %i8 = load ptr, ptr %i7, align 8
  %i9 = tail call i1 @llvm.type.test(ptr %i8, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i9)
  %i10 = getelementptr inbounds ptr, ptr %i8, i64 2
  %i11 = load ptr, ptr %i10, align 8
  %i12 = bitcast ptr %i11 to ptr
  %i13 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i14 = icmp eq ptr %i12, %i13
  br i1 %i14, label %bb15, label %bb17

bb15:                                             ; preds = %bb4
  %i16 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i6, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb19

bb17:                                             ; preds = %bb4
  %i18 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i6, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb19

bb19:                                             ; preds = %bb17, %bb15
  %i20 = phi ptr [ %i16, %bb15 ], [ %i18, %bb17 ]
  br label %bb21

bb21:                                             ; preds = %bb19
  store ptr %i20, ptr %i1, align 8
  %i22 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i20, i64 0, i32 2
  store ptr %i20, ptr %i22, align 8
  %i23 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i20, i64 0, i32 1
  store ptr %i20, ptr %i23, align 8
  br label %bb36

bb24:                                             ; preds = %bb
  %i25 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i2, i64 0, i32 2
  %i26 = load ptr, ptr %i25, align 8
  %i27 = icmp eq ptr %i26, %i2
  br i1 %i27, label %bb36, label %bb28

bb28:                                             ; preds = %bb24
  %i29 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i26, i64 0, i32 0
  %i30 = load ptr, ptr %i29, align 8
  %i31 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i30, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 2
  %i33 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i30, i64 0, i32 1
  %i34 = load i16, ptr %i33, align 8
  %i35 = icmp ult i16 %i34, %i32
  br i1 %i35, label %bb172, label %bb36

bb36:                                             ; preds = %bb28, %bb24, %bb21
  %i37 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) %arg)
  %i38 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 1
  %i39 = load i16, ptr %i38, align 8
  %i40 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i37, i64 0, i32 0
  %i41 = load ptr, ptr %i40, align 8
  %i42 = tail call i1 @llvm.type.test(ptr %i41, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i42)
  %i43 = getelementptr inbounds ptr, ptr %i41, i64 2
  %i44 = load ptr, ptr %i43, align 8
  %i45 = bitcast ptr %i44 to ptr
  %i46 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i47 = icmp eq ptr %i45, %i46
  br i1 %i47, label %bb48, label %bb50

bb48:                                             ; preds = %bb36
  %i49 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i37, i64 noundef 32), !intel_dtrans_type !1099
  br label %bb52

bb50:                                             ; preds = %bb36
  %i51 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i37, i64 noundef 32), !intel_dtrans_type !1099
  br label %bb52

bb52:                                             ; preds = %bb50, %bb48
  %i53 = phi ptr [ %i49, %bb48 ], [ %i51, %bb50 ]
  br label %bb54

bb54:                                             ; preds = %bb52
  %i55 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i53, i64 0, i32 0
  %i56 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", ptr %i55, i64 0, i32 0
  store ptr %i37, ptr %i56, align 8
  %i57 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i53, i64 0, i32 1
  store i16 0, ptr %i57, align 8
  %i58 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i53, i64 0, i32 2
  store i16 %i39, ptr %i58, align 2
  %i59 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i37, i64 0, i32 0
  %i60 = load ptr, ptr %i59, align 8
  %i61 = tail call i1 @llvm.type.test(ptr %i60, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i61)
  %i62 = zext i16 %i39 to i64
  %i63 = mul nuw nsw i64 %i62, 80
  %i64 = getelementptr inbounds ptr, ptr %i60, i64 2
  %i65 = load ptr, ptr %i64, align 8
  %i66 = bitcast ptr %i65 to ptr
  %i67 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i68 = icmp eq ptr %i66, %i67
  br i1 %i68, label %bb69, label %bb71

bb69:                                             ; preds = %bb54
  %i70 = invoke noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i37, i64 noundef %i63)
          to label %bb73 unwind label %bb91, !intel_dtrans_type !1099

bb71:                                             ; preds = %bb54
  %i72 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i37, i64 noundef %i63)
          to label %bb73 unwind label %bb91, !intel_dtrans_type !1099

bb73:                                             ; preds = %bb71, %bb69
  %i74 = phi ptr [ %i70, %bb69 ], [ %i72, %bb71 ]
  br label %bb75

bb75:                                             ; preds = %bb73
  %i76 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i53, i64 0, i32 3
  store ptr %i74, ptr %i76, align 8
  %i77 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i53, i64 0, i32 1
  store i16 0, ptr %i77, align 8
  %i78 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i53, i64 0, i32 2
  store i16 0, ptr %i78, align 2
  %i79 = load i16, ptr %i58, align 2
  %i80 = icmp eq i16 %i79, 0
  br i1 %i80, label %bb108, label %bb81

bb81:                                             ; preds = %bb75
  %i82 = zext i16 %i79 to i64
  br label %bb83

bb83:                                             ; preds = %bb83, %bb81
  %i84 = phi i64 [ 0, %bb81 ], [ %i86, %bb83 ]
  %i85 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i74, i64 %i84
  %i86 = add nuw nsw i64 %i84, 1
  %i87 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i85, i64 0, i32 0
  %i88 = trunc i64 %i86 to i16
  store i16 %i88, ptr %i87, align 4
  %i89 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i85, i64 0, i32 1
  store i32 -2228259, ptr %i89, align 4
  %i90 = icmp eq i64 %i86, %i82
  br i1 %i90, label %bb108, label %bb83, !llvm.loop !1107

bb91:                                             ; preds = %bb71, %bb69
  %i92 = landingpad { ptr, i32 }
          cleanup
  %i93 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i37, i64 0, i32 0
  %i94 = load ptr, ptr %i93, align 8
  %i95 = tail call i1 @llvm.type.test(ptr %i94, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i95) #21
  %i96 = getelementptr inbounds ptr, ptr %i94, i64 3
  %i97 = load ptr, ptr %i96, align 8
  %i98 = bitcast ptr %i97 to ptr
  %i99 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i100 = icmp eq ptr %i98, %i99
  br i1 %i100, label %bb101, label %bb102

bb101:                                            ; preds = %bb91
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef nonnull %i53)
          to label %bb103 unwind label %bb104, !intel_dtrans_type !1105

bb102:                                            ; preds = %bb91
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef nonnull %i53)
          to label %bb103 unwind label %bb104, !intel_dtrans_type !1105

bb103:                                            ; preds = %bb102, %bb101
  br label %bb107

bb104:                                            ; preds = %bb102, %bb101
  %i105 = landingpad { ptr, i32 }
          catch ptr null
  %i106 = extractvalue { ptr, i32 } %i105, 0
  tail call void @__clang_call_terminate(ptr %i106) #23
  unreachable

bb107:                                            ; preds = %bb103
  resume { ptr, i32 } %i92

bb108:                                            ; preds = %bb83, %bb75
  %i109 = load ptr, ptr %i1, align 8, !noalias !1108
  %i110 = icmp eq ptr %i109, null
  br i1 %i110, label %bb114, label %bb111

bb111:                                            ; preds = %bb108
  %i112 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i109, i64 0, i32 2
  %i113 = load ptr, ptr %i112, align 8, !noalias !1108
  br label %bb134

bb114:                                            ; preds = %bb108
  %i115 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i116 = load ptr, ptr %i115, align 8, !noalias !1108
  %i117 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i116, i64 0, i32 0
  %i118 = load ptr, ptr %i117, align 8, !noalias !1108
  %i119 = tail call i1 @llvm.type.test(ptr %i118, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i119)
  %i120 = getelementptr inbounds ptr, ptr %i118, i64 2
  %i121 = load ptr, ptr %i120, align 8, !noalias !1108
  %i122 = bitcast ptr %i121 to ptr
  %i123 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i124 = icmp eq ptr %i122, %i123
  br i1 %i124, label %bb125, label %bb127

bb125:                                            ; preds = %bb114
  %i126 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i116, i64 noundef 24), !noalias !1108, !intel_dtrans_type !1099
  br label %bb129

bb127:                                            ; preds = %bb114
  %i128 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i116, i64 noundef 24), !noalias !1108, !intel_dtrans_type !1099
  br label %bb129

bb129:                                            ; preds = %bb127, %bb125
  %i130 = phi ptr [ %i126, %bb125 ], [ %i128, %bb127 ]
  br label %bb131

bb131:                                            ; preds = %bb129
  store ptr %i130, ptr %i1, align 8, !noalias !1108
  %i132 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i130, i64 0, i32 2
  store ptr %i130, ptr %i132, align 8, !noalias !1108
  %i133 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i130, i64 0, i32 1
  store ptr %i130, ptr %i133, align 8, !noalias !1108
  br label %bb134

bb134:                                            ; preds = %bb131, %bb111
  %i135 = phi ptr [ %i130, %bb131 ], [ %i113, %bb111 ]
  %i136 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i137 = load ptr, ptr %i136, align 8
  %i138 = icmp eq ptr %i137, null
  br i1 %i138, label %bb142, label %bb139

bb139:                                            ; preds = %bb134
  %i140 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i137, i64 0, i32 2
  %i141 = load ptr, ptr %i140, align 8
  br label %bb160

bb142:                                            ; preds = %bb134
  %i143 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i144 = load ptr, ptr %i143, align 8
  %i145 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i144, i64 0, i32 0
  %i146 = load ptr, ptr %i145, align 8
  %i147 = tail call i1 @llvm.type.test(ptr %i146, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i147)
  %i148 = getelementptr inbounds ptr, ptr %i146, i64 2
  %i149 = load ptr, ptr %i148, align 8
  %i150 = bitcast ptr %i149 to ptr
  %i151 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i152 = icmp eq ptr %i150, %i151
  br i1 %i152, label %bb153, label %bb155

bb153:                                            ; preds = %bb142
  %i154 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i144, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb157

bb155:                                            ; preds = %bb142
  %i156 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i144, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb157

bb157:                                            ; preds = %bb155, %bb153
  %i158 = phi ptr [ %i154, %bb153 ], [ %i156, %bb155 ]
  br label %bb159

bb159:                                            ; preds = %bb157
  br label %bb160

bb160:                                            ; preds = %bb159, %bb139
  %i161 = phi ptr [ %i137, %bb139 ], [ %i158, %bb159 ]
  %i162 = phi ptr [ %i141, %bb139 ], [ null, %bb159 ]
  %i163 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i161, i64 0, i32 0
  store ptr %i53, ptr %i163, align 8
  %i164 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i161, i64 0, i32 1
  %i165 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i135, i64 0, i32 1
  %i166 = load ptr, ptr %i165, align 8
  store ptr %i166, ptr %i164, align 8
  %i167 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i161, i64 0, i32 2
  store ptr %i135, ptr %i167, align 8
  %i168 = load ptr, ptr %i165, align 8
  %i169 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i168, i64 0, i32 2
  store ptr %i161, ptr %i169, align 8
  store ptr %i161, ptr %i165, align 8
  store ptr %i162, ptr %i136, align 8
  %i170 = load ptr, ptr %i1, align 8, !noalias !1111
  %i171 = icmp eq ptr %i170, null
  br i1 %i171, label %bb176, label %bb172

bb172:                                            ; preds = %bb160, %bb28
  %i173 = phi ptr [ %i170, %bb160 ], [ %i2, %bb28 ]
  %i174 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i173, i64 0, i32 2
  %i175 = load ptr, ptr %i174, align 8, !noalias !1111
  br label %bb196

bb176:                                            ; preds = %bb160
  %i177 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i178 = load ptr, ptr %i177, align 8, !noalias !1111
  %i179 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i178, i64 0, i32 0
  %i180 = load ptr, ptr %i179, align 8, !noalias !1111
  %i181 = tail call i1 @llvm.type.test(ptr %i180, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i181)
  %i182 = getelementptr inbounds ptr, ptr %i180, i64 2
  %i183 = load ptr, ptr %i182, align 8, !noalias !1111
  %i184 = bitcast ptr %i183 to ptr
  %i185 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i186 = icmp eq ptr %i184, %i185
  br i1 %i186, label %bb187, label %bb189

bb187:                                            ; preds = %bb176
  %i188 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i178, i64 noundef 24), !noalias !1111, !intel_dtrans_type !1099
  br label %bb191

bb189:                                            ; preds = %bb176
  %i190 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i178, i64 noundef 24), !noalias !1111, !intel_dtrans_type !1099
  br label %bb191

bb191:                                            ; preds = %bb189, %bb187
  %i192 = phi ptr [ %i188, %bb187 ], [ %i190, %bb189 ]
  br label %bb193

bb193:                                            ; preds = %bb191
  store ptr %i192, ptr %i1, align 8, !noalias !1111
  %i194 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i192, i64 0, i32 2
  store ptr %i192, ptr %i194, align 8, !noalias !1111
  %i195 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i192, i64 0, i32 1
  store ptr %i192, ptr %i195, align 8, !noalias !1111
  br label %bb196

bb196:                                            ; preds = %bb193, %bb172
  %i197 = phi ptr [ %i192, %bb193 ], [ %i175, %bb172 ]
  %i198 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i197, i64 0, i32 0
  %i199 = load ptr, ptr %i198, align 8
  %i200 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i199, i64 0, i32 1
  %i201 = load i16, ptr %i200, align 8
  %i202 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i199, i64 0, i32 2
  %i203 = load i16, ptr %i202, align 2
  %i204 = icmp eq i16 %i201, %i203
  br i1 %i204, label %bb221, label %bb205

bb205:                                            ; preds = %bb196
  %i206 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i199, i64 0, i32 1
  %i207 = load i16, ptr %i206, align 8
  %i208 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i199, i64 0, i32 2
  %i209 = load i16, ptr %i208, align 2
  %i210 = icmp eq i16 %i207, %i209
  %i211 = zext i16 %i207 to i64
  %i212 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i199, i64 0, i32 3
  %i213 = load ptr, ptr %i212, align 8
  br i1 %i210, label %bb216, label %bb214

bb214:                                            ; preds = %bb205
  %i215 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i213, i64 %i211
  br label %bb221

bb216:                                            ; preds = %bb205
  %i217 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i213, i64 %i211
  %i218 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i217, i64 0, i32 0
  %i219 = load i16, ptr %i218, align 4
  store i16 %i219, ptr %i208, align 2
  %i220 = add i16 %i201, 1
  store i16 %i220, ptr %i200, align 8
  br label %bb221

bb221:                                            ; preds = %bb216, %bb214, %bb196
  %i222 = phi ptr [ null, %bb196 ], [ %i215, %bb214 ], [ %i217, %bb216 ]
  ret ptr %i222
}

; Function Attrs: uwtable
define hidden void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr nocapture noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %arg1) unnamed_addr #15 comdat align 2 !intel.dtrans.func.type !1114 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i2 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 1
  %i3 = load ptr, ptr %i2, align 8, !noalias !1115
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb8, label %bb5

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i3, i64 0, i32 2
  %i7 = load ptr, ptr %i6, align 8, !noalias !1115
  br label %bb28

bb8:                                              ; preds = %bb
  %i9 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i10 = load ptr, ptr %i9, align 8, !noalias !1115
  %i11 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i12 = load ptr, ptr %i11, align 8, !noalias !1115
  %i13 = tail call i1 @llvm.type.test(ptr %i12, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i13)
  %i14 = getelementptr inbounds ptr, ptr %i12, i64 2
  %i15 = load ptr, ptr %i14, align 8, !noalias !1115
  %i16 = bitcast ptr %i15 to ptr
  %i17 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i18 = icmp eq ptr %i16, %i17
  br i1 %i18, label %bb19, label %bb21

bb19:                                             ; preds = %bb8
  %i20 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i10, i64 noundef 24), !noalias !1115, !intel_dtrans_type !1099
  br label %bb23

bb21:                                             ; preds = %bb8
  %i22 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i10, i64 noundef 24), !noalias !1115, !intel_dtrans_type !1099
  br label %bb23

bb23:                                             ; preds = %bb21, %bb19
  %i24 = phi ptr [ %i20, %bb19 ], [ %i22, %bb21 ]
  br label %bb25

bb25:                                             ; preds = %bb23
  store ptr %i24, ptr %i2, align 8, !noalias !1115
  %i26 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i24, i64 0, i32 2
  store ptr %i24, ptr %i26, align 8, !noalias !1115
  %i27 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i24, i64 0, i32 1
  store ptr %i24, ptr %i27, align 8, !noalias !1115
  br label %bb28

bb28:                                             ; preds = %bb25, %bb5
  %i29 = phi ptr [ %i24, %bb25 ], [ %i3, %bb5 ]
  %i30 = phi ptr [ %i24, %bb25 ], [ %i7, %bb5 ]
  %i31 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 0
  %i32 = load ptr, ptr %i31, align 8
  %i33 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i32, i64 0, i32 2
  %i34 = load i16, ptr %i33, align 2
  %i35 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i32, i64 0, i32 1
  store i16 %i34, ptr %i35, align 8
  %i36 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 1
  %i37 = load i16, ptr %i36, align 8
  %i38 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 2
  %i39 = load i16, ptr %i38, align 2
  %i40 = icmp ult i16 %i37, %i39
  br i1 %i40, label %bb59, label %bb41

bb41:                                             ; preds = %bb28
  %i42 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i29, i64 0, i32 2
  %i43 = load ptr, ptr %i42, align 8, !noalias !1118
  %i44 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i43, i64 0, i32 2
  %i45 = load ptr, ptr %i44, align 8
  %i46 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i43, i64 0, i32 1
  %i47 = load ptr, ptr %i46, align 8
  %i48 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i43, i64 0, i32 0
  %i49 = load ptr, ptr %i48, align 8
  %i50 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i47, i64 0, i32 2
  store ptr %i45, ptr %i50, align 8
  %i51 = load ptr, ptr %i44, align 8
  %i52 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i51, i64 0, i32 1
  store ptr %i47, ptr %i52, align 8
  store ptr null, ptr %i46, align 8
  %i53 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i54 = load ptr, ptr %i53, align 8
  store ptr %i49, ptr %i48, align 8
  %i55 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i29, i64 0, i32 1
  %i56 = load ptr, ptr %i55, align 8
  store ptr %i56, ptr %i46, align 8
  store ptr %i29, ptr %i44, align 8
  %i57 = load ptr, ptr %i55, align 8
  %i58 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i57, i64 0, i32 2
  store ptr %i43, ptr %i58, align 8
  store ptr %i43, ptr %i55, align 8
  store ptr %i54, ptr %i53, align 8
  br label %bb59

bb59:                                             ; preds = %bb41, %bb28
  ret void
}

; Function Attrs: uwtable
define hidden void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg) unnamed_addr #15 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1121 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 1
  %i2 = load ptr, ptr %i1, align 8, !noalias !1122
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb4, label %bb24

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !noalias !1123
  %i7 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i6, i64 0, i32 0
  %i8 = load ptr, ptr %i7, align 8, !noalias !1123
  %i9 = tail call i1 @llvm.type.test(ptr %i8, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i9)
  %i10 = getelementptr inbounds ptr, ptr %i8, i64 2
  %i11 = load ptr, ptr %i10, align 8, !noalias !1123
  %i12 = bitcast ptr %i11 to ptr
  %i13 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i14 = icmp eq ptr %i12, %i13
  br i1 %i14, label %bb15, label %bb17

bb15:                                             ; preds = %bb4
  %i16 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i6, i64 noundef 24), !noalias !1123, !intel_dtrans_type !1099
  br label %bb19

bb17:                                             ; preds = %bb4
  %i18 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i6, i64 noundef 24), !noalias !1123, !intel_dtrans_type !1099
  br label %bb19

bb19:                                             ; preds = %bb17, %bb15
  %i20 = phi ptr [ %i16, %bb15 ], [ %i18, %bb17 ]
  br label %bb21

bb21:                                             ; preds = %bb19
  store ptr %i20, ptr %i1, align 8, !noalias !1123
  %i22 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i20, i64 0, i32 2
  store ptr %i20, ptr %i22, align 8, !noalias !1123
  %i23 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i20, i64 0, i32 1
  store ptr %i20, ptr %i23, align 8, !noalias !1123
  br label %bb137

bb24:                                             ; preds = %bb
  %i25 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i2, i64 0, i32 2
  %i26 = load ptr, ptr %i25, align 8, !noalias !1123
  %i27 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i28 = load ptr, ptr %i27, align 8
  %i29 = icmp eq ptr %i26, %i2
  br i1 %i29, label %bb137, label %bb30

bb30:                                             ; preds = %bb110, %bb24
  %i31 = phi ptr [ %i112, %bb110 ], [ %i26, %bb24 ]
  %i32 = getelementptr %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i31, i64 0, i32 0
  %i33 = load ptr, ptr %i32, align 8
  %i34 = icmp eq ptr %i33, null
  br i1 %i34, label %bb110, label %bb35

bb35:                                             ; preds = %bb30
  %i36 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i33, i64 0, i32 1
  %i37 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i33, i64 0, i32 2
  %i38 = load i16, ptr %i37, align 2
  %i39 = icmp eq i16 %i38, 0
  br i1 %i39, label %bb48, label %bb40

bb40:                                             ; preds = %bb35
  %i41 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i33, i64 0, i32 3
  br label %bb42

bb42:                                             ; preds = %bb91, %bb40
  %i43 = phi i16 [ %i38, %bb40 ], [ %i93, %bb91 ]
  %i44 = phi i64 [ 0, %bb40 ], [ %i95, %bb91 ]
  %i45 = phi i16 [ 0, %bb40 ], [ %i94, %bb91 ]
  %i46 = load i16, ptr %i36, align 8
  %i47 = icmp ult i16 %i45, %i46
  br i1 %i47, label %bb70, label %bb48

bb48:                                             ; preds = %bb91, %bb42, %bb35
  %i49 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i33, i64 0, i32 3
  %i50 = load ptr, ptr %i49, align 8
  %i51 = icmp eq ptr %i50, null
  br i1 %i51, label %bb97, label %bb52

bb52:                                             ; preds = %bb48
  %i53 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i33, i64 0, i32 0
  %i54 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator", ptr %i53, i64 0, i32 0
  %i55 = load ptr, ptr %i54, align 8
  %i56 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i55, i64 0, i32 0
  %i57 = load ptr, ptr %i56, align 8
  %i58 = tail call i1 @llvm.type.test(ptr %i57, metadata !"_ZTSN11xercesc_2_713MemoryManagerE") #21
  tail call void @llvm.assume(i1 %i58) #21
  %i59 = getelementptr inbounds ptr, ptr %i57, i64 3
  %i60 = load ptr, ptr %i59, align 8
  %i61 = bitcast ptr %i60 to ptr
  %i62 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i63 = icmp eq ptr %i61, %i62
  br i1 %i63, label %bb64, label %bb65

bb64:                                             ; preds = %bb52
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i55, ptr noundef nonnull %i50)
          to label %bb66 unwind label %bb67, !intel_dtrans_type !1105

bb65:                                             ; preds = %bb52
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i55, ptr noundef nonnull %i50)
          to label %bb66 unwind label %bb67, !intel_dtrans_type !1105

bb66:                                             ; preds = %bb65, %bb64
  br label %bb97

bb67:                                             ; preds = %bb65, %bb64
  %i68 = landingpad { ptr, i32 }
          catch ptr null
  %i69 = extractvalue { ptr, i32 } %i68, 0
  tail call void @__clang_call_terminate(ptr %i69) #23
  unreachable

bb70:                                             ; preds = %bb42
  %i71 = load ptr, ptr %i41, align 8
  %i72 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i71, i64 %i44
  %i73 = zext i16 %i43 to i64
  %i74 = icmp ult i64 %i44, %i73
  br i1 %i74, label %bb75, label %bb83

bb75:                                             ; preds = %bb70
  %i76 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i72, i64 0, i32 1
  %i77 = load i32, ptr %i76, align 4
  %i78 = icmp ne i32 %i77, -2228259
  %i79 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i72, i64 0, i32 0
  %i80 = load i16, ptr %i79, align 4
  %i81 = icmp ugt i16 %i80, %i43
  %i82 = select i1 %i78, i1 true, i1 %i81
  br i1 %i82, label %bb83, label %bb91

bb83:                                             ; preds = %bb75, %bb70
  %i84 = getelementptr %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i72, i64 0, i32 0, i32 0, i32 0, i32 0
  %i85 = load ptr, ptr %i84, align 8
  %i86 = tail call i1 @llvm.type.test(ptr %i85, metadata !"_ZTSN11xalanc_1_1013XStringCachedE") #21
  tail call void @llvm.assume(i1 %i86) #21
  %i87 = load ptr, ptr %i85, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr noundef nonnull align 8 dereferenceable(80) %i72) #21, !intel_dtrans_type !1126
  %i88 = add nuw i16 %i45, 1
  %i89 = load i16, ptr %i37, align 2
  %i90 = zext i16 %i89 to i64
  br label %bb91

bb91:                                             ; preds = %bb83, %bb75
  %i92 = phi i64 [ %i90, %bb83 ], [ %i73, %bb75 ]
  %i93 = phi i16 [ %i89, %bb83 ], [ %i43, %bb75 ]
  %i94 = phi i16 [ %i88, %bb83 ], [ %i45, %bb75 ]
  %i95 = add nuw nsw i64 %i44, 1
  %i96 = icmp ult i64 %i95, %i92
  br i1 %i96, label %bb42, label %bb48, !llvm.loop !1127

bb97:                                             ; preds = %bb66, %bb48
  %i98 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i28, i64 0, i32 0
  %i99 = load ptr, ptr %i98, align 8
  %i100 = tail call i1 @llvm.type.test(ptr %i99, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i100)
  %i101 = getelementptr inbounds ptr, ptr %i99, i64 3
  %i102 = load ptr, ptr %i101, align 8
  %i103 = bitcast ptr %i102 to ptr
  %i104 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv to ptr
  %i105 = icmp eq ptr %i103, %i104
  br i1 %i105, label %bb106, label %bb107

bb106:                                            ; preds = %bb97
  tail call void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i28, ptr noundef nonnull %i33), !intel_dtrans_type !1105
  br label %bb108

bb107:                                            ; preds = %bb97
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i28, ptr noundef nonnull %i33), !intel_dtrans_type !1105
  br label %bb108

bb108:                                            ; preds = %bb107, %bb106
  br label %bb109

bb109:                                            ; preds = %bb108
  br label %bb110

bb110:                                            ; preds = %bb109, %bb30
  %i111 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i31, i64 0, i32 2
  %i112 = load ptr, ptr %i111, align 8, !noalias !1128
  %i113 = icmp eq ptr %i112, %i2
  br i1 %i113, label %bb114, label %bb30, !llvm.loop !1131

bb114:                                            ; preds = %bb110
  %i115 = load ptr, ptr %i1, align 8, !noalias !1132
  %i116 = icmp eq ptr %i115, null
  br i1 %i116, label %bb117, label %bb137

bb117:                                            ; preds = %bb114
  %i118 = getelementptr %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i119 = load ptr, ptr %i118, align 8, !noalias !1132
  %i120 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i119, i64 0, i32 0
  %i121 = load ptr, ptr %i120, align 8, !noalias !1132
  %i122 = tail call i1 @llvm.type.test(ptr %i121, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i122)
  %i123 = getelementptr inbounds ptr, ptr %i121, i64 2
  %i124 = load ptr, ptr %i123, align 8, !noalias !1132
  %i125 = bitcast ptr %i124 to ptr
  %i126 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i127 = icmp eq ptr %i125, %i126
  br i1 %i127, label %bb128, label %bb130

bb128:                                            ; preds = %bb117
  %i129 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i119, i64 noundef 24), !noalias !1132, !intel_dtrans_type !1099
  br label %bb132

bb130:                                            ; preds = %bb117
  %i131 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i119, i64 noundef 24), !noalias !1132, !intel_dtrans_type !1099
  br label %bb132

bb132:                                            ; preds = %bb130, %bb128
  %i133 = phi ptr [ %i129, %bb128 ], [ %i131, %bb130 ]
  br label %bb134

bb134:                                            ; preds = %bb132
  store ptr %i133, ptr %i1, align 8, !noalias !1132
  %i135 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i133, i64 0, i32 2
  store ptr %i133, ptr %i135, align 8, !noalias !1132
  %i136 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i133, i64 0, i32 1
  store ptr %i133, ptr %i136, align 8, !noalias !1132
  br label %bb157

bb137:                                            ; preds = %bb114, %bb24, %bb21
  %i138 = phi ptr [ %i115, %bb114 ], [ %i2, %bb24 ], [ %i20, %bb21 ]
  %i139 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i138, i64 0, i32 2
  %i140 = load ptr, ptr %i139, align 8, !noalias !1132
  %i141 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i142 = icmp eq ptr %i140, %i138
  br i1 %i142, label %bb157, label %bb143

bb143:                                            ; preds = %bb137
  %i144 = load ptr, ptr %i141, align 8
  br label %bb145

bb145:                                            ; preds = %bb145, %bb143
  %i146 = phi ptr [ %i144, %bb143 ], [ %i147, %bb145 ]
  %i147 = phi ptr [ %i140, %bb143 ], [ %i149, %bb145 ]
  %i148 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i147, i64 0, i32 2
  %i149 = load ptr, ptr %i148, align 8, !noalias !1135
  %i150 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i147, i64 0, i32 1
  %i151 = load ptr, ptr %i150, align 8
  %i152 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i151, i64 0, i32 2
  store ptr %i149, ptr %i152, align 8
  %i153 = load ptr, ptr %i148, align 8
  %i154 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i153, i64 0, i32 1
  store ptr %i151, ptr %i154, align 8
  store ptr null, ptr %i150, align 8
  store ptr %i146, ptr %i148, align 8
  %i155 = icmp eq ptr %i149, %i138
  br i1 %i155, label %bb156, label %bb145, !llvm.loop !1138

bb156:                                            ; preds = %bb145
  store ptr %i147, ptr %i141, align 8
  br label %bb157

bb157:                                            ; preds = %bb156, %bb137, %bb134
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define hidden noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %arg) #16 comdat align 2 !intel.dtrans.func.type !1139 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i2 = load ptr, ptr %i1, align 8
  ret ptr %i2
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xalanc_1_1022XStringCachedAllocatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) unnamed_addr #14 align 2 !intel.dtrans.func.type !1140 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %arg, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEED2Ev(ptr noundef nonnull align 8 dereferenceable(41) %i) #21
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1022XStringCachedAllocator12createStringERNS_21XPathExecutionContext25GetAndReleaseCachedStringE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %arg, ptr noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %arg1) #17 align 2 !intel.dtrans.func.type !1141 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %arg, i64 0, i32 0
  %i2 = tail call noundef ptr @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13allocateBlockEv(ptr noundef nonnull align 8 dereferenceable(41) %i)
  %i3 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE16getMemoryManagerEv(ptr noundef nonnull align 8 dereferenceable(40) %i)
  tail call void @_ZN11xalanc_1_1013XStringCachedC1ERNS_21XPathExecutionContext25GetAndReleaseCachedStringERN11xercesc_2_713MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(80) %i2, ptr noundef nonnull align 8 dereferenceable(16) %arg1, ptr noundef nonnull align 8 dereferenceable(8) %i3)
  tail call void @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE16commitAllocationEPS1_(ptr noundef nonnull align 8 dereferenceable(41) %i, ptr noundef nonnull %i2)
  ret ptr %i2
}

; Function Attrs: mustprogress uwtable
define hidden noundef zeroext i1 @_ZN11xalanc_1_1022XStringCachedAllocator7destroyEPNS_13XStringCachedE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) #17 align 2 !intel.dtrans.func.type !1142 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %arg, i64 0, i32 0
  %i2 = tail call noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr noundef nonnull align 8 dereferenceable(41) %i, ptr noundef %arg1)
  ret i1 %i2
}

; Function Attrs: uwtable
define hidden noundef zeroext i1 @_ZN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEE13destroyObjectEPS1_(ptr nocapture noundef nonnull align 8 dereferenceable(41) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) #15 comdat align 2 !intel.dtrans.func.type !1143 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator", ptr %arg, i64 0, i32 2
  %i2 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 1
  %i3 = load ptr, ptr %i2, align 8
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb5, label %bb25

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i7 = load ptr, ptr %i6, align 8
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i7, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 2
  %i12 = load ptr, ptr %i11, align 8
  %i13 = bitcast ptr %i12 to ptr
  %i14 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i15 = icmp eq ptr %i13, %i14
  br i1 %i15, label %bb16, label %bb18

bb16:                                             ; preds = %bb5
  %i17 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i7, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb20

bb18:                                             ; preds = %bb5
  %i19 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i7, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb20

bb20:                                             ; preds = %bb18, %bb16
  %i21 = phi ptr [ %i17, %bb16 ], [ %i19, %bb18 ]
  br label %bb22

bb22:                                             ; preds = %bb20
  store ptr %i21, ptr %i2, align 8
  %i23 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i21, i64 0, i32 2
  store ptr %i21, ptr %i23, align 8
  %i24 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i21, i64 0, i32 1
  store ptr %i21, ptr %i24, align 8
  br label %bb475

bb25:                                             ; preds = %bb
  %i26 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i3, i64 0, i32 2
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, %i3
  br i1 %i28, label %bb475, label %bb29

bb29:                                             ; preds = %bb232, %bb25
  %i30 = phi ptr [ %i234, %bb232 ], [ %i27, %bb25 ]
  %i31 = getelementptr %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 0
  %i32 = load ptr, ptr %i31, align 8
  %i33 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 1
  %i34 = load i16, ptr %i33, align 8
  %i35 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 2
  %i36 = load i16, ptr %i35, align 2
  %i37 = icmp ult i16 %i34, %i36
  br i1 %i37, label %bb38, label %bb236

bb38:                                             ; preds = %bb29
  %i39 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 3
  %i40 = load ptr, ptr %i39, align 8
  %i41 = icmp ugt ptr %i40, %arg1
  br i1 %i41, label %bb232, label %bb42

bb42:                                             ; preds = %bb38
  %i43 = zext i16 %i36 to i64
  %i44 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i40, i64 %i43
  %i45 = icmp ugt ptr %i44, %arg1
  br i1 %i45, label %bb46, label %bb232

bb46:                                             ; preds = %bb42
  %i47 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 1
  %i48 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i32, i64 0, i32 3
  %i49 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 0
  %i50 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i32, i64 0, i32 1
  %i51 = load i16, ptr %i50, align 8
  %i52 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i32, i64 0, i32 2
  %i53 = load i16, ptr %i52, align 2
  %i54 = icmp eq i16 %i51, %i53
  br i1 %i54, label %bb60, label %bb55

bb55:                                             ; preds = %bb46
  %i56 = zext i16 %i51 to i64
  %i57 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i40, i64 %i56
  %i58 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i57, i64 0, i32 0
  store i16 %i53, ptr %i58, align 4
  %i59 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i57, i64 0, i32 1
  store i32 -2228259, ptr %i59, align 4
  store i16 %i51, ptr %i52, align 2
  br label %bb60

bb60:                                             ; preds = %bb55, %bb46
  %i61 = getelementptr %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg1, i64 0, i32 0, i32 0, i32 0, i32 0
  %i62 = load ptr, ptr %i61, align 8
  %i63 = tail call i1 @llvm.type.test(ptr %i62, metadata !"_ZTSN11xalanc_1_1013XStringCachedE") #21
  tail call void @llvm.assume(i1 %i63) #21
  %i64 = load ptr, ptr %i62, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr noundef nonnull align 8 dereferenceable(80) %arg1) #21, !intel_dtrans_type !1126
  %i65 = load i16, ptr %i50, align 8
  %i66 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %arg1, i64 0, i32 0
  store i16 %i65, ptr %i66, align 4
  %i67 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %arg1, i64 0, i32 1
  store i32 -2228259, ptr %i67, align 4
  %i68 = load ptr, ptr %i48, align 8
  %i69 = ptrtoint ptr %arg1 to i64
  %i70 = ptrtoint ptr %i68 to i64
  %i71 = sub i64 %i69, %i70
  %i72 = sdiv exact i64 %i71, 80
  %i73 = trunc i64 %i72 to i16
  store i16 %i73, ptr %i52, align 2
  store i16 %i73, ptr %i50, align 8
  %i74 = load i16, ptr %i47, align 8
  %i75 = add i16 %i74, -1
  store i16 %i75, ptr %i47, align 8
  %i76 = load ptr, ptr %i2, align 8, !noalias !1144
  %i77 = icmp eq ptr %i76, null
  br i1 %i77, label %bb81, label %bb78

bb78:                                             ; preds = %bb60
  %i79 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i76, i64 0, i32 2
  %i80 = load ptr, ptr %i79, align 8, !noalias !1144
  br label %bb101

bb81:                                             ; preds = %bb60
  %i82 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i83 = load ptr, ptr %i82, align 8, !noalias !1144
  %i84 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i83, i64 0, i32 0
  %i85 = load ptr, ptr %i84, align 8, !noalias !1144
  %i86 = tail call i1 @llvm.type.test(ptr %i85, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i86)
  %i87 = getelementptr inbounds ptr, ptr %i85, i64 2
  %i88 = load ptr, ptr %i87, align 8, !noalias !1144
  %i89 = bitcast ptr %i88 to ptr
  %i90 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i91 = icmp eq ptr %i89, %i90
  br i1 %i91, label %bb92, label %bb94

bb92:                                             ; preds = %bb81
  %i93 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i83, i64 noundef 24), !noalias !1144, !intel_dtrans_type !1099
  br label %bb96

bb94:                                             ; preds = %bb81
  %i95 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i83, i64 noundef 24), !noalias !1144, !intel_dtrans_type !1099
  br label %bb96

bb96:                                             ; preds = %bb94, %bb92
  %i97 = phi ptr [ %i93, %bb92 ], [ %i95, %bb94 ]
  br label %bb98

bb98:                                             ; preds = %bb96
  store ptr %i97, ptr %i2, align 8, !noalias !1144
  %i99 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i97, i64 0, i32 2
  store ptr %i97, ptr %i99, align 8, !noalias !1144
  %i100 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i97, i64 0, i32 1
  store ptr %i97, ptr %i100, align 8, !noalias !1144
  br label %bb101

bb101:                                            ; preds = %bb98, %bb78
  %i102 = phi ptr [ %i97, %bb98 ], [ %i80, %bb78 ]
  %i103 = icmp eq ptr %i30, %i102
  br i1 %i103, label %bb175, label %bb104

bb104:                                            ; preds = %bb101
  %i105 = load ptr, ptr %i49, align 8
  %i106 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 2
  %i107 = load ptr, ptr %i106, align 8
  %i108 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 1
  %i109 = load ptr, ptr %i108, align 8
  %i110 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i109, i64 0, i32 2
  store ptr %i107, ptr %i110, align 8
  %i111 = load ptr, ptr %i106, align 8
  %i112 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i111, i64 0, i32 1
  store ptr %i109, ptr %i112, align 8
  store ptr null, ptr %i108, align 8
  %i113 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i114 = load ptr, ptr %i113, align 8
  store ptr %i114, ptr %i106, align 8
  store ptr %i30, ptr %i113, align 8
  %i115 = load ptr, ptr %i2, align 8, !noalias !1147
  %i116 = icmp eq ptr %i115, null
  br i1 %i116, label %bb120, label %bb117

bb117:                                            ; preds = %bb104
  %i118 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i115, i64 0, i32 2
  %i119 = load ptr, ptr %i118, align 8, !noalias !1147
  br label %bb142

bb120:                                            ; preds = %bb104
  %i121 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i122 = load ptr, ptr %i121, align 8, !noalias !1147
  %i123 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i122, i64 0, i32 0
  %i124 = load ptr, ptr %i123, align 8, !noalias !1147
  %i125 = tail call i1 @llvm.type.test(ptr %i124, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i125)
  %i126 = getelementptr inbounds ptr, ptr %i124, i64 2
  %i127 = load ptr, ptr %i126, align 8, !noalias !1147
  %i128 = bitcast ptr %i127 to ptr
  %i129 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i130 = icmp eq ptr %i128, %i129
  br i1 %i130, label %bb131, label %bb133

bb131:                                            ; preds = %bb120
  %i132 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i122, i64 noundef 24), !noalias !1147, !intel_dtrans_type !1099
  br label %bb135

bb133:                                            ; preds = %bb120
  %i134 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i122, i64 noundef 24), !noalias !1147, !intel_dtrans_type !1099
  br label %bb135

bb135:                                            ; preds = %bb133, %bb131
  %i136 = phi ptr [ %i132, %bb131 ], [ %i134, %bb133 ]
  br label %bb137

bb137:                                            ; preds = %bb135
  store ptr %i136, ptr %i2, align 8, !noalias !1147
  %i138 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i136, i64 0, i32 2
  store ptr %i136, ptr %i138, align 8, !noalias !1147
  %i139 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i136, i64 0, i32 1
  store ptr %i136, ptr %i139, align 8, !noalias !1147
  %i140 = load ptr, ptr %i113, align 8
  %i141 = icmp eq ptr %i140, null
  br i1 %i141, label %bb147, label %bb142

bb142:                                            ; preds = %bb137, %bb117
  %i143 = phi ptr [ %i119, %bb117 ], [ %i136, %bb137 ]
  %i144 = phi ptr [ %i30, %bb117 ], [ %i140, %bb137 ]
  %i145 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i144, i64 0, i32 2
  %i146 = load ptr, ptr %i145, align 8
  br label %bb164

bb147:                                            ; preds = %bb137
  %i148 = load ptr, ptr %i121, align 8
  %i149 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i148, i64 0, i32 0
  %i150 = load ptr, ptr %i149, align 8
  %i151 = tail call i1 @llvm.type.test(ptr %i150, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i151)
  %i152 = getelementptr inbounds ptr, ptr %i150, i64 2
  %i153 = load ptr, ptr %i152, align 8
  %i154 = bitcast ptr %i153 to ptr
  %i155 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i156 = icmp eq ptr %i154, %i155
  br i1 %i156, label %bb157, label %bb159

bb157:                                            ; preds = %bb147
  %i158 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i148, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb161

bb159:                                            ; preds = %bb147
  %i160 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i148, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb161

bb161:                                            ; preds = %bb159, %bb157
  %i162 = phi ptr [ %i158, %bb157 ], [ %i160, %bb159 ]
  br label %bb163

bb163:                                            ; preds = %bb161
  br label %bb164

bb164:                                            ; preds = %bb163, %bb142
  %i165 = phi ptr [ %i143, %bb142 ], [ %i136, %bb163 ]
  %i166 = phi ptr [ %i144, %bb142 ], [ %i162, %bb163 ]
  %i167 = phi ptr [ %i146, %bb142 ], [ null, %bb163 ]
  %i168 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i166, i64 0, i32 0
  store ptr %i105, ptr %i168, align 8
  %i169 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i166, i64 0, i32 1
  %i170 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i165, i64 0, i32 1
  %i171 = load ptr, ptr %i170, align 8
  store ptr %i171, ptr %i169, align 8
  %i172 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i166, i64 0, i32 2
  store ptr %i165, ptr %i172, align 8
  %i173 = load ptr, ptr %i170, align 8
  %i174 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i173, i64 0, i32 2
  store ptr %i166, ptr %i174, align 8
  store ptr %i166, ptr %i170, align 8
  store ptr %i167, ptr %i113, align 8
  br label %bb175

bb175:                                            ; preds = %bb164, %bb101
  %i176 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %arg, i64 0, i32 1
  %i177 = load i8, ptr %i176, align 8, !range !1150
  %i178 = icmp eq i8 %i177, 0
  br i1 %i178, label %bb236, label %bb179

bb179:                                            ; preds = %bb175
  %i180 = load ptr, ptr %i2, align 8
  %i181 = icmp eq ptr %i180, null
  br i1 %i181, label %bb182, label %bb202

bb182:                                            ; preds = %bb179
  %i183 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i184 = load ptr, ptr %i183, align 8
  %i185 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i184, i64 0, i32 0
  %i186 = load ptr, ptr %i185, align 8
  %i187 = tail call i1 @llvm.type.test(ptr %i186, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i187)
  %i188 = getelementptr inbounds ptr, ptr %i186, i64 2
  %i189 = load ptr, ptr %i188, align 8
  %i190 = bitcast ptr %i189 to ptr
  %i191 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i192 = icmp eq ptr %i190, %i191
  br i1 %i192, label %bb193, label %bb195

bb193:                                            ; preds = %bb182
  %i194 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i184, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb197

bb195:                                            ; preds = %bb182
  %i196 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i184, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb197

bb197:                                            ; preds = %bb195, %bb193
  %i198 = phi ptr [ %i194, %bb193 ], [ %i196, %bb195 ]
  br label %bb199

bb199:                                            ; preds = %bb197
  store ptr %i198, ptr %i2, align 8
  %i200 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i198, i64 0, i32 2
  store ptr %i198, ptr %i200, align 8
  %i201 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i198, i64 0, i32 1
  store ptr %i198, ptr %i201, align 8
  br label %bb471

bb202:                                            ; preds = %bb179
  %i203 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i180, i64 0, i32 2
  %i204 = load ptr, ptr %i203, align 8
  %i205 = icmp eq ptr %i204, %i180
  br i1 %i205, label %bb236, label %bb206

bb206:                                            ; preds = %bb202
  %i207 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i204, i64 0, i32 0
  %i208 = load ptr, ptr %i207, align 8
  %i209 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i208, i64 0, i32 1
  %i210 = load i16, ptr %i209, align 8
  %i211 = icmp eq i16 %i210, 0
  br i1 %i211, label %bb212, label %bb236

bb212:                                            ; preds = %bb206
  %i213 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i204, i64 0, i32 2
  %i214 = load ptr, ptr %i213, align 8
  %i215 = icmp eq ptr %i214, %i180
  br i1 %i215, label %bb224, label %bb216

bb216:                                            ; preds = %bb212
  %i217 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i214, i64 0, i32 0
  %i218 = load ptr, ptr %i217, align 8
  %i219 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i218, i64 0, i32 1
  %i220 = load i16, ptr %i219, align 8
  %i221 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i218, i64 0, i32 2
  %i222 = load i16, ptr %i221, align 2
  %i223 = icmp ult i16 %i220, %i222
  br i1 %i223, label %bb224, label %bb236

bb224:                                            ; preds = %bb216, %bb212
  %i225 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i226 = load ptr, ptr %i225, align 8
  %i227 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i204, i64 0, i32 1
  %i228 = load ptr, ptr %i227, align 8
  %i229 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i228, i64 0, i32 2
  store ptr %i214, ptr %i229, align 8
  %i230 = load ptr, ptr %i213, align 8
  %i231 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i230, i64 0, i32 1
  store ptr %i228, ptr %i231, align 8
  store ptr null, ptr %i227, align 8
  store ptr %i226, ptr %i213, align 8
  store ptr %i204, ptr %i225, align 8
  br label %bb236

bb232:                                            ; preds = %bb42, %bb38
  %i233 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i30, i64 0, i32 2
  %i234 = load ptr, ptr %i233, align 8, !noalias !1151
  %i235 = icmp eq ptr %i234, %i3
  br i1 %i235, label %bb236, label %bb29, !llvm.loop !1154

bb236:                                            ; preds = %bb232, %bb224, %bb216, %bb206, %bb202, %bb175, %bb29
  %i237 = phi ptr [ %i30, %bb224 ], [ %i30, %bb216 ], [ %i30, %bb206 ], [ %i30, %bb202 ], [ %i30, %bb175 ], [ %i3, %bb232 ], [ %i30, %bb29 ]
  %i238 = phi i1 [ false, %bb224 ], [ false, %bb216 ], [ false, %bb206 ], [ false, %bb202 ], [ false, %bb175 ], [ true, %bb232 ], [ true, %bb29 ]
  %i239 = phi i8 [ 1, %bb224 ], [ 1, %bb216 ], [ 1, %bb206 ], [ 1, %bb202 ], [ 1, %bb175 ], [ 0, %bb232 ], [ 0, %bb29 ]
  %i240 = load ptr, ptr %i2, align 8, !noalias !1122
  %i241 = icmp eq ptr %i240, null
  br i1 %i241, label %bb245, label %bb242

bb242:                                            ; preds = %bb236
  %i243 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i240, i64 0, i32 2
  %i244 = load ptr, ptr %i243, align 8, !noalias !1155
  br i1 %i238, label %bb265, label %bb471

bb245:                                            ; preds = %bb236
  %i246 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i247 = load ptr, ptr %i246, align 8, !noalias !1160
  %i248 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i247, i64 0, i32 0
  %i249 = load ptr, ptr %i248, align 8, !noalias !1160
  %i250 = tail call i1 @llvm.type.test(ptr %i249, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i250)
  %i251 = getelementptr inbounds ptr, ptr %i249, i64 2
  %i252 = load ptr, ptr %i251, align 8, !noalias !1160
  %i253 = bitcast ptr %i252 to ptr
  %i254 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i255 = icmp eq ptr %i253, %i254
  br i1 %i255, label %bb256, label %bb258

bb256:                                            ; preds = %bb245
  %i257 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i247, i64 noundef 24), !noalias !1160, !intel_dtrans_type !1099
  br label %bb260

bb258:                                            ; preds = %bb245
  %i259 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i247, i64 noundef 24), !noalias !1160, !intel_dtrans_type !1099
  br label %bb260

bb260:                                            ; preds = %bb258, %bb256
  %i261 = phi ptr [ %i257, %bb256 ], [ %i259, %bb258 ]
  br label %bb262

bb262:                                            ; preds = %bb260
  store ptr %i261, ptr %i2, align 8, !noalias !1160
  %i263 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i261, i64 0, i32 2
  store ptr %i261, ptr %i263, align 8, !noalias !1160
  %i264 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i261, i64 0, i32 1
  store ptr %i261, ptr %i264, align 8, !noalias !1160
  br i1 %i238, label %bb265, label %bb471

bb265:                                            ; preds = %bb262, %bb242
  %i266 = phi ptr [ %i240, %bb242 ], [ %i261, %bb262 ]
  %i267 = phi ptr [ %i244, %bb242 ], [ %i261, %bb262 ]
  %i268 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i237, i64 0, i32 0
  br label %bb269

bb269:                                            ; preds = %bb468, %bb265
  %i270 = phi ptr [ %i274, %bb468 ], [ %i266, %bb265 ]
  %i271 = icmp eq ptr %i270, %i267
  br i1 %i271, label %bb471, label %bb272

bb272:                                            ; preds = %bb269
  %i273 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i270, i64 0, i32 1
  %i274 = load ptr, ptr %i273, align 8, !noalias !1165
  %i275 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i274, i64 0, i32 0
  %i276 = load ptr, ptr %i275, align 8
  %i277 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i276, i64 0, i32 3
  %i278 = load ptr, ptr %i277, align 8
  %i279 = icmp ugt ptr %i278, %arg1
  br i1 %i279, label %bb468, label %bb280

bb280:                                            ; preds = %bb272
  %i281 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i276, i64 0, i32 2
  %i282 = load i16, ptr %i281, align 2
  %i283 = zext i16 %i282 to i64
  %i284 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i278, i64 %i283
  %i285 = icmp ugt ptr %i284, %arg1
  br i1 %i285, label %bb286, label %bb468

bb286:                                            ; preds = %bb280
  %i287 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i276, i64 0, i32 3
  %i288 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i276, i64 0, i32 1
  %i289 = load i16, ptr %i288, align 8
  %i290 = getelementptr inbounds %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock", ptr %i276, i64 0, i32 2
  %i291 = load i16, ptr %i290, align 2
  %i292 = icmp eq i16 %i289, %i291
  br i1 %i292, label %bb298, label %bb293

bb293:                                            ; preds = %bb286
  %i294 = zext i16 %i289 to i64
  %i295 = getelementptr inbounds %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %i278, i64 %i294
  %i296 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i295, i64 0, i32 0
  store i16 %i291, ptr %i296, align 4
  %i297 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %i295, i64 0, i32 1
  store i32 -2228259, ptr %i297, align 4
  store i16 %i289, ptr %i290, align 2
  br label %bb298

bb298:                                            ; preds = %bb293, %bb286
  %i299 = getelementptr %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached", ptr %arg1, i64 0, i32 0, i32 0, i32 0, i32 0
  %i300 = load ptr, ptr %i299, align 8
  %i301 = tail call i1 @llvm.type.test(ptr %i300, metadata !"_ZTSN11xalanc_1_1013XStringCachedE") #21
  tail call void @llvm.assume(i1 %i301) #21
  %i302 = load ptr, ptr %i300, align 8
  tail call void @_ZN11xalanc_1_1013XStringCachedD2Ev(ptr noundef nonnull align 8 dereferenceable(80) %arg1) #21, !intel_dtrans_type !1126
  %i303 = load i16, ptr %i288, align 8
  %i304 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %arg1, i64 0, i32 0
  store i16 %i303, ptr %i304, align 4
  %i305 = getelementptr inbounds %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %arg1, i64 0, i32 1
  store i32 -2228259, ptr %i305, align 4
  %i306 = load ptr, ptr %i287, align 8
  %i307 = ptrtoint ptr %arg1 to i64
  %i308 = ptrtoint ptr %i306 to i64
  %i309 = sub i64 %i307, %i308
  %i310 = sdiv exact i64 %i309, 80
  %i311 = trunc i64 %i310 to i16
  store i16 %i311, ptr %i290, align 2
  store i16 %i311, ptr %i288, align 8
  %i312 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i276, i64 0, i32 1
  %i313 = load i16, ptr %i312, align 8
  %i314 = add i16 %i313, -1
  store i16 %i314, ptr %i312, align 8
  %i315 = load ptr, ptr %i2, align 8, !noalias !1168
  %i316 = icmp eq ptr %i315, null
  br i1 %i316, label %bb317, label %bb337

bb317:                                            ; preds = %bb298
  %i318 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i319 = load ptr, ptr %i318, align 8, !noalias !1168
  %i320 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i319, i64 0, i32 0
  %i321 = load ptr, ptr %i320, align 8, !noalias !1168
  %i322 = tail call i1 @llvm.type.test(ptr %i321, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i322)
  %i323 = getelementptr inbounds ptr, ptr %i321, i64 2
  %i324 = load ptr, ptr %i323, align 8, !noalias !1168
  %i325 = bitcast ptr %i324 to ptr
  %i326 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i327 = icmp eq ptr %i325, %i326
  br i1 %i327, label %bb328, label %bb330

bb328:                                            ; preds = %bb317
  %i329 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i319, i64 noundef 24), !noalias !1168, !intel_dtrans_type !1099
  br label %bb332

bb330:                                            ; preds = %bb317
  %i331 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i319, i64 noundef 24), !noalias !1168, !intel_dtrans_type !1099
  br label %bb332

bb332:                                            ; preds = %bb330, %bb328
  %i333 = phi ptr [ %i329, %bb328 ], [ %i331, %bb330 ]
  br label %bb334

bb334:                                            ; preds = %bb332
  store ptr %i333, ptr %i2, align 8, !noalias !1168
  %i335 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i333, i64 0, i32 2
  store ptr %i333, ptr %i335, align 8, !noalias !1168
  %i336 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i333, i64 0, i32 1
  store ptr %i333, ptr %i336, align 8, !noalias !1168
  br label %bb337

bb337:                                            ; preds = %bb334, %bb298
  %i338 = phi ptr [ %i333, %bb334 ], [ %i315, %bb298 ]
  %i339 = icmp eq ptr %i270, %i338
  br i1 %i339, label %bb411, label %bb340

bb340:                                            ; preds = %bb337
  %i341 = load ptr, ptr %i268, align 8
  %i342 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i237, i64 0, i32 2
  %i343 = load ptr, ptr %i342, align 8
  %i344 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i237, i64 0, i32 1
  %i345 = load ptr, ptr %i344, align 8
  %i346 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i345, i64 0, i32 2
  store ptr %i343, ptr %i346, align 8
  %i347 = load ptr, ptr %i342, align 8
  %i348 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i347, i64 0, i32 1
  store ptr %i345, ptr %i348, align 8
  store ptr null, ptr %i344, align 8
  %i349 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i350 = load ptr, ptr %i349, align 8
  store ptr %i350, ptr %i342, align 8
  store ptr %i237, ptr %i349, align 8
  %i351 = load ptr, ptr %i2, align 8, !noalias !1173
  %i352 = icmp eq ptr %i351, null
  br i1 %i352, label %bb356, label %bb353

bb353:                                            ; preds = %bb340
  %i354 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i351, i64 0, i32 2
  %i355 = load ptr, ptr %i354, align 8, !noalias !1173
  br label %bb378

bb356:                                            ; preds = %bb340
  %i357 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i358 = load ptr, ptr %i357, align 8, !noalias !1173
  %i359 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i358, i64 0, i32 0
  %i360 = load ptr, ptr %i359, align 8, !noalias !1173
  %i361 = tail call i1 @llvm.type.test(ptr %i360, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i361)
  %i362 = getelementptr inbounds ptr, ptr %i360, i64 2
  %i363 = load ptr, ptr %i362, align 8, !noalias !1173
  %i364 = bitcast ptr %i363 to ptr
  %i365 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i366 = icmp eq ptr %i364, %i365
  br i1 %i366, label %bb367, label %bb369

bb367:                                            ; preds = %bb356
  %i368 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i358, i64 noundef 24), !noalias !1173, !intel_dtrans_type !1099
  br label %bb371

bb369:                                            ; preds = %bb356
  %i370 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i358, i64 noundef 24), !noalias !1173, !intel_dtrans_type !1099
  br label %bb371

bb371:                                            ; preds = %bb369, %bb367
  %i372 = phi ptr [ %i368, %bb367 ], [ %i370, %bb369 ]
  br label %bb373

bb373:                                            ; preds = %bb371
  store ptr %i372, ptr %i2, align 8, !noalias !1173
  %i374 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i372, i64 0, i32 2
  store ptr %i372, ptr %i374, align 8, !noalias !1173
  %i375 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i372, i64 0, i32 1
  store ptr %i372, ptr %i375, align 8, !noalias !1173
  %i376 = load ptr, ptr %i349, align 8
  %i377 = icmp eq ptr %i376, null
  br i1 %i377, label %bb383, label %bb378

bb378:                                            ; preds = %bb373, %bb353
  %i379 = phi ptr [ %i355, %bb353 ], [ %i372, %bb373 ]
  %i380 = phi ptr [ %i237, %bb353 ], [ %i376, %bb373 ]
  %i381 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i380, i64 0, i32 2
  %i382 = load ptr, ptr %i381, align 8
  br label %bb400

bb383:                                            ; preds = %bb373
  %i384 = load ptr, ptr %i357, align 8
  %i385 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i384, i64 0, i32 0
  %i386 = load ptr, ptr %i385, align 8
  %i387 = tail call i1 @llvm.type.test(ptr %i386, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i387)
  %i388 = getelementptr inbounds ptr, ptr %i386, i64 2
  %i389 = load ptr, ptr %i388, align 8
  %i390 = bitcast ptr %i389 to ptr
  %i391 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i392 = icmp eq ptr %i390, %i391
  br i1 %i392, label %bb393, label %bb395

bb393:                                            ; preds = %bb383
  %i394 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i384, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb397

bb395:                                            ; preds = %bb383
  %i396 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i384, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb397

bb397:                                            ; preds = %bb395, %bb393
  %i398 = phi ptr [ %i394, %bb393 ], [ %i396, %bb395 ]
  br label %bb399

bb399:                                            ; preds = %bb397
  br label %bb400

bb400:                                            ; preds = %bb399, %bb378
  %i401 = phi ptr [ %i379, %bb378 ], [ %i372, %bb399 ]
  %i402 = phi ptr [ %i380, %bb378 ], [ %i398, %bb399 ]
  %i403 = phi ptr [ %i382, %bb378 ], [ null, %bb399 ]
  %i404 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i402, i64 0, i32 0
  store ptr %i341, ptr %i404, align 8
  %i405 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i402, i64 0, i32 1
  %i406 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i401, i64 0, i32 1
  %i407 = load ptr, ptr %i406, align 8
  store ptr %i407, ptr %i405, align 8
  %i408 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i402, i64 0, i32 2
  store ptr %i401, ptr %i408, align 8
  %i409 = load ptr, ptr %i406, align 8
  %i410 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i409, i64 0, i32 2
  store ptr %i402, ptr %i410, align 8
  store ptr %i402, ptr %i406, align 8
  store ptr %i403, ptr %i349, align 8
  br label %bb411

bb411:                                            ; preds = %bb400, %bb337
  %i412 = getelementptr inbounds %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator", ptr %arg, i64 0, i32 1
  %i413 = load i8, ptr %i412, align 8, !range !1150
  %i414 = icmp eq i8 %i413, 0
  br i1 %i414, label %bb471, label %bb415

bb415:                                            ; preds = %bb411
  %i416 = load ptr, ptr %i2, align 8
  %i417 = icmp eq ptr %i416, null
  br i1 %i417, label %bb418, label %bb438

bb418:                                            ; preds = %bb415
  %i419 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 0
  %i420 = load ptr, ptr %i419, align 8
  %i421 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i420, i64 0, i32 0
  %i422 = load ptr, ptr %i421, align 8
  %i423 = tail call i1 @llvm.type.test(ptr %i422, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i423)
  %i424 = getelementptr inbounds ptr, ptr %i422, i64 2
  %i425 = load ptr, ptr %i424, align 8
  %i426 = bitcast ptr %i425 to ptr
  %i427 = bitcast ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm to ptr
  %i428 = icmp eq ptr %i426, %i427
  br i1 %i428, label %bb429, label %bb431

bb429:                                            ; preds = %bb418
  %i430 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i420, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb433

bb431:                                            ; preds = %bb418
  %i432 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i420, i64 noundef 24), !intel_dtrans_type !1099
  br label %bb433

bb433:                                            ; preds = %bb431, %bb429
  %i434 = phi ptr [ %i430, %bb429 ], [ %i432, %bb431 ]
  br label %bb435

bb435:                                            ; preds = %bb433
  store ptr %i434, ptr %i2, align 8
  %i436 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i434, i64 0, i32 2
  store ptr %i434, ptr %i436, align 8
  %i437 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i434, i64 0, i32 1
  store ptr %i434, ptr %i437, align 8
  br label %bb471

bb438:                                            ; preds = %bb415
  %i439 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i416, i64 0, i32 2
  %i440 = load ptr, ptr %i439, align 8
  %i441 = icmp eq ptr %i440, %i416
  br i1 %i441, label %bb471, label %bb442

bb442:                                            ; preds = %bb438
  %i443 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i440, i64 0, i32 0
  %i444 = load ptr, ptr %i443, align 8
  %i445 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i444, i64 0, i32 1
  %i446 = load i16, ptr %i445, align 8
  %i447 = icmp eq i16 %i446, 0
  br i1 %i447, label %bb448, label %bb471

bb448:                                            ; preds = %bb442
  %i449 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i440, i64 0, i32 2
  %i450 = load ptr, ptr %i449, align 8
  %i451 = icmp eq ptr %i450, %i416
  br i1 %i451, label %bb460, label %bb452

bb452:                                            ; preds = %bb448
  %i453 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i450, i64 0, i32 0
  %i454 = load ptr, ptr %i453, align 8
  %i455 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i454, i64 0, i32 1
  %i456 = load i16, ptr %i455, align 8
  %i457 = getelementptr inbounds %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase", ptr %i454, i64 0, i32 2
  %i458 = load i16, ptr %i457, align 2
  %i459 = icmp ult i16 %i456, %i458
  br i1 %i459, label %bb460, label %bb471

bb460:                                            ; preds = %bb452, %bb448
  %i461 = getelementptr inbounds %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList", ptr %i, i64 0, i32 2
  %i462 = load ptr, ptr %i461, align 8
  %i463 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i440, i64 0, i32 1
  %i464 = load ptr, ptr %i463, align 8
  %i465 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i464, i64 0, i32 2
  store ptr %i450, ptr %i465, align 8
  %i466 = load ptr, ptr %i449, align 8
  %i467 = getelementptr inbounds %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %i466, i64 0, i32 1
  store ptr %i464, ptr %i467, align 8
  store ptr null, ptr %i463, align 8
  store ptr %i462, ptr %i449, align 8
  store ptr %i440, ptr %i461, align 8
  br label %bb471

bb468:                                            ; preds = %bb280, %bb272
  %i469 = load ptr, ptr %i268, align 8
  %i470 = icmp eq ptr %i276, %i469
  br i1 %i470, label %bb471, label %bb269, !llvm.loop !1176

bb471:                                            ; preds = %bb468, %bb460, %bb452, %bb442, %bb438, %bb435, %bb411, %bb269, %bb262, %bb242, %bb199
  %i472 = phi i8 [ 1, %bb411 ], [ 1, %bb435 ], [ 1, %bb438 ], [ 1, %bb442 ], [ 1, %bb452 ], [ 1, %bb460 ], [ %i239, %bb262 ], [ 1, %bb199 ], [ %i239, %bb242 ], [ %i239, %bb269 ], [ %i239, %bb468 ]
  %i473 = and i8 %i472, 1
  %i474 = icmp ne i8 %i473, 0
  br label %bb475

bb475:                                            ; preds = %bb471, %bb25, %bb22
  %i476 = phi i1 [ %i474, %bb471 ], [ false, %bb25 ], [ false, %bb22 ]
  ret i1 %i476
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xalanc_1_1022XStringCachedAllocator5resetEv(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) #17 align 2 !intel.dtrans.func.type !1177 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator", ptr %arg, i64 0, i32 0
  tail call void @_ZN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEE5resetEv(ptr noundef nonnull align 8 dereferenceable(40) %i)
  ret void
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !1178 dso_local void @_ZNSt9bad_allocD1Ev(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1") unnamed_addr #18

; Function Attrs: nofree noreturn uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1) unnamed_addr #19 comdat align 2 !intel.dtrans.func.type !1180 !_Intel.Devirt.Target !1071 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #21
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #22
  unreachable
}

; Function Attrs: nofree noreturn uwtable
define hidden void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %arg1) unnamed_addr #19 comdat align 2 !intel.dtrans.func.type !1182 !_Intel.Devirt.Target !1071 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #21
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #22
  unreachable
}

attributes #0 = { nofree }
attributes #1 = { nofree noinline noreturn nounwind "pre_loopopt" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #4 = { nofree noreturn }
attributes #5 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { inlinehint mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #11 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #12 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #18 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #19 = { nofree noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #20 = { allocsize(0) }
attributes #21 = { nounwind }
attributes #22 = { noreturn }
attributes #23 = { noreturn nounwind }

!intel.dtrans.types = !{!84, !88, !90, !93, !97, !100, !101, !102, !103, !104, !105, !108, !109, !113, !117, !119, !121, !123, !126, !127, !129, !130, !133, !134, !136, !137, !140, !142, !144, !147, !149, !153, !161, !166, !168, !170, !172, !178, !179, !180, !182, !184, !185, !187, !189, !191, !193, !194, !196, !198, !200, !201, !202, !204, !205, !206, !207, !208, !211, !212, !213, !217, !218, !219, !221, !223, !224, !225, !228, !230, !232, !234, !236, !238, !240, !242, !244, !246, !247, !249, !250, !266, !268, !270, !272, !274, !276, !278, !280, !285, !286, !287, !289, !291, !292, !297, !298, !299, !301, !303, !305, !308, !310, !312, !313, !315, !317, !319, !320, !324, !326, !328, !330, !332, !334, !336, !338, !340, !342, !347, !351, !352, !354, !355, !356, !357, !359, !361, !363, !365, !366, !368, !369, !370, !372, !374, !376, !378, !380, !385, !387, !389, !391, !393, !395, !396, !398, !399, !400, !401, !403, !406, !408, !410, !412, !413, !415, !417, !418, !419, !422, !423, !424, !430, !446, !448, !450, !452, !454, !456, !458, !460, !462, !464, !466, !468, !470, !472, !474, !476, !478, !480, !482, !484, !486, !488, !490, !492, !494, !496, !498, !500, !502, !504, !506, !508, !511, !513, !518, !519, !521, !523, !526, !528, !530, !532, !534, !536, !538, !540, !542, !544, !546, !548, !550, !552, !554, !556, !558, !560, !562, !564, !566, !568, !570, !572, !574, !576, !578, !580, !582, !584, !587, !590, !593, !596, !599, !602, !605, !608, !611, !614, !615, !617, !618, !620, !621, !622, !626, !627, !629, !630, !631, !632, !634, !635, !636, !637, !639, !641, !642, !645, !648, !649, !651, !653, !655, !657, !659, !661, !662, !663, !666, !669, !671, !673, !675, !678, !681, !683, !685, !687, !689, !691, !693, !694, !696, !698, !699, !701, !703, !705, !706, !707, !710, !712, !714, !716, !718, !721, !722, !725, !727, !729, !730, !731, !733, !735, !736, !737, !739, !741, !758, !759, !760, !762, !764, !766, !768, !770, !773, !775, !777, !779, !781, !784, !786, !788, !790, !791, !821, !825, !827, !832, !833, !834, !835, !837, !838, !839, !841, !843, !845, !847, !848, !851, !853, !854, !856, !858, !861, !863, !865, !867, !869, !870, !872, !874, !875, !880, !881, !882, !884, !886, !888, !890, !892, !894, !895, !897, !899, !900, !902, !904, !906, !908, !910, !912, !914, !916, !918, !920, !922, !924, !926, !928, !930, !931, !933, !935, !939, !940, !941, !942, !947, !948, !949, !950, !952, !955, !958, !963, !964, !965, !966, !968, !970, !972, !974, !976, !979, !980, !981, !983, !985, !987, !989, !991, !993, !994, !995, !997, !999, !1001, !1003, !1004, !1005, !1007, !1009, !1011, !1013, !1015, !1018, !1019, !1022, !1024, !1026, !1029, !1030, !1031, !1032, !1034, !1035, !1037, !1038, !1039, !1041, !1042, !1044, !1046, !1048, !1050, !1052, !1053, !1054}
!llvm.ident = !{!1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055, !1055}
!llvm.module.flags = !{!1056, !1057, !1058, !1059, !1060, !1061}

!0 = !{i64 16, !"_ZTSSt9bad_alloc"}
!1 = !{i64 32, !"_ZTSMSt9bad_allocKFPKcvE.virtual"}
!2 = !{i64 16, !"_ZTSSt9exception"}
!3 = !{i64 32, !"_ZTSMSt9exceptionKFPKcvE.virtual"}
!4 = !{!"L", i32 1, !5}
!5 = !{!"A", i32 5, !6}
!6 = !{i8 0, i32 1}
!7 = !{!"L", i32 3, !6, !6, !6}
!8 = !{i32 16, !"_ZTSN11xalanc_1_1011XStringBaseE"}
!9 = !{i32 32, !"_ZTSMN11xalanc_1_1011XStringBaseEFvvE.virtual"}
!10 = !{i32 40, !"_ZTSMN11xalanc_1_1011XStringBaseEFvvE.virtual"}
!11 = !{i32 48, !"_ZTSMN11xalanc_1_1011XStringBaseEKFjvE.virtual"}
!12 = !{i32 56, !"_ZTSMN11xalanc_1_1011XStringBaseEKFRKNS_14XalanDOMStringEvE.virtual"}
!13 = !{i32 64, !"_ZTSMN11xalanc_1_1011XStringBaseEKFdvE.virtual"}
!14 = !{i32 72, !"_ZTSMN11xalanc_1_1011XStringBaseEKFbvE.virtual"}
!15 = !{i32 80, !"_ZTSMN11xalanc_1_1011XStringBaseEKFRKNS_14XalanDOMStringEvE.virtual"}
!16 = !{i32 88, !"_ZTSMN11xalanc_1_1011XStringBaseEKFvRNS_17FormatterListenerEMS1_FvPKtjEE.virtual"}
!17 = !{i32 96, !"_ZTSMN11xalanc_1_1011XStringBaseEKFdvE.virtual"}
!18 = !{i32 104, !"_ZTSMN11xalanc_1_1011XStringBaseEKFvRNS_14XalanDOMStringEE.virtual"}
!19 = !{i32 112, !"_ZTSMN11xalanc_1_1011XStringBaseEKFRKNS_21XalanDocumentFragmentEvE.virtual"}
!20 = !{i32 120, !"_ZTSMN11xalanc_1_1011XStringBaseEKFRKNS_15NodeRefListBaseEvE.virtual"}
!21 = !{i32 128, !"_ZTSMN11xalanc_1_1011XStringBaseEFvRNS_19XObjectTypeCallbackEE.virtual"}
!22 = !{i32 136, !"_ZTSMN11xalanc_1_1011XStringBaseEKFvRNS_19XObjectTypeCallbackEE.virtual"}
!23 = !{i32 144, !"_ZTSMN11xalanc_1_1011XStringBaseEKFNS_7XObject11eObjectTypeEvE.virtual"}
!24 = !{i32 16, !"_ZTSN11xalanc_1_1013XStringCachedE"}
!25 = !{i32 32, !"_ZTSMN11xalanc_1_1013XStringCachedEFvvE.virtual"}
!26 = !{i32 40, !"_ZTSMN11xalanc_1_1013XStringCachedEFvvE.virtual"}
!27 = !{i32 48, !"_ZTSMN11xalanc_1_1013XStringCachedEKFjvE.virtual"}
!28 = !{i32 56, !"_ZTSMN11xalanc_1_1013XStringCachedEKFRKNS_14XalanDOMStringEvE.virtual"}
!29 = !{i32 64, !"_ZTSMN11xalanc_1_1013XStringCachedEKFdvE.virtual"}
!30 = !{i32 72, !"_ZTSMN11xalanc_1_1013XStringCachedEKFbvE.virtual"}
!31 = !{i32 80, !"_ZTSMN11xalanc_1_1013XStringCachedEKFRKNS_14XalanDOMStringEvE.virtual"}
!32 = !{i32 88, !"_ZTSMN11xalanc_1_1013XStringCachedEKFvRNS_17FormatterListenerEMS1_FvPKtjEE.virtual"}
!33 = !{i32 96, !"_ZTSMN11xalanc_1_1013XStringCachedEKFdvE.virtual"}
!34 = !{i32 104, !"_ZTSMN11xalanc_1_1013XStringCachedEKFvRNS_14XalanDOMStringEE.virtual"}
!35 = !{i32 112, !"_ZTSMN11xalanc_1_1013XStringCachedEKFRKNS_21XalanDocumentFragmentEvE.virtual"}
!36 = !{i32 120, !"_ZTSMN11xalanc_1_1013XStringCachedEKFRKNS_15NodeRefListBaseEvE.virtual"}
!37 = !{i32 128, !"_ZTSMN11xalanc_1_1013XStringCachedEFvRNS_19XObjectTypeCallbackEE.virtual"}
!38 = !{i32 136, !"_ZTSMN11xalanc_1_1013XStringCachedEKFvRNS_19XObjectTypeCallbackEE.virtual"}
!39 = !{i32 144, !"_ZTSMN11xalanc_1_1013XStringCachedEKFNS_7XObject11eObjectTypeEvE.virtual"}
!40 = !{i32 16, !"_ZTSN11xalanc_1_1027XalanReferenceCountedObjectE"}
!41 = !{i32 32, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEFvvE.virtual"}
!42 = !{i32 40, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEFvvE.virtual"}
!43 = !{i32 48, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFjvE.virtual"}
!44 = !{i32 56, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFRKNS_14XalanDOMStringEvE.virtual"}
!45 = !{i32 64, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFdvE.virtual"}
!46 = !{i32 72, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFbvE.virtual"}
!47 = !{i32 80, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFRKNS_14XalanDOMStringEvE.virtual"}
!48 = !{i32 88, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFvRNS_17FormatterListenerEMS1_FvPKtjEE.virtual"}
!49 = !{i32 96, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFdvE.virtual"}
!50 = !{i32 104, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFvRNS_14XalanDOMStringEE.virtual"}
!51 = !{i32 112, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFRKNS_21XalanDocumentFragmentEvE.virtual"}
!52 = !{i32 120, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFRKNS_15NodeRefListBaseEvE.virtual"}
!53 = !{i32 128, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEFvRNS_19XObjectTypeCallbackEE.virtual"}
!54 = !{i32 136, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFvRNS_19XObjectTypeCallbackEE.virtual"}
!55 = !{i32 144, !"_ZTSMN11xalanc_1_1027XalanReferenceCountedObjectEKFNS_7XObject11eObjectTypeEvE.virtual"}
!56 = !{i32 16, !"_ZTSN11xalanc_1_107XObjectE"}
!57 = !{i32 32, !"_ZTSMN11xalanc_1_107XObjectEFvvE.virtual"}
!58 = !{i32 40, !"_ZTSMN11xalanc_1_107XObjectEFvvE.virtual"}
!59 = !{i32 48, !"_ZTSMN11xalanc_1_107XObjectEKFjvE.virtual"}
!60 = !{i32 56, !"_ZTSMN11xalanc_1_107XObjectEKFRKNS_14XalanDOMStringEvE.virtual"}
!61 = !{i32 64, !"_ZTSMN11xalanc_1_107XObjectEKFdvE.virtual"}
!62 = !{i32 72, !"_ZTSMN11xalanc_1_107XObjectEKFbvE.virtual"}
!63 = !{i32 80, !"_ZTSMN11xalanc_1_107XObjectEKFRKNS_14XalanDOMStringEvE.virtual"}
!64 = !{i32 88, !"_ZTSMN11xalanc_1_107XObjectEKFvRNS_17FormatterListenerEMS1_FvPKtjEE.virtual"}
!65 = !{i32 96, !"_ZTSMN11xalanc_1_107XObjectEKFdvE.virtual"}
!66 = !{i32 104, !"_ZTSMN11xalanc_1_107XObjectEKFvRNS_14XalanDOMStringEE.virtual"}
!67 = !{i32 112, !"_ZTSMN11xalanc_1_107XObjectEKFRKNS_21XalanDocumentFragmentEvE.virtual"}
!68 = !{i32 120, !"_ZTSMN11xalanc_1_107XObjectEKFRKNS_15NodeRefListBaseEvE.virtual"}
!69 = !{i32 128, !"_ZTSMN11xalanc_1_107XObjectEFvRNS_19XObjectTypeCallbackEE.virtual"}
!70 = !{i32 136, !"_ZTSMN11xalanc_1_107XObjectEKFvRNS_19XObjectTypeCallbackEE.virtual"}
!71 = !{i32 144, !"_ZTSMN11xalanc_1_107XObjectEKFNS0_11eObjectTypeEvE.virtual"}
!72 = !{!"A", i32 19, !6}
!73 = !{i32 16, !"_ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE"}
!74 = !{i32 32, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFPS1_vE.virtual"}
!75 = !{i32 40, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFvPS1_E.virtual"}
!76 = !{i32 48, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEKFbPKS1_E.virtual"}
!77 = !{i32 56, !"_ZTSMN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEEFvvE.virtual"}
!78 = !{i32 16, !"_ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE"}
!79 = !{i32 32, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFPS1_vE.virtual"}
!80 = !{i32 40, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFvPS1_E.virtual"}
!81 = !{i32 48, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEKFbPKS1_E.virtual"}
!82 = !{i32 56, !"_ZTSMN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEEFvvE.virtual"}
!83 = !{!"A", i32 8, !6}
!84 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !85}
!85 = !{!86, i32 2}
!86 = !{!"F", i1 true, i32 0, !87}
!87 = !{i32 0, i32 0}
!88 = !{!"S", %"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1, !89}
!89 = !{i8 0, i32 0}
!90 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 3, !91, !87, !92}
!91 = !{%"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!92 = !{!"A", i32 4, !89}
!93 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorItNS_31MemoryManagedConstructionTraitsItEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !96}
!94 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!95 = !{i64 0, i32 0}
!96 = !{i16 0, i32 1}
!97 = !{!"S", %"class._ZTSN11xalanc_1_103AVTE.xalanc_1_10::AVT" zeroinitializer, i32 6, !85, !98, !95, !96, !87, !99}
!98 = !{%"class._ZTSN11xalanc_1_107AVTPartE.xalanc_1_10::AVTPart" zeroinitializer, i32 2}
!99 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!100 = !{!"S", %"class._ZTSN11xalanc_1_107AVTPartE.xalanc_1_10::AVTPart" zeroinitializer, i32 1, !85}
!101 = !{!"S", %"class._ZTSN11xalanc_1_1024XPathConstructionContextE.xalanc_1_10::XPathConstructionContext" zeroinitializer, i32 2, !85, !94}
!102 = !{!"S", %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 1, !85}
!103 = !{!"S", %"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver" zeroinitializer, i32 1, !85}
!104 = !{!"S", %"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 1, !85}
!105 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 2, !106, !107}
!106 = !{%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 0}
!107 = !{%"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 1}
!108 = !{!"S", %"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 2, !85, !94}
!109 = !{!"S", %"class._ZTSN11xalanc_1_105XPathE.xalanc_1_10::XPath" zeroinitializer, i32 4, !110, !111, !89, !112}
!110 = !{%"class._ZTSN11xalanc_1_1015XPathExpressionE.xalanc_1_10::XPathExpression" zeroinitializer, i32 0}
!111 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 1}
!112 = !{!"A", i32 7, !89}
!113 = !{!"S", %"class._ZTSN11xalanc_1_1015XPathExpressionE.xalanc_1_10::XPathExpression" zeroinitializer, i32 6, !114, !87, !115, !87, !99, !116}
!114 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIiNS_31MemoryManagedConstructionTraitsIiEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!115 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_6XTokenENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!116 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIdNS_31MemoryManagedConstructionTraitsIdEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!117 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIiNS_31MemoryManagedConstructionTraitsIiEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !118}
!118 = !{i32 0, i32 1}
!119 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_6XTokenENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !120}
!120 = !{%"class._ZTSN11xalanc_1_106XTokenE.xalanc_1_10::XToken" zeroinitializer, i32 1}
!121 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIdNS_31MemoryManagedConstructionTraitsIdEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !122}
!122 = !{double 0.000000e+00, i32 1}
!123 = !{!"S", %"class._ZTSN11xalanc_1_106XTokenE.xalanc_1_10::XToken" zeroinitializer, i32 5, !124, !99, !125, !89, !112}
!124 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 0}
!125 = !{double 0.000000e+00, i32 0}
!126 = !{!"S", %"class._ZTSN11xalanc_1_1014XObjectFactoryE.xalanc_1_10::XObjectFactory" zeroinitializer, i32 2, !85, !94}
!127 = !{!"S", %"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 3, !128, !87, !107}
!128 = !{%"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 0}
!129 = !{!"S", %"class._ZTSN11xalanc_1_1027XalanReferenceCountedObjectE.xalanc_1_10::XalanReferenceCountedObject.base" zeroinitializer, i32 2, !85, !87}
!130 = !{!"S", %"class._ZTSN11xalanc_1_1017AttributeListImplE.xalanc_1_10::AttributeListImpl" zeroinitializer, i32 3, !131, !132, !132}
!131 = !{%"class._ZTSN11xercesc_2_713AttributeListE.xercesc_2_7::AttributeList" zeroinitializer, i32 0}
!132 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_20AttributeVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!133 = !{!"S", %"class._ZTSN11xercesc_2_713AttributeListE.xercesc_2_7::AttributeList" zeroinitializer, i32 1, !85}
!134 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_20AttributeVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !135}
!135 = !{%"class._ZTSN11xalanc_1_1020AttributeVectorEntryE.xalanc_1_10::AttributeVectorEntry" zeroinitializer, i32 2}
!136 = !{!"S", %"class._ZTSN11xalanc_1_1020AttributeVectorEntryE.xalanc_1_10::AttributeVectorEntry" zeroinitializer, i32 4, !85, !91, !91, !91}
!137 = !{!"S", %"class._ZTSN11xalanc_1_1013CountersTableE.xalanc_1_10::CountersTable" zeroinitializer, i32 2, !138, !139}
!138 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!139 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!140 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !141}
!141 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!142 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_7CounterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !143}
!143 = !{%"struct._ZTSN11xalanc_1_107CounterE.xalanc_1_10::Counter" zeroinitializer, i32 1}
!144 = !{!"S", %"struct._ZTSN11xalanc_1_107CounterE.xalanc_1_10::Counter" zeroinitializer, i32 4, !95, !139, !145, !146}
!145 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 1}
!146 = !{%"class._ZTSN11xalanc_1_1010ElemNumberE.xalanc_1_10::ElemNumber" zeroinitializer, i32 1}
!147 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_9XalanNodeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !148}
!148 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 2}
!149 = !{!"S", %"class._ZTSN11xalanc_1_1010ElemNumberE.xalanc_1_10::ElemNumber" zeroinitializer, i32 11, !150, !151, !151, !151, !95, !152, !152, !152, !152, !152, !95}
!150 = !{%"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base" zeroinitializer, i32 0}
!151 = !{%"class._ZTSN11xalanc_1_105XPathE.xalanc_1_10::XPath" zeroinitializer, i32 1}
!152 = !{%"class._ZTSN11xalanc_1_103AVTE.xalanc_1_10::AVT" zeroinitializer, i32 1}
!153 = !{!"S", %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement.base" zeroinitializer, i32 11, !154, !155, !156, !87, !92, !157, !157, !157, !158, !159, !160}
!154 = !{%"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver" zeroinitializer, i32 0}
!155 = !{%"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet" zeroinitializer, i32 1}
!156 = !{%"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler" zeroinitializer, i32 0}
!157 = !{%"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 1}
!158 = !{%union._ZTSN11xalanc_1_1019ElemTemplateElementUt_E.anon zeroinitializer, i32 0}
!159 = !{%"class._ZTSN11xalanc_1_1019ElemTemplateElement12LocatorProxyE.xalanc_1_10::ElemTemplateElement::LocatorProxy" zeroinitializer, i32 0}
!160 = !{i16 0, i32 0}
!161 = !{!"S", %"class._ZTSN11xalanc_1_1017NamespacesHandlerE.xalanc_1_10::NamespacesHandler" zeroinitializer, i32 4, !162, !163, !164, !165}
!162 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler9NamespaceENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!163 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler17NamespaceExtendedENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!164 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!165 = !{%"class._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!166 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler9NamespaceENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !167}
!167 = !{%"class._ZTSN11xalanc_1_1017NamespacesHandler9NamespaceE.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 1}
!168 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_17NamespacesHandler17NamespaceExtendedENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !169}
!169 = !{%"class._ZTSN11xalanc_1_1017NamespacesHandler17NamespaceExtendedE.xalanc_1_10::NamespacesHandler::NamespaceExtended" zeroinitializer, i32 1}
!170 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !171}
!171 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 2}
!172 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !173, !174, !94, !175, !95, !95, !176, !176, !177, !95, !95}
!173 = !{%"struct._ZTSN11xalanc_1_1028DOMStringPointerHashFunctionE.xalanc_1_10::DOMStringPointerHashFunction" zeroinitializer, i32 0}
!174 = !{%"struct._ZTSN11xalanc_1_1013pointer_equalINS_14XalanDOMStringEEE.xalanc_1_10::pointer_equal" zeroinitializer, i32 0}
!175 = !{float 0.000000e+00, i32 0}
!176 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!177 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!178 = !{!"S", %"struct._ZTSN11xalanc_1_1028DOMStringPointerHashFunctionE.xalanc_1_10::DOMStringPointerHashFunction" zeroinitializer, i32 1, !89}
!179 = !{!"S", %"struct._ZTSN11xalanc_1_1013pointer_equalINS_14XalanDOMStringEEE.xalanc_1_10::pointer_equal" zeroinitializer, i32 1, !89}
!180 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !181, !181}
!181 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" zeroinitializer, i32 1}
!182 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !183}
!183 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!184 = !{!"S", %union._ZTSN11xalanc_1_1019ElemTemplateElementUt_E.anon zeroinitializer, i32 1, !157}
!185 = !{!"S", %"class._ZTSN11xalanc_1_1019ElemTemplateElement12LocatorProxyE.xalanc_1_10::ElemTemplateElement::LocatorProxy" zeroinitializer, i32 4, !186, !95, !95, !99}
!186 = !{%"class._ZTSN11xalanc_1_1012XalanLocatorE.xalanc_1_10::XalanLocator" zeroinitializer, i32 0}
!187 = !{!"S", %"class._ZTSN11xalanc_1_1012XalanLocatorE.xalanc_1_10::XalanLocator" zeroinitializer, i32 1, !188}
!188 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 0}
!189 = !{!"S", %"class._ZTSN11xalanc_1_1026StylesheetExecutionContextE.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 1, !190}
!190 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 0}
!191 = !{!"S", %"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 12, !154, !155, !156, !87, !92, !157, !157, !157, !158, !159, !160, !192}
!192 = !{!"A", i32 6, !89}
!193 = !{!"S", %"class._ZTSN11xalanc_1_1017NamespacesHandler9NamespaceE.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 2, !99, !99}
!194 = !{!"S", %"class._ZTSN11xalanc_1_1017NamespacesHandler17NamespaceExtendedE.xalanc_1_10::NamespacesHandler::NamespaceExtended" zeroinitializer, i32 2, !195, !99}
!195 = !{%"class._ZTSN11xalanc_1_1017NamespacesHandler9NamespaceE.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 0}
!196 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES6_NS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !197}
!197 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES5_NS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!198 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1, !199}
!199 = !{%"class._ZTSN11xalanc_1_109XalanNodeE.xalanc_1_10::XalanNode" zeroinitializer, i32 0}
!200 = !{!"S", %"class._ZTSN11xalanc_1_1013XalanDocumentE.xalanc_1_10::XalanDocument" zeroinitializer, i32 1, !199}
!201 = !{!"S", %"class._ZTSN11xalanc_1_1012XalanElementE.xalanc_1_10::XalanElement" zeroinitializer, i32 1, !199}
!202 = !{!"S", %"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 1, !203}
!203 = !{%"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 0}
!204 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanCharacterDataE.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 1, !199}
!205 = !{!"S", %"class._ZTSN11xalanc_1_109XalanAttrE.xalanc_1_10::XalanAttr" zeroinitializer, i32 1, !199}
!206 = !{!"S", %"class._ZTSN11xalanc_1_1012XalanCommentE.xalanc_1_10::XalanComment" zeroinitializer, i32 1, !203}
!207 = !{!"S", %"class._ZTSN11xalanc_1_1026XalanProcessingInstructionE.xalanc_1_10::XalanProcessingInstruction" zeroinitializer, i32 1, !199}
!208 = !{!"S", %"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener" zeroinitializer, i32 4, !209, !210, !87, !87}
!209 = !{%"class._ZTSN11xercesc_2_715DocumentHandlerE.xercesc_2_7::DocumentHandler" zeroinitializer, i32 0}
!210 = !{%"class._ZTSN11xalanc_1_1014PrefixResolverE.xalanc_1_10::PrefixResolver" zeroinitializer, i32 1}
!211 = !{!"S", %"class._ZTSN11xercesc_2_715DocumentHandlerE.xercesc_2_7::DocumentHandler" zeroinitializer, i32 1, !85}
!212 = !{!"S", %"class._ZTSN11xalanc_1_1017XalanNamedNodeMapE.xalanc_1_10::XalanNamedNodeMap" zeroinitializer, i32 1, !85}
!213 = !{!"S", %"class._ZTSN11xalanc_1_1017XalanOutputStreamE.xalanc_1_10::XalanOutputStream" zeroinitializer, i32 9, !85, !87, !214, !87, !91, !215, !89, !89, !216}
!214 = !{%"class._ZTSN11xalanc_1_1021XalanOutputTranscoderE.xalanc_1_10::XalanOutputTranscoder" zeroinitializer, i32 1}
!215 = !{%"class._ZTSN11xalanc_1_1014XalanDOMStringE.xalanc_1_10::XalanDOMString" zeroinitializer, i32 0}
!216 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIcNS_31MemoryManagedConstructionTraitsIcEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!217 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIcNS_31MemoryManagedConstructionTraitsIcEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !6}
!218 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanOutputTranscoderE.xalanc_1_10::XalanOutputTranscoder" zeroinitializer, i32 2, !85, !94}
!219 = !{!"S", %"class._ZTSN11xalanc_1_1020DOMStringPrintWriterE.xalanc_1_10::DOMStringPrintWriter" zeroinitializer, i32 2, !220, !99}
!220 = !{%"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter" zeroinitializer, i32 0}
!221 = !{!"S", %"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter" zeroinitializer, i32 3, !222, !89, !94}
!222 = !{%"class._ZTSN11xalanc_1_106WriterE.xalanc_1_10::Writer" zeroinitializer, i32 0}
!223 = !{!"S", %"class._ZTSN11xalanc_1_106WriterE.xalanc_1_10::Writer" zeroinitializer, i32 1, !85}
!224 = !{!"S", %"class._ZTSN11xalanc_1_1010DOMSupportE.xalanc_1_10::DOMSupport" zeroinitializer, i32 1, !85}
!225 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool" zeroinitializer, i32 4, !85, !226, !95, !227}
!226 = !{%"class._ZTSN11xalanc_1_1023XalanDOMStringAllocatorE.xalanc_1_10::XalanDOMStringAllocator" zeroinitializer, i32 0}
!227 = !{%"class._ZTSN11xalanc_1_1023XalanDOMStringHashTableE.xalanc_1_10::XalanDOMStringHashTable" zeroinitializer, i32 0}
!228 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDOMStringAllocatorE.xalanc_1_10::XalanDOMStringAllocator" zeroinitializer, i32 1, !229}
!229 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!230 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !231}
!231 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!232 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !233, !233}
!233 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 1}
!234 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDOMStringHashTableE.xalanc_1_10::XalanDOMStringHashTable" zeroinitializer, i32 6, !95, !95, !235, !95, !87, !92}
!235 = !{%"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEEE.xalanc_1_10::XalanMemMgrAutoPtrArray" zeroinitializer, i32 0}
!236 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEEE.xalanc_1_10::XalanMemMgrAutoPtrArray" zeroinitializer, i32 1, !237}
!237 = !{%"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEE22MemMgrAutoPtrArrayDataE.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" zeroinitializer, i32 0}
!238 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanMemMgrAutoPtrArrayINS_11XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS4_EEEEE22MemMgrAutoPtrArrayDataE.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" zeroinitializer, i32 3, !94, !239, !95}
!239 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!240 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_14XalanDOMStringEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 3, !241, !233, !233}
!241 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!242 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !243}
!243 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!244 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !245, !95, !95, !99}
!245 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!246 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!247 = !{!"S", %"class._ZTSN11xalanc_1_1017XalanQNameByValueE.xalanc_1_10::XalanQNameByValue" zeroinitializer, i32 3, !248, !215, !215}
!248 = !{%"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName" zeroinitializer, i32 0}
!249 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName" zeroinitializer, i32 1, !85}
!250 = !{!"S", %"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet" zeroinitializer, i32 31, !154, !251, !215, !252, !253, !215, !254, !95, !255, !256, !89, !257, !258, !259, !260, !261, !125, !262, !263, !264, !262, !263, !264, !264, !264, !264, !264, !264, !95, !265, !156}
!251 = !{%"class._ZTSN11xalanc_1_1014StylesheetRootE.xalanc_1_10::StylesheetRoot" zeroinitializer, i32 1}
!252 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14KeyDeclarationENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!253 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanSpaceNodeTesterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!254 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_10StylesheetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!255 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!256 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!257 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!258 = !{%"class._ZTSN11xalanc_1_1012ElemTemplateE.xalanc_1_10::ElemTemplate" zeroinitializer, i32 1}
!259 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!260 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!261 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!262 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!263 = !{%"struct._ZTSN11xalanc_1_1016XalanMapIteratorINS_27XalanMapConstIteratorTraitsISt4pairIKNS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS8_EEEEEEENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIS3_SB_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISK_E4NodeEEEEE.xalanc_1_10::XalanMapIterator" zeroinitializer, i32 0}
!264 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!265 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17ElemDecimalFormatENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!266 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14KeyDeclarationENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !267}
!267 = !{%"class._ZTSN11xalanc_1_1014KeyDeclarationE.xalanc_1_10::KeyDeclaration" zeroinitializer, i32 1}
!268 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanSpaceNodeTesterENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !269}
!269 = !{%"class._ZTSN11xalanc_1_1020XalanSpaceNodeTesterE.xalanc_1_10::XalanSpaceNodeTester" zeroinitializer, i32 1}
!270 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_10StylesheetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !271}
!271 = !{%"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet" zeroinitializer, i32 2}
!272 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanDequeINS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !94, !95, !273, !273}
!273 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEENS_31MemoryManagedConstructionTraitsIS8_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!274 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEENS_31MemoryManagedConstructionTraitsIS8_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !275}
!275 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!276 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !94, !95, !277, !277}
!277 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS_31MemoryManagedConstructionTraitsIS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!278 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEENS_31MemoryManagedConstructionTraitsIS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !279}
!279 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!280 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !283, !283, !284, !95, !95}
!281 = !{%"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction" zeroinitializer, i32 0}
!282 = !{%"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to" zeroinitializer, i32 0}
!283 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!284 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!285 = !{!"S", %"struct._ZTSN11xalanc_1_1021DOMStringHashFunctionE.xalanc_1_10::DOMStringHashFunction" zeroinitializer, i32 1, !89}
!286 = !{!"S", %"struct._ZTSSt8equal_toIN11xalanc_1_1014XalanDOMStringEE.std::equal_to" zeroinitializer, i32 1, !89}
!287 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !288, !288}
!288 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" zeroinitializer, i32 1}
!289 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEENS_32ConstructWithMemoryManagerTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !290}
!290 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!291 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !99}
!292 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !293, !294, !94, !175, !95, !95, !295, !295, !296, !95, !95}
!293 = !{%"struct._ZTSN11xalanc_1_1024XalanHashMemberReferenceINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberReference" zeroinitializer, i32 0}
!294 = !{%"struct._ZTSSt8equal_toIN11xalanc_1_1010XalanQNameEE.std::equal_to" zeroinitializer, i32 0}
!295 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!296 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!297 = !{!"S", %"struct._ZTSN11xalanc_1_1024XalanHashMemberReferenceINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberReference" zeroinitializer, i32 1, !89}
!298 = !{!"S", %"struct._ZTSSt8equal_toIN11xalanc_1_1010XalanQNameEE.std::equal_to" zeroinitializer, i32 1, !89}
!299 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !300, !300}
!300 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" zeroinitializer, i32 1}
!301 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !302}
!302 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!303 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !304}
!304 = !{%"class._ZTSN11xalanc_1_1012ElemVariableE.xalanc_1_10::ElemVariable" zeroinitializer, i32 2}
!305 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !306, !306, !307, !95, !95}
!306 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!307 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!308 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !309, !309}
!309 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" zeroinitializer, i32 1}
!310 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !311}
!311 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!312 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISE_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !309}
!313 = !{!"S", %"struct._ZTSN11xalanc_1_1016XalanMapIteratorINS_27XalanMapConstIteratorTraitsISt4pairIKNS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS8_EEEEEEENS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIS3_SB_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISK_E4NodeEEEEE.xalanc_1_10::XalanMapIterator" zeroinitializer, i32 1, !314}
!314 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISE_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 0}
!315 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !316}
!316 = !{%"class._ZTSN11xalanc_1_1021XalanMatchPatternDataE.xalanc_1_10::XalanMatchPatternData" zeroinitializer, i32 2}
!317 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17ElemDecimalFormatENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !318}
!318 = !{%"class._ZTSN11xalanc_1_1017ElemDecimalFormatE.xalanc_1_10::ElemDecimalFormat" zeroinitializer, i32 2}
!319 = !{!"S", %"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 1, !85}
!320 = !{!"S", %"class._ZTSN11xalanc_1_1014StylesheetRootE.xalanc_1_10::StylesheetRoot" zeroinitializer, i32 25, !321, !215, !87, !92, !215, !215, !215, !215, !89, !112, !215, !215, !87, !322, !89, !259, !157, !157, !157, !89, !89, !87, !89, !95, !323}
!321 = !{%"class._ZTSN11xalanc_1_1010StylesheetE.xalanc_1_10::Stylesheet" zeroinitializer, i32 0}
!322 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_10XalanQNameENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!323 = !{%"class._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!324 = !{!"S", %"class._ZTSN11xalanc_1_1014KeyDeclarationE.xalanc_1_10::KeyDeclaration" zeroinitializer, i32 6, !325, !151, !151, !99, !95, !95}
!325 = !{%"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName" zeroinitializer, i32 1}
!326 = !{!"S", %"class._ZTSN11xalanc_1_1020XalanSpaceNodeTesterE.xalanc_1_10::XalanSpaceNodeTester" zeroinitializer, i32 3, !327, !87, !87}
!327 = !{%"class._ZTSN11xalanc_1_105XPath10NodeTesterE.xalanc_1_10::XPath::NodeTester" zeroinitializer, i32 0}
!328 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_10XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !329}
!329 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!330 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_9NameSpaceENS_32ConstructWithMemoryManagerTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !331}
!331 = !{%"class._ZTSN11xalanc_1_109NameSpaceE.xalanc_1_10::NameSpace" zeroinitializer, i32 1}
!332 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" zeroinitializer, i32 3, !333, !288, !288}
!333 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" zeroinitializer, i32 0}
!334 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEENS_31MemoryManagedConstructionTraitsISF_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !335}
!335 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!336 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !337}
!337 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!338 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS0_IPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISE_E4NodeEEENS8_ISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !339}
!339 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISE_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!340 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_10XalanQNameENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !341}
!341 = !{%"class._ZTSN11xalanc_1_1010XalanQNameE.xalanc_1_10::XalanQName" zeroinitializer, i32 2}
!342 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !343, !344, !94, !175, !95, !95, !345, !345, !346, !95, !95}
!343 = !{%"struct._ZTSN11xalanc_1_1022XalanHashMemberPointerINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberPointer" zeroinitializer, i32 0}
!344 = !{%"struct._ZTSN11xalanc_1_1013pointer_equalINS_10XalanQNameEEE.xalanc_1_10::pointer_equal" zeroinitializer, i32 0}
!345 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!346 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEENS_32ConstructWithMemoryManagerTraitsISM_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!347 = !{!"S", %"class._ZTSN11xalanc_1_105XPath10NodeTesterE.xalanc_1_10::XPath::NodeTester" zeroinitializer, i32 5, !348, !99, !99, !349, !349}
!348 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContextE.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 1}
!349 = !{!350, i32 0}
!350 = !{!"L", i32 2, !95, !95}
!351 = !{!"S", %"class._ZTSN11xalanc_1_109NameSpaceE.xalanc_1_10::NameSpace" zeroinitializer, i32 2, !215, !215}
!352 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" zeroinitializer, i32 3, !353, !89, !112}
!353 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEPNS0_18ExtensionNSHandlerEE.std::pair" zeroinitializer, i32 1}
!354 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEPNS_18ExtensionNSHandlerENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !288}
!355 = !{!"S", %"struct._ZTSN11xalanc_1_1022XalanHashMemberPointerINS_10XalanQNameEEE.xalanc_1_10::XalanHashMemberPointer" zeroinitializer, i32 1, !89}
!356 = !{!"S", %"struct._ZTSN11xalanc_1_1013pointer_equalINS_10XalanQNameEEE.xalanc_1_10::pointer_equal" zeroinitializer, i32 1, !89}
!357 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !358, !358}
!358 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" zeroinitializer, i32 1}
!359 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEENS_32ConstructWithMemoryManagerTraitsISM_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !360}
!360 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!361 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEPNS0_18ExtensionNSHandlerEE.std::pair" zeroinitializer, i32 2, !215, !362}
!362 = !{%"class._ZTSN11xalanc_1_1018ExtensionNSHandlerE.xalanc_1_10::ExtensionNSHandler" zeroinitializer, i32 1}
!363 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS0_IPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISF_E4NodeEEENS9_ISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !364}
!364 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISF_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!365 = !{!"S", %"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !348, !99}
!366 = !{!"S", %"class._ZTSN11xalanc_1_1016ElemAttributeSetE.xalanc_1_10::ElemAttributeSet" zeroinitializer, i32 2, !367, !325}
!367 = !{%"class._ZTSN11xalanc_1_107ElemUseE.xalanc_1_10::ElemUse" zeroinitializer, i32 0}
!368 = !{!"S", %"class._ZTSN11xalanc_1_107ElemUseE.xalanc_1_10::ElemUse" zeroinitializer, i32 3, !150, !341, !95}
!369 = !{!"S", %"class._ZTSN11xalanc_1_1012ElemTemplateE.xalanc_1_10::ElemTemplate" zeroinitializer, i32 5, !150, !151, !325, !325, !125}
!370 = !{!"S", %"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr" zeroinitializer, i32 1, !371}
!371 = !{%"class._ZTSN11xalanc_1_107XObjectE.xalanc_1_10::XObject" zeroinitializer, i32 1}
!372 = !{!"S", %"class._ZTSN11xalanc_1_1018MutableNodeRefListE.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 3, !373, !87, !92}
!373 = !{%"class._ZTSN11xalanc_1_1011NodeRefListE.xalanc_1_10::NodeRefList" zeroinitializer, i32 0}
!374 = !{!"S", %"class._ZTSN11xalanc_1_1011NodeRefListE.xalanc_1_10::NodeRefList" zeroinitializer, i32 2, !375, !139}
!375 = !{%"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 0}
!376 = !{!"S", %"class._ZTSN11xalanc_1_1017ElemDecimalFormatE.xalanc_1_10::ElemDecimalFormat" zeroinitializer, i32 6, !150, !151, !151, !151, !325, !377}
!377 = !{%"class._ZTSN11xalanc_1_1025XalanDecimalFormatSymbolsE.xalanc_1_10::XalanDecimalFormatSymbols" zeroinitializer, i32 0}
!378 = !{!"S", %"class._ZTSN11xalanc_1_1025XalanDecimalFormatSymbolsE.xalanc_1_10::XalanDecimalFormatSymbols" zeroinitializer, i32 15, !215, !160, !160, !160, !379, !215, !215, !160, !160, !92, !215, !160, !160, !160, !160}
!379 = !{!"A", i32 2, !89}
!380 = !{!"S", %"class._ZTSN11xalanc_1_1010NodeSorterE.xalanc_1_10::NodeSorter" zeroinitializer, i32 4, !381, !382, !383, !384}
!381 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_IdNS_31MemoryManagedConstructionTraitsIdEEEENS1_IS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!382 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!383 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_11NodeSortKeyENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!384 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_10NodeSorter11VectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!385 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_IdNS_31MemoryManagedConstructionTraitsIdEEEENS1_IS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !386}
!386 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIdNS_31MemoryManagedConstructionTraitsIdEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!387 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !388}
!388 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!389 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_11NodeSortKeyENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !390}
!390 = !{%"class._ZTSN11xalanc_1_1011NodeSortKeyE.xalanc_1_10::NodeSortKey" zeroinitializer, i32 1}
!391 = !{!"S", %"class._ZTSN11xalanc_1_1011NodeSortKeyE.xalanc_1_10::NodeSortKey" zeroinitializer, i32 7, !392, !151, !89, !89, !87, !210, !99}
!392 = !{%"class._ZTSN11xalanc_1_1016ExecutionContextE.xalanc_1_10::ExecutionContext" zeroinitializer, i32 1}
!393 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_10NodeSorter11VectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !394}
!394 = !{%"struct._ZTSN11xalanc_1_1010NodeSorter11VectorEntryE.xalanc_1_10::NodeSorter::VectorEntry" zeroinitializer, i32 1}
!395 = !{!"S", %"struct._ZTSN11xalanc_1_1010NodeSorter11VectorEntryE.xalanc_1_10::NodeSorter::VectorEntry" zeroinitializer, i32 3, !145, !87, !92}
!396 = !{!"S", %"class._ZTSN11xalanc_1_1012ElemVariableE.xalanc_1_10::ElemVariable" zeroinitializer, i32 6, !150, !325, !151, !89, !397, !145}
!397 = !{%"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr" zeroinitializer, i32 0}
!398 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanQNameByReferenceE.xalanc_1_10::XalanQNameByReference" zeroinitializer, i32 3, !248, !99, !99}
!399 = !{!"S", %"struct._ZTSN11xalanc_1_1026StylesheetExecutionContext22UseAttributeSetIndexesE.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" zeroinitializer, i32 2, !87, !87}
!400 = !{!"S", %"class._ZTSN11xalanc_1_1015XPathEnvSupportE.xalanc_1_10::XPathEnvSupport" zeroinitializer, i32 1, !85}
!401 = !{!"S", %"class._ZTSN11xalanc_1_108XalanSetINS_14XalanDOMStringEEE.xalanc_1_10::XalanSet" zeroinitializer, i32 1, !402}
!402 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!403 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !404, !404, !405, !95, !95}
!404 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!405 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!406 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !407, !407}
!407 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" zeroinitializer, i32 1}
!408 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" zeroinitializer, i32 3, !409, !407, !407}
!409 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" zeroinitializer, i32 0}
!410 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" zeroinitializer, i32 3, !411, !89, !112}
!411 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEbE.std::pair" zeroinitializer, i32 1}
!412 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringEbE.std::pair" zeroinitializer, i32 3, !215, !89, !112}
!413 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !414}
!414 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!415 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !416}
!416 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!417 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringEbNS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !407}
!418 = !{!"S", %"struct._ZTSN11xalanc_1_1027hash_null_terminated_arraysItEE.xalanc_1_10::hash_null_terminated_arrays" zeroinitializer, i32 1, !89}
!419 = !{!"S", %"class._ZTSN11xalanc_1_1018ExtensionNSHandlerE.xalanc_1_10::ExtensionNSHandler" zeroinitializer, i32 5, !420, !112, !421, !89, !112}
!420 = !{%"class._ZTSN11xalanc_1_1024ExtensionFunctionHandlerE.xalanc_1_10::ExtensionFunctionHandler.base" zeroinitializer, i32 0}
!421 = !{%"class._ZTSN11xalanc_1_108XalanSetINS_14XalanDOMStringEEE.xalanc_1_10::XalanSet" zeroinitializer, i32 0}
!422 = !{!"S", %"class._ZTSN11xalanc_1_1024ExtensionFunctionHandlerE.xalanc_1_10::ExtensionFunctionHandler.base" zeroinitializer, i32 8, !85, !215, !215, !215, !215, !6, !421, !89}
!423 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !6}
!424 = !{!"S", %"class._ZTSN11xalanc_1_1021FormatterToSourceTreeE.xalanc_1_10::FormatterToSourceTree" zeroinitializer, i32 8, !425, !426, !427, !428, !429, !145, !139, !215}
!425 = !{%"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener" zeroinitializer, i32 0}
!426 = !{%"class._ZTSN11xalanc_1_1023XalanSourceTreeDocumentE.xalanc_1_10::XalanSourceTreeDocument" zeroinitializer, i32 1}
!427 = !{%"class._ZTSN11xalanc_1_1031XalanSourceTreeDocumentFragmentE.xalanc_1_10::XalanSourceTreeDocumentFragment" zeroinitializer, i32 1}
!428 = !{%"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 1}
!429 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_22XalanSourceTreeElementENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!430 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanSourceTreeDocumentE.xalanc_1_10::XalanSourceTreeDocument" zeroinitializer, i32 22, !431, !145, !428, !432, !433, !434, !435, !436, !437, !438, !439, !440, !441, !442, !442, !443, !95, !89, !444, !445, !226, !215}
!431 = !{%"class._ZTSN11xalanc_1_1013XalanDocumentE.xalanc_1_10::XalanDocument" zeroinitializer, i32 0}
!432 = !{%"class._ZTSN11xalanc_1_1033XalanSourceTreeAttributeAllocatorE.xalanc_1_10::XalanSourceTreeAttributeAllocator" zeroinitializer, i32 0}
!433 = !{%"class._ZTSN11xalanc_1_1035XalanSourceTreeAttributeNSAllocatorE.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" zeroinitializer, i32 0}
!434 = !{%"class._ZTSN11xalanc_1_1031XalanSourceTreeCommentAllocatorE.xalanc_1_10::XalanSourceTreeCommentAllocator" zeroinitializer, i32 0}
!435 = !{%"class._ZTSN11xalanc_1_1032XalanSourceTreeElementAAllocatorE.xalanc_1_10::XalanSourceTreeElementAAllocator" zeroinitializer, i32 0}
!436 = !{%"class._ZTSN11xalanc_1_1034XalanSourceTreeElementANSAllocatorE.xalanc_1_10::XalanSourceTreeElementANSAllocator" zeroinitializer, i32 0}
!437 = !{%"class._ZTSN11xalanc_1_1033XalanSourceTreeElementNAAllocatorE.xalanc_1_10::XalanSourceTreeElementNAAllocator" zeroinitializer, i32 0}
!438 = !{%"class._ZTSN11xalanc_1_1035XalanSourceTreeElementNANSAllocatorE.xalanc_1_10::XalanSourceTreeElementNANSAllocator" zeroinitializer, i32 0}
!439 = !{%"class._ZTSN11xalanc_1_1045XalanSourceTreeProcessingInstructionAllocatorE.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" zeroinitializer, i32 0}
!440 = !{%"class._ZTSN11xalanc_1_1028XalanSourceTreeTextAllocatorE.xalanc_1_10::XalanSourceTreeTextAllocator" zeroinitializer, i32 0}
!441 = !{%"class._ZTSN11xalanc_1_1031XalanSourceTreeTextIWSAllocatorE.xalanc_1_10::XalanSourceTreeTextIWSAllocator" zeroinitializer, i32 0}
!442 = !{%"class._ZTSN11xalanc_1_1018XalanDOMStringPoolE.xalanc_1_10::XalanDOMStringPool" zeroinitializer, i32 0}
!443 = !{%"class._ZTSN11xalanc_1_1019XalanArrayAllocatorIPNS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanArrayAllocator" zeroinitializer, i32 0}
!444 = !{%"class._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!445 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!446 = !{!"S", %"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 9, !447, !99, !94, !426, !145, !145, !145, !145, !95}
!447 = !{%"class._ZTSN11xalanc_1_1012XalanElementE.xalanc_1_10::XalanElement" zeroinitializer, i32 0}
!448 = !{!"S", %"class._ZTSN11xalanc_1_1033XalanSourceTreeAttributeAllocatorE.xalanc_1_10::XalanSourceTreeAttributeAllocator" zeroinitializer, i32 1, !449}
!449 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeAttrENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!450 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeAttrENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !451}
!451 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!452 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !453, !453}
!453 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" zeroinitializer, i32 1}
!454 = !{!"S", %"class._ZTSN11xalanc_1_1035XalanSourceTreeAttributeNSAllocatorE.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" zeroinitializer, i32 1, !455}
!455 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_21XalanSourceTreeAttrNSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!456 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_21XalanSourceTreeAttrNSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !457}
!457 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!458 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !459, !459}
!459 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" zeroinitializer, i32 1}
!460 = !{!"S", %"class._ZTSN11xalanc_1_1031XalanSourceTreeCommentAllocatorE.xalanc_1_10::XalanSourceTreeCommentAllocator" zeroinitializer, i32 1, !461}
!461 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeCommentENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!462 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeCommentENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !463}
!463 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!464 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !465, !465}
!465 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" zeroinitializer, i32 1}
!466 = !{!"S", %"class._ZTSN11xalanc_1_1032XalanSourceTreeElementAAllocatorE.xalanc_1_10::XalanSourceTreeElementAAllocator" zeroinitializer, i32 1, !467}
!467 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeElementAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!468 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeElementAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !469}
!469 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!470 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !471, !471}
!471 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" zeroinitializer, i32 1}
!472 = !{!"S", %"class._ZTSN11xalanc_1_1034XalanSourceTreeElementANSAllocatorE.xalanc_1_10::XalanSourceTreeElementANSAllocator" zeroinitializer, i32 1, !473}
!473 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_25XalanSourceTreeElementANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!474 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_25XalanSourceTreeElementANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !475}
!475 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!476 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !477, !477}
!477 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" zeroinitializer, i32 1}
!478 = !{!"S", %"class._ZTSN11xalanc_1_1033XalanSourceTreeElementNAAllocatorE.xalanc_1_10::XalanSourceTreeElementNAAllocator" zeroinitializer, i32 1, !479}
!479 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_24XalanSourceTreeElementNAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!480 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_24XalanSourceTreeElementNAENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !481}
!481 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!482 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !483, !483}
!483 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" zeroinitializer, i32 1}
!484 = !{!"S", %"class._ZTSN11xalanc_1_1035XalanSourceTreeElementNANSAllocatorE.xalanc_1_10::XalanSourceTreeElementNANSAllocator" zeroinitializer, i32 1, !485}
!485 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_26XalanSourceTreeElementNANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!486 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_26XalanSourceTreeElementNANSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !487}
!487 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!488 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !489, !489}
!489 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" zeroinitializer, i32 1}
!490 = !{!"S", %"class._ZTSN11xalanc_1_1045XalanSourceTreeProcessingInstructionAllocatorE.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" zeroinitializer, i32 1, !491}
!491 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_36XalanSourceTreeProcessingInstructionENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!492 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_36XalanSourceTreeProcessingInstructionENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !493}
!493 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!494 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !495, !495}
!495 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" zeroinitializer, i32 1}
!496 = !{!"S", %"class._ZTSN11xalanc_1_1028XalanSourceTreeTextAllocatorE.xalanc_1_10::XalanSourceTreeTextAllocator" zeroinitializer, i32 1, !497}
!497 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeTextENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!498 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_19XalanSourceTreeTextENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !499}
!499 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!500 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !501, !501}
!501 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" zeroinitializer, i32 1}
!502 = !{!"S", %"class._ZTSN11xalanc_1_1031XalanSourceTreeTextIWSAllocatorE.xalanc_1_10::XalanSourceTreeTextIWSAllocator" zeroinitializer, i32 1, !503}
!503 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeTextIWSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!504 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_22XalanSourceTreeTextIWSENS_10ArenaBlockIS1_mEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !95, !505}
!505 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!506 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !507, !507}
!507 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" zeroinitializer, i32 1}
!508 = !{!"S", %"class._ZTSN11xalanc_1_1019XalanArrayAllocatorIPNS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanArrayAllocator" zeroinitializer, i32 3, !509, !95, !510}
!509 = !{%"class._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!510 = !{%"struct._ZTSSt4pairImPN11xalanc_1_1011XalanVectorIPNS0_19XalanSourceTreeAttrENS0_31MemoryManagedConstructionTraitsIS3_EEEEE.std::pair" zeroinitializer, i32 1}
!511 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !512, !512}
!512 = !{%"struct._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEE4NodeE.xalanc_1_10::XalanList<std::pair<unsigned long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" zeroinitializer, i32 1}
!513 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !514, !515, !94, !175, !95, !95, !516, !516, !517, !95, !95}
!514 = !{%"struct._ZTSN11xalanc_1_1027hash_null_terminated_arraysItEE.xalanc_1_10::hash_null_terminated_arrays" zeroinitializer, i32 0}
!515 = !{%"struct._ZTSN11xalanc_1_1028equal_null_terminated_arraysItEE.xalanc_1_10::equal_null_terminated_arrays" zeroinitializer, i32 0}
!516 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!517 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!518 = !{!"S", %"struct._ZTSN11xalanc_1_1028equal_null_terminated_arraysItEE.xalanc_1_10::equal_null_terminated_arrays" zeroinitializer, i32 1, !89}
!519 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !520, !520}
!520 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" zeroinitializer, i32 1}
!521 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEENS_32ConstructWithMemoryManagerTraitsISJ_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !522}
!522 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!523 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !524, !524, !525, !95, !95}
!524 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!525 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!526 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !527, !527}
!527 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" zeroinitializer, i32 1}
!528 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEENS_32ConstructWithMemoryManagerTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !529}
!529 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!530 = !{!"S", %"class._ZTSN11xalanc_1_1031XalanSourceTreeDocumentFragmentE.xalanc_1_10::XalanSourceTreeDocumentFragment" zeroinitializer, i32 4, !531, !94, !426, !145}
!531 = !{%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 0}
!532 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_22XalanSourceTreeElementENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !533}
!533 = !{%"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 2}
!534 = !{!"S", %"class._ZTSN11xalanc_1_1019XalanSourceTreeTextE.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 6, !535, !99, !145, !145, !145, !95}
!535 = !{%"class._ZTSN11xalanc_1_109XalanTextE.xalanc_1_10::XalanText" zeroinitializer, i32 0}
!536 = !{!"S", %"class._ZTSN11xalanc_1_1022XalanSourceTreeCommentE.xalanc_1_10::XalanSourceTreeComment" zeroinitializer, i32 7, !537, !99, !426, !145, !145, !145, !95}
!537 = !{%"class._ZTSN11xalanc_1_1012XalanCommentE.xalanc_1_10::XalanComment" zeroinitializer, i32 0}
!538 = !{!"S", %"class._ZTSN11xalanc_1_1036XalanSourceTreeProcessingInstructionE.xalanc_1_10::XalanSourceTreeProcessingInstruction" zeroinitializer, i32 8, !539, !99, !99, !426, !145, !145, !145, !95}
!539 = !{%"class._ZTSN11xalanc_1_1026XalanProcessingInstructionE.xalanc_1_10::XalanProcessingInstruction" zeroinitializer, i32 0}
!540 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeAttrEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" zeroinitializer, i32 3, !541, !453, !453}
!541 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!542 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_21XalanSourceTreeAttrNSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" zeroinitializer, i32 3, !543, !459, !459}
!543 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!544 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeCommentEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" zeroinitializer, i32 3, !545, !465, !465}
!545 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!546 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_23XalanSourceTreeElementAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" zeroinitializer, i32 3, !547, !471, !471}
!547 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!548 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_25XalanSourceTreeElementANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" zeroinitializer, i32 3, !549, !477, !477}
!549 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!550 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_24XalanSourceTreeElementNAEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" zeroinitializer, i32 3, !551, !483, !483}
!551 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!552 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_26XalanSourceTreeElementNANSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" zeroinitializer, i32 3, !553, !489, !489}
!553 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!554 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" zeroinitializer, i32 3, !555, !495, !495}
!555 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!556 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_19XalanSourceTreeTextEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" zeroinitializer, i32 3, !557, !501, !501}
!557 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!558 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_10ArenaBlockINS_22XalanSourceTreeTextIWSEmEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" zeroinitializer, i32 3, !559, !507, !507}
!559 = !{%"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!560 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEENS_31MemoryManagedConstructionTraitsISG_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !561}
!561 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!562 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS8_E4NodeEEENS_31MemoryManagedConstructionTraitsISD_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !563}
!563 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!564 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !565}
!565 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!566 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !567}
!567 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!568 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !569}
!569 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!570 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !571}
!571 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!572 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !573}
!573 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!574 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !575}
!575 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!576 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !577}
!577 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!578 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !579}
!579 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!580 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !581}
!581 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!582 = !{!"S", %"class._ZTSN11xalanc_1_1010ArenaBlockINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !583}
!583 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!584 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeAttrEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !585, !95, !95, !586}
!585 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!586 = !{%"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 1}
!587 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_21XalanSourceTreeAttrNSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !588, !95, !95, !589}
!588 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_21XalanSourceTreeAttrNSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!589 = !{%"class._ZTSN11xalanc_1_1021XalanSourceTreeAttrNSE.xalanc_1_10::XalanSourceTreeAttrNS" zeroinitializer, i32 1}
!590 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeCommentEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !591, !95, !95, !592}
!591 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeCommentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!592 = !{%"class._ZTSN11xalanc_1_1022XalanSourceTreeCommentE.xalanc_1_10::XalanSourceTreeComment" zeroinitializer, i32 1}
!593 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeElementAEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !594, !95, !95, !595}
!594 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeElementAEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!595 = !{%"class._ZTSN11xalanc_1_1023XalanSourceTreeElementAE.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 1}
!596 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_25XalanSourceTreeElementANSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !597, !95, !95, !598}
!597 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_25XalanSourceTreeElementANSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!598 = !{%"class._ZTSN11xalanc_1_1025XalanSourceTreeElementANSE.xalanc_1_10::XalanSourceTreeElementANS" zeroinitializer, i32 1}
!599 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_24XalanSourceTreeElementNAEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !600, !95, !95, !601}
!600 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_24XalanSourceTreeElementNAEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!601 = !{%"class._ZTSN11xalanc_1_1024XalanSourceTreeElementNAE.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 1}
!602 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_26XalanSourceTreeElementNANSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !603, !95, !95, !604}
!603 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_26XalanSourceTreeElementNANSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!604 = !{%"class._ZTSN11xalanc_1_1026XalanSourceTreeElementNANSE.xalanc_1_10::XalanSourceTreeElementNANS" zeroinitializer, i32 1}
!605 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_36XalanSourceTreeProcessingInstructionEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !606, !95, !95, !607}
!606 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_36XalanSourceTreeProcessingInstructionEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!607 = !{%"class._ZTSN11xalanc_1_1036XalanSourceTreeProcessingInstructionE.xalanc_1_10::XalanSourceTreeProcessingInstruction" zeroinitializer, i32 1}
!608 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_19XalanSourceTreeTextEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !609, !95, !95, !610}
!609 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeTextEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!610 = !{%"class._ZTSN11xalanc_1_1019XalanSourceTreeTextE.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 1}
!611 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_22XalanSourceTreeTextIWSEmEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !612, !95, !95, !613}
!612 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeTextIWSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!613 = !{%"class._ZTSN11xalanc_1_1022XalanSourceTreeTextIWSE.xalanc_1_10::XalanSourceTreeTextIWS" zeroinitializer, i32 1}
!614 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeAttrEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!615 = !{!"S", %"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 5, !616, !99, !99, !428, !95}
!616 = !{%"class._ZTSN11xalanc_1_109XalanAttrE.xalanc_1_10::XalanAttr" zeroinitializer, i32 0}
!617 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_21XalanSourceTreeAttrNSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!618 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanSourceTreeAttrNSE.xalanc_1_10::XalanSourceTreeAttrNS" zeroinitializer, i32 4, !619, !99, !99, !99}
!619 = !{%"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 0}
!620 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeCommentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!621 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeElementAEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!622 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanSourceTreeElementAE.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 4, !623, !624, !625, !95}
!623 = !{%"class._ZTSN11xalanc_1_1022XalanSourceTreeElementE.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 0}
!624 = !{%"class._ZTSN11xalanc_1_1017XalanNamedNodeMapE.xalanc_1_10::XalanNamedNodeMap" zeroinitializer, i32 0}
!625 = !{%"class._ZTSN11xalanc_1_1019XalanSourceTreeAttrE.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 2}
!626 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_25XalanSourceTreeElementANSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!627 = !{!"S", %"class._ZTSN11xalanc_1_1025XalanSourceTreeElementANSE.xalanc_1_10::XalanSourceTreeElementANS" zeroinitializer, i32 4, !628, !99, !99, !99}
!628 = !{%"class._ZTSN11xalanc_1_1023XalanSourceTreeElementAE.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 0}
!629 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_24XalanSourceTreeElementNAEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!630 = !{!"S", %"class._ZTSN11xalanc_1_1024XalanSourceTreeElementNAE.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 1, !623}
!631 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_26XalanSourceTreeElementNANSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!632 = !{!"S", %"class._ZTSN11xalanc_1_1026XalanSourceTreeElementNANSE.xalanc_1_10::XalanSourceTreeElementNANS" zeroinitializer, i32 4, !633, !99, !99, !99}
!633 = !{%"class._ZTSN11xalanc_1_1024XalanSourceTreeElementNAE.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 0}
!634 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_36XalanSourceTreeProcessingInstructionEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!635 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_19XalanSourceTreeTextEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!636 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_22XalanSourceTreeTextIWSEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!637 = !{!"S", %"class._ZTSN11xalanc_1_1022XalanSourceTreeTextIWSE.xalanc_1_10::XalanSourceTreeTextIWS" zeroinitializer, i32 1, !638}
!638 = !{%"class._ZTSN11xalanc_1_1019XalanSourceTreeTextE.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 0}
!639 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_10XObjectPtrENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !640}
!640 = !{%"class._ZTSN11xalanc_1_1010XObjectPtrE.xalanc_1_10::XObjectPtr" zeroinitializer, i32 1}
!641 = !{!"S", %"class._ZTSN11xalanc_1_1014XPathProcessorE.xalanc_1_10::XPathProcessor" zeroinitializer, i32 1, !85}
!642 = !{!"S", %"class._ZTSN11xalanc_1_1031XPathConstructionContextDefaultE.xalanc_1_10::XPathConstructionContextDefault" zeroinitializer, i32 3, !643, !442, !644}
!643 = !{%"class._ZTSN11xalanc_1_1024XPathConstructionContextE.xalanc_1_10::XPathConstructionContext" zeroinitializer, i32 0}
!644 = !{%"class._ZTSN11xalanc_1_1019XalanDOMStringCacheE.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 0}
!645 = !{!"S", %"class._ZTSN11xalanc_1_1019XalanDOMStringCacheE.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 5, !646, !646, !87, !92, !647}
!646 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!647 = !{%"class._ZTSN11xalanc_1_1031XalanDOMStringReusableAllocatorE.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 0}
!648 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_14XalanDOMStringENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !171}
!649 = !{!"S", %"class._ZTSN11xalanc_1_1031XalanDOMStringReusableAllocatorE.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 1, !650}
!650 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!651 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_14XalanDOMStringEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !652, !89, !112}
!652 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!653 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_14XalanDOMStringENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !160, !654}
!654 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!655 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !656, !656}
!656 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 1}
!657 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_14XalanDOMStringEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 3, !658, !656, !656}
!658 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_14XalanDOMStringEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!659 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_14XalanDOMStringEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !660, !160, !160, !92}
!660 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!661 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_14XalanDOMStringEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !245, !160, !160, !99}
!662 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !531}
!663 = !{!"S", %"class._ZTSN11xalanc_1_108KeyTableE.xalanc_1_10::KeyTable" zeroinitializer, i32 3, !85, !664, !665}
!664 = !{%"class._ZTSN11xalanc_1_1013XalanDocumentE.xalanc_1_10::XalanDocument" zeroinitializer, i32 1}
!665 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!666 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !293, !294, !94, !175, !95, !95, !667, !667, !668, !95, !95}
!667 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!668 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!669 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !670, !670}
!670 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" zeroinitializer, i32 1}
!671 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceENS1_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEEENS5_IS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" zeroinitializer, i32 3, !672, !670, !670}
!672 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" zeroinitializer, i32 0}
!673 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceENS0_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEEENS4_IS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" zeroinitializer, i32 3, !674, !89, !112}
!674 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceENS0_8XalanMapINS0_14XalanDOMStringENS0_18MutableNodeRefListENS0_17XalanMapKeyTraitsIS4_EEEEE.std::pair" zeroinitializer, i32 1}
!675 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceENS0_8XalanMapINS0_14XalanDOMStringENS0_18MutableNodeRefListENS0_17XalanMapKeyTraitsIS4_EEEEE.std::pair" zeroinitializer, i32 2, !676, !677}
!676 = !{%"class._ZTSN11xalanc_1_1021XalanQNameByReferenceE.xalanc_1_10::XalanQNameByReference" zeroinitializer, i32 0}
!677 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!678 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !679, !679, !680, !95, !95}
!679 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!680 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEENS_32ConstructWithMemoryManagerTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!681 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !682, !682}
!682 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" zeroinitializer, i32 1}
!683 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" zeroinitializer, i32 3, !684, !682, !682}
!684 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" zeroinitializer, i32 0}
!685 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" zeroinitializer, i32 3, !686, !89, !112}
!686 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_18MutableNodeRefListEE.std::pair" zeroinitializer, i32 1}
!687 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_18MutableNodeRefListEE.std::pair" zeroinitializer, i32 2, !215, !688}
!688 = !{%"class._ZTSN11xalanc_1_1018MutableNodeRefListE.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 0}
!689 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEENS_32ConstructWithMemoryManagerTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !690}
!690 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!691 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListIS9_E4NodeEEENS_31MemoryManagedConstructionTraitsISE_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !692}
!692 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS8_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!693 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS8_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !682}
!694 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !695}
!695 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!696 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS3_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS5_EEEENS7_IS4_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !697}
!697 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS2_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEEENS6_IS3_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!698 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceENS2_INS_14XalanDOMStringENS_18MutableNodeRefListENS_17XalanMapKeyTraitsIS4_EEEENS6_IS3_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !670}
!699 = !{!"S", %"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1, !700}
!700 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 0}
!701 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_14XalanDOMStringES4_NS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" zeroinitializer, i32 3, !702, !181, !181}
!702 = !{%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" zeroinitializer, i32 0}
!703 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" zeroinitializer, i32 3, !704, !89, !112}
!704 = !{%"struct._ZTSSt4pairIKPKN11xalanc_1_1014XalanDOMStringES3_E.std::pair" zeroinitializer, i32 1}
!705 = !{!"S", %"struct._ZTSSt4pairIKPKN11xalanc_1_1014XalanDOMStringES3_E.std::pair" zeroinitializer, i32 2, !99, !99}
!706 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_14XalanDOMStringES5_NS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListIS9_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !181}
!707 = !{!"S", %"class._ZTSN11xalanc_1_1018OutputContextStackE.xalanc_1_10::OutputContextStack" zeroinitializer, i32 3, !708, !709, !95}
!708 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!709 = !{%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_18OutputContextStack13OutputContextEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!710 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanDequeINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !94, !95, !711, !711}
!711 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!712 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !713}
!713 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!714 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !715}
!715 = !{%"struct._ZTSN11xalanc_1_1018OutputContextStack13OutputContextE.xalanc_1_10::OutputContextStack::OutputContext" zeroinitializer, i32 1}
!716 = !{!"S", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_18OutputContextStack13OutputContextEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !717, !95}
!717 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_18OutputContextStack13OutputContextENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!718 = !{!"S", %"struct._ZTSN11xalanc_1_1018OutputContextStack13OutputContextE.xalanc_1_10::OutputContextStack::OutputContext" zeroinitializer, i32 6, !719, !720, !215, !89, !89, !192}
!719 = !{%"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener" zeroinitializer, i32 1}
!720 = !{%"class._ZTSN11xalanc_1_1017AttributeListImplE.xalanc_1_10::AttributeListImpl" zeroinitializer, i32 0}
!721 = !{!"S", %"class._ZTSN11xalanc_1_1015ProblemListenerE.xalanc_1_10::ProblemListener" zeroinitializer, i32 1, !85}
!722 = !{!"S", %"class._ZTSN11xalanc_1_1022ProblemListenerDefaultE.xalanc_1_10::ProblemListenerDefault" zeroinitializer, i32 3, !723, !94, !724}
!723 = !{%"class._ZTSN11xalanc_1_1015ProblemListenerE.xalanc_1_10::ProblemListener" zeroinitializer, i32 0}
!724 = !{%"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter" zeroinitializer, i32 1}
!725 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" zeroinitializer, i32 3, !726, !300, !300}
!726 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" zeroinitializer, i32 0}
!727 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" zeroinitializer, i32 3, !728, !89, !112}
!728 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceEPKNS0_12ElemTemplateEE.std::pair" zeroinitializer, i32 1}
!729 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1021XalanQNameByReferenceEPKNS0_12ElemTemplateEE.std::pair" zeroinitializer, i32 2, !676, !258}
!730 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_21XalanQNameByReferenceEPKNS_12ElemTemplateENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !300}
!731 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" zeroinitializer, i32 3, !732, !309, !309}
!732 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" zeroinitializer, i32 0}
!733 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringENS_11XalanVectorIPKNS_21XalanMatchPatternDataENS_31MemoryManagedConstructionTraitsIS5_EEEENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" zeroinitializer, i32 3, !734, !89, !112}
!734 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_11XalanVectorIPKNS0_21XalanMatchPatternDataENS0_31MemoryManagedConstructionTraitsIS6_EEEEE.std::pair" zeroinitializer, i32 1}
!735 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringENS0_11XalanVectorIPKNS0_21XalanMatchPatternDataENS0_31MemoryManagedConstructionTraitsIS6_EEEEE.std::pair" zeroinitializer, i32 2, !215, !264}
!736 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanMatchPatternDataE.xalanc_1_10::XalanMatchPatternData" zeroinitializer, i32 7, !258, !95, !215, !151, !99, !87, !92}
!737 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_11TopLevelArgENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !738}
!738 = !{%"class._ZTSN11xalanc_1_1011TopLevelArgE.xalanc_1_10::TopLevelArg" zeroinitializer, i32 1}
!739 = !{!"S", %"class._ZTSN11xalanc_1_1011TopLevelArgE.xalanc_1_10::TopLevelArg" zeroinitializer, i32 3, !740, !215, !397}
!740 = !{%"class._ZTSN11xalanc_1_1017XalanQNameByValueE.xalanc_1_10::XalanQNameByValue" zeroinitializer, i32 0}
!741 = !{!"S", %"class._ZTSN11xalanc_1_1014XSLTEngineImplE.xalanc_1_10::XSLTEngineImpl" zeroinitializer, i32 29, !742, !154, !215, !215, !743, !107, !744, !745, !746, !747, !748, !251, !89, !89, !724, !749, !95, !750, !751, !752, !753, !754, !755, !756, !720, !215, !164, !89, !757}
!742 = !{%"class._ZTSN11xalanc_1_1013XSLTProcessorE.xalanc_1_10::XSLTProcessor" zeroinitializer, i32 0}
!743 = !{%"class._ZTSN11xalanc_1_1012XPathFactoryE.xalanc_1_10::XPathFactory" zeroinitializer, i32 1}
!744 = !{%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 0}
!745 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIbNS_31MemoryManagedConstructionTraitsIbEEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!746 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKN11xercesc_2_77LocatorENS_31MemoryManagedConstructionTraitsIS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!747 = !{%"class._ZTSN11xalanc_1_1022ProblemListenerDefaultE.xalanc_1_10::ProblemListenerDefault" zeroinitializer, i32 0}
!748 = !{%"class._ZTSN11xalanc_1_1015ProblemListenerE.xalanc_1_10::ProblemListener" zeroinitializer, i32 1}
!749 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_13TraceListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!750 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_11TopLevelArgENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!751 = !{%"class._ZTSN11xalanc_1_1016XMLParserLiaisonE.xalanc_1_10::XMLParserLiaison" zeroinitializer, i32 1}
!752 = !{%"class._ZTSN11xalanc_1_1015XPathEnvSupportE.xalanc_1_10::XPathEnvSupport" zeroinitializer, i32 1}
!753 = !{%"class._ZTSN11xalanc_1_1010DOMSupportE.xalanc_1_10::DOMSupport" zeroinitializer, i32 1}
!754 = !{%"class._ZTSN11xalanc_1_1026StylesheetExecutionContextE.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 1}
!755 = !{%"class._ZTSN11xalanc_1_1018OutputContextStackE.xalanc_1_10::OutputContextStack" zeroinitializer, i32 0}
!756 = !{%"class._ZTSN11xalanc_1_1020XalanNamespacesStackE.xalanc_1_10::XalanNamespacesStack" zeroinitializer, i32 0}
!757 = !{%"class._ZTSN11xalanc_1_1031XPathConstructionContextDefaultE.xalanc_1_10::XPathConstructionContextDefault" zeroinitializer, i32 0}
!758 = !{!"S", %"class._ZTSN11xalanc_1_1013XSLTProcessorE.xalanc_1_10::XSLTProcessor" zeroinitializer, i32 1, !85}
!759 = !{!"S", %"class._ZTSN11xalanc_1_1012XPathFactoryE.xalanc_1_10::XPathFactory" zeroinitializer, i32 1, !85}
!760 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 1, !761}
!761 = !{%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" zeroinitializer, i32 0}
!762 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_14XPathProcessorELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" zeroinitializer, i32 1, !763}
!763 = !{%"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1014XPathProcessorEE.std::pair" zeroinitializer, i32 0}
!764 = !{!"S", %"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1014XPathProcessorEE.std::pair" zeroinitializer, i32 2, !94, !765}
!765 = !{%"class._ZTSN11xalanc_1_1014XPathProcessorE.xalanc_1_10::XPathProcessor" zeroinitializer, i32 1}
!766 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKN11xercesc_2_77LocatorENS_31MemoryManagedConstructionTraitsIS4_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !767}
!767 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 2}
!768 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_13TraceListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !769}
!769 = !{%"class._ZTSN11xalanc_1_1013TraceListenerE.xalanc_1_10::TraceListener" zeroinitializer, i32 2}
!770 = !{!"S", %"class._ZTSN11xalanc_1_1020XalanNamespacesStackE.xalanc_1_10::XalanNamespacesStack" zeroinitializer, i32 4, !771, !772, !772, !745}
!771 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!772 = !{%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_20XalanNamespacesStack25XalanNamespacesStackEntryEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!773 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanDequeINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !94, !95, !774, !774}
!774 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!775 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEENS_31MemoryManagedConstructionTraitsIS6_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !776}
!776 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!777 = !{!"S", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_20XalanNamespacesStack25XalanNamespacesStackEntryEEENS_10XalanDequeIS3_NS_32ConstructWithMemoryManagerTraitsIS3_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !778, !95}
!778 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!779 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_20XalanNamespacesStack25XalanNamespacesStackEntryENS_32ConstructWithMemoryManagerTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !780}
!780 = !{%"class._ZTSN11xalanc_1_1020XalanNamespacesStack25XalanNamespacesStackEntryE.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" zeroinitializer, i32 1}
!781 = !{!"S", %"class._ZTSN11xalanc_1_1020XalanNamespacesStack25XalanNamespacesStackEntryE.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" zeroinitializer, i32 2, !782, !783}
!782 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!783 = !{%"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_14XalanNamespaceEEENS_10XalanDequeIS2_NS_31MemoryManagedConstructionTraitsIS2_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!784 = !{!"S", %"class._ZTSN11xalanc_1_1010XalanDequeINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !94, !95, !785, !785}
!785 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!786 = !{!"S", %"struct._ZTSN11xalanc_1_1018XalanDequeIteratorINS_24XalanDequeIteratorTraitsINS_14XalanNamespaceEEENS_10XalanDequeIS2_NS_31MemoryManagedConstructionTraitsIS2_EEEEEE.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !787, !95}
!787 = !{%"class._ZTSN11xalanc_1_1010XalanDequeINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!788 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS0_INS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEENS2_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !789}
!789 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!790 = !{!"S", %"class._ZTSN11xalanc_1_1022XalanCollationServices23CollationCompareFunctorE.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" zeroinitializer, i32 1, !85}
!791 = !{!"S", %"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefaultE.xalanc_1_10::StylesheetExecutionContextDefault" zeroinitializer, i32 46, !792, !793, !794, !145, !795, !251, !796, !797, !798, !799, !800, !801, !802, !803, !804, !805, !806, !325, !807, !87, !92, !808, !809, !810, !745, !322, !114, !811, !812, !813, !745, !745, !814, !815, !745, !816, !817, !795, !818, !819, !89, !820, !87, !87, !89, !820}
!792 = !{%"class._ZTSN11xalanc_1_1026StylesheetExecutionContextE.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 0}
!793 = !{%"class._ZTSN11xalanc_1_1028XPathExecutionContextDefaultE.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 0}
!794 = !{%"class._ZTSN11xalanc_1_1014XSLTEngineImplE.xalanc_1_10::XSLTEngineImpl" zeroinitializer, i32 1}
!795 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!796 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17FormatterListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!797 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_11PrintWriterENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!798 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17XalanOutputStreamENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!799 = !{%"class._ZTSN11xalanc_1_1022XalanCollationServices23CollationCompareFunctorE.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" zeroinitializer, i32 1}
!800 = !{%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault19FormatNumberFunctorE.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" zeroinitializer, i32 1}
!801 = !{%"class._ZTSN11xalanc_1_1014VariablesStackE.xalanc_1_10::VariablesStack" zeroinitializer, i32 0}
!802 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!803 = !{%"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!804 = !{%"class._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!805 = !{%"class._ZTSN11xalanc_1_1013CountersTableE.xalanc_1_10::CountersTable" zeroinitializer, i32 0}
!806 = !{%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 0}
!807 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemTemplateENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!808 = !{%"class._ZTSN11xalanc_1_1024XResultTreeFragAllocatorE.xalanc_1_10::XResultTreeFragAllocator" zeroinitializer, i32 0}
!809 = !{%"class._ZTSN11xalanc_1_1040XalanSourceTreeDocumentFragmentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" zeroinitializer, i32 0}
!810 = !{%"class._ZTSN11xalanc_1_1032XalanSourceTreeDocumentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentAllocator" zeroinitializer, i32 0}
!811 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_10XObjectPtrENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!812 = !{%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!813 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_33StylesheetExecutionContextDefault16NodesToTransformENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!814 = !{%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_14XalanDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!815 = !{%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS2_EENS_13DeleteFunctorIS2_EENS_24DefaultCacheResetFunctorIS2_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!816 = !{%"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_21FormatterToSourceTreeENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!817 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!818 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_26StylesheetExecutionContext22UseAttributeSetIndexesENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!819 = !{%"class._ZTSN11xalanc_1_1010NodeSorterE.xalanc_1_10::NodeSorter" zeroinitializer, i32 0}
!820 = !{!"A", i32 3, !89}
!821 = !{!"S", %"class._ZTSN11xalanc_1_1028XPathExecutionContextDefaultE.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 11, !190, !752, !753, !139, !822, !210, !215, !823, !644, !824, !740}
!822 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_15NodeRefListBaseENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!823 = !{%"class._ZTSN11xalanc_1_1016XalanObjectCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_22ClearCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectCache" zeroinitializer, i32 0}
!824 = !{%"struct._ZTSN11xalanc_1_1028XPathExecutionContextDefault28ContextNodeListPositionCacheE.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" zeroinitializer, i32 0}
!825 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_15NodeRefListBaseENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !826}
!826 = !{%"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 2}
!827 = !{!"S", %"class._ZTSN11xalanc_1_1016XalanObjectCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_22ClearCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectCache" zeroinitializer, i32 4, !828, !829, !830, !831}
!828 = !{%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!829 = !{%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!830 = !{%"class._ZTSN11xalanc_1_1022ClearCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::ClearCacheResetFunctor" zeroinitializer, i32 0}
!831 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_18MutableNodeRefListENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!832 = !{!"S", %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !89}
!833 = !{!"S", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !94}
!834 = !{!"S", %"class._ZTSN11xalanc_1_1022ClearCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::ClearCacheResetFunctor" zeroinitializer, i32 1, !89}
!835 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_18MutableNodeRefListENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !836}
!836 = !{%"class._ZTSN11xalanc_1_1018MutableNodeRefListE.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 2}
!837 = !{!"S", %"struct._ZTSN11xalanc_1_1028XPathExecutionContextDefault28ContextNodeListPositionCacheE.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" zeroinitializer, i32 3, !145, !87, !92}
!838 = !{!"S", %"class._ZTSN11xalanc_1_1016XMLParserLiaisonE.xalanc_1_10::XMLParserLiaison" zeroinitializer, i32 1, !85}
!839 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_19ElemTemplateElementENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !840}
!840 = !{%"class._ZTSN11xalanc_1_1019ElemTemplateElementE.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 2}
!841 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17FormatterListenerENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !842}
!842 = !{%"class._ZTSN11xalanc_1_1017FormatterListenerE.xalanc_1_10::FormatterListener" zeroinitializer, i32 2}
!843 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_11PrintWriterENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !844}
!844 = !{%"class._ZTSN11xalanc_1_1011PrintWriterE.xalanc_1_10::PrintWriter" zeroinitializer, i32 2}
!845 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_17XalanOutputStreamENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !846}
!846 = !{%"class._ZTSN11xalanc_1_1017XalanOutputStreamE.xalanc_1_10::XalanOutputStream" zeroinitializer, i32 2}
!847 = !{!"S", %"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault19FormatNumberFunctorE.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" zeroinitializer, i32 1, !85}
!848 = !{!"S", %"class._ZTSN11xalanc_1_1014VariablesStackE.xalanc_1_10::VariablesStack" zeroinitializer, i32 6, !849, !95, !89, !95, !850, !795}
!849 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack10StackEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!850 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!851 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack10StackEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !852}
!852 = !{%"class._ZTSN11xalanc_1_1014VariablesStack10StackEntryE.xalanc_1_10::VariablesStack::StackEntry" zeroinitializer, i32 1}
!853 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemVariableENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !304}
!854 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !855}
!855 = !{%"struct._ZTSN11xalanc_1_1014VariablesStack17ParamsVectorEntryE.xalanc_1_10::VariablesStack::ParamsVectorEntry" zeroinitializer, i32 1}
!856 = !{!"S", %"struct._ZTSN11xalanc_1_1014VariablesStack17ParamsVectorEntryE.xalanc_1_10::VariablesStack::ParamsVectorEntry" zeroinitializer, i32 3, !325, !397, !857}
!857 = !{%"class._ZTSN11xalanc_1_1012ElemVariableE.xalanc_1_10::ElemVariable" zeroinitializer, i32 1}
!858 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !281, !282, !94, !175, !95, !95, !859, !859, !860, !95, !95}
!859 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!860 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!861 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !862, !862}
!862 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" zeroinitializer, i32 1}
!863 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" zeroinitializer, i32 3, !864, !862, !862}
!864 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" zeroinitializer, i32 0}
!865 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" zeroinitializer, i32 3, !866, !89, !112}
!866 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES_IPKNS0_5XPathElEE.std::pair" zeroinitializer, i32 1}
!867 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES_IPKNS0_5XPathElEE.std::pair" zeroinitializer, i32 2, !215, !868}
!868 = !{%"struct._ZTSSt4pairIPKN11xalanc_1_105XPathElE.std::pair" zeroinitializer, i32 0}
!869 = !{!"S", %"struct._ZTSSt4pairIPKN11xalanc_1_105XPathElE.std::pair" zeroinitializer, i32 2, !151, !95}
!870 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEENS_32ConstructWithMemoryManagerTraitsISL_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !871}
!871 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!872 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISD_E4NodeEEENS_31MemoryManagedConstructionTraitsISI_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !873}
!873 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISC_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!874 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringESt4pairIPKNS_5XPathElENS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListISC_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !862}
!875 = !{!"S", %"class._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEEE.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !876, !877, !94, !175, !95, !95, !878, !878, !879, !95, !95}
!876 = !{%"class._ZTSN11xalanc_1_1011XalanHasherIPKNS_9XalanNodeEEE.xalanc_1_10::XalanHasher" zeroinitializer, i32 0}
!877 = !{%"struct._ZTSSt8equal_toIPKN11xalanc_1_109XalanNodeEE.std::equal_to" zeroinitializer, i32 0}
!878 = !{%"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!879 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!880 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanHasherIPKNS_9XalanNodeEEE.xalanc_1_10::XalanHasher" zeroinitializer, i32 1, !89}
!881 = !{!"S", %"struct._ZTSSt8equal_toIPKN11xalanc_1_109XalanNodeEE.std::equal_to" zeroinitializer, i32 1, !89}
!882 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !883, !883}
!883 = !{%"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" zeroinitializer, i32 1}
!884 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" zeroinitializer, i32 3, !885, !883, !883}
!885 = !{%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" zeroinitializer, i32 0}
!886 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" zeroinitializer, i32 3, !887, !89, !112}
!887 = !{%"struct._ZTSSt4pairIKPKN11xalanc_1_109XalanNodeEPNS0_8KeyTableEE.std::pair" zeroinitializer, i32 1}
!888 = !{!"S", %"struct._ZTSSt4pairIKPKN11xalanc_1_109XalanNodeEPNS0_8KeyTableEE.std::pair" zeroinitializer, i32 2, !145, !889}
!889 = !{%"class._ZTSN11xalanc_1_108KeyTableE.xalanc_1_10::KeyTable" zeroinitializer, i32 1}
!890 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEENS_32ConstructWithMemoryManagerTraitsISK_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !891}
!891 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!892 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_21XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS6_EEE5EntryEEENS_9XalanListISC_E4NodeEEENS_31MemoryManagedConstructionTraitsISH_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !893}
!893 = !{%"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!894 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_9XalanNodeEPNS_8KeyTableENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISB_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !883}
!895 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EEE.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 1, !896}
!896 = !{%"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" zeroinitializer, i32 0}
!897 = !{!"S", %"class._ZTSN11xalanc_1_1018XalanMemMgrAutoPtrINS_23XalanSourceTreeDocumentELb1EE17MemMgrAutoPtrDataE.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" zeroinitializer, i32 1, !898}
!898 = !{%"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1023XalanSourceTreeDocumentEE.std::pair" zeroinitializer, i32 0}
!899 = !{!"S", %"struct._ZTSSt4pairIPN11xercesc_2_713MemoryManagerEPN11xalanc_1_1023XalanSourceTreeDocumentEE.std::pair" zeroinitializer, i32 2, !94, !426}
!900 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPKNS_12ElemTemplateENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !901}
!901 = !{%"class._ZTSN11xalanc_1_1012ElemTemplateE.xalanc_1_10::ElemTemplate" zeroinitializer, i32 2}
!902 = !{!"S", %"class._ZTSN11xalanc_1_1024XResultTreeFragAllocatorE.xalanc_1_10::XResultTreeFragAllocator" zeroinitializer, i32 1, !903}
!903 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!904 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !905, !89, !112}
!905 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_15XResultTreeFragENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!906 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_15XResultTreeFragENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !160, !907}
!907 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!908 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !909, !909}
!909 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" zeroinitializer, i32 1}
!910 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_15XResultTreeFragEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" zeroinitializer, i32 3, !911, !909, !909}
!911 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_15XResultTreeFragEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!912 = !{!"S", %"class._ZTSN11xalanc_1_1040XalanSourceTreeDocumentFragmentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" zeroinitializer, i32 1, !913}
!913 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!914 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !915, !89, !112}
!915 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_31XalanSourceTreeDocumentFragmentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!916 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_31XalanSourceTreeDocumentFragmentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !160, !917}
!917 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!918 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !919, !919}
!919 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" zeroinitializer, i32 1}
!920 = !{!"S", %"class._ZTSN11xalanc_1_1032XalanSourceTreeDocumentAllocatorE.xalanc_1_10::XalanSourceTreeDocumentAllocator" zeroinitializer, i32 1, !921}
!921 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!922 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !923, !89, !112}
!923 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeDocumentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!924 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_23XalanSourceTreeDocumentENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !160, !925}
!925 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!926 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !927, !927}
!927 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" zeroinitializer, i32 1}
!928 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_18MutableNodeRefListENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !828, !829, !929, !831, !95}
!929 = !{%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!930 = !{!"S", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_18MutableNodeRefListEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !89}
!931 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_33StylesheetExecutionContextDefault16NodesToTransformENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !932}
!932 = !{%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault16NodesToTransformE.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" zeroinitializer, i32 1}
!933 = !{!"S", %"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault16NodesToTransformE.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" zeroinitializer, i32 3, !934, !87, !92}
!934 = !{%"class._ZTSN11xalanc_1_1015NodeRefListBaseE.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 1}
!935 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_14XalanDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !936, !937, !938, !646, !95}
!936 = !{%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!937 = !{%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!938 = !{%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!939 = !{!"S", %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !89}
!940 = !{!"S", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !94}
!941 = !{!"S", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_14XalanDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !89}
!942 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31DefaultCacheCreateFunctorMemMgrIS2_EENS_13DeleteFunctorIS2_EENS_24DefaultCacheResetFunctorIS2_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !943, !944, !945, !946, !95}
!943 = !{%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!944 = !{%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!945 = !{%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!946 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!947 = !{!"S", %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !89}
!948 = !{!"S", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !94}
!949 = !{!"S", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !89}
!950 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_33StylesheetExecutionContextDefault24FormatterToTextDOMStringENS_31MemoryManagedConstructionTraitsIS3_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !951}
!951 = !{%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault24FormatterToTextDOMStringE.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" zeroinitializer, i32 2}
!952 = !{!"S", %"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefault24FormatterToTextDOMStringE.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" zeroinitializer, i32 2, !953, !954}
!953 = !{%"class._ZTSN11xalanc_1_1015FormatterToTextE.xalanc_1_10::FormatterToText.base" zeroinitializer, i32 0}
!954 = !{%"class._ZTSN11xalanc_1_1020DOMStringPrintWriterE.xalanc_1_10::DOMStringPrintWriter" zeroinitializer, i32 0}
!955 = !{!"S", %"class._ZTSN11xalanc_1_1015FormatterToTextE.xalanc_1_10::FormatterToText.base" zeroinitializer, i32 11, !425, !956, !160, !192, !215, !89, !89, !89, !957, !96, !87}
!956 = !{%"class._ZTSN11xalanc_1_106WriterE.xalanc_1_10::Writer" zeroinitializer, i32 1}
!957 = !{!"A", i32 5, !89}
!958 = !{!"S", %"class._ZTSN11xalanc_1_1021XalanObjectStackCacheINS_21FormatterToSourceTreeENS_31DefaultCacheCreateFunctorMemMgrIS1_EENS_13DeleteFunctorIS1_EENS_24DefaultCacheResetFunctorIS1_EEEE.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !959, !960, !961, !962, !95}
!959 = !{%"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!960 = !{%"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!961 = !{%"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!962 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_21FormatterToSourceTreeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!963 = !{!"S", %"class._ZTSN11xalanc_1_1031DefaultCacheCreateFunctorMemMgrINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !89}
!964 = !{!"S", %"struct._ZTSN11xalanc_1_1013DeleteFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !94}
!965 = !{!"S", %"class._ZTSN11xalanc_1_1024DefaultCacheResetFunctorINS_21FormatterToSourceTreeEEE.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !89}
!966 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_21FormatterToSourceTreeENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !967}
!967 = !{%"class._ZTSN11xalanc_1_1021FormatterToSourceTreeE.xalanc_1_10::FormatterToSourceTree" zeroinitializer, i32 2}
!968 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS0_INS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEENS3_IS5_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !969}
!969 = !{%"class._ZTSN11xalanc_1_1011XalanVectorINS_14VariablesStack17ParamsVectorEntryENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!970 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_26StylesheetExecutionContext22UseAttributeSetIndexesENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !971}
!971 = !{%"struct._ZTSN11xalanc_1_1026StylesheetExecutionContext22UseAttributeSetIndexesE.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" zeroinitializer, i32 1}
!972 = !{!"S", %"class._ZTSN11xalanc_1_1015XResultTreeFragE.xalanc_1_10::XResultTreeFrag" zeroinitializer, i32 6, !124, !973, !99, !754, !215, !125}
!973 = !{%"class._ZTSN11xalanc_1_1021XalanDocumentFragmentE.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1}
!974 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_15XResultTreeFragEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !975, !160, !160, !92}
!975 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_15XResultTreeFragEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!976 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_15XResultTreeFragEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !977, !160, !160, !978}
!977 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!978 = !{%"class._ZTSN11xalanc_1_1015XResultTreeFragE.xalanc_1_10::XResultTreeFrag" zeroinitializer, i32 1}
!979 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_15XResultTreeFragEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!980 = !{!"S", %"class._ZTSN11xalanc_1_1014VariablesStack10StackEntryE.xalanc_1_10::VariablesStack::StackEntry" zeroinitializer, i32 5, !87, !325, !397, !857, !157}
!981 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" zeroinitializer, i32 3, !982, !919, !919}
!982 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!983 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" zeroinitializer, i32 3, !984, !927, !927}
!984 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!985 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !986, !160, !160, !92}
!986 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!987 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !988, !160, !160, !92}
!988 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!989 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_31XalanSourceTreeDocumentFragmentEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !990, !160, !160, !427}
!990 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!991 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_23XalanSourceTreeDocumentEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !992, !160, !160, !426}
!992 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!993 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_31XalanSourceTreeDocumentFragmentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!994 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_23XalanSourceTreeDocumentEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!995 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS7_EEEENS_17XalanMapKeyTraitsIS4_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" zeroinitializer, i32 3, !996, !358, !358}
!996 = !{%"struct._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" zeroinitializer, i32 0}
!997 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS6_EEEENS_17XalanMapKeyTraitsIS3_EEE5EntryE.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" zeroinitializer, i32 3, !998, !89, !112}
!998 = !{%"struct._ZTSSt4pairIKPKN11xalanc_1_1010XalanQNameENS0_11XalanVectorIPNS0_16ElemAttributeSetENS0_31MemoryManagedConstructionTraitsIS7_EEEEE.std::pair" zeroinitializer, i32 1}
!999 = !{!"S", %"struct._ZTSSt4pairIKPKN11xalanc_1_1010XalanQNameENS0_11XalanVectorIPNS0_16ElemAttributeSetENS0_31MemoryManagedConstructionTraitsIS7_EEEEE.std::pair" zeroinitializer, i32 2, !325, !1000}
!1000 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!1001 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !1002}
!1002 = !{%"class._ZTSN11xalanc_1_1016ElemAttributeSetE.xalanc_1_10::ElemAttributeSet" zeroinitializer, i32 2}
!1003 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKNS_10XalanQNameENS_11XalanVectorIPNS_16ElemAttributeSetENS_31MemoryManagedConstructionTraitsIS8_EEEENS_17XalanMapKeyTraitsIS5_EEE5EntryEEENS_9XalanListISF_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !358}
!1004 = !{!"S", %"class._ZTSN11xalanc_1_1013TraceListenerE.xalanc_1_10::TraceListener" zeroinitializer, i32 1, !85}
!1005 = !{!"S", %"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1, !1006}
!1006 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!1007 = !{!"S", %"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !1008, !89, !112}
!1008 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!1009 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !85, !160, !1010}
!1010 = !{%"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!1011 = !{!"S", %"class._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEE.xalanc_1_10::XalanList" zeroinitializer, i32 3, !94, !1012, !1012}
!1012 = !{%"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 1}
!1013 = !{!"S", %"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 3, !124, !125, !1014}
!1014 = !{%"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 0}
!1015 = !{!"S", %"class._ZTSN11xalanc_1_1026XObjectResultTreeFragProxyE.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 2, !1016, !1017}
!1016 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyBaseE.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}
!1017 = !{%"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 0}
!1018 = !{!"S", %"class._ZTSN11xalanc_1_1030XObjectResultTreeFragProxyTextE.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !535, !371, !94}
!1019 = !{!"S", %"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 2, !1020, !1021}
!1020 = !{%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 0}
!1021 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}
!1022 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 3, !1023, !1012, !1012}
!1023 = !{%"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!1024 = !{!"S", %"class._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtEE.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !1025, !160, !160, !92}
!1025 = !{%"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!1026 = !{!"S", %"class._ZTSN11xalanc_1_1014ArenaBlockBaseINS_13XStringCachedEtEE.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !1027, !160, !160, !1028}
!1027 = !{%"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!1028 = !{%"class._ZTSN11xalanc_1_1013XStringCachedE.xalanc_1_10::XStringCached" zeroinitializer, i32 1}
!1029 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanAllocatorINS_13XStringCachedEEE.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !94}
!1030 = !{!"S", %"class._ZTSN11xalanc_1_1014XalanNamespaceE.xalanc_1_10::XalanNamespace" zeroinitializer, i32 2, !215, !215}
!1031 = !{!"S", %"struct._ZTSN11xalanc_1_1018ReusableArenaBlockINS_13XStringCachedEtE9NextBlockE.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" zeroinitializer, i32 2, !160, !87}
!1032 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapINS_14XalanDOMStringES2_NS_17XalanMapKeyTraitsIS2_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" zeroinitializer, i32 3, !1033, !527, !527}
!1033 = !{%"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" zeroinitializer, i32 0}
!1034 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapINS_14XalanDOMStringES3_NS_17XalanMapKeyTraitsIS3_EEE5EntryEEENS_9XalanListIS7_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !527}
!1035 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapINS_14XalanDOMStringES1_NS_17XalanMapKeyTraitsIS1_EEE5EntryE.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" zeroinitializer, i32 3, !1036, !89, !112}
!1036 = !{%"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES1_E.std::pair" zeroinitializer, i32 1}
!1037 = !{!"S", %"struct._ZTSSt4pairIKN11xalanc_1_1014XalanDOMStringES1_E.std::pair" zeroinitializer, i32 2, !215, !215}
!1038 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1, !700}
!1039 = !{!"S", %"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1, !1040}
!1040 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}
!1041 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !85}
!1042 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorINS_14XalanNamespaceENS_31MemoryManagedConstructionTraitsIS1_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !1043}
!1043 = !{%"class._ZTSN11xalanc_1_1014XalanNamespaceE.xalanc_1_10::XalanNamespace" zeroinitializer, i32 1}
!1044 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListISt4pairImPNS_11XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS4_EEEEEE4NodeE.xalanc_1_10::XalanList<std::pair<unsigned long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" zeroinitializer, i32 3, !1045, !512, !512}
!1045 = !{%"struct._ZTSSt4pairImPN11xalanc_1_1011XalanVectorIPNS0_19XalanSourceTreeAttrENS0_31MemoryManagedConstructionTraitsIS3_EEEEE.std::pair" zeroinitializer, i32 0}
!1046 = !{!"S", %"struct._ZTSSt4pairImPN11xalanc_1_1011XalanVectorIPNS0_19XalanSourceTreeAttrENS0_31MemoryManagedConstructionTraitsIS3_EEEEE.std::pair" zeroinitializer, i32 2, !95, !1047}
!1047 = !{%"class._ZTSN11xalanc_1_1011XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!1048 = !{!"S", %"struct._ZTSN11xalanc_1_109XalanListINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS3_EEE5EntryEE4NodeE.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" zeroinitializer, i32 3, !1049, !520, !520}
!1049 = !{%"struct._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEE5EntryE.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" zeroinitializer, i32 0}
!1050 = !{!"S", %"struct._ZTSN11xalanc_1_108XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS2_EEE5EntryE.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" zeroinitializer, i32 3, !1051, !89, !112}
!1051 = !{%"struct._ZTSSt4pairIKPKtPN11xalanc_1_1022XalanSourceTreeElementEE.std::pair" zeroinitializer, i32 1}
!1052 = !{!"S", %"struct._ZTSSt4pairIKPKtPN11xalanc_1_1022XalanSourceTreeElementEE.std::pair" zeroinitializer, i32 2, !96, !428}
!1053 = !{!"S", %"struct._ZTSN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsINS_8XalanMapIPKtPNS_22XalanSourceTreeElementENS_17XalanMapKeyTraitsIS4_EEE5EntryEEENS_9XalanListISA_E4NodeEEE.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !520}
!1054 = !{!"S", %"class._ZTSN11xalanc_1_1011XalanVectorIPNS_19XalanSourceTreeAttrENS_31MemoryManagedConstructionTraitsIS2_EEEE.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !94, !95, !95, !625}
!1055 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!1056 = !{i32 1, !"wchar_size", i32 4}
!1057 = !{i32 1, !"Virtual Function Elim", i32 0}
!1058 = !{i32 7, !"uwtable", i32 2}
!1059 = !{i32 1, !"ThinLTO", i32 0}
!1060 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!1061 = !{i32 1, !"LTOPostLink", i32 1}
!1062 = distinct !{!6, !6}
!1063 = distinct !{!6}
!1064 = distinct !{!6, !6, !6}
!1065 = distinct !{!6}
!1066 = distinct !{!6}
!1067 = distinct !{!1068}
!1068 = !{%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1}
!1069 = distinct !{!6, !1070}
!1070 = !{%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1}
!1071 = !{!"_Intel.Devirt.Target"}
!1072 = distinct !{!1070, !6}
!1073 = distinct !{!1074, !99}
!1074 = !{%"class._ZTSN11xalanc_1_1033StylesheetExecutionContextDefaultE.xalanc_1_10::StylesheetExecutionContextDefault" zeroinitializer, i32 1}
!1075 = distinct !{!1076, !99}
!1076 = !{%"class._ZTSN11xalanc_1_1028XPathExecutionContextDefaultE.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 1}
!1077 = distinct !{!1078, !94}
!1078 = !{%"class._ZTSN11xalanc_1_1011XStringBaseE.xalanc_1_10::XStringBase" zeroinitializer, i32 1}
!1079 = distinct !{!1078}
!1080 = distinct !{!1028, !1081, !94}
!1081 = !{%"class._ZTSN11xalanc_1_1021XPathExecutionContext25GetAndReleaseCachedStringE.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}
!1082 = distinct !{!1028}
!1083 = !{!"F", i1 false, i32 2, !1084, !348, !99}
!1084 = !{i1 false, i32 0}
!1085 = distinct !{!1086, !94}
!1086 = !{%"class._ZTSN11xalanc_1_1022XStringCachedAllocatorE.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1}
!1087 = distinct !{!1088, !94}
!1088 = !{%"class._ZTSN11xalanc_1_1022ReusableArenaAllocatorINS_13XStringCachedEEE.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 1}
!1089 = distinct !{!1090}
!1090 = !{%"class._ZTSN11xalanc_1_1014ArenaAllocatorINS_13XStringCachedENS_18ReusableArenaBlockIS1_tEEEE.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 1}
!1091 = !{!"F", i1 false, i32 1, !1092, !1090}
!1092 = !{!"void", i32 0}
!1093 = !{!1094}
!1094 = distinct !{!1094, !1095, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1095 = distinct !{!1095, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1096 = !{!1097}
!1097 = distinct !{!1097, !1098, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv: %agg.result"}
!1098 = distinct !{!1098, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv"}
!1099 = !{!"F", i1 false, i32 2, !6, !94, !95}
!1100 = !{!1101}
!1101 = distinct !{!1101, !1102, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi: %agg.result"}
!1102 = distinct !{!1102, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi"}
!1103 = distinct !{!1103, !1104}
!1104 = !{!"llvm.loop.mustprogress"}
!1105 = !{!"F", i1 false, i32 2, !1092, !94, !6}
!1106 = distinct !{!1028, !1088}
!1107 = distinct !{!1107, !1104}
!1108 = !{!1109}
!1109 = distinct !{!1109, !1110, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1110 = distinct !{!1110, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1111 = !{!1112}
!1112 = distinct !{!1112, !1113, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1113 = distinct !{!1113, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1114 = distinct !{!1088, !1028}
!1115 = !{!1116}
!1116 = distinct !{!1116, !1117, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1117 = distinct !{!1117, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1118 = !{!1119}
!1119 = distinct !{!1119, !1120, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1120 = distinct !{!1120, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1121 = distinct !{!1090}
!1122 = !{}
!1123 = !{!1124}
!1124 = distinct !{!1124, !1125, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1125 = distinct !{!1125, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1126 = !{!"F", i1 false, i32 1, !1092, !1028}
!1127 = distinct !{!1127, !1104}
!1128 = !{!1129}
!1129 = distinct !{!1129, !1130, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv: %agg.result"}
!1130 = distinct !{!1130, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv"}
!1131 = distinct !{!1131, !1104}
!1132 = !{!1133}
!1133 = distinct !{!1133, !1134, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1134 = distinct !{!1134, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1135 = !{!1136}
!1136 = distinct !{!1136, !1137, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi: %agg.result"}
!1137 = distinct !{!1137, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEi"}
!1138 = distinct !{!1138, !1104}
!1139 = distinct !{!94, !1090}
!1140 = distinct !{!1086}
!1141 = distinct !{!1028, !1086, !1081}
!1142 = distinct !{!1086, !1028}
!1143 = distinct !{!1088, !1028}
!1144 = !{!1145}
!1145 = distinct !{!1145, !1146, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1146 = distinct !{!1146, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1147 = !{!1148}
!1148 = distinct !{!1148, !1149, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1149 = distinct !{!1149, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1150 = !{i8 0, i8 2}
!1151 = !{!1152}
!1152 = distinct !{!1152, !1153, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv: %agg.result"}
!1153 = distinct !{!1153, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEppEv"}
!1154 = distinct !{!1154, !1104}
!1155 = !{!1156, !1158}
!1156 = distinct !{!1156, !1157, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1157 = distinct !{!1157, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1158 = distinct !{!1158, !1159, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv: %agg.result"}
!1159 = distinct !{!1159, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE4rendEv"}
!1160 = !{!1161, !1163}
!1161 = distinct !{!1161, !1162, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv: %agg.result"}
!1162 = distinct !{!1162, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv"}
!1163 = distinct !{!1163, !1164, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv: %agg.result"}
!1164 = distinct !{!1164, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv"}
!1165 = !{!1166}
!1166 = distinct !{!1166, !1167, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv: %agg.result"}
!1167 = distinct !{!1167, !"_ZN11xalanc_1_1021XalanListIteratorBaseINS_23XalanListIteratorTraitsIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEEENS_9XalanListIS5_E4NodeEEmmEv"}
!1168 = !{!1169, !1171}
!1169 = distinct !{!1169, !1170, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv: %agg.result"}
!1170 = distinct !{!1170, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE3endEv"}
!1171 = distinct !{!1171, !1172, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv: %agg.result"}
!1172 = distinct !{!1172, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE6rbeginEv"}
!1173 = !{!1174}
!1174 = distinct !{!1174, !1175, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv: %agg.result"}
!1175 = distinct !{!1175, !"_ZN11xalanc_1_109XalanListIPNS_18ReusableArenaBlockINS_13XStringCachedEtEEE5beginEv"}
!1176 = distinct !{!1176, !1104}
!1177 = distinct !{!1086}
!1178 = distinct !{!1179}
!1179 = !{%"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1}
!1180 = distinct !{!6, !1181}
!1181 = !{%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1}
!1182 = distinct !{!1181, !6}
