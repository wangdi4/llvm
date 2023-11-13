; INTEL_FEATURE_SW_DTRANS
; REQUIRES: asserts, intel_feature_sw_dtrans

; This test checks transformations are not triggered as no targets are
; selected for indirect calls.

; RUN: opt < %s -ippred-skip-indirect-target-heuristic=false -debug-only=ippredopt -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s


; CHECK: No targets selected by heuristics
; CHECK:    Skipped: SideEffects
; CHECK:    Failed: No Candidate

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" = type { ptr, i32, ptr, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
%"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime" = type { %"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber", [8 x i32], [2 x i32], i32, i32, i32, double, i8, ptr, ptr }
%"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" }
%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" = type { ptr }
%"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" = type { %"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" }
%"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" = type { %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", i8, i8, i8, i8, i8, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i8, i8, i8, i16, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal" = type { %"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber", i32, i32, i32, i32, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_724DecimalDatatypeValidatorE.xercesc_2_7::DecimalDatatypeValidator" = type { %"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator", i32, i32 }
%"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" = type { %"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" }
%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" = type { ptr, ptr }
%"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" = type { %"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher", ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
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
%"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
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
%"class._ZTSN11xercesc_2_716XSerializeEngineE.xercesc_2_7::XSerializeEngine" = type <{ i16, i16, [4 x i8], ptr, ptr, ptr, i64, i64, ptr, ptr, ptr, ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" = type { ptr }
%"class._ZTSN11xercesc_2_715BinOutputStreamE.xercesc_2_7::BinOutputStream" = type { ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_713ValueVectorOfIPvEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710XProtoTypeE.xercesc_2_7::XProtoType" = type { ptr, ptr }
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
%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLNumberEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLNumberEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLNumberEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i64, i64, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_723AbstractStringValidatorE.xercesc_2_7::AbstractStringValidator" = type { %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", i32, i32, i32, i8, ptr }
%"class._ZTSN11xercesc_2_730AnySimpleTypeDatatypeValidatorE.xercesc_2_7::AnySimpleTypeDatatypeValidator" = type { %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" }
%"class._ZTSN11xercesc_2_723AnyURIDatatypeValidatorE.xercesc_2_7::AnyURIDatatypeValidator" = type { %"class._ZTSN11xercesc_2_723AbstractStringValidatorE.xercesc_2_7::AbstractStringValidator" }
%"class._ZTSN11xercesc_2_724BooleanDatatypeValidatorE.xercesc_2_7::BooleanDatatypeValidator" = type { %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" }
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
%"class._ZTSN11xercesc_2_722FloatDatatypeValidatorE.xercesc_2_7::FloatDatatypeValidator" = type { %"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" }
%"class._ZTSN11xercesc_2_723DoubleDatatypeValidatorE.xercesc_2_7::DoubleDatatypeValidator" = type { %"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" }
%"class._ZTSN11xercesc_2_725DurationDatatypeValidatorE.xercesc_2_7::DurationDatatypeValidator" = type { %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" }
%"class._ZTSN11xercesc_2_721DateDatatypeValidatorE.xercesc_2_7::DateDatatypeValidator" = type { %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" }
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
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_719XSerializedObjectIdE.xercesc_2_7::XSerializedObjectId" = type { i32 }
%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" = type { %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" }
%"class._ZTSSt9bad_alloc.std::bad_alloc" = type { %"class._ZTSSt9exception.std::exception" }
%"class._ZTSSt9exception.std::exception" = type { ptr }

$__clang_call_terminate = comdat any

$_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_717DatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE = comdat any

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

$_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm = comdat any

$_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv = comdat any

@_ZTIN11xercesc_2_730AnySimpleTypeDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTIN11xercesc_2_723AnyURIDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTIN11xercesc_2_724BooleanDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTIN11xercesc_2_721DateDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTIN11xercesc_2_729AbstractNumericFacetValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@.str.817 = external hidden unnamed_addr constant [4 x i8], align 1
@_ZTIN11xercesc_2_723DoubleDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTIN11xercesc_2_725DurationDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@.str.2.1138 = external hidden unnamed_addr constant [34 x i8], align 1
@_ZTIN11xercesc_2_722FloatDatatypeValidatorE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZN11xercesc_2_7L12gXMLErrArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [56 x i16], [72 x i16] }>, [128 x i16], <{ [20 x i16], [108 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [103 x i16], [25 x i16] }>, <{ [103 x i16], [25 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [93 x i16], [35 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [92 x i16], [36 x i16] }>, <{ [93 x i16], [35 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [120 x i16], [8 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [113 x i16], [15 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [106 x i16], [22 x i16] }>, <{ [83 x i16], [45 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [119 x i16], [9 x i16] }>, <{ [96 x i16], [32 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [83 x i16], [45 x i16] }>, <{ [112 x i16], [16 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [104 x i16], [24 x i16] }>, <{ [101 x i16], [27 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [77 x i16], [51 x i16] }>, [128 x i16], <{ [96 x i16], [32 x i16] }>, <{ [112 x i16], [16 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [107 x i16], [21 x i16] }>, <{ [109 x i16], [19 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [108 x i16], [20 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [115 x i16], [13 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [92 x i16], [36 x i16] }>, <{ [119 x i16], [9 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, [128 x i16], <{ [25 x i16], [103 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [85 x i16], [43 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [14 x i16], [114 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [90 x i16], [38 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [94 x i16], [34 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !2
@_ZN11xercesc_2_7L15gXMLExceptArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }>, <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [84 x i16], [44 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [97 x i16], [31 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [15 x i16], [113 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [88 x i16], [40 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [89 x i16], [39 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [66 x i16], [62 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [68 x i16], [60 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [60 x i16], [68 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [16 x i16], [112 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [42 x i16], [86 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [90 x i16], [38 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [80 x i16], [48 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [100 x i16], [28 x i16] }>, <{ [111 x i16], [17 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [106 x i16], [22 x i16] }>, <{ [95 x i16], [33 x i16] }>, <{ [86 x i16], [42 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [82 x i16], [46 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [47 x i16], [81 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [54 x i16], [74 x i16] }>, <{ [17 x i16], [111 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [46 x i16], [82 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [25 x i16], [103 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !322
@_ZN11xercesc_2_7L17gXMLValidityArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [51 x i16], [77 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [49 x i16], [79 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [49 x i16], [79 x i16] }>, [128 x i16], <{ [50 x i16], [78 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [91 x i16], [37 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [58 x i16], [70 x i16] }>, <{ [29 x i16], [99 x i16] }>, <{ [23 x i16], [105 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [18 x i16], [110 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [63 x i16], [65 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [44 x i16], [84 x i16] }>, [128 x i16], <{ [35 x i16], [93 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [43 x i16], [85 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [34 x i16], [94 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [21 x i16], [107 x i16] }>, <{ [39 x i16], [89 x i16] }>, <{ [32 x i16], [96 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [37 x i16], [91 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [44 x i16], [84 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ [31 x i16], [97 x i16] }>, <{ [20 x i16], [108 x i16] }>, <{ [28 x i16], [100 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [40 x i16], [88 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [19 x i16], [109 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ i16, i16, i16, [125 x i16] }>, <{ [76 x i16], [52 x i16] }>, <{ [53 x i16], [75 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [55 x i16], [73 x i16] }>, <{ [27 x i16], [101 x i16] }>, <{ [33 x i16], [95 x i16] }>, <{ [36 x i16], [92 x i16] }>, <{ [87 x i16], [41 x i16] }>, <{ [101 x i16], [27 x i16] }>, <{ [78 x i16], [50 x i16] }>, <{ [110 x i16], [18 x i16] }>, <{ [26 x i16], [102 x i16] }>, <{ [52 x i16], [76 x i16] }>, <{ [24 x i16], [104 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [81 x i16], [47 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [35 x i16], [93 x i16] }>, <{ [38 x i16], [90 x i16] }>, <{ [99 x i16], [29 x i16] }>, <{ [50 x i16], [78 x i16] }>, <{ [65 x i16], [63 x i16] }>, <{ [79 x i16], [49 x i16] }>, <{ [85 x i16], [43 x i16] }>, <{ [88 x i16], [40 x i16] }>, <{ [105 x i16], [23 x i16] }>, <{ [73 x i16], [55 x i16] }>, <{ [70 x i16], [58 x i16] }>, <{ [61 x i16], [67 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [57 x i16], [71 x i16] }>, <{ [98 x i16], [30 x i16] }>, <{ [59 x i16], [69 x i16] }>, <{ [45 x i16], [83 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !330
@_ZN11xercesc_2_7L15gXMLDOMMsgArrayE = external hidden unnamed_addr constant <{ <{ i16, i16, i16, i16, i16, i16, i16, [121 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [64 x i16], [64 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [67 x i16], [61 x i16] }>, <{ [48 x i16], [80 x i16] }>, <{ [56 x i16], [72 x i16] }>, <{ [74 x i16], [54 x i16] }>, <{ [75 x i16], [53 x i16] }>, <{ [77 x i16], [51 x i16] }>, <{ [71 x i16], [57 x i16] }>, <{ [69 x i16], [59 x i16] }>, <{ [41 x i16], [87 x i16] }>, <{ [62 x i16], [66 x i16] }>, <{ [102 x i16], [26 x i16] }>, <{ [72 x i16], [56 x i16] }>, <{ [115 x i16], [13 x i16] }>, <{ [13 x i16], [115 x i16] }>, <{ [64 x i16], [64 x i16] }>, [128 x i16], <{ [21 x i16], [107 x i16] }>, <{ [30 x i16], [98 x i16] }>, <{ [22 x i16], [106 x i16] }>, <{ i16, i16, i16, i16, i16, [123 x i16] }> }>, align 16, !intel_dtrans_type !334
@.str.1700 = external hidden unnamed_addr constant [37 x i8], align 1
@.str.1.1699 = external hidden unnamed_addr constant [44 x i8], align 1
@.str.2.1698 = external hidden unnamed_addr constant [32 x i8], align 1
@.str.3.1697 = external hidden unnamed_addr constant [23 x i8], align 1
@.str.4.1696 = external hidden unnamed_addr constant [27 x i8], align 1
@.str.5.1695 = external hidden unnamed_addr constant [35 x i8], align 1
@.str.6.1694 = external hidden unnamed_addr constant [38 x i8], align 1
@.str.7.1693 = external hidden unnamed_addr constant [38 x i8], align 1
@.str.8.1692 = external hidden unnamed_addr constant [15 x i8], align 1
@_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !336
@.str.2.1798 = external hidden unnamed_addr constant [28 x i8], align 1
@.str.2472 = external hidden unnamed_addr constant [10 x i8], align 1
@.str.2622 = external hidden unnamed_addr constant [18 x i8], align 1
@_ZN11xercesc_2_7L9DATETIMESE = external hidden unnamed_addr constant [4 x [8 x i32]], align 16, !intel_dtrans_type !337
@_ZN11xercesc_2_7L16msgLoaderCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L15msgMutexCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L10sMsgLoaderE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !340
@_ZN11xercesc_2_7L9sMsgMutexE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !341
@_ZN11xercesc_2_7L23sScannerMutexRegisteredE = external hidden unnamed_addr global i1, align 1
@_ZN11xercesc_2_715gXMLCleanupListE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !342
@_ZN11xercesc_2_7L8gNullStrE = external hidden unnamed_addr constant [7 x i16], align 2, !intel_dtrans_type !343
@_ZN11xercesc_2_76XMLUni14fgExceptDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !223
@_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !169
@_ZN11xercesc_2_76XMLUni11fgDefErrMsgE = external hidden constant [23 x i16], align 16, !intel_dtrans_type !112
@_ZN11xercesc_2_7L16msgLoaderCleanupE.3649 = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L21validatorMutexCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L10sMsgLoaderE.3653 = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !340
@_ZTIN11xercesc_2_78XMLValid5CodesE = external hidden constant { ptr, ptr }, align 8, !intel_dtrans_type !344
@_ZN11xercesc_2_7L9sMsgMutexE.3656 = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !341
@_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !169
@_ZN11xercesc_2_76XMLUni16fgValidityDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !223
@_ZTIN11xercesc_2_721NumberFormatExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE = external hidden constant <{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, align 16, !intel_dtrans_type !345
@.str.4316 = external hidden unnamed_addr constant [34 x i8], align 1
@.str.4453 = external hidden unnamed_addr constant [33 x i8], align 1
@_ZTIN11xercesc_2_722NoSuchElementExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTISt9bad_alloc = external dso_local constant ptr, !intel_dtrans_type !1
@_ZTVSt9bad_alloc = external dso_local unnamed_addr constant { [5 x ptr] }, align 8, !type !353, !type !354, !type !355, !type !356, !intel_dtrans_type !357
@stderr = external dso_local local_unnamed_addr global ptr, align 8, !intel_dtrans_type !359
@_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !360
@_ZN11xercesc_2_76XMLUni15fgZeroLenStringE = external hidden constant [1 x i16], align 2, !intel_dtrans_type !361
@_ZN11xercesc_2_710XMLChar1_019fgCharCharsTable1_0E = external hidden global [65536 x i8], align 16, !intel_dtrans_type !362
@_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@.str.6.5757 = external hidden unnamed_addr constant [31 x i8], align 1
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !0
@_ZTVN11xercesc_2_729AbstractNumericFacetValidatorE.0 = hidden constant [25 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_729AbstractNumericFacetValidatorE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidatorD2Ev, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidatorD0Ev, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator14isSerializableEv, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_717DatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @__cxa_pure_virtual, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_717DatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @__cxa_pure_virtual, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual, ptr @__cxa_pure_virtual], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !type !381, !type !382, !type !383, !type !384, !type !385, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !400, !type !401, !type !402, !type !403, !type !404, !type !405, !type !406, !type !407, !type !408, !type !409, !type !410, !type !411, !type !412, !type !413, !type !414, !type !415, !type !416, !type !417, !type !418, !type !419, !type !420, !type !421, !type !422, !type !423, !type !424, !type !425, !type !426, !type !427, !type !428, !type !429, !intel_dtrans_type !430
@_ZTVN11xercesc_2_730AnySimpleTypeDatatypeValidatorE.0 = hidden constant [14 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_730AnySimpleTypeDatatypeValidatorE, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_717DatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator17isSubstitutableByEPKNS_17DatatypeValidatorE, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !431, !type !432, !type !433, !type !434, !type !435, !type !436, !type !437, !type !438, !type !439, !type !440, !type !441, !intel_dtrans_type !442
@_ZTVN11xercesc_2_723AnyURIDatatypeValidatorE.0 = hidden constant [25 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_723AnyURIDatatypeValidatorE, ptr @_ZN11xercesc_2_723AnyURIDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_723AnyURIDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_723AnyURIDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_723AnyURIDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_723AnyURIDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_723AbstractStringValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_717DatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_723AbstractStringValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_723AbstractStringValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AnyURIDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AbstractStringValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AbstractStringValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_723AbstractStringValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_723AbstractStringValidator20checkAdditionalFacetEPKtPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_723AbstractStringValidator9getLengthEPKtPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AnyURIDatatypeValidator15checkValueSpaceEPKtPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AbstractStringValidator16inspectFacetBaseEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AbstractStringValidator12inheritFacetEv, ptr @_ZN11xercesc_2_723AbstractStringValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723AbstractStringValidator20normalizeEnumerationEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_723AbstractStringValidator16normalizeContentEPtPNS_13MemoryManagerE], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !443, !type !444, !type !445, !type !446, !type !447, !type !448, !type !449, !type !450, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !451, !type !452, !type !453, !type !454, !type !455, !type !456, !type !457, !type !458, !type !459, !type !460, !type !461, !type !462, !type !463, !type !464, !type !465, !type !466, !type !467, !type !468, !type !469, !type !470, !type !471, !type !472, !type !473, !type !474, !type !475, !type !476, !type !477, !type !478, !type !479, !type !480, !type !481, !type !482, !type !483, !type !484, !type !485, !type !486, !type !487, !type !488, !type !489, !type !490, !type !491, !type !492, !type !493, !type !494, !type !495, !type !496, !type !497, !type !498, !type !499, !type !500, !type !501, !type !502, !intel_dtrans_type !430
@_ZTVN11xercesc_2_724BooleanDatatypeValidatorE.0 = hidden constant [15 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_724BooleanDatatypeValidatorE, ptr @_ZN11xercesc_2_717DatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_724BooleanDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_724BooleanDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_724BooleanDatatypeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_724BooleanDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_724BooleanDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_724BooleanDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !503, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !504, !type !505, !type !506, !type !507, !type !508, !type !509, !type !510, !type !511, !type !512, !type !513, !type !514, !type !515, !type !516, !intel_dtrans_type !517
@_ZTVN11xercesc_2_721DateDatatypeValidatorE.0 = hidden constant [28 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_721DateDatatypeValidatorE, ptr @_ZN11xercesc_2_721DateDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_721DateDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_721DateDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_721DateDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_721DateDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_721DateDatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_717DateTimeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_717DateTimeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_721DateDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DateTimeValidator13compareValuesEPKNS_9XMLNumberES3_, ptr @_ZN11xercesc_2_717DateTimeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DateTimeValidator15setMaxInclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMaxExclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMinInclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMinExclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator14setEnumerationEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_721DateDatatypeValidator5parseEPKtPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_721DateDatatypeValidator5parseEPNS_11XMLDateTimeE, ptr @_ZN11xercesc_2_717DateTimeValidator12compareDatesEPKNS_11XMLDateTimeES3_b], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !type !381, !type !382, !type !383, !type !384, !type !385, !type !518, !type !519, !type !520, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !400, !type !401, !type !402, !type !403, !type !404, !type !405, !type !406, !type !407, !type !521, !type !522, !type !523, !type !524, !type !525, !type !526, !type !527, !type !528, !type !529, !type !530, !type !531, !type !532, !type !533, !type !534, !type !535, !type !536, !type !537, !type !538, !type !539, !type !540, !type !541, !type !542, !type !543, !type !544, !type !545, !type !546, !type !547, !type !548, !type !549, !type !550, !type !551, !type !552, !type !553, !type !554, !type !555, !type !556, !type !557, !type !558, !type !559, !type !560, !type !561, !type !562, !type !563, !type !564, !type !565, !type !566, !type !567, !type !568, !type !569, !type !570, !type !571, !type !572, !type !573, !type !408, !type !409, !type !410, !type !411, !type !412, !type !413, !type !414, !type !415, !type !416, !type !417, !type !418, !type !419, !type !420, !type !421, !type !422, !type !423, !type !424, !type !425, !type !426, !type !427, !type !428, !type !429, !type !574, !type !575, !type !576, !intel_dtrans_type !577
@_ZTVN11xercesc_2_723DoubleDatatypeValidatorE.0 = hidden constant [25 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_723DoubleDatatypeValidatorE, ptr @_ZN11xercesc_2_723DoubleDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_723DoubleDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_723DoubleDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_723DoubleDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_724AbstractNumericValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_724AbstractNumericValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator13compareValuesEPKNS_9XMLNumberES3_, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator15setMaxInclusiveEPKt, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator15setMaxExclusiveEPKt, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator15setMinInclusiveEPKt, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator15setMinExclusiveEPKt, ptr @_ZN11xercesc_2_723DoubleDatatypeValidator14setEnumerationEPNS_13MemoryManagerE], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !type !381, !type !382, !type !383, !type !384, !type !385, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !400, !type !401, !type !402, !type !403, !type !404, !type !405, !type !406, !type !407, !type !578, !type !579, !type !580, !type !581, !type !582, !type !583, !type !584, !type !585, !type !586, !type !587, !type !588, !type !589, !type !590, !type !591, !type !592, !type !593, !type !594, !type !595, !type !596, !type !597, !type !598, !type !599, !type !600, !type !601, !type !602, !type !603, !type !604, !type !605, !type !606, !type !607, !type !608, !type !609, !type !610, !type !611, !type !612, !type !613, !type !614, !type !615, !type !616, !type !617, !type !618, !type !619, !type !620, !type !621, !type !408, !type !409, !type !410, !type !411, !type !412, !type !413, !type !414, !type !415, !type !416, !type !417, !type !418, !type !419, !type !420, !type !421, !type !422, !type !423, !type !424, !type !425, !type !426, !type !427, !type !428, !type !429, !intel_dtrans_type !430
@_ZTVN11xercesc_2_725DurationDatatypeValidatorE.0 = hidden constant [28 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_725DurationDatatypeValidatorE, ptr @_ZN11xercesc_2_725DurationDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_725DurationDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_725DurationDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_725DurationDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_725DurationDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_717DatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_717DateTimeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_717DateTimeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_725DurationDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DateTimeValidator13compareValuesEPKNS_9XMLNumberES3_, ptr @_ZN11xercesc_2_717DateTimeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DateTimeValidator15setMaxInclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMaxExclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMinInclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator15setMinExclusiveEPKt, ptr @_ZN11xercesc_2_717DateTimeValidator14setEnumerationEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_725DurationDatatypeValidator5parseEPKtPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_725DurationDatatypeValidator5parseEPNS_11XMLDateTimeE, ptr @_ZN11xercesc_2_725DurationDatatypeValidator12compareDatesEPKNS_11XMLDateTimeES3_b], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !type !381, !type !382, !type !383, !type !384, !type !385, !type !518, !type !519, !type !520, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !400, !type !401, !type !402, !type !403, !type !404, !type !405, !type !406, !type !407, !type !521, !type !522, !type !523, !type !524, !type !525, !type !526, !type !527, !type !528, !type !529, !type !530, !type !531, !type !532, !type !533, !type !534, !type !535, !type !536, !type !537, !type !538, !type !539, !type !540, !type !541, !type !542, !type !543, !type !544, !type !545, !type !546, !type !547, !type !548, !type !622, !type !623, !type !624, !type !625, !type !626, !type !627, !type !628, !type !629, !type !630, !type !631, !type !632, !type !633, !type !634, !type !635, !type !636, !type !637, !type !638, !type !639, !type !640, !type !641, !type !642, !type !643, !type !644, !type !645, !type !646, !type !408, !type !409, !type !410, !type !411, !type !412, !type !413, !type !414, !type !415, !type !416, !type !417, !type !418, !type !419, !type !420, !type !421, !type !422, !type !423, !type !424, !type !425, !type !426, !type !427, !type !428, !type !429, !type !574, !type !575, !type !576, !intel_dtrans_type !577
@_ZTVN11xercesc_2_722FloatDatatypeValidatorE.0 = hidden constant [25 x ptr] [ptr null, ptr @_ZTIN11xercesc_2_722FloatDatatypeValidatorE, ptr @_ZN11xercesc_2_722FloatDatatypeValidatorD2Ev, ptr @_ZN11xercesc_2_722FloatDatatypeValidatorD0Ev, ptr @_ZNK11xercesc_2_722FloatDatatypeValidator14isSerializableEv, ptr @_ZN11xercesc_2_722FloatDatatypeValidator9serializeERNS_16XSerializeEngineE, ptr @_ZNK11xercesc_2_722FloatDatatypeValidator12getProtoTypeEv, ptr @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv, ptr @_ZNK11xercesc_2_724AbstractNumericValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb, ptr @_ZN11xercesc_2_724AbstractNumericValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_, ptr @_ZN11xercesc_2_722FloatDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_722FloatDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE, ptr @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE, ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_722FloatDatatypeValidator13compareValuesEPKNS_9XMLNumberES3_, ptr @_ZN11xercesc_2_722FloatDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE, ptr @_ZN11xercesc_2_722FloatDatatypeValidator15setMaxInclusiveEPKt, ptr @_ZN11xercesc_2_722FloatDatatypeValidator15setMaxExclusiveEPKt, ptr @_ZN11xercesc_2_722FloatDatatypeValidator15setMinInclusiveEPKt, ptr @_ZN11xercesc_2_722FloatDatatypeValidator15setMinExclusiveEPKt, ptr @_ZN11xercesc_2_722FloatDatatypeValidator14setEnumerationEPNS_13MemoryManagerE], !type !364, !type !365, !type !366, !type !367, !type !368, !type !369, !type !370, !type !371, !type !372, !type !373, !type !374, !type !375, !type !376, !type !377, !type !378, !type !379, !type !380, !type !381, !type !382, !type !383, !type !384, !type !385, !type !386, !type !387, !type !388, !type !389, !type !390, !type !391, !type !392, !type !393, !type !394, !type !395, !type !396, !type !397, !type !398, !type !399, !type !400, !type !401, !type !402, !type !403, !type !404, !type !405, !type !406, !type !407, !type !647, !type !648, !type !649, !type !650, !type !651, !type !652, !type !653, !type !654, !type !655, !type !656, !type !657, !type !658, !type !659, !type !660, !type !661, !type !662, !type !663, !type !664, !type !665, !type !666, !type !667, !type !668, !type !600, !type !601, !type !602, !type !603, !type !604, !type !605, !type !606, !type !607, !type !608, !type !609, !type !610, !type !611, !type !612, !type !613, !type !614, !type !615, !type !616, !type !617, !type !618, !type !619, !type !620, !type !621, !type !408, !type !409, !type !410, !type !411, !type !412, !type !413, !type !414, !type !415, !type !416, !type !417, !type !418, !type !419, !type !420, !type !421, !type !422, !type !423, !type !424, !type !425, !type !426, !type !427, !type !428, !type !429, !intel_dtrans_type !430
@_ZTVN11xercesc_2_714InMemMsgLoaderE.0 = external hidden constant [8 x ptr], !type !669, !type !670, !type !671, !type !672, !type !673, !type !674, !type !675, !type !676, !type !677, !type !678, !intel_dtrans_type !679
@_ZTVN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.0 = external hidden constant [9 x ptr], !type !680, !type !681, !type !682, !type !683, !type !684, !type !685, !type !686, !type !687, !type !688, !type !689, !type !690, !type !691, !intel_dtrans_type !692
@_ZTVN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.0 = external hidden constant [9 x ptr], !type !686, !type !687, !type !688, !type !689, !type !690, !type !691, !intel_dtrans_type !692
@_ZTVN11xercesc_2_713XMLBigDecimalE.0 = external hidden constant [11 x ptr], !type !693, !type !694, !type !695, !type !696, !type !697, !type !698, !type !699, !type !700, !type !364, !type !365, !type !366, !type !367, !type !701, !type !702, !type !703, !type !704, !type !705, !type !706, !type !707, !type !708, !type !709, !type !710, !type !711, !type !712, !intel_dtrans_type !713
@_ZTVN11xercesc_2_711XMLDateTimeE.0 = external hidden constant [11 x ptr], !type !714, !type !715, !type !716, !type !717, !type !718, !type !719, !type !720, !type !721, !type !364, !type !365, !type !366, !type !367, !type !701, !type !702, !type !703, !type !704, !type !705, !type !706, !type !707, !type !708, !type !709, !type !710, !type !711, !type !712, !intel_dtrans_type !713
@_ZTVN11xercesc_2_712XMLExceptionE.0 = external hidden constant [5 x ptr], !type !722, !type !723, !intel_dtrans_type !358
@_ZTVN11xercesc_2_79XMLNumberE.0 = external hidden constant [11 x ptr], !type !364, !type !365, !type !366, !type !367, !type !701, !type !702, !type !703, !type !704, !type !705, !type !706, !type !707, !type !708, !type !709, !type !710, !type !711, !type !712, !intel_dtrans_type !713
@_ZTVN11xercesc_2_721NumberFormatExceptionE.0 = external hidden constant [6 x ptr], !type !722, !type !723, !type !724, !type !725, !type !726, !type !727, !intel_dtrans_type !728
@_ZTVN11xercesc_2_722NoSuchElementExceptionE.0 = external hidden constant [6 x ptr], !type !722, !type !723, !type !724, !type !729, !type !730, !type !731, !intel_dtrans_type !728
@_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0 = external hidden constant [6 x ptr], !type !722, !type !723, !type !724, !type !732, !type !733, !type !734, !intel_dtrans_type !728

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind memory(none)
declare i32 @llvm.eh.typeid.for(ptr) #0

; Function Attrs: nofree
declare !intel.dtrans.func.type !1249 dso_local "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2") local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #1

; Function Attrs: nofree noinline norecurse noreturn nounwind
define hidden fastcc void @__clang_call_terminate(ptr noundef %arg) unnamed_addr #2 comdat {
bb:
  %i = tail call ptr @__cxa_begin_catch(ptr %arg) #47
  tail call void @_ZSt9terminatev() #48
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #4

; Function Attrs: nofree
declare !intel.dtrans.func.type !1250 dso_local noalias "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64) local_unnamed_addr #1

; Function Attrs: nofree noreturn
declare !intel.dtrans.func.type !1251 dso_local void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3") local_unnamed_addr #5

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(ptr) local_unnamed_addr #1

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !1252 dso_local void @_ZdaPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #6

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !1253 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #6

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_pure_virtual() unnamed_addr #5

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #9

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #10 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1254 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i5, align 8, !tbaa !1256
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 1
  store i32 0, ptr %i6, align 8, !tbaa !1259
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 2
  store ptr null, ptr %i7, align 8, !tbaa !1267
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 3
  store i32 %arg2, ptr %i8, align 8, !tbaa !1268
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 4
  store ptr null, ptr %i9, align 8, !tbaa !1269
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 5
  store ptr %arg4, ptr %i10, align 8, !tbaa !1270
  %i11 = icmp eq ptr %arg4, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  store ptr %i13, ptr %i10, align 8, !tbaa !1270
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %arg4, %bb ]
  %i16 = icmp eq ptr %arg1, null
  br i1 %i16, label %bb45, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg1) #49
  %i19 = add i64 %i18, 1
  %i20 = and i64 %i19, 4294967295
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb43

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #50
          to label %bb42 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = tail call ptr @__cxa_begin_catch(ptr %i31) #47
  %i33 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i40) #48
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb27
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %i28, ptr nonnull align 1 %arg1, i64 %i20, i1 false)
  br label %bb45

bb43:                                             ; preds = %bb17
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb45:                                             ; preds = %bb42, %bb14
  %i46 = phi ptr [ %i28, %bb42 ], [ null, %bb14 ]
  store ptr %i46, ptr %i7, align 8, !tbaa !1267
  %i47 = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0, i64 0, i64 2), ptr %i47, align 8, !tbaa !1256
  invoke fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb48 unwind label %bb49

bb48:                                             ; preds = %bb45
  ret void

bb49:                                             ; preds = %bb45
  %i50 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #47
  br label %bb36
}

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1274 hidden void @_ZN11xercesc_2_729AbstractNumericFacetValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: norecurse noreturn nounwind uwtable
declare !intel.dtrans.func.type !1276 hidden void @_ZN11xercesc_2_729AbstractNumericFacetValidatorD0Ev(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #12 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1277 hidden noundef zeroext i1 @_ZNK11xercesc_2_729AbstractNumericFacetValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1278 hidden void @_ZN11xercesc_2_729AbstractNumericFacetValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1280 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1282 hidden noundef zeroext i1 @_ZNK11xercesc_2_717DatatypeValidator8isAtomicEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #15 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1283 hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_729AbstractNumericFacetValidator13getEnumStringEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(160) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !1284 hidden noundef zeroext i1 @_ZN11xercesc_2_717DatatypeValidator17isSubstitutableByEPKS0_(ptr noundef nonnull readnone align 8 dereferenceable(104) "intel_dtrans_func_index"="1", ptr noundef readonly "intel_dtrans_func_index"="2") unnamed_addr #17 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define hidden noundef i32 @_ZN11xercesc_2_717DatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, ptr noundef readonly "intel_dtrans_func_index"="3" %arg2, ptr nocapture readnone "intel_dtrans_func_index"="4" %arg3) unnamed_addr #17 comdat align 2 !intel.dtrans.func.type !1285 {
bb:
  %i = icmp eq ptr %arg1, null
  %i4 = icmp eq ptr %arg2, null
  %i5 = or i1 %i, %i4
  br i1 %i5, label %bb6, label %bb40

bb6:                                              ; preds = %bb
  br i1 %i, label %bb7, label %bb25

bb7:                                              ; preds = %bb6
  br i1 %i4, label %bb22, label %bb8

bb8:                                              ; preds = %bb7
  %i9 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i10 = icmp eq i16 %i9, 0
  br i1 %i10, label %bb22, label %bb11

bb11:                                             ; preds = %bb11, %bb8
  %i12 = phi ptr [ %i13, %bb11 ], [ %arg2, %bb8 ]
  %i13 = getelementptr inbounds i16, ptr %i12, i64 1
  %i14 = load i16, ptr %i13, align 2, !tbaa !1286
  %i15 = icmp eq i16 %i14, 0
  br i1 %i15, label %bb16, label %bb11, !llvm.loop !1288

bb16:                                             ; preds = %bb11
  %i17 = ptrtoint ptr %i13 to i64
  %i18 = ptrtoint ptr %arg2 to i64
  %i19 = sub i64 %i17, %i18
  %i20 = lshr exact i64 %i19, 1
  %i21 = trunc i64 %i20 to i32
  br label %bb22

bb22:                                             ; preds = %bb16, %bb8, %bb7
  %i23 = phi i32 [ %i21, %bb16 ], [ 0, %bb8 ], [ 0, %bb7 ]
  %i24 = sub i32 0, %i23
  br label %bb61

bb25:                                             ; preds = %bb6
  br i1 %i4, label %bb26, label %bb40

bb26:                                             ; preds = %bb25
  %i27 = load i16, ptr %arg1, align 2, !tbaa !1286
  %i28 = icmp eq i16 %i27, 0
  br i1 %i28, label %bb61, label %bb29

bb29:                                             ; preds = %bb29, %bb26
  %i30 = phi ptr [ %i31, %bb29 ], [ %arg1, %bb26 ]
  %i31 = getelementptr inbounds i16, ptr %i30, i64 1
  %i32 = load i16, ptr %i31, align 2, !tbaa !1286
  %i33 = icmp eq i16 %i32, 0
  br i1 %i33, label %bb34, label %bb29, !llvm.loop !1288

bb34:                                             ; preds = %bb29
  %i35 = ptrtoint ptr %i31 to i64
  %i36 = ptrtoint ptr %arg1 to i64
  %i37 = sub i64 %i35, %i36
  %i38 = lshr exact i64 %i37, 1
  %i39 = trunc i64 %i38 to i32
  br label %bb61

bb40:                                             ; preds = %bb25, %bb
  %i41 = load i16, ptr %arg1, align 2, !tbaa !1286
  %i42 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i43 = icmp eq i16 %i41, %i42
  br i1 %i43, label %bb50, label %bb44

bb44:                                             ; preds = %bb55, %bb40
  %i45 = phi i16 [ %i41, %bb40 ], [ %i58, %bb55 ]
  %i46 = phi i16 [ %i42, %bb40 ], [ %i59, %bb55 ]
  %i47 = zext i16 %i46 to i32
  %i48 = zext i16 %i45 to i32
  %i49 = sub nsw i32 %i48, %i47
  br label %bb61

bb50:                                             ; preds = %bb55, %bb40
  %i51 = phi i16 [ %i58, %bb55 ], [ %i41, %bb40 ]
  %i52 = phi ptr [ %i57, %bb55 ], [ %arg2, %bb40 ]
  %i53 = phi ptr [ %i56, %bb55 ], [ %arg1, %bb40 ]
  %i54 = icmp eq i16 %i51, 0
  br i1 %i54, label %bb61, label %bb55

bb55:                                             ; preds = %bb50
  %i56 = getelementptr inbounds i16, ptr %i53, i64 1
  %i57 = getelementptr inbounds i16, ptr %i52, i64 1
  %i58 = load i16, ptr %i56, align 2, !tbaa !1286
  %i59 = load i16, ptr %i57, align 2, !tbaa !1286
  %i60 = icmp eq i16 %i58, %i59
  br i1 %i60, label %bb50, label %bb44, !llvm.loop !1290

bb61:                                             ; preds = %bb50, %bb44, %bb34, %bb26, %bb22
  %i62 = phi i32 [ %i24, %bb22 ], [ %i49, %bb44 ], [ %i39, %bb34 ], [ 0, %bb26 ], [ 0, %bb50 ]
  ret i32 %i62
}

; Function Attrs: mustprogress noreturn uwtable
declare !intel.dtrans.func.type !1291 hidden void @_ZN11xercesc_2_729AbstractNumericFacetValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture readnone "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #18 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1292 hidden void @_ZN11xercesc_2_729AbstractNumericFacetValidator22inheritAdditionalFacetEv(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1293 hidden void @_ZNK11xercesc_2_729AbstractNumericFacetValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1294 hidden void @_ZNK11xercesc_2_729AbstractNumericFacetValidator35checkAdditionalFacetConstraintsBaseEPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_722NoSuchElementExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #10 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1295 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i5, align 8, !tbaa !1256
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 1
  store i32 0, ptr %i6, align 8, !tbaa !1259
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 2
  store ptr null, ptr %i7, align 8, !tbaa !1267
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 3
  store i32 %arg2, ptr %i8, align 8, !tbaa !1268
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 4
  store ptr null, ptr %i9, align 8, !tbaa !1269
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 5
  store ptr %arg4, ptr %i10, align 8, !tbaa !1270
  %i11 = icmp eq ptr %arg4, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  store ptr %i13, ptr %i10, align 8, !tbaa !1270
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %arg4, %bb ]
  %i16 = icmp eq ptr %arg1, null
  br i1 %i16, label %bb45, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg1) #49
  %i19 = add i64 %i18, 1
  %i20 = and i64 %i19, 4294967295
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb43

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #50
          to label %bb42 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = tail call ptr @__cxa_begin_catch(ptr %i31) #47
  %i33 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i40) #48
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb27
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %i28, ptr nonnull align 1 %arg1, i64 %i20, i1 false)
  br label %bb45

bb43:                                             ; preds = %bb17
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb45:                                             ; preds = %bb42, %bb14
  %i46 = phi ptr [ %i28, %bb42 ], [ null, %bb14 ]
  store ptr %i46, ptr %i7, align 8, !tbaa !1267
  %i47 = getelementptr %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_722NoSuchElementExceptionE.0, i64 0, i64 2), ptr %i47, align 8, !tbaa !1256
  invoke fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb48 unwind label %bb49

bb48:                                             ; preds = %bb45
  ret void

bb49:                                             ; preds = %bb45
  %i50 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #47
  br label %bb36
}

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1297 hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_724AbstractNumericValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i1 noundef zeroext) unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1299 hidden void @_ZN11xercesc_2_724AbstractNumericValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture noundef readnone "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
declare !intel.dtrans.func.type !1300 hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_723AbstractStringValidator13getEnumStringEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(128) "intel_dtrans_func_index"="2") unnamed_addr #19 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1302 hidden void @_ZN11xercesc_2_723AbstractStringValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define hidden noundef i32 @_ZN11xercesc_2_723AbstractStringValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, ptr noundef readonly "intel_dtrans_func_index"="3" %arg2, ptr nocapture readnone "intel_dtrans_func_index"="4" %arg3) unnamed_addr #20 align 2 !intel.dtrans.func.type !1303 {
bb:
  %i = icmp eq ptr %arg1, null
  %i4 = icmp eq ptr %arg2, null
  %i5 = or i1 %i, %i4
  br i1 %i5, label %bb6, label %bb40

bb6:                                              ; preds = %bb
  br i1 %i, label %bb7, label %bb25

bb7:                                              ; preds = %bb6
  br i1 %i4, label %bb22, label %bb8

bb8:                                              ; preds = %bb7
  %i9 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i10 = icmp eq i16 %i9, 0
  br i1 %i10, label %bb22, label %bb11

bb11:                                             ; preds = %bb11, %bb8
  %i12 = phi ptr [ %i13, %bb11 ], [ %arg2, %bb8 ]
  %i13 = getelementptr inbounds i16, ptr %i12, i64 1
  %i14 = load i16, ptr %i13, align 2, !tbaa !1286
  %i15 = icmp eq i16 %i14, 0
  br i1 %i15, label %bb16, label %bb11, !llvm.loop !1288

bb16:                                             ; preds = %bb11
  %i17 = ptrtoint ptr %i13 to i64
  %i18 = ptrtoint ptr %arg2 to i64
  %i19 = sub i64 %i17, %i18
  %i20 = lshr exact i64 %i19, 1
  %i21 = trunc i64 %i20 to i32
  br label %bb22

bb22:                                             ; preds = %bb16, %bb8, %bb7
  %i23 = phi i32 [ %i21, %bb16 ], [ 0, %bb8 ], [ 0, %bb7 ]
  %i24 = sub i32 0, %i23
  br label %bb61

bb25:                                             ; preds = %bb6
  br i1 %i4, label %bb26, label %bb40

bb26:                                             ; preds = %bb25
  %i27 = load i16, ptr %arg1, align 2, !tbaa !1286
  %i28 = icmp eq i16 %i27, 0
  br i1 %i28, label %bb61, label %bb29

bb29:                                             ; preds = %bb29, %bb26
  %i30 = phi ptr [ %i31, %bb29 ], [ %arg1, %bb26 ]
  %i31 = getelementptr inbounds i16, ptr %i30, i64 1
  %i32 = load i16, ptr %i31, align 2, !tbaa !1286
  %i33 = icmp eq i16 %i32, 0
  br i1 %i33, label %bb34, label %bb29, !llvm.loop !1288

bb34:                                             ; preds = %bb29
  %i35 = ptrtoint ptr %i31 to i64
  %i36 = ptrtoint ptr %arg1 to i64
  %i37 = sub i64 %i35, %i36
  %i38 = lshr exact i64 %i37, 1
  %i39 = trunc i64 %i38 to i32
  br label %bb61

bb40:                                             ; preds = %bb25, %bb
  %i41 = load i16, ptr %arg1, align 2, !tbaa !1286
  %i42 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i43 = icmp eq i16 %i41, %i42
  br i1 %i43, label %bb50, label %bb44

bb44:                                             ; preds = %bb55, %bb40
  %i45 = phi i16 [ %i41, %bb40 ], [ %i58, %bb55 ]
  %i46 = phi i16 [ %i42, %bb40 ], [ %i59, %bb55 ]
  %i47 = zext i16 %i46 to i32
  %i48 = zext i16 %i45 to i32
  %i49 = sub nsw i32 %i48, %i47
  br label %bb61

bb50:                                             ; preds = %bb55, %bb40
  %i51 = phi i16 [ %i58, %bb55 ], [ %i41, %bb40 ]
  %i52 = phi ptr [ %i57, %bb55 ], [ %arg2, %bb40 ]
  %i53 = phi ptr [ %i56, %bb55 ], [ %arg1, %bb40 ]
  %i54 = icmp eq i16 %i51, 0
  br i1 %i54, label %bb61, label %bb55

bb55:                                             ; preds = %bb50
  %i56 = getelementptr inbounds i16, ptr %i53, i64 1
  %i57 = getelementptr inbounds i16, ptr %i52, i64 1
  %i58 = load i16, ptr %i56, align 2, !tbaa !1286
  %i59 = load i16, ptr %i57, align 2, !tbaa !1286
  %i60 = icmp eq i16 %i58, %i59
  br i1 %i60, label %bb50, label %bb44, !llvm.loop !1290

bb61:                                             ; preds = %bb50, %bb44, %bb34, %bb26, %bb22
  %i62 = phi i32 [ %i24, %bb22 ], [ %i49, %bb44 ], [ %i39, %bb34 ], [ 0, %bb26 ], [ 0, %bb50 ]
  ret i32 %i62
}

; Function Attrs: mustprogress noreturn uwtable
declare !intel.dtrans.func.type !1304 hidden void @_ZN11xercesc_2_723AbstractStringValidator21assignAdditionalFacetEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture readnone "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #18 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1305 hidden void @_ZN11xercesc_2_723AbstractStringValidator22inheritAdditionalFacetEv(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1306 hidden void @_ZNK11xercesc_2_723AbstractStringValidator31checkAdditionalFacetConstraintsEPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1307 hidden void @_ZNK11xercesc_2_723AbstractStringValidator20checkAdditionalFacetEPKtPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2", ptr nocapture "intel_dtrans_func_index"="3") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !1308 hidden noundef i32 @_ZNK11xercesc_2_723AbstractStringValidator9getLengthEPKtPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture readnone "intel_dtrans_func_index"="3") unnamed_addr #20 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1309 hidden void @_ZN11xercesc_2_723AbstractStringValidator16inspectFacetBaseEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1310 hidden void @_ZN11xercesc_2_723AbstractStringValidator12inheritFacetEv(ptr nocapture noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1") unnamed_addr #21 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1311 hidden void @_ZN11xercesc_2_723AbstractStringValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1312 hidden void @_ZN11xercesc_2_723AbstractStringValidator20normalizeEnumerationEPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1313 hidden void @_ZNK11xercesc_2_723AbstractStringValidator16normalizeContentEPtPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2", ptr nocapture "intel_dtrans_func_index"="3") unnamed_addr #13 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1314 hidden void @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1316 hidden void @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1317 hidden noundef zeroext i1 @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1318 hidden void @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1319 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1320 hidden noundef zeroext i1 @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator8isAtomicEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #15 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1321 hidden noalias noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_730AnySimpleTypeDatatypeValidator13getEnumStringEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1322 hidden void @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE(ptr nocapture nonnull align 8 "intel_dtrans_func_index"="1", ptr nocapture "intel_dtrans_func_index"="2", ptr nocapture "intel_dtrans_func_index"="3", ptr nocapture "intel_dtrans_func_index"="4") unnamed_addr #15 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1323 hidden noundef zeroext i1 @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator17isSubstitutableByEPKNS_17DatatypeValidatorE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture readnone "intel_dtrans_func_index"="2") unnamed_addr #15 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1324 hidden noundef i32 @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture readnone "intel_dtrans_func_index"="2", ptr nocapture readnone "intel_dtrans_func_index"="3", ptr nocapture readnone "intel_dtrans_func_index"="4") unnamed_addr #15 align 2

; Function Attrs: noreturn uwtable
declare !intel.dtrans.func.type !1325 hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_730AnySimpleTypeDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #22 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1326 hidden void @_ZN11xercesc_2_723AnyURIDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1328 hidden void @_ZN11xercesc_2_723AnyURIDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1329 hidden noundef zeroext i1 @_ZNK11xercesc_2_723AnyURIDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1330 hidden void @_ZN11xercesc_2_723AnyURIDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1331 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_723AnyURIDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1332 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_723AnyURIDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(128) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1333 hidden void @_ZN11xercesc_2_723AnyURIDatatypeValidator15checkValueSpaceEPKtPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3") unnamed_addr #16 align 2

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !1334 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr #23

; Function Attrs: inlinehint nounwind uwtable
declare !intel.dtrans.func.type !1335 hidden void @_ZN11xercesc_2_724BooleanDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1") unnamed_addr #24 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1337 hidden noundef zeroext i1 @_ZNK11xercesc_2_724BooleanDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1338 hidden void @_ZN11xercesc_2_724BooleanDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1339 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1340 hidden noalias noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator13getEnumStringEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1341 hidden noalias noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_724BooleanDatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i1 noundef zeroext) unnamed_addr #14 align 2

; Function Attrs: inlinehint mustprogress uwtable
declare !intel.dtrans.func.type !1342 hidden void @_ZN11xercesc_2_724BooleanDatatypeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #25 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define hidden noundef i32 @_ZN11xercesc_2_724BooleanDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, ptr noundef readonly "intel_dtrans_func_index"="3" %arg2, ptr nocapture readnone "intel_dtrans_func_index"="4" %arg3) unnamed_addr #20 align 2 !intel.dtrans.func.type !1343 {
bb:
  %i = icmp eq ptr %arg1, null
  br i1 %i, label %bb120, label %bb4

bb4:                                              ; preds = %bb
  %i5 = load i16, ptr %arg1, align 2, !tbaa !1286
  switch i16 %i5, label %bb120 [
    i16 102, label %bb6
    i16 48, label %bb18
    i16 116, label %bb62
    i16 49, label %bb76
  ]

bb6:                                              ; preds = %bb11, %bb4
  %i7 = phi i16 [ %i14, %bb11 ], [ 102, %bb4 ]
  %i8 = phi ptr [ %i13, %bb11 ], [ @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, %bb4 ]
  %i9 = phi ptr [ %i12, %bb11 ], [ %arg1, %bb4 ]
  %i10 = icmp eq i16 %i7, 0
  br i1 %i10, label %bb30, label %bb11

bb11:                                             ; preds = %bb6
  %i12 = getelementptr inbounds i16, ptr %i9, i64 1
  %i13 = getelementptr inbounds i16, ptr %i8, i64 1
  %i14 = load i16, ptr %i12, align 2, !tbaa !1286
  %i15 = load i16, ptr %i13, align 2, !tbaa !1286
  %i16 = icmp eq i16 %i14, %i15
  br i1 %i16, label %bb6, label %bb17, !llvm.loop !1344

bb17:                                             ; preds = %bb11
  switch i16 %i5, label %bb120 [
    i16 48, label %bb18
    i16 116, label %bb62
    i16 49, label %bb76
  ]

bb18:                                             ; preds = %bb23, %bb17, %bb4
  %i19 = phi i16 [ %i26, %bb23 ], [ 48, %bb17 ], [ 48, %bb4 ]
  %i20 = phi ptr [ %i25, %bb23 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 2, i32 0), %bb17 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 2, i32 0), %bb4 ]
  %i21 = phi ptr [ %i24, %bb23 ], [ %arg1, %bb17 ], [ %arg1, %bb4 ]
  %i22 = icmp eq i16 %i19, 0
  br i1 %i22, label %bb30, label %bb23

bb23:                                             ; preds = %bb18
  %i24 = getelementptr inbounds i16, ptr %i21, i64 1
  %i25 = getelementptr inbounds i16, ptr %i20, i64 1
  %i26 = load i16, ptr %i24, align 2, !tbaa !1286
  %i27 = getelementptr [32 x i16], ptr %i25, i64 0, i64 0
  %i28 = load i16, ptr %i27, align 2, !tbaa !1286
  %i29 = icmp eq i16 %i26, %i28
  br i1 %i29, label %bb18, label %bb61, !llvm.loop !1344

bb30:                                             ; preds = %bb18, %bb6
  %i31 = phi i16 [ 102, %bb18 ], [ %i5, %bb6 ]
  %i32 = icmp eq ptr %arg2, null
  br i1 %i32, label %bb120, label %bb33

bb33:                                             ; preds = %bb30
  %i34 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i35 = icmp eq i16 %i34, %i31
  br i1 %i35, label %bb36, label %bb47

bb36:                                             ; preds = %bb41, %bb33
  %i37 = phi i16 [ %i44, %bb41 ], [ %i31, %bb33 ]
  %i38 = phi ptr [ %i43, %bb41 ], [ @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, %bb33 ]
  %i39 = phi ptr [ %i42, %bb41 ], [ %arg2, %bb33 ]
  %i40 = icmp eq i16 %i37, 0
  br i1 %i40, label %bb120, label %bb41

bb41:                                             ; preds = %bb36
  %i42 = getelementptr inbounds i16, ptr %i39, i64 1
  %i43 = getelementptr inbounds i16, ptr %i38, i64 1
  %i44 = load i16, ptr %i42, align 2, !tbaa !1286
  %i45 = load i16, ptr %i43, align 2, !tbaa !1286
  %i46 = icmp eq i16 %i44, %i45
  br i1 %i46, label %bb36, label %bb47, !llvm.loop !1344

bb47:                                             ; preds = %bb41, %bb33
  %i48 = icmp eq i16 %i34, 48
  br i1 %i48, label %bb49, label %bb120

bb49:                                             ; preds = %bb54, %bb47
  %i50 = phi i16 [ %i57, %bb54 ], [ 48, %bb47 ]
  %i51 = phi ptr [ %i56, %bb54 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 2, i32 0), %bb47 ]
  %i52 = phi ptr [ %i55, %bb54 ], [ %arg2, %bb47 ]
  %i53 = icmp eq i16 %i50, 0
  br i1 %i53, label %bb120, label %bb54

bb54:                                             ; preds = %bb49
  %i55 = getelementptr inbounds i16, ptr %i52, i64 1
  %i56 = getelementptr inbounds i16, ptr %i51, i64 1
  %i57 = load i16, ptr %i55, align 2, !tbaa !1286
  %i58 = getelementptr [32 x i16], ptr %i56, i64 0, i64 0
  %i59 = load i16, ptr %i58, align 2, !tbaa !1286
  %i60 = icmp eq i16 %i57, %i59
  br i1 %i60, label %bb49, label %bb120, !llvm.loop !1344

bb61:                                             ; preds = %bb23
  switch i16 %i5, label %bb120 [
    i16 116, label %bb62
    i16 49, label %bb76
  ]

bb62:                                             ; preds = %bb67, %bb61, %bb17, %bb4
  %i63 = phi i16 [ %i70, %bb67 ], [ 116, %bb61 ], [ 116, %bb17 ], [ 116, %bb4 ]
  %i64 = phi ptr [ %i69, %bb67 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 1, i32 0), %bb61 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 1, i32 0), %bb17 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 1, i32 0), %bb4 ]
  %i65 = phi ptr [ %i68, %bb67 ], [ %arg1, %bb61 ], [ %arg1, %bb17 ], [ %arg1, %bb4 ]
  %i66 = icmp eq i16 %i63, 0
  br i1 %i66, label %bb88, label %bb67

bb67:                                             ; preds = %bb62
  %i68 = getelementptr inbounds i16, ptr %i65, i64 1
  %i69 = getelementptr inbounds i16, ptr %i64, i64 1
  %i70 = load i16, ptr %i68, align 2, !tbaa !1286
  %i71 = getelementptr [32 x i16], ptr %i69, i64 0, i64 0
  %i72 = load i16, ptr %i71, align 2, !tbaa !1286
  %i73 = icmp eq i16 %i70, %i72
  br i1 %i73, label %bb62, label %bb74, !llvm.loop !1344

bb74:                                             ; preds = %bb67
  %i75 = icmp eq i16 %i5, 49
  br i1 %i75, label %bb76, label %bb120

bb76:                                             ; preds = %bb81, %bb74, %bb61, %bb17, %bb4
  %i77 = phi i16 [ %i84, %bb81 ], [ 49, %bb74 ], [ 49, %bb61 ], [ 49, %bb17 ], [ 49, %bb4 ]
  %i78 = phi ptr [ %i83, %bb81 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 3, i32 0), %bb74 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 3, i32 0), %bb61 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 3, i32 0), %bb17 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 3, i32 0), %bb4 ]
  %i79 = phi ptr [ %i82, %bb81 ], [ %arg1, %bb74 ], [ %arg1, %bb61 ], [ %arg1, %bb17 ], [ %arg1, %bb4 ]
  %i80 = icmp eq i16 %i77, 0
  br i1 %i80, label %bb88, label %bb81

bb81:                                             ; preds = %bb76
  %i82 = getelementptr inbounds i16, ptr %i79, i64 1
  %i83 = getelementptr inbounds i16, ptr %i78, i64 1
  %i84 = load i16, ptr %i82, align 2, !tbaa !1286
  %i85 = getelementptr [32 x i16], ptr %i83, i64 0, i64 0
  %i86 = load i16, ptr %i85, align 2, !tbaa !1286
  %i87 = icmp eq i16 %i84, %i86
  br i1 %i87, label %bb76, label %bb120, !llvm.loop !1344

bb88:                                             ; preds = %bb76, %bb62
  %i89 = phi i16 [ 116, %bb76 ], [ %i5, %bb62 ]
  %i90 = icmp eq ptr %arg2, null
  br i1 %i90, label %bb120, label %bb91

bb91:                                             ; preds = %bb88
  %i92 = load i16, ptr %arg2, align 2, !tbaa !1286
  %i93 = icmp eq i16 %i92, %i89
  br i1 %i93, label %bb94, label %bb106

bb94:                                             ; preds = %bb99, %bb91
  %i95 = phi i16 [ %i102, %bb99 ], [ %i89, %bb91 ]
  %i96 = phi ptr [ %i101, %bb99 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 1, i32 0), %bb91 ]
  %i97 = phi ptr [ %i100, %bb99 ], [ %arg2, %bb91 ]
  %i98 = icmp eq i16 %i95, 0
  br i1 %i98, label %bb120, label %bb99

bb99:                                             ; preds = %bb94
  %i100 = getelementptr inbounds i16, ptr %i97, i64 1
  %i101 = getelementptr inbounds i16, ptr %i96, i64 1
  %i102 = load i16, ptr %i100, align 2, !tbaa !1286
  %i103 = getelementptr [32 x i16], ptr %i101, i64 0, i64 0
  %i104 = load i16, ptr %i103, align 2, !tbaa !1286
  %i105 = icmp eq i16 %i102, %i104
  br i1 %i105, label %bb94, label %bb106, !llvm.loop !1344

bb106:                                            ; preds = %bb99, %bb91
  %i107 = icmp eq i16 %i92, 49
  br i1 %i107, label %bb108, label %bb120

bb108:                                            ; preds = %bb113, %bb106
  %i109 = phi i16 [ %i116, %bb113 ], [ 49, %bb106 ]
  %i110 = phi ptr [ %i115, %bb113 ], [ getelementptr inbounds (<{ <{ i16, i16, i16, i16, i16, [27 x i16] }>, <{ i16, i16, i16, i16, [28 x i16] }>, <{ i16, [31 x i16] }>, <{ i16, [31 x i16] }> }>, ptr @_ZN11xercesc_2_76XMLUni19fgBooleanValueSpaceE, i64 0, i32 3, i32 0), %bb106 ]
  %i111 = phi ptr [ %i114, %bb113 ], [ %arg2, %bb106 ]
  %i112 = icmp eq i16 %i109, 0
  br i1 %i112, label %bb120, label %bb113

bb113:                                            ; preds = %bb108
  %i114 = getelementptr inbounds i16, ptr %i111, i64 1
  %i115 = getelementptr inbounds i16, ptr %i110, i64 1
  %i116 = load i16, ptr %i114, align 2, !tbaa !1286
  %i117 = getelementptr [32 x i16], ptr %i115, i64 0, i64 0
  %i118 = load i16, ptr %i117, align 2, !tbaa !1286
  %i119 = icmp eq i16 %i116, %i118
  br i1 %i119, label %bb108, label %bb120, !llvm.loop !1344

bb120:                                            ; preds = %bb113, %bb108, %bb106, %bb94, %bb88, %bb81, %bb74, %bb61, %bb54, %bb49, %bb47, %bb36, %bb30, %bb17, %bb4, %bb
  %i121 = phi i32 [ 1, %bb74 ], [ 0, %bb49 ], [ 0, %bb36 ], [ 0, %bb108 ], [ 0, %bb94 ], [ 1, %bb81 ], [ 1, %bb30 ], [ 1, %bb54 ], [ 1, %bb47 ], [ 1, %bb61 ], [ 1, %bb17 ], [ 1, %bb4 ], [ 1, %bb ], [ 1, %bb88 ], [ 1, %bb113 ], [ 1, %bb106 ]
  ret i32 %i121
}

; Function Attrs: inlinehint uwtable
declare !intel.dtrans.func.type !1345 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_724BooleanDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #26 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1346 hidden void @_ZN11xercesc_2_724BooleanDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #16 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !1347 hidden noundef i32 @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", i32 noundef, ptr nocapture readnone "intel_dtrans_func_index"="3") unnamed_addr #17 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
declare !intel.dtrans.func.type !1349 hidden noundef zeroext i1 @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3") unnamed_addr #17 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1350 hidden void @_ZN11xercesc_2_717DatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1351 hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_717DatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(104) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i1 noundef zeroext) unnamed_addr #14 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1352 hidden void @_ZN11xercesc_2_721DateDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1354 hidden void @_ZN11xercesc_2_721DateDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1355 hidden noundef zeroext i1 @_ZNK11xercesc_2_721DateDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1356 hidden void @_ZN11xercesc_2_721DateDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1357 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_721DateDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1358 hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_721DateDatatypeValidator26getCanonicalRepresentationEPKtPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i1 noundef zeroext) unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1359 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_721DateDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1360 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_721DateDatatypeValidator5parseEPKtPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1362 hidden void @_ZN11xercesc_2_721DateDatatypeValidator5parseEPNS_11XMLDateTimeE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef nonnull "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1363 hidden void @_ZN11xercesc_2_717DateTimeValidator8validateEPKtPNS_17ValidationContextEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture noundef readnone "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #16 align 2

; Function Attrs: uwtable
define hidden noundef i32 @_ZN11xercesc_2_717DateTimeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1365 {
bb:
  %i = alloca i32, align 4
  %i4 = alloca %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", align 8
  %i5 = alloca %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", align 8
  %i6 = getelementptr %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1256
  %i8 = tail call i1 @llvm.type.test(ptr %i7, metadata !"_ZTSN11xercesc_2_717DateTimeValidatorE")
  tail call void @llvm.assume(i1 %i8)
  %i9 = getelementptr inbounds ptr, ptr %i7, i64 23
  %i10 = load ptr, ptr %i9, align 8
  %i11 = invoke noundef ptr %i10(ptr noundef nonnull align 8 dereferenceable(160) %arg, ptr noundef %arg1, ptr noundef %arg3)
          to label %bb12 unwind label %bb53, !intel_dtrans_type !1366

bb12:                                             ; preds = %bb
  %i13 = getelementptr %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0
  %i14 = load ptr, ptr %i13, align 8, !tbaa !1256
  %i15 = tail call i1 @llvm.type.test(ptr %i14, metadata !"_ZTSN11xercesc_2_717DateTimeValidatorE")
  tail call void @llvm.assume(i1 %i15)
  %i16 = getelementptr inbounds ptr, ptr %i14, i64 23
  %i17 = load ptr, ptr %i16, align 8
  %i18 = invoke noundef ptr %i17(ptr noundef nonnull align 8 dereferenceable(160) %arg, ptr noundef %arg2, ptr noundef %arg3)
          to label %bb19 unwind label %bb55, !intel_dtrans_type !1366

bb19:                                             ; preds = %bb12
  %i20 = getelementptr %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1256
  %i22 = tail call i1 @llvm.type.test(ptr %i21, metadata !"_ZTSN11xercesc_2_717DateTimeValidatorE")
  tail call void @llvm.assume(i1 %i22)
  %i23 = getelementptr inbounds ptr, ptr %i21, i64 25
  %i24 = load ptr, ptr %i23, align 8
  %i25 = icmp eq ptr %i24, @_ZN11xercesc_2_717DateTimeValidator12compareDatesEPKNS_11XMLDateTimeES3_b
  br i1 %i25, label %bb26, label %bb28

bb26:                                             ; preds = %bb19
  %i27 = invoke noundef i32 @_ZN11xercesc_2_717DateTimeValidator12compareDatesEPKNS_11XMLDateTimeES3_b(ptr nonnull align 8 poison, ptr noundef %i11, ptr noundef %i18, i1 zeroext poison)
          to label %bb35 unwind label %bb57, !range !1367, !intel_dtrans_type !1368, !_Intel.Devirt.Call !1273

bb28:                                             ; preds = %bb19
  %i29 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef %i11, ptr noundef %i18)
          to label %bb30 unwind label %bb57, !range !1370

bb30:                                             ; preds = %bb28
  %i31 = icmp eq i32 %i29, 0
  br i1 %i31, label %bb40, label %bb32

bb32:                                             ; preds = %bb30
  call void @llvm.lifetime.start.p0(i64 -1, ptr nonnull %i)
  call void @llvm.lifetime.start.p0(i64 -1, ptr nonnull %i4)
  call void @llvm.lifetime.start.p0(i64 -1, ptr nonnull %i5)
  invoke void @_ZN11xercesc_2_725DurationDatatypeValidator12compareDatesEPKNS_11XMLDateTimeES3_b.6352.extracted(ptr nonnull %i4, ptr nonnull %i5, ptr nonnull %i11, ptr %i18, i1 true, ptr nonnull %i)
          to label %bb33 unwind label %bb57

bb33:                                             ; preds = %bb32
  %i34 = load i32, ptr %i, align 4
  call void @llvm.lifetime.end.p0(i64 -1, ptr nonnull %i)
  br label %bb35

bb35:                                             ; preds = %bb33, %bb26
  %i36 = phi i32 [ %i27, %bb26 ], [ %i34, %bb33 ]
  %i37 = icmp eq i32 %i36, 2
  %i38 = freeze i1 %i37
  %i39 = select i1 %i38, i32 -1, i32 %i36
  br label %bb40

bb40:                                             ; preds = %bb35, %bb30
  %i41 = phi i32 [ 0, %bb30 ], [ %i39, %bb35 ]
  %i42 = icmp eq ptr %i18, null
  br i1 %i42, label %bb47, label %bb43

bb43:                                             ; preds = %bb40
  %i44 = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i18, i64 0, i32 0, i32 0, i32 0
  %i45 = load ptr, ptr %i44, align 8, !tbaa !1256
  %i46 = call i1 @llvm.type.test(ptr %i45, metadata !"_ZTSN11xercesc_2_711XMLDateTimeE")
  tail call void @llvm.assume(i1 %i46)
  tail call void @_ZN11xercesc_2_711XMLDateTimeD0Ev(ptr noundef nonnull align 8 dereferenceable(96) %i18) #47, !intel_dtrans_type !1371, !_Intel.Devirt.Call !1273
  br label %bb47

bb47:                                             ; preds = %bb43, %bb40
  %i48 = icmp eq ptr %i11, null
  br i1 %i48, label %bb83, label %bb49

bb49:                                             ; preds = %bb47
  %i50 = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i11, i64 0, i32 0, i32 0, i32 0
  %i51 = load ptr, ptr %i50, align 8, !tbaa !1256
  %i52 = call i1 @llvm.type.test(ptr %i51, metadata !"_ZTSN11xercesc_2_711XMLDateTimeE")
  tail call void @llvm.assume(i1 %i52)
  tail call void @_ZN11xercesc_2_711XMLDateTimeD0Ev(ptr noundef nonnull align 8 dereferenceable(96) %i11) #47, !intel_dtrans_type !1371, !_Intel.Devirt.Call !1273
  br label %bb83

bb53:                                             ; preds = %bb
  %i54 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  br label %bb71

bb55:                                             ; preds = %bb12
  %i56 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  br label %bb64

bb57:                                             ; preds = %bb32, %bb28, %bb26
  %i58 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  %i59 = icmp eq ptr %i18, null
  br i1 %i59, label %bb64, label %bb60

bb60:                                             ; preds = %bb57
  %i61 = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i18, i64 0, i32 0, i32 0, i32 0
  %i62 = load ptr, ptr %i61, align 8, !tbaa !1256
  %i63 = call i1 @llvm.type.test(ptr %i62, metadata !"_ZTSN11xercesc_2_711XMLDateTimeE")
  tail call void @llvm.assume(i1 %i63)
  tail call void @_ZN11xercesc_2_711XMLDateTimeD0Ev(ptr noundef nonnull align 8 dereferenceable(96) %i18) #47, !intel_dtrans_type !1371, !_Intel.Devirt.Call !1273
  br label %bb64

bb64:                                             ; preds = %bb60, %bb57, %bb55
  %i65 = phi { ptr, i32 } [ %i56, %bb55 ], [ %i58, %bb57 ], [ %i58, %bb60 ]
  %i66 = icmp eq ptr %i11, null
  br i1 %i66, label %bb71, label %bb67

bb67:                                             ; preds = %bb64
  %i68 = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i11, i64 0, i32 0, i32 0, i32 0
  %i69 = load ptr, ptr %i68, align 8, !tbaa !1256
  %i70 = call i1 @llvm.type.test(ptr %i69, metadata !"_ZTSN11xercesc_2_711XMLDateTimeE")
  tail call void @llvm.assume(i1 %i70)
  tail call void @_ZN11xercesc_2_711XMLDateTimeD0Ev(ptr noundef nonnull align 8 dereferenceable(96) %i11) #47, !intel_dtrans_type !1371, !_Intel.Devirt.Call !1273
  br label %bb71

bb71:                                             ; preds = %bb67, %bb64, %bb53
  %i72 = phi { ptr, i32 } [ %i54, %bb53 ], [ %i65, %bb64 ], [ %i65, %bb67 ]
  %i73 = extractvalue { ptr, i32 } %i72, 0
  %i74 = extractvalue { ptr, i32 } %i72, 1
  %i75 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #47
  %i76 = icmp eq i32 %i74, %i75
  %i77 = tail call ptr @__cxa_begin_catch(ptr %i73) #47
  br i1 %i76, label %bb78, label %bb79

bb78:                                             ; preds = %bb71
  invoke void @__cxa_rethrow() #51
          to label %bb88 unwind label %bb80

bb79:                                             ; preds = %bb71
  tail call void @__cxa_end_catch()
  br label %bb83

bb80:                                             ; preds = %bb78
  %i81 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb82 unwind label %bb85

bb82:                                             ; preds = %bb80
  resume { ptr, i32 } %i81

bb83:                                             ; preds = %bb79, %bb49, %bb47
  %i84 = phi i32 [ -1, %bb79 ], [ %i41, %bb47 ], [ %i41, %bb49 ]
  ret i32 %i84

bb85:                                             ; preds = %bb80
  %i86 = landingpad { ptr, i32 }
          catch ptr null
  %i87 = extractvalue { ptr, i32 } %i86, 0
  tail call fastcc void @__clang_call_terminate(ptr %i87) #48
  unreachable

bb88:                                             ; preds = %bb78
  unreachable
}

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1372 hidden noundef i32 @_ZN11xercesc_2_717DateTimeValidator13compareValuesEPKNS_9XMLNumberES3_(ptr nocapture noundef nonnull readonly align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3") unnamed_addr #16 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1373 hidden void @_ZN11xercesc_2_717DateTimeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture noundef readnone "intel_dtrans_func_index"="3", i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1374 hidden void @_ZN11xercesc_2_717DateTimeValidator15setMaxInclusiveEPKt(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1375 hidden void @_ZN11xercesc_2_717DateTimeValidator15setMaxExclusiveEPKt(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1376 hidden void @_ZN11xercesc_2_717DateTimeValidator15setMinInclusiveEPKt(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1377 hidden void @_ZN11xercesc_2_717DateTimeValidator15setMinExclusiveEPKt(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1378 hidden void @_ZN11xercesc_2_717DateTimeValidator14setEnumerationEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr nocapture readnone "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
define hidden noundef i32 @_ZN11xercesc_2_717DateTimeValidator12compareDatesEPKNS_11XMLDateTimeES3_b(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1, ptr nocapture noundef readonly "intel_dtrans_func_index"="3" %arg2, i1 zeroext %arg3) unnamed_addr #16 align 2 !intel.dtrans.func.type !1379 !_Intel.Devirt.Target !1380 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 7, !intel-tbaa !1381
  %i4 = load i32, ptr %i, align 4, !tbaa !1381
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg2, i64 0, i32 1, i64 7, !intel-tbaa !1381
  %i6 = load i32, ptr %i5, align 4, !tbaa !1381
  %i7 = icmp eq i32 %i4, %i6
  br i1 %i7, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = tail call fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef %arg1, ptr noundef nonnull %arg2), !range !1370
  br label %bb36

bb10:                                             ; preds = %bb
  %i11 = icmp eq i32 %i4, 1
  br i1 %i11, label %bb12, label %bb23

bb12:                                             ; preds = %bb10
  %i13 = tail call fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime13compareResultEPKS0_S2_bi(ptr noundef nonnull %arg1, ptr noundef nonnull %arg2, i1 noundef zeroext false, i32 noundef 2), !range !1370
  %i14 = tail call fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime13compareResultEPKS0_S2_bi(ptr noundef nonnull %arg1, ptr noundef nonnull %arg2, i1 noundef zeroext false, i32 noundef 3), !range !1370
  %i15 = icmp eq i32 %i13, -1
  %i16 = icmp eq i32 %i14, 1
  %i17 = and i1 %i15, %i16
  %i18 = icmp eq i32 %i13, 1
  %i19 = icmp eq i32 %i14, -1
  %i20 = and i1 %i18, %i19
  %i21 = or i1 %i17, %i20
  %i22 = select i1 %i21, i32 2, i32 %i13
  br label %bb36

bb23:                                             ; preds = %bb10
  %i24 = icmp eq i32 %i6, 1
  br i1 %i24, label %bb25, label %bb36

bb25:                                             ; preds = %bb23
  %i26 = tail call fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime13compareResultEPKS0_S2_bi(ptr noundef nonnull %arg1, ptr noundef nonnull %arg2, i1 noundef zeroext true, i32 noundef 2), !range !1370
  %i27 = tail call fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime13compareResultEPKS0_S2_bi(ptr noundef nonnull %arg1, ptr noundef nonnull %arg2, i1 noundef zeroext true, i32 noundef 3), !range !1370
  %i28 = icmp eq i32 %i26, -1
  %i29 = icmp eq i32 %i27, 1
  %i30 = and i1 %i28, %i29
  %i31 = icmp eq i32 %i26, 1
  %i32 = icmp eq i32 %i27, -1
  %i33 = and i1 %i31, %i32
  %i34 = or i1 %i30, %i33
  %i35 = select i1 %i34, i32 2, i32 %i26
  br label %bb36

bb36:                                             ; preds = %bb25, %bb23, %bb12, %bb8
  %i37 = phi i32 [ %i9, %bb8 ], [ 2, %bb23 ], [ %i22, %bb12 ], [ %i35, %bb25 ]
  ret i32 %i37
}

; Function Attrs: uwtable
define hidden noundef i32 @_ZN11xercesc_2_724DecimalDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(168) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1389 {
bb:
  %i = alloca %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", align 8
  %i4 = alloca %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", align 8
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %i) #47
  call fastcc void @_ZN11xercesc_2_713XMLBigDecimalC2EPKtPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i, ptr noundef %arg1, ptr noundef %arg3)
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %i4) #47
  invoke fastcc void @_ZN11xercesc_2_713XMLBigDecimalC2EPKtPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i4, ptr noundef %arg2, ptr noundef %arg3)
          to label %bb5 unwind label %bb160

bb5:                                              ; preds = %bb
  %i6 = getelementptr %"class._ZTSN11xercesc_2_724DecimalDatatypeValidatorE.xercesc_2_7::DecimalDatatypeValidator", ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1256
  %i8 = tail call i1 @llvm.type.test(ptr %i7, metadata !"_ZTSN11xercesc_2_724DecimalDatatypeValidatorE")
  tail call void @llvm.assume(i1 %i8)
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 0, i32 0, i32 0
  %i10 = load ptr, ptr %i9, align 8, !tbaa !1256
  %i11 = tail call i1 @llvm.type.test(ptr %i10, metadata !"_ZTSN11xercesc_2_713XMLBigDecimalE")
  tail call void @llvm.assume(i1 %i11)
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 1, !intel-tbaa !1391
  %i13 = load i32, ptr %i12, align 8, !tbaa !1391
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 0, i32 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !1256
  %i16 = tail call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_713XMLBigDecimalE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 1, !intel-tbaa !1391
  %i18 = load i32, ptr %i17, align 8, !tbaa !1391
  %i19 = icmp eq i32 %i13, %i18
  br i1 %i19, label %bb28, label %bb20

bb20:                                             ; preds = %bb5
  %i21 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 0, i32 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713XMLBigDecimalE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 1, !intel-tbaa !1391
  %i25 = load i32, ptr %i24, align 8, !tbaa !1391
  %i26 = icmp sgt i32 %i13, %i25
  %i27 = select i1 %i26, i32 1, i32 -1
  br label %bb118

bb28:                                             ; preds = %bb5
  %i29 = icmp eq i32 %i13, 0
  br i1 %i29, label %bb118, label %bb30

bb30:                                             ; preds = %bb28
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 2, !intel-tbaa !1393
  %i32 = load i32, ptr %i31, align 4, !tbaa !1393
  %i33 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 3, !intel-tbaa !1394
  %i34 = load i32, ptr %i33, align 8, !tbaa !1394
  %i35 = sub i32 %i32, %i34
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 2, !intel-tbaa !1393
  %i37 = load i32, ptr %i36, align 4, !tbaa !1393
  %i38 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 3, !intel-tbaa !1394
  %i39 = load i32, ptr %i38, align 8, !tbaa !1394
  %i40 = sub i32 %i37, %i39
  %i41 = icmp ugt i32 %i35, %i40
  br i1 %i41, label %bb118, label %bb42

bb42:                                             ; preds = %bb30
  %i43 = icmp ult i32 %i35, %i40
  br i1 %i43, label %bb44, label %bb46

bb44:                                             ; preds = %bb42
  %i45 = sub nsw i32 0, %i13
  br label %bb118

bb46:                                             ; preds = %bb42
  %i47 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 6, !intel-tbaa !1395
  %i48 = load ptr, ptr %i47, align 8, !tbaa !1395
  %i49 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 6, !intel-tbaa !1395
  %i50 = load ptr, ptr %i49, align 8, !tbaa !1395
  %i51 = icmp eq ptr %i48, null
  %i52 = icmp eq ptr %i50, null
  %i53 = or i1 %i51, %i52
  br i1 %i53, label %bb54, label %bb88

bb54:                                             ; preds = %bb46
  br i1 %i51, label %bb55, label %bb73

bb55:                                             ; preds = %bb54
  br i1 %i52, label %bb70, label %bb56

bb56:                                             ; preds = %bb55
  %i57 = load i16, ptr %i50, align 2, !tbaa !1286
  %i58 = icmp eq i16 %i57, 0
  br i1 %i58, label %bb70, label %bb59

bb59:                                             ; preds = %bb59, %bb56
  %i60 = phi ptr [ %i61, %bb59 ], [ %i50, %bb56 ]
  %i61 = getelementptr inbounds i16, ptr %i60, i64 1
  %i62 = load i16, ptr %i61, align 2, !tbaa !1286
  %i63 = icmp eq i16 %i62, 0
  br i1 %i63, label %bb64, label %bb59, !llvm.loop !1288

bb64:                                             ; preds = %bb59
  %i65 = ptrtoint ptr %i61 to i64
  %i66 = ptrtoint ptr %i50 to i64
  %i67 = sub i64 %i65, %i66
  %i68 = lshr exact i64 %i67, 1
  %i69 = trunc i64 %i68 to i32
  br label %bb70

bb70:                                             ; preds = %bb64, %bb56, %bb55
  %i71 = phi i32 [ %i69, %bb64 ], [ 0, %bb56 ], [ 0, %bb55 ]
  %i72 = sub i32 0, %i71
  br label %bb109

bb73:                                             ; preds = %bb54
  br i1 %i52, label %bb74, label %bb88

bb74:                                             ; preds = %bb73
  %i75 = load i16, ptr %i48, align 2, !tbaa !1286
  %i76 = icmp eq i16 %i75, 0
  br i1 %i76, label %bb118, label %bb77

bb77:                                             ; preds = %bb77, %bb74
  %i78 = phi ptr [ %i79, %bb77 ], [ %i48, %bb74 ]
  %i79 = getelementptr inbounds i16, ptr %i78, i64 1
  %i80 = load i16, ptr %i79, align 2, !tbaa !1286
  %i81 = icmp eq i16 %i80, 0
  br i1 %i81, label %bb82, label %bb77, !llvm.loop !1288

bb82:                                             ; preds = %bb77
  %i83 = ptrtoint ptr %i79 to i64
  %i84 = ptrtoint ptr %i48 to i64
  %i85 = sub i64 %i83, %i84
  %i86 = lshr exact i64 %i85, 1
  %i87 = trunc i64 %i86 to i32
  br label %bb109

bb88:                                             ; preds = %bb73, %bb46
  %i89 = load i16, ptr %i48, align 2, !tbaa !1286
  %i90 = load i16, ptr %i50, align 2, !tbaa !1286
  %i91 = icmp eq i16 %i89, %i90
  br i1 %i91, label %bb98, label %bb92

bb92:                                             ; preds = %bb103, %bb88
  %i93 = phi i16 [ %i89, %bb88 ], [ %i106, %bb103 ]
  %i94 = phi i16 [ %i90, %bb88 ], [ %i107, %bb103 ]
  %i95 = zext i16 %i94 to i32
  %i96 = zext i16 %i93 to i32
  %i97 = sub nsw i32 %i96, %i95
  br label %bb109

bb98:                                             ; preds = %bb103, %bb88
  %i99 = phi i16 [ %i106, %bb103 ], [ %i89, %bb88 ]
  %i100 = phi ptr [ %i105, %bb103 ], [ %i50, %bb88 ]
  %i101 = phi ptr [ %i104, %bb103 ], [ %i48, %bb88 ]
  %i102 = icmp eq i16 %i99, 0
  br i1 %i102, label %bb118, label %bb103

bb103:                                            ; preds = %bb98
  %i104 = getelementptr inbounds i16, ptr %i101, i64 1
  %i105 = getelementptr inbounds i16, ptr %i100, i64 1
  %i106 = load i16, ptr %i104, align 2, !tbaa !1286
  %i107 = load i16, ptr %i105, align 2, !tbaa !1286
  %i108 = icmp eq i16 %i106, %i107
  br i1 %i108, label %bb98, label %bb92, !llvm.loop !1290

bb109:                                            ; preds = %bb92, %bb82, %bb70
  %i110 = phi i32 [ %i72, %bb70 ], [ %i97, %bb92 ], [ %i87, %bb82 ]
  %i111 = icmp sgt i32 %i110, 0
  %i112 = icmp slt i32 %i110, 0
  %i113 = sub nsw i32 0, %i13
  %i114 = freeze i1 %i112
  %i115 = freeze i1 %i111
  %i116 = select i1 %i114, i32 %i113, i32 0
  %i117 = select i1 %i115, i32 %i13, i32 %i116
  br label %bb118

bb118:                                            ; preds = %bb109, %bb98, %bb74, %bb44, %bb30, %bb28, %bb20
  %i119 = phi i32 [ %i27, %bb20 ], [ 0, %bb28 ], [ %i45, %bb44 ], [ %i13, %bb30 ], [ 0, %bb98 ], [ 0, %bb74 ], [ %i117, %bb109 ]
  %i120 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_713XMLBigDecimalE.0, i64 0, i64 2), ptr %i120, align 8, !tbaa !1256
  %i121 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 5, !intel-tbaa !1396
  %i122 = load ptr, ptr %i121, align 8, !tbaa !1396
  %i123 = icmp eq ptr %i122, null
  br i1 %i123, label %bb139, label %bb124

bb124:                                            ; preds = %bb118
  %i125 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i4, i64 0, i32 7, !intel-tbaa !1397
  %i126 = load ptr, ptr %i125, align 8, !tbaa !1397
  %i127 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i126, i64 0, i32 0
  %i128 = load ptr, ptr %i127, align 8, !tbaa !1256
  %i129 = tail call i1 @llvm.type.test(ptr %i128, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i129)
  %i130 = getelementptr inbounds ptr, ptr %i128, i64 3
  %i131 = load ptr, ptr %i130, align 8
  %i132 = icmp eq ptr %i131, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i132, label %bb133, label %bb134

bb133:                                            ; preds = %bb124
  tail call void @_ZdlPv(ptr noundef nonnull %i122) #47
  br label %bb139

bb134:                                            ; preds = %bb124
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb135 unwind label %bb136, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb135:                                            ; preds = %bb134
  unreachable

bb136:                                            ; preds = %bb134
  %i137 = landingpad { ptr, i32 }
          catch ptr null
  %i138 = extractvalue { ptr, i32 } %i137, 0
  tail call fastcc void @__clang_call_terminate(ptr %i138) #48
  unreachable

bb139:                                            ; preds = %bb133, %bb118
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %i4) #47
  %i140 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_713XMLBigDecimalE.0, i64 0, i64 2), ptr %i140, align 8, !tbaa !1256
  %i141 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 5, !intel-tbaa !1396
  %i142 = load ptr, ptr %i141, align 8, !tbaa !1396
  %i143 = icmp eq ptr %i142, null
  br i1 %i143, label %bb159, label %bb144

bb144:                                            ; preds = %bb139
  %i145 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %i, i64 0, i32 7, !intel-tbaa !1397
  %i146 = load ptr, ptr %i145, align 8, !tbaa !1397
  %i147 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i146, i64 0, i32 0
  %i148 = load ptr, ptr %i147, align 8, !tbaa !1256
  %i149 = tail call i1 @llvm.type.test(ptr %i148, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i149)
  %i150 = getelementptr inbounds ptr, ptr %i148, i64 3
  %i151 = load ptr, ptr %i150, align 8
  %i152 = icmp eq ptr %i151, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i152, label %bb153, label %bb154

bb153:                                            ; preds = %bb144
  tail call void @_ZdlPv(ptr noundef nonnull %i142) #47
  br label %bb159

bb154:                                            ; preds = %bb144
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb155 unwind label %bb156, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb155:                                            ; preds = %bb154
  unreachable

bb156:                                            ; preds = %bb154
  %i157 = landingpad { ptr, i32 }
          catch ptr null
  %i158 = extractvalue { ptr, i32 } %i157, 0
  tail call fastcc void @__clang_call_terminate(ptr %i158) #48
  unreachable

bb159:                                            ; preds = %bb153, %bb139
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %i) #47
  ret i32 %i119

bb160:                                            ; preds = %bb
  %i161 = landingpad { ptr, i32 }
          cleanup
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %i4) #47
  call void @_ZN11xercesc_2_713XMLBigDecimalD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #47
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %i) #47
  resume { ptr, i32 } %i161
}

; Function Attrs: mustprogress nofree norecurse noreturn nounwind uwtable
define hidden void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #27 align 2 !intel.dtrans.func.type !1399 {
bb:
  %i = load ptr, ptr @stderr, align 8, !tbaa !1401
  %i2 = tail call fastcc noundef ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg1)
  %i3 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %i, ptr noundef nonnull @.str.817, ptr noundef nonnull %i2) #52
  tail call void @exit(i32 noundef -1) #48
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !1403 dso_local noundef i32 @fprintf(ptr nocapture noundef "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ...) local_unnamed_addr #28

; Function Attrs: nofree noreturn nounwind
declare dso_local void @exit(i32 noundef) local_unnamed_addr #29

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1404 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1406 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1407 hidden noundef zeroext i1 @_ZNK11xercesc_2_723DoubleDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1408 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1409 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_723DoubleDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1410 hidden noundef i32 @_ZN11xercesc_2_723DoubleDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1411 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_723DoubleDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1412 hidden noundef i32 @_ZN11xercesc_2_723DoubleDatatypeValidator13compareValuesEPKNS_9XMLNumberES3_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3") unnamed_addr #16 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1413 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture noundef readnone "intel_dtrans_func_index"="3", i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1414 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator15setMaxInclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1415 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator15setMaxExclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1416 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator15setMinInclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1417 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator15setMinExclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1418 hidden void @_ZN11xercesc_2_723DoubleDatatypeValidator14setEnumerationEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1419 hidden void @_ZN11xercesc_2_725DurationDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1421 hidden void @_ZN11xercesc_2_725DurationDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1422 hidden noundef zeroext i1 @_ZNK11xercesc_2_725DurationDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1423 hidden void @_ZN11xercesc_2_725DurationDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1424 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_725DurationDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1425 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_725DurationDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1426 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_725DurationDatatypeValidator5parseEPKtPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1427 hidden void @_ZN11xercesc_2_725DurationDatatypeValidator5parseEPNS_11XMLDateTimeE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef nonnull "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1428 hidden noundef i32 @_ZN11xercesc_2_725DurationDatatypeValidator12compareDatesEPKNS_11XMLDateTimeES3_b(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef nonnull readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3", i1 noundef zeroext) unnamed_addr #14 align 2

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #30 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1429 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1431
  %i2 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1436
  store ptr null, ptr %i2, align 8, !tbaa !1436
  %i3 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !1437
  %i4 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 1, !intel-tbaa !1437
  %i5 = load ptr, ptr %i4, align 8, !tbaa !1437
  store ptr %i5, ptr %i3, align 8, !tbaa !1437
  %i6 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !1431
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1431
  %i8 = icmp eq ptr %i7, null
  br i1 %i8, label %bb109, label %bb9

bb9:                                              ; preds = %bb
  %i10 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !1436
  %i11 = load ptr, ptr %i10, align 8, !tbaa !1436
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i11, i64 0, i32 1, !intel-tbaa !1438
  %i13 = load i32, ptr %i12, align 4, !tbaa !1438
  %i14 = load ptr, ptr %i3, align 8, !tbaa !1437
  %i15 = invoke fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i14)
          to label %bb16 unwind label %bb20

bb16:                                             ; preds = %bb9
  %i17 = load ptr, ptr %i6, align 8, !tbaa !1431
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(32) %i15, ptr noundef nonnull align 8 dereferenceable(32) %i17)
          to label %bb18 unwind label %bb22

bb18:                                             ; preds = %bb16
  store ptr %i15, ptr %i, align 8, !tbaa !1431
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
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i15) #47
  br label %bb96

bb24:                                             ; preds = %bb86, %bb18
  %i25 = phi i32 [ %i92, %bb86 ], [ 0, %bb18 ]
  %i26 = load ptr, ptr %i2, align 8, !tbaa !1436
  %i27 = load ptr, ptr %i10, align 8, !tbaa !1436
  %i28 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 1, !intel-tbaa !1438
  %i29 = load i32, ptr %i28, align 4, !tbaa !1438
  %i30 = icmp ugt i32 %i29, %i25
  br i1 %i30, label %bb39, label %bb31

bb31:                                             ; preds = %bb24
  %i32 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i33 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 4, !intel-tbaa !1441
  %i34 = load ptr, ptr %i33, align 8, !tbaa !1441
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i32, ptr noundef nonnull @.str.4453, i32 noundef 249, i32 noundef 116, ptr noundef %i34)
          to label %bb35 unwind label %bb37

bb35:                                             ; preds = %bb31
  invoke void @__cxa_throw(ptr nonnull %i32, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
          to label %bb36 unwind label %bb94

bb36:                                             ; preds = %bb35
  unreachable

bb37:                                             ; preds = %bb31
  %i38 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call void @__cxa_free_exception(ptr %i32) #47
  br label %bb96

bb39:                                             ; preds = %bb24
  %i40 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i27, i64 0, i32 3, !intel-tbaa !1442
  %i41 = load ptr, ptr %i40, align 8, !tbaa !1442
  %i42 = zext i32 %i25 to i64
  %i43 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i41, i64 %i42, i32 2
  %i44 = load ptr, ptr %i43, align 8, !tbaa !1443
  %i45 = load ptr, ptr %i3, align 8, !tbaa !1437
  %i46 = icmp eq ptr %i44, null
  br i1 %i46, label %bb86, label %bb47

bb47:                                             ; preds = %bb39
  %i48 = load i16, ptr %i44, align 2, !tbaa !1286
  %i49 = icmp eq i16 %i48, 0
  br i1 %i49, label %bb61, label %bb50

bb50:                                             ; preds = %bb50, %bb47
  %i51 = phi ptr [ %i52, %bb50 ], [ %i44, %bb47 ]
  %i52 = getelementptr inbounds i16, ptr %i51, i64 1
  %i53 = load i16, ptr %i52, align 2, !tbaa !1286
  %i54 = icmp eq i16 %i53, 0
  br i1 %i54, label %bb55, label %bb50, !llvm.loop !1444

bb55:                                             ; preds = %bb50
  %i56 = ptrtoint ptr %i52 to i64
  %i57 = ptrtoint ptr %i44 to i64
  %i58 = sub i64 %i56, %i57
  %i59 = add i64 %i58, 2
  %i60 = and i64 %i59, 8589934590
  br label %bb61

bb61:                                             ; preds = %bb55, %bb47
  %i62 = phi i64 [ %i60, %bb55 ], [ 2, %bb47 ]
  %i63 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i45, i64 0, i32 0
  %i64 = load ptr, ptr %i63, align 8, !tbaa !1256
  %i65 = tail call i1 @llvm.type.test(ptr %i64, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i65)
  %i66 = getelementptr inbounds ptr, ptr %i64, i64 2
  %i67 = load ptr, ptr %i66, align 8
  %i68 = icmp eq ptr %i67, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i68, label %bb69, label %bb83

bb69:                                             ; preds = %bb61
  %i70 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i62) #50
          to label %bb82 unwind label %bb71

bb71:                                             ; preds = %bb69
  %i72 = landingpad { ptr, i32 }
          catch ptr null
  %i73 = extractvalue { ptr, i32 } %i72, 0
  %i74 = tail call ptr @__cxa_begin_catch(ptr %i73) #47
  %i75 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i75, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i80) #48
  unreachable

bb81:                                             ; preds = %bb71
  unreachable

bb82:                                             ; preds = %bb69
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i70, ptr nonnull align 2 %i44, i64 %i62, i1 false)
  br label %bb86

bb83:                                             ; preds = %bb61
  %i84 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb85 unwind label %bb94, !intel_dtrans_type !1272

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
  br i1 %i93, label %bb109, label %bb24, !llvm.loop !1445

bb94:                                             ; preds = %bb83, %bb35
  %i95 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb96

bb96:                                             ; preds = %bb94, %bb76, %bb37, %bb22, %bb20
  %i97 = phi { ptr, i32 } [ %i21, %bb20 ], [ %i23, %bb22 ], [ %i38, %bb37 ], [ %i95, %bb94 ], [ %i77, %bb76 ]
  %i98 = extractvalue { ptr, i32 } %i97, 1
  %i99 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #47
  %i100 = icmp eq i32 %i98, %i99
  br i1 %i100, label %bb101, label %bb106

bb101:                                            ; preds = %bb96
  %i102 = extractvalue { ptr, i32 } %i97, 0
  %i103 = tail call ptr @__cxa_begin_catch(ptr %i102) #47
  invoke void @__cxa_rethrow() #51
          to label %bb113 unwind label %bb104

bb104:                                            ; preds = %bb101
  %i105 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb107 unwind label %bb110

bb106:                                            ; preds = %bb96
  tail call void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr noundef nonnull align 8 dereferenceable(32) %arg), !intel_dtrans_type !1446
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
  tail call fastcc void @__clang_call_terminate(ptr %i112) #48
  unreachable

bb113:                                            ; preds = %bb101
  unreachable
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #30 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1447 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !1448
  %i2 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 0, !intel-tbaa !1448
  %i3 = load i8, ptr %i2, align 8, !tbaa !1448, !range !1451, !noundef !1452
  store i8 %i3, ptr %i, align 8, !tbaa !1448
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1453
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 1, !intel-tbaa !1453
  %i6 = load i32, ptr %i5, align 4, !tbaa !1453
  store i32 %i6, ptr %i4, align 4, !tbaa !1453
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !1454
  %i8 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 2, !intel-tbaa !1454
  %i9 = load i32, ptr %i8, align 8, !tbaa !1454
  store i32 %i9, ptr %i7, align 8, !tbaa !1454
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !1455
  store ptr null, ptr %i10, align 8, !tbaa !1455
  %i11 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1456
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 4, !intel-tbaa !1456
  %i13 = load ptr, ptr %i12, align 8, !tbaa !1456
  store ptr %i13, ptr %i11, align 8, !tbaa !1456
  %i14 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i13, i64 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !1256
  %i16 = tail call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = getelementptr inbounds ptr, ptr %i15, i64 2
  %i18 = load ptr, ptr %i17, align 8
  %i19 = icmp eq ptr %i18, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i19, label %bb20, label %bb36

bb20:                                             ; preds = %bb
  %i21 = zext i32 %i9 to i64
  %i22 = mul nuw nsw i64 %i21, 24
  %i23 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i22) #50
          to label %bb38 unwind label %bb24

bb24:                                             ; preds = %bb20
  %i25 = landingpad { ptr, i32 }
          catch ptr null
  %i26 = extractvalue { ptr, i32 } %i25, 0
  %i27 = tail call ptr @__cxa_begin_catch(ptr %i26) #47
  %i28 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i28, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i34) #48
  unreachable

bb35:                                             ; preds = %bb24
  unreachable

bb36:                                             ; preds = %bb
  %i37 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb38:                                             ; preds = %bb20
  store ptr %i23, ptr %i10, align 8, !tbaa !1455
  %i39 = load i32, ptr %i7, align 8, !tbaa !1454
  %i40 = zext i32 %i39 to i64
  %i41 = mul nuw nsw i64 %i40, 24
  tail call void @llvm.memset.p0.i64(ptr nonnull align 8 %i23, i8 0, i64 %i41, i1 false)
  %i42 = load i32, ptr %i4, align 4, !tbaa !1453
  %i43 = icmp eq i32 %i42, 0
  br i1 %i43, label %bb49, label %bb44

bb44:                                             ; preds = %bb38
  %i45 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 3, !intel-tbaa !1455
  %i46 = load ptr, ptr %i45, align 8, !tbaa !1455
  %i47 = load ptr, ptr %i10, align 8, !tbaa !1455
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
  %i57 = load ptr, ptr %i52, align 8, !tbaa !1457
  %i58 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 0
  %i59 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 2
  %i60 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i47, i64 %i51, i32 1
  store ptr %i55, ptr %i60, align 8
  store ptr %i56, ptr %i59, align 8
  store ptr %i57, ptr %i58, align 8, !tbaa !1457
  %i61 = add nuw nsw i64 %i51, 1
  %i62 = icmp eq i64 %i61, %i48
  br i1 %i62, label %bb49, label %bb50, !llvm.loop !1459
}

; Function Attrs: nounwind uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #31 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1460 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1431
  %i1 = load ptr, ptr %i, align 8, !tbaa !1431
  %i2 = icmp eq ptr %i1, null
  br i1 %i2, label %bb33, label %bb3

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i1, i64 0, i32 4, !intel-tbaa !1456
  %i5 = load ptr, ptr %i4, align 8, !tbaa !1456
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i1, i64 0, i32 3, !intel-tbaa !1455
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1455
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i5, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !1256
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 3
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i13, label %bb19, label %bb14

bb14:                                             ; preds = %bb3
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb15 unwind label %bb16, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb15:                                             ; preds = %bb14
  unreachable

bb16:                                             ; preds = %bb14
  %i17 = landingpad { ptr, i32 }
          catch ptr null
  %i18 = extractvalue { ptr, i32 } %i17, 0
  tail call fastcc void @__clang_call_terminate(ptr %i18) #48
  unreachable

bb19:                                             ; preds = %bb3
  tail call void @_ZdlPv(ptr noundef %i7) #47
  %i20 = getelementptr inbounds i8, ptr %i1, i64 -8, !intel-tbaa !1461
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1271
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 3
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i26, label %bb32, label %bb27

bb27:                                             ; preds = %bb19
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb28 unwind label %bb29, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb28:                                             ; preds = %bb27
  unreachable

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  tail call fastcc void @__clang_call_terminate(ptr %i31) #48
  unreachable

bb32:                                             ; preds = %bb19
  tail call void @_ZdlPv(ptr noundef nonnull %i20) #47
  br label %bb33

bb33:                                             ; preds = %bb32, %bb
  ret void
}

; Function Attrs: mustprogress uwtable
define dso_local noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #32 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1462 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1453
  %i2 = load i32, ptr %i, align 4, !tbaa !1453
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1456
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1456
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.6.5757, i32 noundef 206, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #47
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !1455
  %i13 = load ptr, ptr %i12, align 8, !tbaa !1455
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i13, i64 %i14, i32 0
  ret ptr %i15
}

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1463 hidden void @_ZN11xercesc_2_722FloatDatatypeValidatorD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: nounwind uwtable
declare !intel.dtrans.func.type !1465 hidden void @_ZN11xercesc_2_722FloatDatatypeValidatorD0Ev(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1") unnamed_addr #11 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1466 hidden noundef zeroext i1 @_ZNK11xercesc_2_722FloatDatatypeValidator14isSerializableEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #13 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1467 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator9serializeERNS_16XSerializeEngineE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef nonnull align 8 dereferenceable(100) "intel_dtrans_func_index"="2") unnamed_addr #16 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1468 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_722FloatDatatypeValidator12getProtoTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2") unnamed_addr #13 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1469 hidden noundef i32 @_ZN11xercesc_2_722FloatDatatypeValidator7compareEPKtS2_PNS_13MemoryManagerE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1470 hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_722FloatDatatypeValidator11newInstanceEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i32 noundef, ptr noundef "intel_dtrans_func_index"="5") unnamed_addr #14 align 2

; Function Attrs: mustprogress uwtable
declare !intel.dtrans.func.type !1471 hidden noundef i32 @_ZN11xercesc_2_722FloatDatatypeValidator13compareValuesEPKNS_9XMLNumberES3_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ptr nocapture noundef readonly "intel_dtrans_func_index"="3") unnamed_addr #16 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1472 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator12checkContentEPKtPNS_17ValidationContextEbPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr nocapture noundef readnone "intel_dtrans_func_index"="3", i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="4") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1473 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator15setMaxInclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1474 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator15setMaxExclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1475 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator15setMinInclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1476 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator15setMinExclusiveEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1477 hidden void @_ZN11xercesc_2_722FloatDatatypeValidator14setEnumerationEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(160) "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #14 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1478 hidden noundef i32 @_ZN11xercesc_2_77HashPtr10getHashValEPKvjPNS_13MemoryManagerE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", i32 noundef, ptr nocapture readnone "intel_dtrans_func_index"="3") unnamed_addr #13 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1480 hidden noundef zeroext i1 @_ZN11xercesc_2_77HashPtr6equalsEPKvS2_(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef readnone "intel_dtrans_func_index"="2", ptr noundef readnone "intel_dtrans_func_index"="3") unnamed_addr #13 align 2

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_712FieldMatcher7matchedEPKtPNS_17DatatypeValidatorEb(ptr nocapture noundef nonnull readonly align 8 dereferenceable(96) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, i1 noundef zeroext %arg3) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1481 !_Intel.Devirt.Target !1380 {
bb:
  br i1 %arg3, label %bb4, label %bb26

bb4:                                              ; preds = %bb
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 1, !intel-tbaa !1483
  %i5 = load ptr, ptr %i, align 8, !tbaa !1483
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 2, !intel-tbaa !1492
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1492
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field", ptr %i7, i64 0, i32 2, !intel-tbaa !1493
  %i9 = load ptr, ptr %i8, align 8, !tbaa !1493
  %i10 = load i8, ptr %i5, align 1, !tbaa !1496, !range !1451, !noundef !1452
  %i11 = getelementptr i8, ptr %i5, i64 48
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq i8 %i10, 0
  br i1 %i13, label %bb26, label %bb14

bb14:                                             ; preds = %bb4
  %i15 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i9, i64 0, i32 0, i32 0
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1256
  %i17 = tail call i1 @llvm.type.test(ptr %i16, metadata !"_ZTSN11xercesc_2_718IdentityConstraintE")
  tail call void @llvm.assume(i1 %i17)
  %i18 = getelementptr inbounds ptr, ptr %i16, i64 5
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, @_ZNK11xercesc_2_76IC_Key7getTypeEv
  br i1 %i20, label %bb21, label %bb26

bb21:                                             ; preds = %bb14
  %i22 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i12, i64 0, i32 50, !intel-tbaa !1500
  %i23 = load ptr, ptr %i22, align 8, !tbaa !1500
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i9, i64 0, i32 2, !intel-tbaa !1537
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1537
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i23, i32 noundef 103, ptr noundef %i25, ptr noundef null, ptr noundef null)
  br label %bb26

bb26:                                             ; preds = %bb21, %bb14, %bb4, %bb
  %i27 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 1, !intel-tbaa !1483
  %i28 = load ptr, ptr %i27, align 8, !tbaa !1483
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 3, !intel-tbaa !1541
  %i30 = load ptr, ptr %i29, align 8, !tbaa !1541
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher", ptr %arg, i64 0, i32 2, !intel-tbaa !1492
  %i32 = load ptr, ptr %i31, align 8, !tbaa !1492
  %i33 = getelementptr i8, ptr %i30, i64 16
  %i34 = load ptr, ptr %i33, align 8, !tbaa !1542
  %i35 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 3, !intel-tbaa !1547
  %i37 = load ptr, ptr %i36, align 8, !tbaa !1547
  %i38 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 2, !intel-tbaa !1551
  %i39 = load i32, ptr %i38, align 4, !tbaa !1551
  %i40 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i37, i64 0, i32 0
  %i41 = load ptr, ptr %i40, align 8, !tbaa !1256
  %i42 = tail call i1 @llvm.type.test(ptr %i41, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i42)
  %i43 = load ptr, ptr %i41, align 8
  %i44 = icmp eq ptr %i43, @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i44, label %bb45, label %bb78

bb45:                                             ; preds = %bb26
  %i46 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 0, !intel-tbaa !1552
  %i47 = load i32, ptr %i46, align 4, !tbaa !1552
  %i48 = icmp ult i32 %i47, 65
  br i1 %i48, label %bb49, label %bb56

bb49:                                             ; preds = %bb45
  %i50 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 2, !intel-tbaa !1555
  %i51 = load i32, ptr %i50, align 4, !tbaa !1555
  %i52 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 3, !intel-tbaa !1556
  %i53 = load i32, ptr %i52, align 4, !tbaa !1556
  %i54 = mul i32 %i53, 31
  %i55 = add i32 %i54, %i51
  br label %bb75

bb56:                                             ; preds = %bb45
  %i57 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 1, !intel-tbaa !1557
  %i58 = load i32, ptr %i57, align 4, !tbaa !1557
  %i59 = add i32 %i58, -1
  %i60 = icmp sgt i32 %i59, -1
  br i1 %i60, label %bb61, label %bb75

bb61:                                             ; preds = %bb56
  %i62 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 4, !intel-tbaa !1558
  %i63 = load ptr, ptr %i62, align 8, !tbaa !1558
  %i64 = zext i32 %i59 to i64
  br label %bb65

bb65:                                             ; preds = %bb65, %bb61
  %i66 = phi i64 [ %i74, %bb65 ], [ %i64, %bb61 ]
  %i67 = phi i32 [ %i72, %bb65 ], [ 0, %bb61 ]
  %i68 = getelementptr inbounds i8, ptr %i63, i64 %i66
  %i69 = load i8, ptr %i68, align 1, !tbaa !1461
  %i70 = zext i8 %i69 to i32
  %i71 = mul nsw i32 %i67, 31
  %i72 = add nsw i32 %i71, %i70
  %i73 = icmp eq i64 %i66, 0
  %i74 = add nsw i64 %i66, -1
  br i1 %i73, label %bb75, label %bb65, !llvm.loop !1559

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
  %i88 = load i16, ptr %i32, align 2, !tbaa !1286
  %i89 = icmp eq i16 %i88, 0
  br i1 %i89, label %bb110, label %bb90

bb90:                                             ; preds = %bb87
  %i91 = zext i16 %i88 to i32
  %i92 = getelementptr inbounds i16, ptr %i32, i64 1
  %i93 = load i16, ptr %i92, align 2, !tbaa !1286
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
  %i105 = load i16, ptr %i104, align 2, !tbaa !1286
  %i106 = icmp eq i16 %i105, 0
  br i1 %i106, label %bb107, label %bb95, !llvm.loop !1560

bb107:                                            ; preds = %bb95, %bb90
  %i108 = phi i32 [ %i91, %bb90 ], [ %i103, %bb95 ]
  %i109 = urem i32 %i108, %i39
  br label %bb110

bb110:                                            ; preds = %bb107, %bb87, %bb85, %bb80, %bb75
  %i111 = phi i32 [ %i77, %bb75 ], [ %i84, %bb80 ], [ %i109, %bb107 ], [ 0, %bb87 ], [ 0, %bb85 ]
  %i112 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i34, i64 0, i32 1, !intel-tbaa !1561
  %i113 = load ptr, ptr %i112, align 8, !tbaa !1561
  %i114 = zext i32 %i111 to i64
  %i115 = getelementptr inbounds ptr, ptr %i113, i64 %i114
  %i116 = load ptr, ptr %i115, align 8, !tbaa !1562
  %i117 = icmp eq ptr %i116, null
  br i1 %i117, label %bb204, label %bb118

bb118:                                            ; preds = %bb200, %bb110
  %i119 = phi ptr [ %i202, %bb200 ], [ %i116, %bb110 ]
  %i120 = load ptr, ptr %i36, align 8, !tbaa !1547
  %i121 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 2, !intel-tbaa !1564
  %i122 = load ptr, ptr %i121, align 8, !tbaa !1564
  %i123 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i120, i64 0, i32 0
  %i124 = load ptr, ptr %i123, align 8, !tbaa !1256
  %i125 = tail call i1 @llvm.type.test(ptr %i124, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i125)
  %i126 = getelementptr inbounds ptr, ptr %i124, i64 1
  %i127 = load ptr, ptr %i126, align 8
  %i128 = icmp eq ptr %i127, @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_
  br i1 %i128, label %bb129, label %bb157

bb129:                                            ; preds = %bb118
  %i130 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 0, !intel-tbaa !1552
  %i131 = load i32, ptr %i130, align 4, !tbaa !1552
  %i132 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 0, !intel-tbaa !1552
  %i133 = load i32, ptr %i132, align 4, !tbaa !1552
  %i134 = icmp eq i32 %i131, %i133
  br i1 %i134, label %bb135, label %bb200

bb135:                                            ; preds = %bb129
  %i136 = icmp ult i32 %i131, 65
  br i1 %i136, label %bb188, label %bb137

bb137:                                            ; preds = %bb135
  %i138 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 1, !intel-tbaa !1557
  %i139 = load i32, ptr %i138, align 4
  %i140 = icmp eq i32 %i139, 0
  br i1 %i140, label %bb211, label %bb141

bb141:                                            ; preds = %bb137
  %i142 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 4
  %i143 = load ptr, ptr %i142, align 8, !tbaa !1558
  %i144 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 4, !intel-tbaa !1558
  %i145 = load ptr, ptr %i144, align 8, !tbaa !1558
  %i146 = zext i32 %i139 to i64
  br label %bb150

bb147:                                            ; preds = %bb150
  %i148 = add nuw nsw i64 %i151, 1
  %i149 = icmp eq i64 %i148, %i146
  br i1 %i149, label %bb211, label %bb150, !llvm.loop !1567

bb150:                                            ; preds = %bb147, %bb141
  %i151 = phi i64 [ 0, %bb141 ], [ %i148, %bb147 ]
  %i152 = getelementptr inbounds i8, ptr %i143, i64 %i151
  %i153 = load i8, ptr %i152, align 1, !tbaa !1461
  %i154 = getelementptr inbounds i8, ptr %i145, i64 %i151
  %i155 = load i8, ptr %i154, align 1, !tbaa !1461
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
  %i166 = load i16, ptr %i32, align 2, !tbaa !1286
  %i167 = load i16, ptr %i122, align 2, !tbaa !1286
  %i168 = icmp eq i16 %i166, %i167
  br i1 %i168, label %bb177, label %bb200

bb169:                                            ; preds = %bb161
  br i1 %i162, label %bb173, label %bb170

bb170:                                            ; preds = %bb169
  %i171 = load i16, ptr %i32, align 2, !tbaa !1286
  %i172 = icmp eq i16 %i171, 0
  br i1 %i172, label %bb173, label %bb200

bb173:                                            ; preds = %bb170, %bb169
  br i1 %i163, label %bb211, label %bb174

bb174:                                            ; preds = %bb173
  %i175 = load i16, ptr %i122, align 2, !tbaa !1286
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
  %i185 = load i16, ptr %i183, align 2, !tbaa !1286
  %i186 = load i16, ptr %i184, align 2, !tbaa !1286
  %i187 = icmp eq i16 %i185, %i186
  br i1 %i187, label %bb177, label %bb200, !llvm.loop !1568

bb188:                                            ; preds = %bb135
  %i189 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 2, !intel-tbaa !1555
  %i190 = load i32, ptr %i189, align 4, !tbaa !1555
  %i191 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 2, !intel-tbaa !1555
  %i192 = load i32, ptr %i191, align 4, !tbaa !1555
  %i193 = icmp eq i32 %i190, %i192
  %i194 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i32, i64 0, i32 3
  %i195 = load i32, ptr %i194, align 4
  %i196 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i122, i64 0, i32 3
  %i197 = load i32, ptr %i196, align 4
  %i198 = icmp eq i32 %i195, %i197
  %i199 = select i1 %i193, i1 %i198, i1 false
  br i1 %i199, label %bb211, label %bb200

bb200:                                            ; preds = %bb188, %bb182, %bb174, %bb170, %bb165, %bb159, %bb150, %bb129
  %i201 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 1, !intel-tbaa !1569
  %i202 = load ptr, ptr %i201, align 8, !tbaa !1562
  %i203 = icmp eq ptr %i202, null
  br i1 %i203, label %bb204, label %bb118, !llvm.loop !1570

bb204:                                            ; preds = %bb200, %bb110
  %i205 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  invoke fastcc void @_ZN11xercesc_2_722NoSuchElementExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i205, ptr noundef nonnull @.str.4316, i32 noundef 153, i32 noundef 50, ptr noundef %i35)
          to label %bb206 unwind label %bb209

bb206:                                            ; preds = %bb204
  tail call void @__cxa_throw(ptr nonnull %i205, ptr nonnull @_ZTIN11xercesc_2_722NoSuchElementExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb207:                                            ; preds = %bb671, %bb476, %bb425, %bb386, %bb289, %bb267, %bb209
  %i208 = phi { ptr, i32 } [ %i268, %bb267 ], [ %i290, %bb289 ], [ %i210, %bb209 ], [ %i477, %bb476 ], [ %i387, %bb386 ], [ %i426, %bb425 ], [ %i672, %bb671 ]
  resume { ptr, i32 } %i208

bb209:                                            ; preds = %bb204
  %i210 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i205) #47
  br label %bb207

bb211:                                            ; preds = %bb188, %bb177, %bb174, %bb173, %bb159, %bb147, %bb137
  %i212 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i119, i64 0, i32 0, !intel-tbaa !1571
  %i213 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i212, i64 0, i32 0
  %i214 = load i8, ptr %i213, align 1, !tbaa !1571, !range !1451, !noundef !1452
  %i215 = icmp eq i8 %i214, 0
  br i1 %i215, label %bb216, label %bb225

bb216:                                            ; preds = %bb211
  %i217 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !1496
  %i218 = load i8, ptr %i217, align 8, !tbaa !1496, !range !1451, !noundef !1452
  %i219 = icmp eq i8 %i218, 0
  br i1 %i219, label %bb225, label %bb220

bb220:                                            ; preds = %bb216
  %i221 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1572
  %i222 = load ptr, ptr %i221, align 8, !tbaa !1572
  %i223 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i222, i64 0, i32 50, !intel-tbaa !1500
  %i224 = load ptr, ptr %i223, align 8, !tbaa !1500
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr noundef nonnull align 8 dereferenceable(40) %i224, i32 noundef 97)
  br label %bb225

bb225:                                            ; preds = %bb220, %bb216, %bb211
  %i226 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 3, !intel-tbaa !1573
  %i227 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !1431
  %i228 = load ptr, ptr %i227, align 8, !tbaa !1431
  %i229 = icmp eq ptr %i228, null
  br i1 %i229, label %bb245, label %bb230

bb230:                                            ; preds = %bb225
  %i231 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i228, i64 0, i32 1, !intel-tbaa !1453
  %i232 = load i32, ptr %i231, align 4, !tbaa !1453
  %i233 = icmp eq i32 %i232, 0
  br i1 %i233, label %bb245, label %bb234

bb234:                                            ; preds = %bb240, %bb230
  %i235 = phi i32 [ %i241, %bb240 ], [ 0, %bb230 ]
  %i236 = load ptr, ptr %i227, align 8, !tbaa !1431
  %i237 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i236, i32 noundef %i235)
  %i238 = load ptr, ptr %i237, align 8, !tbaa !1457
  %i239 = icmp eq ptr %i238, %i32
  br i1 %i239, label %bb243, label %bb240

bb240:                                            ; preds = %bb234
  %i241 = add nuw i32 %i235, 1
  %i242 = icmp eq i32 %i241, %i232
  br i1 %i242, label %bb245, label %bb234, !llvm.loop !1574

bb243:                                            ; preds = %bb234
  %i244 = icmp eq i32 %i235, -1
  br i1 %i244, label %bb245, label %bb254

bb245:                                            ; preds = %bb243, %bb240, %bb230, %bb225
  %i246 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !1496
  %i247 = load i8, ptr %i246, align 8, !tbaa !1496, !range !1451, !noundef !1452
  %i248 = icmp eq i8 %i247, 0
  br i1 %i248, label %bb478, label %bb249

bb249:                                            ; preds = %bb245
  %i250 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1572
  %i251 = load ptr, ptr %i250, align 8, !tbaa !1572
  %i252 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i251, i64 0, i32 50, !intel-tbaa !1500
  %i253 = load ptr, ptr %i252, align 8, !tbaa !1500
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr noundef nonnull align 8 dereferenceable(40) %i253, i32 noundef 98)
  br label %bb478

bb254:                                            ; preds = %bb243
  %i255 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !1575
  %i256 = load ptr, ptr %i255, align 8, !tbaa !1575
  %i257 = icmp eq ptr %i256, null
  br i1 %i257, label %bb276, label %bb258

bb258:                                            ; preds = %bb254
  %i259 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 1, !intel-tbaa !1576
  %i260 = load i32, ptr %i259, align 4, !tbaa !1576
  %i261 = icmp ugt i32 %i260, %i235
  br i1 %i261, label %bb269, label %bb262

bb262:                                            ; preds = %bb258
  %i263 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i264 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 4, !intel-tbaa !1579
  %i265 = load ptr, ptr %i264, align 8, !tbaa !1579
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i263, ptr noundef nonnull @.str.6.5757, i32 noundef 206, i32 noundef 116, ptr noundef %i265)
          to label %bb266 unwind label %bb267

bb266:                                            ; preds = %bb262
  tail call void @__cxa_throw(ptr nonnull %i263, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb267:                                            ; preds = %bb262
  %i268 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i263) #47
  br label %bb207

bb269:                                            ; preds = %bb258
  %i270 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i256, i64 0, i32 3, !intel-tbaa !1580
  %i271 = load ptr, ptr %i270, align 8, !tbaa !1580
  %i272 = zext i32 %i235 to i64
  %i273 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i271, i64 %i272, i32 1
  %i274 = load ptr, ptr %i273, align 8, !tbaa !1581
  %i275 = icmp eq ptr %i274, null
  br i1 %i275, label %bb276, label %bb302

bb276:                                            ; preds = %bb269, %bb254
  %i277 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !1436
  %i278 = load ptr, ptr %i277, align 8, !tbaa !1436
  %i279 = icmp eq ptr %i278, null
  br i1 %i279, label %bb298, label %bb280

bb280:                                            ; preds = %bb276
  %i281 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 1, !intel-tbaa !1438
  %i282 = load i32, ptr %i281, align 4, !tbaa !1438
  %i283 = icmp ugt i32 %i282, %i235
  br i1 %i283, label %bb291, label %bb284

bb284:                                            ; preds = %bb280
  %i285 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i286 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 4, !intel-tbaa !1441
  %i287 = load ptr, ptr %i286, align 8, !tbaa !1441
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i285, ptr noundef nonnull @.str.4453, i32 noundef 249, i32 noundef 116, ptr noundef %i287)
          to label %bb288 unwind label %bb289

bb288:                                            ; preds = %bb284
  tail call void @__cxa_throw(ptr nonnull %i285, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb289:                                            ; preds = %bb284
  %i290 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i285) #47
  br label %bb207

bb291:                                            ; preds = %bb280
  %i292 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i278, i64 0, i32 3, !intel-tbaa !1442
  %i293 = load ptr, ptr %i292, align 8, !tbaa !1442
  %i294 = zext i32 %i235 to i64
  %i295 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i293, i64 %i294, i32 2
  %i296 = load ptr, ptr %i295, align 8, !tbaa !1443
  %i297 = icmp eq ptr %i296, null
  br i1 %i297, label %bb298, label %bb302

bb298:                                            ; preds = %bb291, %bb276
  %i299 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 1, !intel-tbaa !1583
  %i300 = load i32, ptr %i299, align 4, !tbaa !1583
  %i301 = add nsw i32 %i300, 1
  store i32 %i301, ptr %i299, align 4, !tbaa !1583
  br label %bb302

bb302:                                            ; preds = %bb298, %bb291, %bb269
  tail call void @_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt(ptr noundef nonnull align 8 dereferenceable(32) %i226, ptr noundef %i32, ptr noundef %arg2, ptr noundef %arg1)
  %i303 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 1, !intel-tbaa !1583
  %i304 = load i32, ptr %i303, align 4, !tbaa !1583
  %i305 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i226, i64 0, i32 0, !intel-tbaa !1431
  %i306 = load ptr, ptr %i305, align 8, !tbaa !1431
  %i307 = icmp eq ptr %i306, null
  br i1 %i307, label %bb311, label %bb308

bb308:                                            ; preds = %bb302
  %i309 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i306, i64 0, i32 1, !intel-tbaa !1453
  %i310 = load i32, ptr %i309, align 4, !tbaa !1453
  br label %bb311

bb311:                                            ; preds = %bb308, %bb302
  %i312 = phi i32 [ %i310, %bb308 ], [ 0, %bb302 ]
  %i313 = icmp eq i32 %i304, %i312
  br i1 %i313, label %bb314, label %bb478

bb314:                                            ; preds = %bb311
  %i315 = tail call fastcc noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE(ptr noundef nonnull align 8 dereferenceable(80) %i28, ptr noundef nonnull %i226)
  br i1 %i315, label %bb316, label %bb347

bb316:                                            ; preds = %bb314
  %i317 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 0, !intel-tbaa !1496
  %i318 = load i8, ptr %i317, align 8, !tbaa !1496, !range !1451, !noundef !1452
  %i319 = icmp eq i8 %i318, 0
  br i1 %i319, label %bb347, label %bb320

bb320:                                            ; preds = %bb316
  %i321 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 2, !intel-tbaa !1584
  %i322 = load ptr, ptr %i321, align 8, !tbaa !1584
  %i323 = getelementptr %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i322, i64 0, i32 0, i32 0
  %i324 = load ptr, ptr %i323, align 8, !tbaa !1256
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
  %i332 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1572
  %i333 = load ptr, ptr %i332, align 8, !tbaa !1572
  %i334 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i333, i64 0, i32 50, !intel-tbaa !1500
  %i335 = load ptr, ptr %i334, align 8, !tbaa !1500
  %i336 = load ptr, ptr %i321, align 8, !tbaa !1584
  %i337 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i336, i64 0, i32 2, !intel-tbaa !1537
  %i338 = load ptr, ptr %i337, align 8, !tbaa !1537
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i335, i32 noundef 104, ptr noundef %i338, ptr noundef null, ptr noundef null)
  br label %bb347

bb339:                                            ; preds = %bb320
  %i340 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 6, !intel-tbaa !1572
  %i341 = load ptr, ptr %i340, align 8, !tbaa !1572
  %i342 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i341, i64 0, i32 50, !intel-tbaa !1500
  %i343 = load ptr, ptr %i342, align 8, !tbaa !1500
  %i344 = load ptr, ptr %i321, align 8, !tbaa !1584
  %i345 = getelementptr inbounds %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint", ptr %i344, i64 0, i32 2, !intel-tbaa !1537
  %i346 = load ptr, ptr %i345, align 8, !tbaa !1537
  tail call fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr noundef nonnull align 8 dereferenceable(40) %i343, i32 noundef 105, ptr noundef %i346, ptr noundef null, ptr noundef null)
  br label %bb347

bb347:                                            ; preds = %bb339, %bb331, %bb329, %bb316, %bb314
  %i348 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 4, !intel-tbaa !1585
  %i349 = load ptr, ptr %i348, align 8, !tbaa !1585
  %i350 = icmp eq ptr %i349, null
  br i1 %i350, label %bb351, label %bb388

bb351:                                            ; preds = %bb347
  %i352 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 7, !intel-tbaa !1586
  %i353 = load ptr, ptr %i352, align 8, !tbaa !1586
  %i354 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 40, ptr noundef %i353)
  %i355 = load ptr, ptr %i352, align 8, !tbaa !1586
  store ptr getelementptr inbounds ([9 x ptr], ptr @_ZTVN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.0, i64 0, i64 2), ptr %i354, align 8, !tbaa !1256
  %i356 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 1
  store i8 1, ptr %i356, align 1, !tbaa !1587
  %i357 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 2
  store i32 0, ptr %i357, align 4, !tbaa !1590
  %i358 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 3
  store i32 4, ptr %i358, align 4, !tbaa !1591
  %i359 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 4
  store ptr null, ptr %i359, align 8, !tbaa !1592
  %i360 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i354, i64 0, i32 0, i32 5
  store ptr %i355, ptr %i360, align 8, !tbaa !1593
  %i361 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i355, i64 0, i32 0
  %i362 = load ptr, ptr %i361, align 8, !tbaa !1256
  %i363 = tail call i1 @llvm.type.test(ptr %i362, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i363)
  %i364 = getelementptr inbounds ptr, ptr %i362, i64 2
  %i365 = load ptr, ptr %i364, align 8
  %i366 = icmp eq ptr %i365, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i366, label %bb367, label %bb381

bb367:                                            ; preds = %bb351
  %i368 = invoke noalias noundef nonnull dereferenceable(32) ptr @_Znwm(i64 noundef 32) #50
          to label %bb380 unwind label %bb369

bb369:                                            ; preds = %bb367
  %i370 = landingpad { ptr, i32 }
          catch ptr null
  %i371 = extractvalue { ptr, i32 } %i370, 0
  %i372 = tail call ptr @__cxa_begin_catch(ptr %i371) #47
  %i373 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i373, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i378) #48
  unreachable

bb379:                                            ; preds = %bb369
  unreachable

bb380:                                            ; preds = %bb367
  store ptr %i368, ptr %i359, align 8, !tbaa !1592
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(32) %i368, i8 0, i64 32, i1 false), !tbaa !1594
  store ptr getelementptr inbounds ([9 x ptr], ptr @_ZTVN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.0, i64 0, i64 2), ptr %i354, align 8, !tbaa !1256
  store ptr %i354, ptr %i348, align 8, !tbaa !1585
  br label %bb388

bb381:                                            ; preds = %bb351
  %i382 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb383 unwind label %bb384, !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273

bb383:                                            ; preds = %bb381
  unreachable

bb384:                                            ; preds = %bb381
  %i385 = landingpad { ptr, i32 }
          cleanup
  br label %bb386

bb386:                                            ; preds = %bb384, %bb374
  %i387 = phi { ptr, i32 } [ %i385, %bb384 ], [ %i375, %bb374 ]
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef nonnull %i354) #47
  br label %bb207

bb388:                                            ; preds = %bb380, %bb347
  %i389 = phi ptr [ %i354, %bb380 ], [ %i349, %bb347 ]
  %i390 = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %i28, i64 0, i32 7, !intel-tbaa !1586
  %i391 = load ptr, ptr %i390, align 8, !tbaa !1586
  %i392 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 16, ptr noundef %i391)
  invoke void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(32) %i392, ptr noundef nonnull align 8 dereferenceable(32) %i226)
          to label %bb393 unwind label %bb476

bb393:                                            ; preds = %bb388
  %i394 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 2
  %i395 = load i32, ptr %i394, align 4, !tbaa !1590
  %i396 = add i32 %i395, 1
  %i397 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 3
  %i398 = load i32, ptr %i397, align 4, !tbaa !1591
  %i399 = icmp ugt i32 %i396, %i398
  br i1 %i399, label %bb403, label %bb400

bb400:                                            ; preds = %bb393
  %i401 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 4
  %i402 = load ptr, ptr %i401, align 8, !tbaa !1592
  br label %bb470

bb403:                                            ; preds = %bb393
  %i404 = lshr i32 %i398, 1
  %i405 = add i32 %i404, %i398
  %i406 = icmp ult i32 %i396, %i405
  %i407 = select i1 %i406, i32 %i405, i32 %i396
  %i408 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 5
  %i409 = load ptr, ptr %i408, align 8, !tbaa !1593
  %i410 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i409, i64 0, i32 0
  %i411 = load ptr, ptr %i410, align 8, !tbaa !1256
  %i412 = tail call i1 @llvm.type.test(ptr %i411, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i412)
  %i413 = getelementptr inbounds ptr, ptr %i411, i64 2
  %i414 = load ptr, ptr %i413, align 8
  %i415 = icmp eq ptr %i414, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i415, label %bb416, label %bb436

bb416:                                            ; preds = %bb403
  %i417 = zext i32 %i407 to i64
  %i418 = shl nuw nsw i64 %i417, 3
  %i419 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i418) #50
          to label %bb431 unwind label %bb420

bb420:                                            ; preds = %bb416
  %i421 = landingpad { ptr, i32 }
          catch ptr null
  %i422 = extractvalue { ptr, i32 } %i421, 0
  %i423 = tail call ptr @__cxa_begin_catch(ptr %i422) #47
  %i424 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i424, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i429) #48
  unreachable

bb430:                                            ; preds = %bb420
  unreachable

bb431:                                            ; preds = %bb416
  %i432 = load i32, ptr %i394, align 4, !tbaa !1590
  %i433 = icmp eq i32 %i432, 0
  %i434 = getelementptr inbounds %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf", ptr %i389, i64 0, i32 0, i32 4
  %i435 = load ptr, ptr %i434, align 8, !tbaa !1592
  br i1 %i433, label %bb440, label %bb438

bb436:                                            ; preds = %bb403
  %i437 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
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
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %i445, i8 0, i64 %i450, i1 false), !tbaa !1594
  br label %bb458

bb451:                                            ; preds = %bb451, %bb438
  %i452 = phi i64 [ 0, %bb438 ], [ %i456, %bb451 ]
  %i453 = getelementptr inbounds ptr, ptr %i435, i64 %i452
  %i454 = load ptr, ptr %i453, align 8, !tbaa !1594
  %i455 = getelementptr inbounds ptr, ptr %i419, i64 %i452
  store ptr %i454, ptr %i455, align 8, !tbaa !1594
  %i456 = add nuw nsw i64 %i452, 1
  %i457 = icmp eq i64 %i456, %i439
  br i1 %i457, label %bb440, label %bb451, !llvm.loop !1596

bb458:                                            ; preds = %bb442, %bb440
  %i459 = load ptr, ptr %i408, align 8, !tbaa !1593
  %i460 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i459, i64 0, i32 0
  %i461 = load ptr, ptr %i460, align 8, !tbaa !1256
  %i462 = tail call i1 @llvm.type.test(ptr %i461, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i462)
  %i463 = getelementptr inbounds ptr, ptr %i461, i64 3
  %i464 = load ptr, ptr %i463, align 8
  %i465 = icmp eq ptr %i464, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i465, label %bb466, label %bb469

bb466:                                            ; preds = %bb458
  tail call void @_ZdlPv(ptr noundef %i435) #47
  store ptr %i419, ptr %i434, align 8, !tbaa !1592
  store i32 %i407, ptr %i397, align 4, !tbaa !1591
  %i467 = load i32, ptr %i394, align 4, !tbaa !1590
  %i468 = add i32 %i467, 1
  br label %bb470

bb469:                                            ; preds = %bb458
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273
  unreachable

bb470:                                            ; preds = %bb466, %bb400
  %i471 = phi i32 [ %i396, %bb400 ], [ %i468, %bb466 ]
  %i472 = phi i32 [ %i395, %bb400 ], [ %i467, %bb466 ]
  %i473 = phi ptr [ %i402, %bb400 ], [ %i419, %bb466 ]
  %i474 = zext i32 %i472 to i64
  %i475 = getelementptr inbounds ptr, ptr %i473, i64 %i474
  store ptr %i392, ptr %i475, align 8, !tbaa !1594
  store i32 %i471, ptr %i394, align 4, !tbaa !1590
  br label %bb478

bb476:                                            ; preds = %bb388
  %i477 = landingpad { ptr, i32 }
          cleanup
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i392) #47
  br label %bb207

bb478:                                            ; preds = %bb470, %bb311, %bb249, %bb245
  %i479 = load ptr, ptr %i29, align 8, !tbaa !1541
  %i480 = load ptr, ptr %i31, align 8, !tbaa !1492
  %i481 = getelementptr inbounds %"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator", ptr %i479, i64 0, i32 2, !intel-tbaa !1542
  %i482 = load ptr, ptr %i481, align 8, !tbaa !1542
  %i483 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 3, !intel-tbaa !1547
  %i484 = load ptr, ptr %i483, align 8, !tbaa !1547
  %i485 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 2, !intel-tbaa !1551
  %i486 = load i32, ptr %i485, align 4, !tbaa !1551
  %i487 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 0, !intel-tbaa !1597
  %i488 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i484, i64 0, i32 0
  %i489 = load ptr, ptr %i488, align 8, !tbaa !1256
  %i490 = tail call i1 @llvm.type.test(ptr %i489, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i490)
  %i491 = load ptr, ptr %i489, align 8
  %i492 = icmp eq ptr %i491, @_ZN11xercesc_2_714HashCMStateSet10getHashValEPKvjPNS_13MemoryManagerE
  br i1 %i492, label %bb493, label %bb526

bb493:                                            ; preds = %bb478
  %i494 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 0, !intel-tbaa !1552
  %i495 = load i32, ptr %i494, align 4, !tbaa !1552
  %i496 = icmp ult i32 %i495, 65
  br i1 %i496, label %bb497, label %bb504

bb497:                                            ; preds = %bb493
  %i498 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 2, !intel-tbaa !1555
  %i499 = load i32, ptr %i498, align 4, !tbaa !1555
  %i500 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 3, !intel-tbaa !1556
  %i501 = load i32, ptr %i500, align 4, !tbaa !1556
  %i502 = mul i32 %i501, 31
  %i503 = add i32 %i502, %i499
  br label %bb523

bb504:                                            ; preds = %bb493
  %i505 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 1, !intel-tbaa !1557
  %i506 = load i32, ptr %i505, align 4, !tbaa !1557
  %i507 = add i32 %i506, -1
  %i508 = icmp sgt i32 %i507, -1
  br i1 %i508, label %bb509, label %bb523

bb509:                                            ; preds = %bb504
  %i510 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 4, !intel-tbaa !1558
  %i511 = load ptr, ptr %i510, align 8, !tbaa !1558
  %i512 = zext i32 %i507 to i64
  br label %bb513

bb513:                                            ; preds = %bb513, %bb509
  %i514 = phi i64 [ %i522, %bb513 ], [ %i512, %bb509 ]
  %i515 = phi i32 [ %i520, %bb513 ], [ 0, %bb509 ]
  %i516 = getelementptr inbounds i8, ptr %i511, i64 %i514
  %i517 = load i8, ptr %i516, align 1, !tbaa !1461
  %i518 = zext i8 %i517 to i32
  %i519 = mul nsw i32 %i515, 31
  %i520 = add nsw i32 %i519, %i518
  %i521 = icmp eq i64 %i514, 0
  %i522 = add nsw i64 %i514, -1
  br i1 %i521, label %bb523, label %bb513, !llvm.loop !1559

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
  %i536 = load i16, ptr %i480, align 2, !tbaa !1286
  %i537 = icmp eq i16 %i536, 0
  br i1 %i537, label %bb558, label %bb538

bb538:                                            ; preds = %bb535
  %i539 = zext i16 %i536 to i32
  %i540 = getelementptr inbounds i16, ptr %i480, i64 1
  %i541 = load i16, ptr %i540, align 2, !tbaa !1286
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
  %i553 = load i16, ptr %i552, align 2, !tbaa !1286
  %i554 = icmp eq i16 %i553, 0
  br i1 %i554, label %bb555, label %bb543, !llvm.loop !1560

bb555:                                            ; preds = %bb543, %bb538
  %i556 = phi i32 [ %i539, %bb538 ], [ %i551, %bb543 ]
  %i557 = urem i32 %i556, %i486
  br label %bb558

bb558:                                            ; preds = %bb555, %bb535, %bb533, %bb528, %bb523
  %i559 = phi i32 [ %i525, %bb523 ], [ %i532, %bb528 ], [ %i557, %bb555 ], [ 0, %bb535 ], [ 0, %bb533 ]
  %i560 = getelementptr inbounds %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf", ptr %i482, i64 0, i32 1, !intel-tbaa !1561
  %i561 = load ptr, ptr %i560, align 8, !tbaa !1561
  %i562 = zext i32 %i559 to i64
  %i563 = getelementptr inbounds ptr, ptr %i561, i64 %i562
  %i564 = load ptr, ptr %i563, align 8, !tbaa !1562
  %i565 = icmp eq ptr %i564, null
  br i1 %i565, label %bb656, label %bb566

bb566:                                            ; preds = %bb648, %bb558
  %i567 = phi ptr [ %i650, %bb648 ], [ %i564, %bb558 ]
  %i568 = load ptr, ptr %i483, align 8, !tbaa !1547
  %i569 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 2
  %i570 = load ptr, ptr %i569, align 8, !tbaa !1564
  %i571 = getelementptr %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase", ptr %i568, i64 0, i32 0
  %i572 = load ptr, ptr %i571, align 8, !tbaa !1256
  %i573 = tail call i1 @llvm.type.test(ptr %i572, metadata !"_ZTSN11xercesc_2_78HashBaseE")
  tail call void @llvm.assume(i1 %i573)
  %i574 = getelementptr inbounds ptr, ptr %i572, i64 1
  %i575 = load ptr, ptr %i574, align 8
  %i576 = icmp eq ptr %i575, @_ZN11xercesc_2_714HashCMStateSet6equalsEPKvS2_
  br i1 %i576, label %bb577, label %bb605

bb577:                                            ; preds = %bb566
  %i578 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 0, !intel-tbaa !1552
  %i579 = load i32, ptr %i578, align 4, !tbaa !1552
  %i580 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 0, !intel-tbaa !1552
  %i581 = load i32, ptr %i580, align 4, !tbaa !1552
  %i582 = icmp eq i32 %i579, %i581
  br i1 %i582, label %bb583, label %bb648

bb583:                                            ; preds = %bb577
  %i584 = icmp ult i32 %i579, 65
  br i1 %i584, label %bb636, label %bb585

bb585:                                            ; preds = %bb583
  %i586 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 1, !intel-tbaa !1557
  %i587 = load i32, ptr %i586, align 4
  %i588 = icmp eq i32 %i587, 0
  br i1 %i588, label %bb652, label %bb589

bb589:                                            ; preds = %bb585
  %i590 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 4
  %i591 = load ptr, ptr %i590, align 8, !tbaa !1558
  %i592 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 4, !intel-tbaa !1558
  %i593 = load ptr, ptr %i592, align 8, !tbaa !1558
  %i594 = zext i32 %i587 to i64
  br label %bb598

bb595:                                            ; preds = %bb598
  %i596 = add nuw nsw i64 %i599, 1
  %i597 = icmp eq i64 %i596, %i594
  br i1 %i597, label %bb652, label %bb598, !llvm.loop !1567

bb598:                                            ; preds = %bb595, %bb589
  %i599 = phi i64 [ 0, %bb589 ], [ %i596, %bb595 ]
  %i600 = getelementptr inbounds i8, ptr %i591, i64 %i599
  %i601 = load i8, ptr %i600, align 1, !tbaa !1461
  %i602 = getelementptr inbounds i8, ptr %i593, i64 %i599
  %i603 = load i8, ptr %i602, align 1, !tbaa !1461
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
  %i614 = load i16, ptr %i480, align 2, !tbaa !1286
  %i615 = load i16, ptr %i570, align 2, !tbaa !1286
  %i616 = icmp eq i16 %i614, %i615
  br i1 %i616, label %bb625, label %bb648

bb617:                                            ; preds = %bb609
  br i1 %i610, label %bb621, label %bb618

bb618:                                            ; preds = %bb617
  %i619 = load i16, ptr %i480, align 2, !tbaa !1286
  %i620 = icmp eq i16 %i619, 0
  br i1 %i620, label %bb621, label %bb648

bb621:                                            ; preds = %bb618, %bb617
  br i1 %i611, label %bb652, label %bb622

bb622:                                            ; preds = %bb621
  %i623 = load i16, ptr %i570, align 2, !tbaa !1286
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
  %i633 = load i16, ptr %i631, align 2, !tbaa !1286
  %i634 = load i16, ptr %i632, align 2, !tbaa !1286
  %i635 = icmp eq i16 %i633, %i634
  br i1 %i635, label %bb625, label %bb648, !llvm.loop !1568

bb636:                                            ; preds = %bb583
  %i637 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 2, !intel-tbaa !1555
  %i638 = load i32, ptr %i637, align 4, !tbaa !1555
  %i639 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 2, !intel-tbaa !1555
  %i640 = load i32, ptr %i639, align 4, !tbaa !1555
  %i641 = icmp eq i32 %i638, %i640
  %i642 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i480, i64 0, i32 3
  %i643 = load i32, ptr %i642, align 4
  %i644 = getelementptr inbounds %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet", ptr %i570, i64 0, i32 3
  %i645 = load i32, ptr %i644, align 4
  %i646 = icmp eq i32 %i643, %i645
  %i647 = select i1 %i641, i1 %i646, i1 false
  br i1 %i647, label %bb652, label %bb648

bb648:                                            ; preds = %bb636, %bb630, %bb622, %bb618, %bb613, %bb607, %bb598, %bb577
  %i649 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 1, !intel-tbaa !1569
  %i650 = load ptr, ptr %i649, align 8, !tbaa !1562
  %i651 = icmp eq ptr %i650, null
  br i1 %i651, label %bb656, label %bb566, !llvm.loop !1598

bb652:                                            ; preds = %bb636, %bb625, %bb622, %bb621, %bb607, %bb595, %bb585
  %i653 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 2
  %i654 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i567, i64 0, i32 0, !intel-tbaa !1571
  %i655 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i654, i64 0, i32 0
  store i8 0, ptr %i655, align 1, !tbaa !1571
  store ptr %i480, ptr %i653, align 8, !tbaa !1564
  br label %bb687

bb656:                                            ; preds = %bb648, %bb558
  %i657 = load ptr, ptr %i487, align 8, !tbaa !1597
  %i658 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i657, i64 0, i32 0
  %i659 = load ptr, ptr %i658, align 8, !tbaa !1256
  %i660 = tail call i1 @llvm.type.test(ptr %i659, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i660)
  %i661 = getelementptr inbounds ptr, ptr %i659, i64 2
  %i662 = load ptr, ptr %i661, align 8
  %i663 = icmp eq ptr %i662, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i663, label %bb664, label %bb677

bb664:                                            ; preds = %bb656
  %i665 = invoke noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #50
          to label %bb679 unwind label %bb666

bb666:                                            ; preds = %bb664
  %i667 = landingpad { ptr, i32 }
          catch ptr null
  %i668 = extractvalue { ptr, i32 } %i667, 0
  %i669 = tail call ptr @__cxa_begin_catch(ptr %i668) #47
  %i670 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i670, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i675) #48
  unreachable

bb676:                                            ; preds = %bb666
  unreachable

bb677:                                            ; preds = %bb656
  %i678 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb679:                                            ; preds = %bb664
  %i680 = load ptr, ptr %i560, align 8, !tbaa !1561
  %i681 = getelementptr inbounds ptr, ptr %i680, i64 %i562
  %i682 = load ptr, ptr %i681, align 8, !tbaa !1562
  %i683 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 0, !intel-tbaa !1571
  %i684 = getelementptr %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i683, i64 0, i32 0
  store i8 0, ptr %i684, align 1, !tbaa !1571
  %i685 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 1, !intel-tbaa !1569
  store ptr %i682, ptr %i685, align 8, !tbaa !1569
  %i686 = getelementptr inbounds %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem", ptr %i665, i64 0, i32 2, !intel-tbaa !1564
  store ptr %i480, ptr %i686, align 8, !tbaa !1564
  store ptr %i665, ptr %i681, align 8, !tbaa !1562
  br label %bb687

bb687:                                            ; preds = %bb679, %bb652
  ret void
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1599 hidden noundef signext i16 @_ZNK11xercesc_2_76IC_Key7getTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #15 align 2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1601 hidden noundef signext i16 @_ZNK11xercesc_2_79IC_KeyRef7getTypeEv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1") unnamed_addr #15 align 2

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare !intel.dtrans.func.type !1603 dso_local i64 @strlen(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #33

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define hidden noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef writeonly "intel_dtrans_func_index"="2" %arg2, i32 noundef %arg3) unnamed_addr #34 align 2 !intel.dtrans.func.type !1604 {
bb:
  %i = zext i32 %arg3 to i64
  %i4 = getelementptr inbounds i16, ptr %arg2, i64 %i, !intel-tbaa !1286
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %arg, i64 0, i32 1, !intel-tbaa !1606
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1606
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb89, label %bb8

bb8:                                              ; preds = %bb
  %i9 = load i16, ptr %i6, align 2, !tbaa !1286
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
  %i19 = load i16, ptr %i17, align 2, !tbaa !1286
  %i20 = getelementptr [41 x i16], ptr %i18, i64 0, i64 0
  %i21 = load i16, ptr %i20, align 2, !tbaa !1286
  %i22 = icmp eq i16 %i19, %i21
  br i1 %i22, label %bb11, label %bb29, !llvm.loop !1609

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
  %i39 = load i16, ptr %i37, align 2, !tbaa !1286
  %i40 = getelementptr [43 x i16], ptr %i38, i64 0, i64 0
  %i41 = load i16, ptr %i40, align 2, !tbaa !1286
  %i42 = icmp eq i16 %i39, %i41
  br i1 %i42, label %bb31, label %bb49, !llvm.loop !1609

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
  %i59 = load i16, ptr %i57, align 2, !tbaa !1286
  %i60 = getelementptr [43 x i16], ptr %i58, i64 0, i64 0
  %i61 = load i16, ptr %i60, align 2, !tbaa !1286
  %i62 = icmp eq i16 %i59, %i61
  br i1 %i62, label %bb51, label %bb69, !llvm.loop !1609

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
  %i79 = load i16, ptr %i77, align 2, !tbaa !1286
  %i80 = getelementptr [41 x i16], ptr %i78, i64 0, i64 0
  %i81 = load i16, ptr %i80, align 2, !tbaa !1286
  %i82 = icmp eq i16 %i79, %i81
  br i1 %i82, label %bb71, label %bb89, !llvm.loop !1609

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
  %i91 = load i16, ptr %i90, align 2, !tbaa !1286
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
  store i16 %i96, ptr %i98, align 2, !tbaa !1286
  %i101 = load i16, ptr %i99, align 2, !tbaa !1286
  %i102 = icmp ne i16 %i101, 0
  %i103 = icmp ult ptr %i100, %i4
  %i104 = select i1 %i102, i1 %i103, i1 false
  br i1 %i104, label %bb95, label %bb105, !llvm.loop !1610

bb105:                                            ; preds = %bb95, %bb89
  %i106 = phi ptr [ %arg2, %bb89 ], [ %i100, %bb95 ]
  store i16 0, ptr %i106, align 2, !tbaa !1286
  br label %bb107

bb107:                                            ; preds = %bb105, %bb83, %bb63, %bb43, %bb23
  %i108 = phi i1 [ true, %bb105 ], [ false, %bb23 ], [ false, %bb43 ], [ false, %bb63 ], [ false, %bb83 ]
  ret i1 %i108
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !1611 hidden void @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev(ptr nocapture nonnull align 1 "intel_dtrans_func_index"="1") unnamed_addr #35 align 2

; Function Attrs: uwtable
declare !intel.dtrans.func.type !1613 hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2", i64 noundef) unnamed_addr #14 align 2

; Function Attrs: mustprogress norecurse nounwind uwtable
declare !intel.dtrans.func.type !1615 hidden void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") unnamed_addr #36 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define hidden fastcc noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg) unnamed_addr #13 align 2 !intel.dtrans.func.type !1616 {
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
define hidden fastcc noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef "intel_dtrans_func_index"="2" %arg) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1617 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i1 = invoke fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 16, ptr noundef %i)
          to label %bb2 unwind label %bb107

bb2:                                              ; preds = %bb
  %i3 = getelementptr %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %i1, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([8 x ptr], ptr @_ZTVN11xercesc_2_714InMemMsgLoaderE.0, i64 0, i64 2), ptr %i3, align 8, !tbaa !1256
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %i1, i64 0, i32 1, !intel-tbaa !1606
  store ptr null, ptr %i4, align 8, !tbaa !1606
  %i5 = icmp eq ptr %arg, null
  br i1 %i5, label %bb63, label %bb6

bb6:                                              ; preds = %bb2
  %i7 = load i16, ptr %arg, align 2, !tbaa !1286
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
  %i17 = load i16, ptr %i15, align 2, !tbaa !1286
  %i18 = getelementptr [41 x i16], ptr %i16, i64 0, i64 0
  %i19 = load i16, ptr %i18, align 2, !tbaa !1286
  %i20 = icmp eq i16 %i17, %i19
  br i1 %i20, label %bb9, label %bb21, !llvm.loop !1609

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
  %i31 = load i16, ptr %i29, align 2, !tbaa !1286
  %i32 = getelementptr [43 x i16], ptr %i30, i64 0, i64 0
  %i33 = load i16, ptr %i32, align 2, !tbaa !1286
  %i34 = icmp eq i16 %i31, %i33
  br i1 %i34, label %bb23, label %bb35, !llvm.loop !1609

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
  %i45 = load i16, ptr %i43, align 2, !tbaa !1286
  %i46 = getelementptr [41 x i16], ptr %i44, i64 0, i64 0
  %i47 = load i16, ptr %i46, align 2, !tbaa !1286
  %i48 = icmp eq i16 %i45, %i47
  br i1 %i48, label %bb37, label %bb49, !llvm.loop !1609

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
  %i59 = load i16, ptr %i57, align 2, !tbaa !1286
  %i60 = getelementptr [43 x i16], ptr %i58, i64 0, i64 0
  %i61 = load i16, ptr %i60, align 2, !tbaa !1286
  %i62 = icmp eq i16 %i59, %i61
  br i1 %i62, label %bb51, label %bb63, !llvm.loop !1609

bb63:                                             ; preds = %bb56, %bb49, %bb35, %bb21, %bb6, %bb2
  %i64 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1618
  %i65 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i64, i64 0, i32 0
  %i66 = load ptr, ptr %i65, align 8, !tbaa !1256
  %i67 = tail call i1 @llvm.type.test(ptr %i66, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i67)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 3), !intel_dtrans_type !1620, !_Intel.Devirt.Call !1273
  unreachable

bb68:                                             ; preds = %bb51, %bb37, %bb23, %bb9
  %i69 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i70 = load i16, ptr %arg, align 2, !tbaa !1286
  %i71 = icmp eq i16 %i70, 0
  br i1 %i71, label %bb83, label %bb72

bb72:                                             ; preds = %bb72, %bb68
  %i73 = phi ptr [ %i74, %bb72 ], [ %arg, %bb68 ]
  %i74 = getelementptr inbounds i16, ptr %i73, i64 1
  %i75 = load i16, ptr %i74, align 2, !tbaa !1286
  %i76 = icmp eq i16 %i75, 0
  br i1 %i76, label %bb77, label %bb72, !llvm.loop !1621

bb77:                                             ; preds = %bb72
  %i78 = ptrtoint ptr %i74 to i64
  %i79 = ptrtoint ptr %arg to i64
  %i80 = sub i64 %i78, %i79
  %i81 = add i64 %i80, 2
  %i82 = and i64 %i81, 8589934590
  br label %bb83

bb83:                                             ; preds = %bb77, %bb68
  %i84 = phi i64 [ %i82, %bb77 ], [ 2, %bb68 ]
  %i85 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i69, i64 0, i32 0
  %i86 = load ptr, ptr %i85, align 8, !tbaa !1256
  %i87 = tail call i1 @llvm.type.test(ptr %i86, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i87)
  %i88 = getelementptr inbounds ptr, ptr %i86, i64 2
  %i89 = load ptr, ptr %i88, align 8
  %i90 = icmp eq ptr %i89, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i90, label %bb91, label %bb104

bb91:                                             ; preds = %bb83
  %i92 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i84) #50
          to label %bb148 unwind label %bb93

bb93:                                             ; preds = %bb91
  %i94 = landingpad { ptr, i32 }
          catch ptr null
  %i95 = extractvalue { ptr, i32 } %i94, 0
  %i96 = tail call ptr @__cxa_begin_catch(ptr %i95) #47
  %i97 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i97, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i102) #48
  unreachable

bb103:                                            ; preds = %bb93
  unreachable

bb104:                                            ; preds = %bb83
  %i105 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb106 unwind label %bb109, !intel_dtrans_type !1272

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
  %i115 = getelementptr inbounds i8, ptr %i1, i64 -8, !intel-tbaa !1461
  %i116 = load ptr, ptr %i115, align 8, !tbaa !1271
  %i117 = load ptr, ptr %i116, align 8, !tbaa !1256
  %i118 = tail call i1 @llvm.type.test(ptr %i117, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i118)
  %i119 = getelementptr inbounds ptr, ptr %i117, i64 3
  %i120 = load ptr, ptr %i119, align 8
  %i121 = icmp eq ptr %i120, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i121, label %bb122, label %bb123

bb122:                                            ; preds = %bb114
  tail call void @_ZdlPv(ptr noundef nonnull %i115) #47
  br label %bb128

bb123:                                            ; preds = %bb114
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb124 unwind label %bb125, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb124:                                            ; preds = %bb123
  unreachable

bb125:                                            ; preds = %bb123
  %i126 = landingpad { ptr, i32 }
          catch ptr null
  %i127 = extractvalue { ptr, i32 } %i126, 0
  tail call fastcc void @__clang_call_terminate(ptr %i127) #48
  unreachable

bb128:                                            ; preds = %bb122, %bb111, %bb107
  %i129 = phi { ptr, i32 } [ %i108, %bb107 ], [ %i112, %bb111 ], [ %i112, %bb122 ]
  %i130 = extractvalue { ptr, i32 } %i129, 1
  %i131 = extractvalue { ptr, i32 } %i129, 0
  %i132 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #47
  %i133 = icmp eq i32 %i130, %i132
  %i134 = tail call ptr @__cxa_begin_catch(ptr %i131) #47
  br i1 %i133, label %bb135, label %bb136

bb135:                                            ; preds = %bb128
  invoke void @__cxa_rethrow() #51
          to label %bb147 unwind label %bb141

bb136:                                            ; preds = %bb128
  %i137 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1618
  %i138 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i137, i64 0, i32 0
  %i139 = load ptr, ptr %i138, align 8, !tbaa !1256
  %i140 = tail call i1 @llvm.type.test(ptr %i139, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i140)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1620, !_Intel.Devirt.Call !1273
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
  tail call fastcc void @__clang_call_terminate(ptr %i146) #48
  unreachable

bb147:                                            ; preds = %bb135
  unreachable

bb148:                                            ; preds = %bb91
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i92, ptr nonnull align 2 %arg, i64 %i84, i1 false)
  store ptr %i92, ptr %i4, align 8, !tbaa !1606
  ret ptr %i1
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="2" %arg, ptr nocapture noundef nonnull writeonly align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %arg1) unnamed_addr #16 align 2 !intel.dtrans.func.type !1622 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 2, !intel-tbaa !1623
  %i2 = load ptr, ptr %i, align 8, !tbaa !1623
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 1, !intel-tbaa !1624
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1624
  %i5 = icmp eq ptr %i4, null
  br i1 %i5, label %bb51, label %bb6

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i4, i64 0, i32 6, !intel-tbaa !1625
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1625
  %i9 = icmp ne ptr %i8, null
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i4, i64 0, i32 7
  %i11 = load ptr, ptr %i10, align 8
  %i12 = icmp ne ptr %i11, null
  %i13 = select i1 %i9, i1 true, i1 %i12
  br i1 %i13, label %bb51, label %bb14

bb14:                                             ; preds = %bb6
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 6, !intel-tbaa !1627
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1627
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf", ptr %i16, i64 0, i32 1, !intel-tbaa !1628
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i17, i64 0, i32 2, !intel-tbaa !1633
  %i19 = load i32, ptr %i18, align 4, !tbaa !1633
  %i20 = icmp eq i32 %i19, 0
  br i1 %i20, label %bb51, label %bb21

bb21:                                             ; preds = %bb14
  %i22 = add i32 %i19, -1
  %i23 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %arg, i64 0, i32 4, !intel-tbaa !1634
  %i24 = load ptr, ptr %i23, align 8, !tbaa !1634
  %i25 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i24, i32 noundef %i22)
  %i26 = icmp eq ptr %i25, null
  br i1 %i26, label %bb27, label %bb31

bb27:                                             ; preds = %bb46, %bb21
  %i28 = phi i32 [ %i22, %bb21 ], [ %i47, %bb46 ]
  %i29 = load ptr, ptr %i15, align 8, !tbaa !1627
  %i30 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i29, i32 noundef %i28)
  br label %bb51

bb31:                                             ; preds = %bb46, %bb21
  %i32 = phi ptr [ %i49, %bb46 ], [ %i25, %bb21 ]
  %i33 = phi i32 [ %i47, %bb46 ], [ %i22, %bb21 ]
  %i34 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i32, i64 0, i32 6, !intel-tbaa !1625
  %i35 = load ptr, ptr %i34, align 8, !tbaa !1625
  %i36 = icmp ne ptr %i35, null
  %i37 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl", ptr %i32, i64 0, i32 7
  %i38 = load ptr, ptr %i37, align 8
  %i39 = icmp ne ptr %i38, null
  %i40 = select i1 %i36, i1 true, i1 %i39
  br i1 %i40, label %bb41, label %bb44

bb41:                                             ; preds = %bb31
  %i42 = load ptr, ptr %i15, align 8, !tbaa !1627
  %i43 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i42, i32 noundef %i33)
  br label %bb51

bb44:                                             ; preds = %bb31
  %i45 = icmp eq i32 %i33, 0
  br i1 %i45, label %bb51, label %bb46, !llvm.loop !1635

bb46:                                             ; preds = %bb44
  %i47 = add i32 %i33, -1
  %i48 = load ptr, ptr %i23, align 8, !tbaa !1634
  %i49 = tail call fastcc noundef ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(48) %i48, i32 noundef %i47)
  %i50 = icmp eq ptr %i49, null
  br i1 %i50, label %bb27, label %bb31

bb51:                                             ; preds = %bb44, %bb41, %bb27, %bb14, %bb6, %bb
  %i52 = phi ptr [ %i2, %bb6 ], [ %i2, %bb ], [ %i43, %bb41 ], [ %i30, %bb27 ], [ %i2, %bb14 ], [ %i2, %bb44 ]
  %i53 = phi ptr [ %i4, %bb6 ], [ null, %bb ], [ %i32, %bb41 ], [ null, %bb27 ], [ %i4, %bb14 ], [ %i32, %bb44 ]
  store ptr %i53, ptr %arg1, align 8, !tbaa !1636
  ret ptr %i52
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_710RefStackOfINS_13XMLEntityDeclEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #16 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1637 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1638
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 2, !intel-tbaa !1643
  %i3 = load i32, ptr %i2, align 4, !tbaa !1643
  %i4 = icmp ult i32 %i3, %arg1
  br i1 %i4, label %bb5, label %bb14

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1644
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1644
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1798, i32 noundef 55, i32 noundef 79, ptr noundef %i8)
          to label %bb9 unwind label %bb12

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb10:                                             ; preds = %bb21, %bb12
  %i11 = phi { ptr, i32 } [ %i13, %bb12 ], [ %i22, %bb21 ]
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb5
  %i13 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #47
  br label %bb10

bb14:                                             ; preds = %bb
  %i15 = icmp ugt i32 %i3, %arg1
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb14
  %i17 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1644
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1644
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i17, ptr noundef nonnull @.str.4453, i32 noundef 241, i32 noundef 116, ptr noundef %i19)
          to label %bb20 unwind label %bb21

bb20:                                             ; preds = %bb16
  tail call void @__cxa_throw(ptr nonnull %i17, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb21:                                             ; preds = %bb16
  %i22 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i17) #47
  br label %bb10

bb23:                                             ; preds = %bb14
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 4, !intel-tbaa !1645
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1645
  %i26 = zext i32 %arg1 to i64
  %i27 = getelementptr inbounds ptr, ptr %i25, i64 %i26
  %i28 = load ptr, ptr %i27, align 8, !tbaa !1636
  ret ptr %i28
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_710RefStackOfINS_9XMLReaderEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(48) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #16 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1646 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1628
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 2, !intel-tbaa !1633
  %i3 = load i32, ptr %i2, align 4, !tbaa !1633
  %i4 = icmp ult i32 %i3, %arg1
  br i1 %i4, label %bb5, label %bb14

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1647
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1647
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1798, i32 noundef 55, i32 noundef 79, ptr noundef %i8)
          to label %bb9 unwind label %bb12

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb10:                                             ; preds = %bb21, %bb12
  %i11 = phi { ptr, i32 } [ %i13, %bb12 ], [ %i22, %bb21 ]
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb5
  %i13 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #47
  br label %bb10

bb14:                                             ; preds = %bb
  %i15 = icmp ugt i32 %i3, %arg1
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb14
  %i17 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 5, !intel-tbaa !1647
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1647
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i17, ptr noundef nonnull @.str.4453, i32 noundef 241, i32 noundef 116, ptr noundef %i19)
          to label %bb20 unwind label %bb21

bb20:                                             ; preds = %bb16
  tail call void @__cxa_throw(ptr nonnull %i17, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb21:                                             ; preds = %bb16
  %i22 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i17) #47
  br label %bb10

bb23:                                             ; preds = %bb14
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf", ptr %i, i64 0, i32 4, !intel-tbaa !1648
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1648
  %i26 = zext i32 %arg1 to i64
  %i27 = getelementptr inbounds ptr, ptr %i25, i64 %i26
  %i28 = load ptr, ptr %i27, align 8, !tbaa !1649
  ret ptr %i28
}

; Function Attrs: inlinehint mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #37 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1650 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1575
  %i2 = load ptr, ptr %i, align 8, !tbaa !1575
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 1, !intel-tbaa !1576
  %i6 = load i32, ptr %i5, align 4, !tbaa !1576
  %i7 = icmp ugt i32 %i6, %arg1
  br i1 %i7, label %bb15, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 4, !intel-tbaa !1579
  %i11 = load ptr, ptr %i10, align 8, !tbaa !1579
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i9, ptr noundef nonnull @.str.6.5757, i32 noundef 206, i32 noundef 116, ptr noundef %i11)
          to label %bb12 unwind label %bb13

bb12:                                             ; preds = %bb8
  tail call void @__cxa_throw(ptr nonnull %i9, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb13:                                             ; preds = %bb8
  %i14 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i9) #47
  resume { ptr, i32 } %i14

bb15:                                             ; preds = %bb4
  %i16 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 3, !intel-tbaa !1580
  %i17 = load ptr, ptr %i16, align 8, !tbaa !1580
  %i18 = zext i32 %arg1 to i64
  %i19 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i17, i64 %i18, i32 1
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1581
  br label %bb21

bb21:                                             ; preds = %bb15, %bb
  %i22 = phi ptr [ %i20, %bb15 ], [ null, %bb ]
  ret ptr %i22
}

; Function Attrs: inlinehint mustprogress uwtable
define dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) local_unnamed_addr #37 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1651 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1436
  %i2 = load ptr, ptr %i, align 8, !tbaa !1436
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 1, !intel-tbaa !1438
  %i6 = load i32, ptr %i5, align 4, !tbaa !1438
  %i7 = icmp ugt i32 %i6, %arg1
  br i1 %i7, label %bb15, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i10 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 4, !intel-tbaa !1441
  %i11 = load ptr, ptr %i10, align 8, !tbaa !1441
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i9, ptr noundef nonnull @.str.4453, i32 noundef 249, i32 noundef 116, ptr noundef %i11)
          to label %bb12 unwind label %bb13

bb12:                                             ; preds = %bb8
  tail call void @__cxa_throw(ptr nonnull %i9, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb13:                                             ; preds = %bb8
  %i14 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i9) #47
  resume { ptr, i32 } %i14

bb15:                                             ; preds = %bb4
  %i16 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i2, i64 0, i32 3, !intel-tbaa !1442
  %i17 = load ptr, ptr %i16, align 8, !tbaa !1442
  %i18 = zext i32 %arg1 to i64
  %i19 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i17, i64 %i18, i32 2
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1443
  br label %bb21

bb21:                                             ; preds = %bb15, %bb
  %i22 = phi ptr [ %i20, %bb15 ], [ null, %bb ]
  ret ptr %i22
}

; Function Attrs: inlinehint uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3) local_unnamed_addr #38 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1652 {
bb:
  %i = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1431
  %i4 = load ptr, ptr %i, align 8, !tbaa !1431
  %i5 = icmp eq ptr %i4, null
  br i1 %i5, label %bb6, label %bb12

bb6:                                              ; preds = %bb
  %i7 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !1437
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1437
  %i9 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i8)
  %i10 = load ptr, ptr %i7, align 8, !tbaa !1437
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(32) %i9, i32 poison, ptr noundef %i10, i1 zeroext poison)
          to label %bb11 unwind label %bb151

bb11:                                             ; preds = %bb6
  store ptr %i9, ptr %i, align 8, !tbaa !1431
  br label %bb12

bb12:                                             ; preds = %bb11, %bb
  %i13 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1431
  %i14 = load ptr, ptr %i13, align 8, !tbaa !1431
  %i15 = icmp eq ptr %i14, null
  br i1 %i15, label %bb31, label %bb16

bb16:                                             ; preds = %bb12
  %i17 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i14, i64 0, i32 1, !intel-tbaa !1453
  %i18 = load i32, ptr %i17, align 4, !tbaa !1453
  %i19 = icmp eq i32 %i18, 0
  br i1 %i19, label %bb31, label %bb20

bb20:                                             ; preds = %bb26, %bb16
  %i21 = phi i32 [ %i27, %bb26 ], [ 0, %bb16 ]
  %i22 = load ptr, ptr %i13, align 8, !tbaa !1431
  %i23 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i22, i32 noundef %i21)
  %i24 = load ptr, ptr %i23, align 8, !tbaa !1457
  %i25 = icmp eq ptr %i24, %arg1
  br i1 %i25, label %bb29, label %bb26

bb26:                                             ; preds = %bb20
  %i27 = add nuw i32 %i21, 1
  %i28 = icmp eq i32 %i27, %i18
  br i1 %i28, label %bb31, label %bb20, !llvm.loop !1574

bb29:                                             ; preds = %bb20
  %i30 = icmp eq i32 %i21, -1
  br i1 %i30, label %bb31, label %bb87

bb31:                                             ; preds = %bb29, %bb26, %bb16, %bb12
  %i32 = load ptr, ptr %i, align 8, !tbaa !1431
  %i33 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !1437
  %i34 = load ptr, ptr %i33, align 8, !tbaa !1437
  %i35 = icmp eq ptr %arg3, null
  br i1 %i35, label %bb76, label %bb36

bb36:                                             ; preds = %bb31
  %i37 = load i16, ptr %arg3, align 2, !tbaa !1286
  %i38 = icmp eq i16 %i37, 0
  br i1 %i38, label %bb50, label %bb39

bb39:                                             ; preds = %bb39, %bb36
  %i40 = phi ptr [ %i41, %bb39 ], [ %arg3, %bb36 ]
  %i41 = getelementptr inbounds i16, ptr %i40, i64 1
  %i42 = load i16, ptr %i41, align 2, !tbaa !1286
  %i43 = icmp eq i16 %i42, 0
  br i1 %i43, label %bb44, label %bb39, !llvm.loop !1444

bb44:                                             ; preds = %bb39
  %i45 = ptrtoint ptr %i41 to i64
  %i46 = ptrtoint ptr %arg3 to i64
  %i47 = sub i64 %i45, %i46
  %i48 = add i64 %i47, 2
  %i49 = and i64 %i48, 8589934590
  br label %bb50

bb50:                                             ; preds = %bb44, %bb36
  %i51 = phi i64 [ %i49, %bb44 ], [ 2, %bb36 ]
  %i52 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i34, i64 0, i32 0
  %i53 = load ptr, ptr %i52, align 8, !tbaa !1256
  %i54 = tail call i1 @llvm.type.test(ptr %i53, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i54)
  %i55 = getelementptr inbounds ptr, ptr %i53, i64 2
  %i56 = load ptr, ptr %i55, align 8
  %i57 = icmp eq ptr %i56, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i57, label %bb58, label %bb74

bb58:                                             ; preds = %bb50
  %i59 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i51) #50
          to label %bb73 unwind label %bb60

bb60:                                             ; preds = %bb58
  %i61 = landingpad { ptr, i32 }
          catch ptr null
  %i62 = extractvalue { ptr, i32 } %i61, 0
  %i63 = tail call ptr @__cxa_begin_catch(ptr %i62) #47
  %i64 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i64, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i71) #48
  unreachable

bb72:                                             ; preds = %bb60
  unreachable

bb73:                                             ; preds = %bb58
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i59, ptr nonnull align 2 %arg3, i64 %i51, i1 false)
  br label %bb76

bb74:                                             ; preds = %bb50
  %i75 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb76:                                             ; preds = %bb73, %bb31
  %i77 = phi ptr [ %i59, %bb73 ], [ null, %bb31 ]
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr noundef nonnull align 8 dereferenceable(32) %i32, i32 poison)
  %i78 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i32, i64 0, i32 3, !intel-tbaa !1455
  %i79 = load ptr, ptr %i78, align 8, !tbaa !1455
  %i80 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i32, i64 0, i32 1, !intel-tbaa !1453
  %i81 = load i32, ptr %i80, align 4, !tbaa !1453
  %i82 = zext i32 %i81 to i64
  %i83 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 0
  %i84 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 2
  %i85 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i79, i64 %i82, i32 1
  store ptr %arg2, ptr %i85, align 8
  store ptr %i77, ptr %i84, align 8
  store ptr %arg1, ptr %i83, align 8, !tbaa !1457
  %i86 = add i32 %i81, 1
  store i32 %i86, ptr %i80, align 4, !tbaa !1453
  br label %bb150

bb87:                                             ; preds = %bb29
  %i88 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1575
  %i89 = load ptr, ptr %i88, align 8, !tbaa !1575
  %i90 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 1, !intel-tbaa !1576
  %i91 = load i32, ptr %i90, align 4, !tbaa !1576
  %i92 = icmp ugt i32 %i91, %i21
  br i1 %i92, label %bb100, label %bb93

bb93:                                             ; preds = %bb87
  %i94 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i95 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 4, !intel-tbaa !1579
  %i96 = load ptr, ptr %i95, align 8, !tbaa !1579
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i94, ptr noundef nonnull @.str.6.5757, i32 noundef 126, i32 noundef 116, ptr noundef %i96)
          to label %bb97 unwind label %bb98

bb97:                                             ; preds = %bb93
  tail call void @__cxa_throw(ptr nonnull %i94, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb98:                                             ; preds = %bb93
  %i99 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i94) #47
  br label %bb67

bb100:                                            ; preds = %bb87
  %i101 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i89, i64 0, i32 3, !intel-tbaa !1580
  %i102 = load ptr, ptr %i101, align 8, !tbaa !1580
  %i103 = zext i32 %i21 to i64
  %i104 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i102, i64 %i103, i32 1
  store ptr %arg2, ptr %i104, align 8, !tbaa !1581
  %i105 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !1436
  %i106 = load ptr, ptr %i105, align 8, !tbaa !1436
  %i107 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !1437
  %i108 = load ptr, ptr %i107, align 8, !tbaa !1437
  %i109 = icmp eq ptr %arg3, null
  br i1 %i109, label %bb148, label %bb110

bb110:                                            ; preds = %bb100
  %i111 = load i16, ptr %arg3, align 2, !tbaa !1286
  %i112 = icmp eq i16 %i111, 0
  br i1 %i112, label %bb124, label %bb113

bb113:                                            ; preds = %bb113, %bb110
  %i114 = phi ptr [ %i115, %bb113 ], [ %arg3, %bb110 ]
  %i115 = getelementptr inbounds i16, ptr %i114, i64 1
  %i116 = load i16, ptr %i115, align 2, !tbaa !1286
  %i117 = icmp eq i16 %i116, 0
  br i1 %i117, label %bb118, label %bb113, !llvm.loop !1444

bb118:                                            ; preds = %bb113
  %i119 = ptrtoint ptr %i115 to i64
  %i120 = ptrtoint ptr %arg3 to i64
  %i121 = sub i64 %i119, %i120
  %i122 = add i64 %i121, 2
  %i123 = and i64 %i122, 8589934590
  br label %bb124

bb124:                                            ; preds = %bb118, %bb110
  %i125 = phi i64 [ %i123, %bb118 ], [ 2, %bb110 ]
  %i126 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i108, i64 0, i32 0
  %i127 = load ptr, ptr %i126, align 8, !tbaa !1256
  %i128 = tail call i1 @llvm.type.test(ptr %i127, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i128)
  %i129 = getelementptr inbounds ptr, ptr %i127, i64 2
  %i130 = load ptr, ptr %i129, align 8
  %i131 = icmp eq ptr %i130, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i131, label %bb132, label %bb146

bb132:                                            ; preds = %bb124
  %i133 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i125) #50
          to label %bb145 unwind label %bb134

bb134:                                            ; preds = %bb132
  %i135 = landingpad { ptr, i32 }
          catch ptr null
  %i136 = extractvalue { ptr, i32 } %i135, 0
  %i137 = tail call ptr @__cxa_begin_catch(ptr %i136) #47
  %i138 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i138, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i143) #48
  unreachable

bb144:                                            ; preds = %bb134
  unreachable

bb145:                                            ; preds = %bb132
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i133, ptr nonnull align 2 %arg3, i64 %i125, i1 false)
  br label %bb148

bb146:                                            ; preds = %bb124
  %i147 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb148:                                            ; preds = %bb145, %bb100
  %i149 = phi ptr [ %i133, %bb145 ], [ null, %bb100 ]
  tail call void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5838(ptr noundef nonnull align 8 dereferenceable(40) %i106, ptr noundef %i149, i32 noundef %i21), !intel_dtrans_type !1653, !_Intel.Devirt.Call !1273
  br label %bb150

bb150:                                            ; preds = %bb148, %bb76
  ret void

bb151:                                            ; preds = %bb6
  %i152 = landingpad { ptr, i32 }
          cleanup
  tail call fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i9) #47
  br label %bb67
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1) unnamed_addr #16 align 2 !intel.dtrans.func.type !1654 {
bb:
  %i = getelementptr inbounds %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %arg, i64 0, i32 4, !intel-tbaa !1585
  %i2 = load ptr, ptr %i, align 8, !tbaa !1585
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb131, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !1431
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1431
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb11, label %bb8

bb8:                                              ; preds = %bb4
  %i9 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i6, i64 0, i32 1, !intel-tbaa !1453
  %i10 = load i32, ptr %i9, align 4, !tbaa !1453
  br label %bb11

bb11:                                             ; preds = %bb8, %bb4
  %i12 = phi i32 [ %i10, %bb8 ], [ 0, %bb4 ]
  %i13 = load ptr, ptr %i, align 8, !tbaa !1585
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %i13, i64 0, i32 2, !intel-tbaa !1590
  %i15 = load i32, ptr %i14, align 4, !tbaa !1590
  %i18 = icmp eq i32 %i15, 0
  br i1 %i18, label %bb131, label %bb19

bb19:                                             ; preds = %bb11
  %i20 = icmp eq i32 %i12, 0
  br i1 %i20, label %bb117, label %bb21

bb21:                                             ; preds = %bb114, %bb19
  %i22 = phi i32 [ %i115, %bb114 ], [ 0, %bb19 ]
  %i23 = load ptr, ptr %i, align 8, !tbaa !1585
  %i24 = tail call fastcc noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i23, i32 noundef %i22)
  %i25 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 0, i32 0, !intel-tbaa !1431
  %i26 = load ptr, ptr %i25, align 8, !tbaa !1431
  %i27 = icmp eq ptr %i26, null
  br i1 %i27, label %bb31, label %bb28

bb28:                                             ; preds = %bb21
  %i29 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i26, i64 0, i32 1, !intel-tbaa !1453
  %i30 = load i32, ptr %i29, align 4, !tbaa !1453
  br label %bb31

bb31:                                             ; preds = %bb28, %bb21
  %i32 = phi i32 [ %i30, %bb28 ], [ 0, %bb21 ]
  %i33 = icmp eq i32 %i12, %i32
  br i1 %i33, label %bb37, label %bb114

bb34:                                             ; preds = %bb112, %bb104, %bb96, %bb64, %bb61, %bb60
  %i35 = add nuw i32 %i38, 1
  %i36 = icmp eq i32 %i35, %i12
  br i1 %i36, label %bb131, label %bb37, !llvm.loop !1655

bb37:                                             ; preds = %bb34, %bb31
  %i38 = phi i32 [ %i35, %bb34 ], [ 0, %bb31 ]
  %i39 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i24, i32 noundef %i38)
  %i40 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i24, i32 noundef %i38)
  %i41 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i38)
  %i42 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i38)
  %i43 = getelementptr i8, ptr %arg, i64 56
  %i44 = load ptr, ptr %i43, align 8
  %i45 = icmp ne ptr %i39, null
  %i46 = icmp ne ptr %i41, null
  %i47 = and i1 %i45, %i46
  %i48 = icmp eq ptr %i40, null
  %i49 = icmp eq ptr %i42, null
  br i1 %i47, label %bb75, label %bb50

bb50:                                             ; preds = %bb37
  %i51 = or i1 %i48, %i49
  br i1 %i51, label %bb56, label %bb52

bb52:                                             ; preds = %bb50
  %i53 = load i16, ptr %i40, align 2, !tbaa !1286
  %i54 = load i16, ptr %i42, align 2, !tbaa !1286
  %i55 = icmp eq i16 %i53, %i54
  br i1 %i55, label %bb64, label %bb114

bb56:                                             ; preds = %bb50
  br i1 %i48, label %bb60, label %bb57

bb57:                                             ; preds = %bb56
  %i58 = load i16, ptr %i40, align 2, !tbaa !1286
  %i59 = icmp eq i16 %i58, 0
  br i1 %i59, label %bb60, label %bb114

bb60:                                             ; preds = %bb57, %bb56
  br i1 %i49, label %bb34, label %bb61

bb61:                                             ; preds = %bb60
  %i62 = load i16, ptr %i42, align 2, !tbaa !1286
  %i63 = icmp eq i16 %i62, 0
  br i1 %i63, label %bb34, label %bb114

bb64:                                             ; preds = %bb69, %bb52
  %i65 = phi i16 [ %i72, %bb69 ], [ %i53, %bb52 ]
  %i66 = phi ptr [ %i71, %bb69 ], [ %i42, %bb52 ]
  %i67 = phi ptr [ %i70, %bb69 ], [ %i40, %bb52 ]
  %i68 = icmp eq i16 %i65, 0
  br i1 %i68, label %bb34, label %bb69

bb69:                                             ; preds = %bb64
  %i70 = getelementptr inbounds i16, ptr %i67, i64 1
  %i71 = getelementptr inbounds i16, ptr %i66, i64 1
  %i72 = load i16, ptr %i70, align 2, !tbaa !1286
  %i73 = load i16, ptr %i71, align 2, !tbaa !1286
  %i74 = icmp eq i16 %i72, %i73
  br i1 %i74, label %bb64, label %bb114, !llvm.loop !1656

bb75:                                             ; preds = %bb37
  br i1 %i48, label %bb80, label %bb76

bb76:                                             ; preds = %bb75
  %i77 = load i16, ptr %i40, align 2, !tbaa !1286
  %i78 = icmp ne i16 %i77, 0
  %i79 = zext i1 %i78 to i32
  br label %bb80

bb80:                                             ; preds = %bb76, %bb75
  %i81 = phi i1 [ false, %bb75 ], [ %i78, %bb76 ]
  %i82 = phi i32 [ 0, %bb75 ], [ %i79, %bb76 ]
  br i1 %i49, label %bb87, label %bb83

bb83:                                             ; preds = %bb80
  %i84 = load i16, ptr %i42, align 2, !tbaa !1286
  %i85 = icmp ne i16 %i84, 0
  %i86 = zext i1 %i85 to i32
  br label %bb87

bb87:                                             ; preds = %bb83, %bb80
  %i88 = phi i1 [ false, %bb80 ], [ %i85, %bb83 ]
  %i89 = phi i32 [ 0, %bb80 ], [ %i86, %bb83 ]
  %i90 = or i32 %i89, %i82
  %i91 = icmp eq i32 %i90, 0
  br i1 %i91, label %bb112, label %bb92

bb92:                                             ; preds = %bb87
  %i93 = and i1 %i81, %i88
  br i1 %i93, label %bb94, label %bb114

bb94:                                             ; preds = %bb92
  %i95 = icmp eq ptr %i39, %i41
  br i1 %i95, label %bb96, label %bb104

bb96:                                             ; preds = %bb94
  %i97 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %i39, i64 0, i32 0, i32 0
  %i98 = load ptr, ptr %i97, align 8, !tbaa !1256
  %i99 = tail call i1 @llvm.type.test(ptr %i98, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i99)
  %i100 = getelementptr inbounds ptr, ptr %i98, i64 10
  %i101 = load ptr, ptr %i100, align 8
  %i102 = tail call noundef i32 %i101(ptr noundef nonnull align 8 dereferenceable(104) %i39, ptr noundef %i40, ptr noundef %i42, ptr noundef %i44), !intel_dtrans_type !1657
  %i103 = icmp eq i32 %i102, 0
  br i1 %i103, label %bb34, label %bb114

bb104:                                            ; preds = %bb94
  %i105 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %i41, i64 0, i32 0, i32 0
  %i106 = load ptr, ptr %i105, align 8, !tbaa !1256
  %i107 = tail call i1 @llvm.type.test(ptr %i106, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i107)
  %i108 = getelementptr inbounds ptr, ptr %i106, i64 10
  %i109 = load ptr, ptr %i108, align 8
  %i110 = tail call noundef i32 %i109(ptr noundef nonnull align 8 dereferenceable(104) %i41, ptr noundef %i40, ptr noundef %i42, ptr noundef %i44), !intel_dtrans_type !1657
  %i111 = icmp eq i32 %i110, 0
  br i1 %i111, label %bb34, label %bb114

bb112:                                            ; preds = %bb87
  %i113 = icmp eq ptr %i39, %i41
  br i1 %i113, label %bb34, label %bb114

bb114:                                            ; preds = %bb112, %bb104, %bb96, %bb92, %bb69, %bb61, %bb57, %bb52, %bb31
  %i115 = add nuw i32 %i22, 1
  %i116 = icmp eq i32 %i115, %i15
  br i1 %i116, label %bb131, label %bb21, !llvm.loop !1658

bb117:                                            ; preds = %bb128, %bb19
  %i118 = phi i32 [ %i129, %bb128 ], [ 0, %bb19 ]
  %i119 = load ptr, ptr %i, align 8, !tbaa !1585
  %i120 = tail call fastcc noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i119, i32 noundef %i118)
  %i121 = getelementptr inbounds %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i120, i64 0, i32 0, !intel-tbaa !1431
  %i122 = load ptr, ptr %i121, align 8, !tbaa !1431
  %i123 = icmp eq ptr %i122, null
  br i1 %i123, label %bb131, label %bb124

bb124:                                            ; preds = %bb117
  %i125 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %i122, i64 0, i32 1, !intel-tbaa !1453
  %i126 = load i32, ptr %i125, align 4, !tbaa !1453
  %i127 = icmp eq i32 %i126, 0
  br i1 %i127, label %bb131, label %bb128

bb128:                                            ; preds = %bb124
  %i129 = add nuw i32 %i118, 1
  %i130 = icmp eq i32 %i129, %i15
  br i1 %i130, label %bb131, label %bb117, !llvm.loop !1658

bb131:                                            ; preds = %bb128, %bb124, %bb117, %bb114, %bb34, %bb11, %bb
  %i132 = phi i1 [ false, %bb ], [ false, %bb11 ], [ true, %bb124 ], [ false, %bb128 ], [ true, %bb34 ], [ false, %bb114 ], [ true, %bb117 ]
  ret i1 %i132
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !1659 dso_local noundef i32 @printf(ptr nocapture noundef readonly "intel_dtrans_func_index"="1", ...) local_unnamed_addr #28

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) unnamed_addr #16 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1660 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !1590
  %i2 = load i32, ptr %i, align 4, !tbaa !1590
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 5, !intel-tbaa !1593
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1593
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.4453, i32 noundef 249, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #47
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1592
  %i13 = load ptr, ptr %i12, align 8, !tbaa !1592
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1594
  ret ptr %i16
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, i1 zeroext %arg3) unnamed_addr #30 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1662 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !1448
  store i8 0, ptr %i, align 8, !tbaa !1448
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1453
  store i32 0, ptr %i4, align 4, !tbaa !1453
  %i5 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !1454
  store i32 1, ptr %i5, align 8, !tbaa !1454
  %i6 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !1455
  store ptr null, ptr %i6, align 8, !tbaa !1455
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1456
  store ptr %arg2, ptr %i7, align 8, !tbaa !1456
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !1256
  %i10 = tail call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 2
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i13, label %bb14, label %bb28

bb14:                                             ; preds = %bb
  %i15 = invoke noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #50
          to label %bb30 unwind label %bb16

bb16:                                             ; preds = %bb14
  %i17 = landingpad { ptr, i32 }
          catch ptr null
  %i18 = extractvalue { ptr, i32 } %i17, 0
  %i19 = tail call ptr @__cxa_begin_catch(ptr %i18) #47
  %i20 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i20, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i26) #48
  unreachable

bb27:                                             ; preds = %bb16
  unreachable

bb28:                                             ; preds = %bb
  %i29 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb30:                                             ; preds = %bb14
  store ptr %i15, ptr %i6, align 8, !tbaa !1455
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %i15, i8 0, i64 24, i1 false)
  ret void
}

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 %arg1) local_unnamed_addr #39 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1663 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1453
  %i2 = load i32, ptr %i, align 4, !tbaa !1453
  %i3 = add i32 %i2, 1
  %i4 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !1454
  %i5 = load i32, ptr %i4, align 8, !tbaa !1454
  %i6 = icmp ugt i32 %i3, %i5
  br i1 %i6, label %bb7, label %bb69

bb7:                                              ; preds = %bb
  %i8 = uitofp i32 %i2 to double
  %i9 = fmul fast double %i8, 1.250000e+00
  %i10 = fptoui double %i9 to i32
  %i11 = icmp ult i32 %i3, %i10
  %i12 = select i1 %i11, i32 %i10, i32 %i3
  %i13 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1456
  %i14 = load ptr, ptr %i13, align 8, !tbaa !1456
  %i15 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1256
  %i17 = tail call i1 @llvm.type.test(ptr %i16, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i17)
  %i18 = getelementptr inbounds ptr, ptr %i16, i64 2
  %i19 = load ptr, ptr %i18, align 8
  %i20 = icmp eq ptr %i19, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i20, label %bb21, label %bb37

bb21:                                             ; preds = %bb7
  %i22 = zext i32 %i12 to i64
  %i23 = mul nuw nsw i64 %i22, 24
  %i24 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i23) #50
          to label %bb39 unwind label %bb25

bb25:                                             ; preds = %bb21
  %i26 = landingpad { ptr, i32 }
          catch ptr null
  %i27 = extractvalue { ptr, i32 } %i26, 0
  %i28 = tail call ptr @__cxa_begin_catch(ptr %i27) #47
  %i29 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i29, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i35) #48
  unreachable

bb36:                                             ; preds = %bb25
  unreachable

bb37:                                             ; preds = %bb7
  %i38 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb39:                                             ; preds = %bb21
  %i40 = load i32, ptr %i, align 4, !tbaa !1453
  %i41 = icmp eq i32 %i40, 0
  %i42 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3
  %i43 = load ptr, ptr %i42, align 8, !tbaa !1455
  br i1 %i41, label %bb46, label %bb44

bb44:                                             ; preds = %bb39
  %i45 = zext i32 %i40 to i64
  br label %bb56

bb46:                                             ; preds = %bb56, %bb39
  %i47 = load ptr, ptr %i13, align 8, !tbaa !1456
  %i48 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i47, i64 0, i32 0
  %i49 = load ptr, ptr %i48, align 8, !tbaa !1256
  %i50 = tail call i1 @llvm.type.test(ptr %i49, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i50)
  %i51 = getelementptr inbounds ptr, ptr %i49, i64 3
  %i52 = load ptr, ptr %i51, align 8
  %i53 = icmp eq ptr %i52, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i53, label %bb55, label %bb54

bb54:                                             ; preds = %bb46
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273
  unreachable

bb55:                                             ; preds = %bb46
  tail call void @_ZdlPv(ptr noundef %i43) #47
  store ptr %i24, ptr %i42, align 8, !tbaa !1455
  store i32 %i12, ptr %i4, align 8, !tbaa !1454
  br label %bb69

bb56:                                             ; preds = %bb56, %bb44
  %i57 = phi i64 [ 0, %bb44 ], [ %i67, %bb56 ]
  %i58 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 0
  %i59 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 2
  %i60 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i43, i64 %i57, i32 1
  %i61 = load ptr, ptr %i60, align 8
  %i62 = load ptr, ptr %i59, align 8
  %i63 = load ptr, ptr %i58, align 8, !tbaa !1457
  %i64 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 0
  %i65 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 2
  %i66 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i24, i64 %i57, i32 1
  store ptr %i61, ptr %i66, align 8
  store ptr %i62, ptr %i65, align 8
  store ptr %i63, ptr %i64, align 8, !tbaa !1457
  %i67 = add nuw nsw i64 %i57, 1
  %i68 = icmp eq i64 %i67, %i45
  br i1 %i68, label %bb46, label %bb56, !llvm.loop !1664

bb69:                                             ; preds = %bb55, %bb
  ret void
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #10 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1665 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i5, align 8, !tbaa !1256
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 1
  store i32 0, ptr %i6, align 8, !tbaa !1259
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 2
  store ptr null, ptr %i7, align 8, !tbaa !1267
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 3
  store i32 %arg2, ptr %i8, align 8, !tbaa !1268
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 4
  store ptr null, ptr %i9, align 8, !tbaa !1269
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 5
  store ptr %arg4, ptr %i10, align 8, !tbaa !1270
  %i11 = icmp eq ptr %arg4, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  store ptr %i13, ptr %i10, align 8, !tbaa !1270
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %arg4, %bb ]
  %i16 = icmp eq ptr %arg1, null
  br i1 %i16, label %bb45, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg1) #49
  %i19 = add i64 %i18, 1
  %i20 = and i64 %i19, 4294967295
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb43

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #50
          to label %bb42 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = tail call ptr @__cxa_begin_catch(ptr %i31) #47
  %i33 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i40) #48
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb27
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %i28, ptr nonnull align 1 %arg1, i64 %i20, i1 false)
  br label %bb45

bb43:                                             ; preds = %bb17
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb45:                                             ; preds = %bb42, %bb14
  %i46 = phi ptr [ %i28, %bb42 ], [ null, %bb14 ]
  store ptr %i46, ptr %i7, align 8, !tbaa !1267
  %i47 = getelementptr %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_721NumberFormatExceptionE.0, i64 0, i64 2), ptr %i47, align 8, !tbaa !1256
  invoke fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb48 unwind label %bb49

bb48:                                             ; preds = %bb45
  ret void

bb49:                                             ; preds = %bb45
  %i50 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #47
  br label %bb36
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_713XMLBigDecimalC2EPKtPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2) unnamed_addr #10 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1667 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_713XMLBigDecimalE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !1256
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 1, !intel-tbaa !1391
  store i32 0, ptr %i3, align 8, !tbaa !1391
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 2, !intel-tbaa !1393
  store i32 0, ptr %i4, align 4, !tbaa !1393
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 3, !intel-tbaa !1394
  store i32 0, ptr %i5, align 8, !tbaa !1394
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 4, !intel-tbaa !1669
  store i32 0, ptr %i6, align 4, !tbaa !1669
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 5, !intel-tbaa !1396
  store ptr null, ptr %i7, align 8, !tbaa !1396
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 6, !intel-tbaa !1395
  store ptr null, ptr %i8, align 8, !tbaa !1395
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 7, !intel-tbaa !1397
  store ptr %arg2, ptr %i9, align 8, !tbaa !1397
  %i10 = icmp eq ptr %arg1, null
  br i1 %i10, label %bb14, label %bb11

bb11:                                             ; preds = %bb
  %i12 = load i16, ptr %arg1, align 2, !tbaa !1286
  %i13 = icmp eq i16 %i12, 0
  br i1 %i13, label %bb14, label %bb20

bb14:                                             ; preds = %bb11, %bb
  %i15 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i16 = load ptr, ptr %i9, align 8, !tbaa !1397
  invoke fastcc void @_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i15, ptr noundef nonnull @.str.2622, i32 noundef 63, i32 noundef 261, ptr noundef %i16)
          to label %bb17 unwind label %bb18

bb17:                                             ; preds = %bb14
  tail call void @__cxa_throw(ptr nonnull %i15, ptr nonnull @_ZTIN11xercesc_2_721NumberFormatExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb18:                                             ; preds = %bb14
  %i19 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i15) #47
  br label %bb94

bb20:                                             ; preds = %bb20, %bb11
  %i21 = phi ptr [ %i22, %bb20 ], [ %arg1, %bb11 ]
  %i22 = getelementptr inbounds i16, ptr %i21, i64 1
  %i23 = load i16, ptr %i22, align 2, !tbaa !1286
  %i24 = icmp eq i16 %i23, 0
  br i1 %i24, label %bb25, label %bb20, !llvm.loop !1670

bb25:                                             ; preds = %bb20
  %i26 = ptrtoint ptr %i22 to i64
  %i27 = ptrtoint ptr %arg1 to i64
  %i28 = sub i64 %i26, %i27
  %i29 = lshr exact i64 %i28, 1
  %i30 = trunc i64 %i29 to i32
  store i32 %i30, ptr %i6, align 4, !tbaa !1669
  %i31 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i32 = load ptr, ptr %i31, align 8, !tbaa !1256
  %i33 = tail call i1 @llvm.type.test(ptr %i32, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i33)
  %i34 = getelementptr inbounds ptr, ptr %i32, i64 2
  %i35 = load ptr, ptr %i34, align 8
  %i36 = icmp eq ptr %i35, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i36, label %bb37, label %bb53

bb37:                                             ; preds = %bb25
  %i38 = shl i64 %i28, 1
  %i39 = add i64 %i38, 4
  %i40 = and i64 %i39, 8589934590
  %i41 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i40) #50
          to label %bb56 unwind label %bb42

bb42:                                             ; preds = %bb37
  %i43 = landingpad { ptr, i32 }
          catch ptr null
  %i44 = extractvalue { ptr, i32 } %i43, 0
  %i45 = tail call ptr @__cxa_begin_catch(ptr %i44) #47
  %i46 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i46, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
          to label %bb52 unwind label %bb47

bb47:                                             ; preds = %bb42
  %i48 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  invoke void @__cxa_end_catch()
          to label %bb66 unwind label %bb49

bb49:                                             ; preds = %bb47
  %i50 = landingpad { ptr, i32 }
          catch ptr null
  %i51 = extractvalue { ptr, i32 } %i50, 0
  tail call fastcc void @__clang_call_terminate(ptr %i51) #48
  unreachable

bb52:                                             ; preds = %bb42
  unreachable

bb53:                                             ; preds = %bb25
  %i54 = invoke noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison)
          to label %bb55 unwind label %bb64, !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273

bb55:                                             ; preds = %bb53
  unreachable

bb56:                                             ; preds = %bb37
  store ptr %i41, ptr %i7, align 8, !tbaa !1396
  %i57 = load i32, ptr %i6, align 4, !tbaa !1669
  %i58 = zext i32 %i57 to i64
  %i59 = shl nuw nsw i64 %i58, 1
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i41, ptr nonnull align 2 %arg1, i64 %i59, i1 false)
  %i60 = zext i32 %i57 to i64
  %i61 = getelementptr inbounds i16, ptr %i41, i64 %i60
  store i16 0, ptr %i61, align 2, !tbaa !1286
  %i62 = getelementptr inbounds i16, ptr %i61, i64 1, !intel-tbaa !1286
  store ptr %i62, ptr %i8, align 8, !tbaa !1395
  %i63 = load ptr, ptr %i9, align 8, !tbaa !1397
  invoke fastcc void @_ZN11xercesc_2_713XMLBigDecimal12parseDecimalEPKtPtRiS4_S4_PNS_13MemoryManagerE(ptr noundef nonnull %arg1, ptr noundef nonnull %i62, ptr noundef nonnull align 4 dereferenceable(4) %i3, ptr noundef nonnull align 4 dereferenceable(4) %i4, ptr noundef nonnull align 4 dereferenceable(4) %i5, ptr noundef %i63)
          to label %bb76 unwind label %bb64

bb64:                                             ; preds = %bb56, %bb53
  %i65 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb66

bb66:                                             ; preds = %bb64, %bb47
  %i67 = phi { ptr, i32 } [ %i65, %bb64 ], [ %i48, %bb47 ]
  %i68 = extractvalue { ptr, i32 } %i67, 1
  %i69 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #47
  %i70 = icmp eq i32 %i68, %i69
  br i1 %i70, label %bb71, label %bb77

bb71:                                             ; preds = %bb66
  %i72 = extractvalue { ptr, i32 } %i67, 0
  %i73 = tail call ptr @__cxa_begin_catch(ptr %i72) #47
  invoke void @__cxa_rethrow() #51
          to label %bb99 unwind label %bb74

bb74:                                             ; preds = %bb71
  %i75 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb94 unwind label %bb96

bb76:                                             ; preds = %bb56
  ret void

bb77:                                             ; preds = %bb66
  %i78 = load ptr, ptr %i7, align 8, !tbaa !1396
  %i79 = icmp eq ptr %i78, null
  br i1 %i79, label %bb94, label %bb80

bb80:                                             ; preds = %bb77
  %i81 = load ptr, ptr %i9, align 8, !tbaa !1397
  %i82 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i81, i64 0, i32 0
  %i83 = load ptr, ptr %i82, align 8, !tbaa !1256
  %i84 = tail call i1 @llvm.type.test(ptr %i83, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i84)
  %i85 = getelementptr inbounds ptr, ptr %i83, i64 3
  %i86 = load ptr, ptr %i85, align 8
  %i87 = icmp eq ptr %i86, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i87, label %bb90, label %bb88

bb88:                                             ; preds = %bb80
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb89 unwind label %bb91, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb89:                                             ; preds = %bb88
  unreachable

bb90:                                             ; preds = %bb80
  tail call void @_ZdlPv(ptr noundef nonnull %i78) #47
  br label %bb94

bb91:                                             ; preds = %bb88
  %i92 = landingpad { ptr, i32 }
          catch ptr null
  %i93 = extractvalue { ptr, i32 } %i92, 0
  tail call fastcc void @__clang_call_terminate(ptr %i93) #48
  unreachable

bb94:                                             ; preds = %bb90, %bb77, %bb74, %bb18
  %i95 = phi { ptr, i32 } [ %i19, %bb18 ], [ %i75, %bb74 ], [ %i67, %bb90 ], [ %i67, %bb77 ]
  resume { ptr, i32 } %i95

bb96:                                             ; preds = %bb74
  %i97 = landingpad { ptr, i32 }
          catch ptr null
  %i98 = extractvalue { ptr, i32 } %i97, 0
  tail call fastcc void @__clang_call_terminate(ptr %i98) #48
  unreachable

bb99:                                             ; preds = %bb71
  unreachable
}

; Function Attrs: mustprogress uwtable
define hidden fastcc void @_ZN11xercesc_2_713XMLBigDecimal12parseDecimalEPKtPtRiS4_S4_PNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef "intel_dtrans_func_index"="2" %arg1, ptr nocapture noundef nonnull writeonly align 4 dereferenceable(4) "intel_dtrans_func_index"="3" %arg2, ptr nocapture noundef nonnull align 4 dereferenceable(4) "intel_dtrans_func_index"="4" %arg3, ptr nocapture noundef nonnull align 4 dereferenceable(4) "intel_dtrans_func_index"="5" %arg4, ptr noundef "intel_dtrans_func_index"="6" %arg5) unnamed_addr #16 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1671 {
bb:
  store i16 0, ptr %arg1, align 2, !tbaa !1286
  store i32 0, ptr %arg3, align 4, !tbaa !1672
  store i32 0, ptr %arg4, align 4, !tbaa !1672
  br label %bb6

bb6:                                              ; preds = %bb6, %bb
  %i = phi ptr [ %arg, %bb ], [ %i13, %bb6 ]
  %i7 = load i16, ptr %i, align 2, !tbaa !1286
  %i8 = zext i16 %i7 to i64
  %i9 = getelementptr inbounds [65536 x i8], ptr @_ZN11xercesc_2_710XMLChar1_019fgCharCharsTable1_0E, i64 0, i64 %i8, !intel-tbaa !1673
  %i10 = getelementptr [65536 x i8], ptr %i9, i64 0, i64 0
  %i11 = load i8, ptr %i10, align 1, !tbaa !1673
  %i12 = icmp slt i8 %i11, 0
  %i13 = getelementptr inbounds i16, ptr %i, i64 1
  br i1 %i12, label %bb6, label %bb14, !llvm.loop !1675

bb14:                                             ; preds = %bb6
  %i15 = icmp eq i16 %i7, 0
  br i1 %i15, label %bb16, label %bb21

bb16:                                             ; preds = %bb14
  %i17 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  invoke fastcc void @_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i17, ptr noundef nonnull @.str.2622, i32 noundef 220, i32 noundef 262, ptr noundef %arg5)
          to label %bb18 unwind label %bb19

bb18:                                             ; preds = %bb16
  tail call void @__cxa_throw(ptr nonnull %i17, ptr nonnull @_ZTIN11xercesc_2_721NumberFormatExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb19:                                             ; preds = %bb16
  %i20 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i17) #47
  br label %bb123

bb21:                                             ; preds = %bb14
  %i22 = icmp eq ptr %arg, null
  br i1 %i22, label %bb37, label %bb23

bb23:                                             ; preds = %bb21
  %i24 = load i16, ptr %arg, align 2, !tbaa !1286
  %i25 = icmp eq i16 %i24, 0
  br i1 %i25, label %bb37, label %bb26

bb26:                                             ; preds = %bb26, %bb23
  %i27 = phi ptr [ %i28, %bb26 ], [ %arg, %bb23 ]
  %i28 = getelementptr inbounds i16, ptr %i27, i64 1
  %i29 = load i16, ptr %i28, align 2, !tbaa !1286
  %i30 = icmp eq i16 %i29, 0
  br i1 %i30, label %bb31, label %bb26, !llvm.loop !1670

bb31:                                             ; preds = %bb26
  %i32 = ptrtoint ptr %i28 to i64
  %i33 = ptrtoint ptr %arg to i64
  %i34 = sub i64 %i32, %i33
  %i35 = lshr exact i64 %i34, 1
  %i36 = and i64 %i35, 4294967295
  br label %bb37

bb37:                                             ; preds = %bb31, %bb23, %bb21
  %i38 = phi i64 [ %i36, %bb31 ], [ 0, %bb23 ], [ 0, %bb21 ]
  %i39 = getelementptr inbounds i16, ptr %arg, i64 %i38, !intel-tbaa !1286
  br label %bb40

bb40:                                             ; preds = %bb40, %bb37
  %i41 = phi ptr [ %i39, %bb37 ], [ %i42, %bb40 ]
  %i42 = getelementptr inbounds i16, ptr %i41, i64 -1, !intel-tbaa !1286
  %i43 = load i16, ptr %i42, align 2, !tbaa !1286
  %i44 = zext i16 %i43 to i64
  %i45 = getelementptr inbounds [65536 x i8], ptr @_ZN11xercesc_2_710XMLChar1_019fgCharCharsTable1_0E, i64 0, i64 %i44, !intel-tbaa !1673
  %i46 = getelementptr [65536 x i8], ptr %i45, i64 0, i64 0
  %i47 = load i8, ptr %i46, align 1, !tbaa !1673
  %i48 = icmp slt i8 %i47, 0
  br i1 %i48, label %bb40, label %bb49, !llvm.loop !1676

bb49:                                             ; preds = %bb40
  store i32 1, ptr %arg2, align 4, !tbaa !1672
  switch i16 %i7, label %bb52 [
    i16 45, label %bb50
    i16 43, label %bb51
  ]

bb50:                                             ; preds = %bb49
  store i32 -1, ptr %arg2, align 4, !tbaa !1672
  br label %bb52

bb51:                                             ; preds = %bb49
  br label %bb52

bb52:                                             ; preds = %bb51, %bb50, %bb49
  %i53 = phi ptr [ %i13, %bb50 ], [ %i13, %bb51 ], [ %i, %bb49 ]
  br label %bb54

bb54:                                             ; preds = %bb54, %bb52
  %i55 = phi ptr [ %i53, %bb52 ], [ %i58, %bb54 ]
  %i56 = load i16, ptr %i55, align 2, !tbaa !1286
  %i57 = icmp eq i16 %i56, 48
  %i58 = getelementptr inbounds i16, ptr %i55, i64 1
  br i1 %i57, label %bb54, label %bb59, !llvm.loop !1677

bb59:                                             ; preds = %bb54
  %i60 = icmp ult ptr %i55, %i41
  br i1 %i60, label %bb61, label %bb63

bb61:                                             ; preds = %bb59
  %i62 = ptrtoint ptr %i41 to i64
  br label %bb104

bb63:                                             ; preds = %bb59
  store i32 0, ptr %arg2, align 4, !tbaa !1672
  br label %bb122

bb64:                                             ; preds = %bb98
  %i65 = load i32, ptr %arg4, align 4, !tbaa !1672
  br label %bb66

bb66:                                             ; preds = %bb77, %bb64
  %i67 = phi i32 [ %i65, %bb64 ], [ %i82, %bb77 ]
  %i68 = phi ptr [ %i100, %bb64 ], [ %i106, %bb77 ]
  %i69 = icmp sgt i32 %i67, 0
  br i1 %i69, label %bb108, label %bb120

bb70:                                             ; preds = %bb104, %bb77
  %i71 = phi i8 [ %i105, %bb104 ], [ 1, %bb77 ]
  %i72 = phi ptr [ %i107, %bb104 ], [ %i83, %bb77 ]
  %i73 = load i16, ptr %i72, align 2, !tbaa !1286
  %i74 = icmp eq i16 %i73, 46
  br i1 %i74, label %bb75, label %bb90

bb75:                                             ; preds = %bb70
  %i76 = icmp eq i8 %i71, 0
  br i1 %i76, label %bb77, label %bb85

bb77:                                             ; preds = %bb75
  %i78 = ptrtoint ptr %i72 to i64
  %i79 = sub i64 %i62, %i78
  %i80 = lshr exact i64 %i79, 1
  %i81 = trunc i64 %i80 to i32
  %i82 = add i32 %i81, -1
  store i32 %i82, ptr %arg4, align 4, !tbaa !1672
  %i83 = getelementptr inbounds i16, ptr %i72, i64 1
  %i84 = icmp ult ptr %i83, %i41
  br i1 %i84, label %bb70, label %bb66, !llvm.loop !1678

bb85:                                             ; preds = %bb75
  %i86 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  invoke fastcc void @_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i86, ptr noundef nonnull @.str.2622, i32 noundef 268, i32 noundef 263, ptr noundef %arg5)
          to label %bb87 unwind label %bb88

bb87:                                             ; preds = %bb85
  tail call void @__cxa_throw(ptr nonnull %i86, ptr nonnull @_ZTIN11xercesc_2_721NumberFormatExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb88:                                             ; preds = %bb85
  %i89 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i86) #47
  br label %bb123

bb90:                                             ; preds = %bb70
  %i91 = add i16 %i73, -58
  %i92 = icmp ult i16 %i91, -10
  br i1 %i92, label %bb93, label %bb98

bb93:                                             ; preds = %bb90
  %i94 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  invoke fastcc void @_ZN11xercesc_2_721NumberFormatExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i94, ptr noundef nonnull @.str.2622, i32 noundef 273, i32 noundef 264, ptr noundef %arg5)
          to label %bb95 unwind label %bb96

bb95:                                             ; preds = %bb93
  tail call void @__cxa_throw(ptr nonnull %i94, ptr nonnull @_ZTIN11xercesc_2_721NumberFormatExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb96:                                             ; preds = %bb93
  %i97 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i94) #47
  br label %bb123

bb98:                                             ; preds = %bb90
  %i99 = getelementptr inbounds i16, ptr %i72, i64 1
  %i100 = getelementptr inbounds i16, ptr %i106, i64 1
  store i16 %i73, ptr %i106, align 2, !tbaa !1286
  %i101 = load i32, ptr %arg3, align 4, !tbaa !1672
  %i102 = add nsw i32 %i101, 1
  store i32 %i102, ptr %arg3, align 4, !tbaa !1672
  %i103 = icmp ult ptr %i99, %i41
  br i1 %i103, label %bb104, label %bb64, !llvm.loop !1678

bb104:                                            ; preds = %bb98, %bb61
  %i105 = phi i8 [ 0, %bb61 ], [ %i71, %bb98 ]
  %i106 = phi ptr [ %arg1, %bb61 ], [ %i100, %bb98 ]
  %i107 = phi ptr [ %i55, %bb61 ], [ %i99, %bb98 ]
  br label %bb70

bb108:                                            ; preds = %bb114, %bb66
  %i109 = phi i32 [ %i118, %bb114 ], [ %i67, %bb66 ]
  %i110 = phi ptr [ %i111, %bb114 ], [ %i68, %bb66 ]
  %i111 = getelementptr inbounds i16, ptr %i110, i64 -1, !intel-tbaa !1286
  %i112 = load i16, ptr %i111, align 2, !tbaa !1286
  %i113 = icmp eq i16 %i112, 48
  br i1 %i113, label %bb114, label %bb120

bb114:                                            ; preds = %bb108
  %i115 = add nsw i32 %i109, -1
  store i32 %i115, ptr %arg4, align 4, !tbaa !1672
  %i116 = load i32, ptr %arg3, align 4, !tbaa !1672
  %i117 = add nsw i32 %i116, -1
  store i32 %i117, ptr %arg3, align 4, !tbaa !1672
  %i118 = load i32, ptr %arg4, align 4, !tbaa !1672
  %i119 = icmp sgt i32 %i118, 0
  br i1 %i119, label %bb108, label %bb120, !llvm.loop !1679

bb120:                                            ; preds = %bb114, %bb108, %bb66
  %i121 = phi ptr [ %i68, %bb66 ], [ %i110, %bb108 ], [ %i111, %bb114 ]
  store i16 0, ptr %i121, align 2, !tbaa !1286
  br label %bb122

bb122:                                            ; preds = %bb120, %bb63
  ret void

bb123:                                            ; preds = %bb96, %bb88, %bb19
  %i124 = phi { ptr, i32 } [ %i20, %bb19 ], [ %i89, %bb88 ], [ %i97, %bb96 ]
  resume { ptr, i32 } %i124
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_713XMLBigDecimalD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) unnamed_addr #11 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1680 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_713XMLBigDecimalE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !1256
  %i1 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 5, !intel-tbaa !1396
  %i2 = load ptr, ptr %i1, align 8, !tbaa !1396
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb16, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal", ptr %arg, i64 0, i32 7, !intel-tbaa !1397
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1397
  %i7 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i6, i64 0, i32 0
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1256
  %i9 = tail call i1 @llvm.type.test(ptr %i8, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i9)
  %i10 = getelementptr inbounds ptr, ptr %i8, i64 3
  %i11 = load ptr, ptr %i10, align 8
  %i12 = icmp eq ptr %i11, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i12, label %bb15, label %bb13

bb13:                                             ; preds = %bb4
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb14 unwind label %bb17, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb14:                                             ; preds = %bb13
  unreachable

bb15:                                             ; preds = %bb4
  tail call void @_ZdlPv(ptr noundef nonnull %i2) #47
  br label %bb16

bb16:                                             ; preds = %bb15, %bb
  ret void

bb17:                                             ; preds = %bb13
  %i18 = landingpad { ptr, i32 }
          catch ptr null
  %i19 = extractvalue { ptr, i32 } %i18, 0
  tail call fastcc void @__clang_call_terminate(ptr %i19) #48
  unreachable
}

; Function Attrs: uwtable
define hidden fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr nocapture noundef nonnull readonly "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1681 {
bb:
  %i = alloca %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", align 8
  %i2 = alloca %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", align 8
  call void @llvm.lifetime.start.p0(i64 96, ptr nonnull %i) #47
  call fastcc void @_ZN11xercesc_2_711XMLDateTimeC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(96) %i, ptr noundef nonnull align 8 dereferenceable(96) %arg)
  call void @llvm.lifetime.start.p0(i64 96, ptr nonnull %i2) #47
  invoke fastcc void @_ZN11xercesc_2_711XMLDateTimeC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(96) %i2, ptr noundef nonnull align 8 dereferenceable(96) %arg1)
          to label %bb3 unwind label %bb7

bb3:                                              ; preds = %bb
  call fastcc void @_ZN11xercesc_2_711XMLDateTime9normalizeEv(ptr noundef nonnull align 8 dereferenceable(96) %i)
  call fastcc void @_ZN11xercesc_2_711XMLDateTime9normalizeEv(ptr noundef nonnull align 8 dereferenceable(96) %i2)
  br label %bb13

bb4:                                              ; preds = %bb20
  %i5 = add nuw nsw i64 %i14, 1
  %i6 = icmp eq i64 %i5, 8
  br i1 %i6, label %bb22, label %bb13, !llvm.loop !1682

bb7:                                              ; preds = %bb
  %i8 = landingpad { ptr, i32 }
          cleanup
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i2) #47
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i9, align 8, !tbaa !1256
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 8, !intel-tbaa !1683
  %i11 = load ptr, ptr %i10, align 8, !tbaa !1683
  %i12 = icmp eq ptr %i11, null
  br i1 %i12, label %bb92, label %bb77

bb13:                                             ; preds = %bb4, %bb3
  %i14 = phi i64 [ 0, %bb3 ], [ %i5, %bb4 ]
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 1, i64 %i14, !intel-tbaa !1381
  %i16 = load i32, ptr %i15, align 4, !tbaa !1381
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i2, i64 0, i32 1, i64 %i14, !intel-tbaa !1381
  %i18 = load i32, ptr %i17, align 4, !tbaa !1381
  %i19 = icmp slt i32 %i16, %i18
  br i1 %i19, label %bb35, label %bb20

bb20:                                             ; preds = %bb13
  %i21 = icmp sgt i32 %i16, %i18
  br i1 %i21, label %bb35, label %bb4

bb22:                                             ; preds = %bb4
  %i23 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 7, !intel-tbaa !1684
  %i24 = load i8, ptr %i23, align 8, !tbaa !1684, !range !1451, !noundef !1452
  %i25 = icmp eq i8 %i24, 0
  br i1 %i25, label %bb34, label %bb26

bb26:                                             ; preds = %bb22
  %i27 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 6, !intel-tbaa !1685
  %i28 = load double, ptr %i27, align 8, !tbaa !1685
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i2, i64 0, i32 6, !intel-tbaa !1685
  %i30 = load double, ptr %i29, align 8, !tbaa !1685
  %i31 = fcmp fast olt double %i28, %i30
  br i1 %i31, label %bb35, label %bb32

bb32:                                             ; preds = %bb26
  %i33 = fcmp fast ogt double %i28, %i30
  br i1 %i33, label %bb35, label %bb34

bb34:                                             ; preds = %bb32, %bb22
  br label %bb35

bb35:                                             ; preds = %bb34, %bb32, %bb26, %bb20, %bb13
  %i36 = phi i32 [ 0, %bb34 ], [ -1, %bb26 ], [ 1, %bb32 ], [ 1, %bb20 ], [ -1, %bb13 ]
  %i37 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i2, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i37, align 8, !tbaa !1256
  %i38 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i2, i64 0, i32 8, !intel-tbaa !1683
  %i39 = load ptr, ptr %i38, align 8, !tbaa !1683
  %i40 = icmp eq ptr %i39, null
  br i1 %i40, label %bb56, label %bb41

bb41:                                             ; preds = %bb35
  %i42 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i2, i64 0, i32 9, !intel-tbaa !1686
  %i43 = load ptr, ptr %i42, align 8, !tbaa !1686
  %i44 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i43, i64 0, i32 0
  %i45 = load ptr, ptr %i44, align 8, !tbaa !1256
  %i46 = tail call i1 @llvm.type.test(ptr %i45, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i46)
  %i47 = getelementptr inbounds ptr, ptr %i45, i64 3
  %i48 = load ptr, ptr %i47, align 8
  %i49 = icmp eq ptr %i48, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i49, label %bb52, label %bb50

bb50:                                             ; preds = %bb41
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb51 unwind label %bb53, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb51:                                             ; preds = %bb50
  unreachable

bb52:                                             ; preds = %bb41
  tail call void @_ZdlPv(ptr noundef nonnull %i39) #47
  br label %bb56

bb53:                                             ; preds = %bb50
  %i54 = landingpad { ptr, i32 }
          catch ptr null
  %i55 = extractvalue { ptr, i32 } %i54, 0
  tail call fastcc void @__clang_call_terminate(ptr %i55) #48
  unreachable

bb56:                                             ; preds = %bb52, %bb35
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i2) #47
  %i57 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i57, align 8, !tbaa !1256
  %i58 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 8, !intel-tbaa !1683
  %i59 = load ptr, ptr %i58, align 8, !tbaa !1683
  %i60 = icmp eq ptr %i59, null
  br i1 %i60, label %bb76, label %bb61

bb61:                                             ; preds = %bb56
  %i62 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 9, !intel-tbaa !1686
  %i63 = load ptr, ptr %i62, align 8, !tbaa !1686
  %i64 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i63, i64 0, i32 0
  %i65 = load ptr, ptr %i64, align 8, !tbaa !1256
  %i66 = tail call i1 @llvm.type.test(ptr %i65, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i66)
  %i67 = getelementptr inbounds ptr, ptr %i65, i64 3
  %i68 = load ptr, ptr %i67, align 8
  %i69 = icmp eq ptr %i68, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i69, label %bb72, label %bb70

bb70:                                             ; preds = %bb61
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb71 unwind label %bb73, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb71:                                             ; preds = %bb70
  unreachable

bb72:                                             ; preds = %bb61
  tail call void @_ZdlPv(ptr noundef nonnull %i59) #47
  br label %bb76

bb73:                                             ; preds = %bb70
  %i74 = landingpad { ptr, i32 }
          catch ptr null
  %i75 = extractvalue { ptr, i32 } %i74, 0
  tail call fastcc void @__clang_call_terminate(ptr %i75) #48
  unreachable

bb76:                                             ; preds = %bb72, %bb56
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i) #47
  ret i32 %i36

bb77:                                             ; preds = %bb7
  %i78 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 9, !intel-tbaa !1686
  %i79 = load ptr, ptr %i78, align 8, !tbaa !1686
  %i80 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i79, i64 0, i32 0
  %i81 = load ptr, ptr %i80, align 8, !tbaa !1256
  %i82 = tail call i1 @llvm.type.test(ptr %i81, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i82)
  %i83 = getelementptr inbounds ptr, ptr %i81, i64 3
  %i84 = load ptr, ptr %i83, align 8
  %i85 = icmp eq ptr %i84, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i85, label %bb88, label %bb86

bb86:                                             ; preds = %bb77
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb87 unwind label %bb89, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb87:                                             ; preds = %bb86
  unreachable

bb88:                                             ; preds = %bb77
  tail call void @_ZdlPv(ptr noundef nonnull %i11) #47
  br label %bb92

bb89:                                             ; preds = %bb86
  %i90 = landingpad { ptr, i32 }
          catch ptr null
  %i91 = extractvalue { ptr, i32 } %i90, 0
  tail call fastcc void @__clang_call_terminate(ptr %i91) #48
  unreachable

bb92:                                             ; preds = %bb88, %bb7
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i) #47
  resume { ptr, i32 } %i8
}

; Function Attrs: mustprogress nofree nosync nounwind memory(write, argmem: readwrite, inaccessiblemem: none) uwtable
define hidden fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr nocapture noundef "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) unnamed_addr #40 align 2 !intel.dtrans.func.type !1687 {
bb:
  br label %bb12

bb3:                                              ; preds = %bb12
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 6, !intel-tbaa !1685
  store double 0.000000e+00, ptr %i, align 8, !tbaa !1685
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 7, !intel-tbaa !1684
  store i8 0, ptr %i4, align 1, !tbaa !1684
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 1, !intel-tbaa !1688
  store i32 0, ptr %i5, align 4, !tbaa !1688
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 0, !intel-tbaa !1688
  store i32 0, ptr %i6, align 4, !tbaa !1688
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 4, !intel-tbaa !1689
  store i32 0, ptr %i7, align 4, !tbaa !1689
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 3, !intel-tbaa !1690
  store i32 0, ptr %i8, align 4, !tbaa !1690
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 8, !intel-tbaa !1683
  %i10 = load ptr, ptr %i9, align 8, !tbaa !1683
  %i11 = icmp eq ptr %i10, null
  br i1 %i11, label %bb18, label %bb17

bb12:                                             ; preds = %bb12, %bb
  %i13 = phi i64 [ 0, %bb ], [ %i15, %bb12 ]
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 %i13, !intel-tbaa !1381
  store i32 0, ptr %i14, align 4, !tbaa !1381
  %i15 = add nuw nsw i64 %i13, 1
  %i16 = icmp eq i64 %i15, 8
  br i1 %i16, label %bb3, label %bb12, !llvm.loop !1691

bb17:                                             ; preds = %bb3
  store i16 0, ptr %i10, align 2, !tbaa !1286
  br label %bb18

bb18:                                             ; preds = %bb17, %bb3
  %i19 = zext i32 %arg2 to i64
  %i20 = getelementptr inbounds [4 x [8 x i32]], ptr @_ZN11xercesc_2_7L9DATETIMESE, i64 0, i64 %i19, i64 1, !intel-tbaa !1692
  %i21 = load i32, ptr %i20, align 4, !tbaa !1692
  %i22 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 1, !intel-tbaa !1381
  %i23 = load i32, ptr %i22, align 4, !tbaa !1381
  %i24 = add nsw i32 %i23, %i21
  %i25 = add nsw i32 %i24, -1
  %i26 = tail call i64 @div(i32 noundef %i25, i32 noundef 12) #53
  %i27 = trunc i64 %i26 to i32
  %i28 = mul i32 %i27, -12
  %i29 = add i32 %i28, %i24
  %i30 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 1, !intel-tbaa !1381
  store i32 %i29, ptr %i30, align 4, !tbaa !1381
  %i31 = icmp slt i32 %i29, 1
  br i1 %i31, label %bb32, label %bb35

bb32:                                             ; preds = %bb18
  %i33 = add nsw i32 %i29, 12
  store i32 %i33, ptr %i30, align 4, !tbaa !1381
  %i34 = add nsw i32 %i27, -1
  br label %bb35

bb35:                                             ; preds = %bb32, %bb18
  %i36 = phi i32 [ %i33, %bb32 ], [ %i29, %bb18 ]
  %i37 = phi i32 [ %i34, %bb32 ], [ %i27, %bb18 ]
  %i38 = getelementptr inbounds [4 x [8 x i32]], ptr @_ZN11xercesc_2_7L9DATETIMESE, i64 0, i64 %i19, i64 0, !intel-tbaa !1692
  %i39 = load i32, ptr %i38, align 16, !tbaa !1692
  %i40 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 0, !intel-tbaa !1381
  %i41 = load i32, ptr %i40, align 4, !tbaa !1381
  %i42 = add nsw i32 %i39, %i41
  %i43 = add nsw i32 %i42, %i37
  %i44 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 0, !intel-tbaa !1381
  store i32 %i43, ptr %i44, align 4, !tbaa !1381
  %i45 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 5, !intel-tbaa !1381
  %i46 = load i32, ptr %i45, align 4, !tbaa !1381
  %i47 = tail call i64 @div(i32 noundef %i46, i32 noundef 60) #53
  %i48 = trunc i64 %i47 to i32
  %i49 = mul i32 %i48, -60
  %i50 = add nsw i32 %i49, %i46
  %i51 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 5, !intel-tbaa !1381
  store i32 %i50, ptr %i51, align 4, !tbaa !1381
  %i52 = icmp slt i32 %i50, 0
  br i1 %i52, label %bb53, label %bb56

bb53:                                             ; preds = %bb35
  %i54 = add nsw i32 %i50, 60
  store i32 %i54, ptr %i51, align 4, !tbaa !1381
  %i55 = add nsw i32 %i48, -1
  br label %bb56

bb56:                                             ; preds = %bb53, %bb35
  %i57 = phi i32 [ %i55, %bb53 ], [ %i48, %bb35 ]
  %i58 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 4, !intel-tbaa !1381
  %i59 = load i32, ptr %i58, align 4, !tbaa !1381
  %i60 = add nsw i32 %i59, %i57
  %i61 = tail call i64 @div(i32 noundef %i60, i32 noundef 60) #53
  %i62 = trunc i64 %i61 to i32
  %i63 = mul i32 %i62, -60
  %i64 = add nsw i32 %i63, %i60
  %i65 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 4, !intel-tbaa !1381
  store i32 %i64, ptr %i65, align 4, !tbaa !1381
  %i66 = icmp slt i32 %i64, 0
  br i1 %i66, label %bb67, label %bb70

bb67:                                             ; preds = %bb56
  %i68 = add nsw i32 %i64, 60
  store i32 %i68, ptr %i65, align 4, !tbaa !1381
  %i69 = add nsw i32 %i62, -1
  br label %bb70

bb70:                                             ; preds = %bb67, %bb56
  %i71 = phi i32 [ %i69, %bb67 ], [ %i62, %bb56 ]
  %i72 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 3, !intel-tbaa !1381
  %i73 = load i32, ptr %i72, align 4, !tbaa !1381
  %i74 = add nsw i32 %i73, %i71
  %i75 = tail call i64 @div(i32 noundef %i74, i32 noundef 24) #53
  %i76 = trunc i64 %i75 to i32
  %i77 = mul i32 %i76, -24
  %i78 = add nsw i32 %i77, %i74
  %i79 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 3, !intel-tbaa !1381
  store i32 %i78, ptr %i79, align 4, !tbaa !1381
  %i80 = icmp slt i32 %i78, 0
  br i1 %i80, label %bb81, label %bb84

bb81:                                             ; preds = %bb70
  %i82 = add nsw i32 %i78, 24
  store i32 %i82, ptr %i79, align 4, !tbaa !1381
  %i83 = add nsw i32 %i76, -1
  br label %bb84

bb84:                                             ; preds = %bb81, %bb70
  %i85 = phi i32 [ %i83, %bb81 ], [ %i76, %bb70 ]
  %i86 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 2, !intel-tbaa !1381
  %i87 = load i32, ptr %i86, align 4, !tbaa !1381
  %i88 = add nsw i32 %i87, 1
  %i89 = add nsw i32 %i88, %i85
  %i90 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 2, !intel-tbaa !1381
  store i32 %i89, ptr %i90, align 4, !tbaa !1381
  br label %bb91

bb91:                                             ; preds = %bb152, %bb84
  %i92 = phi i32 [ %i140, %bb152 ], [ %i89, %bb84 ]
  %i93 = phi i32 [ %i154, %bb152 ], [ %i36, %bb84 ]
  %i94 = phi i32 [ %i155, %bb152 ], [ %i43, %bb84 ]
  %i95 = and i32 %i93, -3
  %i96 = icmp eq i32 %i95, 4
  %i97 = icmp eq i32 %i95, 9
  %i98 = or i1 %i96, %i97
  br i1 %i98, label %bb111, label %bb99

bb99:                                             ; preds = %bb91
  %i100 = icmp eq i32 %i93, 2
  br i1 %i100, label %bb101, label %bb111

bb101:                                            ; preds = %bb99
  %i102 = and i32 %i94, 3
  %i103 = icmp eq i32 %i102, 0
  br i1 %i103, label %bb104, label %bb111

bb104:                                            ; preds = %bb101
  %i105 = srem i32 %i94, 100
  %i106 = icmp ne i32 %i105, 0
  %i107 = srem i32 %i94, 400
  %i108 = icmp eq i32 %i107, 0
  %i109 = or i1 %i106, %i108
  %i110 = select i1 %i109, i32 29, i32 28
  br label %bb111

bb111:                                            ; preds = %bb104, %bb101, %bb99, %bb91
  %i112 = phi i32 [ 30, %bb91 ], [ 31, %bb99 ], [ 28, %bb101 ], [ %i110, %bb104 ]
  %i113 = icmp slt i32 %i92, 1
  br i1 %i113, label %bb114, label %bb135

bb114:                                            ; preds = %bb111
  %i115 = add nsw i32 %i93, -1
  %i116 = and i32 %i115, -3
  %i117 = icmp eq i32 %i116, 4
  %i118 = icmp eq i32 %i116, 9
  %i119 = or i1 %i117, %i118
  br i1 %i119, label %bb132, label %bb120

bb120:                                            ; preds = %bb114
  %i121 = icmp eq i32 %i115, 2
  br i1 %i121, label %bb122, label %bb132

bb122:                                            ; preds = %bb120
  %i123 = and i32 %i94, 3
  %i124 = icmp eq i32 %i123, 0
  br i1 %i124, label %bb125, label %bb132

bb125:                                            ; preds = %bb122
  %i126 = srem i32 %i94, 100
  %i127 = icmp ne i32 %i126, 0
  %i128 = srem i32 %i94, 400
  %i129 = icmp eq i32 %i128, 0
  %i130 = or i1 %i127, %i129
  %i131 = select i1 %i130, i32 29, i32 28
  br label %bb132

bb132:                                            ; preds = %bb125, %bb122, %bb120, %bb114
  %i133 = phi i32 [ 30, %bb114 ], [ 31, %bb120 ], [ 28, %bb122 ], [ %i131, %bb125 ]
  %i134 = add nsw i32 %i133, %i92
  br label %bb139

bb135:                                            ; preds = %bb111
  %i136 = icmp ugt i32 %i92, %i112
  br i1 %i136, label %bb137, label %bb156

bb137:                                            ; preds = %bb135
  %i138 = sub nsw i32 %i92, %i112
  br label %bb139

bb139:                                            ; preds = %bb137, %bb132
  %i140 = phi i32 [ %i138, %bb137 ], [ %i134, %bb132 ]
  %i141 = phi i32 [ 1, %bb137 ], [ -1, %bb132 ]
  store i32 %i140, ptr %i90, align 4, !tbaa !1381
  %i142 = add nsw i32 %i141, %i93
  %i143 = add nsw i32 %i142, -1
  %i144 = tail call i64 @div(i32 noundef %i143, i32 noundef 12) #53
  %i145 = trunc i64 %i144 to i32
  %i146 = mul i32 %i145, -12
  %i147 = add i32 %i146, %i142
  store i32 %i147, ptr %i30, align 4, !tbaa !1381
  %i148 = icmp slt i32 %i147, 1
  br i1 %i148, label %bb149, label %bb152

bb149:                                            ; preds = %bb139
  %i150 = add nsw i32 %i147, 12
  store i32 %i150, ptr %i30, align 4, !tbaa !1381
  %i151 = add nsw i32 %i94, -1
  br label %bb152

bb152:                                            ; preds = %bb149, %bb139
  %i153 = phi i32 [ %i151, %bb149 ], [ %i94, %bb139 ]
  %i154 = phi i32 [ %i150, %bb149 ], [ %i147, %bb139 ]
  %i155 = add nsw i32 %i153, %i145
  store i32 %i155, ptr %i44, align 4, !tbaa !1381
  br label %bb91, !llvm.loop !1694

bb156:                                            ; preds = %bb135
  %i157 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 7, !intel-tbaa !1381
  store i32 1, ptr %i157, align 4, !tbaa !1381
  ret void
}

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare dso_local i64 @div(i32 noundef, i32 noundef) local_unnamed_addr #41

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_711XMLDateTimeD0Ev(ptr noundef nonnull align 8 dereferenceable(96) "intel_dtrans_func_index"="1" %arg) unnamed_addr #11 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1695 !_Intel.Devirt.Target !1380 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !1256
  %i1 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 8, !intel-tbaa !1683
  %i2 = load ptr, ptr %i1, align 8, !tbaa !1683
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb19, label %bb4

bb4:                                              ; preds = %bb
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 9, !intel-tbaa !1686
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1686
  %i7 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i6, i64 0, i32 0
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1256
  %i9 = tail call i1 @llvm.type.test(ptr %i8, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i9)
  %i10 = getelementptr inbounds ptr, ptr %i8, i64 3
  %i11 = load ptr, ptr %i10, align 8
  %i12 = icmp eq ptr %i11, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i12, label %bb15, label %bb13

bb13:                                             ; preds = %bb4
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb14 unwind label %bb16, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb14:                                             ; preds = %bb13
  unreachable

bb15:                                             ; preds = %bb4
  tail call void @_ZdlPv(ptr noundef nonnull %i2) #47
  br label %bb19

bb16:                                             ; preds = %bb13
  %i17 = landingpad { ptr, i32 }
          catch ptr null
  %i18 = extractvalue { ptr, i32 } %i17, 0
  tail call fastcc void @__clang_call_terminate(ptr %i18) #48
  unreachable

bb19:                                             ; preds = %bb15, %bb
  %i20 = getelementptr inbounds i8, ptr %arg, i64 -8, !intel-tbaa !1461
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1271
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 3
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i26, label %bb32, label %bb27

bb27:                                             ; preds = %bb19
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb28 unwind label %bb29, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb28:                                             ; preds = %bb27
  unreachable

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  tail call fastcc void @__clang_call_terminate(ptr %i31) #48
  unreachable

bb32:                                             ; preds = %bb19
  tail call void @_ZdlPv(ptr noundef nonnull %i20) #47
  ret void
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_711XMLDateTimeC2ERKS0_(ptr nocapture noundef nonnull align 8 dereferenceable(96) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(96) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #10 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1696 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !1256
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 5, !intel-tbaa !1697
  store i32 0, ptr %i2, align 8, !tbaa !1697
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 8, !intel-tbaa !1683
  store ptr null, ptr %i3, align 8, !tbaa !1683
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 9, !intel-tbaa !1686
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 9, !intel-tbaa !1686
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1686
  store ptr %i6, ptr %i4, align 8, !tbaa !1686
  br label %bb27

bb7:                                              ; preds = %bb27
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 6, !intel-tbaa !1685
  %i9 = load double, ptr %i8, align 8, !tbaa !1685
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 6, !intel-tbaa !1685
  store double %i9, ptr %i10, align 8, !tbaa !1685
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 7, !intel-tbaa !1684
  %i12 = load i8, ptr %i11, align 8, !tbaa !1684, !range !1451, !noundef !1452
  %i13 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 7, !intel-tbaa !1684
  store i8 %i12, ptr %i13, align 8, !tbaa !1684
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 2, i64 0, !intel-tbaa !1688
  %i15 = load i32, ptr %i14, align 8, !tbaa !1688
  %i16 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 0, !intel-tbaa !1688
  store i32 %i15, ptr %i16, align 8, !tbaa !1688
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 2, i64 1, !intel-tbaa !1688
  %i18 = load i32, ptr %i17, align 4, !tbaa !1688
  %i19 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 1, !intel-tbaa !1688
  store i32 %i18, ptr %i19, align 4, !tbaa !1688
  %i20 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 3, !intel-tbaa !1690
  %i21 = load i32, ptr %i20, align 8, !tbaa !1690
  %i22 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 3, !intel-tbaa !1690
  store i32 %i21, ptr %i22, align 8, !tbaa !1690
  %i23 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 4, !intel-tbaa !1689
  %i24 = load i32, ptr %i23, align 4, !tbaa !1689
  %i25 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 4, !intel-tbaa !1689
  store i32 %i24, ptr %i25, align 4, !tbaa !1689
  %i26 = icmp sgt i32 %i24, 0
  br i1 %i26, label %bb34, label %bb78

bb27:                                             ; preds = %bb27, %bb
  %i28 = phi i64 [ 0, %bb ], [ %i32, %bb27 ]
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 %i28, !intel-tbaa !1381
  %i30 = load i32, ptr %i29, align 4, !tbaa !1381
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 %i28, !intel-tbaa !1381
  store i32 %i30, ptr %i31, align 4, !tbaa !1381
  %i32 = add nuw nsw i64 %i28, 1
  %i33 = icmp eq i64 %i32, 8
  br i1 %i33, label %bb7, label %bb27, !llvm.loop !1698

bb34:                                             ; preds = %bb7
  %i35 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i6, i64 0, i32 0
  %i36 = load ptr, ptr %i35, align 8, !tbaa !1256
  %i37 = tail call i1 @llvm.type.test(ptr %i36, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i37)
  %i38 = getelementptr inbounds ptr, ptr %i36, i64 3
  %i39 = load ptr, ptr %i38, align 8
  %i40 = icmp eq ptr %i39, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i40, label %bb42, label %bb41

bb41:                                             ; preds = %bb34
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273
  unreachable

bb42:                                             ; preds = %bb34
  tail call void @_ZdlPv(ptr noundef null) #47
  %i43 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 5, !intel-tbaa !1697
  %i44 = load i32, ptr %i43, align 8, !tbaa !1697
  store i32 %i44, ptr %i2, align 8, !tbaa !1697
  %i45 = load ptr, ptr %i4, align 8, !tbaa !1686
  %i46 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i45, i64 0, i32 0
  %i47 = load ptr, ptr %i46, align 8, !tbaa !1256
  %i48 = tail call i1 @llvm.type.test(ptr %i47, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i48)
  %i49 = getelementptr inbounds ptr, ptr %i47, i64 2
  %i50 = load ptr, ptr %i49, align 8
  %i51 = icmp eq ptr %i50, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i51, label %bb52, label %bb69

bb52:                                             ; preds = %bb42
  %i53 = add nsw i32 %i44, 1
  %i54 = sext i32 %i53 to i64
  %i55 = shl nsw i64 %i54, 1
  %i56 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i55) #50
          to label %bb71 unwind label %bb57

bb57:                                             ; preds = %bb52
  %i58 = landingpad { ptr, i32 }
          catch ptr null
  %i59 = extractvalue { ptr, i32 } %i58, 0
  %i60 = tail call ptr @__cxa_begin_catch(ptr %i59) #47
  %i61 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i61, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
          to label %bb68 unwind label %bb62

bb62:                                             ; preds = %bb57
  %i63 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb64 unwind label %bb65

bb64:                                             ; preds = %bb62
  resume { ptr, i32 } %i63

bb65:                                             ; preds = %bb62
  %i66 = landingpad { ptr, i32 }
          catch ptr null
  %i67 = extractvalue { ptr, i32 } %i66, 0
  tail call fastcc void @__clang_call_terminate(ptr %i67) #48
  unreachable

bb68:                                             ; preds = %bb57
  unreachable

bb69:                                             ; preds = %bb42
  %i70 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb71:                                             ; preds = %bb52
  store ptr %i56, ptr %i3, align 8, !tbaa !1683
  %i72 = load i32, ptr %i25, align 4, !tbaa !1689
  %i73 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 8, !intel-tbaa !1683
  %i74 = load ptr, ptr %i73, align 8, !tbaa !1683
  %i75 = add nsw i32 %i72, 1
  %i76 = sext i32 %i75 to i64
  %i77 = shl nsw i64 %i76, 1
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i56, ptr align 2 %i74, i64 %i77, i1 false)
  br label %bb78

bb78:                                             ; preds = %bb71, %bb7
  ret void
}

; Function Attrs: mustprogress nofree nosync nounwind memory(argmem: readwrite) uwtable
define hidden fastcc void @_ZN11xercesc_2_711XMLDateTime9normalizeEv(ptr nocapture noundef nonnull align 8 dereferenceable(96) "intel_dtrans_func_index"="1" %arg) unnamed_addr #42 align 2 !intel.dtrans.func.type !1699 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 7, !intel-tbaa !1381
  %i1 = load i32, ptr %i, align 4, !tbaa !1381
  %i2 = icmp ult i32 %i1, 2
  br i1 %i2, label %bb125, label %bb3

bb3:                                              ; preds = %bb
  %i4 = icmp eq i32 %i1, 2
  %i5 = select i1 %i4, i32 -1, i32 1
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 1, !intel-tbaa !1381
  %i7 = load i32, ptr %i6, align 4, !tbaa !1381
  %i8 = add nsw i32 %i7, -1
  %i9 = tail call i64 @div(i32 noundef %i8, i32 noundef 12) #53
  %i10 = trunc i64 %i9 to i32
  %i11 = mul i32 %i10, -12
  %i12 = add i32 %i11, %i7
  store i32 %i12, ptr %i6, align 4, !tbaa !1381
  %i13 = icmp slt i32 %i12, 1
  br i1 %i13, label %bb14, label %bb17

bb14:                                             ; preds = %bb3
  %i15 = add nsw i32 %i12, 12
  store i32 %i15, ptr %i6, align 4, !tbaa !1381
  %i16 = add nsw i32 %i10, -1
  br label %bb17

bb17:                                             ; preds = %bb14, %bb3
  %i18 = phi i32 [ %i15, %bb14 ], [ %i12, %bb3 ]
  %i19 = phi i32 [ %i16, %bb14 ], [ %i10, %bb3 ]
  %i20 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 0, !intel-tbaa !1381
  %i21 = load i32, ptr %i20, align 8, !tbaa !1381
  %i22 = add nsw i32 %i21, %i19
  store i32 %i22, ptr %i20, align 8, !tbaa !1381
  %i23 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 4, !intel-tbaa !1381
  %i24 = load i32, ptr %i23, align 8, !tbaa !1381
  %i25 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 1, !intel-tbaa !1688
  %i26 = load i32, ptr %i25, align 4, !tbaa !1688
  %i27 = mul nsw i32 %i26, %i5
  %i28 = add nsw i32 %i27, %i24
  %i29 = tail call i64 @div(i32 noundef %i28, i32 noundef 60) #53
  %i30 = trunc i64 %i29 to i32
  %i31 = mul i32 %i30, -60
  %i32 = add nsw i32 %i31, %i28
  store i32 %i32, ptr %i23, align 8, !tbaa !1381
  %i33 = icmp slt i32 %i32, 0
  br i1 %i33, label %bb34, label %bb37

bb34:                                             ; preds = %bb17
  %i35 = add nsw i32 %i32, 60
  store i32 %i35, ptr %i23, align 8, !tbaa !1381
  %i36 = add nsw i32 %i30, -1
  br label %bb37

bb37:                                             ; preds = %bb34, %bb17
  %i38 = phi i32 [ %i36, %bb34 ], [ %i30, %bb17 ]
  %i39 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 3, !intel-tbaa !1381
  %i40 = load i32, ptr %i39, align 4, !tbaa !1381
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 0, !intel-tbaa !1688
  %i42 = load i32, ptr %i41, align 8, !tbaa !1688
  %i43 = mul nsw i32 %i42, %i5
  %i44 = add nsw i32 %i40, %i43
  %i45 = add nsw i32 %i44, %i38
  %i46 = tail call i64 @div(i32 noundef %i45, i32 noundef 24) #53
  %i47 = trunc i64 %i46 to i32
  %i48 = mul i32 %i47, -24
  %i49 = add nsw i32 %i48, %i45
  store i32 %i49, ptr %i39, align 4, !tbaa !1381
  %i50 = icmp slt i32 %i49, 0
  br i1 %i50, label %bb51, label %bb54

bb51:                                             ; preds = %bb37
  %i52 = add nsw i32 %i49, 24
  store i32 %i52, ptr %i39, align 4, !tbaa !1381
  %i53 = add nsw i32 %i47, -1
  br label %bb54

bb54:                                             ; preds = %bb51, %bb37
  %i55 = phi i32 [ %i53, %bb51 ], [ %i47, %bb37 ]
  %i56 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 2, !intel-tbaa !1381
  %i57 = load i32, ptr %i56, align 8, !tbaa !1381
  %i58 = add nsw i32 %i57, %i55
  store i32 %i58, ptr %i56, align 8, !tbaa !1381
  br label %bb59

bb59:                                             ; preds = %bb120, %bb54
  %i60 = phi i32 [ %i108, %bb120 ], [ %i58, %bb54 ]
  %i61 = phi i32 [ %i122, %bb120 ], [ %i18, %bb54 ]
  %i62 = phi i32 [ %i123, %bb120 ], [ %i22, %bb54 ]
  %i63 = and i32 %i61, -3
  %i64 = icmp eq i32 %i63, 4
  %i65 = icmp eq i32 %i63, 9
  %i66 = or i1 %i64, %i65
  br i1 %i66, label %bb79, label %bb67

bb67:                                             ; preds = %bb59
  %i68 = icmp eq i32 %i61, 2
  br i1 %i68, label %bb69, label %bb79

bb69:                                             ; preds = %bb67
  %i70 = and i32 %i62, 3
  %i71 = icmp eq i32 %i70, 0
  br i1 %i71, label %bb72, label %bb79

bb72:                                             ; preds = %bb69
  %i73 = srem i32 %i62, 100
  %i74 = icmp ne i32 %i73, 0
  %i75 = srem i32 %i62, 400
  %i76 = icmp eq i32 %i75, 0
  %i77 = or i1 %i74, %i76
  %i78 = select i1 %i77, i32 29, i32 28
  br label %bb79

bb79:                                             ; preds = %bb72, %bb69, %bb67, %bb59
  %i80 = phi i32 [ 30, %bb59 ], [ 31, %bb67 ], [ 28, %bb69 ], [ %i78, %bb72 ]
  %i81 = icmp slt i32 %i60, 1
  br i1 %i81, label %bb82, label %bb103

bb82:                                             ; preds = %bb79
  %i83 = add nsw i32 %i61, -1
  %i84 = and i32 %i83, -3
  %i85 = icmp eq i32 %i84, 4
  %i86 = icmp eq i32 %i84, 9
  %i87 = or i1 %i85, %i86
  br i1 %i87, label %bb100, label %bb88

bb88:                                             ; preds = %bb82
  %i89 = icmp eq i32 %i83, 2
  br i1 %i89, label %bb90, label %bb100

bb90:                                             ; preds = %bb88
  %i91 = and i32 %i62, 3
  %i92 = icmp eq i32 %i91, 0
  br i1 %i92, label %bb93, label %bb100

bb93:                                             ; preds = %bb90
  %i94 = srem i32 %i62, 100
  %i95 = icmp ne i32 %i94, 0
  %i96 = srem i32 %i62, 400
  %i97 = icmp eq i32 %i96, 0
  %i98 = or i1 %i95, %i97
  %i99 = select i1 %i98, i32 29, i32 28
  br label %bb100

bb100:                                            ; preds = %bb93, %bb90, %bb88, %bb82
  %i101 = phi i32 [ 30, %bb82 ], [ 31, %bb88 ], [ 28, %bb90 ], [ %i99, %bb93 ]
  %i102 = add nsw i32 %i101, %i60
  br label %bb107

bb103:                                            ; preds = %bb79
  %i104 = icmp ugt i32 %i60, %i80
  br i1 %i104, label %bb105, label %bb124

bb105:                                            ; preds = %bb103
  %i106 = sub nsw i32 %i60, %i80
  br label %bb107

bb107:                                            ; preds = %bb105, %bb100
  %i108 = phi i32 [ %i106, %bb105 ], [ %i102, %bb100 ]
  %i109 = phi i32 [ 1, %bb105 ], [ -1, %bb100 ]
  store i32 %i108, ptr %i56, align 8, !tbaa !1381
  %i110 = add nsw i32 %i109, %i61
  %i111 = add nsw i32 %i110, -1
  %i112 = tail call i64 @div(i32 noundef %i111, i32 noundef 12) #53
  %i113 = trunc i64 %i112 to i32
  %i114 = mul i32 %i113, -12
  %i115 = add i32 %i114, %i110
  store i32 %i115, ptr %i6, align 4, !tbaa !1381
  %i116 = icmp slt i32 %i115, 1
  br i1 %i116, label %bb117, label %bb120

bb117:                                            ; preds = %bb107
  %i118 = add nsw i32 %i115, 12
  store i32 %i118, ptr %i6, align 4, !tbaa !1381
  %i119 = add nsw i32 %i62, -1
  br label %bb120

bb120:                                            ; preds = %bb117, %bb107
  %i121 = phi i32 [ %i119, %bb117 ], [ %i62, %bb107 ]
  %i122 = phi i32 [ %i118, %bb117 ], [ %i115, %bb107 ]
  %i123 = add nsw i32 %i121, %i113
  store i32 %i123, ptr %i20, align 8, !tbaa !1381
  br label %bb59, !llvm.loop !1700

bb124:                                            ; preds = %bb103
  store i32 1, ptr %i, align 4, !tbaa !1381
  br label %bb125

bb125:                                            ; preds = %bb124, %bb
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime13compareResultEPKS0_S2_bi(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1, i1 noundef zeroext %arg2, i32 noundef %arg3) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1701 {
bb:
  %i = alloca %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", align 8
  call void @llvm.lifetime.start.p0(i64 96, ptr nonnull %i) #47
  %i4 = select i1 %arg2, ptr %arg, ptr %arg1
  call fastcc void @_ZN11xercesc_2_711XMLDateTimeC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(96) %i, ptr noundef nonnull align 8 dereferenceable(96) %i4)
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 2, i64 0, !intel-tbaa !1688
  store i32 14, ptr %i5, align 8, !tbaa !1688
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 2, i64 1, !intel-tbaa !1688
  store i32 0, ptr %i6, align 4, !tbaa !1688
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 1, i64 7, !intel-tbaa !1381
  store i32 %arg3, ptr %i7, align 4, !tbaa !1381
  call fastcc void @_ZN11xercesc_2_711XMLDateTime9normalizeEv(ptr noundef nonnull align 8 dereferenceable(96) %i)
  br i1 %arg2, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef nonnull %i, ptr noundef %arg1)
          to label %bb12 unwind label %bb34, !range !1370

bb10:                                             ; preds = %bb
  %i11 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef %arg, ptr noundef nonnull %i)
          to label %bb12 unwind label %bb34, !range !1370

bb12:                                             ; preds = %bb10, %bb8
  %i13 = phi i32 [ %i9, %bb8 ], [ %i11, %bb10 ]
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i14, align 8, !tbaa !1256
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 8, !intel-tbaa !1683
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1683
  %i17 = icmp eq ptr %i16, null
  br i1 %i17, label %bb33, label %bb18

bb18:                                             ; preds = %bb12
  %i19 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 9, !intel-tbaa !1686
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1686
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i20, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 3
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i26, label %bb29, label %bb27

bb27:                                             ; preds = %bb18
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb28 unwind label %bb30, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb28:                                             ; preds = %bb27
  unreachable

bb29:                                             ; preds = %bb18
  tail call void @_ZdlPv(ptr noundef nonnull %i16) #47
  br label %bb33

bb30:                                             ; preds = %bb27
  %i31 = landingpad { ptr, i32 }
          catch ptr null
  %i32 = extractvalue { ptr, i32 } %i31, 0
  tail call fastcc void @__clang_call_terminate(ptr %i32) #48
  unreachable

bb33:                                             ; preds = %bb29, %bb12
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i) #47
  ret i32 %i13

bb34:                                             ; preds = %bb10, %bb8
  %i35 = landingpad { ptr, i32 }
          cleanup
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i36, align 8, !tbaa !1256
  %i37 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 8, !intel-tbaa !1683
  %i38 = load ptr, ptr %i37, align 8, !tbaa !1683
  %i39 = icmp eq ptr %i38, null
  br i1 %i39, label %bb55, label %bb40

bb40:                                             ; preds = %bb34
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %i, i64 0, i32 9, !intel-tbaa !1686
  %i42 = load ptr, ptr %i41, align 8, !tbaa !1686
  %i43 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i42, i64 0, i32 0
  %i44 = load ptr, ptr %i43, align 8, !tbaa !1256
  %i45 = tail call i1 @llvm.type.test(ptr %i44, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i45)
  %i46 = getelementptr inbounds ptr, ptr %i44, i64 3
  %i47 = load ptr, ptr %i46, align 8
  %i48 = icmp eq ptr %i47, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i48, label %bb51, label %bb49

bb49:                                             ; preds = %bb40
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb50 unwind label %bb52, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb50:                                             ; preds = %bb49
  unreachable

bb51:                                             ; preds = %bb40
  tail call void @_ZdlPv(ptr noundef nonnull %i38) #47
  br label %bb55

bb52:                                             ; preds = %bb49
  %i53 = landingpad { ptr, i32 }
          catch ptr null
  %i54 = extractvalue { ptr, i32 } %i53, 0
  tail call fastcc void @__clang_call_terminate(ptr %i54) #48
  unreachable

bb55:                                             ; preds = %bb51, %bb34
  call void @llvm.lifetime.end.p0(i64 96, ptr nonnull %i) #47
  resume { ptr, i32 } %i35
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv() #43 align 2

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) unnamed_addr #11 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1702 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i64 0, i64 2), ptr %i, align 8, !tbaa !1256
  %i1 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5, !intel-tbaa !1270
  %i2 = load ptr, ptr %i1, align 8, !tbaa !1270
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !1269
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1269
  %i5 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i2, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1256
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = getelementptr inbounds ptr, ptr %i6, i64 3
  %i9 = load ptr, ptr %i8, align 8
  %i10 = icmp eq ptr %i9, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i10, label %bb13, label %bb11

bb11:                                             ; preds = %bb
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb12 unwind label %bb26, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb12:                                             ; preds = %bb11
  unreachable

bb13:                                             ; preds = %bb
  tail call void @_ZdlPv(ptr noundef %i4) #47
  %i14 = load ptr, ptr %i1, align 8, !tbaa !1270
  %i15 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 2, !intel-tbaa !1267
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1267
  %i17 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 8, !tbaa !1256
  %i19 = tail call i1 @llvm.type.test(ptr %i18, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i19)
  %i20 = getelementptr inbounds ptr, ptr %i18, i64 3
  %i21 = load ptr, ptr %i20, align 8
  %i22 = icmp eq ptr %i21, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i22, label %bb25, label %bb23

bb23:                                             ; preds = %bb13
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison)
          to label %bb24 unwind label %bb26, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb24:                                             ; preds = %bb23
  unreachable

bb25:                                             ; preds = %bb13
  tail call void @_ZdlPv(ptr noundef %i16) #47
  ret void

bb26:                                             ; preds = %bb23, %bb11
  %i27 = landingpad { ptr, i32 }
          catch ptr null
  %i28 = extractvalue { ptr, i32 } %i27, 0
  tail call fastcc void @__clang_call_terminate(ptr %i28) #48
  unreachable
}

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1704 {
bb:
  %i = alloca [2048 x i16], align 16, !intel_dtrans_type !1705
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 1, !intel-tbaa !1259
  store i32 %arg1, ptr %i2, align 8, !tbaa !1259
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i) #47
  %i3 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv()
  %i4 = getelementptr inbounds [2048 x i16], ptr %i, i64 0, i64 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i3, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !1256
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(8) %i3, i32 noundef %arg1, ptr noundef nonnull %i4, i32 noundef 2047), !intel_dtrans_type !1706, !_Intel.Devirt.Call !1273
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5
  %i10 = load ptr, ptr %i9, align 8, !tbaa !1270
  br i1 %i8, label %bb46, label %bb11

bb11:                                             ; preds = %bb11, %bb
  %i12 = phi ptr [ %i13, %bb11 ], [ @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, %bb ]
  %i13 = getelementptr inbounds i16, ptr %i12, i64 1
  %i14 = getelementptr [23 x i16], ptr %i13, i64 0, i64 0
  %i15 = load i16, ptr %i14, align 2, !tbaa !1286
  %i16 = icmp eq i16 %i15, 0
  br i1 %i16, label %bb17, label %bb11, !llvm.loop !1707

bb17:                                             ; preds = %bb11
  %i18 = ptrtoint ptr %i13 to i64
  %i19 = add i64 %i18, add (i64 sub (i64 0, i64 ptrtoint (ptr @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE to i64)), i64 2)
  %i20 = and i64 %i19, 8589934590
  %i21 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i23)
  %i24 = getelementptr inbounds ptr, ptr %i22, i64 2
  %i25 = load ptr, ptr %i24, align 8
  %i26 = icmp eq ptr %i25, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i26, label %bb27, label %bb42

bb27:                                             ; preds = %bb17
  %i28 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i20) #50
          to label %bb44 unwind label %bb29

bb29:                                             ; preds = %bb27
  %i30 = landingpad { ptr, i32 }
          catch ptr null
  %i31 = extractvalue { ptr, i32 } %i30, 0
  %i32 = call ptr @__cxa_begin_catch(ptr %i31) #47
  %i33 = call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i33, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  call fastcc void @__clang_call_terminate(ptr %i40) #48
  unreachable

bb41:                                             ; preds = %bb29
  unreachable

bb42:                                             ; preds = %bb17
  %i43 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb44:                                             ; preds = %bb27
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i28, ptr nonnull align 16 @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, i64 %i20, i1 false)
  %i45 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !1269
  store ptr %i28, ptr %i45, align 8, !tbaa !1269
  br label %bb85

bb46:                                             ; preds = %bb
  %i47 = load i16, ptr %i4, align 16, !tbaa !1286
  %i48 = icmp eq i16 %i47, 0
  br i1 %i48, label %bb60, label %bb49

bb49:                                             ; preds = %bb49, %bb46
  %i50 = phi ptr [ %i51, %bb49 ], [ %i4, %bb46 ]
  %i51 = getelementptr inbounds i16, ptr %i50, i64 1
  %i52 = load i16, ptr %i51, align 2, !tbaa !1286
  %i53 = icmp eq i16 %i52, 0
  br i1 %i53, label %bb54, label %bb49, !llvm.loop !1707

bb54:                                             ; preds = %bb49
  %i55 = ptrtoint ptr %i51 to i64
  %i56 = ptrtoint ptr %i to i64
  %i57 = sub i64 %i55, %i56
  %i58 = add i64 %i57, 2
  %i59 = and i64 %i58, 8589934590
  br label %bb60

bb60:                                             ; preds = %bb54, %bb46
  %i61 = phi i64 [ %i59, %bb54 ], [ 2, %bb46 ]
  %i62 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i63 = load ptr, ptr %i62, align 8, !tbaa !1256
  %i64 = call i1 @llvm.type.test(ptr %i63, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i64)
  %i65 = getelementptr inbounds ptr, ptr %i63, i64 2
  %i66 = load ptr, ptr %i65, align 8
  %i67 = icmp eq ptr %i66, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i67, label %bb68, label %bb81

bb68:                                             ; preds = %bb60
  %i69 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i61) #50
          to label %bb83 unwind label %bb70

bb70:                                             ; preds = %bb68
  %i71 = landingpad { ptr, i32 }
          catch ptr null
  %i72 = extractvalue { ptr, i32 } %i71, 0
  %i73 = call ptr @__cxa_begin_catch(ptr %i72) #47
  %i74 = call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i74, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  call fastcc void @__clang_call_terminate(ptr %i79) #48
  unreachable

bb80:                                             ; preds = %bb70
  unreachable

bb81:                                             ; preds = %bb60
  %i82 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb83:                                             ; preds = %bb68
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i69, ptr nonnull align 16 %i4, i64 %i61, i1 false)
  %i84 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !1269
  store ptr %i69, ptr %i84, align 8, !tbaa !1269
  br label %bb85

bb85:                                             ; preds = %bb83, %bb44
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i) #47
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv() unnamed_addr #14 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1708 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1709
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
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex", ptr %i7, i64 0, i32 0, !intel-tbaa !1711
  store ptr null, ptr %i8, align 8, !tbaa !1711
  store ptr %i7, ptr @_ZN11xercesc_2_7L9sMsgMutexE, align 8, !tbaa !1713
  store ptr @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv, ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, align 8, !tbaa !1715
  %i9 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  %i10 = icmp eq ptr %i9, null
  %i11 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 2), align 8
  %i12 = icmp eq ptr %i11, null
  %i13 = select i1 %i10, i1 %i12, i1 false
  br i1 %i13, label %bb14, label %bb19

bb14:                                             ; preds = %bb6
  %i15 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  store ptr %i15, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  store ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  %i16 = icmp eq ptr %i15, null
  br i1 %i16, label %bb19, label %bb17

bb17:                                             ; preds = %bb14
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i15, i64 0, i32 2, !intel-tbaa !1721
  store ptr @_ZN11xercesc_2_7L15msgMutexCleanupE, ptr %i18, align 8, !tbaa !1721
  br label %bb19

bb19:                                             ; preds = %bb17, %bb14, %bb6
  store i1 true, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br label %bb20

bb20:                                             ; preds = %bb19, %bb4
  %i21 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1709
  %i22 = icmp eq ptr %i21, null
  br i1 %i22, label %bb23, label %bb42

bb23:                                             ; preds = %bb20, %bb2
  %i24 = tail call fastcc noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef nonnull @_ZN11xercesc_2_76XMLUni14fgExceptDomainE)
  store ptr %i24, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1709
  %i25 = icmp eq ptr %i24, null
  br i1 %i25, label %bb26, label %bb31

bb26:                                             ; preds = %bb23
  %i27 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1618
  %i28 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i27, i64 0, i32 0
  %i29 = load ptr, ptr %i28, align 8, !tbaa !1256
  %i30 = tail call i1 @llvm.type.test(ptr %i29, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i30)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1620, !_Intel.Devirt.Call !1273
  unreachable

bb31:                                             ; preds = %bb23
  store ptr @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv, ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, align 8, !tbaa !1715
  %i32 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  %i33 = icmp eq ptr %i32, null
  %i34 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 2), align 8
  %i35 = icmp eq ptr %i34, null
  %i36 = select i1 %i33, i1 %i35, i1 false
  br i1 %i36, label %bb37, label %bb42

bb37:                                             ; preds = %bb31
  %i38 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  store ptr %i38, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  %i39 = icmp eq ptr %i38, null
  br i1 %i39, label %bb42, label %bb40

bb40:                                             ; preds = %bb37
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i38, i64 0, i32 2, !intel-tbaa !1721
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE, ptr %i41, align 8, !tbaa !1721
  br label %bb42

bb42:                                             ; preds = %bb40, %bb37, %bb31, %bb20
  %i43 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !1709
  br label %bb44

bb44:                                             ; preds = %bb42, %bb
  %i45 = phi ptr [ %i43, %bb42 ], [ %i, %bb ]
  ret ptr %i45
}

; Function Attrs: nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv() #21 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_79XMLString13replaceTokensEPtjPKtS3_S3_S3_PNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef readonly "intel_dtrans_func_index"="2" %arg2, ptr noundef readonly "intel_dtrans_func_index"="3" %arg3, ptr noundef readonly "intel_dtrans_func_index"="4" %arg4, ptr noundef readonly "intel_dtrans_func_index"="5" %arg5, ptr noundef readonly "intel_dtrans_func_index"="6" %arg6) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1722 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb46, label %bb7

bb7:                                              ; preds = %bb
  %i8 = load i16, ptr %arg, align 2, !tbaa !1286
  %i9 = icmp eq i16 %i8, 0
  br i1 %i9, label %bb21, label %bb10

bb10:                                             ; preds = %bb10, %bb7
  %i11 = phi ptr [ %i12, %bb10 ], [ %arg, %bb7 ]
  %i12 = getelementptr inbounds i16, ptr %i11, i64 1
  %i13 = load i16, ptr %i12, align 2, !tbaa !1286
  %i14 = icmp eq i16 %i13, 0
  br i1 %i14, label %bb15, label %bb10, !llvm.loop !1288

bb15:                                             ; preds = %bb10
  %i16 = ptrtoint ptr %i12 to i64
  %i17 = ptrtoint ptr %arg to i64
  %i18 = sub i64 %i16, %i17
  %i19 = add i64 %i18, 2
  %i20 = and i64 %i19, 8589934590
  br label %bb21

bb21:                                             ; preds = %bb15, %bb7
  %i22 = phi i64 [ %i20, %bb15 ], [ 2, %bb7 ]
  %i23 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg6, i64 0, i32 0
  %i24 = load ptr, ptr %i23, align 8, !tbaa !1256
  %i25 = tail call i1 @llvm.type.test(ptr %i24, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i25)
  %i26 = getelementptr inbounds ptr, ptr %i24, i64 2
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i28, label %bb29, label %bb43

bb29:                                             ; preds = %bb21
  %i30 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i22) #50
          to label %bb45 unwind label %bb31

bb31:                                             ; preds = %bb29
  %i32 = landingpad { ptr, i32 }
          catch ptr null
  %i33 = extractvalue { ptr, i32 } %i32, 0
  %i34 = tail call ptr @__cxa_begin_catch(ptr %i33) #47
  %i35 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i35, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i41) #48
  unreachable

bb42:                                             ; preds = %bb31
  unreachable

bb43:                                             ; preds = %bb21
  %i44 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb45:                                             ; preds = %bb29
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 2 %i30, ptr nonnull align 2 %arg, i64 %i22, i1 false)
  br label %bb46

bb46:                                             ; preds = %bb45, %bb
  %i47 = phi ptr [ %i30, %bb45 ], [ null, %bb ]
  %i48 = load i16, ptr %i47, align 2, !tbaa !1286
  %i49 = icmp ne i16 %i48, 0
  %i50 = icmp ne i32 %arg1, 0
  %i51 = and i1 %i49, %i50
  br i1 %i51, label %bb52, label %bb126

bb52:                                             ; preds = %bb46
  %i53 = zext i32 %arg1 to i64
  br label %bb54

bb54:                                             ; preds = %bb119, %bb52
  %i55 = phi i16 [ %i122, %bb119 ], [ %i48, %bb52 ]
  %i56 = phi ptr [ %i121, %bb119 ], [ %i47, %bb52 ]
  %i57 = phi i32 [ %i120, %bb119 ], [ 0, %bb52 ]
  %i58 = icmp eq i16 %i55, 123
  br i1 %i58, label %bb76, label %bb59

bb59:                                             ; preds = %bb54
  %i60 = zext i32 %i57 to i64
  br label %bb61

bb61:                                             ; preds = %bb61, %bb59
  %i62 = phi i64 [ %i60, %bb59 ], [ %i66, %bb61 ]
  %i63 = phi ptr [ %i56, %bb59 ], [ %i65, %bb61 ]
  %i64 = phi i16 [ %i55, %bb59 ], [ %i68, %bb61 ]
  %i65 = getelementptr inbounds i16, ptr %i63, i64 1
  %i66 = add nuw nsw i64 %i62, 1
  %i67 = getelementptr inbounds i16, ptr %arg, i64 %i62
  store i16 %i64, ptr %i67, align 2, !tbaa !1286
  %i68 = load i16, ptr %i65, align 2, !tbaa !1286
  %i69 = icmp eq i16 %i68, 123
  %i70 = icmp uge i64 %i66, %i53
  %i71 = select i1 %i69, i1 true, i1 %i70
  %i72 = icmp eq i16 %i68, 0
  %i73 = or i1 %i71, %i72
  br i1 %i73, label %bb74, label %bb61, !llvm.loop !1723

bb74:                                             ; preds = %bb61
  %i75 = trunc i64 %i66 to i32
  br i1 %i69, label %bb76, label %bb126

bb76:                                             ; preds = %bb74, %bb54
  %i77 = phi ptr [ %i65, %bb74 ], [ %i56, %bb54 ]
  %i78 = phi i32 [ %i75, %bb74 ], [ %i57, %bb54 ]
  %i79 = getelementptr inbounds i16, ptr %i77, i64 1, !intel-tbaa !1286
  %i80 = load i16, ptr %i79, align 2, !tbaa !1286
  %i81 = and i16 %i80, -4
  %i82 = icmp eq i16 %i81, 48
  br i1 %i82, label %bb83, label %bb113

bb83:                                             ; preds = %bb76
  %i84 = getelementptr inbounds i16, ptr %i77, i64 2, !intel-tbaa !1286
  %i85 = load i16, ptr %i84, align 2, !tbaa !1286
  %i86 = icmp eq i16 %i85, 125
  br i1 %i86, label %bb87, label %bb113

bb87:                                             ; preds = %bb83
  %i88 = getelementptr inbounds i16, ptr %i77, i64 3, !intel-tbaa !1286
  switch i16 %i80, label %bb89 [
    i16 48, label %bb92
    i16 49, label %bb90
    i16 50, label %bb91
  ]

bb89:                                             ; preds = %bb87
  br label %bb92

bb90:                                             ; preds = %bb87
  br label %bb92

bb91:                                             ; preds = %bb87
  br label %bb92

bb92:                                             ; preds = %bb91, %bb90, %bb89, %bb87
  %i93 = phi ptr [ %arg5, %bb89 ], [ %arg2, %bb87 ], [ %arg3, %bb90 ], [ %arg4, %bb91 ]
  %i94 = icmp eq ptr %i93, null
  %i95 = select i1 %i94, ptr @_ZN11xercesc_2_7L8gNullStrE, ptr %i93
  %i96 = load i16, ptr %i95, align 2, !tbaa !1286
  %i97 = icmp ne i16 %i96, 0
  %i98 = icmp ult i32 %i78, %arg1
  %i99 = select i1 %i97, i1 %i98, i1 false
  br i1 %i99, label %bb100, label %bb119

bb100:                                            ; preds = %bb92
  %i101 = zext i32 %i78 to i64
  br label %bb102

bb102:                                            ; preds = %bb102, %bb100
  %i103 = phi i64 [ %i101, %bb100 ], [ %i107, %bb102 ]
  %i104 = phi i16 [ %i96, %bb100 ], [ %i109, %bb102 ]
  %i105 = phi ptr [ %i95, %bb100 ], [ %i106, %bb102 ]
  %i106 = getelementptr inbounds i16, ptr %i105, i64 1
  %i107 = add nuw nsw i64 %i103, 1
  %i108 = getelementptr inbounds i16, ptr %arg, i64 %i103
  store i16 %i104, ptr %i108, align 2, !tbaa !1286
  %i109 = load i16, ptr %i106, align 2, !tbaa !1286
  %i110 = icmp ne i16 %i109, 0
  %i111 = icmp ult i64 %i107, %i53
  %i112 = select i1 %i110, i1 %i111, i1 false
  br i1 %i112, label %bb102, label %bb117, !llvm.loop !1724

bb113:                                            ; preds = %bb83, %bb76
  %i114 = add i32 %i78, 1
  %i115 = zext i32 %i78 to i64
  %i116 = getelementptr inbounds i16, ptr %arg, i64 %i115
  store i16 123, ptr %i116, align 2, !tbaa !1286
  br label %bb119

bb117:                                            ; preds = %bb102
  %i118 = trunc i64 %i107 to i32
  br label %bb119

bb119:                                            ; preds = %bb117, %bb113, %bb92
  %i120 = phi i32 [ %i114, %bb113 ], [ %i78, %bb92 ], [ %i118, %bb117 ]
  %i121 = phi ptr [ %i79, %bb113 ], [ %i88, %bb92 ], [ %i88, %bb117 ]
  %i122 = load i16, ptr %i121, align 2, !tbaa !1286
  %i123 = icmp ne i16 %i122, 0
  %i124 = icmp ult i32 %i120, %arg1
  %i125 = select i1 %i123, i1 %i124, i1 false
  br i1 %i125, label %bb54, label %bb126, !llvm.loop !1725

bb126:                                            ; preds = %bb119, %bb74, %bb46
  %i127 = phi i32 [ 0, %bb46 ], [ %i120, %bb119 ], [ %i75, %bb74 ]
  %i128 = zext i32 %i127 to i64
  %i129 = getelementptr inbounds i16, ptr %arg, i64 %i128
  store i16 0, ptr %i129, align 2, !tbaa !1286
  %i130 = icmp eq ptr %arg6, null
  br i1 %i130, label %bb141, label %bb131

bb131:                                            ; preds = %bb126
  %i132 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg6, i64 0, i32 0
  %i133 = load ptr, ptr %i132, align 8, !tbaa !1256
  %i134 = tail call i1 @llvm.type.test(ptr %i133, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i134)
  %i135 = getelementptr inbounds ptr, ptr %i133, i64 3
  %i136 = load ptr, ptr %i135, align 8
  %i137 = icmp eq ptr %i136, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i137, label %bb140, label %bb138

bb138:                                            ; preds = %bb131
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb139 unwind label %bb142, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb139:                                            ; preds = %bb138
  unreachable

bb140:                                            ; preds = %bb131
  tail call void @_ZdlPv(ptr noundef nonnull %i47) #47
  br label %bb145

bb141:                                            ; preds = %bb126
  tail call void @_ZdaPv(ptr noundef nonnull %i47) #54
  br label %bb145

bb142:                                            ; preds = %bb138
  %i143 = landingpad { ptr, i32 }
          catch ptr null
  %i144 = extractvalue { ptr, i32 } %i143, 0
  tail call fastcc void @__clang_call_terminate(ptr %i144) #48
  unreachable

bb145:                                            ; preds = %bb141, %bb140
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLValidator15reinitMsgLoaderEv() #43 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #14 align 2 !intel.dtrans.func.type !1726 {
bb:
  %i = alloca ptr, align 8, !intel_dtrans_type !817
  %i2 = alloca [1024 x i16], align 16, !intel_dtrans_type !1727
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1728
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1728
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i4, i64 0, i32 25, !intel-tbaa !1732
  %i6 = load i32, ptr %i5, align 4, !tbaa !1732
  %i7 = add nsw i32 %i6, 1
  store i32 %i7, ptr %i5, align 4, !tbaa !1732
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 2, !intel-tbaa !1733
  %i9 = load ptr, ptr %i8, align 8, !tbaa !1733
  %i10 = icmp eq ptr %i9, null
  br i1 %i10, label %bb48, label %bb11

bb11:                                             ; preds = %bb
  call void @llvm.lifetime.start.p0(i64 2048, ptr nonnull %i2) #47
  %i12 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L12getMsgLoaderEv()
  %i13 = getelementptr inbounds [1024 x i16], ptr %i2, i64 0, i64 0
  %i14 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i12, i64 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !1256
  %i16 = tail call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(8) %i12, i32 noundef %arg1, ptr noundef nonnull %i13, i32 noundef 1023), !intel_dtrans_type !1706, !_Intel.Devirt.Call !1273
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 3, !intel-tbaa !1734
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1734
  %i20 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i19, i64 0, i32 6, !intel-tbaa !1627
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1627
  %i22 = icmp eq ptr %i21, null
  br i1 %i22, label %bb37, label %bb23

bb23:                                             ; preds = %bb11
  %i24 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i19, i64 0, i32 2, !intel-tbaa !1623
  %i25 = load ptr, ptr %i24, align 8, !tbaa !1623
  %i26 = icmp eq ptr %i25, null
  br i1 %i26, label %bb37, label %bb27

bb27:                                             ; preds = %bb23
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #47
  %i28 = call fastcc noundef ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr noundef nonnull align 8 dereferenceable(80) %i19, ptr noundef nonnull align 8 dereferenceable(8) %i)
  %i29 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 22, !intel-tbaa !1735
  %i30 = load ptr, ptr %i29, align 8, !tbaa !1735
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 11, !intel-tbaa !1747
  %i32 = load ptr, ptr %i31, align 8, !tbaa !1747
  %i33 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 6, !intel-tbaa !1748
  %i34 = load i64, ptr %i33, align 8, !tbaa !1748
  %i35 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i28, i64 0, i32 5, !intel-tbaa !1749
  %i36 = load i64, ptr %i35, align 8, !tbaa !1749
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #47
  br label %bb37

bb37:                                             ; preds = %bb27, %bb23, %bb11
  %i38 = phi i64 [ %i36, %bb27 ], [ 0, %bb23 ], [ 0, %bb11 ]
  %i39 = phi i64 [ %i34, %bb27 ], [ 0, %bb23 ], [ 0, %bb11 ]
  %i40 = phi ptr [ %i32, %bb27 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb23 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb11 ]
  %i41 = phi ptr [ %i30, %bb27 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb23 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb11 ]
  %i42 = load ptr, ptr %i8, align 8, !tbaa !1733
  %i43 = getelementptr %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter", ptr %i42, i64 0, i32 0
  %i44 = load ptr, ptr %i43, align 8, !tbaa !1256
  %i45 = call i1 @llvm.type.test(ptr %i44, metadata !"_ZTSN11xercesc_2_716XMLErrorReporterE")
  call void @llvm.assume(i1 %i45)
  %i46 = getelementptr inbounds ptr, ptr %i44, i64 2
  %i47 = load ptr, ptr %i46, align 8
  call void %i47(ptr noundef nonnull align 8 dereferenceable(8) %i42, i32 noundef %arg1, ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, i32 noundef 1, ptr noundef nonnull %i13, ptr noundef %i41, ptr noundef %i40, i64 noundef %i39, i64 noundef %i38), !intel_dtrans_type !1750
  call void @llvm.lifetime.end.p0(i64 2048, ptr nonnull %i2) #47
  br label %bb48

bb48:                                             ; preds = %bb37, %bb
  %i49 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1728
  %i50 = load ptr, ptr %i49, align 8, !tbaa !1728
  %i51 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 6, !intel-tbaa !1751
  %i52 = load i8, ptr %i51, align 1, !tbaa !1751, !range !1451, !noundef !1452
  %i53 = icmp eq i8 %i52, 0
  br i1 %i53, label %bb64, label %bb54

bb54:                                             ; preds = %bb48
  %i55 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 5, !intel-tbaa !1752
  %i56 = load i8, ptr %i55, align 1, !tbaa !1752, !range !1451, !noundef !1452
  %i57 = icmp eq i8 %i56, 0
  br i1 %i57, label %bb64, label %bb58

bb58:                                             ; preds = %bb54
  %i59 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i50, i64 0, i32 7, !intel-tbaa !1753
  %i60 = load i8, ptr %i59, align 1, !tbaa !1753, !range !1451, !noundef !1452
  %i61 = icmp eq i8 %i60, 0
  br i1 %i61, label %bb62, label %bb64

bb62:                                             ; preds = %bb58
  %i63 = call ptr @__cxa_allocate_exception(i64 4) #47
  store i32 %arg1, ptr %i63, align 16, !tbaa !1754
  call void @__cxa_throw(ptr nonnull %i63, ptr nonnull @_ZTIN11xercesc_2_78XMLValid5CodesE, ptr null) #51
  unreachable

bb64:                                             ; preds = %bb58, %bb54, %bb48
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_7L12getMsgLoaderEv() unnamed_addr #14 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1756 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3653, align 8, !tbaa !1709
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb46

bb2:                                              ; preds = %bb
  %i3 = load ptr, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3656, align 8, !tbaa !1713
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb5, label %bb25

bb5:                                              ; preds = %bb2
  %i6 = load ptr, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3656, align 8, !tbaa !1713
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb8, label %bb22

bb8:                                              ; preds = %bb5
  %i9 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i10 = tail call fastcc noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 8, ptr noundef %i9)
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex", ptr %i10, i64 0, i32 0, !intel-tbaa !1711
  store ptr null, ptr %i11, align 8, !tbaa !1711
  store ptr %i10, ptr @_ZN11xercesc_2_7L9sMsgMutexE.3656, align 8, !tbaa !1713
  store ptr @_ZN11xercesc_2_712XMLValidator14reinitMsgMutexEv, ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, align 8, !tbaa !1715
  %i12 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  %i13 = icmp eq ptr %i12, null
  %i14 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 2), align 8
  %i15 = icmp eq ptr %i14, null
  %i16 = select i1 %i13, i1 %i15, i1 false
  br i1 %i16, label %bb17, label %bb22

bb17:                                             ; preds = %bb8
  %i18 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  store ptr %i18, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, i64 0, i32 1), align 8, !tbaa !1719
  store ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  %i19 = icmp eq ptr %i18, null
  br i1 %i19, label %bb22, label %bb20

bb20:                                             ; preds = %bb17
  %i21 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i18, i64 0, i32 2, !intel-tbaa !1721
  store ptr @_ZN11xercesc_2_7L21validatorMutexCleanupE, ptr %i21, align 8, !tbaa !1721
  br label %bb22

bb22:                                             ; preds = %bb20, %bb17, %bb8, %bb5
  %i23 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3653, align 8, !tbaa !1709
  %i24 = icmp eq ptr %i23, null
  br i1 %i24, label %bb25, label %bb44

bb25:                                             ; preds = %bb22, %bb2
  %i26 = tail call fastcc noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE)
  store ptr %i26, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3653, align 8, !tbaa !1709
  %i27 = icmp eq ptr %i26, null
  br i1 %i27, label %bb28, label %bb33

bb28:                                             ; preds = %bb25
  %i29 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !1618
  %i30 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i29, i64 0, i32 0
  %i31 = load ptr, ptr %i30, align 8, !tbaa !1256
  %i32 = tail call i1 @llvm.type.test(ptr %i31, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i32)
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nonnull align 8 poison, i32 noundef 4), !intel_dtrans_type !1620, !_Intel.Devirt.Call !1273
  unreachable

bb33:                                             ; preds = %bb25
  store ptr @_ZN11xercesc_2_712XMLValidator15reinitMsgLoaderEv, ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, align 8, !tbaa !1715
  %i34 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, i64 0, i32 1), align 8, !tbaa !1719
  %i35 = icmp eq ptr %i34, null
  %i36 = load ptr, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, i64 0, i32 2), align 8
  %i37 = icmp eq ptr %i36, null
  %i38 = select i1 %i35, i1 %i37, i1 false
  br i1 %i38, label %bb39, label %bb44

bb39:                                             ; preds = %bb33
  %i40 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  store ptr %i40, ptr getelementptr inbounds (%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, i64 0, i32 1), align 8, !tbaa !1719
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !1720
  %i41 = icmp eq ptr %i40, null
  br i1 %i41, label %bb44, label %bb42

bb42:                                             ; preds = %bb39
  %i43 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i40, i64 0, i32 2, !intel-tbaa !1721
  store ptr @_ZN11xercesc_2_7L16msgLoaderCleanupE.3649, ptr %i43, align 8, !tbaa !1721
  br label %bb44

bb44:                                             ; preds = %bb42, %bb39, %bb33, %bb22
  %i45 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE.3653, align 8, !tbaa !1709
  br label %bb46

bb46:                                             ; preds = %bb44, %bb
  %i47 = phi ptr [ %i45, %bb44 ], [ %i, %bb ]
  ret ptr %i47
}

; Function Attrs: nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLValidator14reinitMsgMutexEv() #21 align 2

; Function Attrs: uwtable
define hidden fastcc void @_ZN11xercesc_2_712XMLValidator9emitErrorENS_8XMLValid5CodesEPKtS4_S4_S4_(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, ptr noundef "intel_dtrans_func_index"="3" %arg3, ptr noundef "intel_dtrans_func_index"="4" %arg4) unnamed_addr #14 align 2 !intel.dtrans.func.type !1757 {
bb:
  %i = alloca ptr, align 8, !intel_dtrans_type !817
  %i5 = alloca [2048 x i16], align 16, !intel_dtrans_type !1705
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1728
  %i7 = load ptr, ptr %i6, align 8, !tbaa !1728
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i7, i64 0, i32 25, !intel-tbaa !1732
  %i9 = load i32, ptr %i8, align 4, !tbaa !1732
  %i10 = add nsw i32 %i9, 1
  store i32 %i10, ptr %i8, align 4, !tbaa !1732
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 2, !intel-tbaa !1733
  %i12 = load ptr, ptr %i11, align 8, !tbaa !1733
  %i13 = icmp eq ptr %i12, null
  br i1 %i13, label %bb58, label %bb14

bb14:                                             ; preds = %bb
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i5) #47
  %i15 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L12getMsgLoaderEv()
  %i16 = getelementptr inbounds [2048 x i16], ptr %i5, i64 0, i64 0
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1728
  %i18 = load ptr, ptr %i17, align 8, !tbaa !1728
  %i19 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i18, i64 0, i32 62, !intel-tbaa !1758
  %i20 = load ptr, ptr %i19, align 8, !tbaa !1758
  %i21 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i15, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 8, !tbaa !1256
  %i23 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i23)
  %i24 = tail call i1 @llvm.type.test(ptr %i22, metadata !"_ZTSN11xercesc_2_714InMemMsgLoaderE")
  tail call void @llvm.assume(i1 %i24)
  %i25 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(16) %i15, i32 noundef %arg1, ptr noundef nonnull %i16, i32 noundef 2047), !intel_dtrans_type !1759, !_Intel.Devirt.Call !1273
  br i1 %i25, label %bb26, label %bb27

bb26:                                             ; preds = %bb14
  call fastcc void @_ZN11xercesc_2_79XMLString13replaceTokensEPtjPKtS3_S3_S3_PNS_13MemoryManagerE(ptr noundef nonnull %i16, i32 noundef 2047, ptr noundef %arg2, ptr noundef %arg3, ptr noundef %arg4, ptr noundef null, ptr noundef %i20)
  br label %bb27

bb27:                                             ; preds = %bb26, %bb14
  %i28 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 3, !intel-tbaa !1734
  %i29 = load ptr, ptr %i28, align 8, !tbaa !1734
  %i30 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i29, i64 0, i32 6, !intel-tbaa !1627
  %i31 = load ptr, ptr %i30, align 8, !tbaa !1627
  %i32 = icmp eq ptr %i31, null
  br i1 %i32, label %bb47, label %bb33

bb33:                                             ; preds = %bb27
  %i34 = getelementptr inbounds %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr %i29, i64 0, i32 2, !intel-tbaa !1623
  %i35 = load ptr, ptr %i34, align 8, !tbaa !1623
  %i36 = icmp eq ptr %i35, null
  br i1 %i36, label %bb47, label %bb37

bb37:                                             ; preds = %bb33
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #47
  %i38 = call fastcc noundef ptr @_ZNK11xercesc_2_79ReaderMgr16getLastExtEntityERPKNS_13XMLEntityDeclE(ptr noundef nonnull align 8 dereferenceable(80) %i29, ptr noundef nonnull align 8 dereferenceable(8) %i)
  %i39 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 22, !intel-tbaa !1735
  %i40 = load ptr, ptr %i39, align 8, !tbaa !1735
  %i41 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 11, !intel-tbaa !1747
  %i42 = load ptr, ptr %i41, align 8, !tbaa !1747
  %i43 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 6, !intel-tbaa !1748
  %i44 = load i64, ptr %i43, align 8, !tbaa !1748
  %i45 = getelementptr inbounds %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader", ptr %i38, i64 0, i32 5, !intel-tbaa !1749
  %i46 = load i64, ptr %i45, align 8, !tbaa !1749
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #47
  br label %bb47

bb47:                                             ; preds = %bb37, %bb33, %bb27
  %i48 = phi i64 [ %i46, %bb37 ], [ 0, %bb33 ], [ 0, %bb27 ]
  %i49 = phi i64 [ %i44, %bb37 ], [ 0, %bb33 ], [ 0, %bb27 ]
  %i50 = phi ptr [ %i42, %bb37 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb33 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb27 ]
  %i51 = phi ptr [ %i40, %bb37 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb33 ], [ @_ZN11xercesc_2_76XMLUni15fgZeroLenStringE, %bb27 ]
  %i52 = load ptr, ptr %i11, align 8, !tbaa !1733
  %i53 = getelementptr %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter", ptr %i52, i64 0, i32 0
  %i54 = load ptr, ptr %i53, align 8, !tbaa !1256
  %i55 = call i1 @llvm.type.test(ptr %i54, metadata !"_ZTSN11xercesc_2_716XMLErrorReporterE")
  call void @llvm.assume(i1 %i55)
  %i56 = getelementptr inbounds ptr, ptr %i54, i64 2
  %i57 = load ptr, ptr %i56, align 8
  call void %i57(ptr noundef nonnull align 8 dereferenceable(8) %i52, i32 noundef %arg1, ptr noundef nonnull @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, i32 noundef 1, ptr noundef nonnull %i16, ptr noundef %i51, ptr noundef %i50, i64 noundef %i49, i64 noundef %i48), !intel_dtrans_type !1750
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i5) #47
  br label %bb58

bb58:                                             ; preds = %bb47, %bb
  %i59 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator", ptr %arg, i64 0, i32 4, !intel-tbaa !1728
  %i60 = load ptr, ptr %i59, align 8, !tbaa !1728
  %i61 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 6, !intel-tbaa !1751
  %i62 = load i8, ptr %i61, align 1, !tbaa !1751, !range !1451, !noundef !1452
  %i63 = icmp eq i8 %i62, 0
  br i1 %i63, label %bb74, label %bb64

bb64:                                             ; preds = %bb58
  %i65 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 5, !intel-tbaa !1752
  %i66 = load i8, ptr %i65, align 1, !tbaa !1752, !range !1451, !noundef !1452
  %i67 = icmp eq i8 %i66, 0
  br i1 %i67, label %bb74, label %bb68

bb68:                                             ; preds = %bb64
  %i69 = getelementptr inbounds %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner", ptr %i60, i64 0, i32 7, !intel-tbaa !1753
  %i70 = load i8, ptr %i69, align 1, !tbaa !1753, !range !1451, !noundef !1452
  %i71 = icmp eq i8 %i70, 0
  br i1 %i71, label %bb72, label %bb74

bb72:                                             ; preds = %bb68
  %i73 = call ptr @__cxa_allocate_exception(i64 4) #47
  store i32 %arg1, ptr %i73, align 16, !tbaa !1754
  call void @__cxa_throw(ptr nonnull %i73, ptr nonnull @_ZTIN11xercesc_2_78XMLValid5CodesE, ptr null) #51
  unreachable

bb74:                                             ; preds = %bb68, %bb64, %bb58
  ret void
}

; Function Attrs: uwtable
define hidden fastcc noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEm(i64 noundef %arg) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1760 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i1 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i, i64 0, i32 0
  %i2 = load ptr, ptr %i1, align 8, !tbaa !1256
  %i3 = tail call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i6, label %bb7, label %bb22

bb7:                                              ; preds = %bb
  %i8 = add nuw nsw i64 %arg, 8
  %i9 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i8) #50
          to label %bb24 unwind label %bb10

bb10:                                             ; preds = %bb7
  %i11 = landingpad { ptr, i32 }
          catch ptr null
  %i12 = extractvalue { ptr, i32 } %i11, 0
  %i13 = tail call ptr @__cxa_begin_catch(ptr %i12) #47
  %i14 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i14, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i20) #48
  unreachable

bb21:                                             ; preds = %bb10
  unreachable

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb24:                                             ; preds = %bb7
  %i25 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  store ptr %i25, ptr %i9, align 8, !tbaa !1271
  %i26 = getelementptr inbounds i8, ptr %i9, i64 8, !intel-tbaa !1461
  ret ptr %i26
}

; Function Attrs: uwtable
define hidden fastcc noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) unnamed_addr #14 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1761 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg1, i64 0, i32 0
  %i2 = load ptr, ptr %i, align 8, !tbaa !1256
  %i3 = tail call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i5 = load ptr, ptr %i4, align 8
  %i6 = icmp eq ptr %i5, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i6, label %bb7, label %bb22

bb7:                                              ; preds = %bb
  %i8 = add nuw nsw i64 %arg, 8
  %i9 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i8) #50
          to label %bb24 unwind label %bb10

bb10:                                             ; preds = %bb7
  %i11 = landingpad { ptr, i32 }
          catch ptr null
  %i12 = extractvalue { ptr, i32 } %i11, 0
  %i13 = tail call ptr @__cxa_begin_catch(ptr %i12) #47
  %i14 = tail call ptr @__cxa_allocate_exception(i64 1) #47
  invoke void @__cxa_throw(ptr nonnull %i14, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #51
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
  tail call fastcc void @__clang_call_terminate(ptr %i20) #48
  unreachable

bb21:                                             ; preds = %bb10
  unreachable

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nonnull align 8 poison, i64 poison), !intel_dtrans_type !1272, !_Intel.Devirt.Call !1273
  unreachable

bb24:                                             ; preds = %bb7
  store ptr %arg1, ptr %i9, align 8, !tbaa !1271
  %i25 = getelementptr inbounds i8, ptr %i9, i64 8, !intel-tbaa !1461
  ret ptr %i25
}

; Function Attrs: mustprogress nounwind uwtable
define hidden fastcc void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg) unnamed_addr #43 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1762 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb12, label %bb1

bb1:                                              ; preds = %bb
  %i2 = getelementptr inbounds i8, ptr %arg, i64 -8, !intel-tbaa !1461
  %i3 = load ptr, ptr %i2, align 8, !tbaa !1271
  %i4 = load ptr, ptr %i3, align 8, !tbaa !1256
  %i5 = tail call i1 @llvm.type.test(ptr %i4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i5)
  %i6 = getelementptr inbounds ptr, ptr %i4, i64 3
  %i7 = load ptr, ptr %i6, align 8
  %i8 = icmp eq ptr %i7, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i8, label %bb11, label %bb9

bb9:                                              ; preds = %bb1
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb10 unwind label %bb13, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb10:                                             ; preds = %bb9
  unreachable

bb11:                                             ; preds = %bb1
  tail call void @_ZdlPv(ptr noundef nonnull %i2) #47
  br label %bb12

bb12:                                             ; preds = %bb11, %bb
  ret void

bb13:                                             ; preds = %bb9
  %i14 = landingpad { ptr, i32 }
          catch ptr null
  %i15 = extractvalue { ptr, i32 } %i14, 0
  tail call fastcc void @__clang_call_terminate(ptr %i15) #48
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !1763 dso_local void @_ZNSt9bad_allocD1Ev(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1") unnamed_addr #44

; Function Attrs: nofree norecurse noreturn uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2" %arg, i64 %arg1) unnamed_addr #45 comdat align 2 !intel.dtrans.func.type !1765 !_Intel.Devirt.Target !1380 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #47
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !1256
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #51
  unreachable
}

; Function Attrs: nofree norecurse noreturn uwtable
define hidden void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr nocapture readnone "intel_dtrans_func_index"="2" %arg1) unnamed_addr #45 comdat align 2 !intel.dtrans.func.type !1767 !_Intel.Devirt.Target !1380 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #47
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !1256
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #51
  unreachable
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #46

; Function Attrs: mustprogress uwtable
define dso_local void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5838(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) unnamed_addr #32 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !1768 {
bb:
  %i = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !1438
  %i3 = load i32, ptr %i, align 4, !tbaa !1438
  %i4 = icmp ugt i32 %i3, %arg2
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #47
  %i7 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1441
  %i8 = load ptr, ptr %i7, align 8, !tbaa !1441
  invoke fastcc void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1138, i32 noundef 52, i32 noundef 116, ptr noundef %i8)
          to label %bb9 unwind label %bb10

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #51
  unreachable

bb10:                                             ; preds = %bb5
  %i11 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #47
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb
  %i13 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !1441
  %i14 = load ptr, ptr %i13, align 8, !tbaa !1441
  %i15 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !1442
  %i16 = load ptr, ptr %i15, align 8, !tbaa !1442
  %i17 = zext i32 %arg2 to i64
  %i18 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i16, i64 %i17, i32 2
  %i19 = load ptr, ptr %i18, align 8, !tbaa !1443
  %i20 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i21 = load ptr, ptr %i20, align 8, !tbaa !1256
  %i22 = tail call i1 @llvm.type.test(ptr %i21, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i22)
  %i23 = getelementptr inbounds ptr, ptr %i21, i64 3
  %i24 = load ptr, ptr %i23, align 8
  %i25 = icmp eq ptr %i24, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i25, label %bb27, label %bb26

bb26:                                             ; preds = %bb12
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr poison), !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273
  unreachable

bb27:                                             ; preds = %bb12
  tail call void @_ZdlPv(ptr noundef %i19) #47
  %i28 = getelementptr inbounds %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !1442
  %i29 = load ptr, ptr %i28, align 8, !tbaa !1442
  %i30 = getelementptr inbounds %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %i29, i64 %i17, i32 2
  store ptr %arg1, ptr %i30, align 8, !tbaa !1443
  ret void
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_725DurationDatatypeValidator12compareDatesEPKNS_11XMLDateTimeES3_b.6352.extracted(ptr nocapture %arg, ptr nocapture %arg1, ptr nocapture readonly %arg2, ptr nocapture readonly %arg3, i1 %arg4, ptr nocapture writeonly %arg5) #14 personality ptr @__gxx_personality_v0 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i6 = getelementptr %"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_79XMLNumberE.0, i64 0, i64 2), ptr %i6, align 8, !tbaa !1256
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i7, align 8, !tbaa !1256
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 5, !intel-tbaa !1697
  store i32 0, ptr %i8, align 8, !tbaa !1697
  %i9 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 8, !intel-tbaa !1683
  store ptr null, ptr %i9, align 8, !tbaa !1683
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 9, !intel-tbaa !1686
  store ptr %i, ptr %i10, align 8, !tbaa !1686
  br label %bb11

bb11:                                             ; preds = %bb11, %bb
  %i12 = phi i64 [ 0, %bb ], [ %i14, %bb11 ]
  %i13 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 1, i64 %i12, !intel-tbaa !1381
  store i32 0, ptr %i13, align 4, !tbaa !1381
  %i14 = add nuw nsw i64 %i12, 1
  %i15 = icmp eq i64 %i14, 8
  br i1 %i15, label %bb16, label %bb11, !llvm.loop !1691

bb16:                                             ; preds = %bb11
  %i17 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 7, !intel-tbaa !1684
  %i18 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 6, !intel-tbaa !1685
  %i19 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 4, !intel-tbaa !1689
  %i20 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 3, !intel-tbaa !1690
  store double 0.000000e+00, ptr %i18, align 8, !tbaa !1685
  store i8 0, ptr %i17, align 8, !tbaa !1684
  %i21 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 1, !intel-tbaa !1688
  store i32 0, ptr %i21, align 4, !tbaa !1688
  %i22 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 2, i64 0, !intel-tbaa !1688
  store i32 0, ptr %i22, align 8, !tbaa !1688
  store i32 0, ptr %i19, align 4, !tbaa !1689
  store i32 0, ptr %i20, align 8, !tbaa !1690
  %i23 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !1271
  %i24 = getelementptr %"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber", ptr %arg1, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_79XMLNumberE.0, i64 0, i64 2), ptr %i24, align 8, !tbaa !1256
  %i25 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i25, align 8, !tbaa !1256
  %i26 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 5, !intel-tbaa !1697
  store i32 0, ptr %i26, align 8, !tbaa !1697
  %i27 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 8, !intel-tbaa !1683
  store ptr null, ptr %i27, align 8, !tbaa !1683
  %i28 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 9, !intel-tbaa !1686
  store ptr %i23, ptr %i28, align 8, !tbaa !1686
  br label %bb29

bb29:                                             ; preds = %bb29, %bb16
  %i30 = phi i64 [ 0, %bb16 ], [ %i32, %bb29 ]
  %i31 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 1, i64 %i30, !intel-tbaa !1381
  store i32 0, ptr %i31, align 4, !tbaa !1381
  %i32 = add nuw nsw i64 %i30, 1
  %i33 = icmp eq i64 %i32, 8
  br i1 %i33, label %bb34, label %bb29, !llvm.loop !1691

bb34:                                             ; preds = %bb29
  %i35 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 7, !intel-tbaa !1684
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 6, !intel-tbaa !1685
  %i37 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 4, !intel-tbaa !1689
  %i38 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 3, !intel-tbaa !1690
  store double 0.000000e+00, ptr %i36, align 8, !tbaa !1685
  store i8 0, ptr %i35, align 8, !tbaa !1684
  %i39 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 2, i64 1, !intel-tbaa !1688
  store i32 0, ptr %i39, align 4, !tbaa !1688
  %i40 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 2, i64 0, !intel-tbaa !1688
  store i32 0, ptr %i40, align 8, !tbaa !1688
  store i32 0, ptr %i37, align 4, !tbaa !1689
  store i32 0, ptr %i38, align 8, !tbaa !1690
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg, ptr noundef nonnull %arg2, i32 noundef 0)
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg1, ptr noundef %arg3, i32 noundef 0)
  %i41 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef nonnull %arg, ptr noundef nonnull %arg1)
          to label %bb42 unwind label %bb93, !range !1370

bb42:                                             ; preds = %bb34
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg, ptr noundef nonnull %arg2, i32 noundef 1)
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg1, ptr noundef %arg3, i32 noundef 1)
  %i43 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef nonnull %arg, ptr noundef nonnull %arg1)
          to label %bb44 unwind label %bb93, !range !1370

bb44:                                             ; preds = %bb42
  %i45 = icmp eq i32 %i41, %i43
  %i46 = select i1 %i45, i32 %i41, i32 2
  br i1 %i45, label %bb84, label %bb47

bb47:                                             ; preds = %bb90, %bb86, %bb44
  %i48 = phi i32 [ 2, %bb44 ], [ %i92, %bb90 ], [ 2, %bb86 ]
  store i32 %i48, ptr %arg5, align 4
  %i49 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i49, align 8, !tbaa !1256
  %i50 = load ptr, ptr %i27, align 8, !tbaa !1683
  %i51 = icmp eq ptr %i50, null
  br i1 %i51, label %bb52, label %bb70

bb52:                                             ; preds = %bb78, %bb47
  %i53 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i53, align 8, !tbaa !1256
  %i54 = load ptr, ptr %i9, align 8, !tbaa !1683
  %i55 = icmp eq ptr %i54, null
  br i1 %i55, label %bb131, label %bb56

bb56:                                             ; preds = %bb52
  %i57 = load ptr, ptr %i10, align 8, !tbaa !1686
  %i58 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i57, i64 0, i32 0
  %i59 = load ptr, ptr %i58, align 8, !tbaa !1256
  %i60 = tail call i1 @llvm.type.test(ptr %i59, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i60)
  %i61 = getelementptr inbounds ptr, ptr %i59, i64 3
  %i62 = load ptr, ptr %i61, align 8
  %i63 = icmp eq ptr %i62, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i63, label %bb64, label %bb65

bb64:                                             ; preds = %bb56
  tail call void @_ZdlPv(ptr noundef nonnull %i54) #47
  br label %bb131

bb65:                                             ; preds = %bb56
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb66 unwind label %bb67, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb66:                                             ; preds = %bb65
  unreachable

bb67:                                             ; preds = %bb65
  %i68 = landingpad { ptr, i32 }
          catch ptr null
  %i69 = extractvalue { ptr, i32 } %i68, 0
  tail call fastcc void @__clang_call_terminate(ptr %i69) #48
  unreachable

bb70:                                             ; preds = %bb47
  %i71 = load ptr, ptr %i28, align 8, !tbaa !1686
  %i72 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i71, i64 0, i32 0
  %i73 = load ptr, ptr %i72, align 8, !tbaa !1256
  %i74 = tail call i1 @llvm.type.test(ptr %i73, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i74)
  %i75 = getelementptr inbounds ptr, ptr %i73, i64 3
  %i76 = load ptr, ptr %i75, align 8
  %i77 = icmp eq ptr %i76, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i77, label %bb78, label %bb79

bb78:                                             ; preds = %bb70
  tail call void @_ZdlPv(ptr noundef nonnull %i50) #47
  br label %bb52

bb79:                                             ; preds = %bb70
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb80 unwind label %bb81, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb80:                                             ; preds = %bb79
  unreachable

bb81:                                             ; preds = %bb79
  %i82 = landingpad { ptr, i32 }
          catch ptr null
  %i83 = extractvalue { ptr, i32 } %i82, 0
  tail call fastcc void @__clang_call_terminate(ptr %i83) #48
  unreachable

bb84:                                             ; preds = %bb44
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg, ptr noundef nonnull %arg2, i32 noundef 2)
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg1, ptr noundef %arg3, i32 noundef 2)
  %i85 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef nonnull %arg, ptr noundef nonnull %arg1)
          to label %bb86 unwind label %bb93, !range !1370

bb86:                                             ; preds = %bb84
  %i87 = icmp eq i32 %i46, %i85
  br i1 %i87, label %bb88, label %bb47

bb88:                                             ; preds = %bb86
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg, ptr noundef nonnull %arg2, i32 noundef 3)
  tail call fastcc void @_ZN11xercesc_2_711XMLDateTime11addDurationEPS0_PKS0_i(ptr noundef nonnull %arg1, ptr noundef %arg3, i32 noundef 3)
  %i89 = invoke fastcc noundef i32 @_ZN11xercesc_2_711XMLDateTime12compareOrderEPKS0_S2_(ptr noundef nonnull %arg, ptr noundef nonnull %arg1)
          to label %bb90 unwind label %bb93, !range !1370

bb90:                                             ; preds = %bb88
  %i91 = icmp eq i32 %i46, %i89
  %i92 = select i1 %i91, i32 %i46, i32 2
  br label %bb47

bb93:                                             ; preds = %bb88, %bb84, %bb42, %bb34
  %i94 = landingpad { ptr, i32 }
          cleanup
  %i95 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg1, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i95, align 8, !tbaa !1256
  %i96 = load ptr, ptr %i27, align 8, !tbaa !1683
  %i97 = icmp eq ptr %i96, null
  br i1 %i97, label %bb98, label %bb117

bb98:                                             ; preds = %bb125, %bb93
  %i99 = getelementptr inbounds %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime", ptr %arg, i64 0, i32 0, i32 0, i32 0
  store ptr getelementptr inbounds ([11 x ptr], ptr @_ZTVN11xercesc_2_711XMLDateTimeE.0, i64 0, i64 2), ptr %i99, align 8, !tbaa !1256
  %i100 = load ptr, ptr %i9, align 8, !tbaa !1683
  %i101 = icmp eq ptr %i100, null
  br i1 %i101, label %bb102, label %bb103

bb102:                                            ; preds = %bb111, %bb98
  resume { ptr, i32 } %i94

bb103:                                            ; preds = %bb98
  %i104 = load ptr, ptr %i10, align 8, !tbaa !1686
  %i105 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i104, i64 0, i32 0
  %i106 = load ptr, ptr %i105, align 8, !tbaa !1256
  %i107 = tail call i1 @llvm.type.test(ptr %i106, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i107)
  %i108 = getelementptr inbounds ptr, ptr %i106, i64 3
  %i109 = load ptr, ptr %i108, align 8
  %i110 = icmp eq ptr %i109, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i110, label %bb111, label %bb112

bb111:                                            ; preds = %bb103
  tail call void @_ZdlPv(ptr noundef nonnull %i100) #47
  br label %bb102

bb112:                                            ; preds = %bb103
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb113 unwind label %bb114, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb113:                                            ; preds = %bb112
  unreachable

bb114:                                            ; preds = %bb112
  %i115 = landingpad { ptr, i32 }
          catch ptr null
  %i116 = extractvalue { ptr, i32 } %i115, 0
  tail call fastcc void @__clang_call_terminate(ptr %i116) #48
  unreachable

bb117:                                            ; preds = %bb93
  %i118 = load ptr, ptr %i28, align 8, !tbaa !1686
  %i119 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i118, i64 0, i32 0
  %i120 = load ptr, ptr %i119, align 8, !tbaa !1256
  %i121 = tail call i1 @llvm.type.test(ptr %i120, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i121)
  %i122 = getelementptr inbounds ptr, ptr %i120, i64 3
  %i123 = load ptr, ptr %i122, align 8
  %i124 = icmp eq ptr %i123, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i124, label %bb125, label %bb126

bb125:                                            ; preds = %bb117
  tail call void @_ZdlPv(ptr noundef nonnull %i96) #47
  br label %bb98

bb126:                                            ; preds = %bb117
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nonnull align 8 poison, ptr nonnull poison)
          to label %bb127 unwind label %bb128, !intel_dtrans_type !1398, !_Intel.Devirt.Call !1273

bb127:                                            ; preds = %bb126
  unreachable

bb128:                                            ; preds = %bb126
  %i129 = landingpad { ptr, i32 }
          catch ptr null
  %i130 = extractvalue { ptr, i32 } %i129, 0
  tail call fastcc void @__clang_call_terminate(ptr %i130) #48
  unreachable

bb131:                                            ; preds = %bb64, %bb52
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
attributes #11 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #12 = { norecurse noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #18 = { mustprogress noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #19 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #20 = { mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #21 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #22 = { noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #23 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #24 = { inlinehint nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #25 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #26 = { inlinehint uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #27 = { mustprogress nofree norecurse noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #28 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #29 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #30 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #31 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #32 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #33 = { mustprogress nofree nounwind willreturn memory(argmem: read) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #34 = { mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #35 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #36 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #37 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "dtrans-vector-size-field"="1" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #38 = { inlinehint uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #39 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #40 = { mustprogress nofree nosync nounwind memory(write, argmem: readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #41 = { mustprogress nofree nosync nounwind willreturn memory(none) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #42 = { mustprogress nofree nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #43 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #44 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #45 = { nofree norecurse noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #46 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #47 = { nounwind }
attributes #48 = { noreturn nounwind }
attributes #49 = { nounwind willreturn memory(read) }
attributes #50 = { allocsize(0) }
attributes #51 = { noreturn }
attributes #52 = { cold }
attributes #53 = { nounwind willreturn memory(none) }
attributes #54 = { builtin nounwind }

!intel.dtrans.types = !{!735, !738, !741, !742, !743, !744, !751, !752, !755, !756, !757, !761, !762, !764, !765, !766, !769, !770, !773, !776, !777, !778, !781, !782, !783, !784, !785, !786, !807, !808, !810, !812, !815, !821, !825, !827, !829, !834, !835, !836, !840, !841, !846, !850, !852, !853, !864, !867, !869, !877, !878, !881, !883, !884, !885, !891, !894, !895, !900, !902, !907, !909, !911, !914, !917, !918, !921, !922, !923, !924, !926, !928, !930, !932, !933, !935, !937, !940, !942, !944, !946, !949, !950, !954, !956, !957, !959, !961, !964, !965, !966, !967, !969, !970, !971, !973, !974, !975, !976, !980, !981, !982, !986, !988, !989, !1002, !1004, !1006, !1009, !1011, !1013, !1015, !1017, !1020, !1021, !1024, !1026, !1028, !1030, !1032, !1033, !1034, !1035, !1037, !1038, !1039, !1041, !1042, !1043, !1047, !1048, !1050, !1052, !1054, !1056, !1060, !1065, !1067, !1070, !1073, !1075, !1077, !1080, !1082, !1084, !1087, !1089, !1091, !1093, !1095, !1097, !1099, !1101, !1103, !1107, !1109, !1110, !1111, !1112, !1114, !1117, !1119, !1121, !1123, !1124, !1126, !1128, !1130, !1132, !1134, !1136, !1138, !1140, !1142, !1144, !1146, !1149, !1151, !1154, !1156, !1159, !1160, !1162, !1163, !1165, !1166, !1168, !1170, !1172, !1174, !1176, !1178, !1180, !1182, !1184, !1186, !1188, !1190, !1191, !1193, !1197, !1199, !1201, !1203, !1205, !1207, !1209, !1211, !1213, !1215, !1218, !1220, !1223, !1224, !1226, !1228, !1231, !1233, !1234, !1235, !1237, !1238, !1239, !1240, !1241}
!llvm.ident = !{!1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242, !1242}
!llvm.module.flags = !{!1243, !1244, !1245, !1246, !1247, !1248}

!0 = !{!"L", i32 3, !1, !1, !1}
!1 = !{i8 0, i32 1}
!2 = !{!3, i32 0}
!3 = !{!"L", i32 311, !4, !8, !12, !16, !20, !24, !28, !32, !4, !35, !39, !41, !45, !47, !24, !51, !55, !59, !63, !65, !66, !35, !70, !74, !78, !63, !82, !86, !90, !94, !98, !102, !106, !86, !110, !114, !118, !122, !126, !122, !122, !130, !134, !138, !20, !45, !142, !142, !144, !146, !59, !148, !55, !152, !70, !156, !158, !162, !28, !164, !166, !170, !166, !16, !174, !178, !170, !180, !182, !28, !8, !28, !122, !78, !114, !114, !20, !186, !134, !190, !35, !180, !194, !196, !198, !200, !202, !162, !204, !39, !12, !206, !194, !204, !106, !55, !162, !204, !118, !210, !8, !164, !213, !215, !219, !162, !35, !146, !102, !41, !146, !213, !164, !196, !146, !174, !221, !94, !225, !229, !47, !233, !55, !70, !55, !156, !219, !235, !152, !144, !239, !241, !243, !24, !98, !65, !229, !235, !213, !245, !247, !251, !59, !134, !158, !78, !200, !194, !210, !221, !186, !255, !174, !257, !261, !265, !28, !39, !178, !225, !164, !110, !74, !39, !16, !51, !268, !272, !74, !32, !4, !65, !126, !274, !114, !164, !130, !74, !90, !276, !274, !272, !114, !274, !278, !274, !196, !280, !282, !284, !286, !274, !66, !82, !286, !288, !286, !278, !90, !90, !41, !126, !158, !110, !290, !118, !276, !274, !280, !292, !158, !296, !106, !280, !282, !280, !284, !59, !278, !298, !138, !282, !138, !90, !296, !288, !274, !70, !82, !130, !282, !122, !12, !300, !302, !292, !304, !130, !82, !282, !306, !78, !86, !90, !122, !304, !196, !194, !55, !221, !298, !138, !82, !286, !304, !210, !180, !206, !20, !308, !286, !204, !296, !274, !310, !180, !78, !314, !164, !24, !12, !204, !86, !156, !148, !272, !158, !63, !272, !134, !41, !78, !221, !59, !158, !302, !86, !316, !41, !302, !134, !28, !78, !318, !39, !302, !118, !166, !156, !320, !302, !45, !245, !164, !148, !16, !308, !118, !28, !86, !32}
!4 = !{!5, i32 0}
!5 = !{!"L", i32 8, !6, !6, !6, !6, !6, !6, !6, !7}
!6 = !{i16 0, i32 0}
!7 = !{!"A", i32 121, !6}
!8 = !{!9, i32 0}
!9 = !{!"L", i32 2, !10, !11}
!10 = !{!"A", i32 40, !6}
!11 = !{!"A", i32 88, !6}
!12 = !{!13, i32 0}
!13 = !{!"L", i32 2, !14, !15}
!14 = !{!"A", i32 59, !6}
!15 = !{!"A", i32 69, !6}
!16 = !{!17, i32 0}
!17 = !{!"L", i32 2, !18, !19}
!18 = !{!"A", i32 94, !6}
!19 = !{!"A", i32 34, !6}
!20 = !{!21, i32 0}
!21 = !{!"L", i32 2, !22, !23}
!22 = !{!"A", i32 66, !6}
!23 = !{!"A", i32 62, !6}
!24 = !{!25, i32 0}
!25 = !{!"L", i32 2, !26, !27}
!26 = !{!"A", i32 61, !6}
!27 = !{!"A", i32 67, !6}
!28 = !{!29, i32 0}
!29 = !{!"L", i32 2, !30, !31}
!30 = !{!"A", i32 44, !6}
!31 = !{!"A", i32 84, !6}
!32 = !{!33, i32 0}
!33 = !{!"L", i32 6, !6, !6, !6, !6, !6, !34}
!34 = !{!"A", i32 123, !6}
!35 = !{!36, i32 0}
!36 = !{!"L", i32 2, !37, !38}
!37 = !{!"A", i32 100, !6}
!38 = !{!"A", i32 28, !6}
!39 = !{!40, i32 0}
!40 = !{!"L", i32 2, !23, !22}
!41 = !{!42, i32 0}
!42 = !{!"L", i32 2, !43, !44}
!43 = !{!"A", i32 76, !6}
!44 = !{!"A", i32 52, !6}
!45 = !{!46, i32 0}
!46 = !{!"L", i32 2, !27, !26}
!47 = !{!48, i32 0}
!48 = !{!"L", i32 2, !49, !50}
!49 = !{!"A", i32 72, !6}
!50 = !{!"A", i32 56, !6}
!51 = !{!52, i32 0}
!52 = !{!"L", i32 2, !53, !54}
!53 = !{!"A", i32 65, !6}
!54 = !{!"A", i32 63, !6}
!55 = !{!56, i32 0}
!56 = !{!"L", i32 2, !57, !58}
!57 = !{!"A", i32 74, !6}
!58 = !{!"A", i32 54, !6}
!59 = !{!60, i32 0}
!60 = !{!"L", i32 2, !61, !62}
!61 = !{!"A", i32 57, !6}
!62 = !{!"A", i32 71, !6}
!63 = !{!64, i32 0}
!64 = !{!"L", i32 2, !50, !49}
!65 = !{!"A", i32 128, !6}
!66 = !{!67, i32 0}
!67 = !{!"L", i32 2, !68, !69}
!68 = !{!"A", i32 20, !6}
!69 = !{!"A", i32 108, !6}
!70 = !{!71, i32 0}
!71 = !{!"L", i32 2, !72, !73}
!72 = !{!"A", i32 49, !6}
!73 = !{!"A", i32 79, !6}
!74 = !{!75, i32 0}
!75 = !{!"L", i32 2, !76, !77}
!76 = !{!"A", i32 58, !6}
!77 = !{!"A", i32 70, !6}
!78 = !{!79, i32 0}
!79 = !{!"L", i32 2, !80, !81}
!80 = !{!"A", i32 46, !6}
!81 = !{!"A", i32 82, !6}
!82 = !{!83, i32 0}
!83 = !{!"L", i32 2, !84, !85}
!84 = !{!"A", i32 36, !6}
!85 = !{!"A", i32 92, !6}
!86 = !{!87, i32 0}
!87 = !{!"L", i32 2, !88, !89}
!88 = !{!"A", i32 50, !6}
!89 = !{!"A", i32 78, !6}
!90 = !{!91, i32 0}
!91 = !{!"L", i32 2, !92, !93}
!92 = !{!"A", i32 27, !6}
!93 = !{!"A", i32 101, !6}
!94 = !{!95, i32 0}
!95 = !{!"L", i32 2, !96, !97}
!96 = !{!"A", i32 97, !6}
!97 = !{!"A", i32 31, !6}
!98 = !{!99, i32 0}
!99 = !{!"L", i32 2, !100, !101}
!100 = !{!"A", i32 77, !6}
!101 = !{!"A", i32 51, !6}
!102 = !{!103, i32 0}
!103 = !{!"L", i32 2, !104, !105}
!104 = !{!"A", i32 98, !6}
!105 = !{!"A", i32 30, !6}
!106 = !{!107, i32 0}
!107 = !{!"L", i32 2, !108, !109}
!108 = !{!"A", i32 53, !6}
!109 = !{!"A", i32 75, !6}
!110 = !{!111, i32 0}
!111 = !{!"L", i32 2, !112, !113}
!112 = !{!"A", i32 23, !6}
!113 = !{!"A", i32 105, !6}
!114 = !{!115, i32 0}
!115 = !{!"L", i32 2, !116, !117}
!116 = !{!"A", i32 24, !6}
!117 = !{!"A", i32 104, !6}
!118 = !{!119, i32 0}
!119 = !{!"L", i32 2, !120, !121}
!120 = !{!"A", i32 42, !6}
!121 = !{!"A", i32 86, !6}
!122 = !{!123, i32 0}
!123 = !{!"L", i32 2, !124, !125}
!124 = !{!"A", i32 39, !6}
!125 = !{!"A", i32 89, !6}
!126 = !{!127, i32 0}
!127 = !{!"L", i32 2, !128, !129}
!128 = !{!"A", i32 25, !6}
!129 = !{!"A", i32 103, !6}
!130 = !{!131, i32 0}
!131 = !{!"L", i32 2, !132, !133}
!132 = !{!"A", i32 33, !6}
!133 = !{!"A", i32 95, !6}
!134 = !{!135, i32 0}
!135 = !{!"L", i32 2, !136, !137}
!136 = !{!"A", i32 45, !6}
!137 = !{!"A", i32 83, !6}
!138 = !{!139, i32 0}
!139 = !{!"L", i32 2, !140, !141}
!140 = !{!"A", i32 38, !6}
!141 = !{!"A", i32 90, !6}
!142 = !{!143, i32 0}
!143 = !{!"L", i32 2, !129, !128}
!144 = !{!145, i32 0}
!145 = !{!"L", i32 2, !31, !30}
!146 = !{!147, i32 0}
!147 = !{!"L", i32 2, !133, !132}
!148 = !{!149, i32 0}
!149 = !{!"L", i32 2, !150, !151}
!150 = !{!"A", i32 48, !6}
!151 = !{!"A", i32 80, !6}
!152 = !{!153, i32 0}
!153 = !{!"L", i32 2, !154, !155}
!154 = !{!"A", i32 73, !6}
!155 = !{!"A", i32 55, !6}
!156 = !{!157, i32 0}
!157 = !{!"L", i32 2, !89, !88}
!158 = !{!159, i32 0}
!159 = !{!"L", i32 2, !160, !161}
!160 = !{!"A", i32 47, !6}
!161 = !{!"A", i32 81, !6}
!162 = !{!163, i32 0}
!163 = !{!"L", i32 2, !121, !120}
!164 = !{!165, i32 0}
!165 = !{!"L", i32 2, !62, !61}
!166 = !{!167, i32 0}
!167 = !{!"L", i32 2, !168, !169}
!168 = !{!"A", i32 87, !6}
!169 = !{!"A", i32 41, !6}
!170 = !{!171, i32 0}
!171 = !{!"L", i32 2, !172, !173}
!172 = !{!"A", i32 93, !6}
!173 = !{!"A", i32 35, !6}
!174 = !{!175, i32 0}
!175 = !{!"L", i32 2, !176, !177}
!176 = !{!"A", i32 99, !6}
!177 = !{!"A", i32 29, !6}
!178 = !{!179, i32 0}
!179 = !{!"L", i32 2, !85, !84}
!180 = !{!181, i32 0}
!181 = !{!"L", i32 2, !169, !168}
!182 = !{!183, i32 0}
!183 = !{!"L", i32 2, !184, !185}
!184 = !{!"A", i32 120, !6}
!185 = !{!"A", i32 8, !6}
!186 = !{!187, i32 0}
!187 = !{!"L", i32 2, !188, !189}
!188 = !{!"A", i32 111, !6}
!189 = !{!"A", i32 17, !6}
!190 = !{!191, i32 0}
!191 = !{!"L", i32 2, !192, !193}
!192 = !{!"A", i32 113, !6}
!193 = !{!"A", i32 15, !6}
!194 = !{!195, i32 0}
!195 = !{!"L", i32 2, !77, !76}
!196 = !{!197, i32 0}
!197 = !{!"L", i32 2, !44, !43}
!198 = !{!199, i32 0}
!199 = !{!"L", i32 2, !81, !80}
!200 = !{!201, i32 0}
!201 = !{!"L", i32 2, !151, !150}
!202 = !{!203, i32 0}
!203 = !{!"L", i32 2, !109, !108}
!204 = !{!205, i32 0}
!205 = !{!"L", i32 2, !101, !100}
!206 = !{!207, i32 0}
!207 = !{!"L", i32 2, !208, !209}
!208 = !{!"A", i32 60, !6}
!209 = !{!"A", i32 68, !6}
!210 = !{!211, i32 0}
!211 = !{!"L", i32 2, !212, !212}
!212 = !{!"A", i32 64, !6}
!213 = !{!214, i32 0}
!214 = !{!"L", i32 2, !113, !112}
!215 = !{!216, i32 0}
!216 = !{!"L", i32 2, !217, !218}
!217 = !{!"A", i32 106, !6}
!218 = !{!"A", i32 22, !6}
!219 = !{!220, i32 0}
!220 = !{!"L", i32 2, !137, !136}
!221 = !{!222, i32 0}
!222 = !{!"L", i32 2, !223, !224}
!223 = !{!"A", i32 43, !6}
!224 = !{!"A", i32 85, !6}
!225 = !{!226, i32 0}
!226 = !{!"L", i32 2, !227, !228}
!227 = !{!"A", i32 119, !6}
!228 = !{!"A", i32 9, !6}
!229 = !{!230, i32 0}
!230 = !{!"L", i32 2, !231, !232}
!231 = !{!"A", i32 96, !6}
!232 = !{!"A", i32 32, !6}
!233 = !{!234, i32 0}
!234 = !{!"L", i32 2, !125, !124}
!235 = !{!236, i32 0}
!236 = !{!"L", i32 2, !237, !238}
!237 = !{!"A", i32 112, !6}
!238 = !{!"A", i32 16, !6}
!239 = !{!240, i32 0}
!240 = !{!"L", i32 2, !117, !116}
!241 = !{!242, i32 0}
!242 = !{!"L", i32 2, !93, !92}
!243 = !{!244, i32 0}
!244 = !{!"L", i32 2, !73, !72}
!245 = !{!246, i32 0}
!246 = !{!"L", i32 2, !161, !160}
!247 = !{!248, i32 0}
!248 = !{!"L", i32 2, !249, !250}
!249 = !{!"A", i32 107, !6}
!250 = !{!"A", i32 21, !6}
!251 = !{!252, i32 0}
!252 = !{!"L", i32 2, !253, !254}
!253 = !{!"A", i32 109, !6}
!254 = !{!"A", i32 19, !6}
!255 = !{!256, i32 0}
!256 = !{!"L", i32 2, !69, !68}
!257 = !{!258, i32 0}
!258 = !{!"L", i32 2, !259, !260}
!259 = !{!"A", i32 102, !6}
!260 = !{!"A", i32 26, !6}
!261 = !{!262, i32 0}
!262 = !{!"L", i32 2, !263, !264}
!263 = !{!"A", i32 115, !6}
!264 = !{!"A", i32 13, !6}
!265 = !{!266, i32 0}
!266 = !{!"L", i32 4, !6, !6, !6, !267}
!267 = !{!"A", i32 125, !6}
!268 = !{!269, i32 0}
!269 = !{!"L", i32 2, !270, !271}
!270 = !{!"A", i32 91, !6}
!271 = !{!"A", i32 37, !6}
!272 = !{!273, i32 0}
!273 = !{!"L", i32 2, !271, !270}
!274 = !{!275, i32 0}
!275 = !{!"L", i32 2, !260, !259}
!276 = !{!277, i32 0}
!277 = !{!"L", i32 2, !254, !253}
!278 = !{!279, i32 0}
!279 = !{!"L", i32 2, !177, !176}
!280 = !{!281, i32 0}
!281 = !{!"L", i32 2, !218, !217}
!282 = !{!283, i32 0}
!283 = !{!"L", i32 2, !105, !104}
!284 = !{!285, i32 0}
!285 = !{!"L", i32 2, !250, !249}
!286 = !{!287, i32 0}
!287 = !{!"L", i32 2, !173, !172}
!288 = !{!289, i32 0}
!289 = !{!"L", i32 2, !19, !18}
!290 = !{!291, i32 0}
!291 = !{!"L", i32 2, !238, !237}
!292 = !{!293, i32 0}
!293 = !{!"L", i32 2, !294, !295}
!294 = !{!"A", i32 18, !6}
!295 = !{!"A", i32 110, !6}
!296 = !{!297, i32 0}
!297 = !{!"L", i32 2, !232, !231}
!298 = !{!299, i32 0}
!299 = !{!"L", i32 2, !97, !96}
!300 = !{!301, i32 0}
!301 = !{!"L", i32 2, !189, !188}
!302 = !{!303, i32 0}
!303 = !{!"L", i32 2, !54, !53}
!304 = !{!305, i32 0}
!305 = !{!"L", i32 2, !38, !37}
!306 = !{!307, i32 0}
!307 = !{!"L", i32 2, !224, !223}
!308 = !{!309, i32 0}
!309 = !{!"L", i32 2, !155, !154}
!310 = !{!311, i32 0}
!311 = !{!"L", i32 2, !312, !313}
!312 = !{!"A", i32 14, !6}
!313 = !{!"A", i32 114, !6}
!314 = !{!315, i32 0}
!315 = !{!"L", i32 2, !58, !57}
!316 = !{!317, i32 0}
!317 = !{!"L", i32 2, !15, !14}
!318 = !{!319, i32 0}
!319 = !{!"L", i32 2, !193, !192}
!320 = !{!321, i32 0}
!321 = !{!"L", i32 2, !141, !140}
!322 = !{!323, i32 0}
!323 = !{!"L", i32 401, !4, !210, !144, !32, !4, !272, !272, !86, !138, !180, !272, !272, !86, !314, !304, !158, !118, !298, !78, !158, !274, !126, !221, !288, !134, !274, !314, !302, !194, !8, !114, !180, !114, !278, !221, !282, !278, !304, !130, !130, !134, !74, !272, !164, !278, !82, !130, !298, !86, !28, !280, !284, !66, !280, !110, !278, !206, !180, !28, !180, !134, !221, !28, !302, !282, !86, !8, !296, !122, !221, !118, !70, !74, !55, !134, !308, !148, !78, !288, !180, !148, !106, !196, !134, !59, !298, !298, !274, !304, !272, !302, !308, !221, !152, !74, !180, !78, !78, !296, !82, !298, !8, !274, !274, !82, !90, !221, !74, !78, !78, !82, !118, !28, !118, !28, !78, !274, !308, !28, !144, !114, !298, !152, !304, !90, !318, !70, !318, !196, !94, !63, !221, !78, !82, !126, !290, !78, !86, !8, !276, !318, !63, !290, !122, !194, !304, !274, !304, !274, !274, !39, !272, !148, !51, !308, !316, !314, !47, !122, !204, !278, !296, !296, !158, !86, !86, !324, !324, !55, !314, !110, !206, !198, !243, !243, !198, !243, !41, !41, !243, !51, !55, !326, !55, !278, !278, !278, !278, !130, !82, !148, !314, !268, !268, !194, !194, !194, !194, !316, !55, !162, !233, !98, !162, !162, !98, !98, !55, !162, !98, !98, !55, !162, !233, !98, !206, !206, !206, !206, !198, !198, !198, !316, !316, !316, !316, !20, !51, !59, !59, !204, !51, !284, !138, !118, !158, !316, !59, !272, !272, !324, !152, !324, !288, !164, !98, !24, !70, !210, !196, !122, !118, !296, !282, !110, !296, !298, !272, !12, !298, !274, !288, !118, !90, !274, !28, !70, !134, !288, !282, !296, !110, !206, !206, !180, !138, !210, !24, !204, !122, !288, !66, !282, !282, !298, !296, !148, !274, !110, !286, !276, !292, !130, !328, !276, !158, !110, !206, !265, !158, !114, !288, !74, !118, !298, !243, !59, !63, !148, !272, !82, !286, !63, !74, !118, !280, !8, !8, !126, !290, !90, !280, !66, !280, !126, !28, !138, !82, !134, !158, !134, !12, !114, !284, !114, !284, !148, !304, !158, !272, !82, !282, !278, !286, !114, !148, !118, !8, !288, !82, !82, !82, !320, !45, !200, !35, !166, !35, !186, !198, !268, !215, !146, !162, !152, !86, !78, !158, !8, !194, !74, !45, !198, !202, !204, !74, !278, !180, !288, !158, !196, !300, !286, !308, !314, !300, !288, !296, !82, !288, !286, !138, !90, !276, !78, !272, !284, !126, !114, !204, !32}
!324 = !{!325, i32 0}
!325 = !{!"L", i32 2, !209, !208}
!326 = !{!327, i32 0}
!327 = !{!"L", i32 2, !11, !10}
!328 = !{!329, i32 0}
!329 = !{!"L", i32 2, !264, !263}
!330 = !{!331, i32 0}
!331 = !{!"L", i32 114, !4, !284, !90, !148, !286, !180, !204, !221, !70, !122, !28, !282, !196, !204, !63, !148, !70, !257, !70, !65, !86, !134, !268, !12, !148, !243, !74, !278, !110, !280, !152, !292, !63, !302, !8, !82, !316, !28, !65, !286, !134, !47, !221, !138, !298, !138, !286, !86, !288, !282, !284, !284, !122, !296, !298, !276, !272, !28, !28, !280, !298, !66, !304, !59, !8, !82, !276, !86, !174, !202, !194, !130, !265, !41, !106, !213, !308, !90, !130, !82, !166, !241, !156, !332, !274, !196, !114, !102, !86, !316, !102, !148, !245, !194, !286, !138, !174, !86, !51, !243, !306, !326, !213, !152, !194, !24, !39, !257, !59, !102, !12, !134, !98, !32}
!332 = !{!333, i32 0}
!333 = !{!"L", i32 2, !295, !294}
!334 = !{!335, i32 0}
!335 = !{!"L", i32 25, !4, !328, !210, !63, !39, !45, !148, !63, !55, !202, !98, !164, !316, !180, !39, !257, !47, !261, !328, !210, !65, !284, !282, !280, !32}
!336 = !{%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 1}
!337 = !{!"A", i32 4, !338}
!338 = !{!"A", i32 8, !339}
!339 = !{i32 0, i32 0}
!340 = !{%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 1}
!341 = !{%"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" zeroinitializer, i32 1}
!342 = !{%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" zeroinitializer, i32 1}
!343 = !{!"A", i32 7, !6}
!344 = !{!"L", i32 2, !1, !1}
!345 = !{!346, i32 0}
!346 = !{!"L", i32 4, !347, !349, !351, !351}
!347 = !{!348, i32 0}
!348 = !{!"L", i32 6, !6, !6, !6, !6, !6, !92}
!349 = !{!350, i32 0}
!350 = !{!"L", i32 5, !6, !6, !6, !6, !38}
!351 = !{!352, i32 0}
!352 = !{!"L", i32 2, !6, !97}
!353 = !{i64 16, !"_ZTSSt9bad_alloc"}
!354 = !{i64 32, !"_ZTSMSt9bad_allocKFPKcvE.virtual"}
!355 = !{i64 16, !"_ZTSSt9exception"}
!356 = !{i64 32, !"_ZTSMSt9exceptionKFPKcvE.virtual"}
!357 = !{!"L", i32 1, !358}
!358 = !{!"A", i32 5, !1}
!359 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}
!360 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!361 = !{!"A", i32 1, !6}
!362 = !{!"A", i32 65536, !363}
!363 = !{i8 0, i32 0}
!364 = !{i32 16, !"_ZTSN11xercesc_2_713XSerializableE"}
!365 = !{i32 32, !"_ZTSMN11xercesc_2_713XSerializableEKFbvE.virtual"}
!366 = !{i32 40, !"_ZTSMN11xercesc_2_713XSerializableEFvRNS_16XSerializeEngineEE.virtual"}
!367 = !{i32 48, !"_ZTSMN11xercesc_2_713XSerializableEKFPNS_10XProtoTypeEvE.virtual"}
!368 = !{i32 56, !"_ZTSMN11xercesc_2_713XSerializableEKFbvE.virtual"}
!369 = !{i32 64, !"_ZTSMN11xercesc_2_713XSerializableEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!370 = !{i32 72, !"_ZTSMN11xercesc_2_713XSerializableEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!371 = !{i32 80, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!372 = !{i32 88, !"_ZTSMN11xercesc_2_713XSerializableEFbPKNS_17DatatypeValidatorEE.virtual"}
!373 = !{i32 96, !"_ZTSMN11xercesc_2_713XSerializableEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!374 = !{i32 104, !"_ZTSMN11xercesc_2_713XSerializableEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!375 = !{i32 112, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!376 = !{i32 120, !"_ZTSMN11xercesc_2_713XSerializableEFvvE.virtual"}
!377 = !{i32 128, !"_ZTSMN11xercesc_2_713XSerializableEKFvPNS_13MemoryManagerEE.virtual"}
!378 = !{i32 136, !"_ZTSMN11xercesc_2_713XSerializableEKFvPNS_13MemoryManagerEE.virtual"}
!379 = !{i32 144, !"_ZTSMN11xercesc_2_713XSerializableEFiPKNS_9XMLNumberES3_E.virtual"}
!380 = !{i32 152, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!381 = !{i32 160, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtE.virtual"}
!382 = !{i32 168, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtE.virtual"}
!383 = !{i32 176, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtE.virtual"}
!384 = !{i32 184, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtE.virtual"}
!385 = !{i32 192, !"_ZTSMN11xercesc_2_713XSerializableEFvPNS_13MemoryManagerEE.virtual"}
!386 = !{i32 16, !"_ZTSN11xercesc_2_717DatatypeValidatorE"}
!387 = !{i32 32, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFbvE.virtual"}
!388 = !{i32 40, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!389 = !{i32 48, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!390 = !{i32 56, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFbvE.virtual"}
!391 = !{i32 64, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!392 = !{i32 72, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!393 = !{i32 80, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!394 = !{i32 88, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFbPKS0_E.virtual"}
!395 = !{i32 96, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!396 = !{i32 104, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFPS0_PNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!397 = !{i32 112, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!398 = !{i32 120, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvvE.virtual"}
!399 = !{i32 128, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!400 = !{i32 136, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!401 = !{i32 144, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!402 = !{i32 152, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!403 = !{i32 160, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtE.virtual"}
!404 = !{i32 168, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtE.virtual"}
!405 = !{i32 176, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtE.virtual"}
!406 = !{i32 184, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtE.virtual"}
!407 = !{i32 192, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!408 = !{i32 16, !"_ZTSN11xercesc_2_729AbstractNumericFacetValidatorE"}
!409 = !{i32 32, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFbvE.virtual"}
!410 = !{i32 40, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!411 = !{i32 48, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!412 = !{i32 56, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFbvE.virtual"}
!413 = !{i32 64, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!414 = !{i32 72, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!415 = !{i32 80, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!416 = !{i32 88, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!417 = !{i32 96, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!418 = !{i32 104, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!419 = !{i32 112, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!420 = !{i32 120, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvvE.virtual"}
!421 = !{i32 128, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!422 = !{i32 136, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!423 = !{i32 144, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!424 = !{i32 152, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!425 = !{i32 160, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtE.virtual"}
!426 = !{i32 168, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtE.virtual"}
!427 = !{i32 176, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtE.virtual"}
!428 = !{i32 184, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPKtE.virtual"}
!429 = !{i32 192, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPNS_13MemoryManagerEE.virtual"}
!430 = !{!"A", i32 25, !1}
!431 = !{i32 16, !"_ZTSN11xercesc_2_730AnySimpleTypeDatatypeValidatorE"}
!432 = !{i32 32, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEKFbvE.virtual"}
!433 = !{i32 40, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!434 = !{i32 48, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!435 = !{i32 56, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEKFbvE.virtual"}
!436 = !{i32 64, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!437 = !{i32 72, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!438 = !{i32 80, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!439 = !{i32 88, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!440 = !{i32 96, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!441 = !{i32 104, !"_ZTSMN11xercesc_2_730AnySimpleTypeDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!442 = !{!"A", i32 14, !1}
!443 = !{i32 136, !"_ZTSMN11xercesc_2_713XSerializableEKFvPKtPNS_13MemoryManagerEE.virtual"}
!444 = !{i32 144, !"_ZTSMN11xercesc_2_713XSerializableEKFiPKtPNS_13MemoryManagerEE.virtual"}
!445 = !{i32 152, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtPNS_13MemoryManagerEE.virtual"}
!446 = !{i32 160, !"_ZTSMN11xercesc_2_713XSerializableEFvPNS_13MemoryManagerEE.virtual"}
!447 = !{i32 168, !"_ZTSMN11xercesc_2_713XSerializableEFvvE.virtual"}
!448 = !{i32 176, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!449 = !{i32 184, !"_ZTSMN11xercesc_2_713XSerializableEFvPNS_13MemoryManagerEE.virtual"}
!450 = !{i32 192, !"_ZTSMN11xercesc_2_713XSerializableEKFvPtPNS_13MemoryManagerEE.virtual"}
!451 = !{i32 136, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFvPKtPNS_13MemoryManagerEE.virtual"}
!452 = !{i32 144, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFiPKtPNS_13MemoryManagerEE.virtual"}
!453 = !{i32 152, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtPNS_13MemoryManagerEE.virtual"}
!454 = !{i32 160, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!455 = !{i32 168, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvvE.virtual"}
!456 = !{i32 176, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!457 = !{i32 184, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!458 = !{i32 192, !"_ZTSMN11xercesc_2_717DatatypeValidatorEKFvPtPNS_13MemoryManagerEE.virtual"}
!459 = !{i32 16, !"_ZTSN11xercesc_2_723AbstractStringValidatorE"}
!460 = !{i32 32, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFbvE.virtual"}
!461 = !{i32 40, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!462 = !{i32 48, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!463 = !{i32 56, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFbvE.virtual"}
!464 = !{i32 64, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!465 = !{i32 72, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!466 = !{i32 80, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!467 = !{i32 88, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!468 = !{i32 96, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!469 = !{i32 104, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!470 = !{i32 112, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!471 = !{i32 120, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvvE.virtual"}
!472 = !{i32 128, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!473 = !{i32 136, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFvPKtPNS_13MemoryManagerEE.virtual"}
!474 = !{i32 144, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFiPKtPNS_13MemoryManagerEE.virtual"}
!475 = !{i32 152, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPKtPNS_13MemoryManagerEE.virtual"}
!476 = !{i32 160, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPNS_13MemoryManagerEE.virtual"}
!477 = !{i32 168, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvvE.virtual"}
!478 = !{i32 176, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!479 = !{i32 184, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEFvPNS_13MemoryManagerEE.virtual"}
!480 = !{i32 192, !"_ZTSMN11xercesc_2_723AbstractStringValidatorEKFvPtPNS_13MemoryManagerEE.virtual"}
!481 = !{i32 16, !"_ZTSN11xercesc_2_723AnyURIDatatypeValidatorE"}
!482 = !{i32 32, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFbvE.virtual"}
!483 = !{i32 40, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!484 = !{i32 48, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!485 = !{i32 56, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFbvE.virtual"}
!486 = !{i32 64, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!487 = !{i32 72, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!488 = !{i32 80, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!489 = !{i32 88, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!490 = !{i32 96, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!491 = !{i32 104, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!492 = !{i32 112, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!493 = !{i32 120, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvvE.virtual"}
!494 = !{i32 128, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!495 = !{i32 136, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFvPKtPNS_13MemoryManagerEE.virtual"}
!496 = !{i32 144, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFiPKtPNS_13MemoryManagerEE.virtual"}
!497 = !{i32 152, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPKtPNS_13MemoryManagerEE.virtual"}
!498 = !{i32 160, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!499 = !{i32 168, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvvE.virtual"}
!500 = !{i32 176, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!501 = !{i32 184, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!502 = !{i32 192, !"_ZTSMN11xercesc_2_723AnyURIDatatypeValidatorEKFvPtPNS_13MemoryManagerEE.virtual"}
!503 = !{i32 112, !"_ZTSMN11xercesc_2_713XSerializableEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!504 = !{i32 112, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!505 = !{i32 16, !"_ZTSN11xercesc_2_724BooleanDatatypeValidatorE"}
!506 = !{i32 32, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEKFbvE.virtual"}
!507 = !{i32 40, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!508 = !{i32 48, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!509 = !{i32 56, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEKFbvE.virtual"}
!510 = !{i32 64, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!511 = !{i32 72, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!512 = !{i32 80, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!513 = !{i32 88, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!514 = !{i32 96, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!515 = !{i32 104, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!516 = !{i32 112, !"_ZTSMN11xercesc_2_724BooleanDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!517 = !{!"A", i32 15, !1}
!518 = !{i32 200, !"_ZTSMN11xercesc_2_713XSerializableEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!519 = !{i32 208, !"_ZTSMN11xercesc_2_713XSerializableEFvPNS_11XMLDateTimeEE.virtual"}
!520 = !{i32 216, !"_ZTSMN11xercesc_2_713XSerializableEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!521 = !{i32 200, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!522 = !{i32 208, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFvPNS_11XMLDateTimeEE.virtual"}
!523 = !{i32 216, !"_ZTSMN11xercesc_2_717DatatypeValidatorEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!524 = !{i32 16, !"_ZTSN11xercesc_2_717DateTimeValidatorE"}
!525 = !{i32 32, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFbvE.virtual"}
!526 = !{i32 40, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!527 = !{i32 48, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!528 = !{i32 56, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFbvE.virtual"}
!529 = !{i32 64, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!530 = !{i32 72, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!531 = !{i32 80, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!532 = !{i32 88, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!533 = !{i32 96, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!534 = !{i32 104, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!535 = !{i32 112, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!536 = !{i32 120, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvvE.virtual"}
!537 = !{i32 128, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!538 = !{i32 136, !"_ZTSMN11xercesc_2_717DateTimeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!539 = !{i32 144, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!540 = !{i32 152, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!541 = !{i32 160, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtE.virtual"}
!542 = !{i32 168, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtE.virtual"}
!543 = !{i32 176, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtE.virtual"}
!544 = !{i32 184, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPKtE.virtual"}
!545 = !{i32 192, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!546 = !{i32 200, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!547 = !{i32 208, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFvPNS_11XMLDateTimeEE.virtual"}
!548 = !{i32 216, !"_ZTSMN11xercesc_2_717DateTimeValidatorEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!549 = !{i32 16, !"_ZTSN11xercesc_2_721DateDatatypeValidatorE"}
!550 = !{i32 32, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFbvE.virtual"}
!551 = !{i32 40, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!552 = !{i32 48, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!553 = !{i32 56, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFbvE.virtual"}
!554 = !{i32 64, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!555 = !{i32 72, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!556 = !{i32 80, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!557 = !{i32 88, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!558 = !{i32 96, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!559 = !{i32 104, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!560 = !{i32 112, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!561 = !{i32 120, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvvE.virtual"}
!562 = !{i32 128, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!563 = !{i32 136, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!564 = !{i32 144, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!565 = !{i32 152, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!566 = !{i32 160, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtE.virtual"}
!567 = !{i32 168, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtE.virtual"}
!568 = !{i32 176, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtE.virtual"}
!569 = !{i32 184, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPKtE.virtual"}
!570 = !{i32 192, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!571 = !{i32 200, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!572 = !{i32 208, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFvPNS_11XMLDateTimeEE.virtual"}
!573 = !{i32 216, !"_ZTSMN11xercesc_2_721DateDatatypeValidatorEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!574 = !{i32 200, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!575 = !{i32 208, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFvPNS_11XMLDateTimeEE.virtual"}
!576 = !{i32 216, !"_ZTSMN11xercesc_2_729AbstractNumericFacetValidatorEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!577 = !{!"A", i32 28, !1}
!578 = !{i32 16, !"_ZTSN11xercesc_2_723DoubleDatatypeValidatorE"}
!579 = !{i32 32, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFbvE.virtual"}
!580 = !{i32 40, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!581 = !{i32 48, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!582 = !{i32 56, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFbvE.virtual"}
!583 = !{i32 64, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!584 = !{i32 72, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!585 = !{i32 80, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!586 = !{i32 88, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!587 = !{i32 96, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!588 = !{i32 104, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!589 = !{i32 112, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!590 = !{i32 120, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvvE.virtual"}
!591 = !{i32 128, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!592 = !{i32 136, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!593 = !{i32 144, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!594 = !{i32 152, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!595 = !{i32 160, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtE.virtual"}
!596 = !{i32 168, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtE.virtual"}
!597 = !{i32 176, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtE.virtual"}
!598 = !{i32 184, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPKtE.virtual"}
!599 = !{i32 192, !"_ZTSMN11xercesc_2_723DoubleDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!600 = !{i32 16, !"_ZTSN11xercesc_2_724AbstractNumericValidatorE"}
!601 = !{i32 32, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFbvE.virtual"}
!602 = !{i32 40, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!603 = !{i32 48, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!604 = !{i32 56, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFbvE.virtual"}
!605 = !{i32 64, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!606 = !{i32 72, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!607 = !{i32 80, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!608 = !{i32 88, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!609 = !{i32 96, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!610 = !{i32 104, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!611 = !{i32 112, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!612 = !{i32 120, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvvE.virtual"}
!613 = !{i32 128, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!614 = !{i32 136, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!615 = !{i32 144, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!616 = !{i32 152, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!617 = !{i32 160, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtE.virtual"}
!618 = !{i32 168, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtE.virtual"}
!619 = !{i32 176, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtE.virtual"}
!620 = !{i32 184, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPKtE.virtual"}
!621 = !{i32 192, !"_ZTSMN11xercesc_2_724AbstractNumericValidatorEFvPNS_13MemoryManagerEE.virtual"}
!622 = !{i32 16, !"_ZTSN11xercesc_2_725DurationDatatypeValidatorE"}
!623 = !{i32 32, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFbvE.virtual"}
!624 = !{i32 40, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!625 = !{i32 48, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!626 = !{i32 56, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFbvE.virtual"}
!627 = !{i32 64, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!628 = !{i32 72, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!629 = !{i32 80, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!630 = !{i32 88, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!631 = !{i32 96, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!632 = !{i32 104, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!633 = !{i32 112, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!634 = !{i32 120, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvvE.virtual"}
!635 = !{i32 128, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!636 = !{i32 136, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!637 = !{i32 144, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!638 = !{i32 152, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!639 = !{i32 160, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtE.virtual"}
!640 = !{i32 168, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtE.virtual"}
!641 = !{i32 176, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtE.virtual"}
!642 = !{i32 184, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPKtE.virtual"}
!643 = !{i32 192, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!644 = !{i32 200, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFPNS_11XMLDateTimeEPKtPNS_13MemoryManagerEE.virtual"}
!645 = !{i32 208, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFvPNS_11XMLDateTimeEE.virtual"}
!646 = !{i32 216, !"_ZTSMN11xercesc_2_725DurationDatatypeValidatorEFiPKNS_11XMLDateTimeES3_bE.virtual"}
!647 = !{i32 16, !"_ZTSN11xercesc_2_722FloatDatatypeValidatorE"}
!648 = !{i32 32, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFbvE.virtual"}
!649 = !{i32 40, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvRNS_16XSerializeEngineEE.virtual"}
!650 = !{i32 48, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFPNS_10XProtoTypeEvE.virtual"}
!651 = !{i32 56, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFbvE.virtual"}
!652 = !{i32 64, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFPKNS_16RefArrayVectorOfItEEvE.virtual"}
!653 = !{i32 72, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFPKtS2_PNS_13MemoryManagerEbE.virtual"}
!654 = !{i32 80, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtPNS_17ValidationContextEPNS_13MemoryManagerEE.virtual"}
!655 = !{i32 88, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFbPKNS_17DatatypeValidatorEE.virtual"}
!656 = !{i32 96, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFiPKtS2_PNS_13MemoryManagerEE.virtual"}
!657 = !{i32 104, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFPNS_17DatatypeValidatorEPNS_14RefHashTableOfINS_12KVStringPairEEEPNS_16RefArrayVectorOfItEEiPNS_13MemoryManagerEE.virtual"}
!658 = !{i32 112, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtS2_PNS_13MemoryManagerEE.virtual"}
!659 = !{i32 120, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvvE.virtual"}
!660 = !{i32 128, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!661 = !{i32 136, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEKFvPNS_13MemoryManagerEE.virtual"}
!662 = !{i32 144, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFiPKNS_9XMLNumberES3_E.virtual"}
!663 = !{i32 152, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtPNS_17ValidationContextEbPNS_13MemoryManagerEE.virtual"}
!664 = !{i32 160, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtE.virtual"}
!665 = !{i32 168, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtE.virtual"}
!666 = !{i32 176, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtE.virtual"}
!667 = !{i32 184, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPKtE.virtual"}
!668 = !{i32 192, !"_ZTSMN11xercesc_2_722FloatDatatypeValidatorEFvPNS_13MemoryManagerEE.virtual"}
!669 = !{i32 16, !"_ZTSN11xercesc_2_712XMLMsgLoaderE"}
!670 = !{i32 32, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjE.virtual"}
!671 = !{i32 40, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjPKtS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!672 = !{i32 48, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEFbjPtjPKcS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!673 = !{i32 56, !"_ZTSMN11xercesc_2_712XMLMsgLoaderEKFPKtvE.virtual"}
!674 = !{i32 16, !"_ZTSN11xercesc_2_714InMemMsgLoaderE"}
!675 = !{i32 32, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjE.virtual"}
!676 = !{i32 40, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjPKtS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!677 = !{i32 48, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEFbjPtjPKcS3_S3_S3_PNS_13MemoryManagerEE.virtual"}
!678 = !{i32 56, !"_ZTSMN11xercesc_2_714InMemMsgLoaderEKFPKtvE.virtual"}
!679 = !{!"A", i32 8, !1}
!680 = !{i32 16, !"_ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE"}
!681 = !{i32 32, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvPS1_jE.virtual"}
!682 = !{i32 40, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!683 = !{i32 48, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvjE.virtual"}
!684 = !{i32 56, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!685 = !{i32 64, !"_ZTSMN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!686 = !{i32 16, !"_ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE"}
!687 = !{i32 32, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvPS1_jE.virtual"}
!688 = !{i32 40, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!689 = !{i32 48, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvjE.virtual"}
!690 = !{i32 56, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!691 = !{i32 64, !"_ZTSMN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEEFvvE.virtual"}
!692 = !{!"A", i32 9, !1}
!693 = !{i32 16, !"_ZTSN11xercesc_2_713XMLBigDecimalE"}
!694 = !{i32 32, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFbvE.virtual"}
!695 = !{i32 40, !"_ZTSMN11xercesc_2_713XMLBigDecimalEFvRNS_16XSerializeEngineEE.virtual"}
!696 = !{i32 48, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFPNS_10XProtoTypeEvE.virtual"}
!697 = !{i32 56, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFPtvE.virtual"}
!698 = !{i32 64, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFPtvE.virtual"}
!699 = !{i32 72, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFPKtvE.virtual"}
!700 = !{i32 80, !"_ZTSMN11xercesc_2_713XMLBigDecimalEKFivE.virtual"}
!701 = !{i32 56, !"_ZTSMN11xercesc_2_713XSerializableEKFPtvE.virtual"}
!702 = !{i32 64, !"_ZTSMN11xercesc_2_713XSerializableEKFPtvE.virtual"}
!703 = !{i32 72, !"_ZTSMN11xercesc_2_713XSerializableEKFPKtvE.virtual"}
!704 = !{i32 80, !"_ZTSMN11xercesc_2_713XSerializableEKFivE.virtual"}
!705 = !{i32 16, !"_ZTSN11xercesc_2_79XMLNumberE"}
!706 = !{i32 32, !"_ZTSMN11xercesc_2_79XMLNumberEKFbvE.virtual"}
!707 = !{i32 40, !"_ZTSMN11xercesc_2_79XMLNumberEFvRNS_16XSerializeEngineEE.virtual"}
!708 = !{i32 48, !"_ZTSMN11xercesc_2_79XMLNumberEKFPNS_10XProtoTypeEvE.virtual"}
!709 = !{i32 56, !"_ZTSMN11xercesc_2_79XMLNumberEKFPtvE.virtual"}
!710 = !{i32 64, !"_ZTSMN11xercesc_2_79XMLNumberEKFPtvE.virtual"}
!711 = !{i32 72, !"_ZTSMN11xercesc_2_79XMLNumberEKFPKtvE.virtual"}
!712 = !{i32 80, !"_ZTSMN11xercesc_2_79XMLNumberEKFivE.virtual"}
!713 = !{!"A", i32 11, !1}
!714 = !{i32 16, !"_ZTSN11xercesc_2_711XMLDateTimeE"}
!715 = !{i32 32, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFbvE.virtual"}
!716 = !{i32 40, !"_ZTSMN11xercesc_2_711XMLDateTimeEFvRNS_16XSerializeEngineEE.virtual"}
!717 = !{i32 48, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFPNS_10XProtoTypeEvE.virtual"}
!718 = !{i32 56, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFPtvE.virtual"}
!719 = !{i32 64, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFPtvE.virtual"}
!720 = !{i32 72, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFPKtvE.virtual"}
!721 = !{i32 80, !"_ZTSMN11xercesc_2_711XMLDateTimeEKFivE.virtual"}
!722 = !{i32 16, !"_ZTSN11xercesc_2_712XMLExceptionE"}
!723 = !{i32 32, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPKtvE.virtual"}
!724 = !{i32 40, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPS0_vE.virtual"}
!725 = !{i32 16, !"_ZTSN11xercesc_2_721NumberFormatExceptionE"}
!726 = !{i32 32, !"_ZTSMN11xercesc_2_721NumberFormatExceptionEKFPKtvE.virtual"}
!727 = !{i32 40, !"_ZTSMN11xercesc_2_721NumberFormatExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!728 = !{!"A", i32 6, !1}
!729 = !{i32 16, !"_ZTSN11xercesc_2_722NoSuchElementExceptionE"}
!730 = !{i32 32, !"_ZTSMN11xercesc_2_722NoSuchElementExceptionEKFPKtvE.virtual"}
!731 = !{i32 40, !"_ZTSMN11xercesc_2_722NoSuchElementExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!732 = !{i32 16, !"_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE"}
!733 = !{i32 32, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPKtvE.virtual"}
!734 = !{i32 40, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!735 = !{!"S", %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 1, !736}
!736 = !{!737, i32 2}
!737 = !{!"F", i1 true, i32 0, !339}
!738 = !{!"S", %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 2, !739, !740}
!739 = !{%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 0}
!740 = !{i16 0, i32 1}
!741 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !736}
!742 = !{!"S", %"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1, !363}
!743 = !{!"S", %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 1, !736}
!744 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !339, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !745, !359, !339, !339, !746, !6, !363, !747, !1, !746, !748, !749, !359, !1, !746, !339, !750}
!745 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}
!746 = !{i64 0, i32 0}
!747 = !{!"A", i32 1, !363}
!748 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}
!749 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}
!750 = !{!"A", i32 20, !363}
!751 = !{!"S", %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 6, !736, !339, !1, !339, !740, !360}
!752 = !{!"S", %"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 16, !753, !754, !754, !754, !754, !754, !754, !754, !754, !754, !754, !754, !754, !754, !754, !360}
!753 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!754 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 1}
!755 = !{!"S", %"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 3, !736, !6, !360}
!756 = !{!"S", %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" zeroinitializer, i32 1, !1}
!757 = !{!"S", %"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 10, !758, !363, !363, !339, !339, !339, !759, !759, !760, !360}
!758 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 0}
!759 = !{i32 0, i32 1}
!760 = !{%"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 1}
!761 = !{!"S", %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 1, !736}
!762 = !{!"S", %"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 2, !763, !360}
!763 = !{%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 0}
!764 = !{!"S", %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 1, !736}
!765 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 10, !763, !339, !339, !740, !740, !740, !740, !740, !740, !360}
!766 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 6, !736, !339, !767, !768, !360, !339}
!767 = !{!"A", i32 4, !363}
!768 = !{%"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 1}
!769 = !{!"S", %"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1, !736}
!770 = !{!"S", %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 6, !763, !360, !771, !772, !339, !339}
!771 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2}
!772 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!773 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 7, !763, !360, !774, !339, !339, !363, !775}
!774 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 1}
!775 = !{!"A", i32 7, !363}
!776 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 6, !763, !360, !774, !339, !339, !363}
!777 = !{!"S", %"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 9, !763, !339, !339, !339, !339, !740, !740, !740, !360}
!778 = !{!"S", %"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 11, !763, !360, !774, !779, !780, !780, !339, !363, !363, !339, !339}
!779 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 1}
!780 = !{%"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 1}
!781 = !{!"S", %"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 10, !763, !339, !339, !339, !363, !363, !339, !740, !740, !360}
!782 = !{!"S", %"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1, !736}
!783 = !{!"S", %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1, !736}
!784 = !{!"S", %"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1, !736}
!785 = !{!"S", %"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1, !736}
!786 = !{!"S", %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 72, !787, !746, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !363, !339, !339, !339, !339, !339, !339, !339, !339, !788, !339, !339, !339, !339, !339, !789, !790, !791, !792, !793, !794, !795, !796, !797, !363, !798, !799, !339, !800, !360, !801, !801, !802, !740, !740, !740, !803, !339, !360, !804, !805, !805, !805, !805, !805, !805, !805, !806}
!787 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 0}
!788 = !{i32 0, i32 2}
!789 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!790 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!791 = !{%"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1}
!792 = !{%"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1}
!793 = !{%"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1}
!794 = !{%"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1}
!795 = !{%"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1}
!796 = !{%"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1}
!797 = !{%"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 1}
!798 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 0}
!799 = !{%"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 1}
!800 = !{%"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 1}
!801 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1}
!802 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 1}
!803 = !{%"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 1}
!804 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 0}
!805 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 0}
!806 = !{%"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 0}
!807 = !{!"S", %"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1, !736}
!808 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !809}
!809 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!810 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !811, !360}
!811 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 2}
!812 = !{!"S", %"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 11, !363, !813, !339, !339, !767, !740, !774, !360, !814, !363, !775}
!813 = !{!"A", i32 3, !363}
!814 = !{%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 1}
!815 = !{!"S", %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 11, !816, !817, !818, !793, !819, !339, !820, !363, !339, !363, !360}
!816 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 0}
!817 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 1}
!818 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 1}
!819 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!820 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!821 = !{!"S", %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 5, !736, !822, !794, !823, !824}
!822 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 1}
!823 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 1}
!824 = !{%"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 1}
!825 = !{!"S", %"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 3, !339, !360, !826}
!826 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 2}
!827 = !{!"S", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 7, !339, !339, !339, !363, !360, !828, !740}
!828 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1}
!829 = !{!"S", %"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 12, !363, !363, !363, !802, !830, !830, !831, !360, !832, !768, !768, !833}
!830 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!831 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 1}
!832 = !{%"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 1}
!833 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!834 = !{!"S", %"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 4, !736, !360, !363, !775}
!835 = !{!"S", %"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 3, !736, !339, !767}
!836 = !{!"S", %"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 13, !339, !339, !837, !838, !339, !339, !339, !339, !339, !339, !339, !839, !360}
!837 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 0}
!838 = !{%"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 2}
!839 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!840 = !{!"S", %"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 8, !763, !339, !339, !740, !740, !740, !740, !360}
!841 = !{!"S", %"class._ZTSN11xercesc_2_716XSerializeEngineE.xercesc_2_7::XSerializeEngine" zeroinitializer, i32 16, !6, !6, !767, !832, !842, !843, !746, !746, !1, !1, !1, !1, !844, !845, !339, !767}
!842 = !{%"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1}
!843 = !{%"class._ZTSN11xercesc_2_715BinOutputStreamE.xercesc_2_7::BinOutputStream" zeroinitializer, i32 1}
!844 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!845 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPvEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!846 = !{!"S", %"class._ZTSN11xercesc_2_710XProtoTypeE.xercesc_2_7::XProtoType" zeroinitializer, i32 2, !1, !847}
!847 = !{!848, i32 1}
!848 = !{!"F", i1 false, i32 1, !849, !360}
!849 = !{%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 1}
!850 = !{!"S", %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1, !851}
!851 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 0}
!852 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 7, !736, !339, !767, !768, !360, !339, !767}
!853 = !{!"S", %"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 14, !360, !854, !855, !856, !858, !802, !860, !861, !862, !855, !768, !363, !363, !863}
!854 = !{%"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1}
!855 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!856 = !{!"A", i32 14, !857}
!857 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!858 = !{!"A", i32 14, !859}
!859 = !{%"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 1}
!860 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!861 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!862 = !{%"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 1}
!863 = !{!"A", i32 6, !363}
!864 = !{!"S", %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 19, !763, !363, !363, !363, !363, !6, !339, !339, !339, !339, !339, !814, !865, !740, !866, !740, !740, !740, !360}
!865 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!866 = !{%"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 1}
!867 = !{!"S", %"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1, !868}
!868 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!869 = !{!"S", %"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 24, !870, !813, !339, !339, !339, !339, !339, !339, !339, !339, !767, !740, !871, !872, !871, !814, !873, !874, !875, !814, !363, !363, !363, !876}
!870 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 0}
!871 = !{%"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 1}
!872 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!873 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!874 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 1}
!875 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 1}
!876 = !{!"A", i32 5, !363}
!877 = !{!"S", %"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2, !339, !740}
!878 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !879, !339, !339, !339, !880}
!879 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!880 = !{%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1}
!881 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !360, !363, !882, !339, !339, !880}
!882 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!883 = !{!"S", %"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 2, !736, !360}
!884 = !{!"S", %"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1, !763}
!885 = !{!"S", %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 32, !339, !886, !339, !887, !888, !746, !746, !339, !740, !363, !363, !740, !339, !889, !339, !339, !339, !363, !339, !339, !363, !363, !740, !842, !363, !363, !890, !339, !1, !363, !339, !360}
!886 = !{!"A", i32 16384, !6}
!887 = !{!"A", i32 16384, !363}
!888 = !{!"A", i32 16384, !339}
!889 = !{!"A", i32 49152, !363}
!890 = !{%"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 1}
!891 = !{!"S", %"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 17, !779, !339, !339, !339, !892, !893, !339, !339, !363, !363, !363, !339, !801, !339, !740, !339, !339}
!892 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 2}
!893 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 1}
!894 = !{!"S", %"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1, !736}
!895 = !{!"S", %"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 15, !363, !363, !339, !339, !339, !339, !896, !740, !740, !897, !754, !760, !898, !899, !360}
!896 = !{%"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 1}
!897 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 1}
!898 = !{%"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 0}
!899 = !{%"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 1}
!900 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !901, !360}
!901 = !{i16 0, i32 2}
!902 = !{!"S", %"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 30, !763, !363, !363, !363, !363, !363, !339, !339, !339, !339, !339, !339, !339, !339, !740, !740, !740, !814, !814, !871, !780, !874, !903, !904, !872, !905, !740, !759, !906, !360}
!903 = !{%"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 1}
!904 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!905 = !{%"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1}
!906 = !{%"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 1}
!907 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !360, !363, !908, !339, !339, !880}
!908 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!909 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !910}
!910 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!911 = !{!"S", %"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 11, !912, !339, !339, !339, !339, !774, !814, !814, !814, !913, !874}
!912 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 0}
!913 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!914 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !915, !916, !1}
!915 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 1}
!916 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!917 = !{!"S", %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1, !736}
!918 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !919, !920, !1, !339, !767}
!919 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 1}
!920 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!921 = !{!"S", %"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2, !339, !339}
!922 = !{!"S", %"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 6, !363, !339, !759, !740, !740, !360}
!923 = !{!"S", %"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 4, !736, !360, !6, !897}
!924 = !{!"S", %"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 2, !925, !360}
!925 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!926 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !927}
!927 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!928 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !874, !929, !1, !339, !767}
!929 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!930 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !931, !360}
!931 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 2}
!932 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !759, !360}
!933 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !934}
!934 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!935 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !936, !360}
!936 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 2}
!937 = !{!"S", %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 8, !763, !740, !740, !938, !939, !360, !339, !767}
!938 = !{%"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 1}
!939 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!940 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !941, !360}
!941 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 2}
!942 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !943}
!943 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!944 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !945, !360}
!945 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 2}
!946 = !{!"S", %"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 3, !763, !947, !948}
!947 = !{%"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 1}
!948 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 1}
!949 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !759, !360}
!950 = !{!"S", %"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" zeroinitializer, i32 12, !951, !363, !363, !363, !363, !363, !952, !952, !952, !952, !953, !854}
!951 = !{%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 0}
!952 = !{%"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber" zeroinitializer, i32 1}
!953 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLNumberEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!954 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !955, !339, !339, !339, !880}
!955 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!956 = !{!"S", %"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber" zeroinitializer, i32 1, !763}
!957 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLNumberEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !958}
!958 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLNumberEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!959 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLNumberEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !960, !360}
!960 = !{%"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber" zeroinitializer, i32 2}
!961 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !962, !963, !1}
!962 = !{%"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 1}
!963 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!964 = !{!"S", %"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 6, !763, !746, !746, !740, !740, !360}
!965 = !{!"S", %"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException" zeroinitializer, i32 1, !851}
!966 = !{!"S", %"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" zeroinitializer, i32 1, !851}
!967 = !{!"S", %"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" zeroinitializer, i32 1, !968}
!968 = !{%"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" zeroinitializer, i32 0}
!969 = !{!"S", %"class._ZTSN11xercesc_2_723AbstractStringValidatorE.xercesc_2_7::AbstractStringValidator" zeroinitializer, i32 6, !951, !339, !339, !339, !363, !854}
!970 = !{!"S", %"class._ZTSN11xercesc_2_730AnySimpleTypeDatatypeValidatorE.xercesc_2_7::AnySimpleTypeDatatypeValidator" zeroinitializer, i32 1, !951}
!971 = !{!"S", %"class._ZTSN11xercesc_2_723AnyURIDatatypeValidatorE.xercesc_2_7::AnyURIDatatypeValidator" zeroinitializer, i32 1, !972}
!972 = !{%"class._ZTSN11xercesc_2_723AbstractStringValidatorE.xercesc_2_7::AbstractStringValidator" zeroinitializer, i32 0}
!973 = !{!"S", %"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1, !736}
!974 = !{!"S", %"class._ZTSN11xercesc_2_715BinOutputStreamE.xercesc_2_7::BinOutputStream" zeroinitializer, i32 1, !736}
!975 = !{!"S", %"class._ZTSN11xercesc_2_724BooleanDatatypeValidatorE.xercesc_2_7::BooleanDatatypeValidator" zeroinitializer, i32 1, !951}
!976 = !{!"S", %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" zeroinitializer, i32 3, !977, !342, !342}
!977 = !{!978, i32 1}
!978 = !{!"F", i1 false, i32 0, !979}
!979 = !{!"void", i32 0}
!980 = !{!"S", %"class._ZTSN11xercesc_2_710CMStateSetE.xercesc_2_7::CMStateSet" zeroinitializer, i32 6, !339, !339, !339, !339, !1, !360}
!981 = !{!"S", %"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 5, !816, !746, !746, !740, !740}
!982 = !{!"S", %"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 6, !983, !984, !872, !985, !339, !339}
!983 = !{%"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 0}
!984 = !{%"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 1}
!985 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 2}
!986 = !{!"S", %"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 7, !987, !363, !929, !339, !872, !360, !1}
!987 = !{%"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 0}
!988 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 1, !736}
!989 = !{!"S", %"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 18, !990, !740, !991, !991, !991, !992, !993, !994, !995, !996, !997, !998, !797, !360, !999, !1000, !363, !1001}
!990 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 0}
!991 = !{%"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 1}
!992 = !{%"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 1}
!993 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!994 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!995 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!996 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!997 = !{%"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 1}
!998 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!999 = !{%"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1}
!1000 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!1001 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 0}
!1002 = !{!"S", %"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 3, !763, !1003, !360}
!1003 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!1004 = !{!"S", %"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 8, !360, !363, !1005, !339, !880, !936, !339, !339}
!1005 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 2}
!1006 = !{!"S", %"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 7, !360, !1007, !1008, !339, !339, !339, !767}
!1007 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2}
!1008 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 2}
!1009 = !{!"S", %"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1, !1010}
!1010 = !{%"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 0}
!1011 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1012, !339, !339, !339, !880}
!1012 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1013 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1014, !339, !339, !339, !880}
!1014 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1015 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 5, !875, !1016, !1, !339, !339}
!1016 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 1}
!1017 = !{!"S", %"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2, !1018, !1019}
!1018 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 1}
!1019 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 1}
!1020 = !{!"S", %"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 2, !763, !360}
!1021 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1022, !1023, !1}
!1022 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 1}
!1023 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1024 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !814, !1025, !1}
!1025 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1026 = !{!"S", %"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 7, !763, !1027, !740, !1022, !740, !339, !339}
!1027 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 0}
!1028 = !{!"S", %"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 5, !763, !339, !740, !1029, !360}
!1029 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1030 = !{!"S", %"class._ZTSN11xercesc_2_714HashCMStateSetE.xercesc_2_7::HashCMStateSet" zeroinitializer, i32 1, !1031}
!1031 = !{%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 0}
!1032 = !{!"S", %"class._ZTSN11xercesc_2_77HashPtrE.xercesc_2_7::HashPtr" zeroinitializer, i32 1, !1031}
!1033 = !{!"S", %"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1, !736}
!1034 = !{!"S", %"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 4, !736, !339, !740, !360}
!1035 = !{!"S", %"class._ZTSN11xercesc_2_722FloatDatatypeValidatorE.xercesc_2_7::FloatDatatypeValidator" zeroinitializer, i32 1, !1036}
!1036 = !{%"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" zeroinitializer, i32 0}
!1037 = !{!"S", %"class._ZTSN11xercesc_2_723DoubleDatatypeValidatorE.xercesc_2_7::DoubleDatatypeValidator" zeroinitializer, i32 1, !1036}
!1038 = !{!"S", %"class._ZTSN11xercesc_2_724DecimalDatatypeValidatorE.xercesc_2_7::DecimalDatatypeValidator" zeroinitializer, i32 3, !1036, !339, !339}
!1039 = !{!"S", %"class._ZTSN11xercesc_2_725DurationDatatypeValidatorE.xercesc_2_7::DurationDatatypeValidator" zeroinitializer, i32 1, !1040}
!1040 = !{%"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" zeroinitializer, i32 0}
!1041 = !{!"S", %"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" zeroinitializer, i32 1, !968}
!1042 = !{!"S", %"class._ZTSN11xercesc_2_721DateDatatypeValidatorE.xercesc_2_7::DateDatatypeValidator" zeroinitializer, i32 1, !1040}
!1043 = !{!"S", %"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime" zeroinitializer, i32 10, !1044, !338, !1045, !339, !339, !339, !1046, !363, !740, !360}
!1044 = !{%"class._ZTSN11xercesc_2_79XMLNumberE.xercesc_2_7::XMLNumber" zeroinitializer, i32 0}
!1045 = !{!"A", i32 2, !339}
!1046 = !{double 0.000000e+00, i32 0}
!1047 = !{!"S", %"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal" zeroinitializer, i32 8, !1044, !339, !339, !339, !339, !740, !740, !360}
!1048 = !{!"S", %"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1, !1049}
!1049 = !{%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 0}
!1050 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !1051, !360}
!1051 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2}
!1052 = !{!"S", %"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf" zeroinitializer, i32 4, !360, !1053, !339, !880}
!1053 = !{%"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 2}
!1054 = !{!"S", %"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 3, !363, !1055, !1}
!1055 = !{%"struct._ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE.xercesc_2_7::ValueHashTableBucketElem" zeroinitializer, i32 1}
!1056 = !{!"S", %"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator" zeroinitializer, i32 4, !1057, !1058, !1059, !360}
!1057 = !{%"class._ZTSN11xercesc_2_715ValueStoreCacheE.xercesc_2_7::ValueStoreCache" zeroinitializer, i32 1}
!1058 = !{%"class._ZTSN11xercesc_2_717XPathMatcherStackE.xercesc_2_7::XPathMatcherStack" zeroinitializer, i32 1}
!1059 = !{%"class._ZTSN11xercesc_2_716ValueHashTableOfIbEE.xercesc_2_7::ValueHashTableOf" zeroinitializer, i32 1}
!1060 = !{!"S", %"class._ZTSN11xercesc_2_715ValueStoreCacheE.xercesc_2_7::ValueStoreCache" zeroinitializer, i32 6, !1061, !1062, !1063, !1064, !824, !360}
!1061 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_10ValueStoreEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1062 = !{%"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!1063 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!1064 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!1065 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !360, !363, !1066, !339, !339, !880}
!1066 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!1067 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !1068, !1069, !1, !339, !767}
!1068 = !{%"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 1}
!1069 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!1070 = !{!"S", %"class._ZTSN11xercesc_2_717XPathMatcherStackE.xercesc_2_7::XPathMatcherStack" zeroinitializer, i32 3, !339, !1071, !1072}
!1071 = !{%"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 1}
!1072 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1073 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1074, !360}
!1074 = !{%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 2}
!1075 = !{!"S", %"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 9, !736, !339, !759, !759, !759, !1076, !1029, !948, !360}
!1076 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1077 = !{!"S", %"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 8, !363, !339, !948, !1078, !1079, !1068, !824, !360}
!1078 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 0}
!1079 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1080 = !{!"S", %"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 2, !1081, !360}
!1081 = !{%"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!1082 = !{!"S", %"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 6, !360, !363, !1083, !339, !339, !880}
!1083 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1084 = !{!"S", %"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 2, !1085, !1086}
!1085 = !{!"A", i32 8, !363}
!1086 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIiEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 0}
!1087 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1088}
!1088 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XPathMatcherEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1089 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1090}
!1090 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1091 = !{!"S", %"__SOADT_AR_class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !1092, !360}
!1092 = !{%"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 1}
!1093 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1068, !1094, !1}
!1094 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_10ValueStoreEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1095 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12ValueStackOfIiEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1096, !360}
!1096 = !{%"class._ZTSN11xercesc_2_712ValueStackOfIiEE.xercesc_2_7::ValueStackOf" zeroinitializer, i32 2}
!1097 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1098, !339, !339, !339, !880}
!1098 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1099 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !801, !1100, !1}
!1100 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1101 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !1102, !360}
!1102 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 2}
!1103 = !{!"S", %"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" zeroinitializer, i32 4, !1104, !1068, !1105, !1106}
!1104 = !{%"class._ZTSN11xercesc_2_712XPathMatcherE.xercesc_2_7::XPathMatcher" zeroinitializer, i32 0}
!1105 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 1}
!1106 = !{%"class._ZTSN11xercesc_2_714FieldActivatorE.xercesc_2_7::FieldActivator" zeroinitializer, i32 1}
!1107 = !{!"S", %"class._ZTSN11xercesc_2_76IC_KeyE.xercesc_2_7::IC_Key" zeroinitializer, i32 2, !1108, !767}
!1108 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base" zeroinitializer, i32 0}
!1109 = !{!"S", %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint.base" zeroinitializer, i32 7, !763, !740, !740, !938, !939, !360, !339}
!1110 = !{!"S", %"class._ZTSN11xercesc_2_79IC_KeyRefE.xercesc_2_7::IC_KeyRef" zeroinitializer, i32 2, !1108, !948}
!1111 = !{!"S", %"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 3, !763, !947, !948}
!1112 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1113, !339, !339, !339, !880}
!1113 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1114 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1115, !1116, !1}
!1115 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 1}
!1116 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1117 = !{!"S", %"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1, !1118}
!1118 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 0}
!1119 = !{!"S", %"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 6, !339, !339, !339, !837, !1120, !360}
!1120 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 2}
!1121 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 3, !1122, !339, !339}
!1122 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 1}
!1123 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 2, !339, !339}
!1124 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1125}
!1125 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1126 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1127, !360}
!1127 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 2}
!1128 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !1085, !1129}
!1129 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!1130 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1131}
!1131 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1132 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1133, !360}
!1133 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 2}
!1134 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !1085, !1135}
!1135 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!1136 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1137}
!1137 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1138 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1139, !360}
!1139 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 2}
!1140 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1141, !339, !339, !339, !880}
!1141 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1142 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !871, !1143, !1}
!1143 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1144 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1145, !339, !339, !339, !880}
!1145 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1146 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1147, !1148, !1}
!1147 = !{%"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 1}
!1148 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1149 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1150, !339, !339, !339, !880}
!1150 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1151 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1152, !1153, !1}
!1152 = !{%"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 1}
!1153 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1154 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !360, !363, !1155, !339, !339, !880}
!1155 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!1156 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !1157, !1158, !1, !339, !767}
!1157 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!1158 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!1159 = !{!"S", %"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 9, !763, !363, !339, !339, !339, !780, !904, !1147, !906}
!1160 = !{!"S", %"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 8, !763, !363, !339, !339, !1161, !1161, !874, !360}
!1161 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1162 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !936, !360}
!1163 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1164}
!1164 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1165 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !985, !360}
!1166 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1167}
!1167 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1168 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1169, !360}
!1169 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 2}
!1170 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_10ValueStoreEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1171}
!1171 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1172 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10ValueStoreEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1173, !360}
!1173 = !{%"__DFDT___SOADT__DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 2}
!1174 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !1085, !1175}
!1175 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!1176 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1177}
!1177 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1178 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_14RefHashTableOfINS_10ValueStoreEEEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1179, !360}
!1179 = !{%"__DFT_class._ZTSN11xercesc_2_714RefHashTableOfINS_10ValueStoreEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 2}
!1180 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1181}
!1181 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1182 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1183, !360}
!1183 = !{%"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2}
!1184 = !{!"S", %"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2, !763, !1185}
!1185 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!1186 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1187, !360}
!1187 = !{%"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 2}
!1188 = !{!"S", %"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 3, !763, !6, !1189}
!1189 = !{%"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 1}
!1190 = !{!"S", %"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 3, !763, !6, !774}
!1191 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1192}
!1192 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1193 = !{!"S", %"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 7, !360, !1194, !768, !858, !860, !1195, !740}
!1194 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 1}
!1195 = !{!"A", i32 14, !1196}
!1196 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!1197 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1198}
!1198 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1199 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1200, !360}
!1200 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 2}
!1201 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1202}
!1202 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1203 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1204, !360}
!1204 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 2}
!1205 = !{!"S", %"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 4, !360, !802, !857, !1206}
!1206 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!1207 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !1208}
!1208 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!1209 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !736, !363, !339, !339, !1210, !360}
!1210 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 2}
!1211 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !360, !363, !1212, !339, !339, !880}
!1212 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!1213 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1214, !339, !339, !339, !880}
!1214 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1215 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1216, !1217, !1}
!1216 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 1}
!1217 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1218 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1219, !339, !339, !339, !880}
!1219 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1220 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1221, !1222, !1}
!1221 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 1}
!1222 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1223 = !{!"S", %"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 3, !360, !1196, !857}
!1224 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !1216, !1225, !1, !339, !767}
!1225 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!1226 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !360, !363, !1227, !339, !339, !339, !880}
!1227 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!1228 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !1229, !1230, !1}
!1229 = !{%"class._ZTSN11xercesc_2_719XSerializedObjectIdE.xercesc_2_7::XSerializedObjectId" zeroinitializer, i32 1}
!1230 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_19XSerializedObjectIdEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!1231 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPvEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !363, !339, !339, !1232, !360}
!1232 = !{i8 0, i32 2}
!1233 = !{!"S", %"class._ZTSN11xercesc_2_719XSerializedObjectIdE.xercesc_2_7::XSerializedObjectId" zeroinitializer, i32 1, !339}
!1234 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1, !1118}
!1235 = !{!"S", %"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1, !1236}
!1236 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}
!1237 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !736}
!1238 = !{!"S", %"__SOADT_EL__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 3, !1105, !814, !740}
!1239 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 -1}
!1240 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 -1}
!1241 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 -1}
!1242 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!1243 = !{i32 1, !"wchar_size", i32 4}
!1244 = !{i32 1, !"Virtual Function Elim", i32 0}
!1245 = !{i32 7, !"uwtable", i32 2}
!1246 = !{i32 1, !"ThinLTO", i32 0}
!1247 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!1248 = !{i32 1, !"LTOPostLink", i32 1}
!1249 = distinct !{!1, !1}
!1250 = distinct !{!1}
!1251 = distinct !{!1, !1, !1}
!1252 = distinct !{!1}
!1253 = distinct !{!1}
!1254 = distinct !{!1255, !1, !360}
!1255 = !{%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1}
!1256 = !{!1257, !1257, i64 0}
!1257 = !{!"vtable pointer", !1258, i64 0}
!1258 = !{!"Simple C++ TBAA"}
!1259 = !{!1260, !1261, i64 8}
!1260 = !{!"struct@_ZTSN11xercesc_2_712XMLExceptionE", !1261, i64 8, !1263, i64 16, !1264, i64 24, !1265, i64 32, !1266, i64 40}
!1261 = !{!"_ZTSN11xercesc_2_710XMLExcepts5CodesE", !1262, i64 0}
!1262 = !{!"omnipotent char", !1258, i64 0}
!1263 = !{!"pointer@_ZTSPc", !1262, i64 0}
!1264 = !{!"int", !1262, i64 0}
!1265 = !{!"pointer@_ZTSPt", !1262, i64 0}
!1266 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !1262, i64 0}
!1267 = !{!1260, !1263, i64 16}
!1268 = !{!1260, !1264, i64 24}
!1269 = !{!1260, !1265, i64 32}
!1270 = !{!1260, !1266, i64 40}
!1271 = !{!1266, !1266, i64 0}
!1272 = !{!"F", i1 false, i32 2, !1, !360, !746}
!1273 = !{!"_Intel.Devirt.Call"}
!1274 = distinct !{!1275}
!1275 = !{%"class._ZTSN11xercesc_2_729AbstractNumericFacetValidatorE.xercesc_2_7::AbstractNumericFacetValidator" zeroinitializer, i32 1}
!1276 = distinct !{!1275}
!1277 = distinct !{!1275}
!1278 = distinct !{!1275, !1279}
!1279 = !{%"class._ZTSN11xercesc_2_716XSerializeEngineE.xercesc_2_7::XSerializeEngine" zeroinitializer, i32 1}
!1280 = distinct !{!1281, !1275}
!1281 = !{%"class._ZTSN11xercesc_2_710XProtoTypeE.xercesc_2_7::XProtoType" zeroinitializer, i32 1}
!1282 = distinct !{!814}
!1283 = distinct !{!854, !1275}
!1284 = distinct !{!814, !814}
!1285 = distinct !{!814, !740, !740, !360}
!1286 = !{!1287, !1287, i64 0}
!1287 = !{!"short", !1262, i64 0}
!1288 = distinct !{!1288, !1289}
!1289 = !{!"llvm.loop.mustprogress"}
!1290 = distinct !{!1290, !1289}
!1291 = distinct !{!1275, !740, !740, !360}
!1292 = distinct !{!1275}
!1293 = distinct !{!1275, !360}
!1294 = distinct !{!1275, !360}
!1295 = distinct !{!1296, !1, !360}
!1296 = !{%"class._ZTSN11xercesc_2_722NoSuchElementExceptionE.xercesc_2_7::NoSuchElementException" zeroinitializer, i32 1}
!1297 = distinct !{!740, !1298, !740, !360}
!1298 = !{%"class._ZTSN11xercesc_2_724AbstractNumericValidatorE.xercesc_2_7::AbstractNumericValidator" zeroinitializer, i32 1}
!1299 = distinct !{!1298, !740, !797, !360}
!1300 = distinct !{!854, !1301}
!1301 = !{%"class._ZTSN11xercesc_2_723AbstractStringValidatorE.xercesc_2_7::AbstractStringValidator" zeroinitializer, i32 1}
!1302 = distinct !{!1301, !740, !797, !360}
!1303 = distinct !{!1301, !740, !740, !360}
!1304 = distinct !{!1301, !740, !740, !360}
!1305 = distinct !{!1301}
!1306 = distinct !{!1301, !360}
!1307 = distinct !{!1301, !740, !360}
!1308 = distinct !{!1301, !740, !360}
!1309 = distinct !{!1301, !360}
!1310 = distinct !{!1301}
!1311 = distinct !{!1301, !740, !797, !360}
!1312 = distinct !{!1301, !360}
!1313 = distinct !{!1301, !740, !360}
!1314 = distinct !{!1315}
!1315 = !{%"class._ZTSN11xercesc_2_730AnySimpleTypeDatatypeValidatorE.xercesc_2_7::AnySimpleTypeDatatypeValidator" zeroinitializer, i32 1}
!1316 = distinct !{!1315}
!1317 = distinct !{!1315}
!1318 = distinct !{!1315, !1279}
!1319 = distinct !{!1281, !1315}
!1320 = distinct !{!1315}
!1321 = distinct !{!854, !1315}
!1322 = distinct !{!1315, !740, !797, !360}
!1323 = distinct !{!1315, !814}
!1324 = distinct !{!1315, !740, !740, !360}
!1325 = distinct !{!814, !1315, !865, !854, !360}
!1326 = distinct !{!1327}
!1327 = !{%"class._ZTSN11xercesc_2_723AnyURIDatatypeValidatorE.xercesc_2_7::AnyURIDatatypeValidator" zeroinitializer, i32 1}
!1328 = distinct !{!1327}
!1329 = distinct !{!1327}
!1330 = distinct !{!1327, !1279}
!1331 = distinct !{!1281, !1327}
!1332 = distinct !{!814, !1327, !865, !854, !360}
!1333 = distinct !{!1327, !740, !360}
!1334 = distinct !{!1}
!1335 = distinct !{!1336}
!1336 = !{%"class._ZTSN11xercesc_2_724BooleanDatatypeValidatorE.xercesc_2_7::BooleanDatatypeValidator" zeroinitializer, i32 1}
!1337 = distinct !{!1336}
!1338 = distinct !{!1336, !1279}
!1339 = distinct !{!1281, !1336}
!1340 = distinct !{!854, !1336}
!1341 = distinct !{!740, !1336, !740, !360}
!1342 = distinct !{!1336, !740, !797, !360}
!1343 = distinct !{!1336, !740, !740, !360}
!1344 = distinct !{!1344, !1289}
!1345 = distinct !{!814, !1336, !865, !854, !360}
!1346 = distinct !{!1336, !740, !797, !360}
!1347 = distinct !{!1348, !1, !360}
!1348 = !{%"class._ZTSN11xercesc_2_714HashCMStateSetE.xercesc_2_7::HashCMStateSet" zeroinitializer, i32 1}
!1349 = distinct !{!1348, !1, !1}
!1350 = distinct !{!814}
!1351 = distinct !{!740, !814, !740, !360}
!1352 = distinct !{!1353}
!1353 = !{%"class._ZTSN11xercesc_2_721DateDatatypeValidatorE.xercesc_2_7::DateDatatypeValidator" zeroinitializer, i32 1}
!1354 = distinct !{!1353}
!1355 = distinct !{!1353}
!1356 = distinct !{!1353, !1279}
!1357 = distinct !{!1281, !1353}
!1358 = distinct !{!740, !1353, !740, !360}
!1359 = distinct !{!814, !1353, !865, !854, !360}
!1360 = distinct !{!1361, !1353, !740, !360}
!1361 = !{%"class._ZTSN11xercesc_2_711XMLDateTimeE.xercesc_2_7::XMLDateTime" zeroinitializer, i32 1}
!1362 = distinct !{!1353, !1361}
!1363 = distinct !{!1364, !740, !797, !360}
!1364 = !{%"class._ZTSN11xercesc_2_717DateTimeValidatorE.xercesc_2_7::DateTimeValidator" zeroinitializer, i32 1}
!1365 = distinct !{!1364, !740, !740, !360}
!1366 = !{!"F", i1 false, i32 3, !1361, !1364, !740, !360}
!1367 = !{i32 -1, i32 3}
!1368 = !{!"F", i1 false, i32 4, !339, !1364, !1361, !1361, !1369}
!1369 = !{i1 false, i32 0}
!1370 = !{i32 -1, i32 2}
!1371 = !{!"F", i1 false, i32 1, !979, !1361}
!1372 = distinct !{!1364, !952, !952}
!1373 = distinct !{!1364, !740, !797, !360}
!1374 = distinct !{!1364, !740}
!1375 = distinct !{!1364, !740}
!1376 = distinct !{!1364, !740}
!1377 = distinct !{!1364, !740}
!1378 = distinct !{!1364, !360}
!1379 = distinct !{!1364, !1361, !1361}
!1380 = !{!"_Intel.Devirt.Target"}
!1381 = !{!1382, !1264, i64 8}
!1382 = !{!"struct@_ZTSN11xercesc_2_711XMLDateTimeE", !1383, i64 0, !1385, i64 8, !1386, i64 40, !1264, i64 48, !1264, i64 52, !1264, i64 56, !1387, i64 64, !1388, i64 72, !1265, i64 80, !1266, i64 88}
!1383 = !{!"struct@_ZTSN11xercesc_2_79XMLNumberE", !1384, i64 0}
!1384 = !{!"struct@_ZTSN11xercesc_2_713XSerializableE"}
!1385 = !{!"array@_ZTSA8_i", !1264, i64 0}
!1386 = !{!"array@_ZTSA2_i", !1264, i64 0}
!1387 = !{!"double", !1262, i64 0}
!1388 = !{!"bool", !1262, i64 0}
!1389 = distinct !{!1390, !740, !740, !360}
!1390 = !{%"class._ZTSN11xercesc_2_724DecimalDatatypeValidatorE.xercesc_2_7::DecimalDatatypeValidator" zeroinitializer, i32 1}
!1391 = !{!1392, !1264, i64 8}
!1392 = !{!"struct@_ZTSN11xercesc_2_713XMLBigDecimalE", !1383, i64 0, !1264, i64 8, !1264, i64 12, !1264, i64 16, !1264, i64 20, !1265, i64 24, !1265, i64 32, !1266, i64 40}
!1393 = !{!1392, !1264, i64 12}
!1394 = !{!1392, !1264, i64 16}
!1395 = !{!1392, !1265, i64 32}
!1396 = !{!1392, !1265, i64 24}
!1397 = !{!1392, !1266, i64 40}
!1398 = !{!"F", i1 false, i32 2, !979, !360, !1}
!1399 = distinct !{!1400}
!1400 = !{%"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1}
!1401 = !{!1402, !1402, i64 0}
!1402 = !{!"pointer@_ZTSP8_IO_FILE", !1262, i64 0}
!1403 = distinct !{!359, !1}
!1404 = distinct !{!1405}
!1405 = !{%"class._ZTSN11xercesc_2_723DoubleDatatypeValidatorE.xercesc_2_7::DoubleDatatypeValidator" zeroinitializer, i32 1}
!1406 = distinct !{!1405}
!1407 = distinct !{!1405}
!1408 = distinct !{!1405, !1279}
!1409 = distinct !{!1281, !1405}
!1410 = distinct !{!1405, !740, !740, !360}
!1411 = distinct !{!814, !1405, !865, !854, !360}
!1412 = distinct !{!1405, !952, !952}
!1413 = distinct !{!1405, !740, !797, !360}
!1414 = distinct !{!1405, !740}
!1415 = distinct !{!1405, !740}
!1416 = distinct !{!1405, !740}
!1417 = distinct !{!1405, !740}
!1418 = distinct !{!1405, !360}
!1419 = distinct !{!1420}
!1420 = !{%"class._ZTSN11xercesc_2_725DurationDatatypeValidatorE.xercesc_2_7::DurationDatatypeValidator" zeroinitializer, i32 1}
!1421 = distinct !{!1420}
!1422 = distinct !{!1420}
!1423 = distinct !{!1420, !1279}
!1424 = distinct !{!1281, !1420}
!1425 = distinct !{!814, !1420, !865, !854, !360}
!1426 = distinct !{!1361, !1420, !740, !360}
!1427 = distinct !{!1420, !1361}
!1428 = distinct !{!1420, !1361, !1361}
!1429 = distinct !{!1430, !1430}
!1430 = !{%"__DFT___SOADT__DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 1}
!1431 = !{!1432, !1433, i64 0}
!1432 = !{!"struct@_ZTSN11xercesc_2_713FieldValueMapE", !1433, i64 0, !1434, i64 8, !1435, i64 16, !1266, i64 24}
!1433 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !1262, i64 0}
!1434 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !1262, i64 0}
!1435 = !{!"pointer@_ZTSPN11xercesc_2_716RefArrayVectorOfItEE", !1262, i64 0}
!1436 = !{!1432, !1435, i64 16}
!1437 = !{!1432, !1266, i64 24}
!1438 = !{!1439, !1264, i64 12}
!1439 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfItEE", !1388, i64 8, !1264, i64 12, !1264, i64 16, !1440, i64 24, !1266, i64 32}
!1440 = !{!"pointer@_ZTSPPt", !1262, i64 0}
!1441 = !{!1439, !1266, i64 32}
!1442 = !{!1439, !1440, i64 24}
!1443 = !{!1265, !1265, i64 0}
!1444 = distinct !{!1444, !1289}
!1445 = distinct !{!1445, !1289}
!1446 = !{!"F", i1 false, i32 1, !979, !1430}
!1447 = distinct !{!1081, !1081}
!1448 = !{!1449, !1388, i64 0}
!1449 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !1388, i64 0, !1264, i64 4, !1264, i64 8, !1450, i64 16, !1266, i64 24}
!1450 = !{!"pointer@_ZTSPPN11xercesc_2_78IC_FieldE", !1262, i64 0}
!1451 = !{i8 0, i8 2}
!1452 = !{}
!1453 = !{!1449, !1264, i64 4}
!1454 = !{!1449, !1264, i64 8}
!1455 = !{!1449, !1450, i64 16}
!1456 = !{!1449, !1266, i64 24}
!1457 = !{!1458, !1458, i64 0}
!1458 = !{!"pointer@_ZTSPN11xercesc_2_78IC_FieldE", !1262, i64 0}
!1459 = distinct !{!1459, !1289}
!1460 = distinct !{!1430}
!1461 = !{!1262, !1262, i64 0}
!1462 = distinct !{!945, !1081}
!1463 = distinct !{!1464}
!1464 = !{%"class._ZTSN11xercesc_2_722FloatDatatypeValidatorE.xercesc_2_7::FloatDatatypeValidator" zeroinitializer, i32 1}
!1465 = distinct !{!1464}
!1466 = distinct !{!1464}
!1467 = distinct !{!1464, !1279}
!1468 = distinct !{!1281, !1464}
!1469 = distinct !{!1464, !740, !740, !360}
!1470 = distinct !{!814, !1464, !865, !854, !360}
!1471 = distinct !{!1464, !952, !952}
!1472 = distinct !{!1464, !740, !797, !360}
!1473 = distinct !{!1464, !740}
!1474 = distinct !{!1464, !740}
!1475 = distinct !{!1464, !740}
!1476 = distinct !{!1464, !740}
!1477 = distinct !{!1464, !360}
!1478 = distinct !{!1479, !1, !360}
!1479 = !{%"class._ZTSN11xercesc_2_77HashPtrE.xercesc_2_7::HashPtr" zeroinitializer, i32 1}
!1480 = distinct !{!1479, !1, !1}
!1481 = distinct !{!1482, !740, !814}
!1482 = !{%"class._ZTSN11xercesc_2_712FieldMatcherE.xercesc_2_7::FieldMatcher" zeroinitializer, i32 1}
!1483 = !{!1484, !1490, i64 72}
!1484 = !{!"struct@_ZTSN11xercesc_2_712FieldMatcherE", !1485, i64 0, !1490, i64 72, !1458, i64 80, !1491, i64 88}
!1485 = !{!"struct@_ZTSN11xercesc_2_712XPathMatcherE", !1264, i64 8, !1486, i64 16, !1486, i64 24, !1486, i64 32, !1487, i64 40, !1488, i64 48, !1489, i64 56, !1266, i64 64}
!1486 = !{!"pointer@_ZTSPi", !1262, i64 0}
!1487 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_12ValueStackOfIiEEEE", !1262, i64 0}
!1488 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE", !1262, i64 0}
!1489 = !{!"pointer@_ZTSPN11xercesc_2_718IdentityConstraintE", !1262, i64 0}
!1490 = !{!"pointer@_ZTSPN11xercesc_2_710ValueStoreE", !1262, i64 0}
!1491 = !{!"pointer@_ZTSPN11xercesc_2_714FieldActivatorE", !1262, i64 0}
!1492 = !{!1484, !1458, i64 80}
!1493 = !{!1494, !1489, i64 16}
!1494 = !{!"struct@_ZTSN11xercesc_2_78IC_FieldE", !1384, i64 0, !1495, i64 8, !1489, i64 16}
!1495 = !{!"pointer@_ZTSPN11xercesc_2_711XercesXPathE", !1262, i64 0}
!1496 = !{!1497, !1388, i64 0}
!1497 = !{!"struct@_ZTSN11xercesc_2_710ValueStoreE", !1388, i64 0, !1264, i64 4, !1489, i64 8, !1432, i64 16, !1498, i64 48, !1490, i64 56, !1499, i64 64, !1266, i64 72}
!1498 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE", !1262, i64 0}
!1499 = !{!"pointer@_ZTSPN11xercesc_2_710XMLScannerE", !1262, i64 0}
!1500 = !{!1501, !1521, i64 264}
!1501 = !{!"struct@_ZTSN11xercesc_2_710XMLScannerE", !1502, i64 0, !1503, i64 8, !1388, i64 16, !1388, i64 17, !1388, i64 18, !1388, i64 19, !1388, i64 20, !1388, i64 21, !1388, i64 22, !1388, i64 23, !1388, i64 24, !1388, i64 25, !1388, i64 26, !1388, i64 27, !1388, i64 28, !1388, i64 29, !1388, i64 30, !1388, i64 31, !1388, i64 32, !1388, i64 33, !1388, i64 34, !1388, i64 35, !1388, i64 36, !1388, i64 37, !1388, i64 38, !1264, i64 40, !1264, i64 44, !1264, i64 48, !1264, i64 52, !1264, i64 56, !1264, i64 60, !1264, i64 64, !1264, i64 68, !1504, i64 72, !1264, i64 80, !1264, i64 84, !1264, i64 88, !1264, i64 92, !1264, i64 96, !1505, i64 104, !1506, i64 112, !1507, i64 120, !1508, i64 128, !1509, i64 136, !1510, i64 144, !1511, i64 152, !1512, i64 160, !1513, i64 168, !1388, i64 176, !1514, i64 184, !1521, i64 264, !1522, i64 272, !1523, i64 280, !1266, i64 288, !1524, i64 296, !1524, i64 304, !1525, i64 312, !1265, i64 320, !1265, i64 328, !1265, i64 336, !1526, i64 344, !1520, i64 352, !1266, i64 360, !1527, i64 368, !1529, i64 392, !1529, i64 432, !1529, i64 472, !1529, i64 512, !1529, i64 552, !1529, i64 592, !1529, i64 632, !1531, i64 672}
!1502 = !{!"struct@_ZTSN11xercesc_2_720XMLBufferFullHandlerE"}
!1503 = !{!"long", !1262, i64 0}
!1504 = !{!"pointer@_ZTSPPj", !1262, i64 0}
!1505 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE", !1262, i64 0}
!1506 = !{!"pointer@_ZTSPN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE", !1262, i64 0}
!1507 = !{!"pointer@_ZTSPN11xercesc_2_718XMLDocumentHandlerE", !1262, i64 0}
!1508 = !{!"pointer@_ZTSPN11xercesc_2_714DocTypeHandlerE", !1262, i64 0}
!1509 = !{!"pointer@_ZTSPN11xercesc_2_716XMLEntityHandlerE", !1262, i64 0}
!1510 = !{!"pointer@_ZTSPN11xercesc_2_716XMLErrorReporterE", !1262, i64 0}
!1511 = !{!"pointer@_ZTSPN11xercesc_2_712ErrorHandlerE", !1262, i64 0}
!1512 = !{!"pointer@_ZTSPN11xercesc_2_711PSVIHandlerE", !1262, i64 0}
!1513 = !{!"pointer@_ZTSPN11xercesc_2_717ValidationContextE", !1262, i64 0}
!1514 = !{!"struct@_ZTSN11xercesc_2_79ReaderMgrE", !1515, i64 0, !1516, i64 8, !1517, i64 16, !1509, i64 24, !1518, i64 32, !1264, i64 40, !1519, i64 48, !1388, i64 56, !1520, i64 60, !1388, i64 64, !1266, i64 72}
!1515 = !{!"struct@_ZTSN11xercesc_2_77LocatorE"}
!1516 = !{!"pointer@_ZTSPN11xercesc_2_713XMLEntityDeclE", !1262, i64 0}
!1517 = !{!"pointer@_ZTSPN11xercesc_2_79XMLReaderE", !1262, i64 0}
!1518 = !{!"pointer@_ZTSPN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE", !1262, i64 0}
!1519 = !{!"pointer@_ZTSPN11xercesc_2_710RefStackOfINS_9XMLReaderEEE", !1262, i64 0}
!1520 = !{!"_ZTSN11xercesc_2_79XMLReader10XMLVersionE", !1262, i64 0}
!1521 = !{!"pointer@_ZTSPN11xercesc_2_712XMLValidatorE", !1262, i64 0}
!1522 = !{!"_ZTSN11xercesc_2_710XMLScanner10ValSchemesE", !1262, i64 0}
!1523 = !{!"pointer@_ZTSPN11xercesc_2_715GrammarResolverE", !1262, i64 0}
!1524 = !{!"pointer@_ZTSPN11xercesc_2_77GrammarE", !1262, i64 0}
!1525 = !{!"pointer@_ZTSPN11xercesc_2_713XMLStringPoolE", !1262, i64 0}
!1526 = !{!"pointer@_ZTSPN11xercesc_2_715SecurityManagerE", !1262, i64 0}
!1527 = !{!"struct@_ZTSN11xercesc_2_712XMLBufferMgrE", !1264, i64 0, !1266, i64 8, !1528, i64 16}
!1528 = !{!"pointer@_ZTSPPN11xercesc_2_79XMLBufferE", !1262, i64 0}
!1529 = !{!"struct@_ZTSN11xercesc_2_79XMLBufferE", !1264, i64 0, !1264, i64 4, !1264, i64 8, !1388, i64 12, !1266, i64 16, !1530, i64 24, !1265, i64 32}
!1530 = !{!"pointer@_ZTSPN11xercesc_2_720XMLBufferFullHandlerE", !1262, i64 0}
!1531 = !{!"struct@_ZTSN11xercesc_2_79ElemStackE", !1264, i64 0, !1264, i64 4, !1532, i64 8, !1535, i64 48, !1264, i64 56, !1264, i64 60, !1264, i64 64, !1264, i64 68, !1264, i64 72, !1264, i64 76, !1264, i64 80, !1536, i64 88, !1266, i64 96}
!1532 = !{!"struct@_ZTSN11xercesc_2_713XMLStringPoolE", !1384, i64 0, !1266, i64 8, !1533, i64 16, !1534, i64 24, !1264, i64 32, !1264, i64 36}
!1533 = !{!"pointer@_ZTSPPN11xercesc_2_713XMLStringPool8PoolElemE", !1262, i64 0}
!1534 = !{!"pointer@_ZTSPN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE", !1262, i64 0}
!1535 = !{!"pointer@_ZTSPPN11xercesc_2_79ElemStack9StackElemE", !1262, i64 0}
!1536 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE", !1262, i64 0}
!1537 = !{!1538, !1265, i64 16}
!1538 = !{!"struct@_ZTSN11xercesc_2_718IdentityConstraintE", !1384, i64 0, !1265, i64 8, !1265, i64 16, !1539, i64 24, !1540, i64 32, !1266, i64 40, !1264, i64 48}
!1539 = !{!"pointer@_ZTSPN11xercesc_2_711IC_SelectorE", !1262, i64 0}
!1540 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE", !1262, i64 0}
!1541 = !{!1484, !1491, i64 88}
!1542 = !{!1543, !1546, i64 16}
!1543 = !{!"struct@_ZTSN11xercesc_2_714FieldActivatorE", !1544, i64 0, !1545, i64 8, !1546, i64 16, !1266, i64 24}
!1544 = !{!"pointer@_ZTSPN11xercesc_2_715ValueStoreCacheE", !1262, i64 0}
!1545 = !{!"pointer@_ZTSPN11xercesc_2_717XPathMatcherStackE", !1262, i64 0}
!1546 = !{!"pointer@_ZTSPN11xercesc_2_716ValueHashTableOfIbEE", !1262, i64 0}
!1547 = !{!1548, !1550, i64 24}
!1548 = !{!"struct@_ZTSN11xercesc_2_716ValueHashTableOfIbEE", !1266, i64 0, !1549, i64 8, !1264, i64 16, !1550, i64 24}
!1549 = !{!"pointer@_ZTSPPN11xercesc_2_724ValueHashTableBucketElemIbEE", !1262, i64 0}
!1550 = !{!"pointer@_ZTSPN11xercesc_2_78HashBaseE", !1262, i64 0}
!1551 = !{!1548, !1264, i64 16}
!1552 = !{!1553, !1264, i64 0}
!1553 = !{!"struct@_ZTSN11xercesc_2_710CMStateSetE", !1264, i64 0, !1264, i64 4, !1264, i64 8, !1264, i64 12, !1554, i64 16, !1266, i64 24}
!1554 = !{!"pointer@_ZTSPh", !1262, i64 0}
!1555 = !{!1553, !1264, i64 8}
!1556 = !{!1553, !1264, i64 12}
!1557 = !{!1553, !1264, i64 4}
!1558 = !{!1553, !1554, i64 16}
!1559 = distinct !{!1559, !1289}
!1560 = distinct !{!1560, !1289}
!1561 = !{!1548, !1549, i64 8}
!1562 = !{!1563, !1563, i64 0}
!1563 = !{!"pointer@_ZTSPN11xercesc_2_724ValueHashTableBucketElemIbEE", !1262, i64 0}
!1564 = !{!1565, !1566, i64 16}
!1565 = !{!"struct@_ZTSN11xercesc_2_724ValueHashTableBucketElemIbEE", !1388, i64 0, !1563, i64 8, !1566, i64 16}
!1566 = !{!"pointer@_ZTSPv", !1262, i64 0}
!1567 = distinct !{!1567, !1289}
!1568 = distinct !{!1568, !1289}
!1569 = !{!1565, !1563, i64 8}
!1570 = distinct !{!1570, !1289}
!1571 = !{!1565, !1388, i64 0}
!1572 = !{!1497, !1499, i64 64}
!1573 = !{!1497, !1432, i64 16}
!1574 = distinct !{!1574, !1289}
!1575 = !{!1432, !1434, i64 8}
!1576 = !{!1577, !1264, i64 4}
!1577 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !1388, i64 0, !1264, i64 4, !1264, i64 8, !1578, i64 16, !1266, i64 24}
!1578 = !{!"pointer@_ZTSPPN11xercesc_2_717DatatypeValidatorE", !1262, i64 0}
!1579 = !{!1577, !1266, i64 24}
!1580 = !{!1577, !1578, i64 16}
!1581 = !{!1582, !1582, i64 0}
!1582 = !{!"pointer@_ZTSPN11xercesc_2_717DatatypeValidatorE", !1262, i64 0}
!1583 = !{!1497, !1264, i64 4}
!1584 = !{!1497, !1489, i64 8}
!1585 = !{!1497, !1498, i64 48}
!1586 = !{!1497, !1266, i64 72}
!1587 = !{!1588, !1388, i64 8}
!1588 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE", !1388, i64 8, !1264, i64 12, !1264, i64 16, !1589, i64 24, !1266, i64 32}
!1589 = !{!"pointer@_ZTSPPN11xercesc_2_713FieldValueMapE", !1262, i64 0}
!1590 = !{!1588, !1264, i64 12}
!1591 = !{!1588, !1264, i64 16}
!1592 = !{!1588, !1589, i64 24}
!1593 = !{!1588, !1266, i64 32}
!1594 = !{!1595, !1595, i64 0}
!1595 = !{!"pointer@_ZTSPN11xercesc_2_713FieldValueMapE", !1262, i64 0}
!1596 = distinct !{!1596, !1289}
!1597 = !{!1548, !1266, i64 0}
!1598 = distinct !{!1598, !1289}
!1599 = distinct !{!1600}
!1600 = !{%"class._ZTSN11xercesc_2_76IC_KeyE.xercesc_2_7::IC_Key" zeroinitializer, i32 1}
!1601 = distinct !{!1602}
!1602 = !{%"class._ZTSN11xercesc_2_79IC_KeyRefE.xercesc_2_7::IC_KeyRef" zeroinitializer, i32 1}
!1603 = distinct !{!1}
!1604 = distinct !{!1605, !740}
!1605 = !{%"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 1}
!1606 = !{!1607, !1265, i64 8}
!1607 = !{!"struct@_ZTSN11xercesc_2_714InMemMsgLoaderE", !1608, i64 0, !1265, i64 8}
!1608 = !{!"struct@_ZTSN11xercesc_2_712XMLMsgLoaderE"}
!1609 = distinct !{!1609, !1289}
!1610 = distinct !{!1610, !1289}
!1611 = distinct !{!1612}
!1612 = !{%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1}
!1613 = distinct !{!1, !1614}
!1614 = !{%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1}
!1615 = distinct !{!1614, !1}
!1616 = distinct !{!1}
!1617 = distinct !{!340, !740}
!1618 = !{!1619, !1619, i64 0}
!1619 = !{!"pointer@_ZTSPN11xercesc_2_712PanicHandlerE", !1262, i64 0}
!1620 = !{!"F", i1 false, i32 2, !979, !336, !339}
!1621 = distinct !{!1621, !1289}
!1622 = distinct !{!818, !823, !1133}
!1623 = !{!1514, !1517, i64 16}
!1624 = !{!1514, !1516, i64 8}
!1625 = !{!1626, !1265, i64 40}
!1626 = !{!"struct@_ZTSN11xercesc_2_713XMLEntityDeclE", !1384, i64 0, !1264, i64 8, !1264, i64 12, !1265, i64 16, !1265, i64 24, !1265, i64 32, !1265, i64 40, !1265, i64 48, !1265, i64 56, !1266, i64 64}
!1627 = !{!1514, !1519, i64 48}
!1628 = !{!1629, !1630, i64 8}
!1629 = !{!"struct@_ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE", !1630, i64 8}
!1630 = !{!"struct@_ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE", !1631, i64 0}
!1631 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE", !1388, i64 8, !1264, i64 12, !1264, i64 16, !1632, i64 24, !1266, i64 32}
!1632 = !{!"pointer@_ZTSPPN11xercesc_2_79XMLReaderE", !1262, i64 0}
!1633 = !{!1631, !1264, i64 12}
!1634 = !{!1514, !1518, i64 32}
!1635 = distinct !{!1635, !1289}
!1636 = !{!1516, !1516, i64 0}
!1637 = distinct !{!817, !819}
!1638 = !{!1639, !1640, i64 8}
!1639 = !{!"struct@_ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE", !1640, i64 8}
!1640 = !{!"struct@_ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE", !1641, i64 0}
!1641 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE", !1388, i64 8, !1264, i64 12, !1264, i64 16, !1642, i64 24, !1266, i64 32}
!1642 = !{!"pointer@_ZTSPPN11xercesc_2_713XMLEntityDeclE", !1262, i64 0}
!1643 = !{!1641, !1264, i64 12}
!1644 = !{!1641, !1266, i64 32}
!1645 = !{!1641, !1642, i64 24}
!1646 = distinct !{!818, !820}
!1647 = !{!1631, !1266, i64 32}
!1648 = !{!1631, !1632, i64 24}
!1649 = !{!1517, !1517, i64 0}
!1650 = distinct !{!814, !1430}
!1651 = distinct !{!740, !1430}
!1652 = distinct !{!1430, !1105, !814, !740}
!1653 = !{!"F", i1 false, i32 3, !979, !1081, !740, !339}
!1654 = distinct !{!1068, !1430}
!1655 = distinct !{!1655, !1289}
!1656 = distinct !{!1656, !1289}
!1657 = !{!"F", i1 false, i32 4, !339, !814, !740, !740, !360}
!1658 = distinct !{!1658, !1289}
!1659 = distinct !{!1}
!1660 = distinct !{!1430, !1661}
!1661 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 1}
!1662 = distinct !{!1081, !360}
!1663 = distinct !{!1081}
!1664 = distinct !{!1664, !1289}
!1665 = distinct !{!1666, !1, !360}
!1666 = !{%"class._ZTSN11xercesc_2_721NumberFormatExceptionE.xercesc_2_7::NumberFormatException" zeroinitializer, i32 1}
!1667 = distinct !{!1668, !740, !360}
!1668 = !{%"class._ZTSN11xercesc_2_713XMLBigDecimalE.xercesc_2_7::XMLBigDecimal" zeroinitializer, i32 1}
!1669 = !{!1392, !1264, i64 20}
!1670 = distinct !{!1670, !1289}
!1671 = distinct !{!740, !740, !759, !759, !759, !360}
!1672 = !{!1264, !1264, i64 0}
!1673 = !{!1674, !1262, i64 0}
!1674 = !{!"array@_ZTSA65536_h", !1262, i64 0}
!1675 = distinct !{!1675, !1289}
!1676 = distinct !{!1676, !1289}
!1677 = distinct !{!1677, !1289}
!1678 = distinct !{!1678, !1289}
!1679 = distinct !{!1679, !1289}
!1680 = distinct !{!1668}
!1681 = distinct !{!1361, !1361}
!1682 = distinct !{!1682, !1289}
!1683 = !{!1382, !1265, i64 80}
!1684 = !{!1382, !1388, i64 72}
!1685 = !{!1382, !1387, i64 64}
!1686 = !{!1382, !1266, i64 88}
!1687 = distinct !{!1361, !1361}
!1688 = !{!1382, !1264, i64 40}
!1689 = !{!1382, !1264, i64 52}
!1690 = !{!1382, !1264, i64 48}
!1691 = distinct !{!1691, !1289}
!1692 = !{!1693, !1264, i64 0}
!1693 = !{!"array@_ZTSA4_A8_i", !1385, i64 0}
!1694 = distinct !{!1694, !1289}
!1695 = distinct !{!1361}
!1696 = distinct !{!1361, !1361}
!1697 = !{!1382, !1264, i64 56}
!1698 = distinct !{!1698, !1289}
!1699 = distinct !{!1361}
!1700 = distinct !{!1700, !1289}
!1701 = distinct !{!1361, !1361}
!1702 = distinct !{!1703}
!1703 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 1}
!1704 = distinct !{!1703}
!1705 = !{!"A", i32 2048, !6}
!1706 = !{!"F", i1 false, i32 4, !1369, !340, !339, !740, !339}
!1707 = distinct !{!1707, !1289}
!1708 = distinct !{!340}
!1709 = !{!1710, !1710, i64 0}
!1710 = !{!"pointer@_ZTSPN11xercesc_2_712XMLMsgLoaderE", !1262, i64 0}
!1711 = !{!1712, !1566, i64 0}
!1712 = !{!"struct@_ZTSN11xercesc_2_78XMLMutexE", !1566, i64 0}
!1713 = !{!1714, !1714, i64 0}
!1714 = !{!"pointer@_ZTSPN11xercesc_2_78XMLMutexE", !1262, i64 0}
!1715 = !{!1716, !1717, i64 0}
!1716 = !{!"struct@_ZTSN11xercesc_2_718XMLRegisterCleanupE", !1717, i64 0, !1718, i64 8, !1718, i64 16}
!1717 = !{!"pointer@_ZTSPFvvE", !1262, i64 0}
!1718 = !{!"pointer@_ZTSPN11xercesc_2_718XMLRegisterCleanupE", !1262, i64 0}
!1719 = !{!1716, !1718, i64 8}
!1720 = !{!1718, !1718, i64 0}
!1721 = !{!1716, !1718, i64 16}
!1722 = distinct !{!740, !740, !740, !740, !740, !360}
!1723 = distinct !{!1723, !1289}
!1724 = distinct !{!1724, !1289}
!1725 = distinct !{!1725, !1289}
!1726 = distinct !{!799}
!1727 = !{!"A", i32 1024, !6}
!1728 = !{!1729, !1499, i64 32}
!1729 = !{!"struct@_ZTSN11xercesc_2_712XMLValidatorE", !1730, i64 8, !1510, i64 16, !1731, i64 24, !1499, i64 32}
!1730 = !{!"pointer@_ZTSPN11xercesc_2_712XMLBufferMgrE", !1262, i64 0}
!1731 = !{!"pointer@_ZTSPN11xercesc_2_79ReaderMgrE", !1262, i64 0}
!1732 = !{!1501, !1264, i64 40}
!1733 = !{!1729, !1510, i64 16}
!1734 = !{!1729, !1731, i64 24}
!1735 = !{!1736, !1265, i64 163928}
!1736 = !{!"struct@_ZTSN11xercesc_2_79XMLReaderE", !1264, i64 0, !1737, i64 4, !1264, i64 32772, !1738, i64 32776, !1739, i64 49160, !1503, i64 114696, !1503, i64 114704, !1740, i64 114712, !1265, i64 114720, !1388, i64 114728, !1388, i64 114729, !1265, i64 114736, !1264, i64 114744, !1741, i64 114748, !1264, i64 163900, !1264, i64 163904, !1742, i64 163908, !1388, i64 163912, !1743, i64 163916, !1264, i64 163920, !1388, i64 163924, !1388, i64 163925, !1265, i64 163928, !1744, i64 163936, !1388, i64 163944, !1388, i64 163945, !1745, i64 163952, !1746, i64 163960, !1554, i64 163968, !1388, i64 163976, !1520, i64 163980, !1266, i64 163984}
!1737 = !{!"array@_ZTSA16384_t", !1287, i64 0}
!1738 = !{!"array@_ZTSA16384_h", !1262, i64 0}
!1739 = !{!"array@_ZTSA16384_j", !1264, i64 0}
!1740 = !{!"_ZTSN11xercesc_2_713XMLRecognizer9EncodingsE", !1262, i64 0}
!1741 = !{!"array@_ZTSA49152_h", !1262, i64 0}
!1742 = !{!"_ZTSN11xercesc_2_79XMLReader7RefFromE", !1262, i64 0}
!1743 = !{!"_ZTSN11xercesc_2_79XMLReader7SourcesE", !1262, i64 0}
!1744 = !{!"pointer@_ZTSPN11xercesc_2_714BinInputStreamE", !1262, i64 0}
!1745 = !{!"pointer@_ZTSPN11xercesc_2_713XMLTranscoderE", !1262, i64 0}
!1746 = !{!"_ZTSN11xercesc_2_79XMLReader5TypesE", !1262, i64 0}
!1747 = !{!1736, !1265, i64 114736}
!1748 = !{!1736, !1503, i64 114704}
!1749 = !{!1736, !1503, i64 114696}
!1750 = !{!"F", i1 false, i32 9, !979, !794, !339, !740, !339, !740, !740, !740, !746, !746}
!1751 = !{!1501, !1388, i64 20}
!1752 = !{!1501, !1388, i64 19}
!1753 = !{!1501, !1388, i64 21}
!1754 = !{!1755, !1755, i64 0}
!1755 = !{!"_ZTSN11xercesc_2_78XMLValid5CodesE", !1262, i64 0}
!1756 = distinct !{!340}
!1757 = distinct !{!799, !740, !740, !740}
!1758 = !{!1501, !1266, i64 360}
!1759 = !{!"F", i1 false, i32 4, !1369, !1605, !339, !740, !339}
!1760 = distinct !{!1}
!1761 = distinct !{!1, !360}
!1762 = distinct !{!1}
!1763 = distinct !{!1764}
!1764 = !{%"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1}
!1765 = distinct !{!1, !1766}
!1766 = !{%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1}
!1767 = distinct !{!1766, !1}
!1768 = distinct !{!1081, !740}

; end INTEL_FEATURE_SW_DTRANS
