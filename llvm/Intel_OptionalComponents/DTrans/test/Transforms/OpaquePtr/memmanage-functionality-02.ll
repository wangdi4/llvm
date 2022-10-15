; This test verifies the following functionalities, which represent the
; Windows form of the functions, are recognized by MemManageTransOP:
;
; getMemManager    : ?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ
; Constructor      : ??0?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G_N@Z
; AllocateBlock    : ?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ
; CommitAllocation : ?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z
; Destructor       : ??1?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAA@XZ
; Reset            : ?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ
; DestroyObject    : ?destroyObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z

; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -passes=dtrans-memmanagetransop -dtrans-memmanageop-ignore-soa-heur -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mattr=+avx2 -debug-only=dtrans-memmanagetransop -disable-output 2>&1 | FileCheck %s

; CHECK: MemManageTransOP transformation:
; CHECK: Considering candidate: %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator"
; CHECK: Recognized GetMemManager: ?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ
; CHECK: Recognized Constructor: ??0?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G_N@Z
; CHECK: Recognized AllocateBlock: ?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ
; CHECK: Recognized CommitAllocation: ?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z
; CHECK: Recognized Destructor: ??1?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAA@XZ
; CHECK: Recognized Reset: ?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ
; CHECK: Recognized DestroyObject: ?destroyObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

%rtti.CompleteObjectLocator = type { i32, i32, i32, i32, i32, i32 }
%eh.ThrowInfo = type { i32, i32, i32, i32 }
%"class..?AVOutOfMemoryException@xercesc_2_7@@.xercesc_2_7::OutOfMemoryException" = type { i8 }
%"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached" = type { %"class..?AVXStringBase@xalanc_1_10@@.xalanc_1_10::XStringBase", %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" }
%"class..?AVXStringBase@xalanc_1_10@@.xalanc_1_10::XStringBase" = type { %"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject", double, %"class..?AVXObjectResultTreeFragProxy@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxy" }
%"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject" = type { %"class..?AVXalanReferenceCountedObject@xalanc_1_10@@.xalanc_1_10::XalanReferenceCountedObject", i32, ptr }
%"class..?AVXalanReferenceCountedObject@xalanc_1_10@@.xalanc_1_10::XalanReferenceCountedObject" = type { ptr, i32 }
%"class..?AVXObjectResultTreeFragProxy@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxy" = type { %"class..?AVXObjectResultTreeFragProxyBase@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyBase", %"class..?AVXObjectResultTreeFragProxyText@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyText" }
%"class..?AVXObjectResultTreeFragProxyBase@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyBase" = type { %"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment" }
%"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" = type { ptr }
%"class..?AVXObjectResultTreeFragProxyText@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyText" = type { %"class..?AVXalanText@xalanc_1_10@@.xalanc_1_10::XalanText", ptr, ptr }
%"class..?AVXalanText@xalanc_1_10@@.xalanc_1_10::XalanText" = type { %"class..?AVXalanCharacterData@xalanc_1_10@@.xalanc_1_10::XalanCharacterData" }
%"class..?AVXalanCharacterData@xalanc_1_10@@.xalanc_1_10::XalanCharacterData" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" = type { ptr, ptr }
%"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext" = type { %"class..?AVExecutionContext@xalanc_1_10@@.xalanc_1_10::ExecutionContext", ptr }
%"class..?AVExecutionContext@xalanc_1_10@@.xalanc_1_10::ExecutionContext" = type { ptr, ptr }
%"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" = type { %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32 }
%"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator" = type { %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" }
%"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" = type { %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", i8 }
%"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" = type { ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" = type { ptr, ptr, ptr }
%"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", i16, i16 }
%"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" = type { i16, i32 }
%"class..?AVbad_alloc@std@@.std::bad_alloc" = type { %"class..?AVexception@std@@.std::exception" }
%"class..?AVexception@std@@.std::exception" = type { ptr, %"struct..?AU__std_exception_data@@.__std_exception_data" }
%"struct..?AU__std_exception_data@@.__std_exception_data" = type { ptr, i8 }
%"class..?AVAVT@xalanc_1_10@@.xalanc_1_10::AVT" = type { ptr, ptr, i64, ptr, i32, ptr }
%"class..?AVAVTPart@xalanc_1_10@@.xalanc_1_10::AVTPart" = type { ptr }
%"class..?AVXPathConstructionContext@xalanc_1_10@@.xalanc_1_10::XPathConstructionContext" = type { ptr, ptr }
%"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" = type { ptr }
%"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver" = type { ptr }
%"class..?AVXObjectFactory@xalanc_1_10@@.xalanc_1_10::XObjectFactory" = type { ptr, ptr }
%"class..?AVXPath@xalanc_1_10@@.xalanc_1_10::XPath" = type { %"class..?AVXPathExpression@xalanc_1_10@@.xalanc_1_10::XPathExpression", ptr, i8 }
%"class..?AVXPathExpression@xalanc_1_10@@.xalanc_1_10::XPathExpression" = type { %"class..?AV?$XalanVector@HU?$MemoryManagedConstructionTraits@H@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, %"class..?AV?$XalanVector@VXToken@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXToken@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, ptr, %"class..?AV?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@HU?$MemoryManagedConstructionTraits@H@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VXToken@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXToken@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXToken@xalanc_1_10@@.xalanc_1_10::XToken" = type { %"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject", ptr, double, i8 }
%"class..?AVAttributeListImpl@xalanc_1_10@@.xalanc_1_10::AttributeListImpl" = type { %"class..?AVAttributeList@xercesc_2_7@@.xercesc_2_7::AttributeList", %"class..?AV?$XalanVector@PEAVAttributeVectorEntry@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVAttributeVectorEntry@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAVAttributeVectorEntry@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVAttributeVectorEntry@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AVAttributeList@xercesc_2_7@@.xercesc_2_7::AttributeList" = type { ptr }
%"class..?AV?$XalanVector@PEAVAttributeVectorEntry@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVAttributeVectorEntry@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVAttributeVectorEntry@xalanc_1_10@@.xalanc_1_10::AttributeVectorEntry" = type { ptr, %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AVCountersTable@xalanc_1_10@@.xalanc_1_10::CountersTable" = type { %"class..?AV?$XalanVector@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUCounter@xalanc_1_10@@.xalanc_1_10::Counter" = type { i32, %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, ptr }
%"class..?AVElemNumber@xalanc_1_10@@.xalanc_1_10::ElemNumber" = type { %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement", ptr, ptr, ptr, i32, ptr, ptr, ptr, ptr, ptr, i32 }
%"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement" = type { %"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver", ptr, %"class..?AVNamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler", i32, ptr, ptr, ptr, %"union..?AT<unnamed-type-$S1>@ElemTemplateElement@xalanc_1_10@@.anon", %"class..?AVLocatorProxy@ElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement::LocatorProxy", i16 }
%"class..?AVNamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler" = type { %"class..?AV?$XalanVector@VNamespace@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespace@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" }
%"class..?AV?$XalanVector@VNamespace@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespace@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringPointerHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringPointerHashFunction", %"struct..?AU?$pointer_equal@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"struct..?AUDOMStringPointerHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringPointerHashFunction" = type { i8 }
%"struct..?AU?$pointer_equal@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"union..?AT<unnamed-type-$S1>@ElemTemplateElement@xalanc_1_10@@.anon" = type { ptr }
%"class..?AVLocatorProxy@ElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement::LocatorProxy" = type { %"class..?AVXalanLocator@xalanc_1_10@@.xalanc_1_10::XalanLocator", i32, i32, ptr }
%"class..?AVXalanLocator@xalanc_1_10@@.xalanc_1_10::XalanLocator" = type { %"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" }
%"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet" = type { %"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver", ptr, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AV?$XalanVector@VKeyDeclaration@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VKeyDeclaration@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@VXalanSpaceNodeTester@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanSpaceNodeTester@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AV?$XalanVector@PEAVStylesheet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVStylesheet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, %"class..?AV?$XalanDeque@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque", %"class..?AV?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque", i8, %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", ptr, %"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"class..?AV?$XalanVector@PEAVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", double, %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"struct..?AU?$XalanMapIterator@U?$XalanMapConstIteratorTraits@U?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanMapIterator", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"struct..?AU?$XalanMapIterator@U?$XalanMapConstIteratorTraits@U?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanMapIterator", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, %"class..?AV?$XalanVector@PEAVElemDecimalFormat@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemDecimalFormat@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVNamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler" }
%"class..?AV?$XalanVector@VKeyDeclaration@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VKeyDeclaration@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VXalanSpaceNodeTester@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanSpaceNodeTester@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVStylesheet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVStylesheet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanDeque@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class..?AV?$XalanVector@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class..?AV?$XalanVector@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction" = type { i8 }
%"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AU?$XalanHashMemberReference@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberReference", %"struct..?AU?$equal_to@VXalanQName@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"struct..?AU?$XalanHashMemberReference@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberReference" = type { i8 }
%"struct..?AU?$equal_to@VXalanQName@xalanc_1_10@@@std@@.std::equal_to" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanMapIterator@U?$XalanMapConstIteratorTraits@U?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanMapIterator" = type { %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVElemDecimalFormat@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemDecimalFormat@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVNamespace@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::Namespace" = type { ptr, ptr }
%"class..?AVNamespaceExtended@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::NamespaceExtended" = type { %"class..?AVNamespace@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::Namespace", ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVStylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext" = type { %"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext" }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AVXalanDocument@xalanc_1_10@@.xalanc_1_10::XalanDocument" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVXalanElement@xalanc_1_10@@.xalanc_1_10::XalanElement" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVXalanAttr@xalanc_1_10@@.xalanc_1_10::XalanAttr" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVXalanComment@xalanc_1_10@@.xalanc_1_10::XalanComment" = type { %"class..?AVXalanCharacterData@xalanc_1_10@@.xalanc_1_10::XalanCharacterData" }
%"class..?AVXalanProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanProcessingInstruction" = type { %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" }
%"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener" = type { %"class..?AVDocumentHandler@xercesc_2_7@@.xercesc_2_7::DocumentHandler", ptr, i32, i32 }
%"class..?AVDocumentHandler@xercesc_2_7@@.xercesc_2_7::DocumentHandler" = type { ptr }
%"class..?AVXalanNamedNodeMap@xalanc_1_10@@.xalanc_1_10::XalanNamedNodeMap" = type { ptr }
%"class..?AVXalanOutputStream@xalanc_1_10@@.xalanc_1_10::XalanOutputStream" = type { ptr, i32, ptr, i32, %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i8, i8, %"class..?AV?$XalanVector@DU?$MemoryManagedConstructionTraits@D@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@DU?$MemoryManagedConstructionTraits@D@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanOutputTranscoder@xalanc_1_10@@.xalanc_1_10::XalanOutputTranscoder" = type { ptr, ptr }
%"class..?AVDOMStringPrintWriter@xalanc_1_10@@.xalanc_1_10::DOMStringPrintWriter" = type { %"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter", ptr }
%"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter" = type { %"class..?AVWriter@xalanc_1_10@@.xalanc_1_10::Writer", i8, ptr }
%"class..?AVWriter@xalanc_1_10@@.xalanc_1_10::Writer" = type { ptr }
%"class..?AVDOMSupport@xalanc_1_10@@.xalanc_1_10::DOMSupport" = type { ptr }
%"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool" = type { ptr, %"class..?AVXalanDOMStringAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringAllocator", i64, %"class..?AVXalanDOMStringHashTable@xalanc_1_10@@.xalanc_1_10::XalanDOMStringHashTable" }
%"class..?AVXalanDOMStringAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanDOMStringHashTable@xalanc_1_10@@.xalanc_1_10::XalanDOMStringHashTable" = type { i64, i64, %"class..?AV?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray", i64, i32 }
%"class..?AV?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray" = type { %"class..?AVMemMgrAutoPtrArrayData@?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" }
%"class..?AVMemMgrAutoPtrArrayData@?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" = type { ptr, ptr, i64 }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" = type { ptr, ptr, ptr }
%"class..?AV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName" = type { ptr }
%"class..?AVXalanQNameByValue@xalanc_1_10@@.xalanc_1_10::XalanQNameByValue" = type { %"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"class..?AVStylesheetRoot@xalanc_1_10@@.xalanc_1_10::StylesheetRoot" = type { %"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i32, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i8, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i32, %"class..?AV?$XalanVector@PEBVXalanQName@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i8, %"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, ptr, ptr, i8, i8, i32, i8, i32, %"class..?AV?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" }
%"class..?AV?$XalanVector@PEBVXalanQName@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AU?$XalanHashMemberPointer@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberPointer", %"struct..?AU?$pointer_equal@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"struct..?AU?$XalanHashMemberPointer@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberPointer" = type { i8 }
%"struct..?AU?$pointer_equal@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVElemTemplate@xalanc_1_10@@.xalanc_1_10::ElemTemplate" = type { %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement", ptr, ptr, ptr, double }
%"class..?AVKeyDeclaration@xalanc_1_10@@.xalanc_1_10::KeyDeclaration" = type { ptr, ptr, ptr, ptr, i32, i32 }
%"class..?AVXalanSpaceNodeTester@xalanc_1_10@@.xalanc_1_10::XalanSpaceNodeTester" = type { %"class..?AVNodeTester@XPath@xalanc_1_10@@.xalanc_1_10::XPath::NodeTester", i32, i32 }
%"class..?AVNodeTester@XPath@xalanc_1_10@@.xalanc_1_10::XPath::NodeTester" = type { ptr, ptr, ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVElemVariable@xalanc_1_10@@.xalanc_1_10::ElemVariable" = type { %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement", ptr, ptr, i8, %"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr", ptr }
%"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr" = type { ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanMatchPatternData@xalanc_1_10@@.xalanc_1_10::XalanMatchPatternData" = type { ptr, i64, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", ptr, ptr, i32 }
%"class..?AVElemDecimalFormat@xalanc_1_10@@.xalanc_1_10::ElemDecimalFormat" = type { %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement", ptr, ptr, ptr, ptr, %"class..?AVXalanDecimalFormatSymbols@xalanc_1_10@@.xalanc_1_10::XalanDecimalFormatSymbols" }
%"class..?AVXalanDecimalFormatSymbols@xalanc_1_10@@.xalanc_1_10::XalanDecimalFormatSymbols" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i16, i16, i16, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i16, i16, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i16, i16, i16, i16 }
%"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase" = type { ptr }
%"class..?AVMutableNodeRefList@xalanc_1_10@@.xalanc_1_10::MutableNodeRefList" = type { %"class..?AVNodeRefList@xalanc_1_10@@.xalanc_1_10::NodeRefList", i32 }
%"class..?AVNodeRefList@xalanc_1_10@@.xalanc_1_10::NodeRefList" = type { %"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase", %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AVNameSpace@xalanc_1_10@@.xalanc_1_10::NameSpace" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVExtensionNSHandler@xalanc_1_10@@.xalanc_1_10::ExtensionNSHandler" = type { %"class..?AVExtensionFunctionHandler@xalanc_1_10@@.xalanc_1_10::ExtensionFunctionHandler", %"class..?AV?$XalanSet@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanSet", i8 }
%"class..?AVExtensionFunctionHandler@xalanc_1_10@@.xalanc_1_10::ExtensionFunctionHandler" = type { ptr, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", ptr, %"class..?AV?$XalanSet@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanSet", i8 }
%"class..?AV?$XalanSet@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanSet" = type { %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AVElemAttributeSet@xalanc_1_10@@.xalanc_1_10::ElemAttributeSet" = type { %"class..?AVElemUse@xalanc_1_10@@.xalanc_1_10::ElemUse", ptr }
%"class..?AVElemUse@xalanc_1_10@@.xalanc_1_10::ElemUse" = type { %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement", ptr, i64 }
%"class..?AVNodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter" = type { %"class..?AV?$XalanVector@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@VNodeSortKey@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodeSortKey@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@UVectorEntry@NodeSorter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UVectorEntry@NodeSorter@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@VNodeSortKey@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodeSortKey@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@UVectorEntry@NodeSorter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UVectorEntry@NodeSorter@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVNodeSortKey@xalanc_1_10@@.xalanc_1_10::NodeSortKey" = type { ptr, ptr, i8, i8, i32, ptr, ptr }
%"struct..?AUVectorEntry@NodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter::VectorEntry" = type { ptr, i32 }
%"class..?AVXalanQNameByReference@xalanc_1_10@@.xalanc_1_10::XalanQNameByReference" = type { %"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName", ptr, ptr }
%"struct..?AUUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" = type { i32, i32 }
%"class..?AVXPathEnvSupport@xalanc_1_10@@.xalanc_1_10::XPathEnvSupport" = type { ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" = type { ptr, i8 }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@_N@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AU?$hash_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::hash_null_terminated_arrays" = type { i8 }
%"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVFormatterToSourceTree@xalanc_1_10@@.xalanc_1_10::FormatterToSourceTree" = type { %"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener", ptr, ptr, ptr, %"class..?AV?$XalanVector@PEAVXalanSourceTreeElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"class..?AV?$XalanVector@PEAVXalanSourceTreeElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanSourceTreeDocument@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocument" = type { %"class..?AVXalanDocument@xalanc_1_10@@.xalanc_1_10::XalanDocument", ptr, ptr, %"class..?AVXalanSourceTreeAttributeAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeAllocator", %"class..?AVXalanSourceTreeAttributeNSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeNSAllocator", %"class..?AVXalanSourceTreeCommentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeCommentAllocator", %"class..?AVXalanSourceTreeElementAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementAAllocator", %"class..?AVXalanSourceTreeElementANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANSAllocator", %"class..?AVXalanSourceTreeElementNAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNAAllocator", %"class..?AVXalanSourceTreeElementNANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANSAllocator", %"class..?AVXalanSourceTreeProcessingInstructionAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator", %"class..?AVXalanSourceTreeTextAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextAllocator", %"class..?AVXalanSourceTreeTextIWSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWSAllocator", %"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool", %"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool", %"class..?AV?$XalanArrayAllocator@PEAVXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanArrayAllocator", i32, i8, %"class..?AV?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"class..?AVXalanDOMStringAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringAllocator", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"class..?AVXalanSourceTreeAttributeAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeAttr@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeAttr@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeAttributeNSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeCommentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeCommentAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeComment@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeComment@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementAAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANSAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementNAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNAAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementNANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANSAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeProcessingInstructionAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeTextAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeText@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeText@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeTextIWSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWSAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i64, %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanArrayAllocator@PEAVXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanArrayAllocator" = type { %"class..?AV?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList", i64, ptr }
%"class..?AV?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AU?$hash_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::hash_null_terminated_arrays", %"struct..?AU?$equal_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::equal_null_terminated_arrays", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"struct..?AU?$equal_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::equal_null_terminated_arrays" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanSourceTreeDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragment" = type { %"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment", ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" = type { %"class..?AVXalanElement@xalanc_1_10@@.xalanc_1_10::XalanElement", ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32 }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AU?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@.std::pair" = type { i64, ptr }
%"struct..?AUNode@?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList<std::pair<unsigned long long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" = type { %"struct..?AU?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@.std::pair", ptr, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" = type { ptr, i8 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanSourceTreeText@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeText" = type { %"class..?AVXalanText@xalanc_1_10@@.xalanc_1_10::XalanText", ptr, ptr, ptr, ptr, i32 }
%"class..?AVXalanSourceTreeComment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeComment" = type { %"class..?AVXalanComment@xalanc_1_10@@.xalanc_1_10::XalanComment", ptr, ptr, ptr, ptr, ptr, i32 }
%"class..?AVXalanSourceTreeProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstruction" = type { %"class..?AVXalanProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanProcessingInstruction", ptr, ptr, ptr, ptr, ptr, ptr, i32 }
%"class..?AV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeComment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeComment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeElementA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeElementA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeText@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeText@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i64, i64, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr" = type { %"class..?AVXalanAttr@xalanc_1_10@@.xalanc_1_10::XalanAttr", ptr, ptr, ptr, i32 }
%"class..?AVXalanSourceTreeAttrNS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttrNS" = type { %"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr", ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementA" = type { %"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement", %"class..?AVXalanNamedNodeMap@xalanc_1_10@@.xalanc_1_10::XalanNamedNodeMap", ptr, i32 }
%"class..?AVXalanSourceTreeElementANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANS" = type { %"class..?AVXalanSourceTreeElementA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementA", ptr, ptr, ptr }
%"class..?AVXalanSourceTreeElementNA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNA" = type { %"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" }
%"class..?AVXalanSourceTreeElementNANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANS" = type { %"class..?AVXalanSourceTreeElementNA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNA", ptr, ptr, ptr }
%"class..?AVXalanSourceTreeTextIWS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWS" = type { %"class..?AVXalanSourceTreeText@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeText" }
%"class..?AVFormatterToText@xalanc_1_10@@.xalanc_1_10::FormatterToText" = type { %"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener", ptr, i16, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i8, i8, i8, ptr, i32 }
%"class..?AV?$XalanVector@VXObjectPtr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXObjectPtr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXPathProcessor@xalanc_1_10@@.xalanc_1_10::XPathProcessor" = type { ptr }
%"class..?AVXPathConstructionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathConstructionContextDefault" = type { %"class..?AVXPathConstructionContext@xalanc_1_10@@.xalanc_1_10::XPathConstructionContext", %"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool", %"class..?AVXalanDOMStringCache@xalanc_1_10@@.xalanc_1_10::XalanDOMStringCache" }
%"class..?AVXalanDOMStringCache@xalanc_1_10@@.xalanc_1_10::XalanDOMStringCache" = type { %"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, %"class..?AVXalanDOMStringReusableAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringReusableAllocator" }
%"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanDOMStringReusableAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringReusableAllocator" = type { %"class..?AV?$ReusableArenaAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" }
%"class..?AV?$ReusableArenaAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", i8 }
%"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" = type { ptr, ptr, ptr }
%"class..?AV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", i16, i16 }
%"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class..?AVKeyTable@xalanc_1_10@@.xalanc_1_10::KeyTable" = type { ptr, ptr, %"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" }
%"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AU?$XalanHashMemberReference@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberReference", %"struct..?AU?$equal_to@VXalanQName@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" = type { ptr, i8 }
%"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@@std@@.std::pair" = type { %"class..?AVXalanQNameByReference@xalanc_1_10@@.xalanc_1_10::XalanQNameByReference", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" = type { ptr, i8 }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVMutableNodeRefList@xalanc_1_10@@.xalanc_1_10::MutableNodeRefList" }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"class..?AVMemoryManagerImpl@xercesc_2_7@@.xercesc_2_7::MemoryManagerImpl" = type { %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" }
%"struct..?AU?$pair@QEBVXalanDOMString@xalanc_1_10@@PEBV12@@std@@.std::pair" = type { ptr, ptr }
%"class..?AVOutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack" = type { %"class..?AV?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator", i64 }
%"class..?AV?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class..?AV?$XalanVector@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class..?AV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUOutputContext@OutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack::OutputContext" = type { ptr, %"class..?AVAttributeListImpl@xalanc_1_10@@.xalanc_1_10::AttributeListImpl", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", i8, i8 }
%"class..?AVProblemListener@xalanc_1_10@@.xalanc_1_10::ProblemListener" = type { ptr }
%"class..?AVProblemListenerDefault@xalanc_1_10@@.xalanc_1_10::ProblemListenerDefault" = type { %"class..?AVProblemListener@xalanc_1_10@@.xalanc_1_10::ProblemListener", ptr, ptr }
%"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@@std@@.std::pair" = type { %"class..?AVXalanQNameByReference@xalanc_1_10@@.xalanc_1_10::XalanQNameByReference", ptr }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@VTopLevelArg@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VTopLevelArg@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVTopLevelArg@xalanc_1_10@@.xalanc_1_10::TopLevelArg" = type { %"class..?AVXalanQNameByValue@xalanc_1_10@@.xalanc_1_10::XalanQNameByValue", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr" }
%"class..?AVXSLTEngineImpl@xalanc_1_10@@.xalanc_1_10::XSLTEngineImpl" = type { %"class..?AVXSLTProcessor@xalanc_1_10@@.xalanc_1_10::XSLTProcessor", %"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", ptr, ptr, %"class..?AV?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVLocator@xercesc_2_7@@U?$MemoryManagedConstructionTraits@PEBVLocator@xercesc_2_7@@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVProblemListenerDefault@xalanc_1_10@@.xalanc_1_10::ProblemListenerDefault", ptr, ptr, i8, i8, ptr, %"class..?AV?$XalanVector@PEAVTraceListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVTraceListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, %"class..?AV?$XalanVector@VTopLevelArg@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VTopLevelArg@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, ptr, ptr, ptr, %"class..?AVOutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack", %"class..?AVXalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack", %"class..?AVAttributeListImpl@xalanc_1_10@@.xalanc_1_10::AttributeListImpl", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i8, %"class..?AVXPathConstructionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathConstructionContextDefault" }
%"class..?AVXSLTProcessor@xalanc_1_10@@.xalanc_1_10::XSLTProcessor" = type { ptr }
%"class..?AV?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" = type { %"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" }
%"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" = type { %"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXPathProcessor@xalanc_1_10@@@std@@.std::pair" }
%"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXPathProcessor@xalanc_1_10@@@std@@.std::pair" = type { ptr, ptr }
%"class..?AV?$XalanVector@PEBVLocator@xercesc_2_7@@U?$MemoryManagedConstructionTraits@PEBVLocator@xercesc_2_7@@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVTraceListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVTraceListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack" = type { %"class..?AV?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class..?AVXPathFactory@xalanc_1_10@@.xalanc_1_10::XPathFactory" = type { ptr }
%"class..?AVXMLParserLiaison@xalanc_1_10@@.xalanc_1_10::XMLParserLiaison" = type { ptr }
%"class..?AVTraceListener@xalanc_1_10@@.xalanc_1_10::TraceListener" = type { ptr }
%"class..?AV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" = type { %"class..?AV?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespace@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" }
%"class..?AV?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" = type { ptr, i64, %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespace@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" = type { ptr, i64 }
%"class..?AV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVCollationCompareFunctor@XalanCollationServices@xalanc_1_10@@.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" = type { ptr }
%"class..?AVStylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault" = type { %"class..?AVStylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext", %"class..?AVXPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault", ptr, ptr, %"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, %"class..?AV?$XalanVector@PEAVFormatterListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAVPrintWriter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVPrintWriter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEAVXalanOutputStream@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanOutputStream@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, ptr, %"class..?AVVariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack", %"class..?AV?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"class..?AV?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap", %"class..?AVCountersTable@xalanc_1_10@@.xalanc_1_10::CountersTable", %"class..?AV?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr", ptr, %"class..?AV?$XalanVector@PEBVElemTemplate@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplate@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, %"class..?AVXResultTreeFragAllocator@xalanc_1_10@@.xalanc_1_10::XResultTreeFragAllocator", %"class..?AVXalanSourceTreeDocumentFragmentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator", %"class..?AVXalanSourceTreeDocumentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentAllocator", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVXalanQName@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@HU?$MemoryManagedConstructionTraits@H@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@VXObjectPtr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXObjectPtr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanObjectStackCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache", %"class..?AV?$XalanVector@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanObjectStackCache@VXalanDOMString@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@2@U?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache", %"class..?AV?$XalanObjectStackCache@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@U?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@V?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanObjectStackCache@VFormatterToSourceTree@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@2@U?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache", %"class..?AV?$XalanVector@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AVNodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter", i8, i32, i32, i8 }
%"class..?AVXPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault" = type { %"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext", ptr, ptr, %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVNodeRefListBase@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVNodeRefListBase@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", ptr, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AV?$XalanObjectCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectCache", %"class..?AVXalanDOMStringCache@xalanc_1_10@@.xalanc_1_10::XalanDOMStringCache", %"struct..?AUContextNodeListPositionCache@XPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache", %"class..?AVXalanQNameByValue@xalanc_1_10@@.xalanc_1_10::XalanQNameByValue" }
%"class..?AV?$XalanVector@PEBVNodeRefListBase@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVNodeRefListBase@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanObjectCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectCache" = type { %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct..?AU?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor", %"class..?AV?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ClearCacheResetFunctor", %"class..?AV?$XalanVector@PEAVMutableNodeRefList@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct..?AU?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class..?AV?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ClearCacheResetFunctor" = type { i8 }
%"class..?AV?$XalanVector@PEAVMutableNodeRefList@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AUContextNodeListPositionCache@XPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" = type { ptr, i32 }
%"class..?AV?$XalanVector@PEAVFormatterListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVPrintWriter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVPrintWriter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEAVXalanOutputStream@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanOutputStream@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVVariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack" = type { %"class..?AV?$XalanVector@VStackEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VStackEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", i32, i8, i32, %"class..?AV?$XalanVector@PEBVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", %"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@VStackEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VStackEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEBVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" = type { %"class..?AV?$XalanHasher@PEBVXalanNode@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHasher", %"struct..?AU?$equal_to@PEBVXalanNode@xalanc_1_10@@@std@@.std::equal_to", ptr, float, i64, i64, %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64, i64 }
%"class..?AV?$XalanHasher@PEBVXalanNode@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHasher" = type { i8 }
%"struct..?AU?$equal_to@PEBVXalanNode@xalanc_1_10@@@std@@.std::equal_to" = type { i8 }
%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" = type { %"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" }
%"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" = type { %"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXalanSourceTreeDocument@xalanc_1_10@@@std@@.std::pair" }
%"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXalanSourceTreeDocument@xalanc_1_10@@@std@@.std::pair" = type { ptr, ptr }
%"class..?AV?$XalanVector@PEBVElemTemplate@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplate@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXResultTreeFragAllocator@xalanc_1_10@@.xalanc_1_10::XResultTreeFragAllocator" = type { %"class..?AV?$ReusableArenaAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" }
%"class..?AV?$ReusableArenaAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" = type { %"class..?AV?$ArenaAllocator@VXResultTreeFrag@xalanc_1_10@@V?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", i8 }
%"class..?AV?$ArenaAllocator@VXResultTreeFrag@xalanc_1_10@@V?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeDocumentFragmentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" = type { %"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" }
%"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", i8 }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AVXalanSourceTreeDocumentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentAllocator" = type { %"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" }
%"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" = type { %"class..?AV?$ArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", i8 }
%"class..?AV?$ArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" = type { ptr, i16, %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" }
%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" = type { ptr, ptr, ptr }
%"class..?AV?$XalanObjectStackCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" = type { %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct..?AU?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor", %"class..?AV?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor", %"class..?AV?$XalanVector@PEAVMutableNodeRefList@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64 }
%"class..?AV?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class..?AV?$XalanVector@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanObjectStackCache@VXalanDOMString@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@2@U?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" = type { %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct..?AU?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor", %"class..?AV?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor", %"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64 }
%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct..?AU?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class..?AV?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class..?AV?$XalanObjectStackCache@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@U?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@V?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" = type { %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct..?AU?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor", %"class..?AV?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor", %"class..?AV?$XalanVector@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64 }
%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct..?AU?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class..?AV?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class..?AV?$XalanVector@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanObjectStackCache@VFormatterToSourceTree@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@2@U?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" = type { %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr", %"struct..?AU?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor", %"class..?AV?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor", %"class..?AV?$XalanVector@PEAVFormatterToSourceTree@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector", i64 }
%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" = type { i8 }
%"struct..?AU?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" = type { ptr }
%"class..?AV?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" = type { i8 }
%"class..?AV?$XalanVector@PEAVFormatterToSourceTree@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AV?$XalanVector@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVFormatNumberFunctor@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" = type { ptr }
%"class..?AVStackEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::StackEntry" = type { i32, ptr, %"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr", ptr, ptr }
%"struct..?AUParamsVectorEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::ParamsVectorEntry" = type { ptr, %"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr", ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" = type { ptr, i8 }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"struct..?AU?$pair@PEBVXPath@xalanc_1_10@@J@std@@.std::pair" }
%"struct..?AU?$pair@PEBVXPath@xalanc_1_10@@J@std@@.std::pair" = type { ptr, i32 }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" = type { %"struct..?AUEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry", ptr, ptr }
%"struct..?AUEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" = type { ptr, i8 }
%"struct..?AU?$pair@QEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@@std@@.std::pair" = type { ptr, ptr }
%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" = type { ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" = type { ptr, ptr, ptr }
%"class..?AV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", i16, i16 }
%"class..?AV?$ArenaBlockBase@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class..?AV?$XalanAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" = type { ptr, ptr, ptr }
%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" = type { ptr, ptr, ptr }
%"class..?AVNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" = type { ptr, i32 }
%"class..?AVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" = type { %"class..?AVFormatterToText@xalanc_1_10@@.xalanc_1_10::FormatterToText", %"class..?AVDOMStringPrintWriter@xalanc_1_10@@.xalanc_1_10::DOMStringPrintWriter" }
%"class..?AVXResultTreeFrag@xalanc_1_10@@.xalanc_1_10::XResultTreeFrag" = type { %"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject", ptr, ptr, ptr, %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", double }
%"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", i16, i16 }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" = type { %"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", i16, i16 }
%"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" = type { %"class..?AV?$XalanAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", i16, i16, ptr }
%"class..?AV?$XalanAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" = type { ptr }
%"struct..?AU?$pair@QEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@@std@@.std::pair" = type { ptr, %"class..?AV?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" }
%"class..?AV?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"class..?AVXalanNamespace@xalanc_1_10@@.xalanc_1_10::XalanNamespace" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V12@@std@@.std::pair" = type { %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" }
%"class..?AVXalanDummyMemoryManager@xalanc_1_10@@.xalanc_1_10::XalanDummyMemoryManager" = type { %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" }
%"class..?AV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" = type { ptr, i64, i64, ptr }
%"struct..?AU?$pair@QEBGPEAVXalanSourceTreeElement@xalanc_1_10@@@std@@.std::pair" = type { ptr, ptr }

$"??_GXStringCached@xalanc_1_10@@UEAAPEAXI@Z" = comdat any

$"??0?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G_N@Z" = comdat any

$"?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ" = comdat any

$"?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z" = comdat any

$"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ" = comdat any

$"?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ" = comdat any

$"??1?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAA@XZ" = comdat any

$"?destroyObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z" = comdat any

$"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z" = comdat any

$"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z" = comdat any

$"??_7?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@" = comdat largest

$"??_7?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@" = comdat largest

@anon.b3c821f2ced5b8e8c000afce8af71354.0 = hidden unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr @"??_R4?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@", ptr @"??_G?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAXI@Z", ptr @"?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ", ptr @"?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z", ptr @"?ownsObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEBA_NPEBVXStringCached@2@@Z", ptr @"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ"] }, comdat($"??_7?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@"), !type !0, !type !1, !intel_dtrans_type !1136
@"??_R4?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@" = external hidden constant %rtti.CompleteObjectLocator
@anon.b3c821f2ced5b8e8c000afce8af71354.1 = hidden unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr @"??_R4?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@", ptr @"??_G?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAPEAXI@Z", ptr @"?allocateBlock@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ", ptr @"?commitAllocation@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z", ptr @"?ownsObject@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEBA_NPEBVXStringCached@2@@Z", ptr @"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ"] }, comdat($"??_7?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@"), !type !0, !intel_dtrans_type !1136
@"??_R4?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@" = external hidden constant %rtti.CompleteObjectLocator
@"_TI2?AVbad_alloc@std@@" = external unnamed_addr constant %eh.ThrowInfo, section ".xdata"
@"??_C@_0P@GHFPNOJB@bad?5allocation?$AA@" = external hidden unnamed_addr constant [15 x i8], align 1
@"_TI2?AVOutOfMemoryException@xercesc_2_7@@" = external hidden unnamed_addr constant %eh.ThrowInfo, section ".xdata"
@"??_7bad_alloc@std@@6B@" = external global ptr, !intel_dtrans_type !144
@"??_7XStringCached@xalanc_1_10@@6B@" = external global ptr, !intel_dtrans_type !144

@"??_7?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@" = hidden unnamed_addr alias ptr, getelementptr inbounds ({ [6 x ptr] }, ptr @anon.b3c821f2ced5b8e8c000afce8af71354.0, i32 0, i32 0, i32 1)
@"??_7?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@" = hidden unnamed_addr alias ptr, getelementptr inbounds ({ [6 x ptr] }, ptr @anon.b3c821f2ced5b8e8c000afce8af71354.1, i32 0, i32 0, i32 1)

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #0

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: nofree
declare !intel.dtrans.func.type !988 dso_local void @_CxxThrowException(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2") local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #2

; Function Attrs: nofree noreturn
declare dso_local void @__std_terminate() local_unnamed_addr #3

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !990 dso_local void @"??3@YAXPEAX@Z"(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #4

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !991 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @"??2@YAPEAX_K@Z"(i64 noundef) local_unnamed_addr #5

; Function Attrs: uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %0, i64 noundef %1) unnamed_addr #6 align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !992 !_Intel.Devirt.Target !994 {
  %3 = alloca %"class..?AVOutOfMemoryException@xercesc_2_7@@.xercesc_2_7::OutOfMemoryException", align 1
  %4 = invoke noalias noundef nonnull ptr @"??2@YAPEAX_K@Z"(i64 noundef %1) #18
          to label %9 unwind label %5

5:                                                ; preds = %2
  %6 = catchswitch within none [label %7] unwind to caller

7:                                                ; preds = %5
  %8 = catchpad within %6 [ptr null, i32 64, ptr null]
  call void @_CxxThrowException(ptr nonnull %3, ptr nonnull @"_TI2?AVOutOfMemoryException@xercesc_2_7@@") #19 [ "funclet"(token %8) ]
  unreachable

9:                                                ; preds = %2
  ret ptr %4
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %0, ptr noundef "intel_dtrans_func_index"="2" %1) unnamed_addr #7 align 2 !intel.dtrans.func.type !995 !_Intel.Devirt.Target !994 {
  tail call void @"??3@YAXPEAX@Z"(ptr noundef %1) #20
  ret void
}

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !996 hidden noundef zeroext i1 @"?releaseCachedString@StylesheetExecutionContextDefault@xalanc_1_10@@UEAA_NAEAVXalanDOMString@2@@Z"(ptr noundef nonnull align 8 dereferenceable(2064) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2") unnamed_addr #8 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !998 hidden noundef zeroext i1 @"?releaseCachedString@XPathExecutionContextDefault@xalanc_1_10@@UEAA_NAEAVXalanDOMString@2@@Z"(ptr noundef nonnull align 8 dereferenceable(432) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2") unnamed_addr #8 align 2

; Function Attrs: mustprogress nofree norecurse nosync willreturn uwtable
declare !intel.dtrans.func.type !1000 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??0XStringBase@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@@Z"(ptr noundef nonnull returned align 8 dereferenceable(72) "intel_dtrans_func_index"="2", ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3") unnamed_addr #9 align 2

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare !intel.dtrans.func.type !1002 hidden void @"??1XStringBase@xalanc_1_10@@UEAA@XZ"(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(72) "intel_dtrans_func_index"="1") unnamed_addr #10 align 2

; Function Attrs: mustprogress nofree norecurse nosync willreturn uwtable
define hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??0XStringCached@xalanc_1_10@@QEAA@AEAVGetAndReleaseCachedString@XPathExecutionContext@1@AEAVMemoryManager@xercesc_2_7@@@Z"(ptr noundef nonnull returned align 8 dereferenceable(88) "intel_dtrans_func_index"="2" %0, ptr nocapture noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %1, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="4" %2) unnamed_addr #9 align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !1003 {
  %4 = tail call noundef ptr @"??0XStringBase@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@@Z"(ptr noundef nonnull align 8 dereferenceable(72) %0, ptr noundef nonnull align 8 dereferenceable(8) %2)
  %5 = getelementptr %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %0, i64 0, i32 0, i32 0, i32 0, i32 0
  store ptr @"??_7XStringCached@xalanc_1_10@@6B@", ptr %5, align 8
  %6 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %0, i64 0, i32 1
  %7 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %6, i64 0, i32 0
  %8 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %1, i64 0, i32 0
  %9 = load ptr, ptr %8, align 8
  store ptr %9, ptr %7, align 8
  %10 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %6, i64 0, i32 1
  %11 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %1, i64 0, i32 1
  %12 = load ptr, ptr %11, align 8
  store ptr %12, ptr %10, align 8
  store ptr null, ptr %11, align 8
  ret ptr %0
}

; Function Attrs: nounwind uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @"??_GXStringCached@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull returned align 8 dereferenceable(88) "intel_dtrans_func_index"="2" %0, i32 noundef %1) unnamed_addr #11 comdat align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !1005 {
  %3 = getelementptr %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %0, i64 0, i32 0, i32 0, i32 0, i32 0
  store ptr @"??_7XStringCached@xalanc_1_10@@6B@", ptr %3, align 8
  %4 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %0, i64 0, i32 1
  %5 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %4, i64 0, i32 1
  %6 = load ptr, ptr %5, align 8
  %7 = icmp eq ptr %6, null
  br i1 %7, label %26, label %8

8:                                                ; preds = %2
  %9 = getelementptr inbounds %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString", ptr %4, i64 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = getelementptr %"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext", ptr %10, i64 0, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = tail call i1 @llvm.type.test(ptr %12, metadata !"?AVXPathExecutionContext@xalanc_1_10@@")
  tail call void @llvm.assume(i1 %13)
  %14 = getelementptr inbounds ptr, ptr %12, i64 23
  %15 = load ptr, ptr %14, align 8
  %16 = bitcast ptr %15 to ptr
  %17 = bitcast ptr @"?releaseCachedString@StylesheetExecutionContextDefault@xalanc_1_10@@UEAA_NAEAVXalanDOMString@2@@Z" to ptr
  %18 = icmp eq ptr %16, %17
  br i1 %18, label %19, label %21

19:                                               ; preds = %8
  %20 = invoke noundef zeroext i1 @"?releaseCachedString@StylesheetExecutionContextDefault@xalanc_1_10@@UEAA_NAEAVXalanDOMString@2@@Z"(ptr noundef nonnull align 8 dereferenceable(24) %10, ptr noundef nonnull align 8 dereferenceable(40) %6)
          to label %23 unwind label %24, !intel_dtrans_type !1006

21:                                               ; preds = %8
  %22 = invoke noundef zeroext i1 @"?releaseCachedString@XPathExecutionContextDefault@xalanc_1_10@@UEAA_NAEAVXalanDOMString@2@@Z"(ptr noundef nonnull align 8 dereferenceable(24) %10, ptr noundef nonnull align 8 dereferenceable(40) %6)
          to label %23 unwind label %24, !intel_dtrans_type !1006

23:                                               ; preds = %21, %19
  br label %26

24:                                               ; preds = %21, %19
  %25 = cleanuppad within none []
  call void @__std_terminate() #21 [ "funclet"(token %25) ]
  unreachable

26:                                               ; preds = %23, %2
  tail call void @"??1XStringBase@xalanc_1_10@@UEAA@XZ"(ptr noundef nonnull align 8 dereferenceable(72) %0) #20
  %27 = icmp eq i32 %1, 0
  br i1 %27, label %29, label %28

28:                                               ; preds = %26
  tail call void @"??3@YAXPEAX@Z"(ptr noundef nonnull %0) #22
  br label %29

29:                                               ; preds = %28, %26
  ret ptr %0
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
define hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??0XStringCachedAllocator@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G@Z"(ptr noundef nonnull returned align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %0, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %1, i16 noundef %2) unnamed_addr #12 align 2 !intel.dtrans.func.type !1008 {
  %4 = getelementptr inbounds %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator", ptr %0, i64 0, i32 0
  %5 = tail call noundef ptr @"??0?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G_N@Z"(ptr noundef nonnull align 8 dereferenceable(48) %4, ptr noundef nonnull align 8 dereferenceable(8) %1, i16 noundef 10, i1 noundef zeroext false)
  ret ptr %0
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
define hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??0?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA@AEAVMemoryManager@xercesc_2_7@@G_N@Z"(ptr noundef nonnull returned writeonly align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %0, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %1, i16 noundef %2, i1 noundef zeroext %3) unnamed_addr #12 comdat align 2 !intel.dtrans.func.type !1010 {
  %5 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 1
  store i16 10, ptr %5, align 8
  %6 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %7 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %6, i64 0, i32 0
  store ptr %1, ptr %7, align 8
  %8 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %6, i64 0, i32 1
  store ptr null, ptr %8, align 8
  %9 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %6, i64 0, i32 2
  store ptr null, ptr %9, align 8
  %10 = getelementptr %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator", ptr %0, i64 0, i32 0, i32 0
  store ptr @"??_7?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@6B@", ptr %10, align 8
  %11 = getelementptr inbounds %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator", ptr %0, i64 0, i32 1
  store i8 0, ptr %11, align 8
  ret ptr %0
}

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1012 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??_G?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull returned align 8 dereferenceable(48) "intel_dtrans_func_index"="2", i32 noundef) unnamed_addr #11 align 2

; Function Attrs: uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @"?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %0) unnamed_addr #13 comdat align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !1013 {
  %2 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %3 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 1
  %4 = load ptr, ptr %3, align 8, !noalias !1014
  %5 = icmp eq ptr %4, null
  br i1 %5, label %6, label %26

6:                                                ; preds = %1
  %7 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %8 = load ptr, ptr %7, align 8, !noalias !1015
  %9 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %8, i64 0, i32 0
  %10 = load ptr, ptr %9, align 8, !noalias !1015
  %11 = tail call i1 @llvm.type.test(ptr %10, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %11)
  %12 = getelementptr inbounds ptr, ptr %10, i64 1
  %13 = load ptr, ptr %12, align 8, !noalias !1015
  %14 = bitcast ptr %13 to ptr
  %15 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %16 = icmp eq ptr %14, %15
  br i1 %16, label %17, label %19

17:                                               ; preds = %6
  %18 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %8, i64 noundef 24), !noalias !1015, !intel_dtrans_type !1018
  br label %21

19:                                               ; preds = %6
  %20 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %8, i64 noundef 24), !noalias !1015, !intel_dtrans_type !1018
  br label %21

21:                                               ; preds = %19, %17
  %22 = phi ptr [ %18, %17 ], [ %20, %19 ]
  br label %23

23:                                               ; preds = %21
  store ptr %22, ptr %3, align 8, !noalias !1015
  %24 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %22, i64 0, i32 2
  store ptr %22, ptr %24, align 8, !noalias !1015
  %25 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %22, i64 0, i32 1
  store ptr %22, ptr %25, align 8, !noalias !1015
  br label %38

26:                                               ; preds = %1
  %27 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %4, i64 0, i32 2
  %28 = load ptr, ptr %27, align 8, !noalias !1015
  %29 = icmp eq ptr %28, %4
  br i1 %29, label %38, label %30

30:                                               ; preds = %26
  %31 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %28, i64 0, i32 0
  %32 = load ptr, ptr %31, align 8
  %33 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %32, i64 0, i32 2
  %34 = load i16, ptr %33, align 2
  %35 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %32, i64 0, i32 1
  %36 = load i16, ptr %35, align 8
  %37 = icmp ult i16 %36, %34
  br i1 %37, label %173, label %38

38:                                               ; preds = %30, %26, %23
  %39 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 1
  %40 = load i16, ptr %39, align 8
  %41 = tail call noundef nonnull align 8 dereferenceable(8) ptr @"?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ"(ptr noundef nonnull align 8 dereferenceable(40) %0)
  %42 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %41, i64 0, i32 0
  %43 = load ptr, ptr %42, align 8
  %44 = tail call i1 @llvm.type.test(ptr %43, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %44)
  %45 = getelementptr inbounds ptr, ptr %43, i64 1
  %46 = load ptr, ptr %45, align 8
  %47 = bitcast ptr %46 to ptr
  %48 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %49 = icmp eq ptr %47, %48
  br i1 %49, label %50, label %52

50:                                               ; preds = %38
  %51 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, i64 noundef 32), !intel_dtrans_type !1018
  br label %54

52:                                               ; preds = %38
  %53 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, i64 noundef 32), !intel_dtrans_type !1018
  br label %54

54:                                               ; preds = %52, %50
  %55 = phi ptr [ %51, %50 ], [ %53, %52 ]
  br label %56

56:                                               ; preds = %54
  %57 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %55, i64 0, i32 0
  %58 = getelementptr inbounds %"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", ptr %57, i64 0, i32 0
  store ptr %41, ptr %58, align 8
  %59 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %55, i64 0, i32 1
  store i16 0, ptr %59, align 8
  %60 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %55, i64 0, i32 2
  store i16 %40, ptr %60, align 2
  %61 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %41, i64 0, i32 0
  %62 = load ptr, ptr %61, align 8
  %63 = tail call i1 @llvm.type.test(ptr %62, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %63)
  %64 = zext i16 %40 to i64
  %65 = mul nuw nsw i64 %64, 88
  %66 = getelementptr inbounds ptr, ptr %62, i64 1
  %67 = load ptr, ptr %66, align 8
  %68 = bitcast ptr %67 to ptr
  %69 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %70 = icmp eq ptr %68, %69
  br i1 %70, label %71, label %73

71:                                               ; preds = %56
  %72 = invoke noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, i64 noundef %65)
          to label %75 unwind label %93, !intel_dtrans_type !1018

73:                                               ; preds = %56
  %74 = invoke noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, i64 noundef %65)
          to label %75 unwind label %93, !intel_dtrans_type !1018

75:                                               ; preds = %73, %71
  %76 = phi ptr [ %72, %71 ], [ %74, %73 ]
  br label %77

77:                                               ; preds = %75
  %78 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %55, i64 0, i32 3
  store ptr %76, ptr %78, align 8
  %79 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %55, i64 0, i32 1
  store i16 0, ptr %79, align 8
  %80 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %55, i64 0, i32 2
  store i16 0, ptr %80, align 2
  %81 = load i16, ptr %60, align 2
  %82 = icmp eq i16 %81, 0
  br i1 %82, label %109, label %83

83:                                               ; preds = %77
  %84 = zext i16 %81 to i64
  br label %85

85:                                               ; preds = %85, %83
  %86 = phi i64 [ 0, %83 ], [ %88, %85 ]
  %87 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %76, i64 %86
  %88 = add nuw nsw i64 %86, 1
  %89 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %87, i64 0, i32 0
  %90 = trunc i64 %88 to i16
  store i16 %90, ptr %89, align 4
  %91 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %87, i64 0, i32 1
  store i32 -2228259, ptr %91, align 4
  %92 = icmp eq i64 %88, %84
  br i1 %92, label %109, label %85, !llvm.loop !1019

93:                                               ; preds = %73, %71
  %94 = cleanuppad within none []
  %95 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %41, i64 0, i32 0
  %96 = load ptr, ptr %95, align 8
  %97 = tail call i1 @llvm.type.test(ptr %96, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %97)
  %98 = getelementptr inbounds ptr, ptr %96, i64 2
  %99 = load ptr, ptr %98, align 8
  %100 = bitcast ptr %99 to ptr
  %101 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %102 = icmp eq ptr %100, %101
  br i1 %102, label %103, label %104

103:                                              ; preds = %93
  invoke void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, ptr noundef nonnull %55) [ "funclet"(token %94) ]
          to label %105 unwind label %106

104:                                              ; preds = %93
  invoke void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %41, ptr noundef nonnull %55) [ "funclet"(token %94) ]
          to label %105 unwind label %106

105:                                              ; preds = %104, %103
  br label %108

106:                                              ; preds = %104, %103
  %107 = cleanuppad within %94 []
  call void @__std_terminate() #21 [ "funclet"(token %107) ]
  unreachable

108:                                              ; preds = %105
  cleanupret from %94 unwind to caller

109:                                              ; preds = %85, %77
  %110 = load ptr, ptr %3, align 8, !noalias !1021
  %111 = icmp eq ptr %110, null
  br i1 %111, label %115, label %112

112:                                              ; preds = %109
  %113 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %110, i64 0, i32 2
  %114 = load ptr, ptr %113, align 8, !noalias !1021
  br label %135

115:                                              ; preds = %109
  %116 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %117 = load ptr, ptr %116, align 8, !noalias !1021
  %118 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %117, i64 0, i32 0
  %119 = load ptr, ptr %118, align 8, !noalias !1021
  %120 = tail call i1 @llvm.type.test(ptr %119, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %120)
  %121 = getelementptr inbounds ptr, ptr %119, i64 1
  %122 = load ptr, ptr %121, align 8, !noalias !1021
  %123 = bitcast ptr %122 to ptr
  %124 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %125 = icmp eq ptr %123, %124
  br i1 %125, label %126, label %128

126:                                              ; preds = %115
  %127 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %117, i64 noundef 24), !noalias !1021, !intel_dtrans_type !1018
  br label %130

128:                                              ; preds = %115
  %129 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %117, i64 noundef 24), !noalias !1021, !intel_dtrans_type !1018
  br label %130

130:                                              ; preds = %128, %126
  %131 = phi ptr [ %127, %126 ], [ %129, %128 ]
  br label %132

132:                                              ; preds = %130
  store ptr %131, ptr %3, align 8, !noalias !1021
  %133 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %131, i64 0, i32 2
  store ptr %131, ptr %133, align 8, !noalias !1021
  %134 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %131, i64 0, i32 1
  store ptr %131, ptr %134, align 8, !noalias !1021
  br label %135

135:                                              ; preds = %132, %112
  %136 = phi ptr [ %131, %132 ], [ %114, %112 ]
  %137 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 2
  %138 = load ptr, ptr %137, align 8
  %139 = icmp eq ptr %138, null
  br i1 %139, label %143, label %140

140:                                              ; preds = %135
  %141 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %138, i64 0, i32 2
  %142 = load ptr, ptr %141, align 8
  br label %161

143:                                              ; preds = %135
  %144 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %145 = load ptr, ptr %144, align 8
  %146 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %145, i64 0, i32 0
  %147 = load ptr, ptr %146, align 8
  %148 = tail call i1 @llvm.type.test(ptr %147, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %148)
  %149 = getelementptr inbounds ptr, ptr %147, i64 1
  %150 = load ptr, ptr %149, align 8
  %151 = bitcast ptr %150 to ptr
  %152 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %153 = icmp eq ptr %151, %152
  br i1 %153, label %154, label %156

154:                                              ; preds = %143
  %155 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %145, i64 noundef 24), !intel_dtrans_type !1018
  br label %158

156:                                              ; preds = %143
  %157 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %145, i64 noundef 24), !intel_dtrans_type !1018
  br label %158

158:                                              ; preds = %156, %154
  %159 = phi ptr [ %155, %154 ], [ %157, %156 ]
  br label %160

160:                                              ; preds = %158
  br label %161

161:                                              ; preds = %160, %140
  %162 = phi ptr [ %138, %140 ], [ %159, %160 ]
  %163 = phi ptr [ %142, %140 ], [ null, %160 ]
  %164 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %162, i64 0, i32 0
  store ptr %55, ptr %164, align 8
  %165 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %162, i64 0, i32 1
  %166 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %136, i64 0, i32 1
  %167 = load ptr, ptr %166, align 8
  store ptr %167, ptr %165, align 8
  %168 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %162, i64 0, i32 2
  store ptr %136, ptr %168, align 8
  %169 = load ptr, ptr %166, align 8
  %170 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %169, i64 0, i32 2
  store ptr %162, ptr %170, align 8
  store ptr %162, ptr %166, align 8
  store ptr %163, ptr %137, align 8
  %171 = load ptr, ptr %3, align 8, !noalias !1024
  %172 = icmp eq ptr %171, null
  br i1 %172, label %177, label %173

173:                                              ; preds = %161, %30
  %174 = phi ptr [ %171, %161 ], [ %4, %30 ]
  %175 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %174, i64 0, i32 2
  %176 = load ptr, ptr %175, align 8, !noalias !1024
  br label %197

177:                                              ; preds = %161
  %178 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %179 = load ptr, ptr %178, align 8, !noalias !1024
  %180 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %179, i64 0, i32 0
  %181 = load ptr, ptr %180, align 8, !noalias !1024
  %182 = tail call i1 @llvm.type.test(ptr %181, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %182)
  %183 = getelementptr inbounds ptr, ptr %181, i64 1
  %184 = load ptr, ptr %183, align 8, !noalias !1024
  %185 = bitcast ptr %184 to ptr
  %186 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %187 = icmp eq ptr %185, %186
  br i1 %187, label %188, label %190

188:                                              ; preds = %177
  %189 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %179, i64 noundef 24), !noalias !1024, !intel_dtrans_type !1018
  br label %192

190:                                              ; preds = %177
  %191 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %179, i64 noundef 24), !noalias !1024, !intel_dtrans_type !1018
  br label %192

192:                                              ; preds = %190, %188
  %193 = phi ptr [ %189, %188 ], [ %191, %190 ]
  br label %194

194:                                              ; preds = %192
  store ptr %193, ptr %3, align 8, !noalias !1024
  %195 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %193, i64 0, i32 2
  store ptr %193, ptr %195, align 8, !noalias !1024
  %196 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %193, i64 0, i32 1
  store ptr %193, ptr %196, align 8, !noalias !1024
  br label %197

197:                                              ; preds = %194, %173
  %198 = phi ptr [ %193, %194 ], [ %176, %173 ]
  %199 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %198, i64 0, i32 0
  %200 = load ptr, ptr %199, align 8
  %201 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %200, i64 0, i32 1
  %202 = load i16, ptr %201, align 8
  %203 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %200, i64 0, i32 2
  %204 = load i16, ptr %203, align 2
  %205 = icmp eq i16 %202, %204
  br i1 %205, label %222, label %206

206:                                              ; preds = %197
  %207 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %200, i64 0, i32 1
  %208 = load i16, ptr %207, align 8
  %209 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %200, i64 0, i32 2
  %210 = load i16, ptr %209, align 2
  %211 = icmp eq i16 %208, %210
  %212 = zext i16 %208 to i64
  %213 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %200, i64 0, i32 3
  %214 = load ptr, ptr %213, align 8
  br i1 %211, label %217, label %215

215:                                              ; preds = %206
  %216 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %214, i64 %212
  br label %222

217:                                              ; preds = %206
  %218 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %214, i64 %212
  %219 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %218, i64 0, i32 0
  %220 = load i16, ptr %219, align 4
  store i16 %220, ptr %209, align 2
  %221 = add i16 %202, 1
  store i16 %221, ptr %201, align 8
  br label %222

222:                                              ; preds = %217, %215, %197
  %223 = phi ptr [ null, %197 ], [ %216, %215 ], [ %218, %217 ]
  ret ptr %223
}

; Function Attrs: uwtable
define hidden void @"?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %1) unnamed_addr #13 comdat align 2 !intel.dtrans.func.type !1027 {
  %3 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %4 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8, !noalias !1028
  %6 = icmp eq ptr %5, null
  br i1 %6, label %10, label %7

7:                                                ; preds = %2
  %8 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %5, i64 0, i32 2
  %9 = load ptr, ptr %8, align 8, !noalias !1028
  br label %30

10:                                               ; preds = %2
  %11 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %12 = load ptr, ptr %11, align 8, !noalias !1028
  %13 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %12, i64 0, i32 0
  %14 = load ptr, ptr %13, align 8, !noalias !1028
  %15 = tail call i1 @llvm.type.test(ptr %14, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %15)
  %16 = getelementptr inbounds ptr, ptr %14, i64 1
  %17 = load ptr, ptr %16, align 8, !noalias !1028
  %18 = bitcast ptr %17 to ptr
  %19 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %20 = icmp eq ptr %18, %19
  br i1 %20, label %21, label %23

21:                                               ; preds = %10
  %22 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %12, i64 noundef 24), !noalias !1028, !intel_dtrans_type !1018
  br label %25

23:                                               ; preds = %10
  %24 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %12, i64 noundef 24), !noalias !1028, !intel_dtrans_type !1018
  br label %25

25:                                               ; preds = %23, %21
  %26 = phi ptr [ %22, %21 ], [ %24, %23 ]
  br label %27

27:                                               ; preds = %25
  store ptr %26, ptr %4, align 8, !noalias !1028
  %28 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %26, i64 0, i32 2
  store ptr %26, ptr %28, align 8, !noalias !1028
  %29 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %26, i64 0, i32 1
  store ptr %26, ptr %29, align 8, !noalias !1028
  br label %30

30:                                               ; preds = %27, %7
  %31 = phi ptr [ %26, %27 ], [ %5, %7 ]
  %32 = phi ptr [ %26, %27 ], [ %9, %7 ]
  %33 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 0
  %34 = load ptr, ptr %33, align 8
  %35 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %34, i64 0, i32 2
  %36 = load i16, ptr %35, align 2
  %37 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %34, i64 0, i32 1
  store i16 %36, ptr %37, align 8
  %38 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 1
  %39 = load i16, ptr %38, align 8
  %40 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 2
  %41 = load i16, ptr %40, align 2
  %42 = icmp ult i16 %39, %41
  br i1 %42, label %61, label %43

43:                                               ; preds = %30
  %44 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %31, i64 0, i32 2
  %45 = load ptr, ptr %44, align 8, !noalias !1031
  %46 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %45, i64 0, i32 2
  %47 = load ptr, ptr %46, align 8
  %48 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %45, i64 0, i32 1
  %49 = load ptr, ptr %48, align 8
  %50 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %45, i64 0, i32 0
  %51 = load ptr, ptr %50, align 8
  %52 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %49, i64 0, i32 2
  store ptr %47, ptr %52, align 8
  %53 = load ptr, ptr %46, align 8
  %54 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %53, i64 0, i32 1
  store ptr %49, ptr %54, align 8
  store ptr null, ptr %48, align 8
  %55 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 2
  %56 = load ptr, ptr %55, align 8
  store ptr %51, ptr %50, align 8
  %57 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %31, i64 0, i32 1
  %58 = load ptr, ptr %57, align 8
  store ptr %58, ptr %48, align 8
  store ptr %31, ptr %46, align 8
  %59 = load ptr, ptr %57, align 8
  %60 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %59, i64 0, i32 2
  store ptr %45, ptr %60, align 8
  store ptr %45, ptr %57, align 8
  store ptr %56, ptr %55, align 8
  br label %61

61:                                               ; preds = %43, %30
  ret void
}

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1034 hidden noundef zeroext i1 @"?ownsObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEBA_NPEBVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1", ptr noundef readnone "intel_dtrans_func_index"="2") unnamed_addr #6 align 2

; Function Attrs: uwtable
define hidden void @"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ"(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %0) unnamed_addr #13 comdat align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !1035 {
  %2 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %3 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %4 = load ptr, ptr %3, align 8
  %5 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 1
  %6 = load ptr, ptr %5, align 8, !noalias !1014
  %7 = icmp eq ptr %6, null
  br i1 %7, label %8, label %26

8:                                                ; preds = %1
  %9 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %4, i64 0, i32 0
  %10 = load ptr, ptr %9, align 8, !noalias !1037
  %11 = tail call i1 @llvm.type.test(ptr %10, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %11)
  %12 = getelementptr inbounds ptr, ptr %10, i64 1
  %13 = load ptr, ptr %12, align 8, !noalias !1037
  %14 = bitcast ptr %13 to ptr
  %15 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %16 = icmp eq ptr %14, %15
  br i1 %16, label %17, label %19

17:                                               ; preds = %8
  %18 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %4, i64 noundef 24), !noalias !1037, !intel_dtrans_type !1018
  br label %21

19:                                               ; preds = %8
  %20 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %4, i64 noundef 24), !noalias !1037, !intel_dtrans_type !1018
  br label %21

21:                                               ; preds = %19, %17
  %22 = phi ptr [ %18, %17 ], [ %20, %19 ]
  br label %23

23:                                               ; preds = %21
  store ptr %22, ptr %5, align 8, !noalias !1037
  %24 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %22, i64 0, i32 2
  store ptr %22, ptr %24, align 8, !noalias !1037
  %25 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %22, i64 0, i32 1
  store ptr %22, ptr %25, align 8, !noalias !1037
  br label %138

26:                                               ; preds = %1
  %27 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %6, i64 0, i32 2
  %28 = load ptr, ptr %27, align 8, !noalias !1040
  %29 = icmp eq ptr %28, %6
  br i1 %29, label %116, label %30

30:                                               ; preds = %110, %26
  %31 = phi ptr [ %112, %110 ], [ %28, %26 ]
  %32 = getelementptr %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %31, i64 0, i32 0
  %33 = load ptr, ptr %32, align 8, !noalias !1043
  %34 = icmp eq ptr %33, null
  br i1 %34, label %110, label %35

35:                                               ; preds = %30
  %36 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %33, i64 0, i32 1
  %37 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %33, i64 0, i32 2
  %38 = load i16, ptr %37, align 2, !noalias !1043
  %39 = icmp eq i16 %38, 0
  br i1 %39, label %48, label %40

40:                                               ; preds = %35
  %41 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %33, i64 0, i32 3
  br label %42

42:                                               ; preds = %91, %40
  %43 = phi i16 [ %38, %40 ], [ %93, %91 ]
  %44 = phi i64 [ 0, %40 ], [ %95, %91 ]
  %45 = phi i16 [ 0, %40 ], [ %94, %91 ]
  %46 = load i16, ptr %36, align 8, !noalias !1043
  %47 = icmp ult i16 %45, %46
  br i1 %47, label %69, label %48

48:                                               ; preds = %91, %42, %35
  %49 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %33, i64 0, i32 3
  %50 = load ptr, ptr %49, align 8, !noalias !1043
  %51 = icmp eq ptr %50, null
  br i1 %51, label %97, label %52

52:                                               ; preds = %48
  %53 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %33, i64 0, i32 0
  %54 = getelementptr inbounds %"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator", ptr %53, i64 0, i32 0
  %55 = load ptr, ptr %54, align 8, !noalias !1043
  %56 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %55, i64 0, i32 0
  %57 = load ptr, ptr %56, align 8, !noalias !1043
  %58 = tail call i1 @llvm.type.test(ptr %57, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %58)
  %59 = getelementptr inbounds ptr, ptr %57, i64 2
  %60 = load ptr, ptr %59, align 8, !noalias !1043
  %61 = bitcast ptr %60 to ptr
  %62 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %63 = icmp eq ptr %61, %62
  br i1 %63, label %64, label %65

64:                                               ; preds = %52
  invoke void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %55, ptr noundef nonnull %50)
          to label %66 unwind label %67, !noalias !1043, !intel_dtrans_type !1046

65:                                               ; preds = %52
  invoke void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %55, ptr noundef nonnull %50)
          to label %66 unwind label %67, !noalias !1043, !intel_dtrans_type !1046

66:                                               ; preds = %65, %64
  br label %97

67:                                               ; preds = %65, %64
  %68 = cleanuppad within none []
  call void @__std_terminate() #21 [ "funclet"(token %68) ], !noalias !1043
  unreachable

69:                                               ; preds = %42
  %70 = load ptr, ptr %41, align 8, !noalias !1043
  %71 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %70, i64 %44
  %72 = zext i16 %43 to i64
  %73 = icmp ult i64 %44, %72
  br i1 %73, label %74, label %82

74:                                               ; preds = %69
  %75 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %71, i64 0, i32 1
  %76 = load i32, ptr %75, align 4, !noalias !1043
  %77 = icmp ne i32 %76, -2228259
  %78 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %71, i64 0, i32 0
  %79 = load i16, ptr %78, align 4, !noalias !1043
  %80 = icmp ugt i16 %79, %43
  %81 = select i1 %77, i1 true, i1 %80
  br i1 %81, label %82, label %91

82:                                               ; preds = %74, %69
  %83 = getelementptr %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %71, i64 0, i32 0, i32 0, i32 0, i32 0
  %84 = load ptr, ptr %83, align 8, !noalias !1043
  %85 = tail call i1 @llvm.type.test(ptr %84, metadata !"?AVXStringCached@xalanc_1_10@@")
  tail call void @llvm.assume(i1 %85)
  %86 = load ptr, ptr %84, align 8, !noalias !1043
  %87 = tail call noundef ptr @"??_GXStringCached@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull align 8 dereferenceable(88) %71, i32 noundef 0) #20, !noalias !1043, !intel_dtrans_type !1048
  %88 = add nuw i16 %45, 1
  %89 = load i16, ptr %37, align 2, !noalias !1043
  %90 = zext i16 %89 to i64
  br label %91

91:                                               ; preds = %82, %74
  %92 = phi i64 [ %90, %82 ], [ %72, %74 ]
  %93 = phi i16 [ %89, %82 ], [ %43, %74 ]
  %94 = phi i16 [ %88, %82 ], [ %45, %74 ]
  %95 = add nuw nsw i64 %44, 1
  %96 = icmp ult i64 %95, %92
  br i1 %96, label %42, label %48, !llvm.loop !1049

97:                                               ; preds = %66, %48
  %98 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %4, i64 0, i32 0
  %99 = load ptr, ptr %98, align 8, !noalias !1043
  %100 = tail call i1 @llvm.type.test(ptr %99, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %100)
  %101 = getelementptr inbounds ptr, ptr %99, i64 2
  %102 = load ptr, ptr %101, align 8, !noalias !1043
  %103 = bitcast ptr %102 to ptr
  %104 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %105 = icmp eq ptr %103, %104
  br i1 %105, label %106, label %107

106:                                              ; preds = %97
  tail call void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %4, ptr noundef nonnull %33), !noalias !1043, !intel_dtrans_type !1046
  br label %108

107:                                              ; preds = %97
  tail call void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %4, ptr noundef nonnull %33), !noalias !1043, !intel_dtrans_type !1046
  br label %108

108:                                              ; preds = %107, %106
  br label %109

109:                                              ; preds = %108
  br label %110

110:                                              ; preds = %109, %30
  %111 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %31, i64 0, i32 2
  %112 = load ptr, ptr %111, align 8, !noalias !1043
  %113 = icmp eq ptr %112, %6
  br i1 %113, label %114, label %30, !llvm.loop !1050

114:                                              ; preds = %110
  %115 = load ptr, ptr %5, align 8, !noalias !1051
  br label %116

116:                                              ; preds = %114, %26
  %117 = phi ptr [ %115, %114 ], [ %28, %26 ]
  %118 = icmp eq ptr %117, null
  br i1 %118, label %119, label %138

119:                                              ; preds = %116
  %120 = load ptr, ptr %3, align 8, !noalias !1051
  %121 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %120, i64 0, i32 0
  %122 = load ptr, ptr %121, align 8, !noalias !1051
  %123 = tail call i1 @llvm.type.test(ptr %122, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %123)
  %124 = getelementptr inbounds ptr, ptr %122, i64 1
  %125 = load ptr, ptr %124, align 8, !noalias !1051
  %126 = bitcast ptr %125 to ptr
  %127 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %128 = icmp eq ptr %126, %127
  br i1 %128, label %129, label %131

129:                                              ; preds = %119
  %130 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %120, i64 noundef 24), !noalias !1051, !intel_dtrans_type !1018
  br label %133

131:                                              ; preds = %119
  %132 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %120, i64 noundef 24), !noalias !1051, !intel_dtrans_type !1018
  br label %133

133:                                              ; preds = %131, %129
  %134 = phi ptr [ %130, %129 ], [ %132, %131 ]
  br label %135

135:                                              ; preds = %133
  store ptr %134, ptr %5, align 8, !noalias !1051
  %136 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %134, i64 0, i32 2
  store ptr %134, ptr %136, align 8, !noalias !1051
  %137 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %134, i64 0, i32 1
  store ptr %134, ptr %137, align 8, !noalias !1051
  br label %158

138:                                              ; preds = %116, %23
  %139 = phi ptr [ %22, %23 ], [ %117, %116 ]
  %140 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %139, i64 0, i32 2
  %141 = load ptr, ptr %140, align 8, !noalias !1051
  %142 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 2
  %143 = icmp eq ptr %141, %139
  br i1 %143, label %158, label %144

144:                                              ; preds = %138
  %145 = load ptr, ptr %142, align 8
  br label %146

146:                                              ; preds = %146, %144
  %147 = phi ptr [ %145, %144 ], [ %148, %146 ]
  %148 = phi ptr [ %141, %144 ], [ %150, %146 ]
  %149 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %148, i64 0, i32 2
  %150 = load ptr, ptr %149, align 8, !noalias !1054
  %151 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %148, i64 0, i32 1
  %152 = load ptr, ptr %151, align 8
  %153 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %152, i64 0, i32 2
  store ptr %150, ptr %153, align 8
  %154 = load ptr, ptr %149, align 8
  %155 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %154, i64 0, i32 1
  store ptr %152, ptr %155, align 8
  store ptr null, ptr %151, align 8
  store ptr %147, ptr %149, align 8
  %156 = icmp eq ptr %150, %139
  br i1 %156, label %157, label %146, !llvm.loop !1057

157:                                              ; preds = %146
  store ptr %148, ptr %142, align 8
  br label %158

158:                                              ; preds = %157, %138, %135
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define hidden noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @"?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ"(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %0) #14 comdat align 2 !type !1058 !intel.dtrans.func.type !1059 {
  %2 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %3 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %2, i64 0, i32 0
  %4 = load ptr, ptr %3, align 8
  ret ptr %4
}

; Function Attrs: nounwind uwtable
define hidden void @"??1?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAA@XZ"(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %0) unnamed_addr #15 comdat align 2 personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !1060 {
  %2 = getelementptr %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 0
  store ptr @"??_7?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@", ptr %2, align 8
  %3 = tail call i1 @llvm.type.test(ptr nonnull @"??_7?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@6B@", metadata !"?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@")
  tail call void @llvm.assume(i1 %3)
  invoke void @"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ"(ptr noundef nonnull align 8 dereferenceable(40) %0)
          to label %4 unwind label %89, !intel_dtrans_type !1061

4:                                                ; preds = %1
  %5 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %6 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %5, i64 0, i32 1
  %7 = load ptr, ptr %6, align 8
  %8 = icmp eq ptr %7, null
  br i1 %8, label %88, label %9

9:                                                ; preds = %4
  %10 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %7, i64 0, i32 2
  %11 = load ptr, ptr %10, align 8, !noalias !1062
  %12 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %5, i64 0, i32 0
  br label %13

13:                                               ; preds = %53, %9
  %14 = phi ptr [ %11, %9 ], [ %41, %53 ]
  %15 = load ptr, ptr %6, align 8, !noalias !1065
  %16 = icmp eq ptr %15, null
  br i1 %16, label %17, label %36

17:                                               ; preds = %13
  %18 = load ptr, ptr %12, align 8, !noalias !1065
  %19 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %18, i64 0, i32 0
  %20 = load ptr, ptr %19, align 8, !noalias !1065
  %21 = tail call i1 @llvm.type.test(ptr %20, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %21)
  %22 = getelementptr inbounds ptr, ptr %20, i64 1
  %23 = load ptr, ptr %22, align 8, !noalias !1065
  %24 = bitcast ptr %23 to ptr
  %25 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %26 = icmp eq ptr %24, %25
  br i1 %26, label %27, label %29

27:                                               ; preds = %17
  %28 = invoke noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %18, i64 noundef 24)
          to label %31 unwind label %86, !intel_dtrans_type !1018

29:                                               ; preds = %17
  %30 = invoke noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %18, i64 noundef 24)
          to label %31 unwind label %86, !intel_dtrans_type !1018

31:                                               ; preds = %29, %27
  %32 = phi ptr [ %28, %27 ], [ %30, %29 ]
  br label %33

33:                                               ; preds = %31
  store ptr %32, ptr %6, align 8, !noalias !1065
  %34 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 2
  store ptr %32, ptr %34, align 8, !noalias !1065
  %35 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 1
  store ptr %32, ptr %35, align 8, !noalias !1065
  br label %36

36:                                               ; preds = %33, %13
  %37 = phi ptr [ %32, %33 ], [ %15, %13 ]
  %38 = icmp eq ptr %14, %37
  br i1 %38, label %54, label %39

39:                                               ; preds = %36
  %40 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %14, i64 0, i32 2
  %41 = load ptr, ptr %40, align 8, !noalias !1068
  %42 = load ptr, ptr %12, align 8
  %43 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %42, i64 0, i32 0
  %44 = load ptr, ptr %43, align 8
  %45 = tail call i1 @llvm.type.test(ptr %44, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %45)
  %46 = getelementptr inbounds ptr, ptr %44, i64 2
  %47 = load ptr, ptr %46, align 8
  %48 = bitcast ptr %47 to ptr
  %49 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %50 = icmp eq ptr %48, %49
  br i1 %50, label %51, label %52

51:                                               ; preds = %39
  invoke void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %42, ptr noundef nonnull %14)
          to label %53 unwind label %86, !llvm.loop !1071, !intel_dtrans_type !1046

52:                                               ; preds = %39
  invoke void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %42, ptr noundef nonnull %14)
          to label %53 unwind label %86, !llvm.loop !1071, !intel_dtrans_type !1046

53:                                               ; preds = %52, %51
  br label %13

54:                                               ; preds = %36
  %55 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %5, i64 0, i32 2
  %56 = load ptr, ptr %55, align 8
  br label %57

57:                                               ; preds = %74, %54
  %58 = phi ptr [ %56, %54 ], [ %65, %74 ]
  %59 = icmp eq ptr %58, null
  %60 = load ptr, ptr %12, align 8
  %61 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %60, i64 0, i32 0
  %62 = load ptr, ptr %61, align 8
  br i1 %59, label %75, label %63

63:                                               ; preds = %57
  %64 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %58, i64 0, i32 2
  %65 = load ptr, ptr %64, align 8
  %66 = tail call i1 @llvm.type.test(ptr %62, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %66)
  %67 = getelementptr inbounds ptr, ptr %62, i64 2
  %68 = load ptr, ptr %67, align 8
  %69 = bitcast ptr %68 to ptr
  %70 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %71 = icmp eq ptr %69, %70
  br i1 %71, label %72, label %73

72:                                               ; preds = %63
  invoke void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %60, ptr noundef nonnull %58)
          to label %74 unwind label %86, !intel_dtrans_type !1046

73:                                               ; preds = %63
  invoke void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %60, ptr noundef nonnull %58)
          to label %74 unwind label %86, !intel_dtrans_type !1046

74:                                               ; preds = %73, %72
  br label %57

75:                                               ; preds = %57
  %76 = load ptr, ptr %6, align 8
  %77 = tail call i1 @llvm.type.test(ptr %62, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %77)
  %78 = getelementptr inbounds ptr, ptr %62, i64 2
  %79 = load ptr, ptr %78, align 8
  %80 = bitcast ptr %79 to ptr
  %81 = bitcast ptr @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z" to ptr
  %82 = icmp eq ptr %80, %81
  br i1 %82, label %83, label %84

83:                                               ; preds = %75
  invoke void @"?deallocate@MemoryManagerImpl@xercesc_2_7@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %60, ptr noundef %76)
          to label %85 unwind label %86, !intel_dtrans_type !1046

84:                                               ; preds = %75
  invoke void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr noundef nonnull align 8 dereferenceable(8) %60, ptr noundef %76)
          to label %85 unwind label %86, !intel_dtrans_type !1046

85:                                               ; preds = %84, %83
  br label %88

86:                                               ; preds = %84, %83, %73, %72, %52, %51, %29, %27
  %87 = cleanuppad within none []
  call void @__std_terminate() #21 [ "funclet"(token %87) ]
  unreachable

88:                                               ; preds = %85, %4
  ret void

89:                                               ; preds = %1
  %90 = cleanuppad within none []
  call void @__std_terminate() #21 [ "funclet"(token %90) ]
  unreachable
}

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1072 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @"??_G?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull returned align 8 dereferenceable(40) "intel_dtrans_func_index"="2", i32 noundef) unnamed_addr #11 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1073 hidden noundef "intel_dtrans_func_index"="1" ptr @"?allocateBlock@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ"(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="2") unnamed_addr #6 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1074 hidden void @"?commitAllocation@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1", ptr nocapture noundef readnone "intel_dtrans_func_index"="2") unnamed_addr #6 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1075 hidden noundef zeroext i1 @"?ownsObject@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEBA_NPEBVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1", ptr noundef readonly "intel_dtrans_func_index"="2") unnamed_addr #6 align 2

; Function Attrs: nounwind uwtable
define hidden void @"??1XStringCachedAllocator@xalanc_1_10@@QEAA@XZ"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %0) unnamed_addr #15 align 2 !intel.dtrans.func.type !1076 {
  %2 = getelementptr inbounds %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator", ptr %0, i64 0, i32 0
  tail call void @"??1?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAA@XZ"(ptr noundef nonnull align 8 dereferenceable(48) %2) #20
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @"?createString@XStringCachedAllocator@xalanc_1_10@@QEAAPEAVXStringCached@2@AEAVGetAndReleaseCachedString@XPathExecutionContext@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %0, ptr nocapture noundef nonnull align 8 dereferenceable(16) "intel_dtrans_func_index"="3" %1) #16 align 2 !type !1077 !intel.dtrans.func.type !1078 {
  %3 = getelementptr inbounds %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator", ptr %0, i64 0, i32 0
  %4 = tail call noundef ptr @"?allocateBlock@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAPEAVXStringCached@2@XZ"(ptr noundef nonnull align 8 dereferenceable(48) %3)
  %5 = tail call noundef nonnull align 8 dereferenceable(8) ptr @"?getMemoryManager@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@QEAAAEAVMemoryManager@xercesc_2_7@@XZ"(ptr noundef nonnull align 8 dereferenceable(40) %3)
  %6 = tail call noundef ptr @"??0XStringCached@xalanc_1_10@@QEAA@AEAVGetAndReleaseCachedString@XPathExecutionContext@1@AEAVMemoryManager@xercesc_2_7@@@Z"(ptr noundef nonnull align 8 dereferenceable(88) %4, ptr noundef nonnull align 8 dereferenceable(16) %1, ptr noundef nonnull align 8 dereferenceable(8) %5)
  tail call void @"?commitAllocation@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@UEAAXPEAVXStringCached@2@@Z"(ptr noundef nonnull align 8 dereferenceable(48) %3, ptr noundef nonnull %4)
  ret ptr %4
}

; Function Attrs: mustprogress uwtable
define hidden noundef zeroext i1 @"?destroy@XStringCachedAllocator@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %0, ptr noundef "intel_dtrans_func_index"="2" %1) #16 align 2 !type !1079 !intel.dtrans.func.type !1080 {
  %3 = getelementptr inbounds %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator", ptr %0, i64 0, i32 0
  %4 = tail call noundef zeroext i1 @"?destroyObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z"(ptr noundef nonnull align 8 dereferenceable(48) %3, ptr noundef %1)
  ret i1 %4
}

; Function Attrs: uwtable
define hidden noundef zeroext i1 @"?destroyObject@?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@QEAA_NPEAVXStringCached@2@@Z"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %0, ptr noundef "intel_dtrans_func_index"="2" %1) #13 comdat align 2 !type !1081 !intel.dtrans.func.type !1082 {
  %3 = getelementptr inbounds %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator", ptr %0, i64 0, i32 2
  %4 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8, !noalias !1014
  %6 = icmp eq ptr %5, null
  br i1 %6, label %7, label %27

7:                                                ; preds = %2
  %8 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %9 = load ptr, ptr %8, align 8, !noalias !1083
  %10 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %9, i64 0, i32 0
  %11 = load ptr, ptr %10, align 8, !noalias !1083
  %12 = tail call i1 @llvm.type.test(ptr %11, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %12)
  %13 = getelementptr inbounds ptr, ptr %11, i64 1
  %14 = load ptr, ptr %13, align 8, !noalias !1083
  %15 = bitcast ptr %14 to ptr
  %16 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %17 = icmp eq ptr %15, %16
  br i1 %17, label %18, label %20

18:                                               ; preds = %7
  %19 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %9, i64 noundef 24), !noalias !1083, !intel_dtrans_type !1018
  br label %22

20:                                               ; preds = %7
  %21 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %9, i64 noundef 24), !noalias !1083, !intel_dtrans_type !1018
  br label %22

22:                                               ; preds = %20, %18
  %23 = phi ptr [ %19, %18 ], [ %21, %20 ]
  br label %24

24:                                               ; preds = %22
  store ptr %23, ptr %4, align 8, !noalias !1083
  %25 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %23, i64 0, i32 2
  store ptr %23, ptr %25, align 8, !noalias !1083
  %26 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %23, i64 0, i32 1
  store ptr %23, ptr %26, align 8, !noalias !1083
  br label %479

27:                                               ; preds = %2
  %28 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %5, i64 0, i32 2
  %29 = load ptr, ptr %28, align 8, !noalias !1083
  %30 = icmp eq ptr %29, %5
  br i1 %30, label %479, label %31

31:                                               ; preds = %235, %27
  %32 = phi ptr [ %237, %235 ], [ %29, %27 ]
  %33 = getelementptr %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 0
  %34 = load ptr, ptr %33, align 8
  %35 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 1
  %36 = load i16, ptr %35, align 8
  %37 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 2
  %38 = load i16, ptr %37, align 2
  %39 = icmp ult i16 %36, %38
  br i1 %39, label %40, label %239

40:                                               ; preds = %31
  %41 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 3
  %42 = load ptr, ptr %41, align 8
  %43 = icmp ugt ptr %42, %1
  br i1 %43, label %235, label %44

44:                                               ; preds = %40
  %45 = zext i16 %38 to i64
  %46 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %42, i64 %45
  %47 = icmp ugt ptr %46, %1
  br i1 %47, label %48, label %235

48:                                               ; preds = %44
  %49 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 1
  %50 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %34, i64 0, i32 3
  %51 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 0
  %52 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %34, i64 0, i32 1
  %53 = load i16, ptr %52, align 8
  %54 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %34, i64 0, i32 2
  %55 = load i16, ptr %54, align 2
  %56 = icmp eq i16 %53, %55
  br i1 %56, label %62, label %57

57:                                               ; preds = %48
  %58 = zext i16 %53 to i64
  %59 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %42, i64 %58
  %60 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %59, i64 0, i32 0
  store i16 %55, ptr %60, align 4
  %61 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %59, i64 0, i32 1
  store i32 -2228259, ptr %61, align 4
  store i16 %53, ptr %54, align 2
  br label %62

62:                                               ; preds = %57, %48
  %63 = getelementptr %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %1, i64 0, i32 0, i32 0, i32 0, i32 0
  %64 = load ptr, ptr %63, align 8
  %65 = tail call i1 @llvm.type.test(ptr %64, metadata !"?AVXStringCached@xalanc_1_10@@")
  tail call void @llvm.assume(i1 %65)
  %66 = load ptr, ptr %64, align 8
  %67 = tail call noundef ptr @"??_GXStringCached@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull align 8 dereferenceable(88) %1, i32 noundef 0) #20, !intel_dtrans_type !1048
  %68 = load i16, ptr %52, align 8
  %69 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %1, i64 0, i32 0
  store i16 %68, ptr %69, align 4
  %70 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %1, i64 0, i32 1
  store i32 -2228259, ptr %70, align 4
  %71 = load ptr, ptr %50, align 8
  %72 = ptrtoint ptr %1 to i64
  %73 = ptrtoint ptr %71 to i64
  %74 = sub i64 %72, %73
  %75 = sdiv exact i64 %74, 88
  %76 = trunc i64 %75 to i16
  store i16 %76, ptr %54, align 2
  store i16 %76, ptr %52, align 8
  %77 = load i16, ptr %49, align 8
  %78 = add i16 %77, -1
  store i16 %78, ptr %49, align 8
  %79 = load ptr, ptr %4, align 8, !noalias !1086
  %80 = icmp eq ptr %79, null
  br i1 %80, label %84, label %81

81:                                               ; preds = %62
  %82 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %79, i64 0, i32 2
  %83 = load ptr, ptr %82, align 8, !noalias !1086
  br label %104

84:                                               ; preds = %62
  %85 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %86 = load ptr, ptr %85, align 8, !noalias !1086
  %87 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %86, i64 0, i32 0
  %88 = load ptr, ptr %87, align 8, !noalias !1086
  %89 = tail call i1 @llvm.type.test(ptr %88, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %89)
  %90 = getelementptr inbounds ptr, ptr %88, i64 1
  %91 = load ptr, ptr %90, align 8, !noalias !1086
  %92 = bitcast ptr %91 to ptr
  %93 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %94 = icmp eq ptr %92, %93
  br i1 %94, label %95, label %97

95:                                               ; preds = %84
  %96 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %86, i64 noundef 24), !noalias !1086, !intel_dtrans_type !1018
  br label %99

97:                                               ; preds = %84
  %98 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %86, i64 noundef 24), !noalias !1086, !intel_dtrans_type !1018
  br label %99

99:                                               ; preds = %97, %95
  %100 = phi ptr [ %96, %95 ], [ %98, %97 ]
  br label %101

101:                                              ; preds = %99
  store ptr %100, ptr %4, align 8, !noalias !1086
  %102 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %100, i64 0, i32 2
  store ptr %100, ptr %102, align 8, !noalias !1086
  %103 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %100, i64 0, i32 1
  store ptr %100, ptr %103, align 8, !noalias !1086
  br label %104

104:                                              ; preds = %101, %81
  %105 = phi ptr [ %100, %101 ], [ %83, %81 ]
  %106 = icmp eq ptr %32, %105
  br i1 %106, label %178, label %107

107:                                              ; preds = %104
  %108 = load ptr, ptr %51, align 8
  %109 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 2
  %110 = load ptr, ptr %109, align 8
  %111 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 1
  %112 = load ptr, ptr %111, align 8
  %113 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %112, i64 0, i32 2
  store ptr %110, ptr %113, align 8
  %114 = load ptr, ptr %109, align 8
  %115 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %114, i64 0, i32 1
  store ptr %112, ptr %115, align 8
  store ptr null, ptr %111, align 8
  %116 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 2
  %117 = load ptr, ptr %116, align 8
  store ptr %117, ptr %109, align 8
  store ptr %32, ptr %116, align 8
  %118 = load ptr, ptr %4, align 8, !noalias !1089
  %119 = icmp eq ptr %118, null
  br i1 %119, label %123, label %120

120:                                              ; preds = %107
  %121 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %118, i64 0, i32 2
  %122 = load ptr, ptr %121, align 8, !noalias !1089
  br label %145

123:                                              ; preds = %107
  %124 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %125 = load ptr, ptr %124, align 8, !noalias !1089
  %126 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %125, i64 0, i32 0
  %127 = load ptr, ptr %126, align 8, !noalias !1089
  %128 = tail call i1 @llvm.type.test(ptr %127, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %128)
  %129 = getelementptr inbounds ptr, ptr %127, i64 1
  %130 = load ptr, ptr %129, align 8, !noalias !1089
  %131 = bitcast ptr %130 to ptr
  %132 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %133 = icmp eq ptr %131, %132
  br i1 %133, label %134, label %136

134:                                              ; preds = %123
  %135 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %125, i64 noundef 24), !noalias !1089, !intel_dtrans_type !1018
  br label %138

136:                                              ; preds = %123
  %137 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %125, i64 noundef 24), !noalias !1089, !intel_dtrans_type !1018
  br label %138

138:                                              ; preds = %136, %134
  %139 = phi ptr [ %135, %134 ], [ %137, %136 ]
  br label %140

140:                                              ; preds = %138
  store ptr %139, ptr %4, align 8, !noalias !1089
  %141 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %139, i64 0, i32 2
  store ptr %139, ptr %141, align 8, !noalias !1089
  %142 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %139, i64 0, i32 1
  store ptr %139, ptr %142, align 8, !noalias !1089
  %143 = load ptr, ptr %116, align 8
  %144 = icmp eq ptr %143, null
  br i1 %144, label %150, label %145

145:                                              ; preds = %140, %120
  %146 = phi ptr [ %122, %120 ], [ %139, %140 ]
  %147 = phi ptr [ %32, %120 ], [ %143, %140 ]
  %148 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %147, i64 0, i32 2
  %149 = load ptr, ptr %148, align 8
  br label %167

150:                                              ; preds = %140
  %151 = load ptr, ptr %124, align 8
  %152 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %151, i64 0, i32 0
  %153 = load ptr, ptr %152, align 8
  %154 = tail call i1 @llvm.type.test(ptr %153, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %154)
  %155 = getelementptr inbounds ptr, ptr %153, i64 1
  %156 = load ptr, ptr %155, align 8
  %157 = bitcast ptr %156 to ptr
  %158 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %159 = icmp eq ptr %157, %158
  br i1 %159, label %160, label %162

160:                                              ; preds = %150
  %161 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %151, i64 noundef 24), !intel_dtrans_type !1018
  br label %164

162:                                              ; preds = %150
  %163 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %151, i64 noundef 24), !intel_dtrans_type !1018
  br label %164

164:                                              ; preds = %162, %160
  %165 = phi ptr [ %161, %160 ], [ %163, %162 ]
  br label %166

166:                                              ; preds = %164
  br label %167

167:                                              ; preds = %166, %145
  %168 = phi ptr [ %146, %145 ], [ %139, %166 ]
  %169 = phi ptr [ %147, %145 ], [ %165, %166 ]
  %170 = phi ptr [ %149, %145 ], [ null, %166 ]
  %171 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %169, i64 0, i32 0
  store ptr %108, ptr %171, align 8
  %172 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %169, i64 0, i32 1
  %173 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %168, i64 0, i32 1
  %174 = load ptr, ptr %173, align 8
  store ptr %174, ptr %172, align 8
  %175 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %169, i64 0, i32 2
  store ptr %168, ptr %175, align 8
  %176 = load ptr, ptr %173, align 8
  %177 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %176, i64 0, i32 2
  store ptr %169, ptr %177, align 8
  store ptr %169, ptr %173, align 8
  store ptr %170, ptr %116, align 8
  br label %178

178:                                              ; preds = %167, %104
  %179 = getelementptr inbounds %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator", ptr %0, i64 0, i32 1
  %180 = load i8, ptr %179, align 8, !range !1092
  %181 = icmp eq i8 %180, 0
  br i1 %181, label %239, label %182

182:                                              ; preds = %178
  %183 = load ptr, ptr %4, align 8, !noalias !1014
  %184 = icmp eq ptr %183, null
  br i1 %184, label %185, label %205

185:                                              ; preds = %182
  %186 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %187 = load ptr, ptr %186, align 8, !noalias !1093
  %188 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %187, i64 0, i32 0
  %189 = load ptr, ptr %188, align 8, !noalias !1093
  %190 = tail call i1 @llvm.type.test(ptr %189, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %190)
  %191 = getelementptr inbounds ptr, ptr %189, i64 1
  %192 = load ptr, ptr %191, align 8, !noalias !1093
  %193 = bitcast ptr %192 to ptr
  %194 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %195 = icmp eq ptr %193, %194
  br i1 %195, label %196, label %198

196:                                              ; preds = %185
  %197 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %187, i64 noundef 24), !noalias !1093, !intel_dtrans_type !1018
  br label %200

198:                                              ; preds = %185
  %199 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %187, i64 noundef 24), !noalias !1093, !intel_dtrans_type !1018
  br label %200

200:                                              ; preds = %198, %196
  %201 = phi ptr [ %197, %196 ], [ %199, %198 ]
  br label %202

202:                                              ; preds = %200
  store ptr %201, ptr %4, align 8, !noalias !1093
  %203 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %201, i64 0, i32 2
  store ptr %201, ptr %203, align 8, !noalias !1093
  %204 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %201, i64 0, i32 1
  store ptr %201, ptr %204, align 8, !noalias !1093
  br label %475

205:                                              ; preds = %182
  %206 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %183, i64 0, i32 2
  %207 = load ptr, ptr %206, align 8, !noalias !1093
  %208 = icmp eq ptr %207, %183
  br i1 %208, label %239, label %209

209:                                              ; preds = %205
  %210 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %207, i64 0, i32 0
  %211 = load ptr, ptr %210, align 8
  %212 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %211, i64 0, i32 1
  %213 = load i16, ptr %212, align 8
  %214 = icmp eq i16 %213, 0
  br i1 %214, label %215, label %239

215:                                              ; preds = %209
  %216 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %207, i64 0, i32 2
  %217 = load ptr, ptr %216, align 8, !noalias !1096
  %218 = icmp eq ptr %217, %183
  br i1 %218, label %227, label %219

219:                                              ; preds = %215
  %220 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %217, i64 0, i32 0
  %221 = load ptr, ptr %220, align 8
  %222 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %221, i64 0, i32 1
  %223 = load i16, ptr %222, align 8
  %224 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %221, i64 0, i32 2
  %225 = load i16, ptr %224, align 2
  %226 = icmp ult i16 %223, %225
  br i1 %226, label %227, label %239

227:                                              ; preds = %219, %215
  %228 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 2
  %229 = load ptr, ptr %228, align 8
  %230 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %207, i64 0, i32 1
  %231 = load ptr, ptr %230, align 8
  %232 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %231, i64 0, i32 2
  store ptr %217, ptr %232, align 8
  %233 = load ptr, ptr %216, align 8
  %234 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %233, i64 0, i32 1
  store ptr %231, ptr %234, align 8
  store ptr null, ptr %230, align 8
  store ptr %229, ptr %216, align 8
  store ptr %207, ptr %228, align 8
  br label %239

235:                                              ; preds = %44, %40
  %236 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %32, i64 0, i32 2
  %237 = load ptr, ptr %236, align 8, !noalias !1099
  %238 = icmp eq ptr %237, %5
  br i1 %238, label %239, label %31, !llvm.loop !1102

239:                                              ; preds = %235, %227, %219, %209, %205, %178, %31
  %240 = phi ptr [ %32, %227 ], [ %32, %219 ], [ %32, %209 ], [ %32, %205 ], [ %32, %178 ], [ %5, %235 ], [ %32, %31 ]
  %241 = phi i1 [ false, %227 ], [ false, %219 ], [ false, %209 ], [ false, %205 ], [ false, %178 ], [ true, %235 ], [ true, %31 ]
  %242 = phi i8 [ 1, %227 ], [ 1, %219 ], [ 1, %209 ], [ 1, %205 ], [ 1, %178 ], [ 0, %235 ], [ 0, %31 ]
  %243 = load ptr, ptr %4, align 8, !noalias !1014
  %244 = icmp eq ptr %243, null
  br i1 %244, label %248, label %245

245:                                              ; preds = %239
  %246 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %243, i64 0, i32 2
  %247 = load ptr, ptr %246, align 8, !noalias !1103
  br i1 %241, label %268, label %475

248:                                              ; preds = %239
  %249 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %250 = load ptr, ptr %249, align 8, !noalias !1108
  %251 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %250, i64 0, i32 0
  %252 = load ptr, ptr %251, align 8, !noalias !1108
  %253 = tail call i1 @llvm.type.test(ptr %252, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %253)
  %254 = getelementptr inbounds ptr, ptr %252, i64 1
  %255 = load ptr, ptr %254, align 8, !noalias !1108
  %256 = bitcast ptr %255 to ptr
  %257 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %258 = icmp eq ptr %256, %257
  br i1 %258, label %259, label %261

259:                                              ; preds = %248
  %260 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %250, i64 noundef 24), !noalias !1108, !intel_dtrans_type !1018
  br label %263

261:                                              ; preds = %248
  %262 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %250, i64 noundef 24), !noalias !1108, !intel_dtrans_type !1018
  br label %263

263:                                              ; preds = %261, %259
  %264 = phi ptr [ %260, %259 ], [ %262, %261 ]
  br label %265

265:                                              ; preds = %263
  store ptr %264, ptr %4, align 8, !noalias !1108
  %266 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %264, i64 0, i32 2
  store ptr %264, ptr %266, align 8, !noalias !1108
  %267 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %264, i64 0, i32 1
  store ptr %264, ptr %267, align 8, !noalias !1108
  br i1 %241, label %268, label %475

268:                                              ; preds = %265, %245
  %269 = phi ptr [ %243, %245 ], [ %264, %265 ]
  %270 = phi ptr [ %247, %245 ], [ %264, %265 ]
  %271 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %240, i64 0, i32 0
  br label %272

272:                                              ; preds = %472, %268
  %273 = phi ptr [ %277, %472 ], [ %269, %268 ]
  %274 = icmp eq ptr %273, %270
  br i1 %274, label %475, label %275

275:                                              ; preds = %272
  %276 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %273, i64 0, i32 1
  %277 = load ptr, ptr %276, align 8, !noalias !1113
  %278 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %277, i64 0, i32 0
  %279 = load ptr, ptr %278, align 8
  %280 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %279, i64 0, i32 3
  %281 = load ptr, ptr %280, align 8
  %282 = icmp ugt ptr %281, %1
  br i1 %282, label %472, label %283

283:                                              ; preds = %275
  %284 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %279, i64 0, i32 2
  %285 = load i16, ptr %284, align 2
  %286 = zext i16 %285 to i64
  %287 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %281, i64 %286
  %288 = icmp ugt ptr %287, %1
  br i1 %288, label %289, label %472

289:                                              ; preds = %283
  %290 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %279, i64 0, i32 3
  %291 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %279, i64 0, i32 1
  %292 = load i16, ptr %291, align 8
  %293 = getelementptr inbounds %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock", ptr %279, i64 0, i32 2
  %294 = load i16, ptr %293, align 2
  %295 = icmp eq i16 %292, %294
  br i1 %295, label %301, label %296

296:                                              ; preds = %289
  %297 = zext i16 %292 to i64
  %298 = getelementptr inbounds %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %281, i64 %297
  %299 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %298, i64 0, i32 0
  store i16 %294, ptr %299, align 4
  %300 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %298, i64 0, i32 1
  store i32 -2228259, ptr %300, align 4
  store i16 %292, ptr %293, align 2
  br label %301

301:                                              ; preds = %296, %289
  %302 = getelementptr %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached", ptr %1, i64 0, i32 0, i32 0, i32 0, i32 0
  %303 = load ptr, ptr %302, align 8
  %304 = tail call i1 @llvm.type.test(ptr %303, metadata !"?AVXStringCached@xalanc_1_10@@")
  tail call void @llvm.assume(i1 %304)
  %305 = load ptr, ptr %303, align 8
  %306 = tail call noundef ptr @"??_GXStringCached@xalanc_1_10@@UEAAPEAXI@Z"(ptr noundef nonnull align 8 dereferenceable(88) %1, i32 noundef 0) #20, !intel_dtrans_type !1048
  %307 = load i16, ptr %291, align 8
  %308 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %1, i64 0, i32 0
  store i16 %307, ptr %308, align 4
  %309 = getelementptr inbounds %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock", ptr %1, i64 0, i32 1
  store i32 -2228259, ptr %309, align 4
  %310 = load ptr, ptr %290, align 8
  %311 = ptrtoint ptr %1 to i64
  %312 = ptrtoint ptr %310 to i64
  %313 = sub i64 %311, %312
  %314 = sdiv exact i64 %313, 88
  %315 = trunc i64 %314 to i16
  store i16 %315, ptr %293, align 2
  store i16 %315, ptr %291, align 8
  %316 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %279, i64 0, i32 1
  %317 = load i16, ptr %316, align 8
  %318 = add i16 %317, -1
  store i16 %318, ptr %316, align 8
  %319 = load ptr, ptr %4, align 8, !noalias !1116
  %320 = icmp eq ptr %319, null
  br i1 %320, label %321, label %341

321:                                              ; preds = %301
  %322 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %323 = load ptr, ptr %322, align 8, !noalias !1116
  %324 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %323, i64 0, i32 0
  %325 = load ptr, ptr %324, align 8, !noalias !1116
  %326 = tail call i1 @llvm.type.test(ptr %325, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %326)
  %327 = getelementptr inbounds ptr, ptr %325, i64 1
  %328 = load ptr, ptr %327, align 8, !noalias !1116
  %329 = bitcast ptr %328 to ptr
  %330 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %331 = icmp eq ptr %329, %330
  br i1 %331, label %332, label %334

332:                                              ; preds = %321
  %333 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %323, i64 noundef 24), !noalias !1116, !intel_dtrans_type !1018
  br label %336

334:                                              ; preds = %321
  %335 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %323, i64 noundef 24), !noalias !1116, !intel_dtrans_type !1018
  br label %336

336:                                              ; preds = %334, %332
  %337 = phi ptr [ %333, %332 ], [ %335, %334 ]
  br label %338

338:                                              ; preds = %336
  store ptr %337, ptr %4, align 8, !noalias !1116
  %339 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %337, i64 0, i32 2
  store ptr %337, ptr %339, align 8, !noalias !1116
  %340 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %337, i64 0, i32 1
  store ptr %337, ptr %340, align 8, !noalias !1116
  br label %341

341:                                              ; preds = %338, %301
  %342 = phi ptr [ %337, %338 ], [ %319, %301 ]
  %343 = icmp eq ptr %273, %342
  br i1 %343, label %415, label %344

344:                                              ; preds = %341
  %345 = load ptr, ptr %271, align 8
  %346 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %240, i64 0, i32 2
  %347 = load ptr, ptr %346, align 8
  %348 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %240, i64 0, i32 1
  %349 = load ptr, ptr %348, align 8
  %350 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %349, i64 0, i32 2
  store ptr %347, ptr %350, align 8
  %351 = load ptr, ptr %346, align 8
  %352 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %351, i64 0, i32 1
  store ptr %349, ptr %352, align 8
  store ptr null, ptr %348, align 8
  %353 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 2
  %354 = load ptr, ptr %353, align 8
  store ptr %354, ptr %346, align 8
  store ptr %240, ptr %353, align 8
  %355 = load ptr, ptr %4, align 8, !noalias !1121
  %356 = icmp eq ptr %355, null
  br i1 %356, label %360, label %357

357:                                              ; preds = %344
  %358 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %355, i64 0, i32 2
  %359 = load ptr, ptr %358, align 8, !noalias !1121
  br label %382

360:                                              ; preds = %344
  %361 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %362 = load ptr, ptr %361, align 8, !noalias !1121
  %363 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %362, i64 0, i32 0
  %364 = load ptr, ptr %363, align 8, !noalias !1121
  %365 = tail call i1 @llvm.type.test(ptr %364, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %365)
  %366 = getelementptr inbounds ptr, ptr %364, i64 1
  %367 = load ptr, ptr %366, align 8, !noalias !1121
  %368 = bitcast ptr %367 to ptr
  %369 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %370 = icmp eq ptr %368, %369
  br i1 %370, label %371, label %373

371:                                              ; preds = %360
  %372 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %362, i64 noundef 24), !noalias !1121, !intel_dtrans_type !1018
  br label %375

373:                                              ; preds = %360
  %374 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %362, i64 noundef 24), !noalias !1121, !intel_dtrans_type !1018
  br label %375

375:                                              ; preds = %373, %371
  %376 = phi ptr [ %372, %371 ], [ %374, %373 ]
  br label %377

377:                                              ; preds = %375
  store ptr %376, ptr %4, align 8, !noalias !1121
  %378 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %376, i64 0, i32 2
  store ptr %376, ptr %378, align 8, !noalias !1121
  %379 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %376, i64 0, i32 1
  store ptr %376, ptr %379, align 8, !noalias !1121
  %380 = load ptr, ptr %353, align 8
  %381 = icmp eq ptr %380, null
  br i1 %381, label %387, label %382

382:                                              ; preds = %377, %357
  %383 = phi ptr [ %359, %357 ], [ %376, %377 ]
  %384 = phi ptr [ %240, %357 ], [ %380, %377 ]
  %385 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %384, i64 0, i32 2
  %386 = load ptr, ptr %385, align 8
  br label %404

387:                                              ; preds = %377
  %388 = load ptr, ptr %361, align 8
  %389 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %388, i64 0, i32 0
  %390 = load ptr, ptr %389, align 8
  %391 = tail call i1 @llvm.type.test(ptr %390, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %391)
  %392 = getelementptr inbounds ptr, ptr %390, i64 1
  %393 = load ptr, ptr %392, align 8
  %394 = bitcast ptr %393 to ptr
  %395 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %396 = icmp eq ptr %394, %395
  br i1 %396, label %397, label %399

397:                                              ; preds = %387
  %398 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %388, i64 noundef 24), !intel_dtrans_type !1018
  br label %401

399:                                              ; preds = %387
  %400 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %388, i64 noundef 24), !intel_dtrans_type !1018
  br label %401

401:                                              ; preds = %399, %397
  %402 = phi ptr [ %398, %397 ], [ %400, %399 ]
  br label %403

403:                                              ; preds = %401
  br label %404

404:                                              ; preds = %403, %382
  %405 = phi ptr [ %383, %382 ], [ %376, %403 ]
  %406 = phi ptr [ %384, %382 ], [ %402, %403 ]
  %407 = phi ptr [ %386, %382 ], [ null, %403 ]
  %408 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %406, i64 0, i32 0
  store ptr %345, ptr %408, align 8
  %409 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %406, i64 0, i32 1
  %410 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %405, i64 0, i32 1
  %411 = load ptr, ptr %410, align 8
  store ptr %411, ptr %409, align 8
  %412 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %406, i64 0, i32 2
  store ptr %405, ptr %412, align 8
  %413 = load ptr, ptr %410, align 8
  %414 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %413, i64 0, i32 2
  store ptr %406, ptr %414, align 8
  store ptr %406, ptr %410, align 8
  store ptr %407, ptr %353, align 8
  br label %415

415:                                              ; preds = %404, %341
  %416 = getelementptr inbounds %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator", ptr %0, i64 0, i32 1
  %417 = load i8, ptr %416, align 8, !range !1092
  %418 = icmp eq i8 %417, 0
  br i1 %418, label %475, label %419

419:                                              ; preds = %415
  %420 = load ptr, ptr %4, align 8, !noalias !1014
  %421 = icmp eq ptr %420, null
  br i1 %421, label %422, label %442

422:                                              ; preds = %419
  %423 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 0
  %424 = load ptr, ptr %423, align 8, !noalias !1124
  %425 = getelementptr %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager", ptr %424, i64 0, i32 0
  %426 = load ptr, ptr %425, align 8, !noalias !1124
  %427 = tail call i1 @llvm.type.test(ptr %426, metadata !"?AVMemoryManager@xercesc_2_7@@")
  tail call void @llvm.assume(i1 %427)
  %428 = getelementptr inbounds ptr, ptr %426, i64 1
  %429 = load ptr, ptr %428, align 8, !noalias !1124
  %430 = bitcast ptr %429 to ptr
  %431 = bitcast ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z" to ptr
  %432 = icmp eq ptr %430, %431
  br i1 %432, label %433, label %435

433:                                              ; preds = %422
  %434 = tail call noundef ptr @"?allocate@MemoryManagerImpl@xercesc_2_7@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %424, i64 noundef 24), !noalias !1124, !intel_dtrans_type !1018
  br label %437

435:                                              ; preds = %422
  %436 = tail call noundef ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr noundef nonnull align 8 dereferenceable(8) %424, i64 noundef 24), !noalias !1124, !intel_dtrans_type !1018
  br label %437

437:                                              ; preds = %435, %433
  %438 = phi ptr [ %434, %433 ], [ %436, %435 ]
  br label %439

439:                                              ; preds = %437
  store ptr %438, ptr %4, align 8, !noalias !1124
  %440 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %438, i64 0, i32 2
  store ptr %438, ptr %440, align 8, !noalias !1124
  %441 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %438, i64 0, i32 1
  store ptr %438, ptr %441, align 8, !noalias !1124
  br label %475

442:                                              ; preds = %419
  %443 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %420, i64 0, i32 2
  %444 = load ptr, ptr %443, align 8, !noalias !1124
  %445 = icmp eq ptr %444, %420
  br i1 %445, label %475, label %446

446:                                              ; preds = %442
  %447 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %444, i64 0, i32 0
  %448 = load ptr, ptr %447, align 8
  %449 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %448, i64 0, i32 1
  %450 = load i16, ptr %449, align 8
  %451 = icmp eq i16 %450, 0
  br i1 %451, label %452, label %475

452:                                              ; preds = %446
  %453 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %444, i64 0, i32 2
  %454 = load ptr, ptr %453, align 8, !noalias !1127
  %455 = icmp eq ptr %454, %420
  br i1 %455, label %464, label %456

456:                                              ; preds = %452
  %457 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %454, i64 0, i32 0
  %458 = load ptr, ptr %457, align 8
  %459 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %458, i64 0, i32 1
  %460 = load i16, ptr %459, align 8
  %461 = getelementptr inbounds %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase", ptr %458, i64 0, i32 2
  %462 = load i16, ptr %461, align 2
  %463 = icmp ult i16 %460, %462
  br i1 %463, label %464, label %475

464:                                              ; preds = %456, %452
  %465 = getelementptr inbounds %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList", ptr %3, i64 0, i32 2
  %466 = load ptr, ptr %465, align 8
  %467 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %444, i64 0, i32 1
  %468 = load ptr, ptr %467, align 8
  %469 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %468, i64 0, i32 2
  store ptr %454, ptr %469, align 8
  %470 = load ptr, ptr %453, align 8
  %471 = getelementptr inbounds %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node", ptr %470, i64 0, i32 1
  store ptr %468, ptr %471, align 8
  store ptr null, ptr %467, align 8
  store ptr %466, ptr %453, align 8
  store ptr %444, ptr %465, align 8
  br label %475

472:                                              ; preds = %283, %275
  %473 = load ptr, ptr %271, align 8
  %474 = icmp eq ptr %279, %473
  br i1 %474, label %475, label %272, !llvm.loop !1130

475:                                              ; preds = %472, %464, %456, %446, %442, %439, %415, %272, %265, %245, %202
  %476 = phi i8 [ 1, %415 ], [ 1, %439 ], [ 1, %442 ], [ 1, %446 ], [ 1, %456 ], [ 1, %464 ], [ %242, %265 ], [ 1, %202 ], [ %242, %245 ], [ %242, %272 ], [ %242, %472 ]
  %477 = and i8 %476, 1
  %478 = icmp ne i8 %477, 0
  br label %479

479:                                              ; preds = %475, %27, %24
  %480 = phi i1 [ %478, %475 ], [ false, %27 ], [ false, %24 ]
  ret i1 %480
}

; Function Attrs: mustprogress uwtable
define hidden void @"?reset@XStringCachedAllocator@xalanc_1_10@@QEAAXXZ"(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %0) #16 align 2 !type !1131 !intel.dtrans.func.type !1132 {
  %2 = getelementptr inbounds %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator", ptr %0, i64 0, i32 0
  tail call void @"?reset@?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@UEAAXXZ"(ptr noundef nonnull align 8 dereferenceable(40) %2)
  ret void
}

; Function Attrs: nofree noreturn uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @"?allocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAPEAX_K@Z"(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %0, i64 noundef %1) unnamed_addr #17 comdat align 2 !intel.dtrans.func.type !1133 !_Intel.Devirt.Target !994 {
  %3 = alloca %"class..?AVbad_alloc@std@@.std::bad_alloc", align 8
  %4 = getelementptr inbounds %"class..?AVexception@std@@.std::exception", ptr %3, i64 0, i32 1
  %5 = getelementptr inbounds i8, ptr %4, i64 8
  store i64 0, ptr %5, align 8
  %6 = getelementptr inbounds %"struct..?AU__std_exception_data@@.__std_exception_data", ptr %4, i64 0, i32 0
  store ptr @"??_C@_0P@GHFPNOJB@bad?5allocation?$AA@", ptr %6, align 8
  %7 = getelementptr %"class..?AVbad_alloc@std@@.std::bad_alloc", ptr %3, i64 0, i32 0, i32 0
  store ptr @"??_7bad_alloc@std@@6B@", ptr %7, align 8
  call void @_CxxThrowException(ptr nonnull %3, ptr nonnull @"_TI2?AVbad_alloc@std@@") #19
  unreachable
}

; Function Attrs: nofree noreturn uwtable
define hidden void @"?deallocate@XalanDummyMemoryManager@xalanc_1_10@@UEAAXPEAX@Z"(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %1) unnamed_addr #17 comdat align 2 !intel.dtrans.func.type !1135 !_Intel.Devirt.Target !994 {
  %3 = alloca %"class..?AVbad_alloc@std@@.std::bad_alloc", align 8
  %4 = getelementptr inbounds %"class..?AVexception@std@@.std::exception", ptr %3, i64 0, i32 1
  %5 = getelementptr inbounds i8, ptr %4, i64 8
  store i64 0, ptr %5, align 8
  %6 = getelementptr inbounds %"struct..?AU__std_exception_data@@.__std_exception_data", ptr %4, i64 0, i32 0
  store ptr @"??_C@_0P@GHFPNOJB@bad?5allocation?$AA@", ptr %6, align 8
  %7 = getelementptr %"class..?AVbad_alloc@std@@.std::bad_alloc", ptr %3, i64 0, i32 0, i32 0
  store ptr @"??_7bad_alloc@std@@6B@", ptr %7, align 8
  call void @_CxxThrowException(ptr nonnull %3, ptr nonnull @"_TI2?AVbad_alloc@std@@") #19
  unreachable
}

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nofree }
attributes #3 = { nofree noreturn }
attributes #4 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { mustprogress nofree norecurse nosync willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #11 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #12 = { argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { nofree noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #18 = { allocsize(0) }
attributes #19 = { noreturn }
attributes #20 = { nounwind }
attributes #21 = { noreturn nounwind }
attributes #22 = { builtin nounwind }

!llvm.linker.options = !{!2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !8, !8, !2, !3, !4, !5, !6, !7, !9, !8, !8, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !8, !8, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !8, !8, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !8, !8, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !8, !8, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !8, !8, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !2, !3, !4, !5, !6, !7, !2, !3, !4, !5, !6, !7, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !10, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !10, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13, !9, !2, !3, !4, !5, !6, !7, !11, !12, !13}
!intel.dtrans.types = !{!14, !18, !19, !21, !25, !28, !29, !30, !31, !32, !33, !36, !37, !41, !45, !47, !49, !51, !54, !55, !57, !58, !61, !62, !64, !65, !68, !70, !72, !75, !77, !81, !89, !94, !96, !98, !100, !106, !107, !108, !110, !112, !113, !115, !117, !119, !120, !122, !124, !126, !127, !128, !130, !131, !132, !133, !134, !137, !138, !139, !143, !145, !147, !148, !150, !151, !153, !155, !156, !157, !160, !162, !164, !166, !168, !170, !172, !174, !176, !178, !179, !180, !182, !198, !200, !202, !204, !206, !208, !210, !212, !217, !218, !219, !221, !223, !224, !229, !230, !231, !233, !235, !237, !240, !242, !244, !245, !247, !249, !251, !252, !254, !256, !260, !262, !264, !266, !268, !270, !272, !274, !276, !278, !283, !285, !286, !288, !289, !290, !291, !293, !295, !297, !299, !300, !302, !303, !304, !306, !308, !309, !314, !316, !318, !320, !322, !324, !325, !327, !328, !329, !330, !332, !334, !337, !339, !341, !343, !344, !346, !348, !349, !350, !352, !353, !359, !375, !377, !379, !381, !383, !385, !387, !389, !391, !393, !395, !397, !399, !401, !403, !405, !407, !409, !411, !413, !415, !417, !419, !421, !423, !425, !427, !429, !431, !433, !435, !437, !440, !442, !447, !448, !450, !452, !455, !457, !459, !461, !463, !465, !467, !469, !471, !473, !475, !477, !479, !481, !483, !485, !487, !489, !491, !493, !495, !497, !499, !501, !503, !505, !507, !509, !511, !513, !516, !519, !522, !525, !528, !531, !534, !537, !540, !543, !544, !546, !547, !549, !550, !551, !555, !556, !558, !559, !560, !561, !563, !564, !565, !566, !568, !570, !572, !573, !576, !579, !580, !582, !584, !586, !588, !590, !592, !593, !594, !597, !600, !602, !604, !606, !609, !612, !614, !616, !618, !620, !622, !624, !625, !627, !629, !630, !631, !633, !635, !637, !638, !639, !642, !644, !646, !648, !650, !653, !654, !657, !659, !661, !662, !663, !665, !667, !668, !669, !671, !673, !690, !691, !692, !694, !696, !698, !700, !702, !705, !707, !709, !711, !713, !716, !718, !720, !722, !723, !752, !756, !758, !763, !764, !765, !766, !768, !769, !770, !772, !774, !776, !778, !779, !782, !784, !785, !787, !789, !792, !794, !796, !798, !800, !801, !803, !805, !806, !811, !812, !813, !815, !817, !819, !821, !823, !825, !826, !828, !830, !831, !833, !835, !837, !839, !841, !843, !845, !847, !849, !851, !853, !855, !857, !859, !861, !862, !864, !866, !870, !871, !872, !873, !878, !879, !880, !881, !883, !886, !891, !892, !893, !894, !896, !898, !900, !902, !904, !907, !908, !909, !911, !913, !915, !917, !919, !921, !922, !923, !925, !927, !929, !931, !932, !933, !935, !937, !939, !941, !943, !946, !947, !950, !952, !954, !957, !958, !959, !960, !962, !963, !965, !966, !967, !969, !971, !973, !975, !977, !978, !979}
!llvm.ident = !{!980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980, !980}
!llvm.module.flags = !{!981, !982, !983, !984, !985, !986, !987}

!0 = !{i64 8, !"?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@"}
!1 = !{i64 8, !"?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@"}
!2 = !{!"/DEFAULTLIB:libcmt.lib"}
!3 = !{!"/DEFAULTLIB:libircmt.lib"}
!4 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!5 = !{!"/DEFAULTLIB:libdecimal.lib"}
!6 = !{!"/DEFAULTLIB:libmmt.lib"}
!7 = !{!"/DEFAULTLIB:oldnames.lib"}
!8 = !{!"/DEFAULTLIB:uuid.lib"}
!9 = !{!"/DEFAULTLIB:libcpmt.lib"}
!10 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!11 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!12 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!13 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!14 = !{!"S", %"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !15}
!15 = !{!16, i32 2}
!16 = !{!"F", i1 true, i32 0, !17}
!17 = !{i32 0, i32 0}
!18 = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, !17, !17, !17, !17}
!19 = !{!"S", %"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" zeroinitializer, i32 2, !20, !17}
!20 = !{%"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!21 = !{!"S", %"class..?AV?$XalanVector@GU?$MemoryManagedConstructionTraits@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !24}
!22 = !{%"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!23 = !{i64 0, i32 0}
!24 = !{i16 0, i32 1}
!25 = !{!"S", %"class..?AVAVT@xalanc_1_10@@.xalanc_1_10::AVT" zeroinitializer, i32 6, !15, !26, !23, !24, !17, !27}
!26 = !{%"class..?AVAVTPart@xalanc_1_10@@.xalanc_1_10::AVTPart" zeroinitializer, i32 2}
!27 = !{%"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!28 = !{!"S", %"class..?AVAVTPart@xalanc_1_10@@.xalanc_1_10::AVTPart" zeroinitializer, i32 1, !15}
!29 = !{!"S", %"class..?AVXPathConstructionContext@xalanc_1_10@@.xalanc_1_10::XPathConstructionContext" zeroinitializer, i32 2, !15, !22}
!30 = !{!"S", %"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" zeroinitializer, i32 1, !15}
!31 = !{!"S", %"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver" zeroinitializer, i32 1, !15}
!32 = !{!"S", %"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" zeroinitializer, i32 1, !15}
!33 = !{!"S", %"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 2, !34, !35}
!34 = !{%"class..?AVExecutionContext@xalanc_1_10@@.xalanc_1_10::ExecutionContext" zeroinitializer, i32 0}
!35 = !{%"class..?AVXObjectFactory@xalanc_1_10@@.xalanc_1_10::XObjectFactory" zeroinitializer, i32 1}
!36 = !{!"S", %"class..?AVExecutionContext@xalanc_1_10@@.xalanc_1_10::ExecutionContext" zeroinitializer, i32 2, !15, !22}
!37 = !{!"S", %"class..?AVXPath@xalanc_1_10@@.xalanc_1_10::XPath" zeroinitializer, i32 3, !38, !39, !40}
!38 = !{%"class..?AVXPathExpression@xalanc_1_10@@.xalanc_1_10::XPathExpression" zeroinitializer, i32 0}
!39 = !{%"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" zeroinitializer, i32 1}
!40 = !{i8 0, i32 0}
!41 = !{!"S", %"class..?AVXPathExpression@xalanc_1_10@@.xalanc_1_10::XPathExpression" zeroinitializer, i32 6, !42, !17, !43, !17, !27, !44}
!42 = !{%"class..?AV?$XalanVector@HU?$MemoryManagedConstructionTraits@H@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!43 = !{%"class..?AV?$XalanVector@VXToken@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXToken@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!44 = !{%"class..?AV?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!45 = !{!"S", %"class..?AV?$XalanVector@HU?$MemoryManagedConstructionTraits@H@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !46}
!46 = !{i32 0, i32 1}
!47 = !{!"S", %"class..?AV?$XalanVector@VXToken@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXToken@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !48}
!48 = !{%"class..?AVXToken@xalanc_1_10@@.xalanc_1_10::XToken" zeroinitializer, i32 1}
!49 = !{!"S", %"class..?AV?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !50}
!50 = !{double 0.000000e+00, i32 1}
!51 = !{!"S", %"class..?AVXToken@xalanc_1_10@@.xalanc_1_10::XToken" zeroinitializer, i32 4, !52, !27, !53, !40}
!52 = !{%"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject" zeroinitializer, i32 0}
!53 = !{double 0.000000e+00, i32 0}
!54 = !{!"S", %"class..?AVXObjectFactory@xalanc_1_10@@.xalanc_1_10::XObjectFactory" zeroinitializer, i32 2, !15, !22}
!55 = !{!"S", %"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject" zeroinitializer, i32 3, !56, !17, !35}
!56 = !{%"class..?AVXalanReferenceCountedObject@xalanc_1_10@@.xalanc_1_10::XalanReferenceCountedObject" zeroinitializer, i32 0}
!57 = !{!"S", %"class..?AVXalanReferenceCountedObject@xalanc_1_10@@.xalanc_1_10::XalanReferenceCountedObject" zeroinitializer, i32 2, !15, !17}
!58 = !{!"S", %"class..?AVAttributeListImpl@xalanc_1_10@@.xalanc_1_10::AttributeListImpl" zeroinitializer, i32 3, !59, !60, !60}
!59 = !{%"class..?AVAttributeList@xercesc_2_7@@.xercesc_2_7::AttributeList" zeroinitializer, i32 0}
!60 = !{%"class..?AV?$XalanVector@PEAVAttributeVectorEntry@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVAttributeVectorEntry@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!61 = !{!"S", %"class..?AVAttributeList@xercesc_2_7@@.xercesc_2_7::AttributeList" zeroinitializer, i32 1, !15}
!62 = !{!"S", %"class..?AV?$XalanVector@PEAVAttributeVectorEntry@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVAttributeVectorEntry@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !63}
!63 = !{%"class..?AVAttributeVectorEntry@xalanc_1_10@@.xalanc_1_10::AttributeVectorEntry" zeroinitializer, i32 2}
!64 = !{!"S", %"class..?AVAttributeVectorEntry@xalanc_1_10@@.xalanc_1_10::AttributeVectorEntry" zeroinitializer, i32 4, !15, !20, !20, !20}
!65 = !{!"S", %"class..?AVCountersTable@xalanc_1_10@@.xalanc_1_10::CountersTable" zeroinitializer, i32 2, !66, !67}
!66 = !{%"class..?AV?$XalanVector@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!67 = !{%"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!68 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !69}
!69 = !{%"class..?AV?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!70 = !{!"S", %"class..?AV?$XalanVector@UCounter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UCounter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !71}
!71 = !{%"struct..?AUCounter@xalanc_1_10@@.xalanc_1_10::Counter" zeroinitializer, i32 1}
!72 = !{!"S", %"struct..?AUCounter@xalanc_1_10@@.xalanc_1_10::Counter" zeroinitializer, i32 4, !17, !67, !73, !74}
!73 = !{%"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" zeroinitializer, i32 1}
!74 = !{%"class..?AVElemNumber@xalanc_1_10@@.xalanc_1_10::ElemNumber" zeroinitializer, i32 1}
!75 = !{!"S", %"class..?AV?$XalanVector@PEAVXalanNode@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !76}
!76 = !{%"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" zeroinitializer, i32 2}
!77 = !{!"S", %"class..?AVElemNumber@xalanc_1_10@@.xalanc_1_10::ElemNumber" zeroinitializer, i32 11, !78, !79, !79, !79, !17, !80, !80, !80, !80, !80, !17}
!78 = !{%"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 0}
!79 = !{%"class..?AVXPath@xalanc_1_10@@.xalanc_1_10::XPath" zeroinitializer, i32 1}
!80 = !{%"class..?AVAVT@xalanc_1_10@@.xalanc_1_10::AVT" zeroinitializer, i32 1}
!81 = !{!"S", %"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 10, !82, !83, !84, !17, !85, !85, !85, !86, !87, !88}
!82 = !{%"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver" zeroinitializer, i32 0}
!83 = !{%"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet" zeroinitializer, i32 1}
!84 = !{%"class..?AVNamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler" zeroinitializer, i32 0}
!85 = !{%"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 1}
!86 = !{%"union..?AT<unnamed-type-$S1>@ElemTemplateElement@xalanc_1_10@@.anon" zeroinitializer, i32 0}
!87 = !{%"class..?AVLocatorProxy@ElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement::LocatorProxy" zeroinitializer, i32 0}
!88 = !{i16 0, i32 0}
!89 = !{!"S", %"class..?AVNamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler" zeroinitializer, i32 4, !90, !91, !92, !93}
!90 = !{%"class..?AV?$XalanVector@VNamespace@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespace@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!91 = !{%"class..?AV?$XalanVector@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!92 = !{%"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!93 = !{%"class..?AV?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!94 = !{!"S", %"class..?AV?$XalanVector@VNamespace@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespace@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !95}
!95 = !{%"class..?AVNamespace@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 1}
!96 = !{!"S", %"class..?AV?$XalanVector@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNamespaceExtended@NamespacesHandler@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !97}
!97 = !{%"class..?AVNamespaceExtended@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::NamespaceExtended" zeroinitializer, i32 1}
!98 = !{!"S", %"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !99}
!99 = !{%"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" zeroinitializer, i32 2}
!100 = !{!"S", %"class..?AV?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !101, !102, !22, !103, !23, !23, !104, !104, !105, !23, !23}
!101 = !{%"struct..?AUDOMStringPointerHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringPointerHashFunction" zeroinitializer, i32 0}
!102 = !{%"struct..?AU?$pointer_equal@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" zeroinitializer, i32 0}
!103 = !{float 0.000000e+00, i32 0}
!104 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!105 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!106 = !{!"S", %"struct..?AUDOMStringPointerHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringPointerHashFunction" zeroinitializer, i32 1, !40}
!107 = !{!"S", %"struct..?AU?$pointer_equal@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" zeroinitializer, i32 1, !40}
!108 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !109, !109}
!109 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" zeroinitializer, i32 1}
!110 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !111}
!111 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!112 = !{!"S", %"union..?AT<unnamed-type-$S1>@ElemTemplateElement@xalanc_1_10@@.anon" zeroinitializer, i32 1, !85}
!113 = !{!"S", %"class..?AVLocatorProxy@ElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement::LocatorProxy" zeroinitializer, i32 4, !114, !17, !17, !27}
!114 = !{%"class..?AVXalanLocator@xalanc_1_10@@.xalanc_1_10::XalanLocator" zeroinitializer, i32 0}
!115 = !{!"S", %"class..?AVXalanLocator@xalanc_1_10@@.xalanc_1_10::XalanLocator" zeroinitializer, i32 1, !116}
!116 = !{%"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" zeroinitializer, i32 0}
!117 = !{!"S", %"class..?AVStylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 1, !118}
!118 = !{%"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 0}
!119 = !{!"S", %"class..?AVNamespace@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 2, !27, !27}
!120 = !{!"S", %"class..?AVNamespaceExtended@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::NamespaceExtended" zeroinitializer, i32 2, !121, !27}
!121 = !{%"class..?AVNamespace@NamespacesHandler@xalanc_1_10@@.xalanc_1_10::NamespacesHandler::Namespace" zeroinitializer, i32 0}
!122 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !123}
!123 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!124 = !{!"S", %"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1, !125}
!125 = !{%"class..?AVXalanNode@xalanc_1_10@@.xalanc_1_10::XalanNode" zeroinitializer, i32 0}
!126 = !{!"S", %"class..?AVXalanDocument@xalanc_1_10@@.xalanc_1_10::XalanDocument" zeroinitializer, i32 1, !125}
!127 = !{!"S", %"class..?AVXalanElement@xalanc_1_10@@.xalanc_1_10::XalanElement" zeroinitializer, i32 1, !125}
!128 = !{!"S", %"class..?AVXalanText@xalanc_1_10@@.xalanc_1_10::XalanText" zeroinitializer, i32 1, !129}
!129 = !{%"class..?AVXalanCharacterData@xalanc_1_10@@.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 0}
!130 = !{!"S", %"class..?AVXalanCharacterData@xalanc_1_10@@.xalanc_1_10::XalanCharacterData" zeroinitializer, i32 1, !125}
!131 = !{!"S", %"class..?AVXalanAttr@xalanc_1_10@@.xalanc_1_10::XalanAttr" zeroinitializer, i32 1, !125}
!132 = !{!"S", %"class..?AVXalanComment@xalanc_1_10@@.xalanc_1_10::XalanComment" zeroinitializer, i32 1, !129}
!133 = !{!"S", %"class..?AVXalanProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanProcessingInstruction" zeroinitializer, i32 1, !125}
!134 = !{!"S", %"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener" zeroinitializer, i32 4, !135, !136, !17, !17}
!135 = !{%"class..?AVDocumentHandler@xercesc_2_7@@.xercesc_2_7::DocumentHandler" zeroinitializer, i32 0}
!136 = !{%"class..?AVPrefixResolver@xalanc_1_10@@.xalanc_1_10::PrefixResolver" zeroinitializer, i32 1}
!137 = !{!"S", %"class..?AVDocumentHandler@xercesc_2_7@@.xercesc_2_7::DocumentHandler" zeroinitializer, i32 1, !15}
!138 = !{!"S", %"class..?AVXalanNamedNodeMap@xalanc_1_10@@.xalanc_1_10::XalanNamedNodeMap" zeroinitializer, i32 1, !15}
!139 = !{!"S", %"class..?AVXalanOutputStream@xalanc_1_10@@.xalanc_1_10::XalanOutputStream" zeroinitializer, i32 9, !15, !17, !140, !17, !20, !141, !40, !40, !142}
!140 = !{%"class..?AVXalanOutputTranscoder@xalanc_1_10@@.xalanc_1_10::XalanOutputTranscoder" zeroinitializer, i32 1}
!141 = !{%"class..?AVXalanDOMString@xalanc_1_10@@.xalanc_1_10::XalanDOMString" zeroinitializer, i32 0}
!142 = !{%"class..?AV?$XalanVector@DU?$MemoryManagedConstructionTraits@D@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!143 = !{!"S", %"class..?AV?$XalanVector@DU?$MemoryManagedConstructionTraits@D@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !144}
!144 = !{i8 0, i32 1}
!145 = !{!"S", %"class..?AVexception@std@@.std::exception" zeroinitializer, i32 2, !15, !146}
!146 = !{%"struct..?AU__std_exception_data@@.__std_exception_data" zeroinitializer, i32 0}
!147 = !{!"S", %"struct..?AU__std_exception_data@@.__std_exception_data" zeroinitializer, i32 2, !144, !40}
!148 = !{!"S", %"class..?AVbad_alloc@std@@.std::bad_alloc" zeroinitializer, i32 1, !149}
!149 = !{%"class..?AVexception@std@@.std::exception" zeroinitializer, i32 0}
!150 = !{!"S", %"class..?AVXalanOutputTranscoder@xalanc_1_10@@.xalanc_1_10::XalanOutputTranscoder" zeroinitializer, i32 2, !15, !22}
!151 = !{!"S", %"class..?AVDOMStringPrintWriter@xalanc_1_10@@.xalanc_1_10::DOMStringPrintWriter" zeroinitializer, i32 2, !152, !27}
!152 = !{%"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter" zeroinitializer, i32 0}
!153 = !{!"S", %"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter" zeroinitializer, i32 3, !154, !40, !22}
!154 = !{%"class..?AVWriter@xalanc_1_10@@.xalanc_1_10::Writer" zeroinitializer, i32 0}
!155 = !{!"S", %"class..?AVWriter@xalanc_1_10@@.xalanc_1_10::Writer" zeroinitializer, i32 1, !15}
!156 = !{!"S", %"class..?AVDOMSupport@xalanc_1_10@@.xalanc_1_10::DOMSupport" zeroinitializer, i32 1, !15}
!157 = !{!"S", %"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool" zeroinitializer, i32 4, !15, !158, !23, !159}
!158 = !{%"class..?AVXalanDOMStringAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringAllocator" zeroinitializer, i32 0}
!159 = !{%"class..?AVXalanDOMStringHashTable@xalanc_1_10@@.xalanc_1_10::XalanDOMStringHashTable" zeroinitializer, i32 0}
!160 = !{!"S", %"class..?AVXalanDOMStringAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringAllocator" zeroinitializer, i32 1, !161}
!161 = !{%"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!162 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !163}
!163 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!164 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !165, !165}
!165 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 1}
!166 = !{!"S", %"class..?AVXalanDOMStringHashTable@xalanc_1_10@@.xalanc_1_10::XalanDOMStringHashTable" zeroinitializer, i32 5, !23, !23, !167, !23, !17}
!167 = !{%"class..?AV?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray" zeroinitializer, i32 0}
!168 = !{!"S", %"class..?AV?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray" zeroinitializer, i32 1, !169}
!169 = !{%"class..?AVMemMgrAutoPtrArrayData@?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" zeroinitializer, i32 0}
!170 = !{!"S", %"class..?AVMemMgrAutoPtrArrayData@?$XalanMemMgrAutoPtrArray@V?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtrArray<xalanc_1_10::XalanVector<const xalanc_1_10::XalanDOMString *>>::MemMgrAutoPtrArrayData" zeroinitializer, i32 3, !22, !171, !23}
!171 = !{%"class..?AV?$XalanVector@PEBVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!172 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 3, !173, !165, !165}
!173 = !{%"class..?AV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!174 = !{!"S", %"class..?AV?$ArenaBlock@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !175}
!175 = !{%"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!176 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !177, !23, !23, !27}
!177 = !{%"class..?AV?$XalanAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!178 = !{!"S", %"class..?AV?$XalanAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!179 = !{!"S", %"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName" zeroinitializer, i32 1, !15}
!180 = !{!"S", %"class..?AVXalanQNameByValue@xalanc_1_10@@.xalanc_1_10::XalanQNameByValue" zeroinitializer, i32 3, !181, !141, !141}
!181 = !{%"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName" zeroinitializer, i32 0}
!182 = !{!"S", %"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet" zeroinitializer, i32 31, !82, !183, !141, !184, !185, !141, !186, !23, !187, !188, !40, !189, !190, !191, !192, !193, !53, !194, !195, !196, !194, !195, !196, !196, !196, !196, !196, !196, !23, !197, !84}
!183 = !{%"class..?AVStylesheetRoot@xalanc_1_10@@.xalanc_1_10::StylesheetRoot" zeroinitializer, i32 1}
!184 = !{%"class..?AV?$XalanVector@VKeyDeclaration@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VKeyDeclaration@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!185 = !{%"class..?AV?$XalanVector@VXalanSpaceNodeTester@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanSpaceNodeTester@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!186 = !{%"class..?AV?$XalanVector@PEAVStylesheet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVStylesheet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!187 = !{%"class..?AV?$XalanDeque@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!188 = !{%"class..?AV?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!189 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!190 = !{%"class..?AVElemTemplate@xalanc_1_10@@.xalanc_1_10::ElemTemplate" zeroinitializer, i32 1}
!191 = !{%"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!192 = !{%"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!193 = !{%"class..?AV?$XalanVector@PEAVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!194 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!195 = !{%"struct..?AU?$XalanMapIterator@U?$XalanMapConstIteratorTraits@U?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanMapIterator" zeroinitializer, i32 0}
!196 = !{%"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!197 = !{%"class..?AV?$XalanVector@PEAVElemDecimalFormat@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemDecimalFormat@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!198 = !{!"S", %"class..?AV?$XalanVector@VKeyDeclaration@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VKeyDeclaration@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !199}
!199 = !{%"class..?AVKeyDeclaration@xalanc_1_10@@.xalanc_1_10::KeyDeclaration" zeroinitializer, i32 1}
!200 = !{!"S", %"class..?AV?$XalanVector@VXalanSpaceNodeTester@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanSpaceNodeTester@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !201}
!201 = !{%"class..?AVXalanSpaceNodeTester@xalanc_1_10@@.xalanc_1_10::XalanSpaceNodeTester" zeroinitializer, i32 1}
!202 = !{!"S", %"class..?AV?$XalanVector@PEAVStylesheet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVStylesheet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !203}
!203 = !{%"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet" zeroinitializer, i32 2}
!204 = !{!"S", %"class..?AV?$XalanDeque@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !22, !23, !205, !205}
!205 = !{%"class..?AV?$XalanVector@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!206 = !{!"S", %"class..?AV?$XalanVector@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !207}
!207 = !{%"class..?AV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!208 = !{!"S", %"class..?AV?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !22, !23, !209, !209}
!209 = !{%"class..?AV?$XalanVector@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!210 = !{!"S", %"class..?AV?$XalanVector@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !211}
!211 = !{%"class..?AV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!212 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !215, !215, !216, !23, !23}
!213 = !{%"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction" zeroinitializer, i32 0}
!214 = !{%"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 0}
!215 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!216 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!217 = !{!"S", %"struct..?AUDOMStringHashFunction@xalanc_1_10@@.xalanc_1_10::DOMStringHashFunction" zeroinitializer, i32 1, !40}
!218 = !{!"S", %"struct..?AU?$equal_to@VXalanDOMString@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 1, !40}
!219 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !220, !220}
!220 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" zeroinitializer, i32 1}
!221 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !222}
!222 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!223 = !{!"S", %"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !27}
!224 = !{!"S", %"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !225, !226, !22, !103, !23, !23, !227, !227, !228, !23, !23}
!225 = !{%"struct..?AU?$XalanHashMemberReference@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberReference" zeroinitializer, i32 0}
!226 = !{%"struct..?AU?$equal_to@VXalanQName@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 0}
!227 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!228 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!229 = !{!"S", %"struct..?AU?$XalanHashMemberReference@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberReference" zeroinitializer, i32 1, !40}
!230 = !{!"S", %"struct..?AU?$equal_to@VXalanQName@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 1, !40}
!231 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !232, !232}
!232 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" zeroinitializer, i32 1}
!233 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !234}
!234 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!235 = !{!"S", %"class..?AV?$XalanVector@PEAVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !236}
!236 = !{%"class..?AVElemVariable@xalanc_1_10@@.xalanc_1_10::ElemVariable" zeroinitializer, i32 2}
!237 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !238, !238, !239, !23, !23}
!238 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!239 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!240 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !241, !241}
!241 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" zeroinitializer, i32 1}
!242 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !243}
!243 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!244 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !241}
!245 = !{!"S", %"struct..?AU?$XalanMapIterator@U?$XalanMapConstIteratorTraits@U?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanMapIterator" zeroinitializer, i32 1, !246}
!246 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 0}
!247 = !{!"S", %"class..?AV?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !248}
!248 = !{%"class..?AVXalanMatchPatternData@xalanc_1_10@@.xalanc_1_10::XalanMatchPatternData" zeroinitializer, i32 2}
!249 = !{!"S", %"class..?AV?$XalanVector@PEAVElemDecimalFormat@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemDecimalFormat@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !250}
!250 = !{%"class..?AVElemDecimalFormat@xalanc_1_10@@.xalanc_1_10::ElemDecimalFormat" zeroinitializer, i32 2}
!251 = !{!"S", %"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 1, !15}
!252 = !{!"S", %"class..?AVMutableNodeRefList@xalanc_1_10@@.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 2, !253, !17}
!253 = !{%"class..?AVNodeRefList@xalanc_1_10@@.xalanc_1_10::NodeRefList" zeroinitializer, i32 0}
!254 = !{!"S", %"class..?AVNodeRefList@xalanc_1_10@@.xalanc_1_10::NodeRefList" zeroinitializer, i32 2, !255, !67}
!255 = !{%"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 0}
!256 = !{!"S", %"class..?AVStylesheetRoot@xalanc_1_10@@.xalanc_1_10::StylesheetRoot" zeroinitializer, i32 23, !257, !141, !17, !141, !141, !141, !141, !40, !141, !141, !17, !258, !40, !191, !85, !85, !85, !40, !40, !17, !40, !17, !259}
!257 = !{%"class..?AVStylesheet@xalanc_1_10@@.xalanc_1_10::Stylesheet" zeroinitializer, i32 0}
!258 = !{%"class..?AV?$XalanVector@PEBVXalanQName@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!259 = !{%"class..?AV?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!260 = !{!"S", %"class..?AVKeyDeclaration@xalanc_1_10@@.xalanc_1_10::KeyDeclaration" zeroinitializer, i32 6, !261, !79, !79, !27, !17, !17}
!261 = !{%"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName" zeroinitializer, i32 1}
!262 = !{!"S", %"class..?AVXalanSpaceNodeTester@xalanc_1_10@@.xalanc_1_10::XalanSpaceNodeTester" zeroinitializer, i32 3, !263, !17, !17}
!263 = !{%"class..?AVNodeTester@XPath@xalanc_1_10@@.xalanc_1_10::XPath::NodeTester" zeroinitializer, i32 0}
!264 = !{!"S", %"class..?AV?$XalanVector@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !265}
!265 = !{%"class..?AV?$XalanDeque@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!266 = !{!"S", %"class..?AV?$XalanVector@VNameSpace@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VNameSpace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !267}
!267 = !{%"class..?AVNameSpace@xalanc_1_10@@.xalanc_1_10::NameSpace" zeroinitializer, i32 1}
!268 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry>::Node" zeroinitializer, i32 3, !269, !220, !220}
!269 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" zeroinitializer, i32 0}
!270 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !271}
!271 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!272 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !273}
!273 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!274 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !275}
!275 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!276 = !{!"S", %"class..?AV?$XalanVector@PEBVXalanQName@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !277}
!277 = !{%"class..?AVXalanQName@xalanc_1_10@@.xalanc_1_10::XalanQName" zeroinitializer, i32 2}
!278 = !{!"S", %"class..?AV?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !279, !280, !22, !103, !23, !23, !281, !281, !282, !23, !23}
!279 = !{%"struct..?AU?$XalanHashMemberPointer@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberPointer" zeroinitializer, i32 0}
!280 = !{%"struct..?AU?$pointer_equal@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" zeroinitializer, i32 0}
!281 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!282 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!283 = !{!"S", %"class..?AVNodeTester@XPath@xalanc_1_10@@.xalanc_1_10::XPath::NodeTester" zeroinitializer, i32 5, !284, !27, !27, !144, !144}
!284 = !{%"class..?AVXPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext" zeroinitializer, i32 1}
!285 = !{!"S", %"class..?AVNameSpace@xalanc_1_10@@.xalanc_1_10::NameSpace" zeroinitializer, i32 2, !141, !141}
!286 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::ExtensionNSHandler *>::Entry" zeroinitializer, i32 2, !287, !40}
!287 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@@std@@.std::pair" zeroinitializer, i32 1}
!288 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !220}
!289 = !{!"S", %"struct..?AU?$XalanHashMemberPointer@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHashMemberPointer" zeroinitializer, i32 1, !40}
!290 = !{!"S", %"struct..?AU?$pointer_equal@VXalanQName@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::pointer_equal" zeroinitializer, i32 1, !40}
!291 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !292, !292}
!292 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" zeroinitializer, i32 1}
!293 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !294}
!294 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!295 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@PEAVExtensionNSHandler@2@@std@@.std::pair" zeroinitializer, i32 2, !141, !296}
!296 = !{%"class..?AVExtensionNSHandler@xalanc_1_10@@.xalanc_1_10::ExtensionNSHandler" zeroinitializer, i32 1}
!297 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !298}
!298 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!299 = !{!"S", %"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 2, !284, !27}
!300 = !{!"S", %"class..?AVElemAttributeSet@xalanc_1_10@@.xalanc_1_10::ElemAttributeSet" zeroinitializer, i32 2, !301, !261}
!301 = !{%"class..?AVElemUse@xalanc_1_10@@.xalanc_1_10::ElemUse" zeroinitializer, i32 0}
!302 = !{!"S", %"class..?AVElemUse@xalanc_1_10@@.xalanc_1_10::ElemUse" zeroinitializer, i32 3, !78, !277, !23}
!303 = !{!"S", %"class..?AVElemTemplate@xalanc_1_10@@.xalanc_1_10::ElemTemplate" zeroinitializer, i32 5, !78, !79, !261, !261, !53}
!304 = !{!"S", %"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr" zeroinitializer, i32 1, !305}
!305 = !{%"class..?AVXObject@xalanc_1_10@@.xalanc_1_10::XObject" zeroinitializer, i32 1}
!306 = !{!"S", %"class..?AVElemDecimalFormat@xalanc_1_10@@.xalanc_1_10::ElemDecimalFormat" zeroinitializer, i32 6, !78, !79, !79, !79, !261, !307}
!307 = !{%"class..?AVXalanDecimalFormatSymbols@xalanc_1_10@@.xalanc_1_10::XalanDecimalFormatSymbols" zeroinitializer, i32 0}
!308 = !{!"S", %"class..?AVXalanDecimalFormatSymbols@xalanc_1_10@@.xalanc_1_10::XalanDecimalFormatSymbols" zeroinitializer, i32 13, !141, !88, !88, !88, !141, !141, !88, !88, !141, !88, !88, !88, !88}
!309 = !{!"S", %"class..?AVNodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter" zeroinitializer, i32 4, !310, !311, !312, !313}
!310 = !{%"class..?AV?$XalanVector@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!311 = !{%"class..?AV?$XalanVector@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!312 = !{%"class..?AV?$XalanVector@VNodeSortKey@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodeSortKey@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!313 = !{%"class..?AV?$XalanVector@UVectorEntry@NodeSorter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UVectorEntry@NodeSorter@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!314 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !315}
!315 = !{%"class..?AV?$XalanVector@NU?$MemoryManagedConstructionTraits@N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!316 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !317}
!317 = !{%"class..?AV?$XalanVector@VXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!318 = !{!"S", %"class..?AV?$XalanVector@VNodeSortKey@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodeSortKey@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !319}
!319 = !{%"class..?AVNodeSortKey@xalanc_1_10@@.xalanc_1_10::NodeSortKey" zeroinitializer, i32 1}
!320 = !{!"S", %"class..?AVNodeSortKey@xalanc_1_10@@.xalanc_1_10::NodeSortKey" zeroinitializer, i32 7, !321, !79, !40, !40, !17, !136, !27}
!321 = !{%"class..?AVExecutionContext@xalanc_1_10@@.xalanc_1_10::ExecutionContext" zeroinitializer, i32 1}
!322 = !{!"S", %"class..?AV?$XalanVector@UVectorEntry@NodeSorter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UVectorEntry@NodeSorter@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !323}
!323 = !{%"struct..?AUVectorEntry@NodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter::VectorEntry" zeroinitializer, i32 1}
!324 = !{!"S", %"struct..?AUVectorEntry@NodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter::VectorEntry" zeroinitializer, i32 2, !73, !17}
!325 = !{!"S", %"class..?AVElemVariable@xalanc_1_10@@.xalanc_1_10::ElemVariable" zeroinitializer, i32 6, !78, !261, !79, !40, !326, !73}
!326 = !{%"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr" zeroinitializer, i32 0}
!327 = !{!"S", %"class..?AVXalanQNameByReference@xalanc_1_10@@.xalanc_1_10::XalanQNameByReference" zeroinitializer, i32 3, !181, !27, !27}
!328 = !{!"S", %"struct..?AUUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" zeroinitializer, i32 2, !17, !17}
!329 = !{!"S", %"class..?AVXPathEnvSupport@xalanc_1_10@@.xalanc_1_10::XPathEnvSupport" zeroinitializer, i32 1, !15}
!330 = !{!"S", %"class..?AVExtensionFunctionHandler@xalanc_1_10@@.xalanc_1_10::ExtensionFunctionHandler" zeroinitializer, i32 8, !15, !141, !141, !141, !141, !144, !331, !40}
!331 = !{%"class..?AV?$XalanSet@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanSet" zeroinitializer, i32 0}
!332 = !{!"S", %"class..?AV?$XalanSet@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanSet" zeroinitializer, i32 1, !333}
!333 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!334 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !335, !335, !336, !23, !23}
!335 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!336 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!337 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !338, !338}
!338 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" zeroinitializer, i32 1}
!339 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry>::Node" zeroinitializer, i32 3, !340, !338, !338}
!340 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" zeroinitializer, i32 0}
!341 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, bool>::Entry" zeroinitializer, i32 2, !342, !40}
!342 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@_N@std@@.std::pair" zeroinitializer, i32 1}
!343 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@_N@std@@.std::pair" zeroinitializer, i32 2, !141, !40}
!344 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !345}
!345 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!346 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !347}
!347 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!348 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@_NU?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !338}
!349 = !{!"S", %"struct..?AU?$hash_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::hash_null_terminated_arrays" zeroinitializer, i32 1, !40}
!350 = !{!"S", %"class..?AVExtensionNSHandler@xalanc_1_10@@.xalanc_1_10::ExtensionNSHandler" zeroinitializer, i32 3, !351, !331, !40}
!351 = !{%"class..?AVExtensionFunctionHandler@xalanc_1_10@@.xalanc_1_10::ExtensionFunctionHandler" zeroinitializer, i32 0}
!352 = !{!"S", %"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !144}
!353 = !{!"S", %"class..?AVFormatterToSourceTree@xalanc_1_10@@.xalanc_1_10::FormatterToSourceTree" zeroinitializer, i32 8, !354, !355, !356, !357, !358, !73, !67, !141}
!354 = !{%"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener" zeroinitializer, i32 0}
!355 = !{%"class..?AVXalanSourceTreeDocument@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocument" zeroinitializer, i32 1}
!356 = !{%"class..?AVXalanSourceTreeDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragment" zeroinitializer, i32 1}
!357 = !{%"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 1}
!358 = !{%"class..?AV?$XalanVector@PEAVXalanSourceTreeElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!359 = !{!"S", %"class..?AVXalanSourceTreeDocument@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocument" zeroinitializer, i32 22, !360, !73, !357, !361, !362, !363, !364, !365, !366, !367, !368, !369, !370, !371, !371, !372, !17, !40, !373, !374, !158, !141}
!360 = !{%"class..?AVXalanDocument@xalanc_1_10@@.xalanc_1_10::XalanDocument" zeroinitializer, i32 0}
!361 = !{%"class..?AVXalanSourceTreeAttributeAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeAllocator" zeroinitializer, i32 0}
!362 = !{%"class..?AVXalanSourceTreeAttributeNSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" zeroinitializer, i32 0}
!363 = !{%"class..?AVXalanSourceTreeCommentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeCommentAllocator" zeroinitializer, i32 0}
!364 = !{%"class..?AVXalanSourceTreeElementAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementAAllocator" zeroinitializer, i32 0}
!365 = !{%"class..?AVXalanSourceTreeElementANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANSAllocator" zeroinitializer, i32 0}
!366 = !{%"class..?AVXalanSourceTreeElementNAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNAAllocator" zeroinitializer, i32 0}
!367 = !{%"class..?AVXalanSourceTreeElementNANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANSAllocator" zeroinitializer, i32 0}
!368 = !{%"class..?AVXalanSourceTreeProcessingInstructionAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" zeroinitializer, i32 0}
!369 = !{%"class..?AVXalanSourceTreeTextAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextAllocator" zeroinitializer, i32 0}
!370 = !{%"class..?AVXalanSourceTreeTextIWSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWSAllocator" zeroinitializer, i32 0}
!371 = !{%"class..?AVXalanDOMStringPool@xalanc_1_10@@.xalanc_1_10::XalanDOMStringPool" zeroinitializer, i32 0}
!372 = !{%"class..?AV?$XalanArrayAllocator@PEAVXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanArrayAllocator" zeroinitializer, i32 0}
!373 = !{%"class..?AV?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!374 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!375 = !{!"S", %"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 9, !376, !27, !22, !355, !73, !73, !73, !73, !17}
!376 = !{%"class..?AVXalanElement@xalanc_1_10@@.xalanc_1_10::XalanElement" zeroinitializer, i32 0}
!377 = !{!"S", %"class..?AVXalanSourceTreeAttributeAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeAllocator" zeroinitializer, i32 1, !378}
!378 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeAttr@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!379 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeAttr@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !380}
!380 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!381 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !382, !382}
!382 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" zeroinitializer, i32 1}
!383 = !{!"S", %"class..?AVXalanSourceTreeAttributeNSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttributeNSAllocator" zeroinitializer, i32 1, !384}
!384 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!385 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !386}
!386 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!387 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !388, !388}
!388 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" zeroinitializer, i32 1}
!389 = !{!"S", %"class..?AVXalanSourceTreeCommentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeCommentAllocator" zeroinitializer, i32 1, !390}
!390 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeComment@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!391 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeComment@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !392}
!392 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!393 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !394, !394}
!394 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" zeroinitializer, i32 1}
!395 = !{!"S", %"class..?AVXalanSourceTreeElementAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementAAllocator" zeroinitializer, i32 1, !396}
!396 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!397 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !398}
!398 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!399 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !400, !400}
!400 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" zeroinitializer, i32 1}
!401 = !{!"S", %"class..?AVXalanSourceTreeElementANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANSAllocator" zeroinitializer, i32 1, !402}
!402 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!403 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !404}
!404 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!405 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !406, !406}
!406 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" zeroinitializer, i32 1}
!407 = !{!"S", %"class..?AVXalanSourceTreeElementNAAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNAAllocator" zeroinitializer, i32 1, !408}
!408 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!409 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !410}
!410 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!411 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !412, !412}
!412 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" zeroinitializer, i32 1}
!413 = !{!"S", %"class..?AVXalanSourceTreeElementNANSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANSAllocator" zeroinitializer, i32 1, !414}
!414 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!415 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !416}
!416 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!417 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !418, !418}
!418 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" zeroinitializer, i32 1}
!419 = !{!"S", %"class..?AVXalanSourceTreeProcessingInstructionAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstructionAllocator" zeroinitializer, i32 1, !420}
!420 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!421 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !422}
!422 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!423 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !424, !424}
!424 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" zeroinitializer, i32 1}
!425 = !{!"S", %"class..?AVXalanSourceTreeTextAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextAllocator" zeroinitializer, i32 1, !426}
!426 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeText@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!427 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeText@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !428}
!428 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!429 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !430, !430}
!430 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" zeroinitializer, i32 1}
!431 = !{!"S", %"class..?AVXalanSourceTreeTextIWSAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWSAllocator" zeroinitializer, i32 1, !432}
!432 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!433 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@V?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !23, !434}
!434 = !{%"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!435 = !{!"S", %"class..?AV?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !436, !436}
!436 = !{%"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" zeroinitializer, i32 1}
!437 = !{!"S", %"class..?AV?$XalanArrayAllocator@PEAVXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanArrayAllocator" zeroinitializer, i32 3, !438, !23, !439}
!438 = !{%"class..?AV?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!439 = !{%"struct..?AU?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 1}
!440 = !{!"S", %"class..?AV?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !441, !441}
!441 = !{%"struct..?AUNode@?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList<std::pair<unsigned long long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" zeroinitializer, i32 1}
!442 = !{!"S", %"class..?AV?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !443, !444, !22, !103, !23, !23, !445, !445, !446, !23, !23}
!443 = !{%"struct..?AU?$hash_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::hash_null_terminated_arrays" zeroinitializer, i32 0}
!444 = !{%"struct..?AU?$equal_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::equal_null_terminated_arrays" zeroinitializer, i32 0}
!445 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!446 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!447 = !{!"S", %"struct..?AU?$equal_null_terminated_arrays@G@xalanc_1_10@@.xalanc_1_10::equal_null_terminated_arrays" zeroinitializer, i32 1, !40}
!448 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !449, !449}
!449 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" zeroinitializer, i32 1}
!450 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !451}
!451 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!452 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !453, !453, !454, !23, !23}
!453 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!454 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!455 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !456, !456}
!456 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" zeroinitializer, i32 1}
!457 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !458}
!458 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!459 = !{!"S", %"class..?AVXalanSourceTreeDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragment" zeroinitializer, i32 4, !460, !22, !355, !73}
!460 = !{%"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 0}
!461 = !{!"S", %"class..?AV?$XalanVector@PEAVXalanSourceTreeElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !462}
!462 = !{%"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 2}
!463 = !{!"S", %"class..?AVXalanSourceTreeText@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 6, !464, !27, !73, !73, !73, !17}
!464 = !{%"class..?AVXalanText@xalanc_1_10@@.xalanc_1_10::XalanText" zeroinitializer, i32 0}
!465 = !{!"S", %"class..?AVXalanSourceTreeComment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeComment" zeroinitializer, i32 7, !466, !27, !355, !73, !73, !73, !17}
!466 = !{%"class..?AVXalanComment@xalanc_1_10@@.xalanc_1_10::XalanComment" zeroinitializer, i32 0}
!467 = !{!"S", %"class..?AVXalanSourceTreeProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstruction" zeroinitializer, i32 8, !468, !27, !27, !355, !73, !73, !73, !17}
!468 = !{%"class..?AVXalanProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanProcessingInstruction" zeroinitializer, i32 0}
!469 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttr> *>::Node" zeroinitializer, i32 3, !470, !382, !382}
!470 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!471 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeAttrNS> *>::Node" zeroinitializer, i32 3, !472, !388, !388}
!472 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!473 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeComment> *>::Node" zeroinitializer, i32 3, !474, !394, !394}
!474 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!475 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementA> *>::Node" zeroinitializer, i32 3, !476, !400, !400}
!476 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!477 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementANS> *>::Node" zeroinitializer, i32 3, !478, !406, !406}
!478 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!479 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNA> *>::Node" zeroinitializer, i32 3, !480, !412, !412}
!480 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!481 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeElementNANS> *>::Node" zeroinitializer, i32 3, !482, !418, !418}
!482 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!483 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeProcessingInstruction> *>::Node" zeroinitializer, i32 3, !484, !424, !424}
!484 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!485 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeText> *>::Node" zeroinitializer, i32 3, !486, !430, !430}
!486 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!487 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ArenaBlock<xalanc_1_10::XalanSourceTreeTextIWS> *>::Node" zeroinitializer, i32 3, !488, !436, !436}
!488 = !{%"class..?AV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1}
!489 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !490}
!490 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!491 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !492}
!492 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!493 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !494}
!494 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!495 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !496}
!496 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!497 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !498}
!498 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!499 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !500}
!500 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!501 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !502}
!502 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!503 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !504}
!504 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!505 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !506}
!506 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!507 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !508}
!508 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!509 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !510}
!510 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!511 = !{!"S", %"class..?AV?$ArenaBlock@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlock" zeroinitializer, i32 1, !512}
!512 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!513 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttr@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !514, !23, !23, !515}
!514 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!515 = !{%"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 1}
!516 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeAttrNS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !517, !23, !23, !518}
!517 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!518 = !{%"class..?AVXalanSourceTreeAttrNS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttrNS" zeroinitializer, i32 1}
!519 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeComment@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !520, !23, !23, !521}
!520 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeComment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!521 = !{%"class..?AVXalanSourceTreeComment@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeComment" zeroinitializer, i32 1}
!522 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !523, !23, !23, !524}
!523 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeElementA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!524 = !{%"class..?AVXalanSourceTreeElementA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 1}
!525 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !526, !23, !23, !527}
!526 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!527 = !{%"class..?AVXalanSourceTreeElementANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANS" zeroinitializer, i32 1}
!528 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNA@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !529, !23, !23, !530}
!529 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!530 = !{%"class..?AVXalanSourceTreeElementNA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 1}
!531 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeElementNANS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !532, !23, !23, !533}
!532 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!533 = !{%"class..?AVXalanSourceTreeElementNANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANS" zeroinitializer, i32 1}
!534 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !535, !23, !23, !536}
!535 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!536 = !{%"class..?AVXalanSourceTreeProcessingInstruction@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeProcessingInstruction" zeroinitializer, i32 1}
!537 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeText@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !538, !23, !23, !539}
!538 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeText@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!539 = !{%"class..?AVXalanSourceTreeText@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 1}
!540 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeTextIWS@xalanc_1_10@@_K@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !541, !23, !23, !542}
!541 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!542 = !{%"class..?AVXalanSourceTreeTextIWS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWS" zeroinitializer, i32 1}
!543 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeAttr@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!544 = !{!"S", %"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 5, !545, !27, !27, !357, !17}
!545 = !{%"class..?AVXalanAttr@xalanc_1_10@@.xalanc_1_10::XalanAttr" zeroinitializer, i32 0}
!546 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeAttrNS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!547 = !{!"S", %"class..?AVXalanSourceTreeAttrNS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttrNS" zeroinitializer, i32 4, !548, !27, !27, !27}
!548 = !{%"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 0}
!549 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeComment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!550 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeElementA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!551 = !{!"S", %"class..?AVXalanSourceTreeElementA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 4, !552, !553, !554, !17}
!552 = !{%"class..?AVXalanSourceTreeElement@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElement" zeroinitializer, i32 0}
!553 = !{%"class..?AVXalanNamedNodeMap@xalanc_1_10@@.xalanc_1_10::XalanNamedNodeMap" zeroinitializer, i32 0}
!554 = !{%"class..?AVXalanSourceTreeAttr@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeAttr" zeroinitializer, i32 2}
!555 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeElementANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!556 = !{!"S", %"class..?AVXalanSourceTreeElementANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementANS" zeroinitializer, i32 4, !557, !27, !27, !27}
!557 = !{%"class..?AVXalanSourceTreeElementA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementA" zeroinitializer, i32 0}
!558 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeElementNA@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!559 = !{!"S", %"class..?AVXalanSourceTreeElementNA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 1, !552}
!560 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeElementNANS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!561 = !{!"S", %"class..?AVXalanSourceTreeElementNANS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNANS" zeroinitializer, i32 4, !562, !27, !27, !27}
!562 = !{%"class..?AVXalanSourceTreeElementNA@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeElementNA" zeroinitializer, i32 0}
!563 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeProcessingInstruction@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!564 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeText@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!565 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeTextIWS@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!566 = !{!"S", %"class..?AVXalanSourceTreeTextIWS@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeTextIWS" zeroinitializer, i32 1, !567}
!567 = !{%"class..?AVXalanSourceTreeText@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeText" zeroinitializer, i32 0}
!568 = !{!"S", %"class..?AVFormatterToText@xalanc_1_10@@.xalanc_1_10::FormatterToText" zeroinitializer, i32 9, !354, !569, !88, !141, !40, !40, !40, !24, !17}
!569 = !{%"class..?AVWriter@xalanc_1_10@@.xalanc_1_10::Writer" zeroinitializer, i32 1}
!570 = !{!"S", %"class..?AV?$XalanVector@VXObjectPtr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXObjectPtr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !571}
!571 = !{%"class..?AVXObjectPtr@xalanc_1_10@@.xalanc_1_10::XObjectPtr" zeroinitializer, i32 1}
!572 = !{!"S", %"class..?AVXPathProcessor@xalanc_1_10@@.xalanc_1_10::XPathProcessor" zeroinitializer, i32 1, !15}
!573 = !{!"S", %"class..?AVXPathConstructionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathConstructionContextDefault" zeroinitializer, i32 3, !574, !371, !575}
!574 = !{%"class..?AVXPathConstructionContext@xalanc_1_10@@.xalanc_1_10::XPathConstructionContext" zeroinitializer, i32 0}
!575 = !{%"class..?AVXalanDOMStringCache@xalanc_1_10@@.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 0}
!576 = !{!"S", %"class..?AVXalanDOMStringCache@xalanc_1_10@@.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 4, !577, !577, !17, !578}
!577 = !{%"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!578 = !{%"class..?AVXalanDOMStringReusableAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 0}
!579 = !{!"S", %"class..?AV?$XalanVector@PEAVXalanDOMString@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !99}
!580 = !{!"S", %"class..?AVXalanDOMStringReusableAllocator@xalanc_1_10@@.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 1, !581}
!581 = !{%"class..?AV?$ReusableArenaAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!582 = !{!"S", %"class..?AV?$ReusableArenaAllocator@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 2, !583, !40}
!583 = !{%"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!584 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanDOMString@xalanc_1_10@@V?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !88, !585}
!585 = !{%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!586 = !{!"S", %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !587, !587}
!587 = !{%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 1}
!588 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 3, !589, !587, !587}
!589 = !{%"class..?AV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!590 = !{!"S", %"class..?AV?$ReusableArenaBlock@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 3, !591, !88, !88}
!591 = !{%"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!592 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanDOMString@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !177, !88, !88, !27}
!593 = !{!"S", %"class..?AVXObjectResultTreeFragProxyBase@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 1, !460}
!594 = !{!"S", %"class..?AVKeyTable@xalanc_1_10@@.xalanc_1_10::KeyTable" zeroinitializer, i32 3, !15, !595, !596}
!595 = !{%"class..?AVXalanDocument@xalanc_1_10@@.xalanc_1_10::XalanDocument" zeroinitializer, i32 1}
!596 = !{%"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!597 = !{!"S", %"class..?AV?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !225, !226, !22, !103, !23, !23, !598, !598, !599, !23, !23}
!598 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!599 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!600 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !601, !601}
!601 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" zeroinitializer, i32 1}
!602 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry>::Node" zeroinitializer, i32 3, !603, !601, !601}
!603 = !{%"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" zeroinitializer, i32 0}
!604 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>>::Entry" zeroinitializer, i32 2, !605, !40}
!605 = !{%"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 1}
!606 = !{!"S", %"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 2, !607, !608}
!607 = !{%"class..?AVXalanQNameByReference@xalanc_1_10@@.xalanc_1_10::XalanQNameByReference" zeroinitializer, i32 0}
!608 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!609 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !610, !610, !611, !23, !23}
!610 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!611 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!612 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !613, !613}
!613 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" zeroinitializer, i32 1}
!614 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry>::Node" zeroinitializer, i32 3, !615, !613, !613}
!615 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" zeroinitializer, i32 0}
!616 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::MutableNodeRefList>::Entry" zeroinitializer, i32 2, !617, !40}
!617 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@@std@@.std::pair" zeroinitializer, i32 1}
!618 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@@std@@.std::pair" zeroinitializer, i32 2, !141, !619}
!619 = !{%"class..?AVMutableNodeRefList@xalanc_1_10@@.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 0}
!620 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !621}
!621 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!622 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !623}
!623 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!624 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !613}
!625 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !626}
!626 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!627 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !628}
!628 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!629 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@V?$XalanMap@VXalanDOMString@xalanc_1_10@@VMutableNodeRefList@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !601}
!630 = !{!"S", %"class..?AVOutOfMemoryException@xercesc_2_7@@.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1, !40}
!631 = !{!"S", %"class..?AVMemoryManagerImpl@xercesc_2_7@@.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1, !632}
!632 = !{%"class..?AVMemoryManager@xercesc_2_7@@.xercesc_2_7::MemoryManager" zeroinitializer, i32 0}
!633 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry>::Node" zeroinitializer, i32 3, !634, !109, !109}
!634 = !{%"struct..?AUEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" zeroinitializer, i32 0}
!635 = !{!"S", %"struct..?AUEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanDOMString *, const xalanc_1_10::XalanDOMString *>::Entry" zeroinitializer, i32 2, !636, !40}
!636 = !{%"struct..?AU?$pair@QEBVXalanDOMString@xalanc_1_10@@PEBV12@@std@@.std::pair" zeroinitializer, i32 1}
!637 = !{!"S", %"struct..?AU?$pair@QEBVXalanDOMString@xalanc_1_10@@PEBV12@@std@@.std::pair" zeroinitializer, i32 2, !27, !27}
!638 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanDOMString@xalanc_1_10@@PEBV12@U?$XalanMapKeyTraits@PEBVXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !109}
!639 = !{!"S", %"class..?AVOutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack" zeroinitializer, i32 3, !640, !641, !23}
!640 = !{%"class..?AV?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!641 = !{%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!642 = !{!"S", %"class..?AV?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !22, !23, !643, !643}
!643 = !{%"class..?AV?$XalanVector@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!644 = !{!"S", %"class..?AV?$XalanVector@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !645}
!645 = !{%"class..?AV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!646 = !{!"S", %"class..?AV?$XalanVector@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !647}
!647 = !{%"struct..?AUOutputContext@OutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack::OutputContext" zeroinitializer, i32 1}
!648 = !{!"S", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !649, !23}
!649 = !{%"class..?AV?$XalanDeque@UOutputContext@OutputContextStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@UOutputContext@OutputContextStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!650 = !{!"S", %"struct..?AUOutputContext@OutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack::OutputContext" zeroinitializer, i32 5, !651, !652, !141, !40, !40}
!651 = !{%"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener" zeroinitializer, i32 1}
!652 = !{%"class..?AVAttributeListImpl@xalanc_1_10@@.xalanc_1_10::AttributeListImpl" zeroinitializer, i32 0}
!653 = !{!"S", %"class..?AVProblemListener@xalanc_1_10@@.xalanc_1_10::ProblemListener" zeroinitializer, i32 1, !15}
!654 = !{!"S", %"class..?AVProblemListenerDefault@xalanc_1_10@@.xalanc_1_10::ProblemListenerDefault" zeroinitializer, i32 3, !655, !22, !656}
!655 = !{%"class..?AVProblemListener@xalanc_1_10@@.xalanc_1_10::ProblemListener" zeroinitializer, i32 0}
!656 = !{%"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter" zeroinitializer, i32 1}
!657 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry>::Node" zeroinitializer, i32 3, !658, !232, !232}
!658 = !{%"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" zeroinitializer, i32 0}
!659 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanQNameByReference, const xalanc_1_10::ElemTemplate *>::Entry" zeroinitializer, i32 2, !660, !40}
!660 = !{%"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@@std@@.std::pair" zeroinitializer, i32 1}
!661 = !{!"S", %"struct..?AU?$pair@$$CBVXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@@std@@.std::pair" zeroinitializer, i32 2, !607, !190}
!662 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanQNameByReference@xalanc_1_10@@PEBVElemTemplate@2@U?$XalanMapKeyTraits@VXalanQNameByReference@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !232}
!663 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry>::Node" zeroinitializer, i32 3, !664, !241, !241}
!664 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" zeroinitializer, i32 0}
!665 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanVector<const xalanc_1_10::XalanMatchPatternData *>>::Entry" zeroinitializer, i32 2, !666, !40}
!666 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 1}
!667 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V?$XalanVector@PEBVXalanMatchPatternData@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVXalanMatchPatternData@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 2, !141, !196}
!668 = !{!"S", %"class..?AVXalanMatchPatternData@xalanc_1_10@@.xalanc_1_10::XalanMatchPatternData" zeroinitializer, i32 6, !190, !23, !141, !79, !27, !17}
!669 = !{!"S", %"class..?AV?$XalanVector@VTopLevelArg@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VTopLevelArg@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !670}
!670 = !{%"class..?AVTopLevelArg@xalanc_1_10@@.xalanc_1_10::TopLevelArg" zeroinitializer, i32 1}
!671 = !{!"S", %"class..?AVTopLevelArg@xalanc_1_10@@.xalanc_1_10::TopLevelArg" zeroinitializer, i32 3, !672, !141, !326}
!672 = !{%"class..?AVXalanQNameByValue@xalanc_1_10@@.xalanc_1_10::XalanQNameByValue" zeroinitializer, i32 0}
!673 = !{!"S", %"class..?AVXSLTEngineImpl@xalanc_1_10@@.xalanc_1_10::XSLTEngineImpl" zeroinitializer, i32 29, !674, !82, !141, !141, !675, !35, !676, !677, !678, !679, !680, !183, !40, !40, !656, !681, !17, !682, !683, !684, !685, !686, !687, !688, !652, !141, !92, !40, !689}
!674 = !{%"class..?AVXSLTProcessor@xalanc_1_10@@.xalanc_1_10::XSLTProcessor" zeroinitializer, i32 0}
!675 = !{%"class..?AVXPathFactory@xalanc_1_10@@.xalanc_1_10::XPathFactory" zeroinitializer, i32 1}
!676 = !{%"class..?AV?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 0}
!677 = !{%"class..?AV?$XalanVector@_NU?$MemoryManagedConstructionTraits@_N@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!678 = !{%"class..?AV?$XalanVector@PEBVLocator@xercesc_2_7@@U?$MemoryManagedConstructionTraits@PEBVLocator@xercesc_2_7@@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!679 = !{%"class..?AVProblemListenerDefault@xalanc_1_10@@.xalanc_1_10::ProblemListenerDefault" zeroinitializer, i32 0}
!680 = !{%"class..?AVProblemListener@xalanc_1_10@@.xalanc_1_10::ProblemListener" zeroinitializer, i32 1}
!681 = !{%"class..?AV?$XalanVector@PEAVTraceListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVTraceListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!682 = !{%"class..?AV?$XalanVector@VTopLevelArg@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VTopLevelArg@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!683 = !{%"class..?AVXMLParserLiaison@xalanc_1_10@@.xalanc_1_10::XMLParserLiaison" zeroinitializer, i32 1}
!684 = !{%"class..?AVXPathEnvSupport@xalanc_1_10@@.xalanc_1_10::XPathEnvSupport" zeroinitializer, i32 1}
!685 = !{%"class..?AVDOMSupport@xalanc_1_10@@.xalanc_1_10::DOMSupport" zeroinitializer, i32 1}
!686 = !{%"class..?AVStylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 1}
!687 = !{%"class..?AVOutputContextStack@xalanc_1_10@@.xalanc_1_10::OutputContextStack" zeroinitializer, i32 0}
!688 = !{%"class..?AVXalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack" zeroinitializer, i32 0}
!689 = !{%"class..?AVXPathConstructionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathConstructionContextDefault" zeroinitializer, i32 0}
!690 = !{!"S", %"class..?AVXSLTProcessor@xalanc_1_10@@.xalanc_1_10::XSLTProcessor" zeroinitializer, i32 1, !15}
!691 = !{!"S", %"class..?AVXPathFactory@xalanc_1_10@@.xalanc_1_10::XPathFactory" zeroinitializer, i32 1, !15}
!692 = !{!"S", %"class..?AV?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 1, !693}
!693 = !{%"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" zeroinitializer, i32 0}
!694 = !{!"S", %"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXPathProcessor@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XPathProcessor, true>::MemMgrAutoPtrData" zeroinitializer, i32 1, !695}
!695 = !{%"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXPathProcessor@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 0}
!696 = !{!"S", %"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXPathProcessor@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 2, !22, !697}
!697 = !{%"class..?AVXPathProcessor@xalanc_1_10@@.xalanc_1_10::XPathProcessor" zeroinitializer, i32 1}
!698 = !{!"S", %"class..?AV?$XalanVector@PEBVLocator@xercesc_2_7@@U?$MemoryManagedConstructionTraits@PEBVLocator@xercesc_2_7@@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !699}
!699 = !{%"class..?AVLocator@xercesc_2_7@@.xercesc_2_7::Locator" zeroinitializer, i32 2}
!700 = !{!"S", %"class..?AV?$XalanVector@PEAVTraceListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVTraceListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !701}
!701 = !{%"class..?AVTraceListener@xalanc_1_10@@.xalanc_1_10::TraceListener" zeroinitializer, i32 2}
!702 = !{!"S", %"class..?AVXalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack" zeroinitializer, i32 4, !703, !704, !704, !677}
!703 = !{%"class..?AV?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!704 = !{%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!705 = !{!"S", %"class..?AV?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !22, !23, !706, !706}
!706 = !{%"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!707 = !{!"S", %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !708}
!708 = !{%"class..?AV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!709 = !{!"S", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !710, !23}
!710 = !{%"class..?AV?$XalanDeque@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!711 = !{!"S", %"class..?AV?$XalanVector@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@VXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !712}
!712 = !{%"class..?AVXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" zeroinitializer, i32 1}
!713 = !{!"S", %"class..?AVXalanNamespacesStackEntry@XalanNamespacesStack@xalanc_1_10@@.xalanc_1_10::XalanNamespacesStack::XalanNamespacesStackEntry" zeroinitializer, i32 2, !714, !715}
!714 = !{%"class..?AV?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 0}
!715 = !{%"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespace@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 0}
!716 = !{!"S", %"class..?AV?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 4, !22, !23, !717, !717}
!717 = !{%"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!718 = !{!"S", %"struct..?AU?$XalanDequeIterator@U?$XalanDequeIteratorTraits@VXalanNamespace@xalanc_1_10@@@xalanc_1_10@@V?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@2@@xalanc_1_10@@.xalanc_1_10::XalanDequeIterator" zeroinitializer, i32 2, !719, !23}
!719 = !{%"class..?AV?$XalanDeque@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanDeque" zeroinitializer, i32 1}
!720 = !{!"S", %"class..?AV?$XalanVector@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !721}
!721 = !{%"class..?AV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 2}
!722 = !{!"S", %"class..?AVCollationCompareFunctor@XalanCollationServices@xalanc_1_10@@.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" zeroinitializer, i32 1, !15}
!723 = !{!"S", %"class..?AVStylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault" zeroinitializer, i32 43, !724, !725, !726, !73, !727, !183, !728, !729, !730, !731, !732, !733, !734, !735, !736, !737, !738, !261, !739, !17, !740, !741, !742, !677, !258, !42, !743, !744, !745, !677, !677, !746, !747, !677, !748, !749, !727, !750, !751, !40, !17, !17, !40}
!724 = !{%"class..?AVStylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext" zeroinitializer, i32 0}
!725 = !{%"class..?AVXPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 0}
!726 = !{%"class..?AVXSLTEngineImpl@xalanc_1_10@@.xalanc_1_10::XSLTEngineImpl" zeroinitializer, i32 1}
!727 = !{%"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!728 = !{%"class..?AV?$XalanVector@PEAVFormatterListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!729 = !{%"class..?AV?$XalanVector@PEAVPrintWriter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVPrintWriter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!730 = !{%"class..?AV?$XalanVector@PEAVXalanOutputStream@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanOutputStream@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!731 = !{%"class..?AVCollationCompareFunctor@XalanCollationServices@xalanc_1_10@@.xalanc_1_10::XalanCollationServices::CollationCompareFunctor" zeroinitializer, i32 1}
!732 = !{%"class..?AVFormatNumberFunctor@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" zeroinitializer, i32 1}
!733 = !{%"class..?AVVariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack" zeroinitializer, i32 0}
!734 = !{%"class..?AV?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!735 = !{%"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!736 = !{%"class..?AV?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 0}
!737 = !{%"class..?AVCountersTable@xalanc_1_10@@.xalanc_1_10::CountersTable" zeroinitializer, i32 0}
!738 = !{%"class..?AV?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 0}
!739 = !{%"class..?AV?$XalanVector@PEBVElemTemplate@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplate@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!740 = !{%"class..?AVXResultTreeFragAllocator@xalanc_1_10@@.xalanc_1_10::XResultTreeFragAllocator" zeroinitializer, i32 0}
!741 = !{%"class..?AVXalanSourceTreeDocumentFragmentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" zeroinitializer, i32 0}
!742 = !{%"class..?AVXalanSourceTreeDocumentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentAllocator" zeroinitializer, i32 0}
!743 = !{%"class..?AV?$XalanVector@VXObjectPtr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXObjectPtr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!744 = !{%"class..?AV?$XalanObjectStackCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!745 = !{%"class..?AV?$XalanVector@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!746 = !{%"class..?AV?$XalanObjectStackCache@VXalanDOMString@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@2@U?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!747 = !{%"class..?AV?$XalanObjectStackCache@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@U?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@V?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!748 = !{%"class..?AV?$XalanObjectStackCache@VFormatterToSourceTree@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@2@U?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 0}
!749 = !{%"class..?AV?$XalanVector@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!750 = !{%"class..?AV?$XalanVector@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!751 = !{%"class..?AVNodeSorter@xalanc_1_10@@.xalanc_1_10::NodeSorter" zeroinitializer, i32 0}
!752 = !{!"S", %"class..?AVXPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 11, !118, !684, !685, !67, !753, !136, !141, !754, !575, !755, !672}
!753 = !{%"class..?AV?$XalanVector@PEBVNodeRefListBase@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVNodeRefListBase@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!754 = !{%"class..?AV?$XalanObjectCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectCache" zeroinitializer, i32 0}
!755 = !{%"struct..?AUContextNodeListPositionCache@XPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" zeroinitializer, i32 0}
!756 = !{!"S", %"class..?AV?$XalanVector@PEBVNodeRefListBase@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVNodeRefListBase@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !757}
!757 = !{%"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 2}
!758 = !{!"S", %"class..?AV?$XalanObjectCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectCache" zeroinitializer, i32 4, !759, !760, !761, !762}
!759 = !{%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!760 = !{%"struct..?AU?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!761 = !{%"class..?AV?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ClearCacheResetFunctor" zeroinitializer, i32 0}
!762 = !{%"class..?AV?$XalanVector@PEAVMutableNodeRefList@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!763 = !{!"S", %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !40}
!764 = !{!"S", %"struct..?AU?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !22}
!765 = !{!"S", %"class..?AV?$ClearCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ClearCacheResetFunctor" zeroinitializer, i32 1, !40}
!766 = !{!"S", %"class..?AV?$XalanVector@PEAVMutableNodeRefList@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !767}
!767 = !{%"class..?AVMutableNodeRefList@xalanc_1_10@@.xalanc_1_10::MutableNodeRefList" zeroinitializer, i32 2}
!768 = !{!"S", %"struct..?AUContextNodeListPositionCache@XPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault::ContextNodeListPositionCache" zeroinitializer, i32 2, !73, !17}
!769 = !{!"S", %"class..?AVXMLParserLiaison@xalanc_1_10@@.xalanc_1_10::XMLParserLiaison" zeroinitializer, i32 1, !15}
!770 = !{!"S", %"class..?AV?$XalanVector@PEBVElemTemplateElement@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplateElement@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !771}
!771 = !{%"class..?AVElemTemplateElement@xalanc_1_10@@.xalanc_1_10::ElemTemplateElement" zeroinitializer, i32 2}
!772 = !{!"S", %"class..?AV?$XalanVector@PEAVFormatterListener@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterListener@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !773}
!773 = !{%"class..?AVFormatterListener@xalanc_1_10@@.xalanc_1_10::FormatterListener" zeroinitializer, i32 2}
!774 = !{!"S", %"class..?AV?$XalanVector@PEAVPrintWriter@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVPrintWriter@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !775}
!775 = !{%"class..?AVPrintWriter@xalanc_1_10@@.xalanc_1_10::PrintWriter" zeroinitializer, i32 2}
!776 = !{!"S", %"class..?AV?$XalanVector@PEAVXalanOutputStream@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanOutputStream@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !777}
!777 = !{%"class..?AVXalanOutputStream@xalanc_1_10@@.xalanc_1_10::XalanOutputStream" zeroinitializer, i32 2}
!778 = !{!"S", %"class..?AVFormatNumberFunctor@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatNumberFunctor" zeroinitializer, i32 1, !15}
!779 = !{!"S", %"class..?AVVariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack" zeroinitializer, i32 6, !780, !17, !40, !17, !781, !727}
!780 = !{%"class..?AV?$XalanVector@VStackEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VStackEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!781 = !{%"class..?AV?$XalanVector@PEBVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!782 = !{!"S", %"class..?AV?$XalanVector@VStackEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VStackEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !783}
!783 = !{%"class..?AVStackEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::StackEntry" zeroinitializer, i32 1}
!784 = !{!"S", %"class..?AV?$XalanVector@PEBVElemVariable@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemVariable@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !236}
!785 = !{!"S", %"class..?AV?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !786}
!786 = !{%"struct..?AUParamsVectorEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::ParamsVectorEntry" zeroinitializer, i32 1}
!787 = !{!"S", %"struct..?AUParamsVectorEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::ParamsVectorEntry" zeroinitializer, i32 3, !261, !326, !788}
!788 = !{%"class..?AVElemVariable@xalanc_1_10@@.xalanc_1_10::ElemVariable" zeroinitializer, i32 1}
!789 = !{!"S", %"class..?AV?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !213, !214, !22, !103, !23, !23, !790, !790, !791, !23, !23}
!790 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!791 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!792 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !793, !793}
!793 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" zeroinitializer, i32 1}
!794 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry>::Node" zeroinitializer, i32 3, !795, !793, !793}
!795 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" zeroinitializer, i32 0}
!796 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, std::pair<const xalanc_1_10::XPath *, long>>::Entry" zeroinitializer, i32 2, !797, !40}
!797 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@@std@@.std::pair" zeroinitializer, i32 1}
!798 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@@std@@.std::pair" zeroinitializer, i32 2, !141, !799}
!799 = !{%"struct..?AU?$pair@PEBVXPath@xalanc_1_10@@J@std@@.std::pair" zeroinitializer, i32 0}
!800 = !{!"S", %"struct..?AU?$pair@PEBVXPath@xalanc_1_10@@J@std@@.std::pair" zeroinitializer, i32 2, !79, !17}
!801 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !802}
!802 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!803 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !804}
!804 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!805 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@U?$pair@PEBVXPath@xalanc_1_10@@J@std@@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !793}
!806 = !{!"S", %"class..?AV?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap" zeroinitializer, i32 11, !807, !808, !22, !103, !23, !23, !809, !809, !810, !23, !23}
!807 = !{%"class..?AV?$XalanHasher@PEBVXalanNode@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHasher" zeroinitializer, i32 0}
!808 = !{%"struct..?AU?$equal_to@PEBVXalanNode@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 0}
!809 = !{%"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!810 = !{%"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!811 = !{!"S", %"class..?AV?$XalanHasher@PEBVXalanNode@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanHasher" zeroinitializer, i32 1, !40}
!812 = !{!"S", %"struct..?AU?$equal_to@PEBVXalanNode@xalanc_1_10@@@std@@.std::equal_to" zeroinitializer, i32 1, !40}
!813 = !{!"S", %"class..?AV?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !814, !814}
!814 = !{%"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" zeroinitializer, i32 1}
!815 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry>::Node" zeroinitializer, i32 3, !816, !814, !814}
!816 = !{%"struct..?AUEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" zeroinitializer, i32 0}
!817 = !{!"S", %"struct..?AUEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanNode *, xalanc_1_10::KeyTable *>::Entry" zeroinitializer, i32 2, !818, !40}
!818 = !{%"struct..?AU?$pair@QEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@@std@@.std::pair" zeroinitializer, i32 1}
!819 = !{!"S", %"struct..?AU?$pair@QEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@@std@@.std::pair" zeroinitializer, i32 2, !73, !820}
!820 = !{%"class..?AVKeyTable@xalanc_1_10@@.xalanc_1_10::KeyTable" zeroinitializer, i32 1}
!821 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$ConstructWithMemoryManagerTraits@V?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !822}
!822 = !{%"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!823 = !{!"S", %"class..?AV?$XalanVector@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !824}
!824 = !{%"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1}
!825 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanNode@xalanc_1_10@@PEAVKeyTable@2@U?$XalanMapKeyTraits@PEBVXalanNode@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !814}
!826 = !{!"S", %"class..?AV?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr" zeroinitializer, i32 1, !827}
!827 = !{%"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" zeroinitializer, i32 0}
!828 = !{!"S", %"class..?AVMemMgrAutoPtrData@?$XalanMemMgrAutoPtr@VXalanSourceTreeDocument@xalanc_1_10@@$00@xalanc_1_10@@.xalanc_1_10::XalanMemMgrAutoPtr<xalanc_1_10::XalanSourceTreeDocument, true>::MemMgrAutoPtrData" zeroinitializer, i32 1, !829}
!829 = !{%"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXalanSourceTreeDocument@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 0}
!830 = !{!"S", %"struct..?AU?$pair@PEAVMemoryManager@xercesc_2_7@@PEAVXalanSourceTreeDocument@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 2, !22, !355}
!831 = !{!"S", %"class..?AV?$XalanVector@PEBVElemTemplate@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEBVElemTemplate@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !832}
!832 = !{%"class..?AVElemTemplate@xalanc_1_10@@.xalanc_1_10::ElemTemplate" zeroinitializer, i32 2}
!833 = !{!"S", %"class..?AVXResultTreeFragAllocator@xalanc_1_10@@.xalanc_1_10::XResultTreeFragAllocator" zeroinitializer, i32 1, !834}
!834 = !{%"class..?AV?$ReusableArenaAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!835 = !{!"S", %"class..?AV?$ReusableArenaAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 2, !836, !40}
!836 = !{%"class..?AV?$ArenaAllocator@VXResultTreeFrag@xalanc_1_10@@V?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!837 = !{!"S", %"class..?AV?$ArenaAllocator@VXResultTreeFrag@xalanc_1_10@@V?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !88, !838}
!838 = !{%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!839 = !{!"S", %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !840, !840}
!840 = !{%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" zeroinitializer, i32 1}
!841 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XResultTreeFrag> *>::Node" zeroinitializer, i32 3, !842, !840, !840}
!842 = !{%"class..?AV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!843 = !{!"S", %"class..?AVXalanSourceTreeDocumentFragmentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentFragmentAllocator" zeroinitializer, i32 1, !844}
!844 = !{%"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!845 = !{!"S", %"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 2, !846, !40}
!846 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!847 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !88, !848}
!848 = !{%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!849 = !{!"S", %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !850, !850}
!850 = !{%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" zeroinitializer, i32 1}
!851 = !{!"S", %"class..?AVXalanSourceTreeDocumentAllocator@xalanc_1_10@@.xalanc_1_10::XalanSourceTreeDocumentAllocator" zeroinitializer, i32 1, !852}
!852 = !{%"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!853 = !{!"S", %"class..?AV?$ReusableArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 2, !854, !40}
!854 = !{%"class..?AV?$ArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!855 = !{!"S", %"class..?AV?$ArenaAllocator@VXalanSourceTreeDocument@xalanc_1_10@@V?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !88, !856}
!856 = !{%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!857 = !{!"S", %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !858, !858}
!858 = !{%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" zeroinitializer, i32 1}
!859 = !{!"S", %"class..?AV?$XalanObjectStackCache@VMutableNodeRefList@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VMutableNodeRefList@xalanc_1_10@@@2@U?$DeleteFunctor@VMutableNodeRefList@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !759, !760, !860, !762, !23}
!860 = !{%"class..?AV?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!861 = !{!"S", %"class..?AV?$DefaultCacheResetFunctor@VMutableNodeRefList@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !40}
!862 = !{!"S", %"class..?AV?$XalanVector@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !863}
!863 = !{%"class..?AVNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" zeroinitializer, i32 1}
!864 = !{!"S", %"class..?AVNodesToTransform@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::NodesToTransform" zeroinitializer, i32 2, !865, !17}
!865 = !{%"class..?AVNodeRefListBase@xalanc_1_10@@.xalanc_1_10::NodeRefListBase" zeroinitializer, i32 1}
!866 = !{!"S", %"class..?AV?$XalanObjectStackCache@VXalanDOMString@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@2@U?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !867, !868, !869, !577, !23}
!867 = !{%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!868 = !{%"struct..?AU?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!869 = !{%"class..?AV?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!870 = !{!"S", %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !40}
!871 = !{!"S", %"struct..?AU?$DeleteFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !22}
!872 = !{!"S", %"class..?AV?$DefaultCacheResetFunctor@VXalanDOMString@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !40}
!873 = !{!"S", %"class..?AV?$XalanObjectStackCache@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@U?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@V?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !874, !875, !876, !877, !23}
!874 = !{%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!875 = !{%"struct..?AU?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!876 = !{%"class..?AV?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!877 = !{%"class..?AV?$XalanVector@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!878 = !{!"S", %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !40}
!879 = !{!"S", %"struct..?AU?$DeleteFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !22}
!880 = !{!"S", %"class..?AV?$DefaultCacheResetFunctor@VFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !40}
!881 = !{!"S", %"class..?AV?$XalanVector@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !882}
!882 = !{%"class..?AVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" zeroinitializer, i32 2}
!883 = !{!"S", %"class..?AVFormatterToTextDOMString@StylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault::FormatterToTextDOMString" zeroinitializer, i32 2, !884, !885}
!884 = !{%"class..?AVFormatterToText@xalanc_1_10@@.xalanc_1_10::FormatterToText" zeroinitializer, i32 0}
!885 = !{%"class..?AVDOMStringPrintWriter@xalanc_1_10@@.xalanc_1_10::DOMStringPrintWriter" zeroinitializer, i32 0}
!886 = !{!"S", %"class..?AV?$XalanObjectStackCache@VFormatterToSourceTree@xalanc_1_10@@V?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@2@U?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@V?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanObjectStackCache" zeroinitializer, i32 5, !887, !888, !889, !890, !23}
!887 = !{%"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 0}
!888 = !{%"struct..?AU?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 0}
!889 = !{%"class..?AV?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 0}
!890 = !{%"class..?AV?$XalanVector@PEAVFormatterToSourceTree@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!891 = !{!"S", %"class..?AV?$DefaultCacheCreateFunctorMemMgr@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheCreateFunctorMemMgr" zeroinitializer, i32 1, !40}
!892 = !{!"S", %"struct..?AU?$DeleteFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DeleteFunctor" zeroinitializer, i32 1, !22}
!893 = !{!"S", %"class..?AV?$DefaultCacheResetFunctor@VFormatterToSourceTree@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::DefaultCacheResetFunctor" zeroinitializer, i32 1, !40}
!894 = !{!"S", %"class..?AV?$XalanVector@PEAVFormatterToSourceTree@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVFormatterToSourceTree@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !895}
!895 = !{%"class..?AVFormatterToSourceTree@xalanc_1_10@@.xalanc_1_10::FormatterToSourceTree" zeroinitializer, i32 2}
!896 = !{!"S", %"class..?AV?$XalanVector@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@U?$MemoryManagedConstructionTraits@V?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !897}
!897 = !{%"class..?AV?$XalanVector@UParamsVectorEntry@VariablesStack@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UParamsVectorEntry@VariablesStack@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!898 = !{!"S", %"class..?AV?$XalanVector@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@U?$MemoryManagedConstructionTraits@UUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@@3@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !899}
!899 = !{%"struct..?AUUseAttributeSetIndexes@StylesheetExecutionContext@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContext::UseAttributeSetIndexes" zeroinitializer, i32 1}
!900 = !{!"S", %"class..?AVXResultTreeFrag@xalanc_1_10@@.xalanc_1_10::XResultTreeFrag" zeroinitializer, i32 6, !52, !901, !27, !686, !141, !53}
!901 = !{%"class..?AVXalanDocumentFragment@xalanc_1_10@@.xalanc_1_10::XalanDocumentFragment" zeroinitializer, i32 1}
!902 = !{!"S", %"class..?AV?$ReusableArenaBlock@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 3, !903, !88, !88}
!903 = !{%"class..?AV?$ArenaBlockBase@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!904 = !{!"S", %"class..?AV?$ArenaBlockBase@VXResultTreeFrag@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !905, !88, !88, !906}
!905 = !{%"class..?AV?$XalanAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!906 = !{%"class..?AVXResultTreeFrag@xalanc_1_10@@.xalanc_1_10::XResultTreeFrag" zeroinitializer, i32 1}
!907 = !{!"S", %"class..?AV?$XalanAllocator@VXResultTreeFrag@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!908 = !{!"S", %"class..?AVStackEntry@VariablesStack@xalanc_1_10@@.xalanc_1_10::VariablesStack::StackEntry" zeroinitializer, i32 5, !17, !261, !326, !788, !85}
!909 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocumentFragment> *>::Node" zeroinitializer, i32 3, !910, !850, !850}
!910 = !{%"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!911 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanSourceTreeDocument> *>::Node" zeroinitializer, i32 3, !912, !858, !858}
!912 = !{%"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!913 = !{!"S", %"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 3, !914, !88, !88}
!914 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!915 = !{!"S", %"class..?AV?$ReusableArenaBlock@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 3, !916, !88, !88}
!916 = !{%"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!917 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocumentFragment@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !918, !88, !88, !356}
!918 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!919 = !{!"S", %"class..?AV?$ArenaBlockBase@VXalanSourceTreeDocument@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !920, !88, !88, !355}
!920 = !{%"class..?AV?$XalanAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!921 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeDocumentFragment@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!922 = !{!"S", %"class..?AV?$XalanAllocator@VXalanSourceTreeDocument@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!923 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry>::Node" zeroinitializer, i32 3, !924, !292, !292}
!924 = !{%"struct..?AUEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" zeroinitializer, i32 0}
!925 = !{!"S", %"struct..?AUEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const xalanc_1_10::XalanQName *, xalanc_1_10::XalanVector<xalanc_1_10::ElemAttributeSet *>>::Entry" zeroinitializer, i32 2, !926, !40}
!926 = !{%"struct..?AU?$pair@QEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 1}
!927 = !{!"S", %"struct..?AU?$pair@QEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@@std@@.std::pair" zeroinitializer, i32 2, !261, !928}
!928 = !{%"class..?AV?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!929 = !{!"S", %"class..?AV?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !930}
!930 = !{%"class..?AVElemAttributeSet@xalanc_1_10@@.xalanc_1_10::ElemAttributeSet" zeroinitializer, i32 2}
!931 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBVXalanQName@xalanc_1_10@@V?$XalanVector@PEAVElemAttributeSet@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVElemAttributeSet@xalanc_1_10@@@2@@2@U?$XalanMapKeyTraits@PEBVXalanQName@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !292}
!932 = !{!"S", %"class..?AVTraceListener@xalanc_1_10@@.xalanc_1_10::TraceListener" zeroinitializer, i32 1, !15}
!933 = !{!"S", %"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1, !934}
!934 = !{%"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!935 = !{!"S", %"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 2, !936, !40}
!936 = !{%"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!937 = !{!"S", %"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !15, !88, !938}
!938 = !{%"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!939 = !{!"S", %"class..?AV?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList" zeroinitializer, i32 3, !22, !940, !940}
!940 = !{%"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 1}
!941 = !{!"S", %"class..?AVXStringBase@xalanc_1_10@@.xalanc_1_10::XStringBase" zeroinitializer, i32 3, !52, !53, !942}
!942 = !{%"class..?AVXObjectResultTreeFragProxy@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 0}
!943 = !{!"S", %"class..?AVXObjectResultTreeFragProxy@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxy" zeroinitializer, i32 2, !944, !945}
!944 = !{%"class..?AVXObjectResultTreeFragProxyBase@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyBase" zeroinitializer, i32 0}
!945 = !{%"class..?AVXObjectResultTreeFragProxyText@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 0}
!946 = !{!"S", %"class..?AVXObjectResultTreeFragProxyText@xalanc_1_10@@.xalanc_1_10::XObjectResultTreeFragProxyText" zeroinitializer, i32 3, !464, !305, !22}
!947 = !{!"S", %"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached" zeroinitializer, i32 2, !948, !949}
!948 = !{%"class..?AVXStringBase@xalanc_1_10@@.xalanc_1_10::XStringBase" zeroinitializer, i32 0}
!949 = !{%"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 0}
!950 = !{!"S", %"struct..?AUNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached> *>::Node" zeroinitializer, i32 3, !951, !940, !940}
!951 = !{%"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!952 = !{!"S", %"class..?AV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 3, !953, !88, !88}
!953 = !{%"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!954 = !{!"S", %"class..?AV?$ArenaBlockBase@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !955, !88, !88, !956}
!955 = !{%"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!956 = !{%"class..?AVXStringCached@xalanc_1_10@@.xalanc_1_10::XStringCached" zeroinitializer, i32 1}
!957 = !{!"S", %"class..?AV?$XalanAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !22}
!958 = !{!"S", %"class..?AVXalanNamespace@xalanc_1_10@@.xalanc_1_10::XalanNamespace" zeroinitializer, i32 2, !141, !141}
!959 = !{!"S", %"struct..?AUNextBlock@?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XStringCached>::NextBlock" zeroinitializer, i32 2, !88, !17}
!960 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry>::Node" zeroinitializer, i32 3, !961, !456, !456}
!961 = !{%"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" zeroinitializer, i32 0}
!962 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !456}
!963 = !{!"S", %"struct..?AUEntry@?$XalanMap@VXalanDOMString@xalanc_1_10@@V12@U?$XalanMapKeyTraits@VXalanDOMString@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<xalanc_1_10::XalanDOMString, xalanc_1_10::XalanDOMString>::Entry" zeroinitializer, i32 2, !964, !40}
!964 = !{%"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V12@@std@@.std::pair" zeroinitializer, i32 1}
!965 = !{!"S", %"struct..?AU?$pair@$$CBVXalanDOMString@xalanc_1_10@@V12@@std@@.std::pair" zeroinitializer, i32 2, !141, !141}
!966 = !{!"S", %"class..?AVXalanDummyMemoryManager@xalanc_1_10@@.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1, !632}
!967 = !{!"S", %"class..?AV?$XalanVector@VXalanNamespace@xalanc_1_10@@U?$MemoryManagedConstructionTraits@VXalanNamespace@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !968}
!968 = !{%"class..?AVXalanNamespace@xalanc_1_10@@.xalanc_1_10::XalanNamespace" zeroinitializer, i32 1}
!969 = !{!"S", %"struct..?AUNode@?$XalanList@U?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@@xalanc_1_10@@.xalanc_1_10::XalanList<std::pair<unsigned long long, xalanc_1_10::XalanVector<xalanc_1_10::XalanSourceTreeAttr *> *>>::Node" zeroinitializer, i32 3, !970, !441, !441}
!970 = !{%"struct..?AU?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 0}
!971 = !{!"S", %"struct..?AU?$pair@_KPEAV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 2, !23, !972}
!972 = !{%"class..?AV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 1}
!973 = !{!"S", %"struct..?AUNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::XalanList<xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry>::Node" zeroinitializer, i32 3, !974, !449, !449}
!974 = !{%"struct..?AUEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" zeroinitializer, i32 0}
!975 = !{!"S", %"struct..?AUEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@.xalanc_1_10::XalanMap<const unsigned short *, xalanc_1_10::XalanSourceTreeElement *>::Entry" zeroinitializer, i32 2, !976, !40}
!976 = !{%"struct..?AU?$pair@QEBGPEAVXalanSourceTreeElement@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 1}
!977 = !{!"S", %"struct..?AU?$pair@QEBGPEAVXalanSourceTreeElement@xalanc_1_10@@@std@@.std::pair" zeroinitializer, i32 2, !24, !357}
!978 = !{!"S", %"struct..?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@UEntry@?$XalanMap@PEBGPEAVXalanSourceTreeElement@xalanc_1_10@@U?$XalanMapKeyTraits@PEBG@2@@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanListIteratorBase" zeroinitializer, i32 1, !449}
!979 = !{!"S", %"class..?AV?$XalanVector@PEAVXalanSourceTreeAttr@xalanc_1_10@@U?$MemoryManagedConstructionTraits@PEAVXalanSourceTreeAttr@xalanc_1_10@@@2@@xalanc_1_10@@.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !22, !23, !23, !554}
!980 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!981 = !{i32 1, !"wchar_size", i32 2}
!982 = !{i32 1, !"Virtual Function Elim", i32 0}
!983 = !{i32 7, !"PIC Level", i32 2}
!984 = !{i32 7, !"uwtable", i32 2}
!985 = !{i32 1, !"ThinLTO", i32 0}
!986 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!987 = !{i32 1, !"LTOPostLink", i32 1}
!988 = distinct !{!144, !989}
!989 = !{%eh.ThrowInfo zeroinitializer, i32 1}
!990 = distinct !{!144}
!991 = distinct !{!144}
!992 = distinct !{!144, !993}
!993 = !{%"class..?AVMemoryManagerImpl@xercesc_2_7@@.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1}
!994 = !{!"_Intel.Devirt.Target"}
!995 = distinct !{!993, !144}
!996 = distinct !{!997, !27}
!997 = !{%"class..?AVStylesheetExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::StylesheetExecutionContextDefault" zeroinitializer, i32 1}
!998 = distinct !{!999, !27}
!999 = !{%"class..?AVXPathExecutionContextDefault@xalanc_1_10@@.xalanc_1_10::XPathExecutionContextDefault" zeroinitializer, i32 1}
!1000 = distinct !{!1001, !1001, !22}
!1001 = !{%"class..?AVXStringBase@xalanc_1_10@@.xalanc_1_10::XStringBase" zeroinitializer, i32 1}
!1002 = distinct !{!1001}
!1003 = distinct !{!956, !956, !1004, !22}
!1004 = !{%"class..?AVGetAndReleaseCachedString@XPathExecutionContext@xalanc_1_10@@.xalanc_1_10::XPathExecutionContext::GetAndReleaseCachedString" zeroinitializer, i32 1}
!1005 = distinct !{!144, !956}
!1006 = !{!"F", i1 false, i32 2, !1007, !284, !27}
!1007 = !{i1 false, i32 0}
!1008 = distinct !{!1009, !1009, !22}
!1009 = !{%"class..?AVXStringCachedAllocator@xalanc_1_10@@.xalanc_1_10::XStringCachedAllocator" zeroinitializer, i32 1}
!1010 = distinct !{!1011, !1011, !22}
!1011 = !{%"class..?AV?$ReusableArenaAllocator@VXStringCached@xalanc_1_10@@@xalanc_1_10@@.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 1}
!1012 = distinct !{!144, !1011}
!1013 = distinct !{!956, !1011}
!1014 = !{}
!1015 = !{!1016}
!1016 = distinct !{!1016, !1017, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1017 = distinct !{!1017, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1018 = !{!"F", i1 false, i32 2, !144, !22, !23}
!1019 = distinct !{!1019, !1020}
!1020 = !{!"llvm.loop.mustprogress"}
!1021 = !{!1022}
!1022 = distinct !{!1022, !1023, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1023 = distinct !{!1023, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1024 = !{!1025}
!1025 = distinct !{!1025, !1026, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1026 = distinct !{!1026, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1027 = distinct !{!1011, !956}
!1028 = !{!1029}
!1029 = distinct !{!1029, !1030, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1030 = distinct !{!1030, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1031 = !{!1032}
!1032 = distinct !{!1032, !1033, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1033 = distinct !{!1033, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1034 = distinct !{!1011, !956}
!1035 = distinct !{!1036}
!1036 = !{%"class..?AV?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 1}
!1037 = !{!1038}
!1038 = distinct !{!1038, !1039, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1039 = distinct !{!1039, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1040 = !{!1041}
!1041 = distinct !{!1041, !1042, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1042 = distinct !{!1042, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1043 = !{!1044}
!1044 = distinct !{!1044, !1045, !"??$for_each@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@U?$DeleteFunctor@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@std@@YA?AU?$DeleteFunctor@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@0U12@@Z: %agg.result"}
!1045 = distinct !{!1045, !"??$for_each@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@U?$DeleteFunctor@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@std@@YA?AU?$DeleteFunctor@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@0U12@@Z"}
!1046 = !{!"F", i1 false, i32 2, !1047, !22, !144}
!1047 = !{!"void", i32 0}
!1048 = !{!"F", i1 false, i32 2, !144, !956, !17}
!1049 = distinct !{!1049, !1020}
!1050 = distinct !{!1050, !1020}
!1051 = !{!1052}
!1052 = distinct !{!1052, !1053, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1053 = distinct !{!1053, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1054 = !{!1055}
!1055 = distinct !{!1055, !1056, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@H@Z: %agg.result"}
!1056 = distinct !{!1056, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@H@Z"}
!1057 = distinct !{!1057, !1020}
!1058 = !{i64 0, !"?AP8?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@EAAAEAVMemoryManager@xercesc_2_7@@XZ"}
!1059 = distinct !{!22, !1036}
!1060 = distinct !{!1036}
!1061 = !{!"F", i1 false, i32 1, !1047, !1036}
!1062 = !{!1063}
!1063 = distinct !{!1063, !1064, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1064 = distinct !{!1064, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1065 = !{!1066}
!1066 = distinct !{!1066, !1067, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1067 = distinct !{!1067, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1068 = !{!1069}
!1069 = distinct !{!1069, !1070, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@H@Z: %agg.result"}
!1070 = distinct !{!1070, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@H@Z"}
!1071 = distinct !{!1071, !1020}
!1072 = distinct !{!144, !1036}
!1073 = distinct !{!956, !1036}
!1074 = distinct !{!1036, !956}
!1075 = distinct !{!1036, !956}
!1076 = distinct !{!1009}
!1077 = !{i64 0, !"?AP8XStringCachedAllocator@xalanc_1_10@@EAAPEAVXStringCached@1@AEAVGetAndReleaseCachedString@XPathExecutionContext@1@@Z"}
!1078 = distinct !{!956, !1009, !1004}
!1079 = !{i64 0, !"?AP8XStringCachedAllocator@xalanc_1_10@@EAA_NPEAVXStringCached@1@@Z"}
!1080 = distinct !{!1009, !956}
!1081 = !{i64 0, !"?AP8?$ArenaAllocator@VXStringCached@xalanc_1_10@@V?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@2@@xalanc_1_10@@EAA_NPEAVXStringCached@1@@Z"}
!1082 = distinct !{!1011, !956}
!1083 = !{!1084}
!1084 = distinct !{!1084, !1085, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1085 = distinct !{!1085, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1086 = !{!1087}
!1087 = distinct !{!1087, !1088, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1088 = distinct !{!1088, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1089 = !{!1090}
!1090 = distinct !{!1090, !1091, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1091 = distinct !{!1091, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1092 = !{i8 0, i8 2}
!1093 = !{!1094}
!1094 = distinct !{!1094, !1095, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1095 = distinct !{!1095, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1096 = !{!1097}
!1097 = distinct !{!1097, !1098, !"??E?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ: %agg.result"}
!1098 = distinct !{!1098, !"??E?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ"}
!1099 = !{!1100}
!1100 = distinct !{!1100, !1101, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ: %agg.result"}
!1101 = distinct !{!1101, !"??E?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ"}
!1102 = distinct !{!1102, !1020}
!1103 = !{!1104, !1106}
!1104 = distinct !{!1104, !1105, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1105 = distinct !{!1105, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1106 = distinct !{!1106, !1107, !"?rend@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ: %agg.result"}
!1107 = distinct !{!1107, !"?rend@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ"}
!1108 = !{!1109, !1111}
!1109 = distinct !{!1109, !1110, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1110 = distinct !{!1110, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1111 = distinct !{!1111, !1112, !"?rbegin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ: %agg.result"}
!1112 = distinct !{!1112, !"?rbegin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ"}
!1113 = !{!1114}
!1114 = distinct !{!1114, !1115, !"??F?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ: %agg.result"}
!1115 = distinct !{!1115, !"??F?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ"}
!1116 = !{!1117, !1119}
!1117 = distinct !{!1117, !1118, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1118 = distinct !{!1118, !"?end@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1119 = distinct !{!1119, !1120, !"?rbegin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ: %agg.result"}
!1120 = distinct !{!1120, !"?rbegin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AV?$reverse_iterator@U?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@@std@@XZ"}
!1121 = !{!1122}
!1122 = distinct !{!1122, !1123, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1123 = distinct !{!1123, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEAA?AU?$XalanListIteratorBase@U?$XalanListIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1124 = !{!1125}
!1125 = distinct !{!1125, !1126, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ: %agg.result"}
!1126 = distinct !{!1126, !"?begin@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@QEBA?AU?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@2@XZ"}
!1127 = !{!1128}
!1128 = distinct !{!1128, !1129, !"??E?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ: %agg.result"}
!1129 = distinct !{!1129, !"??E?$XalanListIteratorBase@U?$XalanListConstIteratorTraits@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@xalanc_1_10@@UNode@?$XalanList@PEAV?$ReusableArenaBlock@VXStringCached@xalanc_1_10@@G@xalanc_1_10@@@2@@xalanc_1_10@@QEAA?AU01@XZ"}
!1130 = distinct !{!1130, !1020}
!1131 = !{i64 0, !"?AP8XStringCachedAllocator@xalanc_1_10@@EAAXXZ"}
!1132 = distinct !{!1009}
!1133 = distinct !{!144, !1134}
!1134 = !{%"class..?AVXalanDummyMemoryManager@xalanc_1_10@@.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1}
!1135 = distinct !{!1134, !144}
!1136 = !{!"A", i32 6,  !144}
