; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test checks basic transformations to hoist control conditions.

; RUN: opt < %s  -S -ippred-skip-callee-legal-checks=true  -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s

; CHECK-LABEL: void @_ZN11xercesc_2_712FieldMatcher7matchedEPKtPNS_17DatatypeValidatorEb(
; CHECK: bb314:                                            ; preds = %bb311
; CHECK:  %0 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2
; CHECK:  %1 = load ptr, ptr %0, align 8
; CHECK:  %2 = icmp ne ptr %1, null
; CHECK:  br i1 %2, label %3, label %35


; CHECK:   %4 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0
; CHECK:  %5 = load i8, ptr %4, align 8
; CHECK:  %6 = icmp ne i8 %5, 0
; CHECK:  %7 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2
; CHECK:  %8 = load ptr, ptr %7, align 8
; CHECK:  %9 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %8, i64 0, i32 0, i32 0
; CHECK:  %10 = load ptr, ptr %9, align 8
; CHECK:  %11 = getelementptr inbounds ptr, ptr %10, i64 5
; CHECK:  %12 = load ptr, ptr %11, align 8
; CHECK:  %13 = icmp eq ptr %12, @_ZNK11xercesc_2_76IC_Key7getTypeEv
; CHECK:  %14 = select i1 %6, i1 %13, i1 false
; CHECK:  %15 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0
; CHECK:  %16 = load i8, ptr %15, align 8
; CHECK:  %17 = icmp ne i8 %16, 0
; CHECK:  %18 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2
; CHECK:  %19 = load ptr, ptr %18, align 8
; CHECK:  %20 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %19, i64 0, i32 0, i32 0
; CHECK:  %21 = load ptr, ptr %20, align 8
; CHECK:  %22 = getelementptr inbounds ptr, ptr %21, i64 5
; CHECK:  %23 = load ptr, ptr %22, align 8
; CHECK:  %24 = icmp ne ptr %23, @_ZNK11xercesc_2_76IC_Key7getTypeEv
; CHECK:  %25 = select i1 %17, i1 %24, i1 false
; CHECK:  %26 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2
; CHECK:  %27 = load ptr, ptr %26, align 8
; CHECK:  %28 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %27, i64 0, i32 0, i32 0
; CHECK:  %29 = load ptr, ptr %28, align 8
; CHECK:  %30 = getelementptr inbounds ptr, ptr %29, i64 5
; CHECK:  %31 = load ptr, ptr %30, align 8
; CHECK:  %32 = icmp ne ptr %31, @_ZNK11xercesc_2_79IC_KeyRef7getTypeEv
; CHECK:  %33 = select i1 %25, i1 %32, i1 false
; CHECK:  %34 = select i1 %14, i1 true, i1 %33
; CHECK:  br i1 %34, label %35, label %bb347

; CHECK: 35:                                               ; preds = %bb314, %3
; CHECK:  %i315 = tail call fastcc noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE
; CHECK: br i1 %i315, label %bb316, label %bb347

; CHECK: bb316:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" = type { ptr, i32, ptr, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" = type { ptr, ptr }
%"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" = type { %"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher", ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" = type { ptr }
%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" = type { %"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler", i64, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, ptr, %"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" }
%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" = type { ptr }
%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" = type { %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator", ptr, ptr, ptr, ptr, i32, ptr, i8, i32, i8, ptr }
%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" = type { ptr }
%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" = type { i32, ptr, ptr }
%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" = type { i32, i32, i32, i8, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" = type { i32, i32, %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool", ptr, i32, i32, i32, i32, i32, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, i32, i32 }
%"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf" = type { ptr, ptr, i32, ptr }
%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" = type { ptr }
%"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet" = type { i32, i32, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" = type { i8, ptr, ptr }
%"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" = type { i8, i32, ptr, %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator" = type { ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" = type { %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr }
%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" = type { ptr }
%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" = type { ptr }
%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i8, i8, i8, i16, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" = type { ptr }
%"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" = type { ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" = type { i32, [16384 x i16], i32, [16384 x i8], [16384 x i32], i64, i64, i32, ptr, i8, i8, ptr, i32, [49152 x i8], i32, i32, i32, i8, i32, i32, i8, i8, ptr, ptr, i8, i8, ptr, i32, ptr, i8, i32, ptr }
%"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" = type { ptr }
%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" = type { i8 }
%struct._ZTS8_IO_FILE._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque
%"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" = type { ptr, i16, ptr }
%"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" = type { %"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token", i8, i8, i32, i32, i32, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr }
%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" = type <{ ptr, i32, [4 x i8], ptr, ptr, i32 }>
%"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" = type <{ ptr, ptr, ptr, [14 x ptr], [14 x ptr], ptr, ptr, ptr, ptr, ptr, ptr, i8, i8, [6 x i8] }>
%"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" = type { ptr }
%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" = type { i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, i32, i32, i8, [7 x i8] }>
%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, i32, i32, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, i32, i32, i8 }>
%"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, ptr, ptr, i32, i8, i8, i32, i32 }
%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, i32, i8, i8, i32, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" = type { ptr }
%"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" = type { ptr }
%"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" = type { ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" = type { ptr }
%"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" = type { ptr, ptr }
%"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" = type { i8, i8, i8, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" }
%"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" = type <{ ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" = type <{ i8, [3 x i8], i32, i32, [4 x i8], ptr, ptr, ptr, ptr, i8, [7 x i8] }>
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
%"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" = type <{ ptr, ptr, i8, [7 x i8] }>
%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" = type { ptr, i32, i32, i32, ptr, ptr, i32, i32, i8, i8, i8, i32, ptr, i32, ptr, i32, i32 }
%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" = type <{ ptr, i32, [4 x i8], ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" = type { ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" = type { i8, i8, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, %"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory", ptr, ptr }
%"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" = type { ptr, ptr }
%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" = type <{ %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base", [3 x i8], i32, i32, i32, i32, i32, i32, i32, i32, [4 x i8], ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, i8, i8, [5 x i8] }>
%"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" = type { %"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef", i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" = type <{ ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" = type { ptr }
%"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" = type { ptr, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" = type { i32, i32 }
%"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" = type { ptr }
%"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" = type { i8, i32, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" = type { ptr, ptr, i16, ptr }
%"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" = type { %"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList", ptr, ptr, ptr, i32, i32 }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" = type { %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator", i64, i64, ptr, ptr }
%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" = type <{ ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i64, i64, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" = type { %"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator", i8, ptr, i32, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" = type { ptr }
%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" = type { %"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar", ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, %"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" }
%"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" = type { ptr, i8, ptr, i32, ptr, ptr, i32, i32 }
%"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" = type <{ ptr, ptr, ptr, i32, i32, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" = type { i32, i32, i32, %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool", ptr, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" = type { %"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" }
%"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" = type { ptr, ptr, ptr, i32, i32 }
%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" = type { ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base", ptr, ptr, ptr, i32, i32 }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_714HashCMStateSetE.xercesc_2_7::HashCMStateSet" = type { %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" }
%"class._ZTSN11xercesc_2_77HashPtrE.xercesc_2_7::HashPtr" = type { %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" }
%"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" = type { %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" }
%"class._ZTSN11xercesc_2_715ValueStoreCacheE.xercesc_2_7::ValueStoreCache" = type { ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_717XPathMatcherStackE.xercesc_2_7::XPathMatcherStack" = type { i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_10ValueStoreEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_710RefStackOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" = type <{ ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_76IC_KeyE.xercesc_2_7::IC_Key" = type { %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base", [4 x i8] }
%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, ptr, ptr, i32 }>
%"class._ZTSN11xercesc_2_79IC_KeyRefE.xercesc_2_7::IC_KeyRef" = type { %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base", ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" }
%"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" = type { ptr, i32, i32 }
%"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" = type { i32, i32 }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i32, i32, i32, ptr, ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i32, i32, ptr, ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" = type <{ ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i16, ptr }
%"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i16, ptr }
%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" = type { ptr, ptr, ptr, [14 x ptr], ptr, [14 x ptr], ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" = type <{ ptr, ptr, ptr, i32, [4 x i8] }>
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" }
%"class._ZTSSt9bad_alloc.std::bad_alloc" = type { %"class._ZTSSt9exception.std::exception" }
%"class._ZTSSt9exception.std::exception" = type { ptr }

$__clang_call_terminate = comdat any

$_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_722NoSuchElementExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_ = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj = comdat any

$_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj = comdat any

$_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj = comdat any

$_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj = comdat any

$_ZNK11xercesc_2_713FieldValueMap10getValueAtEj = comdat any

$_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt = comdat any

$_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv = comdat any

@.str.817 = external hidden unnamed_addr constant [4 x i8], align 1
@.str.2.1138 = external hidden unnamed_addr constant [34 x i8], align 1
@_ZN11xercesc_2_7L12gXMLErrArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [56 x i16], [72 x i16] }>, [128 x i16], <{ [20 x i16], [108 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [103 x i16], [25 x i16] }>, <{ [103 x i16], [25 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [93 x i16], [35 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [92 x i16], [36 x i16] }>, <{ [93 x i16], [35 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [120 x i16], [8 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [113 x i16], [15 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [106 x i16], [22 x i16] }>, <{ [83 x i16], [45 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [119 x i16], [9 x i16] }>, <{ [96 x i16], [32 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [83 x i16], [45 x i16] }>, <{ [112 x i16], [16 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [104 x i16], [24 x i16] }>, <{ [101 x i16], [27 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [77 x i16], [51 x i16] }>, [128 x i16], <{ [96 x i16], [32 x i16] }>, <{ [112 x i16], [16 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [107 x i16], [21 x i16] }>, <{ [109 x i16], [19 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [108 x i16], [20 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [115 x i16], [13 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [92 x i16], [36 x i16] }>, <{ [119 x i16], [9 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, [128 x i16], <{ [25 x i16], [103 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [85 x i16], [43 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [14 x i16], [114 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [90 x i16], [38 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !0
@_ZN11xercesc_2_7L15gXMLExceptArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [88 x i16], [40 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [90 x i16], [38 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [106 x i16], [22 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !320
@_ZN11xercesc_2_7L17gXMLValidityArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [49 x i16], [79 x i16] }>, [128 x i16], <{ [50 x i16], [78 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [44 x i16], [84 x i16] }>, [128 x i16], <{ [35 x i16], [93 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [101 x i16], [27 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [110 x i16], [18 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [85 x i16], [43 x i16] }>, <{ [88 x i16], [40 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !328
@_ZN11xercesc_2_7L15gXMLDOMMsgArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [115 x i16], [13 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [64 x i16], [64 x i16] }>, [128 x i16], <{ [21 x i16], [107 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !332
@.str.1700 = external hidden unnamed_addr constant [37 x i8], align 1
@.str.1.1699 = external hidden unnamed_addr constant [44 x i8], align 1
@.str.2.1698 = external hidden unnamed_addr constant [32 x i8], align 1
@.str.3.1697 = external hidden unnamed_addr constant [23 x i8], align 1
@.str.4.1696 = external hidden unnamed_addr constant [27 x i8], align 1
@.str.5.1695 = external hidden unnamed_addr constant [35 x i8], align 1
@.str.6.1694 = external hidden unnamed_addr constant [38 x i8], align 1
@.str.7.1693 = external hidden unnamed_addr constant [38 x i8], align 1
@.str.8.1692 = external hidden unnamed_addr constant [15 x i8], align 1
@_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !334
@.str.2.1798 = external hidden unnamed_addr constant [28 x i8], align 1
@_ZN11xercesc_2_7L16msgLoaderCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L15msgMutexCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L10sMsgLoaderE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !335
@_ZN11xercesc_2_7L9sMsgMutexE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !336
@_ZN11xercesc_2_7L23sScannerMutexRegisteredE = external hidden unnamed_addr global i1, align 1
@_ZN11xercesc_2_715gXMLCleanupListE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !337
@_ZN11xercesc_2_7L8gNullStrE = external hidden unnamed_addr constant [7 x i16], align 2, !intel_dtrans_type !338
@_ZN11xercesc_2_76XMLUni14fgExceptDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !221
@_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !167
@_ZN11xercesc_2_76XMLUni11fgDefErrMsgE = external hidden constant [23 x i16], align 16, !intel_dtrans_type !110
@_ZN11xercesc_2_7L16msgLoaderCleanupE.3648 = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L21validatorMutexCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L10sMsgLoaderE.3652 = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !335
@_ZTIN11xercesc_2_78XMLValid5CodesE = external hidden constant { ptr, ptr }, align 8, !intel_dtrans_type !339
@_ZN11xercesc_2_7L9sMsgMutexE.3655 = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !336
@_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !167
@_ZN11xercesc_2_76XMLUni16fgValidityDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !221
@.str.4315 = external hidden unnamed_addr constant [34 x i8], align 1
@.str.4452 = external hidden unnamed_addr constant [33 x i8], align 1
@_ZTIN11xercesc_2_722NoSuchElementExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !341
@_ZTISt9bad_alloc = external dso_local constant ptr, !intel_dtrans_type !340
@_ZTVSt9bad_alloc = external dso_local unnamed_addr constant { [5 x ptr] }, align 8, !type !342, !type !343, !type !344, !type !345, !intel_dtrans_type !346
@stderr = external dso_local local_unnamed_addr global ptr, align 8, !intel_dtrans_type !348
@_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !349
@_ZN11xercesc_2_76XMLUni15fgZeroLenStringE = external hidden constant [1 x i16], align 2, !intel_dtrans_type !350
@_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !341
@.str.6.5756 = external hidden unnamed_addr constant [31 x i8], align 1
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !341
@_ZTVN11xercesc_2_714InMemMsgLoaderE.0 = external hidden constant [8 x ptr], !type !351, !type !352, !type !353, !type !354, !type !355, !type !356, !type !357, !type !358, !type !359, !type !360, !intel_dtrans_type !361
@_ZTVN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.0 = external hidden constant [9 x ptr], !type !362, !type !363, !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !intel_dtrans_type !374
@_ZTVN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.0 = external hidden constant [9 x ptr], !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !intel_dtrans_type !374
@_ZTVN11xercesc_2_712XMLExceptionE.0 = external hidden constant [5 x ptr], !type !375, !type !376, !intel_dtrans_type !347
@_ZTVN11xercesc_2_722NoSuchElementExceptionE.0 = external hidden constant [6 x ptr], !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !intel_dtrans_type !381
@_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0 = external hidden constant [6 x ptr], !type !375, !type !376, !type !377, !type !382, !type !383, !type !384, !intel_dtrans_type !381

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind memory(none)
declare i32 @llvm.eh.typeid.for(ptr) #0

; Function Attrs: nofree
declare !intel.dtrans.func.type !854 dso_local "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2") local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #1

; Function Attrs: nofree noinline norecurse noreturn nounwind
define hidden fastcc void @__clang_call_terminate(ptr noundef %arg) unnamed_addr #2 comdat {
bb:
  %i = tail call ptr @__cxa_begin_catch(ptr %arg) #36
  tail call void @_ZSt9terminatev() #37
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #4

; Function Attrs: nofree
declare !intel.dtrans.func.type !855 dso_local noalias "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64) local_unnamed_addr #1

; Function Attrs: nofree noreturn
declare !intel.dtrans.func.type !856 dso_local void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3") local_unnamed_addr #5

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(ptr) local_unnamed_addr #1

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !857 dso_local void @_ZdaPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #6

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !858 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #9

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #10 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !859 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i5, align 8, !tbaa !861
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 1
  store i32 0, ptr %i6, align 8, !tbaa !864
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 2
  store ptr null, ptr %i7, align 8, !tbaa !872
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 3
  store i32 %arg2, ptr %i8, align 8, !tbaa !873
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 4
  store ptr null, ptr %i9, align 8, !tbaa !874
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 5
  store ptr %arg4, ptr %i10, align 8, !tbaa !875
  %i11 = icmp eq ptr %arg4, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  store ptr %i13, ptr %i10, align 8, !tbaa !875
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %arg4, %bb ]
  %i16 = icmp eq ptr %arg1, null
  br i1 %i16, label %bb45, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg1) #38
  %i19 = add i64 %i18, 1
  %i20 = and i64 %i19, 4294967295
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !861
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb43

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #39
          to label %bb42 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = tail call ptr @__cxa_begin_catch(ptr %i31) #36
  %i33 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb41 unwind label %bb34

bb34:                                             ; preds = %bb29
  %i35 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb38

bb36:                                             ; preds = %bb49, %bb34
  %i37 = phi { ptr, i32 } [ %i50, %bb49 ], [ %i35, %bb34 ]
  resume { ptr, i32 } %i37

bb38:                                             ; preds = %bb34
  %i39 = landingpad { ptr, i32 }
          catch ptr null
  %i40 = extractvalue { ptr, i32 } %i39, 0
  tail call fastcc void @__clang_call_terminate(ptr %i40) #37
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb27
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %i28, ptr nonnull align 1 %arg1, i64 %i20, i1 false)
  br label %bb45

bb43:                                             ; preds = %bb17
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb45:                                             ; preds = %bb42, %bb14
  %i46 = phi ptr [ %i28, %bb42 ], [ null, %bb14 ]
  store ptr %i46, ptr %i7, align 8, !tbaa !872
  %i47 = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0, i64 0, i64 2), ptr %i47, align 8, !tbaa !861
  invoke fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb48 unwind label %bb49

bb48:                                             ; preds = %bb45
  ret void

bb49:                                             ; preds = %bb45
  %i50 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #36
  br label %bb36
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_722NoSuchElementExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #10 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !879 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i5, align 8, !tbaa !861
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 1
  store i32 0, ptr %i6, align 8, !tbaa !864
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 2
  store ptr null, ptr %i7, align 8, !tbaa !872
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 3
  store i32 %arg2, ptr %i8, align 8, !tbaa !873
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 4
  store ptr null, ptr %i9, align 8, !tbaa !874
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 5
  store ptr %arg4, ptr %i10, align 8, !tbaa !875
  %i11 = icmp eq ptr %arg4, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  store ptr %i13, ptr %i10, align 8, !tbaa !875
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %arg4, %bb ]
  %i16 = icmp eq ptr %arg1, null
  br i1 %i16, label %bb45, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg1) #38
  %i19 = add i64 %i18, 1
  %i20 = and i64 %i19, 4294967295
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !861
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb43

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #39
          to label %bb42 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = tail call ptr @__cxa_begin_catch(ptr %i31) #36
  %i33 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb41 unwind label %bb34

bb34:                                             ; preds = %bb29
  %i35 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb38

bb36:                                             ; preds = %bb49, %bb34
  %i37 = phi { ptr, i32 } [ %i50, %bb49 ], [ %i35, %bb34 ]
  resume { ptr, i32 } %i37

bb38:                                             ; preds = %bb34
  %i39 = landingpad { ptr, i32 }
          catch ptr null
  %i40 = extractvalue { ptr, i32 } %i39, 0
  tail call fastcc void @__clang_call_terminate(ptr %i40) #37
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb27
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %i28, ptr nonnull align 1 %arg1, i64 %i20, i1 false)
  br label %bb45

bb43:                                             ; preds = %bb17
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb45:                                             ; preds = %bb42, %bb14
  %i46 = phi ptr [ %i28, %bb42 ], [ null, %bb14 ]
  store ptr %i46, ptr %i7, align 8, !tbaa !872
  %i47 = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_722NoSuchElementExceptionE.0, i64 0, i64 2), ptr %i47, align 8, !tbaa !861
  invoke fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb48 unwind label %bb49

bb48:                                             ; preds = %bb45
  ret void

bb49:                                             ; preds = %bb45
  %i50 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #36
  br label %bb36
}

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !881 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr #11

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !882 hidden noundef i32 @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", i32 noundef, ptr nocapture readnone "intel_dtrans_func_index"="3") unnamed_addr #12 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !884 hidden noundef zeroext i1 @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3") unnamed_addr #12 align 2

; Function Attrs: mustprogress nofree norecurse noreturn nounwind uwtable
define hidden void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #13 align 2 !intel.dtrans.func.type !885 {
bb:
  %i = load ptr, ptr @stderr, align 8, !tbaa !887
  %i2 = tail call fastcc noundef ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg1)
  %i3 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %i, ptr noundef nonnull @.str.817, ptr noundef nonnull %i2) #41
  tail call void @exit(i32 noundef -1) #37
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !889 dso_local noundef i32 @fprintf(ptr nocapture noundef "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ...) local_unnamed_addr #14

; Function Attrs: nofree noreturn nounwind
declare dso_local void @exit(i32 noundef) local_unnamed_addr #15

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #16 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !890 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !892
  %i2 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !897
  store ptr null, ptr %i2, align 8, !tbaa !897
  %i3 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !898
  %i4 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 1, !intel-tbaa !898
  %i5 = load ptr, ptr %i4, align 8, !tbaa !898
  store ptr %i5, ptr %i3, align 8, !tbaa !898
  %i6 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !892
  %i7 = load ptr, ptr %i6, align 8, !tbaa !892
  %i8 = icmp eq ptr %i7, null
  br i1 %i8, label %bb109, label %bb9

bb9:                                              ; preds = %bb
  %i10 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !897
  %i11 = load ptr, ptr %i10, align 8, !tbaa !897
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i11, i64 0, i32 1, !intel-tbaa !899
  %i13 = load i32, ptr %i12, align 4, !tbaa !899
  %i14 = load ptr, ptr %i3, align 8, !tbaa !898
  %i15 = invoke fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i14)
          to label %bb16 unwind label %bb20

bb16:                                             ; preds = %bb9
  %i17 = load ptr, ptr %i6, align 8, !tbaa !892
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(32) %i15, ptr noundef nonnull align 8 dereferenceable(32) %i17)
          to label %bb18 unwind label %bb22

bb18:                                             ; preds = %bb16
  store ptr %i15, ptr %i, align 8, !tbaa !892
  %i19 = icmp eq i32 %i13, 0
  br i1 %i19, label %bb109, label %bb24

bb20:                                             ; preds = %bb9
  %i21 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb96

bb22:                                             ; preds = %bb16
  %i23 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i15) #36
  br label %bb96

bb24:                                             ; preds = %bb86, %bb18
  %i25 = phi i32 [ %i92, %bb86 ], [ 0, %bb18 ]
  %i26 = load ptr, ptr %i2, align 8, !tbaa !897
  %i27 = load ptr, ptr %i10, align 8, !tbaa !897
  %i28 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 1, !intel-tbaa !899
  %i29 = load i32, ptr %i28, align 4, !tbaa !899
  %i30 = icmp ugt i32 %i29, %i25
  br i1 %i30, label %bb39, label %bb31

bb31:                                             ; preds = %bb24
  %i32 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i33 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 4, !intel-tbaa !903
  %i34 = load ptr, ptr %i33, align 8, !tbaa !903
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i32, ptr noundef nonnull @.str.4452, i32 noundef 249, i32 noundef 116, ptr noundef %i34)
          to label %bb35 unwind label %bb37

bb35:                                             ; preds = %bb31
  invoke void @__cxa_throw(ptr nonnull %i32, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
          to label %bb36 unwind label %bb94

bb36:                                             ; preds = %bb35
  unreachable

bb37:                                             ; preds = %bb31
  %i38 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call void @__cxa_free_exception(ptr %i32) #36
  br label %bb96

bb39:                                             ; preds = %bb24
  %i40 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 3, !intel-tbaa !904
  %i41 = load ptr, ptr %i40, align 8, !tbaa !904
  %i42 = zext i32 %i25 to i64
  %i43 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i41, i64 %i42, i32 2
  %i44 = load ptr, ptr %i43, align 8, !tbaa !905
  %i45 = load ptr, ptr %i3, align 8, !tbaa !898
  %i46 = icmp eq ptr %i44, null
  br i1 %i46, label %bb86, label %bb47

bb47:                                             ; preds = %bb39
  %i48 = load i16, ptr %i44, align 2, !tbaa !906
  %i49 = icmp eq i16 %i48, 0
  br i1 %i49, label %bb61, label %bb50

bb50:                                             ; preds = %bb50, %bb47
  %i51 = phi ptr [ %i52, %bb50 ], [ %i44, %bb47 ]
  %i52 = getelementptr inbounds i16, ptr %i51, i64 1
  %i53 = load i16, ptr %i52, align 2, !tbaa !906
  %i54 = icmp eq i16 %i53, 0
  br i1 %i54, label %bb55, label %bb50, !llvm.loop !908

bb55:                                             ; preds = %bb50
  %i56 = ptrtoint ptr %i52 to i64
  %i57 = ptrtoint ptr %i44 to i64
  %i58 = sub i64 2, %i57
  %i59 = add i64 %i58, %i56
  %i60 = and i64 %i59, 8589934590
  br label %bb61

bb61:                                             ; preds = %bb55, %bb47
  %i62 = phi i64 [ %i60, %bb55 ], [ 2, %bb47 ]
  %i63 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i45, i64 0, i32 0
  %i64 = load ptr, ptr %i63, align 8, !tbaa !861
  %i65 = tail call i1 @llvm.type.test(ptr %i64, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i65)
  %i66 = getelementptr inbounds ptr, ptr %i64, i64 2
  %i67 = load ptr, ptr %i66, align 8
  %i68 = icmp eq ptr %i67, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i68, label %bb69, label %bb83

bb69:                                             ; preds = %bb61
  %i70 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i62) #39
          to label %bb82 unwind label %bb71

bb71:                                             ; preds = %bb69
  %i72 = landingpad { ptr, i32 }
          catch ptr null
  %i73 = extractvalue { ptr, i32 } %i72, 0
  %i74 = tail call ptr @__cxa_begin_catch(ptr %i73) #36
  %i75 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i75, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb81 unwind label %bb76

bb76:                                             ; preds = %bb71
  %i77 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  invoke void @__cxa_end_catch()
          to label %bb96 unwind label %bb78

bb78:                                             ; preds = %bb76
  %i79 = landingpad { ptr, i32 }
          catch ptr null
  %i80 = extractvalue { ptr, i32 } %i79, 0
  tail call fastcc void @__clang_call_terminate(ptr %i80) #37
  unreachable

bb81:                                             ; preds = %bb71
  unreachable

bb82:                                             ; preds = %bb69
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i70, ptr nonnull align 2 %i44, i64 %i62, i1 false)
  br label %bb86

bb83:                                             ; preds = %bb61
  %i84 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb85 unwind label %bb94, !intel_dtrans_type !877

bb85:                                             ; preds = %bb83
  unreachable

bb86:                                             ; preds = %bb82, %bb39
  %i87 = phi ptr [ %i70, %bb82 ], [ null, %bb39 ]
  %i88 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i26, i64 0, i32 3
  %i89 = load ptr, ptr %i88, align 8
  %i90 = zext i32 %i25 to i64
  %i91 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i89, i64 %i90, i32 2
  store ptr %i87, ptr %i91, align 8
  %i92 = add nuw i32 %i25, 1
  %i93 = icmp eq i32 %i92, %i13
  br i1 %i93, label %bb109, label %bb24, !llvm.loop !910

bb94:                                             ; preds = %bb83, %bb35
  %i95 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb96

bb96:                                             ; preds = %bb94, %bb76, %bb37, %bb22, %bb20
  %i97 = phi { ptr, i32 } [ %i21, %bb20 ], [ %i23, %bb22 ], [ %i38, %bb37 ], [ %i95, %bb94 ], [ %i77, %bb76 ]
  %i98 = extractvalue { ptr, i32 } %i97, 1
  %i99 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #36
  %i100 = icmp eq i32 %i98, %i99
  br i1 %i100, label %bb101, label %bb106

bb101:                                            ; preds = %bb96
  %i102 = extractvalue { ptr, i32 } %i97, 0
  %i103 = tail call ptr @__cxa_begin_catch(ptr %i102) #36
  invoke void @__cxa_rethrow() #40
          to label %bb113 unwind label %bb104

bb104:                                            ; preds = %bb101
  %i105 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb107 unwind label %bb110

bb106:                                            ; preds = %bb96
  tail call void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr noundef nonnull align 8 dereferenceable(32) %arg), !intel_dtrans_type !911
  br label %bb107

bb107:                                            ; preds = %bb106, %bb104
  %i108 = phi { ptr, i32 } [ %i97, %bb106 ], [ %i105, %bb104 ]
  resume { ptr, i32 } %i108

bb109:                                            ; preds = %bb86, %bb18, %bb
  ret void

bb110:                                            ; preds = %bb104
  %i111 = landingpad { ptr, i32 }
          catch ptr null
  %i112 = extractvalue { ptr, i32 } %i111, 0
  tail call fastcc void @__clang_call_terminate(ptr %i112) #37
  unreachable

bb113:                                            ; preds = %bb101
  unreachable
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #16 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !912 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !913
  %i2 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 0, !intel-tbaa !913
  %i3 = load i8, ptr %i2, align 8, !tbaa !913, !range !916, !noundef !917
  store i8 %i3, ptr %i, align 8, !tbaa !913
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 1, !intel-tbaa !918
  %i6 = load i32, ptr %i5, align 4, !tbaa !918
  store i32 %i6, ptr %i4, align 4, !tbaa !918
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !919
  %i8 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 2, !intel-tbaa !919
  %i9 = load i32, ptr %i8, align 8, !tbaa !919
  store i32 %i9, ptr %i7, align 8, !tbaa !919
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !920
  store ptr null, ptr %i10, align 8, !tbaa !920
  %i11 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !921
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 4, !intel-tbaa !921
  %i13 = load ptr, ptr %i12, align 8, !tbaa !921
  store ptr %i13, ptr %i11, align 8, !tbaa !921
  %i14 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i13, i64 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !861
  %i16 = tail call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = getelementptr inbounds ptr, ptr %i15, i64 2
  %i18 = load ptr, ptr %i17, align 8
  %i19 = icmp eq ptr %i18, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i19, label %bb20, label %bb36

bb20:                                             ; preds = %bb
  %i21 = zext i32 %i9 to i64
  %i22 = mul nuw nsw i64 %i21, 24
  %i23 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i22) #39
          to label %bb38 unwind label %bb24

bb24:                                             ; preds = %bb20
  %i25 = landingpad { ptr, i32 }
          catch ptr null
  %i26 = extractvalue { ptr, i32 } %i25, 0
  %i27 = tail call ptr @__cxa_begin_catch(ptr %i26) #36
  %i28 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i28, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb35 unwind label %bb29

bb29:                                             ; preds = %bb24
  %i30 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb31 unwind label %bb32

bb31:                                             ; preds = %bb29
  resume { ptr, i32 } %i30

bb32:                                             ; preds = %bb29
  %i33 = landingpad { ptr, i32 }
          catch ptr null
  %i34 = extractvalue { ptr, i32 } %i33, 0
  tail call fastcc void @__clang_call_terminate(ptr %i34) #37
  unreachable

bb35:                                             ; preds = %bb24
  unreachable

bb36:                                             ; preds = %bb
  %i37 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb38:                                             ; preds = %bb20
  store ptr %i23, ptr %i10, align 8, !tbaa !920
  %i39 = load i32, ptr %i7, align 8, !tbaa !919
  %i40 = zext i32 %i39 to i64
  %i41 = mul nuw nsw i64 %i40, 24
  tail call void @llvm.memset.p0.i64(ptr nonnull align 8 %i23, i8 0, i64 %i41, i1 false)
  %i42 = load i32, ptr %i4, align 4, !tbaa !918
  %i43 = icmp eq i32 %i42, 0
  br i1 %i43, label %bb49, label %bb44

bb44:                                             ; preds = %bb38
  %i45 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 3, !intel-tbaa !920
  %i46 = load ptr, ptr %i45, align 8, !tbaa !920
  %i47 = load ptr, ptr %i10, align 8, !tbaa !920
  %i48 = zext i32 %i42 to i64
  br label %bb50

bb49:                                             ; preds = %bb50, %bb38
  ret void

bb50:                                             ; preds = %bb50, %bb44
  %i51 = phi i64 [ 0, %bb44 ], [ %i61, %bb50 ]
  %i52 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i46, i64 %i51, i32 0
  %i53 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i46, i64 %i51, i32 2
  %i54 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i46, i64 %i51, i32 1
  %i55 = load ptr, ptr %i54, align 8
  %i56 = load ptr, ptr %i53, align 8
  %i57 = load ptr, ptr %i52, align 8, !tbaa !922
  %i58 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 0
  %i59 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 2
  %i60 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 1
  store ptr %i55, ptr %i60, align 8
  store ptr %i56, ptr %i59, align 8
  store ptr %i57, ptr %i58, align 8, !tbaa !922
  %i61 = add nuw nsw i64 %i51, 1
  %i62 = icmp eq i64 %i61, %i48
  br i1 %i62, label %bb49, label %bb50, !llvm.loop !924
}

; Function Attrs: nounwind uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #17 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !925 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !892
  %i1 = load ptr, ptr %i, align 8, !tbaa !892
  %i2 = icmp eq ptr %i1, null
  br i1 %i2, label %bb33, label %bb3

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i1, i64 0, i32 4, !intel-tbaa !921
  %i5 = load ptr, ptr %i4, align 8, !tbaa !921
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i1, i64 0, i32 3, !intel-tbaa !920
  %i7 = load ptr, ptr %i6, align 8, !tbaa !920
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i5, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !861
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 3
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i13, label %bb19, label %bb14

bb14:                                             ; preds = %bb3
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb15 unwind label %bb16, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb15:                                             ; preds = %bb14
  unreachable

bb16:                                             ; preds = %bb14
  %i17 = landingpad { ptr, i32 }
          catch ptr null
  %i18 = extractvalue { ptr, i32 } %i17, 0
  tail call fastcc void @__clang_call_terminate(ptr %i18) #37
  unreachable

bb19:                                             ; preds = %bb3
  tail call void @_ZdlPv(ptr noundef %i7) #36
  %i20 = getelementptr inbounds i8, ptr %i1, i64 -8, !intel-tbaa !927
  %i21 = load ptr, ptr %i20, align 8, !tbaa !876
  %i22 = load ptr, ptr %i21, align 8, !tbaa !861
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 3
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i26, label %bb32, label %bb27

bb27:                                             ; preds = %bb19
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb28 unwind label %bb29, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb28:                                             ; preds = %bb27
  unreachable

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  tail call fastcc void @__clang_call_terminate(ptr %i31) #37
  unreachable

bb32:                                             ; preds = %bb19
  tail call void @_ZdlPv(ptr noundef nonnull %i20) #36
  br label %bb33

bb33:                                             ; preds = %bb32, %bb
  ret void
}

; Function Attrs: mustprogress uwtable
define dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #18 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !928 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  %i2 = load i32, ptr %i, align 4, !tbaa !918
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !921
  %i7 = load ptr, ptr %i6, align 8, !tbaa !921
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.6.5756, i32 noundef 206, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !920
  %i13 = load ptr, ptr %i12, align 8, !tbaa !920
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i13, i64 %i14, i32 0
  ret ptr %i15
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !929 hidden noundef i32 @_ZN11xercesc_2_77HashPtr10getHashValEPKvjPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", i32 noundef, ptr nocapture readnone "intel_dtrans_func_index"="3") unnamed_addr #19 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !931 hidden noundef zeroext i1 @_ZN11xercesc_2_77HashPtr6equalsEPKvS2_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef readnone "intel_dtrans_func_index"="2", ptr noundef readnone "intel_dtrans_func_index"="3") unnamed_addr #19 align 2

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_712FieldMatcher7matchedEPKtPNS_17DatatypeValidatorEb(ptr nocapture noundef nonnull readonly align 8 dereferenceable(96) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, i1 noundef zeroext %arg3) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !932 !_Intel.Devirt.Target !934 {
bb:
  br i1 %arg3, label %bb4, label %bb26

bb4:                                              ; preds = %bb
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 1, !intel-tbaa !935
  %i5 = load ptr, ptr %i, align 8, !tbaa !935
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 2, !intel-tbaa !944
  %i7 = load ptr, ptr %i6, align 8, !tbaa !944
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field", ptr %i7, i64 0, i32 2, !intel-tbaa !945
  %i9 = load ptr, ptr %i8, align 8, !tbaa !945
  %i10 = load i8, ptr %i5, align 1, !tbaa !949, !range !916, !noundef !917
  %i11 = getelementptr i8, ptr %i5, i64 48
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq i8 %i10, 0
  br i1 %i13, label %bb26, label %bb14

bb14:                                             ; preds = %bb4
  %i15 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i9, i64 0, i32 0, i32 0
  %i16 = load ptr, ptr %i15, align 8, !tbaa !861
  %i17 = tail call i1 @llvm.type.test(ptr %i16, metadata !"_ZTSN11xercesc_2_718IdentityConstraintE")
  tail call void @llvm.assume(i1 %i17)
  %i18 = getelementptr inbounds ptr, ptr %i16, i64 5
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, @_ZNK11xercesc_2_76IC_Key7getTypeEv
  br i1 %i20, label %bb21, label %bb26

bb21:                                             ; preds = %bb14
  %i22 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i12, i64 0, i32 50, !intel-tbaa !953
  %i23 = load ptr, ptr %i22, align 8, !tbaa !953
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i9, i64 0, i32 2, !intel-tbaa !990
  %i25 = load ptr, ptr %i24, align 8, !tbaa !990
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i23, i32 noundef 103, ptr noundef %i25, ptr noundef null, ptr noundef null)
  br label %bb26

bb26:                                             ; preds = %bb21, %bb14, %bb4, %bb
  %i27 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 1, !intel-tbaa !935
  %i28 = load ptr, ptr %i27, align 8, !tbaa !935
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 3, !intel-tbaa !994
  %i30 = load ptr, ptr %i29, align 8, !tbaa !994
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 2, !intel-tbaa !944
  %i32 = load ptr, ptr %i31, align 8, !tbaa !944
  %i33 = getelementptr i8, ptr %i30, i64 16
  %i34 = load ptr, ptr %i33, align 8, !tbaa !995
  %i35 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 3, !intel-tbaa !1000
  %i37 = load ptr, ptr %i36, align 8, !tbaa !1000
  %i38 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 2, !intel-tbaa !1004
  %i39 = load i32, ptr %i38, align 4, !tbaa !1004
  %i40 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i37, i64 0, i32 0
  %i41 = load ptr, ptr %i40, align 8, !tbaa !861
  %i42 = tail call i1 @llvm.type.test(ptr %i41, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i42)
  %i43 = load ptr, ptr %i41, align 8
  %i44 = icmp eq ptr %i43, @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i44, label %bb45, label %bb78

bb45:                                             ; preds = %bb26
  %i46 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 0, !intel-tbaa !1005
  %i47 = load i32, ptr %i46, align 4, !tbaa !1005
  %i48 = icmp ult i32 %i47, 65
  br i1 %i48, label %bb49, label %bb56

bb49:                                             ; preds = %bb45
  %i50 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 2, !intel-tbaa !1008
  %i51 = load i32, ptr %i50, align 4, !tbaa !1008
  %i52 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 3, !intel-tbaa !1009
  %i53 = load i32, ptr %i52, align 4, !tbaa !1009
  %i54 = mul i32 %i53, 31
  %i55 = add i32 %i54, %i51
  br label %bb75

bb56:                                             ; preds = %bb45
  %i57 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 1, !intel-tbaa !1010
  %i58 = load i32, ptr %i57, align 4, !tbaa !1010
  %i59 = add i32 %i58, -1
  %i60 = icmp sgt i32 %i59, -1
  br i1 %i60, label %bb61, label %bb75

bb61:                                             ; preds = %bb56
  %i62 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 4, !intel-tbaa !1011
  %i63 = load ptr, ptr %i62, align 8, !tbaa !1011
  %i64 = zext i32 %i59 to i64
  br label %bb65

bb65:                                             ; preds = %bb65, %bb61
  %i66 = phi i64 [ %i74, %bb65 ], [ %i64, %bb61 ]
  %i67 = phi i32 [ %i72, %bb65 ], [ 0, %bb61 ]
  %i68 = getelementptr inbounds i8, ptr %i63, i64 %i66
  %i69 = load i8, ptr %i68, align 1, !tbaa !927
  %i70 = zext i8 %i69 to i32
  %i71 = mul nsw i32 %i67, 31
  %i72 = add nsw i32 %i71, %i70
  %i73 = icmp eq i64 %i66, 0
  %i74 = add nsw i64 %i66, -1
  br i1 %i73, label %bb75, label %bb65, !llvm.loop !1012

bb75:                                             ; preds = %bb65, %bb56, %bb49
  %i76 = phi i32 [ %i55, %bb49 ], [ 0, %bb56 ], [ %i72, %bb65 ]
  %i77 = urem i32 %i76, %i39
  br label %bb110

bb78:                                             ; preds = %bb26
  %i79 = icmp eq ptr %i43, @_ZN11xercesc_2_77HashPtr10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i79, label %bb80, label %bb85

bb80:                                             ; preds = %bb78
  %i81 = ptrtoint ptr %i32 to i64
  %i82 = zext i32 %i39 to i64
  %i83 = urem i64 %i81, %i82
  %i84 = trunc i64 %i83 to i32
  br label %bb110

bb85:                                             ; preds = %bb78
  %i86 = icmp eq ptr %i32, null
  br i1 %i86, label %bb110, label %bb87

bb87:                                             ; preds = %bb85
  %i88 = load i16, ptr %i32, align 2, !tbaa !906
  %i89 = icmp eq i16 %i88, 0
  br i1 %i89, label %bb110, label %bb90

bb90:                                             ; preds = %bb87
  %i91 = zext i16 %i88 to i32
  %i92 = getelementptr inbounds i16, ptr %i32, i64 1
  %i93 = load i16, ptr %i92, align 2, !tbaa !906
  %i94 = icmp eq i16 %i93, 0
  br i1 %i94, label %bb107, label %bb95

bb95:                                             ; preds = %bb95, %bb90
  %i96 = phi i16 [ %i105, %bb95 ], [ %i93, %bb90 ]
  %i97 = phi ptr [ %i104, %bb95 ], [ %i92, %bb90 ]
  %i98 = phi i32 [ %i103, %bb95 ], [ %i91, %bb90 ]
  %i99 = mul i32 %i98, 38
  %i100 = lshr i32 %i98, 24
  %i101 = zext i16 %i96 to i32
  %i102 = add nuw nsw i32 %i100, %i101
  %i103 = add i32 %i102, %i99
  %i104 = getelementptr inbounds i16, ptr %i97, i64 1
  %i105 = load i16, ptr %i104, align 2, !tbaa !906
  %i106 = icmp eq i16 %i105, 0
  br i1 %i106, label %bb107, label %bb95, !llvm.loop !1013

bb107:                                            ; preds = %bb95, %bb90
  %i108 = phi i32 [ %i91, %bb90 ], [ %i103, %bb95 ]
  %i109 = urem i32 %i108, %i39
  br label %bb110

bb110:                                            ; preds = %bb107, %bb87, %bb85, %bb80, %bb75
  %i111 = phi i32 [ %i77, %bb75 ], [ %i84, %bb80 ], [ %i109, %bb107 ], [ 0, %bb87 ], [ 0, %bb85 ]
  %i112 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 1, !intel-tbaa !1014
  %i113 = load ptr, ptr %i112, align 8, !tbaa !1014
  %i114 = zext i32 %i111 to i64
  %i115 = getelementptr inbounds ptr, ptr %i113, i64 %i114
  %i116 = load ptr, ptr %i115, align 8, !tbaa !1015
  %i117 = icmp eq ptr %i116, null
  br i1 %i117, label %bb204, label %bb118

bb118:                                            ; preds = %bb200, %bb110
  %i119 = phi ptr [ %i202, %bb200 ], [ %i116, %bb110 ]
  %i120 = load ptr, ptr %i36, align 8, !tbaa !1000
  %i121 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 2, !intel-tbaa !1017
  %i122 = load ptr, ptr %i121, align 8, !tbaa !1017
  %i123 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i120, i64 0, i32 0
  %i124 = load ptr, ptr %i123, align 8, !tbaa !861
  %i125 = tail call i1 @llvm.type.test(ptr %i124, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i125)
  %i126 = getelementptr inbounds ptr, ptr %i124, i64 1
  %i127 = load ptr, ptr %i126, align 8
  %i128 = icmp eq ptr %i127, @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_
  br i1 %i128, label %bb129, label %bb157

bb129:                                            ; preds = %bb118
  %i130 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 0, !intel-tbaa !1005
  %i131 = load i32, ptr %i130, align 4, !tbaa !1005
  %i132 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 0, !intel-tbaa !1005
  %i133 = load i32, ptr %i132, align 4, !tbaa !1005
  %i134 = icmp eq i32 %i131, %i133
  br i1 %i134, label %bb135, label %bb200

bb135:                                            ; preds = %bb129
  %i136 = icmp ult i32 %i131, 65
  br i1 %i136, label %bb188, label %bb137

bb137:                                            ; preds = %bb135
  %i138 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 1, !intel-tbaa !1010
  %i139 = load i32, ptr %i138, align 4, !tbaa !1010
  %i140 = icmp eq i32 %i139, 0
  br i1 %i140, label %bb211, label %bb141

bb141:                                            ; preds = %bb137
  %i142 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 4
  %i143 = load ptr, ptr %i142, align 8, !tbaa !1011
  %i144 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 4, !intel-tbaa !1011
  %i145 = load ptr, ptr %i144, align 8, !tbaa !1011
  %i146 = zext i32 %i139 to i64
  br label %bb150

bb147:                                            ; preds = %bb150
  %i148 = add nuw nsw i64 %i151, 1
  %i149 = icmp eq i64 %i148, %i146
  br i1 %i149, label %bb211, label %bb150, !llvm.loop !1020

bb150:                                            ; preds = %bb147, %bb141
  %i151 = phi i64 [ 0, %bb141 ], [ %i148, %bb147 ]
  %i152 = getelementptr inbounds i8, ptr %i143, i64 %i151
  %i153 = load i8, ptr %i152, align 1, !tbaa !927
  %i154 = getelementptr inbounds i8, ptr %i145, i64 %i151
  %i155 = load i8, ptr %i154, align 1, !tbaa !927
  %i156 = icmp eq i8 %i153, %i155
  br i1 %i156, label %bb147, label %bb200

bb157:                                            ; preds = %bb118
  %i158 = icmp eq ptr %i127, @_ZN11xercesc_2_77HashPtr6equalsEPKvS2_
  br i1 %i158, label %bb159, label %bb161

bb159:                                            ; preds = %bb157
  %i160 = icmp eq ptr %i32, %i122
  br i1 %i160, label %bb211, label %bb200

bb161:                                            ; preds = %bb157
  %i162 = icmp eq ptr %i32, null
  %i163 = icmp eq ptr %i122, null
  %i164 = or i1 %i162, %i163
  br i1 %i164, label %bb169, label %bb165

bb165:                                            ; preds = %bb161
  %i166 = load i16, ptr %i32, align 2, !tbaa !906
  %i167 = load i16, ptr %i122, align 2, !tbaa !906
  %i168 = icmp eq i16 %i166, %i167
  br i1 %i168, label %bb177, label %bb200

bb169:                                            ; preds = %bb161
  br i1 %i162, label %bb173, label %bb170

bb170:                                            ; preds = %bb169
  %i171 = load i16, ptr %i32, align 2, !tbaa !906
  %i172 = icmp eq i16 %i171, 0
  br i1 %i172, label %bb173, label %bb200

bb173:                                            ; preds = %bb170, %bb169
  br i1 %i163, label %bb211, label %bb174

bb174:                                            ; preds = %bb173
  %i175 = load i16, ptr %i122, align 2, !tbaa !906
  %i176 = icmp eq i16 %i175, 0
  br i1 %i176, label %bb211, label %bb200

bb177:                                            ; preds = %bb182, %bb165
  %i178 = phi i16 [ %i185, %bb182 ], [ %i166, %bb165 ]
  %i179 = phi ptr [ %i184, %bb182 ], [ %i122, %bb165 ]
  %i180 = phi ptr [ %i183, %bb182 ], [ %i32, %bb165 ]
  %i181 = icmp eq i16 %i178, 0
  br i1 %i181, label %bb211, label %bb182

bb182:                                            ; preds = %bb177
  %i183 = getelementptr inbounds i16, ptr %i180, i64 1
  %i184 = getelementptr inbounds i16, ptr %i179, i64 1
  %i185 = load i16, ptr %i183, align 2, !tbaa !906
  %i186 = load i16, ptr %i184, align 2, !tbaa !906
  %i187 = icmp eq i16 %i185, %i186
  br i1 %i187, label %bb177, label %bb200, !llvm.loop !1021

bb188:                                            ; preds = %bb135
  %i189 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 2, !intel-tbaa !1008
  %i190 = load i32, ptr %i189, align 4, !tbaa !1008
  %i191 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 2, !intel-tbaa !1008
  %i192 = load i32, ptr %i191, align 4, !tbaa !1008
  %i193 = icmp eq i32 %i190, %i192
  %i194 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 3
  %i195 = load i32, ptr %i194, align 4
  %i196 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 3
  %i197 = load i32, ptr %i196, align 4
  %i198 = icmp eq i32 %i195, %i197
  %i199 = select i1 %i193, i1 %i198, i1 false
  br i1 %i199, label %bb211, label %bb200

bb200:                                            ; preds = %bb188, %bb182, %bb174, %bb170, %bb165, %bb159, %bb150, %bb129
  %i201 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 1, !intel-tbaa !1022
  %i202 = load ptr, ptr %i201, align 8, !tbaa !1015
  %i203 = icmp eq ptr %i202, null
  br i1 %i203, label %bb204, label %bb118, !llvm.loop !1023

bb204:                                            ; preds = %bb200, %bb110
  %i205 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  invoke fastcc void @_ZN11xercesc_2_722NoSuchElementExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i205, ptr noundef nonnull @.str.4315, i32 noundef 153, i32 noundef 50, ptr noundef %i35)
          to label %bb206 unwind label %bb209

bb206:                                            ; preds = %bb204
  tail call void @__cxa_throw(ptr nonnull %i205, ptr nonnull @_ZTIN11xercesc_2_722NoSuchElementExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb207:                                            ; preds = %bb671, %bb476, %bb425, %bb386, %bb289, %bb267, %bb209
  %i208 = phi { ptr, i32 } [ %i268, %bb267 ], [ %i290, %bb289 ], [ %i210, %bb209 ], [ %i477, %bb476 ], [ %i387, %bb386 ], [ %i426, %bb425 ], [ %i672, %bb671 ]
  resume { ptr, i32 } %i208

bb209:                                            ; preds = %bb204
  %i210 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i205) #36
  br label %bb207

bb211:                                            ; preds = %bb188, %bb177, %bb174, %bb173, %bb159, %bb147, %bb137
  %i212 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 0, !intel-tbaa !1024
  %i213 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i212, i64 0, i32 0
  %i214 = load i8, ptr %i213, align 1, !tbaa !1024, !range !916, !noundef !917
  %i215 = icmp eq i8 %i214, 0
  br i1 %i215, label %bb216, label %bb225

bb216:                                            ; preds = %bb211
  %i217 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !949
  %i218 = load i8, ptr %i217, align 8, !tbaa !949, !range !916, !noundef !917
  %i219 = icmp eq i8 %i218, 0
  br i1 %i219, label %bb225, label %bb220

bb220:                                            ; preds = %bb216
  %i221 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1025
  %i222 = load ptr, ptr %i221, align 8, !tbaa !1025
  %i223 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i222, i64 0, i32 50, !intel-tbaa !953
  %i224 = load ptr, ptr %i223, align 8, !tbaa !953
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr noundef nonnull align 8 dereferenceable(40) %i224, i32 noundef 97)
  br label %bb225

bb225:                                            ; preds = %bb220, %bb216, %bb211
  %i226 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 3, !intel-tbaa !1026
  %i227 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !892
  %i228 = load ptr, ptr %i227, align 8, !tbaa !892
  %i229 = icmp eq ptr %i228, null
  br i1 %i229, label %bb245, label %bb230

bb230:                                            ; preds = %bb225
  %i231 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i228, i64 0, i32 1, !intel-tbaa !918
  %i232 = load i32, ptr %i231, align 4, !tbaa !918
  %i233 = icmp eq i32 %i232, 0
  br i1 %i233, label %bb245, label %bb234

bb234:                                            ; preds = %bb240, %bb230
  %i235 = phi i32 [ %i241, %bb240 ], [ 0, %bb230 ]
  %i236 = load ptr, ptr %i227, align 8, !tbaa !892
  %i237 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i236, i32 noundef %i235)
  %i238 = load ptr, ptr %i237, align 8, !tbaa !922
  %i239 = icmp eq ptr %i238, %i32
  br i1 %i239, label %bb243, label %bb240

bb240:                                            ; preds = %bb234
  %i241 = add nuw i32 %i235, 1
  %i242 = icmp eq i32 %i241, %i232
  br i1 %i242, label %bb245, label %bb234, !llvm.loop !1027

bb243:                                            ; preds = %bb234
  %i244 = icmp eq i32 %i235, -1
  br i1 %i244, label %bb245, label %bb254

bb245:                                            ; preds = %bb243, %bb240, %bb230, %bb225
  %i246 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !949
  %i247 = load i8, ptr %i246, align 8, !tbaa !949, !range !916, !noundef !917
  %i248 = icmp eq i8 %i247, 0
  br i1 %i248, label %bb478, label %bb249

bb249:                                            ; preds = %bb245
  %i250 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1025
  %i251 = load ptr, ptr %i250, align 8, !tbaa !1025
  %i252 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i251, i64 0, i32 50, !intel-tbaa !953
  %i253 = load ptr, ptr %i252, align 8, !tbaa !953
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr noundef nonnull align 8 dereferenceable(40) %i253, i32 noundef 98)
  br label %bb478

bb254:                                            ; preds = %bb243
  %i255 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !1028
  %i256 = load ptr, ptr %i255, align 8, !tbaa !1028
  %i257 = icmp eq ptr %i256, null
  br i1 %i257, label %bb276, label %bb258

bb258:                                            ; preds = %bb254
  %i259 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 1, !intel-tbaa !1029
  %i260 = load i32, ptr %i259, align 4, !tbaa !1029
  %i261 = icmp ugt i32 %i260, %i235
  br i1 %i261, label %bb269, label %bb262

bb262:                                            ; preds = %bb258
  %i263 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i264 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 4, !intel-tbaa !1032
  %i265 = load ptr, ptr %i264, align 8, !tbaa !1032
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i263, ptr noundef nonnull @.str.6.5756, i32 noundef 206, i32 noundef 116, ptr noundef %i265)
          to label %bb266 unwind label %bb267

bb266:                                            ; preds = %bb262
  tail call void @__cxa_throw(ptr nonnull %i263, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb267:                                            ; preds = %bb262
  %i268 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i263) #36
  br label %bb207

bb269:                                            ; preds = %bb258
  %i270 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 3, !intel-tbaa !1033
  %i271 = load ptr, ptr %i270, align 8, !tbaa !1033
  %i272 = zext i32 %i235 to i64
  %i273 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i271, i64 %i272, i32 1
  %i274 = load ptr, ptr %i273, align 8, !tbaa !1034
  %i275 = icmp eq ptr %i274, null
  br i1 %i275, label %bb276, label %bb302

bb276:                                            ; preds = %bb269, %bb254
  %i277 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !897
  %i278 = load ptr, ptr %i277, align 8, !tbaa !897
  %i279 = icmp eq ptr %i278, null
  br i1 %i279, label %bb298, label %bb280

bb280:                                            ; preds = %bb276
  %i281 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 1, !intel-tbaa !899
  %i282 = load i32, ptr %i281, align 4, !tbaa !899
  %i283 = icmp ugt i32 %i282, %i235
  br i1 %i283, label %bb291, label %bb284

bb284:                                            ; preds = %bb280
  %i285 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i286 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 4, !intel-tbaa !903
  %i287 = load ptr, ptr %i286, align 8, !tbaa !903
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i285, ptr noundef nonnull @.str.4452, i32 noundef 249, i32 noundef 116, ptr noundef %i287)
          to label %bb288 unwind label %bb289

bb288:                                            ; preds = %bb284
  tail call void @__cxa_throw(ptr nonnull %i285, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb289:                                            ; preds = %bb284
  %i290 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i285) #36
  br label %bb207

bb291:                                            ; preds = %bb280
  %i292 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 3, !intel-tbaa !904
  %i293 = load ptr, ptr %i292, align 8, !tbaa !904
  %i294 = zext i32 %i235 to i64
  %i295 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i293, i64 %i294, i32 2
  %i296 = load ptr, ptr %i295, align 8, !tbaa !905
  %i297 = icmp eq ptr %i296, null
  br i1 %i297, label %bb298, label %bb302

bb298:                                            ; preds = %bb291, %bb276
  %i299 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 1, !intel-tbaa !1036
  %i300 = load i32, ptr %i299, align 4, !tbaa !1036
  %i301 = add nsw i32 %i300, 1
  store i32 %i301, ptr %i299, align 4, !tbaa !1036
  br label %bb302

bb302:                                            ; preds = %bb298, %bb291, %bb269
  tail call void @_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt(ptr noundef nonnull align 8 dereferenceable(32) %i226, ptr noundef %i32, ptr noundef %arg2, ptr noundef %arg1)
  %i303 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 1, !intel-tbaa !1036
  %i304 = load i32, ptr %i303, align 4, !tbaa !1036
  %i305 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !892
  %i306 = load ptr, ptr %i305, align 8, !tbaa !892
  %i307 = icmp eq ptr %i306, null
  br i1 %i307, label %bb311, label %bb308

bb308:                                            ; preds = %bb302
  %i309 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i306, i64 0, i32 1, !intel-tbaa !918
  %i310 = load i32, ptr %i309, align 4, !tbaa !918
  br label %bb311

bb311:                                            ; preds = %bb308, %bb302
  %i312 = phi i32 [ %i310, %bb308 ], [ 0, %bb302 ]
  %i313 = icmp eq i32 %i304, %i312
  br i1 %i313, label %bb314, label %bb478

bb314:                                            ; preds = %bb311
  %i315 = tail call fastcc noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE(ptr noundef nonnull align 8 dereferenceable(80) %i28, ptr noundef nonnull %i226)
  br i1 %i315, label %bb316, label %bb347

bb316:                                            ; preds = %bb314
  %i317 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !949
  %i318 = load i8, ptr %i317, align 8, !tbaa !949, !range !916, !noundef !917
  %i319 = icmp eq i8 %i318, 0
  br i1 %i319, label %bb347, label %bb320

bb320:                                            ; preds = %bb316
  %i321 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2, !intel-tbaa !1037
  %i322 = load ptr, ptr %i321, align 8, !tbaa !1037
  %i323 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i322, i64 0, i32 0, i32 0
  %i324 = load ptr, ptr %i323, align 8, !tbaa !861
  %i325 = tail call i1 @llvm.type.test(ptr %i324, metadata !"_ZTSN11xercesc_2_718IdentityConstraintE")
  tail call void @llvm.assume(i1 %i325)
  %i326 = getelementptr inbounds ptr, ptr %i324, i64 5
  %i327 = load ptr, ptr %i326, align 8
  %i328 = icmp eq ptr %i327, @_ZNK11xercesc_2_76IC_Key7getTypeEv
  br i1 %i328, label %bb339, label %bb329

bb329:                                            ; preds = %bb320
  %i330 = icmp eq ptr %i327, @_ZNK11xercesc_2_79IC_KeyRef7getTypeEv
  br i1 %i330, label %bb347, label %bb331

bb331:                                            ; preds = %bb329
  %i332 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1025
  %i333 = load ptr, ptr %i332, align 8, !tbaa !1025
  %i334 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i333, i64 0, i32 50, !intel-tbaa !953
  %i335 = load ptr, ptr %i334, align 8, !tbaa !953
  %i336 = load ptr, ptr %i321, align 8, !tbaa !1037
  %i337 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i336, i64 0, i32 2, !intel-tbaa !990
  %i338 = load ptr, ptr %i337, align 8, !tbaa !990
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i335, i32 noundef 104, ptr noundef %i338, ptr noundef null, ptr noundef null)
  br label %bb347

bb339:                                            ; preds = %bb320
  %i340 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1025
  %i341 = load ptr, ptr %i340, align 8, !tbaa !1025
  %i342 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i341, i64 0, i32 50, !intel-tbaa !953
  %i343 = load ptr, ptr %i342, align 8, !tbaa !953
  %i344 = load ptr, ptr %i321, align 8, !tbaa !1037
  %i345 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i344, i64 0, i32 2, !intel-tbaa !990
  %i346 = load ptr, ptr %i345, align 8, !tbaa !990
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i343, i32 noundef 105, ptr noundef %i346, ptr noundef null, ptr noundef null)
  br label %bb347

bb347:                                            ; preds = %bb339, %bb331, %bb329, %bb316, %bb314
  %i348 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 4, !intel-tbaa !1038
  %i349 = load ptr, ptr %i348, align 8, !tbaa !1038
  %i350 = icmp eq ptr %i349, null
  br i1 %i350, label %bb351, label %bb388

bb351:                                            ; preds = %bb347
  %i352 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 7, !intel-tbaa !1039
  %i353 = load ptr, ptr %i352, align 8, !tbaa !1039
  %i354 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 40, ptr noundef %i353)
  %i355 = load ptr, ptr %i352, align 8, !tbaa !1039
  store ptr getelementptr inbounds ([9 x ptr], ptr @_ZTVN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.0, i64 0, i64 2), ptr %i354, align 8, !tbaa !861
  %i356 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 1
  store i8 1, ptr %i356, align 1, !tbaa !1040
  %i357 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 2
  store i32 0, ptr %i357, align 4, !tbaa !1043
  %i358 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 3
  store i32 4, ptr %i358, align 4, !tbaa !1044
  %i359 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 4
  store ptr null, ptr %i359, align 8, !tbaa !1045
  %i360 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 5
  store ptr %i355, ptr %i360, align 8, !tbaa !1046
  %i361 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i355, i64 0, i32 0
  %i362 = load ptr, ptr %i361, align 8, !tbaa !861
  %i363 = tail call i1 @llvm.type.test(ptr %i362, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i363)
  %i364 = getelementptr inbounds ptr, ptr %i362, i64 2
  %i365 = load ptr, ptr %i364, align 8
  %i366 = icmp eq ptr %i365, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i366, label %bb367, label %bb381

bb367:                                            ; preds = %bb351
  %i368 = invoke noalias noundef nonnull dereferenceable(32) ptr @_Znwm(i64 noundef 32) #39
          to label %bb380 unwind label %bb369

bb369:                                            ; preds = %bb367
  %i370 = landingpad { ptr, i32 }
          catch ptr null
  %i371 = extractvalue { ptr, i32 } %i370, 0
  %i372 = tail call ptr @__cxa_begin_catch(ptr %i371) #36
  %i373 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i373, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb379 unwind label %bb374

bb374:                                            ; preds = %bb369
  %i375 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb386 unwind label %bb376

bb376:                                            ; preds = %bb374
  %i377 = landingpad { ptr, i32 }
          catch ptr null
  %i378 = extractvalue { ptr, i32 } %i377, 0
  tail call fastcc void @__clang_call_terminate(ptr %i378) #37
  unreachable

bb379:                                            ; preds = %bb369
  unreachable

bb380:                                            ; preds = %bb367
  store ptr %i368, ptr %i359, align 8, !tbaa !1045
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(32) %i368, i8 0, i64 32, i1 false), !tbaa !1047
  store ptr getelementptr inbounds ([9 x ptr], ptr @_ZTVN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.0, i64 0, i64 2), ptr %i354, align 8, !tbaa !861
  store ptr %i354, ptr %i348, align 8, !tbaa !1038
  br label %bb388

bb381:                                            ; preds = %bb351
  %i382 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb383 unwind label %bb384, !intel_dtrans_type !877, !_Intel.Devirt.Call !878

bb383:                                            ; preds = %bb381
  unreachable

bb384:                                            ; preds = %bb381
  %i385 = landingpad { ptr, i32 }
          cleanup
  br label %bb386

bb386:                                            ; preds = %bb384, %bb374
  %i387 = phi { ptr, i32 } [ %i385, %bb384 ], [ %i375, %bb374 ]
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef nonnull %i354) #36
  br label %bb207

bb388:                                            ; preds = %bb380, %bb347
  %i389 = phi ptr [ %i354, %bb380 ], [ %i349, %bb347 ]
  %i390 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 7, !intel-tbaa !1039
  %i391 = load ptr, ptr %i390, align 8, !tbaa !1039
  %i392 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 16, ptr noundef %i391)
  invoke void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(32) %i392, ptr noundef nonnull align 8 dereferenceable(32) %i226)
          to label %bb393 unwind label %bb476

bb393:                                            ; preds = %bb388
  %i394 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 2
  %i395 = load i32, ptr %i394, align 4, !tbaa !1043
  %i396 = add i32 %i395, 1
  %i397 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 3
  %i398 = load i32, ptr %i397, align 4, !tbaa !1044
  %i399 = icmp ugt i32 %i396, %i398
  br i1 %i399, label %bb403, label %bb400

bb400:                                            ; preds = %bb393
  %i401 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 4
  %i402 = load ptr, ptr %i401, align 8, !tbaa !1045
  br label %bb470

bb403:                                            ; preds = %bb393
  %i404 = lshr i32 %i398, 1
  %i405 = add i32 %i404, %i398
  %i406 = icmp ult i32 %i396, %i405
  %i407 = select i1 %i406, i32 %i405, i32 %i396
  %i408 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 5
  %i409 = load ptr, ptr %i408, align 8, !tbaa !1046
  %i410 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i409, i64 0, i32 0
  %i411 = load ptr, ptr %i410, align 8, !tbaa !861
  %i412 = tail call i1 @llvm.type.test(ptr %i411, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i412)
  %i413 = getelementptr inbounds ptr, ptr %i411, i64 2
  %i414 = load ptr, ptr %i413, align 8
  %i415 = icmp eq ptr %i414, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i415, label %bb416, label %bb436

bb416:                                            ; preds = %bb403
  %i417 = zext i32 %i407 to i64
  %i418 = shl nuw nsw i64 %i417, 3
  %i419 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i418) #39
          to label %bb431 unwind label %bb420

bb420:                                            ; preds = %bb416
  %i421 = landingpad { ptr, i32 }
          catch ptr null
  %i422 = extractvalue { ptr, i32 } %i421, 0
  %i423 = tail call ptr @__cxa_begin_catch(ptr %i422) #36
  %i424 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i424, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb430 unwind label %bb425

bb425:                                            ; preds = %bb420
  %i426 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb207 unwind label %bb427

bb427:                                            ; preds = %bb425
  %i428 = landingpad { ptr, i32 }
          catch ptr null
  %i429 = extractvalue { ptr, i32 } %i428, 0
  tail call fastcc void @__clang_call_terminate(ptr %i429) #37
  unreachable

bb430:                                            ; preds = %bb420
  unreachable

bb431:                                            ; preds = %bb416
  %i432 = load i32, ptr %i394, align 4, !tbaa !1043
  %i433 = icmp eq i32 %i432, 0
  %i434 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 4
  %i435 = load ptr, ptr %i434, align 8, !tbaa !1045
  br i1 %i433, label %bb440, label %bb438

bb436:                                            ; preds = %bb403
  %i437 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb438:                                            ; preds = %bb431
  %i439 = zext i32 %i432 to i64
  br label %bb451

bb440:                                            ; preds = %bb451, %bb431
  %i441 = icmp ult i32 %i432, %i407
  br i1 %i441, label %bb442, label %bb458

bb442:                                            ; preds = %bb440
  %i443 = zext i32 %i432 to i64
  %i444 = shl nuw nsw i64 %i443, 3
  %i445 = getelementptr i8, ptr %i419, i64 %i444
  %i446 = xor i32 %i432, -1
  %i447 = add i32 %i407, %i446
  %i448 = zext i32 %i447 to i64
  %i449 = shl nuw nsw i64 %i448, 3
  %i450 = add nuw nsw i64 %i449, 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %i445, i8 0, i64 %i450, i1 false), !tbaa !1047
  br label %bb458

bb451:                                            ; preds = %bb451, %bb438
  %i452 = phi i64 [ 0, %bb438 ], [ %i456, %bb451 ]
  %i453 = getelementptr inbounds ptr, ptr %i435, i64 %i452
  %i454 = load ptr, ptr %i453, align 8, !tbaa !1047
  %i455 = getelementptr inbounds ptr, ptr %i419, i64 %i452
  store ptr %i454, ptr %i455, align 8, !tbaa !1047
  %i456 = add nuw nsw i64 %i452, 1
  %i457 = icmp eq i64 %i456, %i439
  br i1 %i457, label %bb440, label %bb451, !llvm.loop !1049

bb458:                                            ; preds = %bb442, %bb440
  %i459 = load ptr, ptr %i408, align 8, !tbaa !1046
  %i460 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i459, i64 0, i32 0
  %i461 = load ptr, ptr %i460, align 8, !tbaa !861
  %i462 = tail call i1 @llvm.type.test(ptr %i461, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i462)
  %i463 = getelementptr inbounds ptr, ptr %i461, i64 3
  %i464 = load ptr, ptr %i463, align 8
  %i465 = icmp eq ptr %i464, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i465, label %bb466, label %bb469

bb466:                                            ; preds = %bb458
  tail call void @_ZdlPv(ptr noundef %i435) #36
  store ptr %i419, ptr %i434, align 8, !tbaa !1045
  store i32 %i407, ptr %i397, align 4, !tbaa !1044
  %i467 = load i32, ptr %i394, align 4, !tbaa !1043
  %i468 = add i32 %i467, 1
  br label %bb470

bb469:                                            ; preds = %bb458
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !926, !_Intel.Devirt.Call !878
  unreachable

bb470:                                            ; preds = %bb466, %bb400
  %i471 = phi i32 [ %i396, %bb400 ], [ %i468, %bb466 ]
  %i472 = phi i32 [ %i395, %bb400 ], [ %i467, %bb466 ]
  %i473 = phi ptr [ %i402, %bb400 ], [ %i419, %bb466 ]
  %i474 = zext i32 %i472 to i64
  %i475 = getelementptr inbounds ptr, ptr %i473, i64 %i474
  store ptr %i392, ptr %i475, align 8, !tbaa !1047
  store i32 %i471, ptr %i394, align 4, !tbaa !1043
  br label %bb478

bb476:                                            ; preds = %bb388
  %i477 = landingpad { ptr, i32 }
          cleanup
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i392) #36
  br label %bb207

bb478:                                            ; preds = %bb470, %bb311, %bb249, %bb245
  %i479 = load ptr, ptr %i29, align 8, !tbaa !994
  %i480 = load ptr, ptr %i31, align 8, !tbaa !944
  %i481 = getelementptr inbounds %"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator", ptr %i479, i64 0, i32 2, !intel-tbaa !995
  %i482 = load ptr, ptr %i481, align 8, !tbaa !995
  %i483 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 3, !intel-tbaa !1000
  %i484 = load ptr, ptr %i483, align 8, !tbaa !1000
  %i485 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 2, !intel-tbaa !1004
  %i486 = load i32, ptr %i485, align 4, !tbaa !1004
  %i487 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 0, !intel-tbaa !1050
  %i488 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i484, i64 0, i32 0
  %i489 = load ptr, ptr %i488, align 8, !tbaa !861
  %i490 = tail call i1 @llvm.type.test(ptr %i489, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i490)
  %i491 = load ptr, ptr %i489, align 8
  %i492 = icmp eq ptr %i491, @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i492, label %bb493, label %bb526

bb493:                                            ; preds = %bb478
  %i494 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 0, !intel-tbaa !1005
  %i495 = load i32, ptr %i494, align 4, !tbaa !1005
  %i496 = icmp ult i32 %i495, 65
  br i1 %i496, label %bb497, label %bb504

bb497:                                            ; preds = %bb493
  %i498 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 2, !intel-tbaa !1008
  %i499 = load i32, ptr %i498, align 4, !tbaa !1008
  %i500 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 3, !intel-tbaa !1009
  %i501 = load i32, ptr %i500, align 4, !tbaa !1009
  %i502 = mul i32 %i501, 31
  %i503 = add i32 %i502, %i499
  br label %bb523

bb504:                                            ; preds = %bb493
  %i505 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 1, !intel-tbaa !1010
  %i506 = load i32, ptr %i505, align 4, !tbaa !1010
  %i507 = add i32 %i506, -1
  %i508 = icmp sgt i32 %i507, -1
  br i1 %i508, label %bb509, label %bb523

bb509:                                            ; preds = %bb504
  %i510 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 4, !intel-tbaa !1011
  %i511 = load ptr, ptr %i510, align 8, !tbaa !1011
  %i512 = zext i32 %i507 to i64
  br label %bb513

bb513:                                            ; preds = %bb513, %bb509
  %i514 = phi i64 [ %i522, %bb513 ], [ %i512, %bb509 ]
  %i515 = phi i32 [ %i520, %bb513 ], [ 0, %bb509 ]
  %i516 = getelementptr inbounds i8, ptr %i511, i64 %i514
  %i517 = load i8, ptr %i516, align 1, !tbaa !927
  %i518 = zext i8 %i517 to i32
  %i519 = mul nsw i32 %i515, 31
  %i520 = add nsw i32 %i519, %i518
  %i521 = icmp eq i64 %i514, 0
  %i522 = add nsw i64 %i514, -1
  br i1 %i521, label %bb523, label %bb513, !llvm.loop !1012

bb523:                                            ; preds = %bb513, %bb504, %bb497
  %i524 = phi i32 [ %i503, %bb497 ], [ 0, %bb504 ], [ %i520, %bb513 ]
  %i525 = urem i32 %i524, %i486
  br label %bb558

bb526:                                            ; preds = %bb478
  %i527 = icmp eq ptr %i491, @_ZN11xercesc_2_77HashPtr10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i527, label %bb528, label %bb533

bb528:                                            ; preds = %bb526
  %i529 = ptrtoint ptr %i480 to i64
  %i530 = zext i32 %i486 to i64
  %i531 = urem i64 %i529, %i530
  %i532 = trunc i64 %i531 to i32
  br label %bb558

bb533:                                            ; preds = %bb526
  %i534 = icmp eq ptr %i480, null
  br i1 %i534, label %bb558, label %bb535

bb535:                                            ; preds = %bb533
  %i536 = load i16, ptr %i480, align 2, !tbaa !906
  %i537 = icmp eq i16 %i536, 0
  br i1 %i537, label %bb558, label %bb538

bb538:                                            ; preds = %bb535
  %i539 = zext i16 %i536 to i32
  %i540 = getelementptr inbounds i16, ptr %i480, i64 1
  %i541 = load i16, ptr %i540, align 2, !tbaa !906
  %i542 = icmp eq i16 %i541, 0
  br i1 %i542, label %bb555, label %bb543

bb543:                                            ; preds = %bb543, %bb538
  %i544 = phi i16 [ %i553, %bb543 ], [ %i541, %bb538 ]
  %i545 = phi ptr [ %i552, %bb543 ], [ %i540, %bb538 ]
  %i546 = phi i32 [ %i551, %bb543 ], [ %i539, %bb538 ]
  %i547 = mul i32 %i546, 38
  %i548 = lshr i32 %i546, 24
  %i549 = zext i16 %i544 to i32
  %i550 = add nuw nsw i32 %i548, %i549
  %i551 = add i32 %i550, %i547
  %i552 = getelementptr inbounds i16, ptr %i545, i64 1
  %i553 = load i16, ptr %i552, align 2, !tbaa !906
  %i554 = icmp eq i16 %i553, 0
  br i1 %i554, label %bb555, label %bb543, !llvm.loop !1013

bb555:                                            ; preds = %bb543, %bb538
  %i556 = phi i32 [ %i539, %bb538 ], [ %i551, %bb543 ]
  %i557 = urem i32 %i556, %i486
  br label %bb558

bb558:                                            ; preds = %bb555, %bb535, %bb533, %bb528, %bb523
  %i559 = phi i32 [ %i525, %bb523 ], [ %i532, %bb528 ], [ %i557, %bb555 ], [ 0, %bb535 ], [ 0, %bb533 ]
  %i560 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 1, !intel-tbaa !1014
  %i561 = load ptr, ptr %i560, align 8, !tbaa !1014
  %i562 = zext i32 %i559 to i64
  %i563 = getelementptr inbounds ptr, ptr %i561, i64 %i562
  %i564 = load ptr, ptr %i563, align 8, !tbaa !1015
  %i565 = icmp eq ptr %i564, null
  br i1 %i565, label %bb656, label %bb566

bb566:                                            ; preds = %bb648, %bb558
  %i567 = phi ptr [ %i650, %bb648 ], [ %i564, %bb558 ]
  %i568 = load ptr, ptr %i483, align 8, !tbaa !1000
  %i569 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 2
  %i570 = load ptr, ptr %i569, align 8, !tbaa !1017
  %i571 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i568, i64 0, i32 0
  %i572 = load ptr, ptr %i571, align 8, !tbaa !861
  %i573 = tail call i1 @llvm.type.test(ptr %i572, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i573)
  %i574 = getelementptr inbounds ptr, ptr %i572, i64 1
  %i575 = load ptr, ptr %i574, align 8
  %i576 = icmp eq ptr %i575, @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_
  br i1 %i576, label %bb577, label %bb605

bb577:                                            ; preds = %bb566
  %i578 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 0, !intel-tbaa !1005
  %i579 = load i32, ptr %i578, align 4, !tbaa !1005
  %i580 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 0, !intel-tbaa !1005
  %i581 = load i32, ptr %i580, align 4, !tbaa !1005
  %i582 = icmp eq i32 %i579, %i581
  br i1 %i582, label %bb583, label %bb648

bb583:                                            ; preds = %bb577
  %i584 = icmp ult i32 %i579, 65
  br i1 %i584, label %bb636, label %bb585

bb585:                                            ; preds = %bb583
  %i586 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 1, !intel-tbaa !1010
  %i587 = load i32, ptr %i586, align 4, !tbaa !1010
  %i588 = icmp eq i32 %i587, 0
  br i1 %i588, label %bb652, label %bb589

bb589:                                            ; preds = %bb585
  %i590 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 4
  %i591 = load ptr, ptr %i590, align 8, !tbaa !1011
  %i592 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 4, !intel-tbaa !1011
  %i593 = load ptr, ptr %i592, align 8, !tbaa !1011
  %i594 = zext i32 %i587 to i64
  br label %bb598

bb595:                                            ; preds = %bb598
  %i596 = add nuw nsw i64 %i599, 1
  %i597 = icmp eq i64 %i596, %i594
  br i1 %i597, label %bb652, label %bb598, !llvm.loop !1020

bb598:                                            ; preds = %bb595, %bb589
  %i599 = phi i64 [ 0, %bb589 ], [ %i596, %bb595 ]
  %i600 = getelementptr inbounds i8, ptr %i591, i64 %i599
  %i601 = load i8, ptr %i600, align 1, !tbaa !927
  %i602 = getelementptr inbounds i8, ptr %i593, i64 %i599
  %i603 = load i8, ptr %i602, align 1, !tbaa !927
  %i604 = icmp eq i8 %i601, %i603
  br i1 %i604, label %bb595, label %bb648

bb605:                                            ; preds = %bb566
  %i606 = icmp eq ptr %i575, @_ZN11xercesc_2_77HashPtr6equalsEPKvS2_
  br i1 %i606, label %bb607, label %bb609

bb607:                                            ; preds = %bb605
  %i608 = icmp eq ptr %i480, %i570
  br i1 %i608, label %bb652, label %bb648

bb609:                                            ; preds = %bb605
  %i610 = icmp eq ptr %i480, null
  %i611 = icmp eq ptr %i570, null
  %i612 = or i1 %i610, %i611
  br i1 %i612, label %bb617, label %bb613

bb613:                                            ; preds = %bb609
  %i614 = load i16, ptr %i480, align 2, !tbaa !906
  %i615 = load i16, ptr %i570, align 2, !tbaa !906
  %i616 = icmp eq i16 %i614, %i615
  br i1 %i616, label %bb625, label %bb648

bb617:                                            ; preds = %bb609
  br i1 %i610, label %bb621, label %bb618

bb618:                                            ; preds = %bb617
  %i619 = load i16, ptr %i480, align 2, !tbaa !906
  %i620 = icmp eq i16 %i619, 0
  br i1 %i620, label %bb621, label %bb648

bb621:                                            ; preds = %bb618, %bb617
  br i1 %i611, label %bb652, label %bb622

bb622:                                            ; preds = %bb621
  %i623 = load i16, ptr %i570, align 2, !tbaa !906
  %i624 = icmp eq i16 %i623, 0
  br i1 %i624, label %bb652, label %bb648

bb625:                                            ; preds = %bb630, %bb613
  %i626 = phi i16 [ %i633, %bb630 ], [ %i614, %bb613 ]
  %i627 = phi ptr [ %i632, %bb630 ], [ %i570, %bb613 ]
  %i628 = phi ptr [ %i631, %bb630 ], [ %i480, %bb613 ]
  %i629 = icmp eq i16 %i626, 0
  br i1 %i629, label %bb652, label %bb630

bb630:                                            ; preds = %bb625
  %i631 = getelementptr inbounds i16, ptr %i628, i64 1
  %i632 = getelementptr inbounds i16, ptr %i627, i64 1
  %i633 = load i16, ptr %i631, align 2, !tbaa !906
  %i634 = load i16, ptr %i632, align 2, !tbaa !906
  %i635 = icmp eq i16 %i633, %i634
  br i1 %i635, label %bb625, label %bb648, !llvm.loop !1021

bb636:                                            ; preds = %bb583
  %i637 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 2, !intel-tbaa !1008
  %i638 = load i32, ptr %i637, align 4, !tbaa !1008
  %i639 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 2, !intel-tbaa !1008
  %i640 = load i32, ptr %i639, align 4, !tbaa !1008
  %i641 = icmp eq i32 %i638, %i640
  %i642 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 3
  %i643 = load i32, ptr %i642, align 4
  %i644 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 3
  %i645 = load i32, ptr %i644, align 4
  %i646 = icmp eq i32 %i643, %i645
  %i647 = select i1 %i641, i1 %i646, i1 false
  br i1 %i647, label %bb652, label %bb648

bb648:                                            ; preds = %bb636, %bb630, %bb622, %bb618, %bb613, %bb607, %bb598, %bb577
  %i649 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 1, !intel-tbaa !1022
  %i650 = load ptr, ptr %i649, align 8, !tbaa !1015
  %i651 = icmp eq ptr %i650, null
  br i1 %i651, label %bb656, label %bb566, !llvm.loop !1051

bb652:                                            ; preds = %bb636, %bb625, %bb622, %bb621, %bb607, %bb595, %bb585
  %i653 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 2
  %i654 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 0, !intel-tbaa !1024
  %i655 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i654, i64 0, i32 0
  store i8 0, ptr %i655, align 1, !tbaa !1024
  store ptr %i480, ptr %i653, align 8, !tbaa !1017
  br label %bb687

bb656:                                            ; preds = %bb648, %bb558
  %i657 = load ptr, ptr %i487, align 8, !tbaa !1050
  %i658 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i657, i64 0, i32 0
  %i659 = load ptr, ptr %i658, align 8, !tbaa !861
  %i660 = tail call i1 @llvm.type.test(ptr %i659, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i660)
  %i661 = getelementptr inbounds ptr, ptr %i659, i64 2
  %i662 = load ptr, ptr %i661, align 8
  %i663 = icmp eq ptr %i662, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i663, label %bb664, label %bb677

bb664:                                            ; preds = %bb656
  %i665 = invoke noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #39
          to label %bb679 unwind label %bb666

bb666:                                            ; preds = %bb664
  %i667 = landingpad { ptr, i32 }
          catch ptr null
  %i668 = extractvalue { ptr, i32 } %i667, 0
  %i669 = tail call ptr @__cxa_begin_catch(ptr %i668) #36
  %i670 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i670, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb676 unwind label %bb671

bb671:                                            ; preds = %bb666
  %i672 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb207 unwind label %bb673

bb673:                                            ; preds = %bb671
  %i674 = landingpad { ptr, i32 }
          catch ptr null
  %i675 = extractvalue { ptr, i32 } %i674, 0
  tail call fastcc void @__clang_call_terminate(ptr %i675) #37
  unreachable

bb676:                                            ; preds = %bb666
  unreachable

bb677:                                            ; preds = %bb656
  %i678 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb679:                                            ; preds = %bb664
  %i680 = load ptr, ptr %i560, align 8, !tbaa !1014
  %i681 = getelementptr inbounds ptr, ptr %i680, i64 %i562
  %i682 = load ptr, ptr %i681, align 8, !tbaa !1015
  %i683 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 0, !intel-tbaa !1024
  %i684 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i683, i64 0, i32 0
  store i8 0, ptr %i684, align 1, !tbaa !1024
  %i685 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 1, !intel-tbaa !1022
  store ptr %i682, ptr %i685, align 8, !tbaa !1022
  %i686 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 2, !intel-tbaa !1017
  store ptr %i480, ptr %i686, align 8, !tbaa !1017
  store ptr %i665, ptr %i681, align 8, !tbaa !1015
  br label %bb687

bb687:                                            ; preds = %bb679, %bb652
  ret void
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1052 hidden noundef signext i16 @_ZNK11xercesc_2_76IC_Key7getTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #21 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1054 hidden noundef signext i16 @_ZNK11xercesc_2_79IC_KeyRef7getTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #21 align 2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare !intel.dtrans.func.type !1056 dso_local i64 @strlen(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #22

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define hidden noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef writeonly "intel_dtrans_func_index"="2" %arg2, i32 noundef %arg3) unnamed_addr #23 align 2 !intel.dtrans.func.type !1057 {
bb:
  %i = zext i32 %arg3 to i64
  %i4 = getelementptr inbounds i16, ptr %arg2, i64 %i, !intel-tbaa !906
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %arg, i64 0, i32 1, !intel-tbaa !1059
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1059
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb89, label %bb8

bb8:                                              ; preds = %bb
  %i9 = load i16, ptr %i6, align 2, !tbaa !906
  %i10 = icmp eq i16 %i9, 104
  br i1 %i10, label %bb11, label %bb89

bb11:                                             ; preds = %bb16, %bb8
  %i12 = phi i16 [ %i19, %bb16 ], [ 104, %bb8 ]
  %i13 = phi ptr [ %i18, %bb16 ], [ @_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE, %bb8 ]
  %i14 = phi ptr [ %i17, %bb16 ], [ %i6, %bb8 ]
  %i15 = icmp eq i16 %i12, 0
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb11
  %i17 = getelementptr inbounds i16, ptr %i14, i64 1
  %i18 = getelementptr inbounds i16, ptr %i13, i64 1
  %i19 = load i16, ptr %i17, align 2, !tbaa !906
  %i20 = getelementptr [41 x i16], ptr %i18, i64 0, i64 0
  %i21 = load i16, ptr %i20, align 2, !tbaa !906
  %i22 = icmp eq i16 %i19, %i21
  br i1 %i22, label %bb11, label %bb29, !llvm.loop !1062

bb23:                                             ; preds = %bb11
  %i24 = icmp ugt i32 %arg1, 312
  br i1 %i24, label %bb107, label %bb25

bb25:                                             ; preds = %bb23
  %i26 = add nsw i32 %arg1, -1
  %i27 = zext i32 %i26 to i64
  %i28 = getelementptr inbounds [311 x [128 x i16]], ptr @_ZN11xercesc_2_7L12gXMLErrArrayE, i64 0, i64 %i27, i64 0
  br label %bb89

bb29:                                             ; preds = %bb16
  %i30 = icmp eq i16 %i9, 104
  br i1 %i30, label %bb31, label %bb89

bb31:                                             ; preds = %bb36, %bb29
  %i32 = phi i16 [ %i39, %bb36 ], [ 104, %bb29 ]
  %i33 = phi ptr [ %i38, %bb36 ], [ @_ZN11xercesc_2_76XMLUni14fgExceptDomainE, %bb29 ]
  %i34 = phi ptr [ %i37, %bb36 ], [ %i6, %bb29 ]
  %i35 = icmp eq i16 %i32, 0
  br i1 %i35, label %bb43, label %bb36

bb36:                                             ; preds = %bb31
  %i37 = getelementptr inbounds i16, ptr %i34, i64 1
  %i38 = getelementptr inbounds i16, ptr %i33, i64 1
  %i39 = load i16, ptr %i37, align 2, !tbaa !906
  %i40 = getelementptr [43 x i16], ptr %i38, i64 0, i64 0
  %i41 = load i16, ptr %i40, align 2, !tbaa !906
  %i42 = icmp eq i16 %i39, %i41
  br i1 %i42, label %bb31, label %bb49, !llvm.loop !1062

bb43:                                             ; preds = %bb31
  %i44 = icmp ugt i32 %arg1, 404
  br i1 %i44, label %bb107, label %bb45

bb45:                                             ; preds = %bb43
  %i46 = add nsw i32 %arg1, -1
  %i47 = zext i32 %i46 to i64
  %i48 = getelementptr inbounds [401 x [128 x i16]], ptr @_ZN11xercesc_2_7L15gXMLExceptArrayE, i64 0, i64 %i47, i64 0
  br label %bb89

bb49:                                             ; preds = %bb36
  %i50 = icmp eq i16 %i9, 104
  br i1 %i50, label %bb51, label %bb89

bb51:                                             ; preds = %bb56, %bb49
  %i52 = phi i16 [ %i59, %bb56 ], [ 104, %bb49 ]
  %i53 = phi ptr [ %i58, %bb56 ], [ @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, %bb49 ]
  %i54 = phi ptr [ %i57, %bb56 ], [ %i6, %bb49 ]
  %i55 = icmp eq i16 %i52, 0
  br i1 %i55, label %bb63, label %bb56

bb56:                                             ; preds = %bb51
  %i57 = getelementptr inbounds i16, ptr %i54, i64 1
  %i58 = getelementptr inbounds i16, ptr %i53, i64 1
  %i59 = load i16, ptr %i57, align 2, !tbaa !906
  %i60 = getelementptr [43 x i16], ptr %i58, i64 0, i64 0
  %i61 = load i16, ptr %i60, align 2, !tbaa !906
  %i62 = icmp eq i16 %i59, %i61
  br i1 %i62, label %bb51, label %bb69, !llvm.loop !1062

bb63:                                             ; preds = %bb51
  %i64 = icmp ugt i32 %arg1, 119
  br i1 %i64, label %bb107, label %bb65

bb65:                                             ; preds = %bb63
  %i66 = add nsw i32 %arg1, -1
  %i67 = zext i32 %i66 to i64
  %i68 = getelementptr inbounds [114 x [128 x i16]], ptr @_ZN11xercesc_2_7L17gXMLValidityArrayE, i64 0, i64 %i67, i64 0
  br label %bb89

bb69:                                             ; preds = %bb56
  %i70 = icmp eq i16 %i9, 104
  br i1 %i70, label %bb71, label %bb89

bb71:                                             ; preds = %bb76, %bb69
  %i72 = phi i16 [ %i79, %bb76 ], [ 104, %bb69 ]
  %i73 = phi ptr [ %i78, %bb76 ], [ @_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE, %bb69 ]
  %i74 = phi ptr [ %i77, %bb76 ], [ %i6, %bb69 ]
  %i75 = icmp eq i16 %i72, 0
  br i1 %i75, label %bb83, label %bb76

bb76:                                             ; preds = %bb71
  %i77 = getelementptr inbounds i16, ptr %i74, i64 1
  %i78 = getelementptr inbounds i16, ptr %i73, i64 1
  %i79 = load i16, ptr %i77, align 2, !tbaa !906
  %i80 = getelementptr [41 x i16], ptr %i78, i64 0, i64 0
  %i81 = load i16, ptr %i80, align 2, !tbaa !906
  %i82 = icmp eq i16 %i79, %i81
  br i1 %i82, label %bb71, label %bb89, !llvm.loop !1062

bb83:                                             ; preds = %bb71
  %i84 = icmp ugt i32 %arg1, 30
  br i1 %i84, label %bb107, label %bb85

bb85:                                             ; preds = %bb83
  %i86 = add nsw i32 %arg1, -1
  %i87 = zext i32 %i86 to i64
  %i88 = getelementptr inbounds [25 x [128 x i16]], ptr @_ZN11xercesc_2_7L15gXMLDOMMsgArrayE, i64 0, i64 %i87, i64 0
  br label %bb89

bb89:                                             ; preds = %bb85, %bb76, %bb69, %bb65, %bb49, %bb45, %bb29, %bb25, %bb8, %bb
  %i90 = phi ptr [ %i28, %bb25 ], [ %i48, %bb45 ], [ %i68, %bb65 ], [ %i88, %bb85 ], [ null, %bb69 ], [ null, %bb76 ], [ null, %bb49 ], [ null, %bb29 ], [ null, %bb8 ], [ null, %bb ]
  %i91 = load i16, ptr %i90, align 2, !tbaa !906
  %i92 = icmp ne i16 %i91, 0
  %i93 = icmp ne i32 %arg3, 0
  %i94 = and i1 %i92, %i93
  br i1 %i94, label %bb95, label %bb105

bb95:                                             ; preds = %bb95, %bb89
  %i96 = phi i16 [ %i101, %bb95 ], [ %i91, %bb89 ]
  %i97 = phi ptr [ %i99, %bb95 ], [ %i90, %bb89 ]
  %i98 = phi ptr [ %i100, %bb95 ], [ %arg2, %bb89 ]
  %i99 = getelementptr inbounds i16, ptr %i97, i64 1
  %i100 = getelementptr inbounds i16, ptr %i98, i64 1
  store i16 %i96, ptr %i98, align 2, !tbaa !906
  %i101 = load i16, ptr %i99, align 2, !tbaa !906
  %i102 = icmp ne i16 %i101, 0
  %i103 = icmp ult ptr %i100, %i4
  %i104 = select i1 %i102, i1 %i103, i1 false
  br i1 %i104, label %bb95, label %bb105, !llvm.loop !1063

bb105:                                            ; preds = %bb95, %bb89
  %i106 = phi ptr [ %arg2, %bb89 ], [ %i100, %bb95 ]
  store i16 0, ptr %i106, align 2, !tbaa !906
  br label %bb107

bb107:                                            ; preds = %bb105, %bb83, %bb63, %bb43, %bb23
  %i108 = phi i1 [ true, %bb105 ], [ false, %bb23 ], [ false, %bb43 ], [ false, %bb63 ], [ false, %bb83 ]
  ret i1 %i108
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1064 hidden void @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev(ptr nocapture nonnull align 1 "intel_dtrans_func_index"="1") unnamed_addr #24 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1066 hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2", i64 noundef) unnamed_addr #20 align 2

; Function Attrs: mustprogress norecurse nounwind uwtable
declare !intel.dtrans.func.type !1068 hidden void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #25 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define hidden fastcc noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg) unnamed_addr #19 align 2 !intel.dtrans.func.type !1069 {
bb:
  switch i32 %arg, label %bb8 [
    i32 0, label %bb9
    i32 1, label %bb1
    i32 2, label %bb2
    i32 3, label %bb3
    i32 4, label %bb4
    i32 5, label %bb5
    i32 6, label %bb6
    i32 8, label %bb7
  ]

bb1:                                              ; preds = %bb
  br label %bb9

bb2:                                              ; preds = %bb
  br label %bb9

bb3:                                              ; preds = %bb
  br label %bb9

bb4:                                              ; preds = %bb
  br label %bb9

bb5:                                              ; preds = %bb
  br label %bb9

bb6:                                              ; preds = %bb
  br label %bb9

bb7:                                              ; preds = %bb
  br label %bb9

bb8:                                              ; preds = %bb
  br label %bb9

bb9:                                              ; preds = %bb8, %bb7, %bb6, %bb5, %bb4, %bb3, %bb2, %bb1, %bb
  %i = phi ptr [ @.str.8.1692, %bb8 ], [ @.str.7.1693, %bb7 ], [ @.str.6.1694, %bb6 ], [ @.str.5.1695, %bb5 ], [ @.str.4.1696, %bb4 ], [ @.str.3.1697, %bb3 ], [ @.str.2.1698, %bb2 ], [ @.str.1.1699, %bb1 ], [ @.str.1700, %bb ]
  ret ptr %i
}

; Function Attrs: uwtable
define hidden fastcc noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef "intel_dtrans_func_index"="2" %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1070 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  %i1 = invoke fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 16, ptr noundef %i)
          to label %bb2 unwind label %bb107

bb2:                                              ; preds = %bb
  %i3 = getelementptr %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %i1, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([8 x ptr], ptr @_ZTVN11xercesc_2_714InMemMsgLoaderE.0, i64 0, i64 2), ptr %i3, align 8, !tbaa !861
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %i1, i64 0, i32 1, !intel-tbaa !1059
  store ptr null, ptr %i4, align 8, !tbaa !1059
  %i5 = icmp eq ptr %arg, null
  br i1 %i5, label %bb63, label %bb6

bb6:                                              ; preds = %bb2
  %i7 = load i16, ptr %arg, align 2, !tbaa !906
  %i8 = icmp eq i16 %i7, 104
  br i1 %i8, label %bb9, label %bb63

bb9:                                              ; preds = %bb14, %bb6
  %i10 = phi i16 [ %i17, %bb14 ], [ 104, %bb6 ]
  %i11 = phi ptr [ %i16, %bb14 ], [ @_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE, %bb6 ]
  %i12 = phi ptr [ %i15, %bb14 ], [ %arg, %bb6 ]
  %i13 = icmp eq i16 %i10, 0
  br i1 %i13, label %bb68, label %bb14

bb14:                                             ; preds = %bb9
  %i15 = getelementptr inbounds i16, ptr %i12, i64 1
  %i16 = getelementptr inbounds i16, ptr %i11, i64 1
  %i17 = load i16, ptr %i15, align 2, !tbaa !906
  %i18 = getelementptr [41 x i16], ptr %i16, i64 0, i64 0
  %i19 = load i16, ptr %i18, align 2, !tbaa !906
  %i20 = icmp eq i16 %i17, %i19
  br i1 %i20, label %bb9, label %bb21, !llvm.loop !1062

bb21:                                             ; preds = %bb14
  %i22 = icmp eq i16 %i7, 104
  br i1 %i22, label %bb23, label %bb63

bb23:                                             ; preds = %bb28, %bb21
  %i24 = phi i16 [ %i31, %bb28 ], [ 104, %bb21 ]
  %i25 = phi ptr [ %i30, %bb28 ], [ @_ZN11xercesc_2_76XMLUni14fgExceptDomainE, %bb21 ]
  %i26 = phi ptr [ %i29, %bb28 ], [ %arg, %bb21 ]
  %i27 = icmp eq i16 %i24, 0
  br i1 %i27, label %bb68, label %bb28

bb28:                                             ; preds = %bb23
  %i29 = getelementptr inbounds i16, ptr %i26, i64 1
  %i30 = getelementptr inbounds i16, ptr %i25, i64 1
  %i31 = load i16, ptr %i29, align 2, !tbaa !906
  %i32 = getelementptr [43 x i16], ptr %i30, i64 0, i64 0
  %i33 = load i16, ptr %i32, align 2, !tbaa !906
  %i34 = icmp eq i16 %i31, %i33
  br i1 %i34, label %bb23, label %bb35, !llvm.loop !1062

bb35:                                             ; preds = %bb28
  %i36 = icmp eq i16 %i7, 104
  br i1 %i36, label %bb37, label %bb63

bb37:                                             ; preds = %bb42, %bb35
  %i38 = phi i16 [ %i45, %bb42 ], [ 104, %bb35 ]
  %i39 = phi ptr [ %i44, %bb42 ], [ @_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE, %bb35 ]
  %i40 = phi ptr [ %i43, %bb42 ], [ %arg, %bb35 ]
  %i41 = icmp eq i16 %i38, 0
  br i1 %i41, label %bb68, label %bb42

bb42:                                             ; preds = %bb37
  %i43 = getelementptr inbounds i16, ptr %i40, i64 1
  %i44 = getelementptr inbounds i16, ptr %i39, i64 1
  %i45 = load i16, ptr %i43, align 2, !tbaa !906
  %i46 = getelementptr [41 x i16], ptr %i44, i64 0, i64 0
  %i47 = load i16, ptr %i46, align 2, !tbaa !906
  %i48 = icmp eq i16 %i45, %i47
  br i1 %i48, label %bb37, label %bb49, !llvm.loop !1062

bb49:                                             ; preds = %bb42
  %i50 = icmp eq i16 %i7, 104
  br i1 %i50, label %bb51, label %bb63

bb51:                                             ; preds = %bb56, %bb49
  %i52 = phi i16 [ %i59, %bb56 ], [ 104, %bb49 ]
  %i53 = phi ptr [ %i58, %bb56 ], [ @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, %bb49 ]
  %i54 = phi ptr [ %i57, %bb56 ], [ %arg, %bb49 ]
  %i55 = icmp eq i16 %i52, 0
  br i1 %i55, label %bb68, label %bb56

bb56:                                             ; preds = %bb51
  %i57 = getelementptr inbounds i16, ptr %i54, i64 1
  %i58 = getelementptr inbounds i16, ptr %i53, i64 1
  %i59 = load i16, ptr %i57, align 2, !tbaa !906
  %i60 = getelementptr [43 x i16], ptr %i58, i64 0, i64 0
  %i61 = load i16, ptr %i60, align 2, !tbaa !906
  %i62 = icmp eq i16 %i59, %i61
  br i1 %i62, label %bb51, label %bb63, !llvm.loop !1062

bb63:                                             ; preds = %bb56, %bb49, %bb35, %bb21, %bb6, %bb2
  %i64 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1071
  %i65 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i64, i64 0, i32 0
  %i66 = load ptr, ptr %i65, align 8, !tbaa !861
  %i67 = tail call i1 @llvm.type.test(ptr %i66, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i67)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 3), !intel_dtrans_type !1073
  unreachable

bb68:                                             ; preds = %bb51, %bb37, %bb23, %bb9
  %i69 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  %i70 = load i16, ptr %arg, align 2, !tbaa !906
  %i71 = icmp eq i16 %i70, 0
  br i1 %i71, label %bb83, label %bb72

bb72:                                             ; preds = %bb72, %bb68
  %i73 = phi ptr [ %i74, %bb72 ], [ %arg, %bb68 ]
  %i74 = getelementptr inbounds i16, ptr %i73, i64 1
  %i75 = load i16, ptr %i74, align 2, !tbaa !906
  %i76 = icmp eq i16 %i75, 0
  br i1 %i76, label %bb77, label %bb72, !llvm.loop !1074

bb77:                                             ; preds = %bb72
  %i78 = ptrtoint ptr %i74 to i64
  %i79 = ptrtoint ptr %arg to i64
  %i80 = sub i64 2, %i79
  %i81 = add i64 %i80, %i78
  %i82 = and i64 %i81, 8589934590
  br label %bb83

bb83:                                             ; preds = %bb77, %bb68
  %i84 = phi i64 [ %i82, %bb77 ], [ 2, %bb68 ]
  %i85 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i69, i64 0, i32 0
  %i86 = load ptr, ptr %i85, align 8, !tbaa !861
  %i87 = tail call i1 @llvm.type.test(ptr %i86, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i87)
  %i88 = getelementptr inbounds ptr, ptr %i86, i64 2
  %i89 = load ptr, ptr %i88, align 8
  %i90 = icmp eq ptr %i89, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i90, label %bb91, label %bb104

bb91:                                             ; preds = %bb83
  %i92 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i84) #39
          to label %bb148 unwind label %bb93

bb93:                                             ; preds = %bb91
  %i94 = landingpad { ptr, i32 }
          catch ptr null
  %i95 = extractvalue { ptr, i32 } %i94, 0
  %i96 = tail call ptr @__cxa_begin_catch(ptr %i95) #36
  %i97 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i97, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb103 unwind label %bb98

bb98:                                             ; preds = %bb93
  %i99 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  invoke void @__cxa_end_catch()
          to label %bb111 unwind label %bb100

bb100:                                            ; preds = %bb98
  %i101 = landingpad { ptr, i32 }
          catch ptr null
  %i102 = extractvalue { ptr, i32 } %i101, 0
  tail call fastcc void @__clang_call_terminate(ptr %i102) #37
  unreachable

bb103:                                            ; preds = %bb93
  unreachable

bb104:                                            ; preds = %bb83
  %i105 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb106 unwind label %bb109, !intel_dtrans_type !877

bb106:                                            ; preds = %bb104
  unreachable

bb107:                                            ; preds = %bb
  %i108 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  br label %bb128

bb109:                                            ; preds = %bb104
  %i110 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  br label %bb111

bb111:                                            ; preds = %bb109, %bb98
  %i112 = phi { ptr, i32 } [ %i110, %bb109 ], [ %i99, %bb98 ]
  %i113 = icmp eq ptr %i1, null
  br i1 %i113, label %bb128, label %bb114

bb114:                                            ; preds = %bb111
  %i115 = getelementptr inbounds i8, ptr %i1, i64 -8, !intel-tbaa !927
  %i116 = load ptr, ptr %i115, align 8, !tbaa !876
  %i117 = load ptr, ptr %i116, align 8, !tbaa !861
  %i118 = tail call i1 @llvm.type.test(ptr %i117, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i118)
  %i119 = getelementptr inbounds ptr, ptr %i117, i64 3
  %i120 = load ptr, ptr %i119, align 8
  %i121 = icmp eq ptr %i120, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i121, label %bb122, label %bb123

bb122:                                            ; preds = %bb114
  tail call void @_ZdlPv(ptr noundef nonnull %i115) #36
  br label %bb128

bb123:                                            ; preds = %bb114
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb124 unwind label %bb125, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb124:                                            ; preds = %bb123
  unreachable

bb125:                                            ; preds = %bb123
  %i126 = landingpad { ptr, i32 }
          catch ptr null
  %i127 = extractvalue { ptr, i32 } %i126, 0
  tail call fastcc void @__clang_call_terminate(ptr %i127) #37
  unreachable

bb128:                                            ; preds = %bb122, %bb111, %bb107
  %i129 = phi { ptr, i32 } [ %i108, %bb107 ], [ %i112, %bb111 ], [ %i112, %bb122 ]
  %i130 = extractvalue { ptr, i32 } %i129, 1
  %i131 = extractvalue { ptr, i32 } %i129, 0
  %i132 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #36
  %i133 = icmp eq i32 %i130, %i132
  %i134 = tail call ptr @__cxa_begin_catch(ptr %i131) #36
  br i1 %i133, label %bb135, label %bb136

bb135:                                            ; preds = %bb128
  invoke void @__cxa_rethrow() #40
          to label %bb147 unwind label %bb141

bb136:                                            ; preds = %bb128
  %i137 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1071
  %i138 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i137, i64 0, i32 0
  %i139 = load ptr, ptr %i138, align 8, !tbaa !861
  %i140 = tail call i1 @llvm.type.test(ptr %i139, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i140)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1073
  unreachable

bb141:                                            ; preds = %bb135
  %i142 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb143 unwind label %bb144

bb143:                                            ; preds = %bb141
  resume { ptr, i32 } %i142

bb144:                                            ; preds = %bb141
  %i145 = landingpad { ptr, i32 }
          catch ptr null
  %i146 = extractvalue { ptr, i32 } %i145, 0
  tail call fastcc void @__clang_call_terminate(ptr %i146) #37
  unreachable

bb147:                                            ; preds = %bb135
  unreachable

bb148:                                            ; preds = %bb91
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i92, ptr nonnull align 2 %arg, i64 %i84, i1 false)
  store ptr %i92, ptr %i4, align 8, !tbaa !1059
  ret ptr %i1
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="2" %arg, ptr nocapture noundef nonnull writeonly align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %arg1) unnamed_addr #26 align 2 !intel.dtrans.func.type !1075 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 2, !intel-tbaa !1076
  %i2 = load ptr, ptr %i, align 8, !tbaa !1076
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 1, !intel-tbaa !1077
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1077
  %i5 = icmp eq ptr %i4, null
  br i1 %i5, label %bb51, label %bb6

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i4, i64 0, i32 6, !intel-tbaa !1078
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1078
  %i9 = icmp ne ptr %i8, null
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i4, i64 0, i32 7
  %i11 = load ptr, ptr %i10, align 8
  %i12 = icmp ne ptr %i11, null
  %i13 = select i1 %i9, i1 true, i1 %i12
  br i1 %i13, label %bb51, label %bb14

bb14:                                             ; preds = %bb6
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 6, !intel-tbaa !1080
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1080
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf", ptr %i16, i64 0, i32 1, !intel-tbaa !1081
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i17, i64 0, i32 2, !intel-tbaa !1086
  %i19 = load i32, ptr %i18, align 4, !tbaa !1086
  %i20 = icmp eq i32 %i19, 0
  br i1 %i20, label %bb51, label %bb21

bb21:                                             ; preds = %bb14
  %i22 = add i32 %i19, -1
  %i23 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 4, !intel-tbaa !1087
  %i24 = load ptr, ptr %i23, align 8, !tbaa !1087
  %i25 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i24, i32 noundef %i22)
  %i26 = icmp eq ptr %i25, null
  br i1 %i26, label %bb27, label %bb31

bb27:                                             ; preds = %bb46, %bb21
  %i28 = phi i32 [ %i22, %bb21 ], [ %i47, %bb46 ]
  %i29 = load ptr, ptr %i15, align 8, !tbaa !1080
  %i30 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i29, i32 noundef %i28)
  br label %bb51

bb31:                                             ; preds = %bb46, %bb21
  %i32 = phi ptr [ %i49, %bb46 ], [ %i25, %bb21 ]
  %i33 = phi i32 [ %i47, %bb46 ], [ %i22, %bb21 ]
  %i34 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i32, i64 0, i32 6, !intel-tbaa !1078
  %i35 = load ptr, ptr %i34, align 8, !tbaa !1078
  %i36 = icmp ne ptr %i35, null
  %i37 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i32, i64 0, i32 7
  %i38 = load ptr, ptr %i37, align 8
  %i39 = icmp ne ptr %i38, null
  %i40 = select i1 %i36, i1 true, i1 %i39
  br i1 %i40, label %bb41, label %bb44

bb41:                                             ; preds = %bb31
  %i42 = load ptr, ptr %i15, align 8, !tbaa !1080
  %i43 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i42, i32 noundef %i33)
  br label %bb51

bb44:                                             ; preds = %bb31
  %i45 = icmp eq i32 %i33, 0
  br i1 %i45, label %bb51, label %bb46, !llvm.loop !1088

bb46:                                             ; preds = %bb44
  %i47 = add i32 %i33, -1
  %i48 = load ptr, ptr %i23, align 8, !tbaa !1087
  %i49 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i48, i32 noundef %i47)
  %i50 = icmp eq ptr %i49, null
  br i1 %i50, label %bb27, label %bb31

bb51:                                             ; preds = %bb44, %bb41, %bb27, %bb14, %bb6, %bb
  %i52 = phi ptr [ %i2, %bb6 ], [ %i2, %bb ], [ %i43, %bb41 ], [ %i30, %bb27 ], [ %i2, %bb14 ], [ %i2, %bb44 ]
  %i53 = phi ptr [ %i4, %bb6 ], [ null, %bb ], [ %i32, %bb41 ], [ null, %bb27 ], [ %i4, %bb14 ], [ %i32, %bb44 ]
  store ptr %i53, ptr %arg1, align 8, !tbaa !1089
  ret ptr %i52
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #26 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1090 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1091
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 2, !intel-tbaa !1096
  %i3 = load i32, ptr %i2, align 4, !tbaa !1096
  %i4 = icmp ult i32 %i3, %arg1
  br i1 %i4, label %bb5, label %bb14

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1097
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1097
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1798, i32 noundef 55, i32 noundef 79, ptr noundef %i8)
          to label %bb9 unwind label %bb12

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb10:                                             ; preds = %bb21, %bb12
  %i11 = phi { ptr, i32 } [ %i13, %bb12 ], [ %i22, %bb21 ]
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb5
  %i13 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #36
  br label %bb10

bb14:                                             ; preds = %bb
  %i15 = icmp ugt i32 %i3, %arg1
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb14
  %i17 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1097
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1097
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i17, ptr noundef nonnull @.str.4452, i32 noundef 241, i32 noundef 116, ptr noundef %i19)
          to label %bb20 unwind label %bb21

bb20:                                             ; preds = %bb16
  tail call void @__cxa_throw(ptr nonnull %i17, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb21:                                             ; preds = %bb16
  %i22 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i17) #36
  br label %bb10

bb23:                                             ; preds = %bb14
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 4, !intel-tbaa !1098
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1098
  %i26 = zext i32 %arg1 to i64
  %i27 = getelementptr inbounds ptr, ptr %i25, i64 %i26
  %i28 = load ptr, ptr %i27, align 8, !tbaa !1089
  ret ptr %i28
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #26 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1099 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1081
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 2, !intel-tbaa !1086
  %i3 = load i32, ptr %i2, align 4, !tbaa !1086
  %i4 = icmp ult i32 %i3, %arg1
  br i1 %i4, label %bb5, label %bb14

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1100
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1100
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1798, i32 noundef 55, i32 noundef 79, ptr noundef %i8)
          to label %bb9 unwind label %bb12

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb10:                                             ; preds = %bb21, %bb12
  %i11 = phi { ptr, i32 } [ %i13, %bb12 ], [ %i22, %bb21 ]
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb5
  %i13 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #36
  br label %bb10

bb14:                                             ; preds = %bb
  %i15 = icmp ugt i32 %i3, %arg1
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb14
  %i17 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1100
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1100
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i17, ptr noundef nonnull @.str.4452, i32 noundef 241, i32 noundef 116, ptr noundef %i19)
          to label %bb20 unwind label %bb21

bb20:                                             ; preds = %bb16
  tail call void @__cxa_throw(ptr nonnull %i17, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb21:                                             ; preds = %bb16
  %i22 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i17) #36
  br label %bb10

bb23:                                             ; preds = %bb14
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 4, !intel-tbaa !1101
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1101
  %i26 = zext i32 %arg1 to i64
  %i27 = getelementptr inbounds ptr, ptr %i25, i64 %i26
  %i28 = load ptr, ptr %i27, align 8, !tbaa !1102
  ret ptr %i28
}

; Function Attrs: inlinehint mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #27 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1103 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1028
  %i2 = load ptr, ptr %i, align 8, !tbaa !1028
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 1, !intel-tbaa !1029
  %i6 = load i32, ptr %i5, align 4, !tbaa !1029
  %i7 = icmp ugt i32 %i6, %arg1
  br i1 %i7, label %bb15, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 4, !intel-tbaa !1032
  %i11 = load ptr, ptr %i10, align 8, !tbaa !1032
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i9, ptr noundef nonnull @.str.6.5756, i32 noundef 206, i32 noundef 116, ptr noundef %i11)
          to label %bb12 unwind label %bb13

bb12:                                             ; preds = %bb8
  tail call void @__cxa_throw(ptr nonnull %i9, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb13:                                             ; preds = %bb8
  %i14 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i9) #36
  resume { ptr, i32 } %i14

bb15:                                             ; preds = %bb4
  %i16 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 3, !intel-tbaa !1033
  %i17 = load ptr, ptr %i16, align 8, !tbaa !1033
  %i18 = zext i32 %arg1 to i64
  %i19 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i17, i64 %i18, i32 1
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1034
  br label %bb21

bb21:                                             ; preds = %bb15, %bb
  %i22 = phi ptr [ %i20, %bb15 ], [ null, %bb ]
  ret ptr %i22
}

; Function Attrs: inlinehint mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #27 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1104 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !897
  %i2 = load ptr, ptr %i, align 8, !tbaa !897
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 1, !intel-tbaa !899
  %i6 = load i32, ptr %i5, align 4, !tbaa !899
  %i7 = icmp ugt i32 %i6, %arg1
  br i1 %i7, label %bb15, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 4, !intel-tbaa !903
  %i11 = load ptr, ptr %i10, align 8, !tbaa !903
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i9, ptr noundef nonnull @.str.4452, i32 noundef 249, i32 noundef 116, ptr noundef %i11)
          to label %bb12 unwind label %bb13

bb12:                                             ; preds = %bb8
  tail call void @__cxa_throw(ptr nonnull %i9, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb13:                                             ; preds = %bb8
  %i14 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i9) #36
  resume { ptr, i32 } %i14

bb15:                                             ; preds = %bb4
  %i16 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 3, !intel-tbaa !904
  %i17 = load ptr, ptr %i16, align 8, !tbaa !904
  %i18 = zext i32 %arg1 to i64
  %i19 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i17, i64 %i18, i32 2
  %i20 = load ptr, ptr %i19, align 8, !tbaa !905
  br label %bb21

bb21:                                             ; preds = %bb15, %bb
  %i22 = phi ptr [ %i20, %bb15 ], [ null, %bb ]
  ret ptr %i22
}

; Function Attrs: inlinehint uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3) local_unnamed_addr #28 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1105 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !892
  %i4 = load ptr, ptr %i, align 8, !tbaa !892
  %i5 = icmp eq ptr %i4, null
  br i1 %i5, label %bb6, label %bb12

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !898
  %i8 = load ptr, ptr %i7, align 8, !tbaa !898
  %i9 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i8)
  %i10 = load ptr, ptr %i7, align 8, !tbaa !898
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(32) %i9, i32 poison, ptr noundef %i10, i1 zeroext poison)
          to label %bb11 unwind label %bb151

bb11:                                             ; preds = %bb6
  store ptr %i9, ptr %i, align 8, !tbaa !892
  br label %bb12

bb12:                                             ; preds = %bb11, %bb
  %i13 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !892
  %i14 = load ptr, ptr %i13, align 8, !tbaa !892
  %i15 = icmp eq ptr %i14, null
  br i1 %i15, label %bb31, label %bb16

bb16:                                             ; preds = %bb12
  %i17 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i14, i64 0, i32 1, !intel-tbaa !918
  %i18 = load i32, ptr %i17, align 4, !tbaa !918
  %i19 = icmp eq i32 %i18, 0
  br i1 %i19, label %bb31, label %bb20

bb20:                                             ; preds = %bb26, %bb16
  %i21 = phi i32 [ %i27, %bb26 ], [ 0, %bb16 ]
  %i22 = load ptr, ptr %i13, align 8, !tbaa !892
  %i23 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i22, i32 noundef %i21)
  %i24 = load ptr, ptr %i23, align 8, !tbaa !922
  %i25 = icmp eq ptr %i24, %arg1
  br i1 %i25, label %bb29, label %bb26

bb26:                                             ; preds = %bb20
  %i27 = add nuw i32 %i21, 1
  %i28 = icmp eq i32 %i27, %i18
  br i1 %i28, label %bb31, label %bb20, !llvm.loop !1027

bb29:                                             ; preds = %bb20
  %i30 = icmp eq i32 %i21, -1
  br i1 %i30, label %bb31, label %bb87

bb31:                                             ; preds = %bb29, %bb26, %bb16, %bb12
  %i32 = load ptr, ptr %i, align 8, !tbaa !892
  %i33 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !898
  %i34 = load ptr, ptr %i33, align 8, !tbaa !898
  %i35 = icmp eq ptr %arg3, null
  br i1 %i35, label %bb76, label %bb36

bb36:                                             ; preds = %bb31
  %i37 = load i16, ptr %arg3, align 2, !tbaa !906
  %i38 = icmp eq i16 %i37, 0
  br i1 %i38, label %bb50, label %bb39

bb39:                                             ; preds = %bb39, %bb36
  %i40 = phi ptr [ %i41, %bb39 ], [ %arg3, %bb36 ]
  %i41 = getelementptr inbounds i16, ptr %i40, i64 1
  %i42 = load i16, ptr %i41, align 2, !tbaa !906
  %i43 = icmp eq i16 %i42, 0
  br i1 %i43, label %bb44, label %bb39, !llvm.loop !908

bb44:                                             ; preds = %bb39
  %i45 = ptrtoint ptr %i41 to i64
  %i46 = ptrtoint ptr %arg3 to i64
  %i47 = sub i64 2, %i46
  %i48 = add i64 %i47, %i45
  %i49 = and i64 %i48, 8589934590
  br label %bb50

bb50:                                             ; preds = %bb44, %bb36
  %i51 = phi i64 [ %i49, %bb44 ], [ 2, %bb36 ]
  %i52 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i34, i64 0, i32 0
  %i53 = load ptr, ptr %i52, align 8, !tbaa !861
  %i54 = tail call i1 @llvm.type.test(ptr %i53, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i54)
  %i55 = getelementptr inbounds ptr, ptr %i53, i64 2
  %i56 = load ptr, ptr %i55, align 8
  %i57 = icmp eq ptr %i56, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i57, label %bb58, label %bb74

bb58:                                             ; preds = %bb50
  %i59 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i51) #39
          to label %bb73 unwind label %bb60

bb60:                                             ; preds = %bb58
  %i61 = landingpad { ptr, i32 }
          catch ptr null
  %i62 = extractvalue { ptr, i32 } %i61, 0
  %i63 = tail call ptr @__cxa_begin_catch(ptr %i62) #36
  %i64 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i64, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb72 unwind label %bb65

bb65:                                             ; preds = %bb60
  %i66 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb67 unwind label %bb69

bb67:                                             ; preds = %bb151, %bb139, %bb98, %bb65
  %i68 = phi { ptr, i32 } [ %i99, %bb98 ], [ %i152, %bb151 ], [ %i66, %bb65 ], [ %i140, %bb139 ]
  resume { ptr, i32 } %i68

bb69:                                             ; preds = %bb65
  %i70 = landingpad { ptr, i32 }
          catch ptr null
  %i71 = extractvalue { ptr, i32 } %i70, 0
  tail call fastcc void @__clang_call_terminate(ptr %i71) #37
  unreachable

bb72:                                             ; preds = %bb60
  unreachable

bb73:                                             ; preds = %bb58
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i59, ptr nonnull align 2 %arg3, i64 %i51, i1 false)
  br label %bb76

bb74:                                             ; preds = %bb50
  %i75 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb76:                                             ; preds = %bb73, %bb31
  %i77 = phi ptr [ %i59, %bb73 ], [ null, %bb31 ]
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr noundef nonnull align 8 dereferenceable(32) %i32, i32 poison)
  %i78 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i32, i64 0, i32 3, !intel-tbaa !920
  %i79 = load ptr, ptr %i78, align 8, !tbaa !920
  %i80 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i32, i64 0, i32 1, !intel-tbaa !918
  %i81 = load i32, ptr %i80, align 4, !tbaa !918
  %i82 = zext i32 %i81 to i64
  %i83 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 0
  %i84 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 2
  %i85 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 1
  store ptr %arg2, ptr %i85, align 8
  store ptr %i77, ptr %i84, align 8
  store ptr %arg1, ptr %i83, align 8, !tbaa !922
  %i86 = add i32 %i81, 1
  store i32 %i86, ptr %i80, align 4, !tbaa !918
  br label %bb150

bb87:                                             ; preds = %bb29
  %i88 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1028
  %i89 = load ptr, ptr %i88, align 8, !tbaa !1028
  %i90 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 1, !intel-tbaa !1029
  %i91 = load i32, ptr %i90, align 4, !tbaa !1029
  %i92 = icmp ugt i32 %i91, %i21
  br i1 %i92, label %bb100, label %bb93

bb93:                                             ; preds = %bb87
  %i94 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i95 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 4, !intel-tbaa !1032
  %i96 = load ptr, ptr %i95, align 8, !tbaa !1032
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i94, ptr noundef nonnull @.str.6.5756, i32 noundef 126, i32 noundef 116, ptr noundef %i96)
          to label %bb97 unwind label %bb98

bb97:                                             ; preds = %bb93
  tail call void @__cxa_throw(ptr nonnull %i94, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb98:                                             ; preds = %bb93
  %i99 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i94) #36
  br label %bb67

bb100:                                            ; preds = %bb87
  %i101 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 3, !intel-tbaa !1033
  %i102 = load ptr, ptr %i101, align 8, !tbaa !1033
  %i103 = zext i32 %i21 to i64
  %i104 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i102, i64 %i103, i32 1
  store ptr %arg2, ptr %i104, align 8, !tbaa !1034
  %i105 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !897
  %i106 = load ptr, ptr %i105, align 8, !tbaa !897
  %i107 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !898
  %i108 = load ptr, ptr %i107, align 8, !tbaa !898
  %i109 = icmp eq ptr %arg3, null
  br i1 %i109, label %bb148, label %bb110

bb110:                                            ; preds = %bb100
  %i111 = load i16, ptr %arg3, align 2, !tbaa !906
  %i112 = icmp eq i16 %i111, 0
  br i1 %i112, label %bb124, label %bb113

bb113:                                            ; preds = %bb113, %bb110
  %i114 = phi ptr [ %i115, %bb113 ], [ %arg3, %bb110 ]
  %i115 = getelementptr inbounds i16, ptr %i114, i64 1
  %i116 = load i16, ptr %i115, align 2, !tbaa !906
  %i117 = icmp eq i16 %i116, 0
  br i1 %i117, label %bb118, label %bb113, !llvm.loop !908

bb118:                                            ; preds = %bb113
  %i119 = ptrtoint ptr %i115 to i64
  %i120 = ptrtoint ptr %arg3 to i64
  %i121 = sub i64 2, %i120
  %i122 = add i64 %i121, %i119
  %i123 = and i64 %i122, 8589934590
  br label %bb124

bb124:                                            ; preds = %bb118, %bb110
  %i125 = phi i64 [ %i123, %bb118 ], [ 2, %bb110 ]
  %i126 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i108, i64 0, i32 0
  %i127 = load ptr, ptr %i126, align 8, !tbaa !861
  %i128 = tail call i1 @llvm.type.test(ptr %i127, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i128)
  %i129 = getelementptr inbounds ptr, ptr %i127, i64 2
  %i130 = load ptr, ptr %i129, align 8
  %i131 = icmp eq ptr %i130, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i131, label %bb132, label %bb146

bb132:                                            ; preds = %bb124
  %i133 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i125) #39
          to label %bb145 unwind label %bb134

bb134:                                            ; preds = %bb132
  %i135 = landingpad { ptr, i32 }
          catch ptr null
  %i136 = extractvalue { ptr, i32 } %i135, 0
  %i137 = tail call ptr @__cxa_begin_catch(ptr %i136) #36
  %i138 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i138, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb144 unwind label %bb139

bb139:                                            ; preds = %bb134
  %i140 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb67 unwind label %bb141

bb141:                                            ; preds = %bb139
  %i142 = landingpad { ptr, i32 }
          catch ptr null
  %i143 = extractvalue { ptr, i32 } %i142, 0
  tail call fastcc void @__clang_call_terminate(ptr %i143) #37
  unreachable

bb144:                                            ; preds = %bb134
  unreachable

bb145:                                            ; preds = %bb132
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i133, ptr nonnull align 2 %arg3, i64 %i125, i1 false)
  br label %bb148

bb146:                                            ; preds = %bb124
  %i147 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb148:                                            ; preds = %bb145, %bb100
  %i149 = phi ptr [ %i133, %bb145 ], [ null, %bb100 ]
  tail call void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831(ptr noundef nonnull align 8 dereferenceable(40) %i106, ptr noundef %i149, i32 noundef %i21), !intel_dtrans_type !1106
  br label %bb150

bb150:                                            ; preds = %bb148, %bb76
  ret void

bb151:                                            ; preds = %bb6
  %i152 = landingpad { ptr, i32 }
          cleanup
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i9) #36
  br label %bb67
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1) unnamed_addr #26 align 2 !intel.dtrans.func.type !1107 {
bb:
  %i = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %arg, i64 0, i32 4, !intel-tbaa !1038
  %i2 = load ptr, ptr %i, align 8, !tbaa !1038
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb130, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !892
  %i6 = load ptr, ptr %i5, align 8, !tbaa !892
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb11, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i6, i64 0, i32 1, !intel-tbaa !918
  %i10 = load i32, ptr %i9, align 4, !tbaa !918
  br label %bb11

bb11:                                             ; preds = %bb8, %bb4
  %i12 = phi i32 [ %i10, %bb8 ], [ 0, %bb4 ]
  %i13 = load ptr, ptr %i, align 8, !tbaa !1038
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %i13, i64 0, i32 2, !intel-tbaa !1043
  %i15 = load i32, ptr %i14, align 4, !tbaa !1043
  %i16 = icmp eq i32 %i15, 0
  br i1 %i16, label %bb130, label %bb17

bb17:                                             ; preds = %bb11
  %i18 = icmp eq i32 %i12, 0
  br i1 %i18, label %bb116, label %bb19

bb19:                                             ; preds = %bb113, %bb17
  %i20 = phi i32 [ %i114, %bb113 ], [ 0, %bb17 ]
  %i21 = load ptr, ptr %i, align 8, !tbaa !1038
  %i22 = tail call fastcc noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i21, i32 noundef %i20)
  %i23 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i22, i64 0, i32 0, !intel-tbaa !892
  %i24 = load ptr, ptr %i23, align 8, !tbaa !892
  %i25 = icmp eq ptr %i24, null
  br i1 %i25, label %bb29, label %bb26

bb26:                                             ; preds = %bb19
  %i27 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i24, i64 0, i32 1, !intel-tbaa !918
  %i28 = load i32, ptr %i27, align 4, !tbaa !918
  br label %bb29

bb29:                                             ; preds = %bb26, %bb19
  %i30 = phi i32 [ %i28, %bb26 ], [ 0, %bb19 ]
  %i31 = icmp eq i32 %i12, %i30
  br i1 %i31, label %bb35, label %bb113

bb32:                                             ; preds = %bb111, %bb103, %bb95, %bb62, %bb59, %bb58
  %i33 = add nuw i32 %i36, 1
  %i34 = icmp eq i32 %i33, %i12
  br i1 %i34, label %bb130, label %bb35, !llvm.loop !1108

bb35:                                             ; preds = %bb32, %bb29
  %i36 = phi i32 [ %i33, %bb32 ], [ 0, %bb29 ]
  %i37 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i22, i32 noundef %i36)
  %i38 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i22, i32 noundef %i36)
  %i39 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i36)
  %i40 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i36)
  %i41 = getelementptr i8, ptr %arg, i64 56
  %i42 = load ptr, ptr %i41, align 8
  %i43 = icmp ne ptr %i37, null
  %i44 = icmp ne ptr %i39, null
  %i45 = and i1 %i43, %i44
  %i46 = icmp eq ptr %i38, null
  br i1 %i45, label %bb73, label %bb47

bb47:                                             ; preds = %bb35
  %i48 = icmp eq ptr %i40, null
  %i49 = or i1 %i46, %i48
  br i1 %i49, label %bb54, label %bb50

bb50:                                             ; preds = %bb47
  %i51 = load i16, ptr %i38, align 2, !tbaa !906
  %i52 = load i16, ptr %i40, align 2, !tbaa !906
  %i53 = icmp eq i16 %i51, %i52
  br i1 %i53, label %bb62, label %bb113

bb54:                                             ; preds = %bb47
  br i1 %i46, label %bb58, label %bb55

bb55:                                             ; preds = %bb54
  %i56 = load i16, ptr %i38, align 2, !tbaa !906
  %i57 = icmp eq i16 %i56, 0
  br i1 %i57, label %bb58, label %bb113

bb58:                                             ; preds = %bb55, %bb54
  br i1 %i48, label %bb32, label %bb59

bb59:                                             ; preds = %bb58
  %i60 = load i16, ptr %i40, align 2, !tbaa !906
  %i61 = icmp eq i16 %i60, 0
  br i1 %i61, label %bb32, label %bb113

bb62:                                             ; preds = %bb67, %bb50
  %i63 = phi i16 [ %i70, %bb67 ], [ %i51, %bb50 ]
  %i64 = phi ptr [ %i69, %bb67 ], [ %i40, %bb50 ]
  %i65 = phi ptr [ %i68, %bb67 ], [ %i38, %bb50 ]
  %i66 = icmp eq i16 %i63, 0
  br i1 %i66, label %bb32, label %bb67

bb67:                                             ; preds = %bb62
  %i68 = getelementptr inbounds i16, ptr %i65, i64 1
  %i69 = getelementptr inbounds i16, ptr %i64, i64 1
  %i70 = load i16, ptr %i68, align 2, !tbaa !906
  %i71 = load i16, ptr %i69, align 2, !tbaa !906
  %i72 = icmp eq i16 %i70, %i71
  br i1 %i72, label %bb62, label %bb113, !llvm.loop !1109

bb73:                                             ; preds = %bb35
  br i1 %i46, label %bb78, label %bb74

bb74:                                             ; preds = %bb73
  %i75 = load i16, ptr %i38, align 2, !tbaa !906
  %i76 = icmp ne i16 %i75, 0
  %i77 = zext i1 %i76 to i32
  br label %bb78

bb78:                                             ; preds = %bb74, %bb73
  %i79 = phi i1 [ false, %bb73 ], [ %i76, %bb74 ]
  %i80 = phi i32 [ 0, %bb73 ], [ %i77, %bb74 ]
  %i81 = icmp eq ptr %i40, null
  br i1 %i81, label %bb86, label %bb82

bb82:                                             ; preds = %bb78
  %i83 = load i16, ptr %i40, align 2, !tbaa !906
  %i84 = icmp ne i16 %i83, 0
  %i85 = zext i1 %i84 to i32
  br label %bb86

bb86:                                             ; preds = %bb82, %bb78
  %i87 = phi i1 [ false, %bb78 ], [ %i84, %bb82 ]
  %i88 = phi i32 [ 0, %bb78 ], [ %i85, %bb82 ]
  %i89 = or i32 %i88, %i80
  %i90 = icmp eq i32 %i89, 0
  br i1 %i90, label %bb111, label %bb91

bb91:                                             ; preds = %bb86
  %i92 = and i1 %i79, %i87
  br i1 %i92, label %bb93, label %bb113

bb93:                                             ; preds = %bb91
  %i94 = icmp eq ptr %i37, %i39
  br i1 %i94, label %bb95, label %bb103

bb95:                                             ; preds = %bb93
  %i96 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %i37, i64 0, i32 0, i32 0
  %i97 = load ptr, ptr %i96, align 8, !tbaa !861
  %i98 = tail call i1 @llvm.type.test(ptr %i97, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i98)
  %i99 = getelementptr inbounds ptr, ptr %i97, i64 10
  %i100 = load ptr, ptr %i99, align 8
  %i101 = tail call noundef i32 %i100(ptr noundef nonnull align 8 dereferenceable(104) %i37, ptr noundef %i38, ptr noundef %i40, ptr noundef %i42), !intel_dtrans_type !1110
  %i102 = icmp eq i32 %i101, 0
  br i1 %i102, label %bb32, label %bb113

bb103:                                            ; preds = %bb93
  %i104 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %i39, i64 0, i32 0, i32 0
  %i105 = load ptr, ptr %i104, align 8, !tbaa !861
  %i106 = tail call i1 @llvm.type.test(ptr %i105, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i106)
  %i107 = getelementptr inbounds ptr, ptr %i105, i64 10
  %i108 = load ptr, ptr %i107, align 8
  %i109 = tail call noundef i32 %i108(ptr noundef nonnull align 8 dereferenceable(104) %i39, ptr noundef %i38, ptr noundef %i40, ptr noundef %i42), !intel_dtrans_type !1110
  %i110 = icmp eq i32 %i109, 0
  br i1 %i110, label %bb32, label %bb113

bb111:                                            ; preds = %bb86
  %i112 = icmp eq ptr %i37, %i39
  br i1 %i112, label %bb32, label %bb113

bb113:                                            ; preds = %bb111, %bb103, %bb95, %bb91, %bb67, %bb59, %bb55, %bb50, %bb29
  %i114 = add nuw i32 %i20, 1
  %i115 = icmp eq i32 %i114, %i15
  br i1 %i115, label %bb130, label %bb19, !llvm.loop !1111

bb116:                                            ; preds = %bb127, %bb17
  %i117 = phi i32 [ %i128, %bb127 ], [ 0, %bb17 ]
  %i118 = load ptr, ptr %i, align 8, !tbaa !1038
  %i119 = tail call fastcc noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i118, i32 noundef %i117)
  %i120 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i119, i64 0, i32 0, !intel-tbaa !892
  %i121 = load ptr, ptr %i120, align 8, !tbaa !892
  %i122 = icmp eq ptr %i121, null
  br i1 %i122, label %bb130, label %bb123

bb123:                                            ; preds = %bb116
  %i124 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i121, i64 0, i32 1, !intel-tbaa !918
  %i125 = load i32, ptr %i124, align 4, !tbaa !918
  %i126 = icmp eq i32 %i125, 0
  br i1 %i126, label %bb130, label %bb127

bb127:                                            ; preds = %bb123
  %i128 = add nuw i32 %i117, 1
  %i129 = icmp eq i32 %i128, %i15
  br i1 %i129, label %bb130, label %bb116, !llvm.loop !1111

bb130:                                            ; preds = %bb127, %bb123, %bb116, %bb113, %bb32, %bb11, %bb
  %i131 = phi i1 [ false, %bb ], [ false, %bb11 ], [ true, %bb123 ], [ false, %bb127 ], [ true, %bb32 ], [ false, %bb113 ], [ true, %bb116 ]
  ret i1 %i131
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #26 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1112 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !1043
  %i2 = load i32, ptr %i, align 4, !tbaa !1043
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 5, !intel-tbaa !1046
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1046
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.4452, i32 noundef 249, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1045
  %i13 = load ptr, ptr %i12, align 8, !tbaa !1045
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1047
  ret ptr %i16
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, i1 zeroext %arg3) unnamed_addr #16 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1114 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !913
  store i8 0, ptr %i, align 8, !tbaa !913
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  store i32 0, ptr %i4, align 4, !tbaa !918
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !919
  store i32 1, ptr %i5, align 8, !tbaa !919
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !920
  store ptr null, ptr %i6, align 8, !tbaa !920
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !921
  store ptr %arg2, ptr %i7, align 8, !tbaa !921
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !861
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 2
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i13, label %bb14, label %bb28

bb14:                                             ; preds = %bb
  %i15 = invoke noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #39
          to label %bb30 unwind label %bb16

bb16:                                             ; preds = %bb14
  %i17 = landingpad { ptr, i32 }
          catch ptr null
  %i18 = extractvalue { ptr, i32 } %i17, 0
  %i19 = tail call ptr @__cxa_begin_catch(ptr %i18) #36
  %i20 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i20, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb27 unwind label %bb21

bb21:                                             ; preds = %bb16
  %i22 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb23 unwind label %bb24

bb23:                                             ; preds = %bb21
  resume { ptr, i32 } %i22

bb24:                                             ; preds = %bb21
  %i25 = landingpad { ptr, i32 }
          catch ptr null
  %i26 = extractvalue { ptr, i32 } %i25, 0
  tail call fastcc void @__clang_call_terminate(ptr %i26) #37
  unreachable

bb27:                                             ; preds = %bb16
  unreachable

bb28:                                             ; preds = %bb
  %i29 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb30:                                             ; preds = %bb14
  store ptr %i15, ptr %i6, align 8, !tbaa !920
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %i15, i8 0, i64 24, i1 false)
  ret void
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 %arg1) local_unnamed_addr #29 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1115 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  %i2 = load i32, ptr %i, align 4, !tbaa !918
  %i3 = add i32 %i2, 1
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !919
  %i5 = load i32, ptr %i4, align 8, !tbaa !919
  %i6 = icmp ugt i32 %i3, %i5
  br i1 %i6, label %bb7, label %bb69

bb7:                                              ; preds = %bb
  %i8 = uitofp i32 %i2 to double
  %i9 = fmul fast double %i8, 1.250000e+00
  %i10 = fptoui double %i9 to i32
  %i11 = icmp ult i32 %i3, %i10
  %i12 = select i1 %i11, i32 %i10, i32 %i3
  %i13 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !921
  %i14 = load ptr, ptr %i13, align 8, !tbaa !921
  %i15 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i16 = load ptr, ptr %i15, align 8, !tbaa !861
  %i17 = tail call i1 @llvm.type.test(ptr %i16, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i17)
  %i18 = getelementptr inbounds ptr, ptr %i16, i64 2
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i20, label %bb21, label %bb37

bb21:                                             ; preds = %bb7
  %i22 = zext i32 %i12 to i64
  %i23 = mul nuw nsw i64 %i22, 24
  %i24 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i23) #39
          to label %bb39 unwind label %bb25

bb25:                                             ; preds = %bb21
  %i26 = landingpad { ptr, i32 }
          catch ptr null
  %i27 = extractvalue { ptr, i32 } %i26, 0
  %i28 = tail call ptr @__cxa_begin_catch(ptr %i27) #36
  %i29 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i29, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb36 unwind label %bb30

bb30:                                             ; preds = %bb25
  %i31 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb32 unwind label %bb33

bb32:                                             ; preds = %bb30
  resume { ptr, i32 } %i31

bb33:                                             ; preds = %bb30
  %i34 = landingpad { ptr, i32 }
          catch ptr null
  %i35 = extractvalue { ptr, i32 } %i34, 0
  tail call fastcc void @__clang_call_terminate(ptr %i35) #37
  unreachable

bb36:                                             ; preds = %bb25
  unreachable

bb37:                                             ; preds = %bb7
  %i38 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb39:                                             ; preds = %bb21
  %i40 = load i32, ptr %i, align 4, !tbaa !918
  %i41 = icmp eq i32 %i40, 0
  %i42 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3
  %i43 = load ptr, ptr %i42, align 8, !tbaa !920
  br i1 %i41, label %bb46, label %bb44

bb44:                                             ; preds = %bb39
  %i45 = zext i32 %i40 to i64
  br label %bb56

bb46:                                             ; preds = %bb56, %bb39
  %i47 = load ptr, ptr %i13, align 8, !tbaa !921
  %i48 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i47, i64 0, i32 0
  %i49 = load ptr, ptr %i48, align 8, !tbaa !861
  %i50 = tail call i1 @llvm.type.test(ptr %i49, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i50)
  %i51 = getelementptr inbounds ptr, ptr %i49, i64 3
  %i52 = load ptr, ptr %i51, align 8
  %i53 = icmp eq ptr %i52, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i53, label %bb55, label %bb54

bb54:                                             ; preds = %bb46
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !926, !_Intel.Devirt.Call !878
  unreachable

bb55:                                             ; preds = %bb46
  tail call void @_ZdlPv(ptr noundef %i43) #36
  store ptr %i24, ptr %i42, align 8, !tbaa !920
  store i32 %i12, ptr %i4, align 8, !tbaa !919
  br label %bb69

bb56:                                             ; preds = %bb56, %bb44
  %i57 = phi i64 [ 0, %bb44 ], [ %i67, %bb56 ]
  %i58 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 0
  %i59 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 2
  %i60 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 1
  %i61 = load ptr, ptr %i60, align 8
  %i62 = load ptr, ptr %i59, align 8
  %i63 = load ptr, ptr %i58, align 8, !tbaa !922
  %i64 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 0
  %i65 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 2
  %i66 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 1
  store ptr %i61, ptr %i66, align 8
  store ptr %i62, ptr %i65, align 8
  store ptr %i63, ptr %i64, align 8, !tbaa !922
  %i67 = add nuw nsw i64 %i57, 1
  %i68 = icmp eq i64 %i67, %i45
  br i1 %i68, label %bb46, label %bb56, !llvm.loop !1116

bb69:                                             ; preds = %bb55, %bb
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv() #30 align 2

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) unnamed_addr #31 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1117 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !861
  %i1 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5, !intel-tbaa !875
  %i2 = load ptr, ptr %i1, align 8, !tbaa !875
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !874
  %i4 = load ptr, ptr %i3, align 8, !tbaa !874
  %i5 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i2, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !861
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = getelementptr inbounds ptr, ptr %i6, i64 3
  %i9 = load ptr, ptr %i8, align 8
  %i10 = icmp eq ptr %i9, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i10, label %bb13, label %bb11

bb11:                                             ; preds = %bb
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb12 unwind label %bb26, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb12:                                             ; preds = %bb11
  unreachable

bb13:                                             ; preds = %bb
  tail call void @_ZdlPv(ptr noundef %i4) #36
  %i14 = load ptr, ptr %i1, align 8, !tbaa !875
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 2, !intel-tbaa !872
  %i16 = load ptr, ptr %i15, align 8, !tbaa !872
  %i17 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 8, !tbaa !861
  %i19 = tail call i1 @llvm.type.test(ptr %i18, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i19)
  %i20 = getelementptr inbounds ptr, ptr %i18, i64 3
  %i21 = load ptr, ptr %i20, align 8
  %i22 = icmp eq ptr %i21, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i22, label %bb25, label %bb23

bb23:                                             ; preds = %bb13
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb24 unwind label %bb26, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb24:                                             ; preds = %bb23
  unreachable

bb25:                                             ; preds = %bb13
  tail call void @_ZdlPv(ptr noundef %i16) #36
  ret void

bb26:                                             ; preds = %bb23, %bb11
  %i27 = landingpad { ptr, i32 }
          catch ptr null
  %i28 = extractvalue { ptr, i32 } %i27, 0
  tail call fastcc void @__clang_call_terminate(ptr %i28) #37
  unreachable
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1119 {
bb:
  %i = alloca [2048 x i16], align 16, !intel_dtrans_type !1120
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 1, !intel-tbaa !864
  store i32 %arg1, ptr %i2, align 8, !tbaa !864
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i) #36
  %i3 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv()
  %i4 = getelementptr inbounds [2048 x i16], ptr %i, i64 0, i64 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i3, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !861
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(8) %i3, i32 noundef %arg1, ptr noundef nonnull %i4, i32 noundef 2047), !intel_dtrans_type !1121
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5
  %i10 = load ptr, ptr %i9, align 8, !tbaa !875
  br i1 %i8, label %bb46, label %bb11

bb11:                                             ; preds = %bb11, %bb
  %i12 = phi ptr [ %i13, %bb11 ], [ @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, %bb ]
  %i13 = getelementptr inbounds i16, ptr %i12, i64 1
  %i14 = getelementptr [23 x i16], ptr %i13, i64 0, i64 0
  %i15 = load i16, ptr %i14, align 2, !tbaa !906
  %i16 = icmp eq i16 %i15, 0
  br i1 %i16, label %bb17, label %bb11, !llvm.loop !1123

bb17:                                             ; preds = %bb11
  %i18 = ptrtoint ptr %i13 to i64
  %i19 = add i64 %i18, sub (i64 2, i64 ptrtoint (ptr @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE to i64))
  %i20 = and i64 %i19, 8589934590
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !861
  %i23 = call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb42

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #39
          to label %bb44 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = call ptr @__cxa_begin_catch(ptr %i31) #36
  %i33 = call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb41 unwind label %bb34

bb34:                                             ; preds = %bb29
  %i35 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb38

bb36:                                             ; preds = %bb75, %bb34
  %i37 = phi { ptr, i32 } [ %i35, %bb34 ], [ %i76, %bb75 ]
  resume { ptr, i32 } %i37

bb38:                                             ; preds = %bb34
  %i39 = landingpad { ptr, i32 }
          catch ptr null
  %i40 = extractvalue { ptr, i32 } %i39, 0
  call fastcc void @__clang_call_terminate(ptr %i40) #37
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb17
  %i43 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb44:                                             ; preds = %bb27
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i28, ptr nonnull align 16 @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, i64 %i20, i1 false)
  %i45 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !874
  store ptr %i28, ptr %i45, align 8, !tbaa !874
  br label %bb85

bb46:                                             ; preds = %bb
  %i47 = load i16, ptr %i4, align 16, !tbaa !906
  %i48 = icmp eq i16 %i47, 0
  br i1 %i48, label %bb60, label %bb49

bb49:                                             ; preds = %bb49, %bb46
  %i50 = phi ptr [ %i51, %bb49 ], [ %i4, %bb46 ]
  %i51 = getelementptr inbounds i16, ptr %i50, i64 1
  %i52 = load i16, ptr %i51, align 2, !tbaa !906
  %i53 = icmp eq i16 %i52, 0
  br i1 %i53, label %bb54, label %bb49, !llvm.loop !1123

bb54:                                             ; preds = %bb49
  %i55 = ptrtoint ptr %i51 to i64
  %i56 = ptrtoint ptr %i to i64
  %i57 = sub i64 2, %i56
  %i58 = add i64 %i57, %i55
  %i59 = and i64 %i58, 8589934590
  br label %bb60

bb60:                                             ; preds = %bb54, %bb46
  %i61 = phi i64 [ %i59, %bb54 ], [ 2, %bb46 ]
  %i62 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i63 = load ptr, ptr %i62, align 8, !tbaa !861
  %i64 = call i1 @llvm.type.test(ptr %i63, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i64)
  %i65 = getelementptr inbounds ptr, ptr %i63, i64 2
  %i66 = load ptr, ptr %i65, align 8
  %i67 = icmp eq ptr %i66, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i67, label %bb68, label %bb81

bb68:                                             ; preds = %bb60
  %i69 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i61) #39
          to label %bb83 unwind label %bb70

bb70:                                             ; preds = %bb68
  %i71 = landingpad { ptr, i32 }
          catch ptr null
  %i72 = extractvalue { ptr, i32 } %i71, 0
  %i73 = call ptr @__cxa_begin_catch(ptr %i72) #36
  %i74 = call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i74, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb80 unwind label %bb75

bb75:                                             ; preds = %bb70
  %i76 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb77

bb77:                                             ; preds = %bb75
  %i78 = landingpad { ptr, i32 }
          catch ptr null
  %i79 = extractvalue { ptr, i32 } %i78, 0
  call fastcc void @__clang_call_terminate(ptr %i79) #37
  unreachable

bb80:                                             ; preds = %bb70
  unreachable

bb81:                                             ; preds = %bb60
  %i82 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb83:                                             ; preds = %bb68
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i69, ptr nonnull align 16 %i4, i64 %i61, i1 false)
  %i84 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !874
  store ptr %i69, ptr %i84, align 8, !tbaa !874
  br label %bb85

bb85:                                             ; preds = %bb83, %bb44
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i) #36
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv() unnamed_addr #20 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1124 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1125
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb44

bb2:                                              ; preds = %bb
  %i3 = load i1, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br i1 %i3, label %bb23, label %bb4

bb4:                                              ; preds = %bb2
  %i5 = load i1, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br i1 %i5, label %bb20, label %bb6

bb6:                                              ; preds = %bb4
  %i7 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEm(i64 noundef 8)
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex", ptr %i7, i64 0, i32 0, !intel-tbaa !1127
  store ptr null, ptr %i8, align 8, !tbaa !1127
  store ptr %i7, ptr @_ZN11xercesc_2_7L9sMsgMutexE, align 8, !tbaa !1129
  store ptr @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv, ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, align 8, !tbaa !1131
  %i9 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  %i10 = icmp eq ptr %i9, null
  %i11 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 2), align 8
  %i12 = icmp eq ptr %i11, null
  %i13 = select i1 %i10, i1 %i12, i1 false
  br i1 %i13, label %bb14, label %bb19

bb14:                                             ; preds = %bb6
  %i15 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  store ptr %i15, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  store ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  %i16 = icmp eq ptr %i15, null
  br i1 %i16, label %bb19, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i15, i64 0, i32 2, !intel-tbaa !1137
  store ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, ptr %i18, align 8, !tbaa !1137
  br label %bb19

bb19:                                             ; preds = %bb17, %bb14, %bb6
  store i1 true, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br label %bb20

bb20:                                             ; preds = %bb19, %bb4
  %i21 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1125
  %i22 = icmp eq ptr %i21, null
  br i1 %i22, label %bb23, label %bb42

bb23:                                             ; preds = %bb20, %bb2
  %i24 = tail call fastcc noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef nonnull @_ZN11xercesc_2_76XMLUni14fgExceptDomainE)
  store ptr %i24, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1125
  %i25 = icmp eq ptr %i24, null
  br i1 %i25, label %bb26, label %bb31

bb26:                                             ; preds = %bb23
  %i27 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1071
  %i28 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i27, i64 0, i32 0
  %i29 = load ptr, ptr %i28, align 8, !tbaa !861
  %i30 = tail call i1 @llvm.type.test(ptr %i29, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i30)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1073
  unreachable

bb31:                                             ; preds = %bb23
  store ptr @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv, ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, align 8, !tbaa !1131
  %i32 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  %i33 = icmp eq ptr %i32, null
  %i34 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 2), align 8
  %i35 = icmp eq ptr %i34, null
  %i36 = select i1 %i33, i1 %i35, i1 false
  br i1 %i36, label %bb37, label %bb42

bb37:                                             ; preds = %bb31
  %i38 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  store ptr %i38, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  %i39 = icmp eq ptr %i38, null
  br i1 %i39, label %bb42, label %bb40

bb40:                                             ; preds = %bb37
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i38, i64 0, i32 2, !intel-tbaa !1137
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, ptr %i41, align 8, !tbaa !1137
  br label %bb42

bb42:                                             ; preds = %bb40, %bb37, %bb31, %bb20
  %i43 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1125
  br label %bb44

bb44:                                             ; preds = %bb42, %bb
  %i45 = phi ptr [ %i43, %bb42 ], [ %i, %bb ]
  ret ptr %i45
}

; Function Attrs: nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv() #32 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_79XMLString13replaceTokensEPtjPKtS3_S3_S3_PNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef readonly "intel_dtrans_func_index"="2" %arg2, ptr noundef readonly "intel_dtrans_func_index"="3" %arg3, ptr noundef readonly "intel_dtrans_func_index"="4" %arg4, ptr noundef readonly "intel_dtrans_func_index"="5" %arg5, ptr noundef readonly "intel_dtrans_func_index"="6" %arg6) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1138 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb46, label %bb7

bb7:                                              ; preds = %bb
  %i8 = load i16, ptr %arg, align 2, !tbaa !906
  %i9 = icmp eq i16 %i8, 0
  br i1 %i9, label %bb21, label %bb10

bb10:                                             ; preds = %bb10, %bb7
  %i11 = phi ptr [ %i12, %bb10 ], [ %arg, %bb7 ]
  %i12 = getelementptr inbounds i16, ptr %i11, i64 1
  %i13 = load i16, ptr %i12, align 2, !tbaa !906
  %i14 = icmp eq i16 %i13, 0
  br i1 %i14, label %bb15, label %bb10, !llvm.loop !1139

bb15:                                             ; preds = %bb10
  %i16 = ptrtoint ptr %i12 to i64
  %i17 = ptrtoint ptr %arg to i64
  %i18 = sub i64 2, %i17
  %i19 = add i64 %i18, %i16
  %i20 = and i64 %i19, 8589934590
  br label %bb21

bb21:                                             ; preds = %bb15, %bb7
  %i22 = phi i64 [ %i20, %bb15 ], [ 2, %bb7 ]
  %i23 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg6, i64 0, i32 0
  %i24 = load ptr, ptr %i23, align 8, !tbaa !861
  %i25 = tail call i1 @llvm.type.test(ptr %i24, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i25)
  %i26 = getelementptr inbounds ptr, ptr %i24, i64 2
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i28, label %bb29, label %bb43

bb29:                                             ; preds = %bb21
  %i30 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i22) #39
          to label %bb45 unwind label %bb31

bb31:                                             ; preds = %bb29
  %i32 = landingpad { ptr, i32 }
          catch ptr null
  %i33 = extractvalue { ptr, i32 } %i32, 0
  %i34 = tail call ptr @__cxa_begin_catch(ptr %i33) #36
  %i35 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i35, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb42 unwind label %bb36

bb36:                                             ; preds = %bb31
  %i37 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb38 unwind label %bb39

bb38:                                             ; preds = %bb36
  resume { ptr, i32 } %i37

bb39:                                             ; preds = %bb36
  %i40 = landingpad { ptr, i32 }
          catch ptr null
  %i41 = extractvalue { ptr, i32 } %i40, 0
  tail call fastcc void @__clang_call_terminate(ptr %i41) #37
  unreachable

bb42:                                             ; preds = %bb31
  unreachable

bb43:                                             ; preds = %bb21
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb45:                                             ; preds = %bb29
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i30, ptr nonnull align 2 %arg, i64 %i22, i1 false)
  br label %bb46

bb46:                                             ; preds = %bb45, %bb
  %i47 = phi ptr [ %i30, %bb45 ], [ null, %bb ]
  %i48 = load i16, ptr %i47, align 2, !tbaa !906
  %i49 = icmp ne i16 %i48, 0
  %i50 = icmp ne i32 %arg1, 0
  %i51 = and i1 %i49, %i50
  br i1 %i51, label %bb52, label %bb124

bb52:                                             ; preds = %bb46
  %i53 = zext i32 %arg1 to i64
  br label %bb54

bb54:                                             ; preds = %bb117, %bb52
  %i55 = phi i16 [ %i120, %bb117 ], [ %i48, %bb52 ]
  %i56 = phi ptr [ %i119, %bb117 ], [ %i47, %bb52 ]
  %i57 = phi i32 [ %i118, %bb117 ], [ 0, %bb52 ]
  %i58 = icmp ult i32 %i57, %arg1
  br i1 %i58, label %bb59, label %bb71

bb59:                                             ; preds = %bb63, %bb54
  %i60 = phi ptr [ %i64, %bb63 ], [ %i56, %bb54 ]
  %i61 = phi i32 [ %i69, %bb63 ], [ %i57, %bb54 ]
  %i62 = phi i16 [ %i68, %bb63 ], [ %i55, %bb54 ]
  switch i16 %i62, label %bb63 [
    i16 123, label %bb71
    i16 0, label %bb71
  ]

bb63:                                             ; preds = %bb59
  %i64 = getelementptr inbounds i16, ptr %i60, i64 1
  %i65 = add nuw i32 %i61, 1
  %i66 = zext i32 %i61 to i64
  %i67 = getelementptr inbounds i16, ptr %arg, i64 %i66
  store i16 %i62, ptr %i67, align 2, !tbaa !906
  %i68 = load i16, ptr %i64, align 2, !tbaa !906
  %i69 = freeze i32 %i65
  %i70 = icmp ult i32 %i69, %arg1
  br i1 %i70, label %bb59, label %bb71, !llvm.loop !1140

bb71:                                             ; preds = %bb63, %bb59, %bb59, %bb54
  %i72 = phi i16 [ %i55, %bb54 ], [ %i62, %bb59 ], [ %i62, %bb59 ], [ %i68, %bb63 ]
  %i73 = phi i32 [ %i57, %bb54 ], [ %i61, %bb59 ], [ %i61, %bb59 ], [ %i69, %bb63 ]
  %i74 = phi ptr [ %i56, %bb54 ], [ %i60, %bb59 ], [ %i60, %bb59 ], [ %i64, %bb63 ]
  %i75 = icmp eq i16 %i72, 123
  br i1 %i75, label %bb76, label %bb124

bb76:                                             ; preds = %bb71
  %i77 = getelementptr inbounds i16, ptr %i74, i64 1, !intel-tbaa !906
  %i78 = load i16, ptr %i77, align 2, !tbaa !906
  %i79 = and i16 %i78, -4
  %i80 = icmp eq i16 %i79, 48
  br i1 %i80, label %bb81, label %bb111

bb81:                                             ; preds = %bb76
  %i82 = getelementptr inbounds i16, ptr %i74, i64 2, !intel-tbaa !906
  %i83 = load i16, ptr %i82, align 2, !tbaa !906
  %i84 = icmp eq i16 %i83, 125
  br i1 %i84, label %bb85, label %bb111

bb85:                                             ; preds = %bb81
  %i86 = getelementptr inbounds i16, ptr %i74, i64 3, !intel-tbaa !906
  switch i16 %i78, label %bb87 [
    i16 48, label %bb90
    i16 49, label %bb88
    i16 50, label %bb89
  ]

bb87:                                             ; preds = %bb85
  br label %bb90

bb88:                                             ; preds = %bb85
  br label %bb90

bb89:                                             ; preds = %bb85
  br label %bb90

bb90:                                             ; preds = %bb89, %bb88, %bb87, %bb85
  %i91 = phi ptr [ %arg5, %bb87 ], [ %arg2, %bb85 ], [ %arg3, %bb88 ], [ %arg4, %bb89 ]
  %i92 = icmp eq ptr %i91, null
  %i93 = select i1 %i92, ptr @_ZN11xercesc_2_7L8gNullStrE, ptr %i91
  %i94 = load i16, ptr %i93, align 2, !tbaa !906
  %i95 = icmp ne i16 %i94, 0
  %i96 = icmp ult i32 %i73, %arg1
  %i97 = select i1 %i95, i1 %i96, i1 false
  br i1 %i97, label %bb98, label %bb117

bb98:                                             ; preds = %bb90
  %i99 = zext i32 %i73 to i64
  br label %bb100

bb100:                                            ; preds = %bb100, %bb98
  %i101 = phi i64 [ %i99, %bb98 ], [ %i105, %bb100 ]
  %i102 = phi i16 [ %i94, %bb98 ], [ %i107, %bb100 ]
  %i103 = phi ptr [ %i93, %bb98 ], [ %i104, %bb100 ]
  %i104 = getelementptr inbounds i16, ptr %i103, i64 1
  %i105 = add nuw nsw i64 %i101, 1
  %i106 = getelementptr inbounds i16, ptr %arg, i64 %i101
  store i16 %i102, ptr %i106, align 2, !tbaa !906
  %i107 = load i16, ptr %i104, align 2, !tbaa !906
  %i108 = icmp ne i16 %i107, 0
  %i109 = icmp ult i64 %i105, %i53
  %i110 = select i1 %i108, i1 %i109, i1 false
  br i1 %i110, label %bb100, label %bb115, !llvm.loop !1141

bb111:                                            ; preds = %bb81, %bb76
  %i112 = add i32 %i73, 1
  %i113 = zext i32 %i73 to i64
  %i114 = getelementptr inbounds i16, ptr %arg, i64 %i113
  store i16 123, ptr %i114, align 2, !tbaa !906
  br label %bb117

bb115:                                            ; preds = %bb100
  %i116 = trunc i64 %i105 to i32
  br label %bb117

bb117:                                            ; preds = %bb115, %bb111, %bb90
  %i118 = phi i32 [ %i112, %bb111 ], [ %i73, %bb90 ], [ %i116, %bb115 ]
  %i119 = phi ptr [ %i77, %bb111 ], [ %i86, %bb90 ], [ %i86, %bb115 ]
  %i120 = load i16, ptr %i119, align 2, !tbaa !906
  %i121 = icmp ne i16 %i120, 0
  %i122 = icmp ult i32 %i118, %arg1
  %i123 = select i1 %i121, i1 %i122, i1 false
  br i1 %i123, label %bb54, label %bb124, !llvm.loop !1142

bb124:                                            ; preds = %bb117, %bb71, %bb46
  %i125 = phi i32 [ 0, %bb46 ], [ %i118, %bb117 ], [ %i73, %bb71 ]
  %i126 = zext i32 %i125 to i64
  %i127 = getelementptr inbounds i16, ptr %arg, i64 %i126
  store i16 0, ptr %i127, align 2, !tbaa !906
  %i128 = icmp eq ptr %arg6, null
  br i1 %i128, label %bb139, label %bb129

bb129:                                            ; preds = %bb124
  %i130 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg6, i64 0, i32 0
  %i131 = load ptr, ptr %i130, align 8, !tbaa !861
  %i132 = tail call i1 @llvm.type.test(ptr %i131, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i132)
  %i133 = getelementptr inbounds ptr, ptr %i131, i64 3
  %i134 = load ptr, ptr %i133, align 8
  %i135 = icmp eq ptr %i134, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i135, label %bb138, label %bb136

bb136:                                            ; preds = %bb129
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb137 unwind label %bb140, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb137:                                            ; preds = %bb136
  unreachable

bb138:                                            ; preds = %bb129
  tail call void @_ZdlPv(ptr noundef nonnull %i47) #36
  br label %bb143

bb139:                                            ; preds = %bb124
  tail call void @_ZdaPv(ptr noundef nonnull %i47) #42
  br label %bb143

bb140:                                            ; preds = %bb136
  %i141 = landingpad { ptr, i32 }
          catch ptr null
  %i142 = extractvalue { ptr, i32 } %i141, 0
  tail call fastcc void @__clang_call_terminate(ptr %i142) #37
  unreachable

bb143:                                            ; preds = %bb139, %bb138
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLValidator15reinitMsgLoaderEv() #30 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #20 align 2 !intel.dtrans.func.type !1143 {
bb:
  %i = alloca ptr, align 8, !intel_dtrans_type !469
  %i2 = alloca [1024 x i16], align 16, !intel_dtrans_type !1144
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1145
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1145
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i4, i64 0, i32 25, !intel-tbaa !1149
  %i6 = load i32, ptr %i5, align 4, !tbaa !1149
  %i7 = add nsw i32 %i6, 1
  store i32 %i7, ptr %i5, align 4, !tbaa !1149
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 2, !intel-tbaa !1150
  %i9 = load ptr, ptr %i8, align 8, !tbaa !1150
  %i10 = icmp eq ptr %i9, null
  br i1 %i10, label %bb48, label %bb11

bb11:                                             ; preds = %bb
  call void @llvm.lifetime.start.p0(i64 2048, ptr nonnull %i2) #36
  %i12 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L12getMsgLoaderEv()
  %i13 = getelementptr inbounds [1024 x i16], ptr %i2, i64 0, i64 0
  %i14 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i12, i64 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !861
  %i16 = tail call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(8) %i12, i32 noundef %arg1, ptr noundef nonnull %i13, i32 noundef 1023), !intel_dtrans_type !1121
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 3, !intel-tbaa !1151
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1151
  %i20 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i19, i64 0, i32 6, !intel-tbaa !1080
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1080
  %i22 = icmp eq ptr %i21, null
  br i1 %i22, label %bb37, label %bb23

bb23:                                             ; preds = %bb11
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i19, i64 0, i32 2, !intel-tbaa !1076
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1076
  %i26 = icmp eq ptr %i25, null
  br i1 %i26, label %bb37, label %bb27

bb27:                                             ; preds = %bb23
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #36
  %i28 = call fastcc noundef ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr noundef nonnull align 8 dereferenceable(80) %i19, ptr noundef nonnull align 8 dereferenceable(8) %i)
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 22, !intel-tbaa !1152
  %i30 = load ptr, ptr %i29, align 8, !tbaa !1152
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 11, !intel-tbaa !1164
  %i32 = load ptr, ptr %i31, align 8, !tbaa !1164
  %i33 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 6, !intel-tbaa !1165
  %i34 = load i64, ptr %i33, align 8, !tbaa !1165
  %i35 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 5, !intel-tbaa !1166
  %i36 = load i64, ptr %i35, align 8, !tbaa !1166
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #36
  br label %bb37

bb37:                                             ; preds = %bb27, %bb23, %bb11
  %i38 = phi i64 [ %i36, %bb27 ], [ 0, %bb23 ], [ 0, %bb11 ]
  %i39 = phi i64 [ %i34, %bb27 ], [ 0, %bb23 ], [ 0, %bb11 ]
  %i40 = phi ptr [ %i32, %bb27 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb23 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb11 ]
  %i41 = phi ptr [ %i30, %bb27 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb23 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb11 ]
  %i42 = load ptr, ptr %i8, align 8, !tbaa !1150
  %i43 = getelementptr %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter", ptr %i42, i64 0, i32 0
  %i44 = load ptr, ptr %i43, align 8, !tbaa !861
  %i45 = call i1 @llvm.type.test(ptr %i44, metadata !"_ZTSN11xercesc_2_716XMLErrorReporterE")
  call void @llvm.assume(i1 %i45)
  %i46 = getelementptr inbounds ptr, ptr %i44, i64 2
  %i47 = load ptr, ptr %i46, align 8
  call void %i47(ptr noundef nonnull align 8 dereferenceable(8) %i42, i32 noundef %arg1, ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, i32 noundef 1, ptr noundef nonnull %i13, ptr noundef %i41, ptr noundef %i40, i64 noundef %i39, i64 noundef %i38), !intel_dtrans_type !1167
  call void @llvm.lifetime.end.p0(i64 2048, ptr nonnull %i2) #36
  br label %bb48

bb48:                                             ; preds = %bb37, %bb
  %i49 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1145
  %i50 = load ptr, ptr %i49, align 8, !tbaa !1145
  %i51 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 6, !intel-tbaa !1168
  %i52 = load i8, ptr %i51, align 1, !tbaa !1168, !range !916, !noundef !917
  %i53 = icmp eq i8 %i52, 0
  br i1 %i53, label %bb64, label %bb54

bb54:                                             ; preds = %bb48
  %i55 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 5, !intel-tbaa !1169
  %i56 = load i8, ptr %i55, align 1, !tbaa !1169, !range !916, !noundef !917
  %i57 = icmp eq i8 %i56, 0
  br i1 %i57, label %bb64, label %bb58

bb58:                                             ; preds = %bb54
  %i59 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 7, !intel-tbaa !1170
  %i60 = load i8, ptr %i59, align 1, !tbaa !1170, !range !916, !noundef !917
  %i61 = icmp eq i8 %i60, 0
  br i1 %i61, label %bb62, label %bb64

bb62:                                             ; preds = %bb58
  %i63 = call ptr @__cxa_allocate_exception(i64 4) #36
  store i32 %arg1, ptr %i63, align 16, !tbaa !1171
  call void @__cxa_throw(ptr nonnull %i63, ptr nonnull @_ZTIN11xercesc_2_78XMLValid5CodesE, ptr null) #40
  unreachable

bb64:                                             ; preds = %bb58, %bb54, %bb48
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_7L12getMsgLoaderEv() unnamed_addr #20 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1173 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3652, align 8, !tbaa !1125
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb46

bb2:                                              ; preds = %bb
  %i3 = load ptr, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3655, align 8, !tbaa !1129
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb5, label %bb25

bb5:                                              ; preds = %bb2
  %i6 = load ptr, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3655, align 8, !tbaa !1129
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb8, label %bb22

bb8:                                              ; preds = %bb5
  %i9 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  %i10 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 8, ptr noundef %i9)
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex", ptr %i10, i64 0, i32 0, !intel-tbaa !1127
  store ptr null, ptr %i11, align 8, !tbaa !1127
  store ptr %i10, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3655, align 8, !tbaa !1129
  store ptr @_ZN11xercesc_2_712XMLValidator14reinitMsgMutexEv, ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, align 8, !tbaa !1131
  %i12 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  %i13 = icmp eq ptr %i12, null
  %i14 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 2), align 8
  %i15 = icmp eq ptr %i14, null
  %i16 = select i1 %i13, i1 %i15, i1 false
  br i1 %i16, label %bb17, label %bb22

bb17:                                             ; preds = %bb8
  %i18 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  store ptr %i18, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1135
  store ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  %i19 = icmp eq ptr %i18, null
  br i1 %i19, label %bb22, label %bb20

bb20:                                             ; preds = %bb17
  %i21 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i18, i64 0, i32 2, !intel-tbaa !1137
  store ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, ptr %i21, align 8, !tbaa !1137
  br label %bb22

bb22:                                             ; preds = %bb20, %bb17, %bb8, %bb5
  %i23 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3652, align 8, !tbaa !1125
  %i24 = icmp eq ptr %i23, null
  br i1 %i24, label %bb25, label %bb44

bb25:                                             ; preds = %bb22, %bb2
  %i26 = tail call fastcc noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE)
  store ptr %i26, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3652, align 8, !tbaa !1125
  %i27 = icmp eq ptr %i26, null
  br i1 %i27, label %bb28, label %bb33

bb28:                                             ; preds = %bb25
  %i29 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1071
  %i30 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i29, i64 0, i32 0
  %i31 = load ptr, ptr %i30, align 8, !tbaa !861
  %i32 = tail call i1 @llvm.type.test(ptr %i31, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i32)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1073
  unreachable

bb33:                                             ; preds = %bb25
  store ptr @_ZN11xercesc_2_712XMLValidator15reinitMsgLoaderEv, ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, align 8, !tbaa !1131
  %i34 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, i64 0, i32 1), align 8, !tbaa !1135
  %i35 = icmp eq ptr %i34, null
  %i36 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, i64 0, i32 2), align 8
  %i37 = icmp eq ptr %i36, null
  %i38 = select i1 %i35, i1 %i37, i1 false
  br i1 %i38, label %bb39, label %bb44

bb39:                                             ; preds = %bb33
  %i40 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  store ptr %i40, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, i64 0, i32 1), align 8, !tbaa !1135
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1136
  %i41 = icmp eq ptr %i40, null
  br i1 %i41, label %bb44, label %bb42

bb42:                                             ; preds = %bb39
  %i43 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i40, i64 0, i32 2, !intel-tbaa !1137
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3648, ptr %i43, align 8, !tbaa !1137
  br label %bb44

bb44:                                             ; preds = %bb42, %bb39, %bb33, %bb22
  %i45 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3652, align 8, !tbaa !1125
  br label %bb46

bb46:                                             ; preds = %bb44, %bb
  %i47 = phi ptr [ %i45, %bb44 ], [ %i, %bb ]
  ret ptr %i47
}

; Function Attrs: nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLValidator14reinitMsgMutexEv() #32 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, ptr noundef "intel_dtrans_func_index"="3" %arg3, ptr noundef "intel_dtrans_func_index"="4" %arg4) unnamed_addr #20 align 2 !intel.dtrans.func.type !1174 {
bb:
  %i = alloca ptr, align 8, !intel_dtrans_type !469
  %i5 = alloca [2048 x i16], align 16, !intel_dtrans_type !1120
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1145
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1145
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i7, i64 0, i32 25, !intel-tbaa !1149
  %i9 = load i32, ptr %i8, align 4, !tbaa !1149
  %i10 = add nsw i32 %i9, 1
  store i32 %i10, ptr %i8, align 4, !tbaa !1149
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 2, !intel-tbaa !1150
  %i12 = load ptr, ptr %i11, align 8, !tbaa !1150
  %i13 = icmp eq ptr %i12, null
  br i1 %i13, label %bb58, label %bb14

bb14:                                             ; preds = %bb
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i5) #36
  %i15 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L12getMsgLoaderEv()
  %i16 = getelementptr inbounds [2048 x i16], ptr %i5, i64 0, i64 0
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1145
  %i18 = load ptr, ptr %i17, align 8, !tbaa !1145
  %i19 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i18, i64 0, i32 62, !intel-tbaa !1175
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1175
  %i21 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !861
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_714InMemMsgLoaderE")
  tail call void @llvm.assume(i1 %i24)
  %i25 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(16) %i15, i32 noundef %arg1, ptr noundef nonnull %i16, i32 noundef 2047), !intel_dtrans_type !1176
  br i1 %i25, label %bb26, label %bb27

bb26:                                             ; preds = %bb14
  call fastcc void @_ZN11xercesc_2_79XMLString13replaceTokensEPtjPKtS3_S3_S3_PNS_13MemoryManagerE(ptr noundef nonnull %i16, i32 noundef 2047, ptr noundef %arg2, ptr noundef %arg3, ptr noundef %arg4, ptr noundef null, ptr noundef %i20)
  br label %bb27

bb27:                                             ; preds = %bb26, %bb14
  %i28 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 3, !intel-tbaa !1151
  %i29 = load ptr, ptr %i28, align 8, !tbaa !1151
  %i30 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i29, i64 0, i32 6, !intel-tbaa !1080
  %i31 = load ptr, ptr %i30, align 8, !tbaa !1080
  %i32 = icmp eq ptr %i31, null
  br i1 %i32, label %bb47, label %bb33

bb33:                                             ; preds = %bb27
  %i34 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i29, i64 0, i32 2, !intel-tbaa !1076
  %i35 = load ptr, ptr %i34, align 8, !tbaa !1076
  %i36 = icmp eq ptr %i35, null
  br i1 %i36, label %bb47, label %bb37

bb37:                                             ; preds = %bb33
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #36
  %i38 = call fastcc noundef ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr noundef nonnull align 8 dereferenceable(80) %i29, ptr noundef nonnull align 8 dereferenceable(8) %i)
  %i39 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 22, !intel-tbaa !1152
  %i40 = load ptr, ptr %i39, align 8, !tbaa !1152
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 11, !intel-tbaa !1164
  %i42 = load ptr, ptr %i41, align 8, !tbaa !1164
  %i43 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 6, !intel-tbaa !1165
  %i44 = load i64, ptr %i43, align 8, !tbaa !1165
  %i45 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 5, !intel-tbaa !1166
  %i46 = load i64, ptr %i45, align 8, !tbaa !1166
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #36
  br label %bb47

bb47:                                             ; preds = %bb37, %bb33, %bb27
  %i48 = phi i64 [ %i46, %bb37 ], [ 0, %bb33 ], [ 0, %bb27 ]
  %i49 = phi i64 [ %i44, %bb37 ], [ 0, %bb33 ], [ 0, %bb27 ]
  %i50 = phi ptr [ %i42, %bb37 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb33 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb27 ]
  %i51 = phi ptr [ %i40, %bb37 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb33 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb27 ]
  %i52 = load ptr, ptr %i11, align 8, !tbaa !1150
  %i53 = getelementptr %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter", ptr %i52, i64 0, i32 0
  %i54 = load ptr, ptr %i53, align 8, !tbaa !861
  %i55 = call i1 @llvm.type.test(ptr %i54, metadata !"_ZTSN11xercesc_2_716XMLErrorReporterE")
  call void @llvm.assume(i1 %i55)
  %i56 = getelementptr inbounds ptr, ptr %i54, i64 2
  %i57 = load ptr, ptr %i56, align 8
  call void %i57(ptr noundef nonnull align 8 dereferenceable(8) %i52, i32 noundef %arg1, ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, i32 noundef 1, ptr noundef nonnull %i16, ptr noundef %i51, ptr noundef %i50, i64 noundef %i49, i64 noundef %i48), !intel_dtrans_type !1167
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i5) #36
  br label %bb58

bb58:                                             ; preds = %bb47, %bb
  %i59 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1145
  %i60 = load ptr, ptr %i59, align 8, !tbaa !1145
  %i61 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 6, !intel-tbaa !1168
  %i62 = load i8, ptr %i61, align 1, !tbaa !1168, !range !916, !noundef !917
  %i63 = icmp eq i8 %i62, 0
  br i1 %i63, label %bb74, label %bb64

bb64:                                             ; preds = %bb58
  %i65 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 5, !intel-tbaa !1169
  %i66 = load i8, ptr %i65, align 1, !tbaa !1169, !range !916, !noundef !917
  %i67 = icmp eq i8 %i66, 0
  br i1 %i67, label %bb74, label %bb68

bb68:                                             ; preds = %bb64
  %i69 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 7, !intel-tbaa !1170
  %i70 = load i8, ptr %i69, align 1, !tbaa !1170, !range !916, !noundef !917
  %i71 = icmp eq i8 %i70, 0
  br i1 %i71, label %bb72, label %bb74

bb72:                                             ; preds = %bb68
  %i73 = call ptr @__cxa_allocate_exception(i64 4) #36
  store i32 %arg1, ptr %i73, align 16, !tbaa !1171
  call void @__cxa_throw(ptr nonnull %i73, ptr nonnull @_ZTIN11xercesc_2_78XMLValid5CodesE, ptr null) #40
  unreachable

bb74:                                             ; preds = %bb68, %bb64, %bb58
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEm(i64 noundef %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1177 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  %i1 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i, i64 0, i32 0
  %i2 = load ptr, ptr %i1, align 8, !tbaa !861
  %i3 = tail call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i6, label %bb7, label %bb22

bb7:                                              ; preds = %bb
  %i8 = add nuw nsw i64 %arg, 8
  %i9 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i8) #39
          to label %bb24 unwind label %bb10

bb10:                                             ; preds = %bb7
  %i11 = landingpad { ptr, i32 }
          catch ptr null
  %i12 = extractvalue { ptr, i32 } %i11, 0
  %i13 = tail call ptr @__cxa_begin_catch(ptr %i12) #36
  %i14 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i14, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb21 unwind label %bb15

bb15:                                             ; preds = %bb10
  %i16 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb17 unwind label %bb18

bb17:                                             ; preds = %bb15
  resume { ptr, i32 } %i16

bb18:                                             ; preds = %bb15
  %i19 = landingpad { ptr, i32 }
          catch ptr null
  %i20 = extractvalue { ptr, i32 } %i19, 0
  tail call fastcc void @__clang_call_terminate(ptr %i20) #37
  unreachable

bb21:                                             ; preds = %bb10
  unreachable

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb24:                                             ; preds = %bb7
  %i25 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !876
  store ptr %i25, ptr %i9, align 8, !tbaa !876
  %i26 = getelementptr inbounds i8, ptr %i9, i64 8, !intel-tbaa !927
  ret ptr %i26
}

; Function Attrs: uwtable
define hidden fastcc noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1178 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg1, i64 0, i32 0
  %i2 = load ptr, ptr %i, align 8, !tbaa !861
  %i3 = tail call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i6, label %bb7, label %bb22

bb7:                                              ; preds = %bb
  %i8 = add nuw nsw i64 %arg, 8
  %i9 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i8) #39
          to label %bb24 unwind label %bb10

bb10:                                             ; preds = %bb7
  %i11 = landingpad { ptr, i32 }
          catch ptr null
  %i12 = extractvalue { ptr, i32 } %i11, 0
  %i13 = tail call ptr @__cxa_begin_catch(ptr %i12) #36
  %i14 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i14, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #40
          to label %bb21 unwind label %bb15

bb15:                                             ; preds = %bb10
  %i16 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb17 unwind label %bb18

bb17:                                             ; preds = %bb15
  resume { ptr, i32 } %i16

bb18:                                             ; preds = %bb15
  %i19 = landingpad { ptr, i32 }
          catch ptr null
  %i20 = extractvalue { ptr, i32 } %i19, 0
  tail call fastcc void @__clang_call_terminate(ptr %i20) #37
  unreachable

bb21:                                             ; preds = %bb10
  unreachable

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !877, !_Intel.Devirt.Call !878
  unreachable

bb24:                                             ; preds = %bb7
  store ptr %arg1, ptr %i9, align 8, !tbaa !876
  %i25 = getelementptr inbounds i8, ptr %i9, i64 8, !intel-tbaa !927
  ret ptr %i25
}

; Function Attrs: mustprogress nounwind uwtable
define hidden fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg) unnamed_addr #30 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1179 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb12, label %bb1

bb1:                                              ; preds = %bb
  %i2 = getelementptr inbounds i8, ptr %arg, i64 -8, !intel-tbaa !927
  %i3 = load ptr, ptr %i2, align 8, !tbaa !876
  %i4 = load ptr, ptr %i3, align 8, !tbaa !861
  %i5 = tail call i1 @llvm.type.test(ptr %i4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i5)
  %i6 = getelementptr inbounds ptr, ptr %i4, i64 3
  %i7 = load ptr, ptr %i6, align 8
  %i8 = icmp eq ptr %i7, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i8, label %bb11, label %bb9

bb9:                                              ; preds = %bb1
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb10 unwind label %bb13, !intel_dtrans_type !926, !_Intel.Devirt.Call !878

bb10:                                             ; preds = %bb9
  unreachable

bb11:                                             ; preds = %bb1
  tail call void @_ZdlPv(ptr noundef nonnull %i2) #36
  br label %bb12

bb12:                                             ; preds = %bb11, %bb
  ret void

bb13:                                             ; preds = %bb9
  %i14 = landingpad { ptr, i32 }
          catch ptr null
  %i15 = extractvalue { ptr, i32 } %i14, 0
  tail call fastcc void @__clang_call_terminate(ptr %i15) #37
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !1180 dso_local void @_ZNSt9bad_allocD1Ev(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1") unnamed_addr #33

; Function Attrs: nofree norecurse noreturn uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2" %arg, i64 %arg1) unnamed_addr #34 comdat align 2 !intel.dtrans.func.type !1182 !_Intel.Devirt.Target !934 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #36
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !861
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #40
  unreachable
}

; Function Attrs: nofree norecurse noreturn uwtable
define hidden void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr nocapture readnone "intel_dtrans_func_index"="2" %arg1) unnamed_addr #34 comdat align 2 !intel.dtrans.func.type !1184 !_Intel.Devirt.Target !934 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #36
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !861
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #40
  unreachable
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #35

; Function Attrs: mustprogress uwtable
define dso_local void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) unnamed_addr #18 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1185 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !899
  %i3 = load i32, ptr %i, align 4, !tbaa !899
  %i4 = icmp ugt i32 %i3, %arg2
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !903
  %i8 = load ptr, ptr %i7, align 8, !tbaa !903
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1138, i32 noundef 52, i32 noundef 116, ptr noundef %i8)
          to label %bb9 unwind label %bb10

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #40
  unreachable

bb10:                                             ; preds = %bb5
  %i11 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #36
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb
  %i13 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !903
  %i14 = load ptr, ptr %i13, align 8, !tbaa !903
  %i15 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !904
  %i16 = load ptr, ptr %i15, align 8, !tbaa !904
  %i17 = zext i32 %arg2 to i64
  %i18 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i16, i64 %i17, i32 2
  %i19 = load ptr, ptr %i18, align 8, !tbaa !905
  %i20 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i21 = load ptr, ptr %i20, align 8, !tbaa !861
  %i22 = tail call i1 @llvm.type.test(ptr %i21, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i22)
  %i23 = getelementptr inbounds ptr, ptr %i21, i64 3
  %i24 = load ptr, ptr %i23, align 8
  %i25 = icmp eq ptr %i24, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i25, label %bb27, label %bb26

bb26:                                             ; preds = %bb12
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !926, !_Intel.Devirt.Call !878
  unreachable

bb27:                                             ; preds = %bb12
  tail call void @_ZdlPv(ptr noundef %i19) #36
  %i28 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !904
  %i29 = load ptr, ptr %i28, align 8, !tbaa !904
  %i30 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i29, i64 %i17, i32 2
  store ptr %arg1, ptr %i30, align 8, !tbaa !905
  ret void
}

attributes #0 = { nounwind memory(none) }
attributes #1 = { nofree }
attributes #2 = { nofree noinline norecurse noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree noreturn nounwind }
attributes #4 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #5 = { nofree noreturn }
attributes #6 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #10 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #11 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #12 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { mustprogress nofree norecurse noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #18 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #19 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #20 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #21 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #22 = { mustprogress nofree nounwind willreturn memory(argmem: read) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #23 = { mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #24 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #25 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #26 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #27 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "dtrans-vector-size-field"="1" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #28 = { inlinehint uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #29 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #30 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #31 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #32 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #33 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #34 = { nofree norecurse noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #35 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #36 = { nounwind }
attributes #37 = { noreturn nounwind }
attributes #38 = { nounwind willreturn memory(read) }
attributes #39 = { allocsize(0) }
attributes #40 = { noreturn }
attributes #41 = { cold }
attributes #42 = { builtin nounwind }

!intel.dtrans.types = !{!385, !389, !392, !393, !395, !396, !403, !404, !407, !408, !409, !413, !414, !416, !417, !418, !421, !422, !425, !428, !429, !430, !433, !434, !435, !436, !437, !438, !459, !460, !462, !464, !467, !473, !477, !479, !481, !486, !487, !488, !492, !493, !495, !496, !507, !510, !512, !520, !521, !524, !526, !527, !528, !535, !538, !539, !544, !546, !551, !553, !555, !558, !561, !562, !565, !566, !567, !568, !570, !572, !574, !576, !577, !579, !581, !584, !586, !588, !590, !593, !594, !596, !599, !600, !601, !602, !606, !607, !608, !612, !614, !615, !628, !630, !632, !635, !637, !639, !641, !643, !646, !647, !650, !652, !654, !656, !658, !659, !660, !661, !663, !665, !667, !669, !673, !678, !680, !683, !686, !688, !690, !693, !695, !697, !700, !702, !704, !706, !708, !710, !712, !714, !716, !720, !722, !723, !724, !725, !727, !730, !732, !734, !736, !737, !739, !741, !743, !745, !747, !749, !751, !753, !755, !757, !759, !762, !764, !767, !769, !772, !773, !775, !776, !778, !779, !781, !783, !785, !787, !789, !791, !793, !795, !797, !799, !801, !803, !804, !806, !810, !812, !814, !816, !818, !820, !822, !824, !826, !828, !831, !833, !836, !837, !839, !840, !842, !843, !844, !845, !846}
!llvm.ident = !{!847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847, !847}
!llvm.module.flags = !{!848, !849, !850, !851, !852, !853}

!0 = !{!1, i32 0}
!1 = !{!"L", i32 311, !2, !6, !10, !14, !18, !22, !26, !30, !2, !33, !37, !39, !43, !45, !22, !49, !53, !57, !61, !63, !64, !33, !68, !72, !76, !61, !80, !84, !88, !92, !96, !100, !104, !84, !108, !112, !116, !120, !124, !120, !120, !128, !132, !136, !18, !43, !140, !140, !142, !144, !57, !146, !53, !150, !68, !154, !156, !160, !26, !162, !164, !168, !164, !14, !172, !176, !168, !178, !180, !26, !6, !26, !120, !76, !112, !112, !18, !184, !132, !188, !33, !178, !192, !194, !196, !198, !200, !160, !202, !37, !10, !204, !192, !202, !104, !53, !160, !202, !116, !208, !6, !162, !211, !213, !217, !160, !33, !144, !100, !39, !144, !211, !162, !194, !144, !172, !219, !92, !223, !227, !45, !231, !53, !68, !53, !154, !217, !233, !150, !142, !237, !239, !241, !22, !96, !63, !227, !233, !211, !243, !245, !249, !57, !132, !156, !76, !198, !192, !208, !219, !184, !253, !172, !255, !259, !263, !26, !37, !176, !223, !162, !108, !72, !37, !14, !49, !266, !270, !72, !30, !2, !63, !124, !272, !112, !162, !128, !72, !88, !274, !272, !270, !112, !272, !276, !272, !194, !278, !280, !282, !284, !272, !64, !80, !284, !286, !284, !276, !88, !88, !39, !124, !156, !108, !288, !116, !274, !272, !278, !290, !156, !294, !104, !278, !280, !278, !282, !57, !276, !296, !136, !280, !136, !88, !294, !286, !272, !68, !80, !128, !280, !120, !10, !298, !300, !290, !302, !128, !80, !280, !304, !76, !84, !88, !120, !302, !194, !192, !53, !219, !296, !136, !80, !284, !302, !208, !178, !204, !18, !306, !284, !202, !294, !272, !308, !178, !76, !312, !162, !22, !10, !202, !84, !154, !146, !270, !156, !61, !270, !132, !39, !76, !219, !57, !156, !300, !84, !314, !39, !300, !132, !26, !76, !316, !37, !300, !116, !164, !154, !318, !300, !43, !243, !162, !146, !14, !306, !116, !26, !84, !30}
!2 = !{!3, i32 0}
!3 = !{!"L", i32 8, !4, !4, !4, !4, !4, !4, !4, !5}
!4 = !{i16 0, i32 0}
!5 = !{!"A", i32 121, !4}
!6 = !{!7, i32 0}
!7 = !{!"L", i32 2, !8, !9}
!8 = !{!"A", i32 40, !4}
!9 = !{!"A", i32 88, !4}
!10 = !{!11, i32 0}
!11 = !{!"L", i32 2, !12, !13}
!12 = !{!"A", i32 59, !4}
!13 = !{!"A", i32 69, !4}
!14 = !{!15, i32 0}
!15 = !{!"L", i32 2, !16, !17}
!16 = !{!"A", i32 94, !4}
!17 = !{!"A", i32 34, !4}
!18 = !{!19, i32 0}
!19 = !{!"L", i32 2, !20, !21}
!20 = !{!"A", i32 66, !4}
!21 = !{!"A", i32 62, !4}
!22 = !{!23, i32 0}
!23 = !{!"L", i32 2, !24, !25}
!24 = !{!"A", i32 61, !4}
!25 = !{!"A", i32 67, !4}
!26 = !{!27, i32 0}
!27 = !{!"L", i32 2, !28, !29}
!28 = !{!"A", i32 44, !4}
!29 = !{!"A", i32 84, !4}
!30 = !{!31, i32 0}
!31 = !{!"L", i32 6, !4, !4, !4, !4, !4, !32}
!32 = !{!"A", i32 123, !4}
!33 = !{!34, i32 0}
!34 = !{!"L", i32 2, !35, !36}
!35 = !{!"A", i32 100, !4}
!36 = !{!"A", i32 28, !4}
!37 = !{!38, i32 0}
!38 = !{!"L", i32 2, !21, !20}
!39 = !{!40, i32 0}
!40 = !{!"L", i32 2, !41, !42}
!41 = !{!"A", i32 76, !4}
!42 = !{!"A", i32 52, !4}
!43 = !{!44, i32 0}
!44 = !{!"L", i32 2, !25, !24}
!45 = !{!46, i32 0}
!46 = !{!"L", i32 2, !47, !48}
!47 = !{!"A", i32 72, !4}
!48 = !{!"A", i32 56, !4}
!49 = !{!50, i32 0}
!50 = !{!"L", i32 2, !51, !52}
!51 = !{!"A", i32 65, !4}
!52 = !{!"A", i32 63, !4}
!53 = !{!54, i32 0}
!54 = !{!"L", i32 2, !55, !56}
!55 = !{!"A", i32 74, !4}
!56 = !{!"A", i32 54, !4}
!57 = !{!58, i32 0}
!58 = !{!"L", i32 2, !59, !60}
!59 = !{!"A", i32 57, !4}
!60 = !{!"A", i32 71, !4}
!61 = !{!62, i32 0}
!62 = !{!"L", i32 2, !48, !47}
!63 = !{!"A", i32 128, !4}
!64 = !{!65, i32 0}
!65 = !{!"L", i32 2, !66, !67}
!66 = !{!"A", i32 20, !4}
!67 = !{!"A", i32 108, !4}
!68 = !{!69, i32 0}
!69 = !{!"L", i32 2, !70, !71}
!70 = !{!"A", i32 49, !4}
!71 = !{!"A", i32 79, !4}
!72 = !{!73, i32 0}
!73 = !{!"L", i32 2, !74, !75}
!74 = !{!"A", i32 58, !4}
!75 = !{!"A", i32 70, !4}
!76 = !{!77, i32 0}
!77 = !{!"L", i32 2, !78, !79}
!78 = !{!"A", i32 46, !4}
!79 = !{!"A", i32 82, !4}
!80 = !{!81, i32 0}
!81 = !{!"L", i32 2, !82, !83}
!82 = !{!"A", i32 36, !4}
!83 = !{!"A", i32 92, !4}
!84 = !{!85, i32 0}
!85 = !{!"L", i32 2, !86, !87}
!86 = !{!"A", i32 50, !4}
!87 = !{!"A", i32 78, !4}
!88 = !{!89, i32 0}
!89 = !{!"L", i32 2, !90, !91}
!90 = !{!"A", i32 27, !4}
!91 = !{!"A", i32 101, !4}
!92 = !{!93, i32 0}
!93 = !{!"L", i32 2, !94, !95}
!94 = !{!"A", i32 97, !4}
!95 = !{!"A", i32 31, !4}
!96 = !{!97, i32 0}
!97 = !{!"L", i32 2, !98, !99}
!98 = !{!"A", i32 77, !4}
!99 = !{!"A", i32 51, !4}
!100 = !{!101, i32 0}
!101 = !{!"L", i32 2, !102, !103}
!102 = !{!"A", i32 98, !4}
!103 = !{!"A", i32 30, !4}
!104 = !{!105, i32 0}
!105 = !{!"L", i32 2, !106, !107}
!106 = !{!"A", i32 53, !4}
!107 = !{!"A", i32 75, !4}
!108 = !{!109, i32 0}
!109 = !{!"L", i32 2, !110, !111}
!110 = !{!"A", i32 23, !4}
!111 = !{!"A", i32 105, !4}
!112 = !{!113, i32 0}
!113 = !{!"L", i32 2, !114, !115}
!114 = !{!"A", i32 24, !4}
!115 = !{!"A", i32 104, !4}
!116 = !{!117, i32 0}
!117 = !{!"L", i32 2, !118, !119}
!118 = !{!"A", i32 42, !4}
!119 = !{!"A", i32 86, !4}
!120 = !{!121, i32 0}
!121 = !{!"L", i32 2, !122, !123}
!122 = !{!"A", i32 39, !4}
!123 = !{!"A", i32 89, !4}
!124 = !{!125, i32 0}
!125 = !{!"L", i32 2, !126, !127}
!126 = !{!"A", i32 25, !4}
!127 = !{!"A", i32 103, !4}
!128 = !{!129, i32 0}
!129 = !{!"L", i32 2, !130, !131}
!130 = !{!"A", i32 33, !4}
!131 = !{!"A", i32 95, !4}
!132 = !{!133, i32 0}
!133 = !{!"L", i32 2, !134, !135}
!134 = !{!"A", i32 45, !4}
!135 = !{!"A", i32 83, !4}
!136 = !{!137, i32 0}
!137 = !{!"L", i32 2, !138, !139}
!138 = !{!"A", i32 38, !4}
!139 = !{!"A", i32 90, !4}
!140 = !{!141, i32 0}
!141 = !{!"L", i32 2, !127, !126}
!142 = !{!143, i32 0}
!143 = !{!"L", i32 2, !29, !28}
!144 = !{!145, i32 0}
!145 = !{!"L", i32 2, !131, !130}
!146 = !{!147, i32 0}
!147 = !{!"L", i32 2, !148, !149}
!148 = !{!"A", i32 48, !4}
!149 = !{!"A", i32 80, !4}
!150 = !{!151, i32 0}
!151 = !{!"L", i32 2, !152, !153}
!152 = !{!"A", i32 73, !4}
!153 = !{!"A", i32 55, !4}
!154 = !{!155, i32 0}
!155 = !{!"L", i32 2, !87, !86}
!156 = !{!157, i32 0}
!157 = !{!"L", i32 2, !158, !159}
!158 = !{!"A", i32 47, !4}
!159 = !{!"A", i32 81, !4}
!160 = !{!161, i32 0}
!161 = !{!"L", i32 2, !119, !118}
!162 = !{!163, i32 0}
!163 = !{!"L", i32 2, !60, !59}
!164 = !{!165, i32 0}
!165 = !{!"L", i32 2, !166, !167}
!166 = !{!"A", i32 87, !4}
!167 = !{!"A", i32 41, !4}
!168 = !{!169, i32 0}
!169 = !{!"L", i32 2, !170, !171}
!170 = !{!"A", i32 93, !4}
!171 = !{!"A", i32 35, !4}
!172 = !{!173, i32 0}
!173 = !{!"L", i32 2, !174, !175}
!174 = !{!"A", i32 99, !4}
!175 = !{!"A", i32 29, !4}
!176 = !{!177, i32 0}
!177 = !{!"L", i32 2, !83, !82}
!178 = !{!179, i32 0}
!179 = !{!"L", i32 2, !167, !166}
!180 = !{!181, i32 0}
!181 = !{!"L", i32 2, !182, !183}
!182 = !{!"A", i32 120, !4}
!183 = !{!"A", i32 8, !4}
!184 = !{!185, i32 0}
!185 = !{!"L", i32 2, !186, !187}
!186 = !{!"A", i32 111, !4}
!187 = !{!"A", i32 17, !4}
!188 = !{!189, i32 0}
!189 = !{!"L", i32 2, !190, !191}
!190 = !{!"A", i32 113, !4}
!191 = !{!"A", i32 15, !4}
!192 = !{!193, i32 0}
!193 = !{!"L", i32 2, !75, !74}
!194 = !{!195, i32 0}
!195 = !{!"L", i32 2, !42, !41}
!196 = !{!197, i32 0}
!197 = !{!"L", i32 2, !79, !78}
!198 = !{!199, i32 0}
!199 = !{!"L", i32 2, !149, !148}
!200 = !{!201, i32 0}
!201 = !{!"L", i32 2, !107, !106}
!202 = !{!203, i32 0}
!203 = !{!"L", i32 2, !99, !98}
!204 = !{!205, i32 0}
!205 = !{!"L", i32 2, !206, !207}
!206 = !{!"A", i32 60, !4}
!207 = !{!"A", i32 68, !4}
!208 = !{!209, i32 0}
!209 = !{!"L", i32 2, !210, !210}
!210 = !{!"A", i32 64, !4}
!211 = !{!212, i32 0}
!212 = !{!"L", i32 2, !111, !110}
!213 = !{!214, i32 0}
!214 = !{!"L", i32 2, !215, !216}
!215 = !{!"A", i32 106, !4}
!216 = !{!"A", i32 22, !4}
!217 = !{!218, i32 0}
!218 = !{!"L", i32 2, !135, !134}
!219 = !{!220, i32 0}
!220 = !{!"L", i32 2, !221, !222}
!221 = !{!"A", i32 43, !4}
!222 = !{!"A", i32 85, !4}
!223 = !{!224, i32 0}
!224 = !{!"L", i32 2, !225, !226}
!225 = !{!"A", i32 119, !4}
!226 = !{!"A", i32 9, !4}
!227 = !{!228, i32 0}
!228 = !{!"L", i32 2, !229, !230}
!229 = !{!"A", i32 96, !4}
!230 = !{!"A", i32 32, !4}
!231 = !{!232, i32 0}
!232 = !{!"L", i32 2, !123, !122}
!233 = !{!234, i32 0}
!234 = !{!"L", i32 2, !235, !236}
!235 = !{!"A", i32 112, !4}
!236 = !{!"A", i32 16, !4}
!237 = !{!238, i32 0}
!238 = !{!"L", i32 2, !115, !114}
!239 = !{!240, i32 0}
!240 = !{!"L", i32 2, !91, !90}
!241 = !{!242, i32 0}
!242 = !{!"L", i32 2, !71, !70}
!243 = !{!244, i32 0}
!244 = !{!"L", i32 2, !159, !158}
!245 = !{!246, i32 0}
!246 = !{!"L", i32 2, !247, !248}
!247 = !{!"A", i32 107, !4}
!248 = !{!"A", i32 21, !4}
!249 = !{!250, i32 0}
!250 = !{!"L", i32 2, !251, !252}
!251 = !{!"A", i32 109, !4}
!252 = !{!"A", i32 19, !4}
!253 = !{!254, i32 0}
!254 = !{!"L", i32 2, !67, !66}
!255 = !{!256, i32 0}
!256 = !{!"L", i32 2, !257, !258}
!257 = !{!"A", i32 102, !4}
!258 = !{!"A", i32 26, !4}
!259 = !{!260, i32 0}
!260 = !{!"L", i32 2, !261, !262}
!261 = !{!"A", i32 115, !4}
!262 = !{!"A", i32 13, !4}
!263 = !{!264, i32 0}
!264 = !{!"L", i32 4, !4, !4, !4, !265}
!265 = !{!"A", i32 125, !4}
!266 = !{!267, i32 0}
!267 = !{!"L", i32 2, !268, !269}
!268 = !{!"A", i32 91, !4}
!269 = !{!"A", i32 37, !4}
!270 = !{!271, i32 0}
!271 = !{!"L", i32 2, !269, !268}
!272 = !{!273, i32 0}
!273 = !{!"L", i32 2, !258, !257}
!274 = !{!275, i32 0}
!275 = !{!"L", i32 2, !252, !251}
!276 = !{!277, i32 0}
!277 = !{!"L", i32 2, !175, !174}
!278 = !{!279, i32 0}
!279 = !{!"L", i32 2, !216, !215}
!280 = !{!281, i32 0}
!281 = !{!"L", i32 2, !103, !102}
!282 = !{!283, i32 0}
!283 = !{!"L", i32 2, !248, !247}
!284 = !{!285, i32 0}
!285 = !{!"L", i32 2, !171, !170}
!286 = !{!287, i32 0}
!287 = !{!"L", i32 2, !17, !16}
!288 = !{!289, i32 0}
!289 = !{!"L", i32 2, !236, !235}
!290 = !{!291, i32 0}
!291 = !{!"L", i32 2, !292, !293}
!292 = !{!"A", i32 18, !4}
!293 = !{!"A", i32 110, !4}
!294 = !{!295, i32 0}
!295 = !{!"L", i32 2, !230, !229}
!296 = !{!297, i32 0}
!297 = !{!"L", i32 2, !95, !94}
!298 = !{!299, i32 0}
!299 = !{!"L", i32 2, !187, !186}
!300 = !{!301, i32 0}
!301 = !{!"L", i32 2, !52, !51}
!302 = !{!303, i32 0}
!303 = !{!"L", i32 2, !36, !35}
!304 = !{!305, i32 0}
!305 = !{!"L", i32 2, !222, !221}
!306 = !{!307, i32 0}
!307 = !{!"L", i32 2, !153, !152}
!308 = !{!309, i32 0}
!309 = !{!"L", i32 2, !310, !311}
!310 = !{!"A", i32 14, !4}
!311 = !{!"A", i32 114, !4}
!312 = !{!313, i32 0}
!313 = !{!"L", i32 2, !56, !55}
!314 = !{!315, i32 0}
!315 = !{!"L", i32 2, !13, !12}
!316 = !{!317, i32 0}
!317 = !{!"L", i32 2, !191, !190}
!318 = !{!319, i32 0}
!319 = !{!"L", i32 2, !139, !138}
!320 = !{!321, i32 0}
!321 = !{!"L", i32 401, !2, !208, !142, !30, !2, !270, !270, !84, !136, !178, !270, !270, !84, !312, !302, !156, !116, !296, !76, !156, !272, !124, !219, !286, !132, !272, !312, !300, !192, !6, !112, !178, !112, !276, !219, !280, !276, !302, !128, !128, !132, !72, !270, !162, !276, !80, !128, !296, !84, !26, !278, !282, !64, !278, !108, !276, !204, !178, !26, !178, !132, !219, !26, !300, !280, !84, !6, !294, !120, !219, !116, !68, !72, !53, !132, !306, !146, !76, !286, !178, !146, !104, !194, !132, !57, !296, !296, !272, !302, !270, !300, !306, !219, !150, !72, !178, !76, !76, !294, !80, !296, !6, !272, !272, !80, !88, !219, !72, !76, !76, !80, !116, !26, !116, !26, !76, !272, !306, !26, !142, !112, !296, !150, !302, !88, !316, !68, !316, !194, !92, !61, !219, !76, !80, !124, !288, !76, !84, !6, !274, !316, !61, !288, !120, !192, !302, !272, !302, !272, !272, !37, !270, !146, !49, !306, !314, !312, !45, !120, !202, !276, !294, !294, !156, !84, !84, !322, !322, !53, !312, !108, !204, !196, !241, !241, !196, !241, !39, !39, !241, !49, !53, !324, !53, !276, !276, !276, !276, !128, !80, !146, !312, !266, !266, !192, !192, !192, !192, !314, !53, !160, !231, !96, !160, !160, !96, !96, !53, !160, !96, !96, !53, !160, !231, !96, !204, !204, !204, !204, !196, !196, !196, !314, !314, !314, !314, !18, !49, !57, !57, !202, !49, !282, !136, !116, !156, !314, !57, !270, !270, !322, !150, !322, !286, !162, !96, !22, !68, !208, !194, !120, !116, !294, !280, !108, !294, !296, !270, !10, !296, !272, !286, !116, !88, !272, !26, !68, !132, !286, !280, !294, !108, !204, !204, !178, !136, !208, !22, !202, !120, !286, !64, !280, !280, !296, !294, !146, !272, !108, !284, !274, !290, !128, !326, !274, !156, !108, !204, !263, !156, !112, !286, !72, !116, !296, !241, !57, !61, !146, !270, !80, !284, !61, !72, !116, !278, !6, !6, !124, !288, !88, !278, !64, !278, !124, !26, !136, !80, !132, !156, !132, !10, !112, !282, !112, !282, !146, !302, !156, !270, !80, !280, !276, !284, !112, !146, !116, !6, !286, !80, !80, !80, !318, !43, !198, !33, !164, !33, !184, !196, !266, !213, !144, !160, !150, !84, !76, !156, !6, !192, !72, !43, !196, !200, !202, !72, !276, !178, !286, !156, !194, !298, !284, !306, !312, !298, !286, !294, !80, !286, !284, !136, !88, !274, !76, !270, !282, !124, !112, !202, !30}
!322 = !{!323, i32 0}
!323 = !{!"L", i32 2, !207, !206}
!324 = !{!325, i32 0}
!325 = !{!"L", i32 2, !9, !8}
!326 = !{!327, i32 0}
!327 = !{!"L", i32 2, !262, !261}
!328 = !{!329, i32 0}
!329 = !{!"L", i32 114, !2, !282, !88, !146, !284, !178, !202, !219, !68, !120, !26, !280, !194, !202, !61, !146, !68, !255, !68, !63, !84, !132, !266, !10, !146, !241, !72, !276, !108, !278, !150, !290, !61, !300, !6, !80, !314, !26, !63, !284, !132, !45, !219, !136, !296, !136, !284, !84, !286, !280, !282, !282, !120, !294, !296, !274, !270, !26, !26, !278, !296, !64, !302, !57, !6, !80, !274, !84, !172, !200, !192, !128, !263, !39, !104, !211, !306, !88, !128, !80, !164, !239, !154, !330, !272, !194, !112, !100, !84, !314, !100, !146, !243, !192, !284, !136, !172, !84, !49, !241, !304, !324, !211, !150, !192, !22, !37, !255, !57, !100, !10, !132, !96, !30}
!330 = !{!331, i32 0}
!331 = !{!"L", i32 2, !293, !292}
!332 = !{!333, i32 0}
!333 = !{!"L", i32 25, !2, !326, !208, !61, !37, !43, !146, !61, !53, !200, !96, !162, !314, !178, !37, !255, !45, !259, !326, !208, !63, !282, !280, !278, !30}
!334 = !{%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 1}
!335 = !{%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 1}
!336 = !{%"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" zeroinitializer, i32 1}
!337 = !{%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" zeroinitializer, i32 1}
!338 = !{!"A", i32 7, !4}
!339 = !{!"L", i32 2, !340, !340}
!340 = !{i8 0, i32 1}
!341 = !{!"L", i32 3, !340, !340, !340}
!342 = !{i64 16, !"_ZTSSt9bad_alloc"}
!343 = !{i64 32, !"_ZTSMSt9bad_allocKFPKcvE.virtual"}
!344 = !{i64 16, !"_ZTSSt9exception"}
!345 = !{i64 32, !"_ZTSMSt9exceptionKFPKcvE.virtual"}
!346 = !{!"L", i32 1, !347}
!347 = !{!"A", i32 5, !340}
!348 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}
!349 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!350 = !{!"A", i32 1, !4}
!351 = !{i32 16, !"_ZTSN11xercesc_2_712XMLMsgLoaderE"}
!352 = !{i32 32, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjE.virtual"}
!353 = !{i32 40, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjPKtS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!354 = !{i32 48, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjPKcS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!355 = !{i32 56, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEKFPKtvE.virtual"}
!356 = !{i32 16, !"_ZTSN11xercesc_2_714InMemMsgLoaderE"}
!357 = !{i32 32, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjE.virtual"}
!358 = !{i32 40, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjPKtS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!359 = !{i32 48, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjPKcS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!360 = !{i32 56, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEKFPKtvE.virtual"}
!361 = !{!"A", i32 8, !340}
!362 = !{i32 16, !"_ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE"}
!363 = !{i32 32, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvPS1_jE.virtual"}
!364 = !{i32 40, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!365 = !{i32 48, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvjE.virtual"}
!366 = !{i32 56, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!367 = !{i32 64, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!368 = !{i32 16, !"_ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE"}
!369 = !{i32 32, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvPS1_jE.virtual"}
!370 = !{i32 40, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!371 = !{i32 48, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvjE.virtual"}
!372 = !{i32 56, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!373 = !{i32 64, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!374 = !{!"A", i32 9, !340}
!375 = !{i32 16, !"_ZTSN11xercesc_2_712XMLExceptionE"}
!376 = !{i32 32, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPKtvE.virtual"}
!377 = !{i32 40, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPS0_vE.virtual"}
!378 = !{i32 16, !"_ZTSN11xercesc_2_722NoSuchElementExceptionE"}
!379 = !{i32 32, !"_ZTSMN11xercesc_2_722NoSuchElementExceptionEKFPKtvE.virtual"}
!380 = !{i32 40, !"_ZTSMN11xercesc_2_722NoSuchElementExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!381 = !{!"A", i32 6, !340}
!382 = !{i32 16, !"_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE"}
!383 = !{i32 32, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPKtvE.virtual"}
!384 = !{i32 40, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!385 = !{!"S", %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 1, !386}
!386 = !{!387, i32 2}
!387 = !{!"F", i1 true, i32 0, !388}
!388 = !{i32 0, i32 0}
!389 = !{!"S", %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 2, !390, !391}
!390 = !{%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 0}
!391 = !{i16 0, i32 1}
!392 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !386}
!393 = !{!"S", %"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1, !394}
!394 = !{i8 0, i32 0}
!395 = !{!"S", %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 1, !386}
!396 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !388, !340, !340, !340, !340, !340, !340, !340, !340, !340, !340, !340, !397, !348, !388, !388, !398, !4, !394, !399, !340, !398, !400, !401, !348, !340, !398, !388, !402}
!397 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}
!398 = !{i64 0, i32 0}
!399 = !{!"A", i32 1, !394}
!400 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}
!401 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}
!402 = !{!"A", i32 20, !394}
!403 = !{!"S", %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 6, !386, !388, !340, !388, !391, !349}
!404 = !{!"S", %"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 16, !405, !406, !406, !406, !406, !406, !406, !406, !406, !406, !406, !406, !406, !406, !406, !349}
!405 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!406 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 1}
!407 = !{!"S", %"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 3, !386, !4, !349}
!408 = !{!"S", %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" zeroinitializer, i32 1, !340}
!409 = !{!"S", %"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 10, !410, !394, !394, !388, !388, !388, !411, !411, !412, !349}
!410 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 0}
!411 = !{i32 0, i32 1}
!412 = !{%"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 1}
!413 = !{!"S", %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 1, !386}
!414 = !{!"S", %"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 2, !415, !349}
!415 = !{%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 0}
!416 = !{!"S", %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 1, !386}
!417 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 10, !415, !388, !388, !391, !391, !391, !391, !391, !391, !349}
!418 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 6, !386, !388, !419, !420, !349, !388}
!419 = !{!"A", i32 4, !394}
!420 = !{%"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 1}
!421 = !{!"S", %"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1, !386}
!422 = !{!"S", %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 6, !415, !349, !423, !424, !388, !388}
!423 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2}
!424 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!425 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 7, !415, !349, !426, !388, !388, !394, !427}
!426 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 1}
!427 = !{!"A", i32 7, !394}
!428 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 6, !415, !349, !426, !388, !388, !394}
!429 = !{!"S", %"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 9, !415, !388, !388, !388, !388, !391, !391, !391, !349}
!430 = !{!"S", %"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 11, !415, !349, !426, !431, !432, !432, !388, !394, !394, !388, !388}
!431 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 1}
!432 = !{%"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 1}
!433 = !{!"S", %"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 10, !415, !388, !388, !388, !394, !394, !388, !391, !391, !349}
!434 = !{!"S", %"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1, !386}
!435 = !{!"S", %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1, !386}
!436 = !{!"S", %"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1, !386}
!437 = !{!"S", %"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1, !386}
!438 = !{!"S", %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 72, !439, !398, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !394, !388, !388, !388, !388, !388, !388, !388, !388, !440, !388, !388, !388, !388, !388, !441, !442, !443, !444, !445, !446, !447, !448, !449, !394, !450, !451, !388, !452, !349, !453, !453, !454, !391, !391, !391, !455, !388, !349, !456, !457, !457, !457, !457, !457, !457, !457, !458}
!439 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 0}
!440 = !{i32 0, i32 2}
!441 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!442 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!443 = !{%"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1}
!444 = !{%"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1}
!445 = !{%"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1}
!446 = !{%"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1}
!447 = !{%"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1}
!448 = !{%"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1}
!449 = !{%"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 1}
!450 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 0}
!451 = !{%"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 1}
!452 = !{%"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 1}
!453 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1}
!454 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 1}
!455 = !{%"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 1}
!456 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 0}
!457 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 0}
!458 = !{%"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 0}
!459 = !{!"S", %"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1, !386}
!460 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !461}
!461 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!462 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !463, !349}
!463 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 2}
!464 = !{!"S", %"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 11, !394, !465, !388, !388, !419, !391, !426, !349, !466, !394, !427}
!465 = !{!"A", i32 3, !394}
!466 = !{%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 1}
!467 = !{!"S", %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 11, !468, !469, !470, !445, !471, !388, !472, !394, !388, !394, !349}
!468 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 0}
!469 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 1}
!470 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 1}
!471 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!472 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!473 = !{!"S", %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 5, !386, !474, !446, !475, !476}
!474 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 1}
!475 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 1}
!476 = !{%"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 1}
!477 = !{!"S", %"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 3, !388, !349, !478}
!478 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 2}
!479 = !{!"S", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 7, !388, !388, !388, !394, !349, !480, !391}
!480 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1}
!481 = !{!"S", %"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 12, !394, !394, !394, !454, !482, !482, !483, !349, !484, !420, !420, !485}
!482 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!483 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 1}
!484 = !{%"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 1}
!485 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!486 = !{!"S", %"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 4, !386, !349, !394, !427}
!487 = !{!"S", %"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 3, !386, !388, !419}
!488 = !{!"S", %"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 13, !388, !388, !489, !490, !388, !388, !388, !388, !388, !388, !388, !491, !349}
!489 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 0}
!490 = !{%"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 2}
!491 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!492 = !{!"S", %"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 8, !415, !388, !388, !391, !391, !391, !391, !349}
!493 = !{!"S", %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1, !494}
!494 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 0}
!495 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 7, !386, !388, !419, !420, !349, !388, !419}
!496 = !{!"S", %"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 14, !349, !497, !498, !499, !501, !454, !503, !504, !505, !498, !420, !394, !394, !506}
!497 = !{%"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1}
!498 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!499 = !{!"A", i32 14, !500}
!500 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!501 = !{!"A", i32 14, !502}
!502 = !{%"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 1}
!503 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!504 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!505 = !{%"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 1}
!506 = !{!"A", i32 6, !394}
!507 = !{!"S", %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 19, !415, !394, !394, !394, !394, !4, !388, !388, !388, !388, !388, !466, !508, !391, !509, !391, !391, !391, !349}
!508 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!509 = !{%"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 1}
!510 = !{!"S", %"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1, !511}
!511 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!512 = !{!"S", %"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 24, !513, !465, !388, !388, !388, !388, !388, !388, !388, !388, !419, !391, !514, !515, !514, !466, !516, !517, !518, !466, !394, !394, !394, !519}
!513 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 0}
!514 = !{%"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 1}
!515 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!516 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!517 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 1}
!518 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 1}
!519 = !{!"A", i32 5, !394}
!520 = !{!"S", %"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2, !388, !391}
!521 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !522, !388, !388, !388, !523}
!522 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!523 = !{%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1}
!524 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !349, !394, !525, !388, !388, !523}
!525 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!526 = !{!"S", %"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 2, !386, !349}
!527 = !{!"S", %"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1, !415}
!528 = !{!"S", %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 32, !388, !529, !388, !530, !531, !398, !398, !388, !391, !394, !394, !391, !388, !532, !388, !388, !388, !394, !388, !388, !394, !394, !391, !533, !394, !394, !534, !388, !340, !394, !388, !349}
!529 = !{!"A", i32 16384, !4}
!530 = !{!"A", i32 16384, !394}
!531 = !{!"A", i32 16384, !388}
!532 = !{!"A", i32 49152, !394}
!533 = !{%"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1}
!534 = !{%"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 1}
!535 = !{!"S", %"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 17, !431, !388, !388, !388, !536, !537, !388, !388, !394, !394, !394, !388, !453, !388, !391, !388, !388}
!536 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 2}
!537 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 1}
!538 = !{!"S", %"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1, !386}
!539 = !{!"S", %"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 15, !394, !394, !388, !388, !388, !388, !540, !391, !391, !541, !406, !412, !542, !543, !349}
!540 = !{%"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 1}
!541 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 1}
!542 = !{%"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 0}
!543 = !{%"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 1}
!544 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !545, !349}
!545 = !{i16 0, i32 2}
!546 = !{!"S", %"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 30, !415, !394, !394, !394, !394, !394, !388, !388, !388, !388, !388, !388, !388, !388, !391, !391, !391, !466, !466, !514, !432, !517, !547, !548, !515, !549, !391, !411, !550, !349}
!547 = !{%"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 1}
!548 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!549 = !{%"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1}
!550 = !{%"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 1}
!551 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !349, !394, !552, !388, !388, !523}
!552 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!553 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !554}
!554 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!555 = !{!"S", %"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 11, !556, !388, !388, !388, !388, !426, !466, !466, !466, !557, !517}
!556 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 0}
!557 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!558 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !559, !560, !340}
!559 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 1}
!560 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!561 = !{!"S", %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1, !386}
!562 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !563, !564, !340, !388, !419}
!563 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 1}
!564 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!565 = !{!"S", %"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2, !388, !388}
!566 = !{!"S", %"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 6, !394, !388, !411, !391, !391, !349}
!567 = !{!"S", %"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 4, !386, !349, !4, !541}
!568 = !{!"S", %"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 2, !569, !349}
!569 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!570 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !571}
!571 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!572 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !517, !573, !340, !388, !419}
!573 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!574 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !575, !349}
!575 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 2}
!576 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !411, !349}
!577 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !578}
!578 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!579 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !580, !349}
!580 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 2}
!581 = !{!"S", %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 8, !415, !391, !391, !582, !583, !349, !388, !419}
!582 = !{%"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 1}
!583 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!584 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !585, !349}
!585 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 2}
!586 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !587}
!587 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!588 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !589, !349}
!589 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 2}
!590 = !{!"S", %"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 3, !415, !591, !592}
!591 = !{%"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 1}
!592 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 1}
!593 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !411, !349}
!594 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !595, !388, !388, !388, !523}
!595 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!596 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !597, !598, !340}
!597 = !{%"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 1}
!598 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!599 = !{!"S", %"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 6, !415, !398, !398, !391, !391, !349}
!600 = !{!"S", %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" zeroinitializer, i32 1, !494}
!601 = !{!"S", %"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1, !386}
!602 = !{!"S", %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" zeroinitializer, i32 3, !603, !337, !337}
!603 = !{!604, i32 1}
!604 = !{!"F", i1 false, i32 0, !605}
!605 = !{!"void", i32 0}
!606 = !{!"S", %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet" zeroinitializer, i32 6, !388, !388, !388, !388, !340, !349}
!607 = !{!"S", %"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 5, !468, !398, !398, !391, !391}
!608 = !{!"S", %"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 6, !609, !610, !515, !611, !388, !388}
!609 = !{%"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 0}
!610 = !{%"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 1}
!611 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 2}
!612 = !{!"S", %"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 7, !613, !394, !573, !388, !515, !349, !340}
!613 = !{%"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 0}
!614 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 1, !386}
!615 = !{!"S", %"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 18, !616, !391, !617, !617, !617, !618, !619, !620, !621, !622, !623, !624, !449, !349, !625, !626, !394, !627}
!616 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 0}
!617 = !{%"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 1}
!618 = !{%"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 1}
!619 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!620 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!621 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!622 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!623 = !{%"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 1}
!624 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!625 = !{%"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1}
!626 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!627 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 0}
!628 = !{!"S", %"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 3, !415, !629, !349}
!629 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!630 = !{!"S", %"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 8, !349, !394, !631, !388, !523, !580, !388, !388}
!631 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 2}
!632 = !{!"S", %"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 7, !349, !633, !634, !388, !388, !388, !419}
!633 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2}
!634 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 2}
!635 = !{!"S", %"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1, !636}
!636 = !{%"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 0}
!637 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !638, !388, !388, !388, !523}
!638 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!639 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !640, !388, !388, !388, !523}
!640 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!641 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 5, !518, !642, !340, !388, !388}
!642 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 1}
!643 = !{!"S", %"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2, !644, !645}
!644 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 1}
!645 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 1}
!646 = !{!"S", %"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 2, !415, !349}
!647 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !648, !649, !340}
!648 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 1}
!649 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!650 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !466, !651, !340}
!651 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!652 = !{!"S", %"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 7, !415, !653, !391, !648, !391, !388, !388}
!653 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 0}
!654 = !{!"S", %"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 5, !415, !388, !391, !655, !349}
!655 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!656 = !{!"S", %"class._ZTSN11xercesc_2_714HashCMStateSetE.xercesc_2_7::HashCMStateSet" zeroinitializer, i32 1, !657}
!657 = !{%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 0}
!658 = !{!"S", %"class._ZTSN11xercesc_2_77HashPtrE.xercesc_2_7::HashPtr" zeroinitializer, i32 1, !657}
!659 = !{!"S", %"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1, !386}
!660 = !{!"S", %"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 4, !386, !388, !391, !349}
!661 = !{!"S", %"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1, !662}
!662 = !{%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 0}
!663 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !664, !349}
!664 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2}
!665 = !{!"S", %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf" zeroinitializer, i32 4, !349, !666, !388, !523}
!666 = !{%"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 2}
!667 = !{!"S", %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 3, !394, !668, !340}
!668 = !{%"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 1}
!669 = !{!"S", %"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator" zeroinitializer, i32 4, !670, !671, !672, !349}
!670 = !{%"class._ZTSN11xercesc_2_715ValueStoreCacheE.xercesc_2_7::ValueStoreCache" zeroinitializer, i32 1}
!671 = !{%"class._ZTSN11xercesc_2_717XPathMatcherStackE.xercesc_2_7::XPathMatcherStack" zeroinitializer, i32 1}
!672 = !{%"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf" zeroinitializer, i32 1}
!673 = !{!"S", %"class._ZTSN11xercesc_2_715ValueStoreCacheE.xercesc_2_7::ValueStoreCache" zeroinitializer, i32 6, !674, !675, !676, !677, !476, !349}
!674 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_10ValueStoreEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!675 = !{%"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!676 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!677 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!678 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !349, !394, !679, !388, !388, !523}
!679 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!680 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !681, !682, !340, !388, !419}
!681 = !{%"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 1}
!682 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!683 = !{!"S", %"class._ZTSN11xercesc_2_717XPathMatcherStackE.xercesc_2_7::XPathMatcherStack" zeroinitializer, i32 3, !388, !684, !685}
!684 = !{%"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 1}
!685 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!686 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !687, !349}
!687 = !{%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 2}
!688 = !{!"S", %"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 9, !386, !388, !411, !411, !411, !689, !655, !592, !349}
!689 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!690 = !{!"S", %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 8, !394, !388, !592, !691, !692, !681, !476, !349}
!691 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 0}
!692 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!693 = !{!"S", %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 2, !694, !349}
!694 = !{%"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!695 = !{!"S", %"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 6, !349, !394, !696, !388, !388, !523}
!696 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!697 = !{!"S", %"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 2, !698, !699}
!698 = !{!"A", i32 8, !394}
!699 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 0}
!700 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !701}
!701 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!702 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !703}
!703 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!704 = !{!"S", %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !705, !349}
!705 = !{%"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 1}
!706 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !681, !707, !340}
!707 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!708 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !709, !349}
!709 = !{%"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 2}
!710 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !711, !388, !388, !388, !523}
!711 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!712 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !453, !713, !340}
!713 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!714 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !715, !349}
!715 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 2}
!716 = !{!"S", %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" zeroinitializer, i32 4, !717, !681, !718, !719}
!717 = !{%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 0}
!718 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 1}
!719 = !{%"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator" zeroinitializer, i32 1}
!720 = !{!"S", %"class._ZTSN11xercesc_2_76IC_KeyE.xercesc_2_7::IC_Key" zeroinitializer, i32 2, !721, !419}
!721 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base" zeroinitializer, i32 0}
!722 = !{!"S", %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base" zeroinitializer, i32 7, !415, !391, !391, !582, !583, !349, !388}
!723 = !{!"S", %"class._ZTSN11xercesc_2_79IC_KeyRefE.xercesc_2_7::IC_KeyRef" zeroinitializer, i32 2, !721, !592}
!724 = !{!"S", %"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 3, !415, !591, !592}
!725 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !726, !388, !388, !388, !523}
!726 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!727 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !728, !729, !340}
!728 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 1}
!729 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!730 = !{!"S", %"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1, !731}
!731 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 0}
!732 = !{!"S", %"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 6, !388, !388, !388, !489, !733, !349}
!733 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 2}
!734 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 3, !735, !388, !388}
!735 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 1}
!736 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 2, !388, !388}
!737 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !738}
!738 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!739 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !740, !349}
!740 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 2}
!741 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !698, !742}
!742 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!743 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !744}
!744 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!745 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !746, !349}
!746 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 2}
!747 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !698, !748}
!748 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!749 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !750}
!750 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!751 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !752, !349}
!752 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 2}
!753 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !754, !388, !388, !388, !523}
!754 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!755 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !514, !756, !340}
!756 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!757 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !758, !388, !388, !388, !523}
!758 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!759 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !760, !761, !340}
!760 = !{%"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 1}
!761 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!762 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !763, !388, !388, !388, !523}
!763 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!764 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !765, !766, !340}
!765 = !{%"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 1}
!766 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!767 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !349, !394, !768, !388, !388, !523}
!768 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!769 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !770, !771, !340, !388, !419}
!770 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!771 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!772 = !{!"S", %"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 9, !415, !394, !388, !388, !388, !432, !548, !760, !550}
!773 = !{!"S", %"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 8, !415, !394, !388, !388, !774, !774, !517, !349}
!774 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!775 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !394, !388, !388, !580, !349}
!776 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !777}
!777 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!778 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !611, !349}
!779 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !780}
!780 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!781 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !782, !349}
!782 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 2}
!783 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_10ValueStoreEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !784}
!784 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!785 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !786, !349}
!786 = !{%"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 2}
!787 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !698, !788}
!788 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!789 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !790}
!790 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!791 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !792, !349}
!792 = !{%"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 2}
!793 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !794}
!794 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!795 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !796, !349}
!796 = !{%"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2}
!797 = !{!"S", %"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2, !415, !798}
!798 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!799 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !800, !349}
!800 = !{%"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 2}
!801 = !{!"S", %"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 3, !415, !4, !802}
!802 = !{%"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 1}
!803 = !{!"S", %"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 3, !415, !4, !426}
!804 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !805}
!805 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!806 = !{!"S", %"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 7, !349, !807, !420, !501, !503, !808, !391}
!807 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 1}
!808 = !{!"A", i32 14, !809}
!809 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!810 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !811}
!811 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!812 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !813, !349}
!813 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 2}
!814 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !815}
!815 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!816 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !817, !349}
!817 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 2}
!818 = !{!"S", %"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 4, !349, !454, !500, !819}
!819 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!820 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !821}
!821 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!822 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !386, !394, !388, !388, !823, !349}
!823 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 2}
!824 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !349, !394, !825, !388, !388, !523}
!825 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!826 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !827, !388, !388, !388, !523}
!827 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!828 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !829, !830, !340}
!829 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 1}
!830 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!831 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !349, !394, !832, !388, !388, !388, !523}
!832 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!833 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !834, !835, !340}
!834 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 1}
!835 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!836 = !{!"S", %"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 3, !349, !809, !500}
!837 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !829, !838, !340, !388, !419}
!838 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!839 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1, !731}
!840 = !{!"S", %"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1, !841}
!841 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}
!842 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !386}
!843 = !{!"S", %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 3, !718, !466, !391}
!844 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 -1}
!845 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 -1}
!846 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 -1}
!847 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!848 = !{i32 1, !"wchar_size", i32 4}
!849 = !{i32 1, !"Virtual Function Elim", i32 0}
!850 = !{i32 7, !"uwtable", i32 2}
!851 = !{i32 1, !"ThinLTO", i32 0}
!852 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!853 = !{i32 1, !"LTOPostLink", i32 1}
!854 = distinct !{!340, !340}
!855 = distinct !{!340}
!856 = distinct !{!340, !340, !340}
!857 = distinct !{!340}
!858 = distinct !{!340}
!859 = distinct !{!860, !340, !349}
!860 = !{%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1}
!861 = !{!862, !862, i64 0}
!862 = !{!"vtable pointer", !863, i64 0}
!863 = !{!"Simple C++ TBAA"}
!864 = !{!865, !866, i64 8}
!865 = !{!"struct@_ZTSN11xercesc_2_712XMLExceptionE", !866, i64 8, !868, i64 16, !869, i64 24, !870, i64 32, !871, i64 40}
!866 = !{!"_ZTSN11xercesc_2_710XMLExcepts5CodesE", !867, i64 0}
!867 = !{!"omnipotent char", !863, i64 0}
!868 = !{!"pointer@_ZTSPc", !867, i64 0}
!869 = !{!"int", !867, i64 0}
!870 = !{!"pointer@_ZTSPt", !867, i64 0}
!871 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !867, i64 0}
!872 = !{!865, !868, i64 16}
!873 = !{!865, !869, i64 24}
!874 = !{!865, !870, i64 32}
!875 = !{!865, !871, i64 40}
!876 = !{!871, !871, i64 0}
!877 = !{!"F", i1 false, i32 2, !340, !349, !398}
!878 = !{!"_Intel.Devirt.Call"}
!879 = distinct !{!880, !340, !349}
!880 = !{%"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" zeroinitializer, i32 1}
!881 = distinct !{!340}
!882 = distinct !{!883, !340, !349}
!883 = !{%"class._ZTSN11xercesc_2_714HashCMStateSetE.xercesc_2_7::HashCMStateSet" zeroinitializer, i32 1}
!884 = distinct !{!883, !340, !340}
!885 = distinct !{!886}
!886 = !{%"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1}
!887 = !{!888, !888, i64 0}
!888 = !{!"pointer@_ZTSP8_IO_FILE", !867, i64 0}
!889 = distinct !{!348, !340}
!890 = distinct !{!891, !891}
!891 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 1}
!892 = !{!893, !894, i64 0}
!893 = !{!"struct@_ZTSN11xercesc_2_713FieldValueMapE", !894, i64 0, !895, i64 8, !896, i64 16, !871, i64 24}
!894 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !867, i64 0}
!895 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !867, i64 0}
!896 = !{!"pointer@_ZTSPN11xercesc_2_716RefArrayVectorOfItEE", !867, i64 0}
!897 = !{!893, !896, i64 16}
!898 = !{!893, !871, i64 24}
!899 = !{!900, !869, i64 12}
!900 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfItEE", !901, i64 8, !869, i64 12, !869, i64 16, !902, i64 24, !871, i64 32}
!901 = !{!"bool", !867, i64 0}
!902 = !{!"pointer@_ZTSPPt", !867, i64 0}
!903 = !{!900, !871, i64 32}
!904 = !{!900, !902, i64 24}
!905 = !{!870, !870, i64 0}
!906 = !{!907, !907, i64 0}
!907 = !{!"short", !867, i64 0}
!908 = distinct !{!908, !909}
!909 = !{!"llvm.loop.mustprogress"}
!910 = distinct !{!910, !909}
!911 = !{!"F", i1 false, i32 1, !605, !891}
!912 = distinct !{!694, !694}
!913 = !{!914, !901, i64 0}
!914 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !901, i64 0, !869, i64 4, !869, i64 8, !915, i64 16, !871, i64 24}
!915 = !{!"pointer@_ZTSPPN11xercesc_2_78IC_FieldE", !867, i64 0}
!916 = !{i8 0, i8 2}
!917 = !{}
!918 = !{!914, !869, i64 4}
!919 = !{!914, !869, i64 8}
!920 = !{!914, !915, i64 16}
!921 = !{!914, !871, i64 24}
!922 = !{!923, !923, i64 0}
!923 = !{!"pointer@_ZTSPN11xercesc_2_78IC_FieldE", !867, i64 0}
!924 = distinct !{!924, !909}
!925 = distinct !{!891}
!926 = !{!"F", i1 false, i32 2, !605, !349, !340}
!927 = !{!867, !867, i64 0}
!928 = distinct !{!589, !694}
!929 = distinct !{!930, !340, !349}
!930 = !{%"class._ZTSN11xercesc_2_77HashPtrE.xercesc_2_7::HashPtr" zeroinitializer, i32 1}
!931 = distinct !{!930, !340, !340}
!932 = distinct !{!933, !391, !466}
!933 = !{%"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" zeroinitializer, i32 1}
!934 = !{!"_Intel.Devirt.Target"}
!935 = !{!936, !942, i64 72}
!936 = !{!"struct@_ZTSN11xercesc_2_712FieldMatcherE", !937, i64 0, !942, i64 72, !923, i64 80, !943, i64 88}
!937 = !{!"struct@_ZTSN11xercesc_2_712XPathMatcherE", !869, i64 8, !938, i64 16, !938, i64 24, !938, i64 32, !939, i64 40, !940, i64 48, !941, i64 56, !871, i64 64}
!938 = !{!"pointer@_ZTSPi", !867, i64 0}
!939 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE", !867, i64 0}
!940 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE", !867, i64 0}
!941 = !{!"pointer@_ZTSPN11xercesc_2_718IdentityConstraintE", !867, i64 0}
!942 = !{!"pointer@_ZTSPN11xercesc_2_710ValueStoreE", !867, i64 0}
!943 = !{!"pointer@_ZTSPN11xercesc_2_714FieldActivatorE", !867, i64 0}
!944 = !{!936, !923, i64 80}
!945 = !{!946, !941, i64 16}
!946 = !{!"struct@_ZTSN11xercesc_2_78IC_FieldE", !947, i64 0, !948, i64 8, !941, i64 16}
!947 = !{!"struct@_ZTSN11xercesc_2_713XSerializableE"}
!948 = !{!"pointer@_ZTSPN11xercesc_2_711XercesXPathE", !867, i64 0}
!949 = !{!950, !901, i64 0}
!950 = !{!"struct@_ZTSN11xercesc_2_710ValueStoreE", !901, i64 0, !869, i64 4, !941, i64 8, !893, i64 16, !951, i64 48, !942, i64 56, !952, i64 64, !871, i64 72}
!951 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE", !867, i64 0}
!952 = !{!"pointer@_ZTSPN11xercesc_2_710XMLScannerE", !867, i64 0}
!953 = !{!954, !974, i64 264}
!954 = !{!"struct@_ZTSN11xercesc_2_710XMLScannerE", !955, i64 0, !956, i64 8, !901, i64 16, !901, i64 17, !901, i64 18, !901, i64 19, !901, i64 20, !901, i64 21, !901, i64 22, !901, i64 23, !901, i64 24, !901, i64 25, !901, i64 26, !901, i64 27, !901, i64 28, !901, i64 29, !901, i64 30, !901, i64 31, !901, i64 32, !901, i64 33, !901, i64 34, !901, i64 35, !901, i64 36, !901, i64 37, !901, i64 38, !869, i64 40, !869, i64 44, !869, i64 48, !869, i64 52, !869, i64 56, !869, i64 60, !869, i64 64, !869, i64 68, !957, i64 72, !869, i64 80, !869, i64 84, !869, i64 88, !869, i64 92, !869, i64 96, !958, i64 104, !959, i64 112, !960, i64 120, !961, i64 128, !962, i64 136, !963, i64 144, !964, i64 152, !965, i64 160, !966, i64 168, !901, i64 176, !967, i64 184, !974, i64 264, !975, i64 272, !976, i64 280, !871, i64 288, !977, i64 296, !977, i64 304, !978, i64 312, !870, i64 320, !870, i64 328, !870, i64 336, !979, i64 344, !973, i64 352, !871, i64 360, !980, i64 368, !982, i64 392, !982, i64 432, !982, i64 472, !982, i64 512, !982, i64 552, !982, i64 592, !982, i64 632, !984, i64 672}
!955 = !{!"struct@_ZTSN11xercesc_2_720XMLBufferFullHandlerE"}
!956 = !{!"long", !867, i64 0}
!957 = !{!"pointer@_ZTSPPj", !867, i64 0}
!958 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE", !867, i64 0}
!959 = !{!"pointer@_ZTSPN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE", !867, i64 0}
!960 = !{!"pointer@_ZTSPN11xercesc_2_718XMLDocumentHandlerE", !867, i64 0}
!961 = !{!"pointer@_ZTSPN11xercesc_2_714DocTypeHandlerE", !867, i64 0}
!962 = !{!"pointer@_ZTSPN11xercesc_2_716XMLEntityHandlerE", !867, i64 0}
!963 = !{!"pointer@_ZTSPN11xercesc_2_716XMLErrorReporterE", !867, i64 0}
!964 = !{!"pointer@_ZTSPN11xercesc_2_712ErrorHandlerE", !867, i64 0}
!965 = !{!"pointer@_ZTSPN11xercesc_2_711PSVIHandlerE", !867, i64 0}
!966 = !{!"pointer@_ZTSPN11xercesc_2_717ValidationContextE", !867, i64 0}
!967 = !{!"struct@_ZTSN11xercesc_2_79ReaderMgrE", !968, i64 0, !969, i64 8, !970, i64 16, !962, i64 24, !971, i64 32, !869, i64 40, !972, i64 48, !901, i64 56, !973, i64 60, !901, i64 64, !871, i64 72}
!968 = !{!"struct@_ZTSN11xercesc_2_77LocatorE"}
!969 = !{!"pointer@_ZTSPN11xercesc_2_713XMLEntityDeclE", !867, i64 0}
!970 = !{!"pointer@_ZTSPN11xercesc_2_79XMLReaderE", !867, i64 0}
!971 = !{!"pointer@_ZTSPN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE", !867, i64 0}
!972 = !{!"pointer@_ZTSPN11xercesc_2_710RefStackOfINS_9XMLReaderEEE", !867, i64 0}
!973 = !{!"_ZTSN11xercesc_2_79XMLReader10XMLVersionE", !867, i64 0}
!974 = !{!"pointer@_ZTSPN11xercesc_2_712XMLValidatorE", !867, i64 0}
!975 = !{!"_ZTSN11xercesc_2_710XMLScanner10ValSchemesE", !867, i64 0}
!976 = !{!"pointer@_ZTSPN11xercesc_2_715GrammarResolverE", !867, i64 0}
!977 = !{!"pointer@_ZTSPN11xercesc_2_77GrammarE", !867, i64 0}
!978 = !{!"pointer@_ZTSPN11xercesc_2_713XMLStringPoolE", !867, i64 0}
!979 = !{!"pointer@_ZTSPN11xercesc_2_715SecurityManagerE", !867, i64 0}
!980 = !{!"struct@_ZTSN11xercesc_2_712XMLBufferMgrE", !869, i64 0, !871, i64 8, !981, i64 16}
!981 = !{!"pointer@_ZTSPPN11xercesc_2_79XMLBufferE", !867, i64 0}
!982 = !{!"struct@_ZTSN11xercesc_2_79XMLBufferE", !869, i64 0, !869, i64 4, !869, i64 8, !901, i64 12, !871, i64 16, !983, i64 24, !870, i64 32}
!983 = !{!"pointer@_ZTSPN11xercesc_2_720XMLBufferFullHandlerE", !867, i64 0}
!984 = !{!"struct@_ZTSN11xercesc_2_79ElemStackE", !869, i64 0, !869, i64 4, !985, i64 8, !988, i64 48, !869, i64 56, !869, i64 60, !869, i64 64, !869, i64 68, !869, i64 72, !869, i64 76, !869, i64 80, !989, i64 88, !871, i64 96}
!985 = !{!"struct@_ZTSN11xercesc_2_713XMLStringPoolE", !947, i64 0, !871, i64 8, !986, i64 16, !987, i64 24, !869, i64 32, !869, i64 36}
!986 = !{!"pointer@_ZTSPPN11xercesc_2_713XMLStringPool8PoolElemE", !867, i64 0}
!987 = !{!"pointer@_ZTSPN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE", !867, i64 0}
!988 = !{!"pointer@_ZTSPPN11xercesc_2_79ElemStack9StackElemE", !867, i64 0}
!989 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE", !867, i64 0}
!990 = !{!991, !870, i64 16}
!991 = !{!"struct@_ZTSN11xercesc_2_718IdentityConstraintE", !947, i64 0, !870, i64 8, !870, i64 16, !992, i64 24, !993, i64 32, !871, i64 40, !869, i64 48}
!992 = !{!"pointer@_ZTSPN11xercesc_2_711IC_SelectorE", !867, i64 0}
!993 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE", !867, i64 0}
!994 = !{!936, !943, i64 88}
!995 = !{!996, !999, i64 16}
!996 = !{!"struct@_ZTSN11xercesc_2_714FieldActivatorE", !997, i64 0, !998, i64 8, !999, i64 16, !871, i64 24}
!997 = !{!"pointer@_ZTSPN11xercesc_2_715ValueStoreCacheE", !867, i64 0}
!998 = !{!"pointer@_ZTSPN11xercesc_2_717XPathMatcherStackE", !867, i64 0}
!999 = !{!"pointer@_ZTSPN11xercesc_2_716ValueHashTableOfIbEE", !867, i64 0}
!1000 = !{!1001, !1003, i64 24}
!1001 = !{!"struct@_ZTSN11xercesc_2_716ValueHashTableOfIbEE", !871, i64 0, !1002, i64 8, !869, i64 16, !1003, i64 24}
!1002 = !{!"pointer@_ZTSPPN11xercesc_2_724ValueHashTableBucketElemIbEE", !867, i64 0}
!1003 = !{!"pointer@_ZTSPN11xercesc_2_78HashBaseE", !867, i64 0}
!1004 = !{!1001, !869, i64 16}
!1005 = !{!1006, !869, i64 0}
!1006 = !{!"struct@_ZTSN11xercesc_2_710CMStateSetE", !869, i64 0, !869, i64 4, !869, i64 8, !869, i64 12, !1007, i64 16, !871, i64 24}
!1007 = !{!"pointer@_ZTSPh", !867, i64 0}
!1008 = !{!1006, !869, i64 8}
!1009 = !{!1006, !869, i64 12}
!1010 = !{!1006, !869, i64 4}
!1011 = !{!1006, !1007, i64 16}
!1012 = distinct !{!1012, !909}
!1013 = distinct !{!1013, !909}
!1014 = !{!1001, !1002, i64 8}
!1015 = !{!1016, !1016, i64 0}
!1016 = !{!"pointer@_ZTSPN11xercesc_2_724ValueHashTableBucketElemIbEE", !867, i64 0}
!1017 = !{!1018, !1019, i64 16}
!1018 = !{!"struct@_ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE", !901, i64 0, !1016, i64 8, !1019, i64 16}
!1019 = !{!"pointer@_ZTSPv", !867, i64 0}
!1020 = distinct !{!1020, !909}
!1021 = distinct !{!1021, !909}
!1022 = !{!1018, !1016, i64 8}
!1023 = distinct !{!1023, !909}
!1024 = !{!1018, !901, i64 0}
!1025 = !{!950, !952, i64 64}
!1026 = !{!950, !893, i64 16}
!1027 = distinct !{!1027, !909}
!1028 = !{!893, !895, i64 8}
!1029 = !{!1030, !869, i64 4}
!1030 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !901, i64 0, !869, i64 4, !869, i64 8, !1031, i64 16, !871, i64 24}
!1031 = !{!"pointer@_ZTSPPN11xercesc_2_717DatatypeValidatorE", !867, i64 0}
!1032 = !{!1030, !871, i64 24}
!1033 = !{!1030, !1031, i64 16}
!1034 = !{!1035, !1035, i64 0}
!1035 = !{!"pointer@_ZTSPN11xercesc_2_717DatatypeValidatorE", !867, i64 0}
!1036 = !{!950, !869, i64 4}
!1037 = !{!950, !941, i64 8}
!1038 = !{!950, !951, i64 48}
!1039 = !{!950, !871, i64 72}
!1040 = !{!1041, !901, i64 8}
!1041 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE", !901, i64 8, !869, i64 12, !869, i64 16, !1042, i64 24, !871, i64 32}
!1042 = !{!"pointer@_ZTSPPN11xercesc_2_713FieldValueMapE", !867, i64 0}
!1043 = !{!1041, !869, i64 12}
!1044 = !{!1041, !869, i64 16}
!1045 = !{!1041, !1042, i64 24}
!1046 = !{!1041, !871, i64 32}
!1047 = !{!1048, !1048, i64 0}
!1048 = !{!"pointer@_ZTSPN11xercesc_2_713FieldValueMapE", !867, i64 0}
!1049 = distinct !{!1049, !909}
!1050 = !{!1001, !871, i64 0}
!1051 = distinct !{!1051, !909}
!1052 = distinct !{!1053}
!1053 = !{%"class._ZTSN11xercesc_2_76IC_KeyE.xercesc_2_7::IC_Key" zeroinitializer, i32 1}
!1054 = distinct !{!1055}
!1055 = !{%"class._ZTSN11xercesc_2_79IC_KeyRefE.xercesc_2_7::IC_KeyRef" zeroinitializer, i32 1}
!1056 = distinct !{!340}
!1057 = distinct !{!1058, !391}
!1058 = !{%"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 1}
!1059 = !{!1060, !870, i64 8}
!1060 = !{!"struct@_ZTSN11xercesc_2_714InMemMsgLoaderE", !1061, i64 0, !870, i64 8}
!1061 = !{!"struct@_ZTSN11xercesc_2_712XMLMsgLoaderE"}
!1062 = distinct !{!1062, !909}
!1063 = distinct !{!1063, !909}
!1064 = distinct !{!1065}
!1065 = !{%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1}
!1066 = distinct !{!340, !1067}
!1067 = !{%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1}
!1068 = distinct !{!1067, !340}
!1069 = distinct !{!340}
!1070 = distinct !{!335, !391}
!1071 = !{!1072, !1072, i64 0}
!1072 = !{!"pointer@_ZTSPN11xercesc_2_712PanicHandlerE", !867, i64 0}
!1073 = !{!"F", i1 false, i32 2, !605, !334, !388}
!1074 = distinct !{!1074, !909}
!1075 = distinct !{!470, !475, !746}
!1076 = !{!967, !970, i64 16}
!1077 = !{!967, !969, i64 8}
!1078 = !{!1079, !870, i64 40}
!1079 = !{!"struct@_ZTSN11xercesc_2_713XMLEntityDeclE", !947, i64 0, !869, i64 8, !869, i64 12, !870, i64 16, !870, i64 24, !870, i64 32, !870, i64 40, !870, i64 48, !870, i64 56, !871, i64 64}
!1080 = !{!967, !972, i64 48}
!1081 = !{!1082, !1083, i64 8}
!1082 = !{!"struct@_ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE", !1083, i64 8}
!1083 = !{!"struct@_ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE", !1084, i64 0}
!1084 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE", !901, i64 8, !869, i64 12, !869, i64 16, !1085, i64 24, !871, i64 32}
!1085 = !{!"pointer@_ZTSPPN11xercesc_2_79XMLReaderE", !867, i64 0}
!1086 = !{!1084, !869, i64 12}
!1087 = !{!967, !971, i64 32}
!1088 = distinct !{!1088, !909}
!1089 = !{!969, !969, i64 0}
!1090 = distinct !{!469, !471}
!1091 = !{!1092, !1093, i64 8}
!1092 = !{!"struct@_ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE", !1093, i64 8}
!1093 = !{!"struct@_ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE", !1094, i64 0}
!1094 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE", !901, i64 8, !869, i64 12, !869, i64 16, !1095, i64 24, !871, i64 32}
!1095 = !{!"pointer@_ZTSPPN11xercesc_2_713XMLEntityDeclE", !867, i64 0}
!1096 = !{!1094, !869, i64 12}
!1097 = !{!1094, !871, i64 32}
!1098 = !{!1094, !1095, i64 24}
!1099 = distinct !{!470, !472}
!1100 = !{!1084, !871, i64 32}
!1101 = !{!1084, !1085, i64 24}
!1102 = !{!970, !970, i64 0}
!1103 = distinct !{!466, !891}
!1104 = distinct !{!391, !891}
!1105 = distinct !{!891, !718, !466, !391}
!1106 = !{!"F", i1 false, i32 3, !605, !694, !391, !388}
!1107 = distinct !{!681, !891}
!1108 = distinct !{!1108, !909}
!1109 = distinct !{!1109, !909}
!1110 = !{!"F", i1 false, i32 4, !388, !466, !391, !391, !349}
!1111 = distinct !{!1111, !909}
!1112 = distinct !{!891, !1113}
!1113 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 1}
!1114 = distinct !{!694, !349}
!1115 = distinct !{!694}
!1116 = distinct !{!1116, !909}
!1117 = distinct !{!1118}
!1118 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 1}
!1119 = distinct !{!1118}
!1120 = !{!"A", i32 2048, !4}
!1121 = !{!"F", i1 false, i32 4, !1122, !335, !388, !391, !388}
!1122 = !{i1 false, i32 0}
!1123 = distinct !{!1123, !909}
!1124 = distinct !{!335}
!1125 = !{!1126, !1126, i64 0}
!1126 = !{!"pointer@_ZTSPN11xercesc_2_712XMLMsgLoaderE", !867, i64 0}
!1127 = !{!1128, !1019, i64 0}
!1128 = !{!"struct@_ZTSN11xercesc_2_78XMLMutexE", !1019, i64 0}
!1129 = !{!1130, !1130, i64 0}
!1130 = !{!"pointer@_ZTSPN11xercesc_2_78XMLMutexE", !867, i64 0}
!1131 = !{!1132, !1133, i64 0}
!1132 = !{!"struct@_ZTSN11xercesc_2_718XMLRegisterCleanupE", !1133, i64 0, !1134, i64 8, !1134, i64 16}
!1133 = !{!"pointer@_ZTSPFvvE", !867, i64 0}
!1134 = !{!"pointer@_ZTSPN11xercesc_2_718XMLRegisterCleanupE", !867, i64 0}
!1135 = !{!1132, !1134, i64 8}
!1136 = !{!1134, !1134, i64 0}
!1137 = !{!1132, !1134, i64 16}
!1138 = distinct !{!391, !391, !391, !391, !391, !349}
!1139 = distinct !{!1139, !909}
!1140 = distinct !{!1140, !909}
!1141 = distinct !{!1141, !909}
!1142 = distinct !{!1142, !909}
!1143 = distinct !{!451}
!1144 = !{!"A", i32 1024, !4}
!1145 = !{!1146, !952, i64 32}
!1146 = !{!"struct@_ZTSN11xercesc_2_712XMLValidatorE", !1147, i64 8, !963, i64 16, !1148, i64 24, !952, i64 32}
!1147 = !{!"pointer@_ZTSPN11xercesc_2_712XMLBufferMgrE", !867, i64 0}
!1148 = !{!"pointer@_ZTSPN11xercesc_2_79ReaderMgrE", !867, i64 0}
!1149 = !{!954, !869, i64 40}
!1150 = !{!1146, !963, i64 16}
!1151 = !{!1146, !1148, i64 24}
!1152 = !{!1153, !870, i64 163928}
!1153 = !{!"struct@_ZTSN11xercesc_2_79XMLReaderE", !869, i64 0, !1154, i64 4, !869, i64 32772, !1155, i64 32776, !1156, i64 49160, !956, i64 114696, !956, i64 114704, !1157, i64 114712, !870, i64 114720, !901, i64 114728, !901, i64 114729, !870, i64 114736, !869, i64 114744, !1158, i64 114748, !869, i64 163900, !869, i64 163904, !1159, i64 163908, !901, i64 163912, !1160, i64 163916, !869, i64 163920, !901, i64 163924, !901, i64 163925, !870, i64 163928, !1161, i64 163936, !901, i64 163944, !901, i64 163945, !1162, i64 163952, !1163, i64 163960, !1007, i64 163968, !901, i64 163976, !973, i64 163980, !871, i64 163984}
!1154 = !{!"array@_ZTSA16384_t", !907, i64 0}
!1155 = !{!"array@_ZTSA16384_h", !867, i64 0}
!1156 = !{!"array@_ZTSA16384_j", !869, i64 0}
!1157 = !{!"_ZTSN11xercesc_2_713XMLRecognizer9EncodingsE", !867, i64 0}
!1158 = !{!"array@_ZTSA49152_h", !867, i64 0}
!1159 = !{!"_ZTSN11xercesc_2_79XMLReader7RefFromE", !867, i64 0}
!1160 = !{!"_ZTSN11xercesc_2_79XMLReader7SourcesE", !867, i64 0}
!1161 = !{!"pointer@_ZTSPN11xercesc_2_714BinInputStreamE", !867, i64 0}
!1162 = !{!"pointer@_ZTSPN11xercesc_2_713XMLTranscoderE", !867, i64 0}
!1163 = !{!"_ZTSN11xercesc_2_79XMLReader5TypesE", !867, i64 0}
!1164 = !{!1153, !870, i64 114736}
!1165 = !{!1153, !956, i64 114704}
!1166 = !{!1153, !956, i64 114696}
!1167 = !{!"F", i1 false, i32 9, !605, !446, !388, !391, !388, !391, !391, !391, !398, !398}
!1168 = !{!954, !901, i64 20}
!1169 = !{!954, !901, i64 19}
!1170 = !{!954, !901, i64 21}
!1171 = !{!1172, !1172, i64 0}
!1172 = !{!"_ZTSN11xercesc_2_78XMLValid5CodesE", !867, i64 0}
!1173 = distinct !{!335}
!1174 = distinct !{!451, !391, !391, !391, !391}
!1175 = !{!954, !871, i64 360}
!1176 = !{!"F", i1 false, i32 4, !1122, !1058, !388, !391, !388}
!1177 = distinct !{!340}
!1178 = distinct !{!340, !349}
!1179 = distinct !{!340, !349}
!1180 = distinct !{!1181}
!1181 = !{%"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1}
!1182 = distinct !{!340, !1183}
!1183 = !{%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1}
!1184 = distinct !{!1183, !340}
!1185 = distinct !{!694, !391}

; end INTEL_FEATURE_SW_DTRANS
