; This test verifies that "ippredopt-callsite" attribute is set
; for some of the calls during SOAToAOSOP transformation.

; RUN: opt < %s -S -opaque-pointers -whole-program-assume                 \
; RUN:          -intel-libirc-allowed                                     \
; RUN:          -passes=dtrans-soatoaosop -enable-intel-advanced-opts     \
; RUN:          -mtriple=i686-- -mattr=+avx2  -dtrans-outofboundsok=false \
; RUN:          -dtrans-usecrulecompat=true                               \
; RUN:          2>&1 | FileCheck %s

; CHECK: tail call noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i14, i32 noundef %i13) #[[ATTR:[0-9]+]]
; CHECK: call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i15, i32 noundef %i22) #[[ATTR]]
; CHECK:  call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i15, i32 noundef %i22) #[[ATTR]]
; CHECK: call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i22) #[[ATTR]]
; CHECK: call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i22) #[[ATTR]]
; CHECK: attributes #[[ATTR]] = { "ippredopt-callsite" }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" = type { ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" = type { ptr }
%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" = type { %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" }
%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" = type { ptr, i32, ptr, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" = type { ptr }
%"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" = type { ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" = type { %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr }
%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" = type { ptr }
%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" = type { ptr }
%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i8, i8, i8, i8, i16, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" = type { i8, i32, i32, ptr, ptr }
%"_DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" = type { i8, i32, ptr, %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_712XMLMutexLockE.xercesc_2_7::XMLMutexLock" = type { ptr }
%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" = type { i8 }
%struct._ZTS8_IO_FILE._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque
%"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" = type { ptr, i16, ptr }
%"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" = type { ptr }
%"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" = type { %"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token", i8, i8, i32, i32, i32, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" = type { ptr }
%"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr }
%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" = type <{ ptr, i32, [4 x i8], ptr, ptr, i32 }>
%"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" = type <{ ptr, ptr, ptr, [14 x ptr], [14 x ptr], ptr, ptr, ptr, ptr, ptr, ptr, i8, i8, [6 x i8] }>
%"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" = type { ptr }
%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, i32, i32 }
%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" = type { i32, ptr }
%"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" = type { ptr, i8, ptr, i32, i32, i32, ptr }
%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, i32, i32, i8, [7 x i8] }>
%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, i32, i32, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, i32, i32, i8 }>
%"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, ptr, ptr, i32, i8, i8, i32, i32 }
%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, i32, i32, i8, i8, i32, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" = type { ptr }
%"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" = type { ptr }
%"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" = type { ptr }
%"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" = type { ptr }
%"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" = type { %"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler", i64, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i8, %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr", ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, ptr, %"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer", %"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" }
%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" = type { ptr }
%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" = type { %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator", ptr, ptr, ptr, ptr, i32, ptr, i8, i32, i8, ptr }
%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" = type { i32, ptr, ptr }
%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" = type { i32, i32, i32, i8, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" = type { i32, i32, %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool", ptr, i32, i32, i32, i32, i32, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" = type { ptr, i8, ptr, i32, i32, ptr }
%"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" = type { ptr }
%"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" = type { ptr, ptr }
%"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" = type { ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" = type { i8, i8, i8, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" }
%"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" = type <{ ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" = type <{ i8, [3 x i8], i32, i32, [4 x i8], ptr, ptr, ptr, ptr, i8, [7 x i8] }>
%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" = type { i32, [16384 x i16], i32, [16384 x i8], [16384 x i32], i64, i64, i32, ptr, i8, i8, ptr, i32, [49152 x i8], i32, i32, i32, i8, i32, i32, i8, i8, ptr, ptr, i8, i8, ptr, i32, ptr, i8, i32, ptr }
%"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" = type { [8 x i8], %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
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
%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" = type { ptr }
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
%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" = type <{ %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr, ptr, ptr, ptr, i32, [4 x i8] }>
%"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", ptr, ptr }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" }
%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" = type { %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable", i32, ptr, ptr, ptr }
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
%"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" = type { %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" }
%"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" = type { %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" }
%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" = type { ptr, ptr, ptr }
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

@.str.3 = external hidden unnamed_addr constant [33 x i8], align 1
@.str.817 = external hidden unnamed_addr constant [4 x i8], align 1
@.str.1.1140 = external hidden unnamed_addr constant [31 x i8], align 1
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
@_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE = external hidden global ptr, align 8, !intel_dtrans_type !334
@.str.1.2472 = external hidden unnamed_addr constant [31 x i8], align 1
@.str.2.2471 = external hidden unnamed_addr constant [33 x i8], align 1
@_ZN11xercesc_2_7L16msgLoaderCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L15msgMutexCleanupE = external hidden global %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", align 8
@_ZN11xercesc_2_7L10sMsgLoaderE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !335
@_ZN11xercesc_2_7L9sMsgMutexE = external hidden unnamed_addr global ptr, align 8, !intel_dtrans_type !336
@_ZN11xercesc_2_7L23sScannerMutexRegisteredE = external hidden unnamed_addr global i1, align 1
@_ZN11xercesc_2_715gXMLCleanupListE = external hidden global ptr, align 8, !intel_dtrans_type !337
@_ZN11xercesc_2_76XMLUni14fgExceptDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !221
@_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !167
@_ZN11xercesc_2_76XMLUni11fgDefErrMsgE = external hidden constant [23 x i16], align 16, !intel_dtrans_type !110
@_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE = external hidden constant [41 x i16], align 16, !intel_dtrans_type !167
@_ZN11xercesc_2_76XMLUni16fgValidityDomainE = external hidden constant [43 x i16], align 16, !intel_dtrans_type !221
@_ZN11xercesc_2_716XMLPlatformUtils13fgAtomicMutexE = external hidden global ptr, align 8, !intel_dtrans_type !336
@_ZTISt9bad_alloc = external dso_local constant ptr, !intel_dtrans_type !338
@_ZTVSt9bad_alloc = external dso_local unnamed_addr constant { [5 x ptr] }, align 8, !type !339, !type !340, !type !341, !type !342, !intel_dtrans_type !343
@stderr = external dso_local local_unnamed_addr global ptr, align 8, !intel_dtrans_type !345
@_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE = external hidden global ptr, align 8, !intel_dtrans_type !346
@_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !347
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = external hidden constant { ptr, ptr, ptr }, align 8, !intel_dtrans_type !347
@_ZTVN11xercesc_2_712XMLExceptionE.0 = external hidden constant [5 x ptr], !type !348, !type !349, !intel_dtrans_type !344
@_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0 = external hidden constant [6 x ptr], !type !348, !type !349, !type !350, !type !351, !type !352, !type !353, !intel_dtrans_type !354

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_716XMLPlatformUtils11loadAMsgSetEPKt(ptr noundef "intel_dtrans_func_index"="2" %arg) #0 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !768 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !769
  %i1 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 16, ptr noundef %i)
          to label %bb2 unwind label %bb3

bb2:                                              ; preds = %bb
  invoke void @_ZN11xercesc_2_714InMemMsgLoaderC1EPKt(ptr noundef nonnull align 8 dereferenceable(16) %i1, ptr noundef %arg)
          to label %bb22 unwind label %bb5

bb3:                                              ; preds = %bb
  %i4 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  br label %bb7

bb5:                                              ; preds = %bb2
  %i6 = landingpad { ptr, i32 }
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
          catch ptr null
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i1, ptr noundef %i) #36
  br label %bb7

bb7:                                              ; preds = %bb5, %bb3
  %i8 = phi { ptr, i32 } [ %i6, %bb5 ], [ %i4, %bb3 ]
  %i9 = extractvalue { ptr, i32 } %i8, 1
  %i10 = extractvalue { ptr, i32 } %i8, 0
  %i11 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #36
  %i12 = icmp eq i32 %i9, %i11
  br i1 %i12, label %bb13, label %bb15

bb13:                                             ; preds = %bb7
  %i14 = tail call ptr @__cxa_begin_catch(ptr %i10) #36
  invoke void @__cxa_rethrow() #37
          to label %bb29 unwind label %bb20

bb15:                                             ; preds = %bb7
  %i16 = tail call ptr @__cxa_begin_catch(ptr %i10) #36
  invoke void @_ZN11xercesc_2_716XMLPlatformUtils5panicENS_12PanicHandler12PanicReasonsE(i32 noundef 4)
          to label %bb17 unwind label %bb18

bb17:                                             ; preds = %bb15
  tail call void @__cxa_end_catch()
  br label %bb22

bb18:                                             ; preds = %bb15
  %i19 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb24 unwind label %bb26

bb20:                                             ; preds = %bb13
  %i21 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb24 unwind label %bb26

bb22:                                             ; preds = %bb17, %bb2
  %i23 = phi ptr [ undef, %bb17 ], [ %i1, %bb2 ]
  ret ptr %i23

bb24:                                             ; preds = %bb20, %bb18
  %i25 = phi { ptr, i32 } [ %i19, %bb18 ], [ %i21, %bb20 ]
  resume { ptr, i32 } %i25

bb26:                                             ; preds = %bb20, %bb18
  %i27 = landingpad { ptr, i32 }
          catch ptr null
  %i28 = extractvalue { ptr, i32 } %i27, 0
  tail call void @__clang_call_terminate(ptr %i28) #38
  unreachable

bb29:                                             ; preds = %bb13
  unreachable
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind memory(none)
declare i32 @llvm.eh.typeid.for(ptr) #1

; Function Attrs: nofree
declare !intel.dtrans.func.type !773 dso_local "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2") local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #2

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_716XMLPlatformUtils5panicENS_12PanicHandler12PanicReasonsE(i32 noundef %arg) #0 align 2 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils21fgDefaultPanicHandlerE, align 8, !tbaa !774
  %i1 = getelementptr %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler", ptr %i, i64 0, i32 0
  %i2 = load ptr, ptr %i1, align 8, !tbaa !776
  %i3 = call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i5 = load ptr, ptr %i4, align 8
  tail call void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr noundef nonnull align 8 dereferenceable(8) %i, i32 noundef %arg), !intel_dtrans_type !778, !_Intel.Devirt.Call !779
  ret void
}

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #2

; Function Attrs: nofree noinline noreturn nounwind uwtable
define hidden void @__clang_call_terminate(ptr noundef %arg) #3 {
bb:
  %i = tail call ptr @__cxa_begin_catch(ptr %arg) #36
  tail call void @_ZSt9terminatev() #38
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #5

; Function Attrs: nofree
declare !intel.dtrans.func.type !780 dso_local noalias "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64) local_unnamed_addr #2

; Function Attrs: nofree noreturn
declare !intel.dtrans.func.type !781 dso_local void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3") local_unnamed_addr #6

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(ptr) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !782 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr #9

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #10

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, i32 noundef %arg3, ptr noundef "intel_dtrans_func_index"="3" %arg4) unnamed_addr #11 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !783 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0
  tail call void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i, ptr noundef %arg1, i32 noundef %arg2, ptr noundef %arg4)
  %i5 = getelementptr %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([6 x ptr], ptr @_ZTVN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.0, i32 0, i64 2), ptr %i5, align 8, !tbaa !776
  invoke void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef nonnull align 8 dereferenceable(48) %i, i32 noundef %arg3)
          to label %bb6 unwind label %bb7

bb6:                                              ; preds = %bb
  ret void

bb7:                                              ; preds = %bb
  %i8 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %i) #36
  resume { ptr, i32 } %i8
}

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !785 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr #12

; Function Attrs: inlinehint mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="2" %arg, ptr noundef "intel_dtrans_func_index"="3" %arg1) #13 align 2 !intel.dtrans.func.type !786 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb31, label %bb2

bb2:                                              ; preds = %bb
  %i3 = load i16, ptr %arg, align 2, !tbaa !787
  %i4 = icmp eq i16 %i3, 0
  br i1 %i4, label %bb16, label %bb5

bb5:                                              ; preds = %bb5, %bb2
  %i6 = phi ptr [ %i7, %bb5 ], [ %arg, %bb2 ]
  %i7 = getelementptr inbounds i16, ptr %i6, i64 1
  %i8 = load i16, ptr %i7, align 2, !tbaa !787
  %i9 = icmp eq i16 %i8, 0
  br i1 %i9, label %bb10, label %bb5, !llvm.loop !789

bb10:                                             ; preds = %bb5
  %i11 = ptrtoint ptr %i7 to i64
  %i12 = ptrtoint ptr %arg to i64
  %i13 = sub i64 %i11, %i12
  %i14 = add i64 %i13, 2
  %i15 = and i64 %i14, 8589934590
  br label %bb16

bb16:                                             ; preds = %bb10, %bb2
  %i17 = phi i64 [ %i15, %bb10 ], [ 2, %bb2 ]
  %i18 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg1, i64 0, i32 0
  %i19 = load ptr, ptr %i18, align 8, !tbaa !776
  %i20 = call i1 @llvm.type.test(ptr %i19, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i20)
  %i21 = getelementptr inbounds ptr, ptr %i19, i64 2
  %i22 = load ptr, ptr %i21, align 8
  %i23 = icmp eq ptr %i22, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i23, label %bb24, label %bb26

bb24:                                             ; preds = %bb16
  %i25 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i17), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb28

bb26:                                             ; preds = %bb16
  %i27 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i17), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb28

bb28:                                             ; preds = %bb26, %bb24
  %i29 = phi ptr [ %i25, %bb24 ], [ %i27, %bb26 ]
  br label %bb30

bb30:                                             ; preds = %bb28
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i29, ptr nonnull align 2 %arg, i64 %i17, i1 false)
  br label %bb31

bb31:                                             ; preds = %bb30, %bb
  %i32 = phi ptr [ %i29, %bb30 ], [ null, %bb ]
  ret ptr %i32
}

; Function Attrs: mustprogress nofree noreturn nounwind uwtable
define hidden void @_ZN11xercesc_2_719DefaultPanicHandler5panicENS_12PanicHandler12PanicReasonsE(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) unnamed_addr #14 align 2 !intel.dtrans.func.type !792 {
bb:
  %i = load ptr, ptr @stderr, align 8, !tbaa !794
  %i2 = tail call noundef ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg1)
  %i3 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %i, ptr noundef nonnull @.str.817, ptr noundef %i2) #39
  tail call void @exit(i32 noundef -1) #38
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !796 dso_local noundef i32 @fprintf(ptr nocapture noundef "intel_dtrans_func_index"="1", ptr nocapture noundef readonly "intel_dtrans_func_index"="2", ...) local_unnamed_addr #15

; Function Attrs: nofree noreturn nounwind
declare dso_local void @exit(i32 noundef) local_unnamed_addr #16

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable
define hidden void @_ZN11xercesc_2_713FieldValueMapC2EPNS_13MemoryManagerE(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) unnamed_addr #17 align 2 !intel.dtrans.func.type !797 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  store ptr null, ptr %i, align 8, !tbaa !799
  %i2 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  store ptr null, ptr %i2, align 8, !tbaa !804
  %i3 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  store ptr null, ptr %i3, align 8, !tbaa !805
  %i4 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 3, !intel-tbaa !806
  store ptr %arg1, ptr %i4, align 8, !tbaa !806
  ret void
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #18 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !807 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  store ptr null, ptr %i, align 8, !tbaa !799
  %i2 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  store ptr null, ptr %i2, align 8, !tbaa !804
  %i3 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  store ptr null, ptr %i3, align 8, !tbaa !805
  %i4 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 3, !intel-tbaa !806
  %i5 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 3, !intel-tbaa !806
  %i6 = load ptr, ptr %i5, align 8, !tbaa !806
  store ptr %i6, ptr %i4, align 8, !tbaa !806
  %i7 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 0, !intel-tbaa !799
  %i8 = load ptr, ptr %i7, align 8, !tbaa !799
  %i9 = icmp eq ptr %i8, null
  br i1 %i9, label %bb68, label %bb10

bb10:                                             ; preds = %bb
  %i11 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 2, !intel-tbaa !805
  %i12 = load ptr, ptr %i11, align 8, !tbaa !805
  %i13 = tail call noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv.5823(ptr noundef nonnull align 8 dereferenceable(40) %i12)
  %i14 = load ptr, ptr %i4, align 8, !tbaa !806
  %i15 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i14)
          to label %bb16 unwind label %bb33

bb16:                                             ; preds = %bb10
  %i17 = load ptr, ptr %i7, align 8, !tbaa !799
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(32) %i15, ptr noundef nonnull align 8 dereferenceable(32) %i17)
          to label %bb18 unwind label %bb35

bb18:                                             ; preds = %bb16
  store ptr %i15, ptr %i, align 8, !tbaa !799
  %i19 = load ptr, ptr %i4, align 8, !tbaa !806
  %i20 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i19)
          to label %bb21 unwind label %bb33

bb21:                                             ; preds = %bb18
  %i22 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 1, !intel-tbaa !804
  %i23 = load ptr, ptr %i22, align 8, !tbaa !804
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(32) %i20, ptr noundef nonnull align 8 dereferenceable(32) %i23)
          to label %bb24 unwind label %bb37

bb24:                                             ; preds = %bb21
  store ptr %i20, ptr %i2, align 8, !tbaa !804
  %i25 = load ptr, ptr %i4, align 8, !tbaa !806
  %i26 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i25)
          to label %bb27 unwind label %bb33

bb27:                                             ; preds = %bb24
  %i28 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg1, i64 0, i32 2, !intel-tbaa !805
  %i29 = load ptr, ptr %i28, align 8
  invoke void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE.5825.5833.5834(ptr noundef nonnull align 8 dereferenceable(40) %i26, ptr noundef nonnull align 8 dereferenceable(40) %i29)
          to label %bb30 unwind label %bb39

bb30:                                             ; preds = %bb27
  br label %bb31

bb31:                                             ; preds = %bb30
  store ptr %i26, ptr %i3, align 8, !tbaa !805
  %i32 = icmp eq i32 %i13, 0
  br i1 %i32, label %bb68, label %bb41

bb33:                                             ; preds = %bb24, %bb18, %bb10
  %i34 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb55

bb35:                                             ; preds = %bb16
  %i36 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i15, ptr noundef %i14) #36
  br label %bb55

bb37:                                             ; preds = %bb21
  %i38 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i20, ptr noundef %i19) #36
  br label %bb55

bb39:                                             ; preds = %bb27
  %i40 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i26, ptr noundef %i25) #36
  br label %bb55

bb41:                                             ; preds = %bb50, %bb31
  %i42 = phi i32 [ %i51, %bb50 ], [ 0, %bb31 ]
  %i43 = load ptr, ptr %i3, align 8, !tbaa !805
  %i44 = load ptr, ptr %i11, align 8, !tbaa !805
  %i45 = invoke noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj.5826(ptr noundef nonnull align 8 dereferenceable(40) %i44, i32 noundef %i42)
          to label %bb46 unwind label %bb53

bb46:                                             ; preds = %bb41
  %i47 = load ptr, ptr %i4, align 8, !tbaa !806
  %i48 = invoke noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef %i45, ptr noundef %i47)
          to label %bb49 unwind label %bb53

bb49:                                             ; preds = %bb46
  invoke void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831.5832(ptr noundef nonnull align 8 dereferenceable(40) %i43, ptr noundef %i48, i32 noundef %i42)
          to label %bb50 unwind label %bb53

bb50:                                             ; preds = %bb49
  %i51 = add nuw i32 %i42, 1
  %i52 = icmp eq i32 %i51, %i13
  br i1 %i52, label %bb68, label %bb41, !llvm.loop !808

bb53:                                             ; preds = %bb49, %bb46, %bb41
  %i54 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %bb55

bb55:                                             ; preds = %bb53, %bb39, %bb37, %bb35, %bb33
  %i56 = phi { ptr, i32 } [ %i54, %bb53 ], [ %i40, %bb39 ], [ %i34, %bb33 ], [ %i38, %bb37 ], [ %i36, %bb35 ]
  %i57 = extractvalue { ptr, i32 } %i56, 1
  %i58 = tail call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #36
  %i59 = icmp eq i32 %i57, %i58
  br i1 %i59, label %bb60, label %bb65

bb60:                                             ; preds = %bb55
  %i61 = extractvalue { ptr, i32 } %i56, 0
  %i62 = tail call ptr @__cxa_begin_catch(ptr %i61) #36
  invoke void @__cxa_rethrow() #37
          to label %bb72 unwind label %bb63

bb63:                                             ; preds = %bb60
  %i64 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb66 unwind label %bb69

bb65:                                             ; preds = %bb55
  tail call void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr noundef nonnull align 8 dereferenceable(32) %arg), !intel_dtrans_type !809
  br label %bb66

bb66:                                             ; preds = %bb65, %bb63
  %i67 = phi { ptr, i32 } [ %i56, %bb65 ], [ %i64, %bb63 ]
  resume { ptr, i32 } %i67

bb68:                                             ; preds = %bb50, %bb31, %bb
  ret void

bb69:                                             ; preds = %bb63
  %i70 = landingpad { ptr, i32 }
          catch ptr null
  %i71 = extractvalue { ptr, i32 } %i70, 0
  tail call void @__clang_call_terminate(ptr %i71) #38
  unreachable

bb72:                                             ; preds = %bb60
  unreachable
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #18 align 2 !intel.dtrans.func.type !810 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !811
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 0, !intel-tbaa !811
  %i3 = load i8, ptr %i2, align 8, !tbaa !811, !range !816, !noundef !817
  store i8 %i3, ptr %i, align 8, !tbaa !811
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 1, !intel-tbaa !818
  %i6 = load i32, ptr %i5, align 4, !tbaa !818
  store i32 %i6, ptr %i4, align 4, !tbaa !818
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !819
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 2, !intel-tbaa !819
  %i9 = load i32, ptr %i8, align 8, !tbaa !819
  store i32 %i9, ptr %i7, align 8, !tbaa !819
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !820
  store ptr null, ptr %i10, align 8, !tbaa !820
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !821
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 4, !intel-tbaa !821
  %i13 = load ptr, ptr %i12, align 8, !tbaa !821
  store ptr %i13, ptr %i11, align 8, !tbaa !821
  %i14 = zext i32 %i9 to i64
  %i15 = shl nuw nsw i64 %i14, 3
  %i16 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i13, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 8, !tbaa !776
  %i18 = call i1 @llvm.type.test(ptr %i17, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i18)
  %i19 = getelementptr inbounds ptr, ptr %i17, i64 2
  %i20 = load ptr, ptr %i19, align 8
  %i21 = icmp eq ptr %i20, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i21, label %bb22, label %bb24

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb24:                                             ; preds = %bb
  %i25 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb26:                                             ; preds = %bb24, %bb22
  %i27 = phi ptr [ %i23, %bb22 ], [ %i25, %bb24 ]
  br label %bb28

bb28:                                             ; preds = %bb26
  store ptr %i27, ptr %i10, align 8, !tbaa !820
  %i29 = load i32, ptr %i7, align 8, !tbaa !819
  %i30 = zext i32 %i29 to i64
  %i31 = shl nuw nsw i64 %i30, 3
  tail call void @llvm.memset.p0.i64(ptr align 8 %i27, i8 0, i64 %i31, i1 false)
  %i32 = load i32, ptr %i4, align 4, !tbaa !818
  %i33 = icmp eq i32 %i32, 0
  br i1 %i33, label %bb39, label %bb34

bb34:                                             ; preds = %bb28
  %i35 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 3, !intel-tbaa !820
  %i36 = load ptr, ptr %i35, align 8, !tbaa !820
  %i37 = load ptr, ptr %i10, align 8, !tbaa !820
  %i38 = zext i32 %i32 to i64
  br label %bb40

bb39:                                             ; preds = %bb40, %bb28
  ret void

bb40:                                             ; preds = %bb40, %bb34
  %i41 = phi i64 [ 0, %bb34 ], [ %i45, %bb40 ]
  %i42 = getelementptr inbounds ptr, ptr %i36, i64 %i41
  %i43 = load ptr, ptr %i42, align 8, !tbaa !822
  %i44 = getelementptr inbounds ptr, ptr %i37, i64 %i41
  store ptr %i43, ptr %i44, align 8, !tbaa !822
  %i45 = add nuw nsw i64 %i41, 1
  %i46 = icmp eq i64 %i45, %i38
  br i1 %i46, label %bb39, label %bb40, !llvm.loop !824
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg1) unnamed_addr #18 align 2 !intel.dtrans.func.type !825 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !826
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 0, !intel-tbaa !826
  %i3 = load i8, ptr %i2, align 8, !tbaa !826, !range !816, !noundef !817
  store i8 %i3, ptr %i, align 8, !tbaa !826
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 1, !intel-tbaa !829
  %i6 = load i32, ptr %i5, align 4, !tbaa !829
  store i32 %i6, ptr %i4, align 4, !tbaa !829
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !830
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 2, !intel-tbaa !830
  %i9 = load i32, ptr %i8, align 8, !tbaa !830
  store i32 %i9, ptr %i7, align 8, !tbaa !830
  %i10 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  store ptr null, ptr %i10, align 8, !tbaa !831
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 4, !intel-tbaa !832
  %i13 = load ptr, ptr %i12, align 8, !tbaa !832
  store ptr %i13, ptr %i11, align 8, !tbaa !832
  %i14 = zext i32 %i9 to i64
  %i15 = shl nuw nsw i64 %i14, 3
  %i16 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i13, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 8, !tbaa !776
  %i18 = call i1 @llvm.type.test(ptr %i17, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i18)
  %i19 = getelementptr inbounds ptr, ptr %i17, i64 2
  %i20 = load ptr, ptr %i19, align 8
  %i21 = icmp eq ptr %i20, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i21, label %bb22, label %bb24

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb24:                                             ; preds = %bb
  %i25 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb26:                                             ; preds = %bb24, %bb22
  %i27 = phi ptr [ %i23, %bb22 ], [ %i25, %bb24 ]
  br label %bb28

bb28:                                             ; preds = %bb26
  store ptr %i27, ptr %i10, align 8, !tbaa !831
  %i29 = load i32, ptr %i7, align 8, !tbaa !830
  %i30 = zext i32 %i29 to i64
  %i31 = shl nuw nsw i64 %i30, 3
  tail call void @llvm.memset.p0.i64(ptr align 8 %i27, i8 0, i64 %i31, i1 false)
  %i32 = load i32, ptr %i4, align 4, !tbaa !829
  %i33 = icmp eq i32 %i32, 0
  br i1 %i33, label %bb39, label %bb34

bb34:                                             ; preds = %bb28
  %i35 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg1, i64 0, i32 3, !intel-tbaa !831
  %i36 = load ptr, ptr %i35, align 8, !tbaa !831
  %i37 = load ptr, ptr %i10, align 8, !tbaa !831
  %i38 = zext i32 %i32 to i64
  br label %bb40

bb39:                                             ; preds = %bb40, %bb28
  ret void

bb40:                                             ; preds = %bb40, %bb34
  %i41 = phi i64 [ 0, %bb34 ], [ %i45, %bb40 ]
  %i42 = getelementptr inbounds ptr, ptr %i36, i64 %i41
  %i43 = load ptr, ptr %i42, align 8, !tbaa !833
  %i44 = getelementptr inbounds ptr, ptr %i37, i64 %i41
  store ptr %i43, ptr %i44, align 8, !tbaa !833
  %i45 = add nuw nsw i64 %i41, 1
  %i46 = icmp eq i64 %i45, %i38
  br i1 %i46, label %bb39, label %bb40, !llvm.loop !835
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) #19 align 2 !intel.dtrans.func.type !836 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  %i1 = load ptr, ptr %i, align 8, !tbaa !799
  %i2 = icmp eq ptr %i1, null
  br i1 %i2, label %bb4, label %bb3

bb3:                                              ; preds = %bb
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(ptr noundef nonnull align 8 dereferenceable(32) %i1) #36
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef nonnull %i1) #36
  br label %bb4

bb4:                                              ; preds = %bb3, %bb
  %i5 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  %i6 = load ptr, ptr %i5, align 8, !tbaa !804
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb9, label %bb8

bb8:                                              ; preds = %bb4
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(ptr noundef nonnull align 8 dereferenceable(32) %i6) #36
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef nonnull %i6) #36
  br label %bb9

bb9:                                              ; preds = %bb8, %bb4
  %i10 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  %i11 = load ptr, ptr %i10, align 8, !tbaa !805
  %i12 = icmp eq ptr %i11, null
  br i1 %i12, label %bb14, label %bb13

bb13:                                             ; preds = %bb9
  tail call void @_ZN11xercesc_2_716RefArrayVectorOfItED2Ev.5830(ptr noundef nonnull align 8 dereferenceable(40) %i11) #36
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef nonnull %i11) #36
  br label %bb14

bb14:                                             ; preds = %bb13, %bb9
  ret void
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !837 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !821
  %i1 = load ptr, ptr %i, align 8, !tbaa !821
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !820
  %i3 = load ptr, ptr %i2, align 8, !tbaa !820
  %i4 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i1, i64 0, i32 0
  %i5 = load ptr, ptr %i4, align 8, !tbaa !776
  %i6 = call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  %i7 = getelementptr inbounds ptr, ptr %i5, i64 3
  %i8 = load ptr, ptr %i7, align 8
  %i9 = icmp eq ptr %i8, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i9, label %bb10, label %bb11

bb10:                                             ; preds = %bb
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i1, ptr noundef %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb11:                                             ; preds = %bb
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i1, ptr noundef %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb12:                                             ; preds = %bb11, %bb10
  br label %bb13

bb13:                                             ; preds = %bb12
  ret void

bb14:                                             ; preds = %bb11, %bb10
  %i15 = landingpad { ptr, i32 }
          catch ptr null
  %i16 = extractvalue { ptr, i32 } %i15, 0
  tail call void @__clang_call_terminate(ptr %i16) #38
  unreachable
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !839 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  %i1 = load ptr, ptr %i, align 8, !tbaa !832
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  %i3 = load ptr, ptr %i2, align 8, !tbaa !831
  %i4 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i1, i64 0, i32 0
  %i5 = load ptr, ptr %i4, align 8, !tbaa !776
  %i6 = call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  %i7 = getelementptr inbounds ptr, ptr %i5, i64 3
  %i8 = load ptr, ptr %i7, align 8
  %i9 = icmp eq ptr %i8, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i9, label %bb10, label %bb11

bb10:                                             ; preds = %bb
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i1, ptr noundef %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb11:                                             ; preds = %bb
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i1, ptr noundef %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb12:                                             ; preds = %bb11, %bb10
  br label %bb13

bb13:                                             ; preds = %bb12
  ret void

bb14:                                             ; preds = %bb11, %bb10
  %i15 = landingpad { ptr, i32 }
          catch ptr null
  %i16 = extractvalue { ptr, i32 } %i15, 0
  tail call void @__clang_call_terminate(ptr %i16) #38
  unreachable
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_713FieldValueMapD2Ev(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !840 {
bb:
  tail call void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr noundef nonnull align 8 dereferenceable(32) %arg)
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef i32 @_ZNK11xercesc_2_713FieldValueMap7indexOfEPKNS_8IC_FieldE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr noundef readnone "intel_dtrans_func_index"="2" %arg1) #21 align 2 !intel.dtrans.func.type !841 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  %i2 = load ptr, ptr %i, align 8, !tbaa !799
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb16, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(32) %i2)
  %i6 = icmp eq i32 %i5, 0
  br i1 %i6, label %bb16, label %bb7

bb7:                                              ; preds = %bb13, %bb4
  %i8 = phi i32 [ %i14, %bb13 ], [ 0, %bb4 ]
  %i9 = load ptr, ptr %i, align 8, !tbaa !799
  %i10 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i9, i32 noundef %i8)
  %i11 = load ptr, ptr %i10, align 8, !tbaa !822
  %i12 = icmp eq ptr %i11, %arg1
  br i1 %i12, label %bb16, label %bb13

bb13:                                             ; preds = %bb7
  %i14 = add nuw i32 %i8, 1
  %i15 = icmp eq i32 %i14, %i5
  br i1 %i15, label %bb16, label %bb7, !llvm.loop !843

bb16:                                             ; preds = %bb13, %bb7, %bb4, %bb
  %i17 = phi i32 [ -1, %bb ], [ -1, %bb4 ], [ -1, %bb13 ], [ %i8, %bb7 ]
  ret i32 %i17
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define hidden noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE4sizeEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) #22 align 2 !intel.dtrans.func.type !844 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  %i1 = load i32, ptr %i, align 4, !tbaa !818
  ret i32 %i1
}

; Function Attrs: mustprogress uwtable
define hidden noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #21 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !845 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  %i2 = load i32, ptr %i, align 4, !tbaa !818
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !821
  %i7 = load ptr, ptr %i6, align 8, !tbaa !821
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.1.1140, i32 noundef 206, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !820
  %i13 = load ptr, ptr %i12, align 8, !tbaa !820
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  ret ptr %i15
}

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare !intel.dtrans.func.type !846 dso_local i64 @strlen(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #23

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define hidden noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef writeonly "intel_dtrans_func_index"="2" %arg2, i32 noundef %arg3) unnamed_addr #24 align 2 !intel.dtrans.func.type !847 {
bb:
  %i = zext i32 %arg3 to i64
  %i4 = getelementptr inbounds i16, ptr %arg2, i64 %i, !intel-tbaa !787
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader", ptr %arg, i64 0, i32 1, !intel-tbaa !849
  %i6 = load ptr, ptr %i5, align 8, !tbaa !849
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb93, label %bb8

bb8:                                              ; preds = %bb
  %i9 = load i16, ptr %i6, align 2, !tbaa !787
  %i10 = icmp eq i16 %i9, 104
  br i1 %i10, label %bb11, label %bb93

bb11:                                             ; preds = %bb16, %bb8
  %i12 = phi i16 [ %i19, %bb16 ], [ 104, %bb8 ]
  %i13 = phi ptr [ %i18, %bb16 ], [ @_ZN11xercesc_2_76XMLUni14fgXMLErrDomainE, %bb8 ]
  %i14 = phi ptr [ %i17, %bb16 ], [ %i6, %bb8 ]
  %i15 = icmp eq i16 %i12, 0
  br i1 %i15, label %bb23, label %bb16

bb16:                                             ; preds = %bb11
  %i17 = getelementptr inbounds i16, ptr %i14, i64 1
  %i18 = getelementptr inbounds i16, ptr %i13, i64 1
  %i19 = load i16, ptr %i17, align 2, !tbaa !787
  %i20 = getelementptr [41 x i16], ptr %i18, i64 0, i32 0
  %i21 = load i16, ptr %i20, align 2, !tbaa !787
  %i22 = icmp eq i16 %i19, %i21
  br i1 %i22, label %bb11, label %bb30, !llvm.loop !853

bb23:                                             ; preds = %bb11
  %i24 = icmp ugt i32 %arg1, 312
  br i1 %i24, label %bb111, label %bb25

bb25:                                             ; preds = %bb23
  %i26 = add nsw i32 %arg1, -1
  %i27 = zext i32 %i26 to i64
  %i28 = getelementptr inbounds [311 x [128 x i16]], ptr @_ZN11xercesc_2_7L12gXMLErrArrayE, i64 0, i64 %i27, !intel-tbaa !854
  %i29 = getelementptr inbounds [128 x i16], ptr %i28, i64 0, i64 0
  br label %bb93

bb30:                                             ; preds = %bb16
  %i31 = icmp eq i16 %i9, 104
  br i1 %i31, label %bb32, label %bb93

bb32:                                             ; preds = %bb37, %bb30
  %i33 = phi i16 [ %i40, %bb37 ], [ 104, %bb30 ]
  %i34 = phi ptr [ %i39, %bb37 ], [ @_ZN11xercesc_2_76XMLUni14fgExceptDomainE, %bb30 ]
  %i35 = phi ptr [ %i38, %bb37 ], [ %i6, %bb30 ]
  %i36 = icmp eq i16 %i33, 0
  br i1 %i36, label %bb44, label %bb37

bb37:                                             ; preds = %bb32
  %i38 = getelementptr inbounds i16, ptr %i35, i64 1
  %i39 = getelementptr inbounds i16, ptr %i34, i64 1
  %i40 = load i16, ptr %i38, align 2, !tbaa !787
  %i41 = getelementptr [43 x i16], ptr %i39, i64 0, i32 0
  %i42 = load i16, ptr %i41, align 2, !tbaa !787
  %i43 = icmp eq i16 %i40, %i42
  br i1 %i43, label %bb32, label %bb51, !llvm.loop !853

bb44:                                             ; preds = %bb32
  %i45 = icmp ugt i32 %arg1, 404
  br i1 %i45, label %bb111, label %bb46

bb46:                                             ; preds = %bb44
  %i47 = add nsw i32 %arg1, -1
  %i48 = zext i32 %i47 to i64
  %i49 = getelementptr inbounds [401 x [128 x i16]], ptr @_ZN11xercesc_2_7L15gXMLExceptArrayE, i64 0, i64 %i48, !intel-tbaa !857
  %i50 = getelementptr inbounds [128 x i16], ptr %i49, i64 0, i64 0
  br label %bb93

bb51:                                             ; preds = %bb37
  %i52 = icmp eq i16 %i9, 104
  br i1 %i52, label %bb53, label %bb93

bb53:                                             ; preds = %bb58, %bb51
  %i54 = phi i16 [ %i61, %bb58 ], [ 104, %bb51 ]
  %i55 = phi ptr [ %i60, %bb58 ], [ @_ZN11xercesc_2_76XMLUni16fgValidityDomainE, %bb51 ]
  %i56 = phi ptr [ %i59, %bb58 ], [ %i6, %bb51 ]
  %i57 = icmp eq i16 %i54, 0
  br i1 %i57, label %bb65, label %bb58

bb58:                                             ; preds = %bb53
  %i59 = getelementptr inbounds i16, ptr %i56, i64 1
  %i60 = getelementptr inbounds i16, ptr %i55, i64 1
  %i61 = load i16, ptr %i59, align 2, !tbaa !787
  %i62 = getelementptr [43 x i16], ptr %i60, i64 0, i32 0
  %i63 = load i16, ptr %i62, align 2, !tbaa !787
  %i64 = icmp eq i16 %i61, %i63
  br i1 %i64, label %bb53, label %bb72, !llvm.loop !853

bb65:                                             ; preds = %bb53
  %i66 = icmp ugt i32 %arg1, 119
  br i1 %i66, label %bb111, label %bb67

bb67:                                             ; preds = %bb65
  %i68 = add nsw i32 %arg1, -1
  %i69 = zext i32 %i68 to i64
  %i70 = getelementptr inbounds [114 x [128 x i16]], ptr @_ZN11xercesc_2_7L17gXMLValidityArrayE, i64 0, i64 %i69, !intel-tbaa !859
  %i71 = getelementptr inbounds [128 x i16], ptr %i70, i64 0, i64 0
  br label %bb93

bb72:                                             ; preds = %bb58
  %i73 = icmp eq i16 %i9, 104
  br i1 %i73, label %bb74, label %bb93

bb74:                                             ; preds = %bb79, %bb72
  %i75 = phi i16 [ %i82, %bb79 ], [ 104, %bb72 ]
  %i76 = phi ptr [ %i81, %bb79 ], [ @_ZN11xercesc_2_76XMLUni17fgXMLDOMMsgDomainE, %bb72 ]
  %i77 = phi ptr [ %i80, %bb79 ], [ %i6, %bb72 ]
  %i78 = icmp eq i16 %i75, 0
  br i1 %i78, label %bb86, label %bb79

bb79:                                             ; preds = %bb74
  %i80 = getelementptr inbounds i16, ptr %i77, i64 1
  %i81 = getelementptr inbounds i16, ptr %i76, i64 1
  %i82 = load i16, ptr %i80, align 2, !tbaa !787
  %i83 = getelementptr [41 x i16], ptr %i81, i64 0, i32 0
  %i84 = load i16, ptr %i83, align 2, !tbaa !787
  %i85 = icmp eq i16 %i82, %i84
  br i1 %i85, label %bb74, label %bb93, !llvm.loop !853

bb86:                                             ; preds = %bb74
  %i87 = icmp ugt i32 %arg1, 30
  br i1 %i87, label %bb111, label %bb88

bb88:                                             ; preds = %bb86
  %i89 = add nsw i32 %arg1, -1
  %i90 = zext i32 %i89 to i64
  %i91 = getelementptr inbounds [25 x [128 x i16]], ptr @_ZN11xercesc_2_7L15gXMLDOMMsgArrayE, i64 0, i64 %i90, !intel-tbaa !861
  %i92 = getelementptr inbounds [128 x i16], ptr %i91, i64 0, i64 0
  br label %bb93

bb93:                                             ; preds = %bb88, %bb79, %bb72, %bb67, %bb51, %bb46, %bb30, %bb25, %bb8, %bb
  %i94 = phi ptr [ %i29, %bb25 ], [ %i50, %bb46 ], [ %i71, %bb67 ], [ %i92, %bb88 ], [ null, %bb72 ], [ null, %bb79 ], [ null, %bb51 ], [ null, %bb30 ], [ null, %bb8 ], [ null, %bb ]
  %i95 = load i16, ptr %i94, align 2, !tbaa !787
  %i96 = icmp ne i16 %i95, 0
  %i97 = icmp ne i32 %arg3, 0
  %i98 = and i1 %i96, %i97
  br i1 %i98, label %bb99, label %bb109

bb99:                                             ; preds = %bb99, %bb93
  %i100 = phi i16 [ %i105, %bb99 ], [ %i95, %bb93 ]
  %i101 = phi ptr [ %i103, %bb99 ], [ %i94, %bb93 ]
  %i102 = phi ptr [ %i104, %bb99 ], [ %arg2, %bb93 ]
  %i103 = getelementptr inbounds i16, ptr %i101, i64 1
  %i104 = getelementptr inbounds i16, ptr %i102, i64 1
  store i16 %i100, ptr %i102, align 2, !tbaa !787
  %i105 = load i16, ptr %i103, align 2, !tbaa !787
  %i106 = icmp ne i16 %i105, 0
  %i107 = icmp ult ptr %i104, %i4
  %i108 = select i1 %i106, i1 %i107, i1 false
  br i1 %i108, label %bb99, label %bb109, !llvm.loop !863

bb109:                                            ; preds = %bb99, %bb93
  %i110 = phi ptr [ %arg2, %bb93 ], [ %i104, %bb99 ]
  store i16 0, ptr %i110, align 2, !tbaa !787
  br label %bb111

bb111:                                            ; preds = %bb109, %bb86, %bb65, %bb44, %bb23
  %i112 = phi i1 [ true, %bb109 ], [ false, %bb23 ], [ false, %bb44 ], [ false, %bb65 ], [ false, %bb86 ]
  ret i1 %i112
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
declare !intel.dtrans.func.type !864 hidden void @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev(ptr nocapture noundef nonnull align 1 dereferenceable(1) "intel_dtrans_func_index"="1") unnamed_addr #25 align 2

; Function Attrs: uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1) unnamed_addr #26 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !866 !_Intel.Devirt.Target !868 {
bb:
  %i = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %arg1) #40
          to label %bb9 unwind label %bb2

bb2:                                              ; preds = %bb
  %i3 = landingpad { ptr, i32 }
          catch ptr null
  %i4 = extractvalue { ptr, i32 } %i3, 0
  %i5 = tail call ptr @__cxa_begin_catch(ptr %i4) #36
  %i6 = tail call ptr @__cxa_allocate_exception(i64 1) #36
  invoke void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_720OutOfMemoryExceptionE, ptr nonnull @_ZN11xercesc_2_720OutOfMemoryExceptionD2Ev) #37
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
  tail call void @__clang_call_terminate(ptr %i13) #38
  unreachable

bb14:                                             ; preds = %bb2
  unreachable
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr nocapture nonnull readnone align 8 "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) unnamed_addr #27 align 2 !intel.dtrans.func.type !869 !_Intel.Devirt.Target !868 {
bb:
  tail call void @_ZdlPv(ptr noundef %arg1) #36
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define hidden noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_712PanicHandler20getPanicReasonStringENS0_12PanicReasonsE(i32 noundef %arg) #28 align 2 !intel.dtrans.func.type !870 {
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

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef "intel_dtrans_func_index"="2" %arg) #0 align 2 !intel.dtrans.func.type !871 {
bb:
  %i = tail call noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils11loadAMsgSetEPKt(ptr noundef %arg)
  ret ptr %i
}

; Function Attrs: inlinehint mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #29 align 2 !intel.dtrans.func.type !872 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  %i2 = load ptr, ptr %i, align 8, !tbaa !804
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb7, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i2, i32 noundef %arg1)
  %i6 = load ptr, ptr %i5, align 8, !tbaa !833
  br label %bb7

bb7:                                              ; preds = %bb4, %bb
  %i8 = phi ptr [ %i6, %bb4 ], [ null, %bb ]
  ret ptr %i8
}

; Function Attrs: inlinehint mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #29 align 2 !intel.dtrans.func.type !873 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  %i2 = load ptr, ptr %i, align 8, !tbaa !805
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb6, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj.5826(ptr noundef nonnull align 8 dereferenceable(40) %i2, i32 noundef %arg1)
  br label %bb6

bb6:                                              ; preds = %bb4, %bb
  %i7 = phi ptr [ %i5, %bb4 ], [ null, %bb ]
  ret ptr %i7
}

; Function Attrs: inlinehint mustprogress uwtable
define hidden void @_ZN11xercesc_2_713FieldValueMap3putEPNS_8IC_FieldEPNS_17DatatypeValidatorEPKt(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3) #29 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !874 {
bb:
  %i = alloca ptr, align 8, !intel_dtrans_type !361
  %i4 = alloca ptr, align 8, !intel_dtrans_type !842
  %i5 = alloca ptr, align 8, !intel_dtrans_type !436
  store ptr %arg1, ptr %i4, align 8, !tbaa !822
  store ptr %arg2, ptr %i5, align 8, !tbaa !833
  %i6 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  %i7 = load ptr, ptr %i6, align 8, !tbaa !799
  %i8 = icmp eq ptr %i7, null
  br i1 %i8, label %bb9, label %bb32

bb9:                                              ; preds = %bb
  %i10 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 3, !intel-tbaa !806
  %i11 = load ptr, ptr %i10, align 8, !tbaa !806
  %i12 = tail call noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i11)
  %i13 = load ptr, ptr %i10, align 8, !tbaa !806
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(32) %i12, i32 noundef 1, ptr noundef %i13, i1 noundef zeroext false)
          to label %bb14 unwind label %bb26

bb14:                                             ; preds = %bb9
  store ptr %i12, ptr %i6, align 8, !tbaa !799
  %i15 = load ptr, ptr %i10, align 8, !tbaa !806
  %i16 = tail call noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i15)
  %i17 = load ptr, ptr %i10, align 8, !tbaa !806
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2EjPNS_13MemoryManagerEb(ptr noundef nonnull align 8 dereferenceable(32) %i16, i32 noundef 1, ptr noundef %i17, i1 noundef zeroext false)
          to label %bb18 unwind label %bb28

bb18:                                             ; preds = %bb14
  %i19 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  store ptr %i16, ptr %i19, align 8, !tbaa !804
  %i20 = load ptr, ptr %i10, align 8, !tbaa !806
  %i21 = tail call noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i20)
  %i22 = load ptr, ptr %i10, align 8, !tbaa !806
  invoke void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE.5825(ptr noundef nonnull align 8 dereferenceable(40) %i21, i32 noundef 1, ptr noundef %i22, i1 noundef zeroext false)
          to label %bb23 unwind label %bb30

bb23:                                             ; preds = %bb18
  br label %bb24

bb24:                                             ; preds = %bb23
  %i25 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  store ptr %i21, ptr %i25, align 8, !tbaa !805
  br label %bb32

bb26:                                             ; preds = %bb9
  %i27 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i12, ptr noundef %i11) #36
  br label %bb53

bb28:                                             ; preds = %bb14
  %i29 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i16, ptr noundef %i15) #36
  br label %bb53

bb30:                                             ; preds = %bb18
  %i31 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %i21, ptr noundef %i20) #36
  br label %bb53

bb32:                                             ; preds = %bb24, %bb
  %i33 = tail call noundef i32 @_ZNK11xercesc_2_713FieldValueMap7indexOfEPKNS_8IC_FieldE(ptr noundef nonnull align 8 dereferenceable(32) %arg, ptr noundef %arg1)
  %i34 = icmp eq i32 %i33, -1
  br i1 %i34, label %bb35, label %bb44

bb35:                                             ; preds = %bb32
  %i36 = load ptr, ptr %i6, align 8, !tbaa !799
  call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE10addElementERKS2_(ptr noundef nonnull align 8 dereferenceable(32) %i36, ptr noundef nonnull align 8 dereferenceable(8) %i4)
  %i37 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  %i38 = load ptr, ptr %i37, align 8, !tbaa !804
  call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE10addElementERKS2_(ptr noundef nonnull align 8 dereferenceable(32) %i38, ptr noundef nonnull align 8 dereferenceable(8) %i5)
  %i39 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  %i40 = load ptr, ptr %i39, align 8, !tbaa !805
  %i41 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 3, !intel-tbaa !806
  %i42 = load ptr, ptr %i41, align 8, !tbaa !806
  %i43 = call noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef %arg3, ptr noundef %i42)
  store ptr %i43, ptr %i, align 8
  call void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt.5827(ptr noundef nonnull align 8 dereferenceable(40) %i40, ptr noundef %i)
  br label %bb52

bb44:                                             ; preds = %bb32
  %i45 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 1, !intel-tbaa !804
  %i46 = load ptr, ptr %i45, align 8, !tbaa !804
  call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE12setElementAtERKS2_j(ptr noundef nonnull align 8 dereferenceable(32) %i46, ptr noundef nonnull align 8 dereferenceable(8) %i5, i32 noundef %i33)
  %i47 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 2, !intel-tbaa !805
  %i48 = load ptr, ptr %i47, align 8, !tbaa !805
  %i49 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 3, !intel-tbaa !806
  %i50 = load ptr, ptr %i49, align 8, !tbaa !806
  %i51 = call noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef %arg3, ptr noundef %i50)
  call void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831(ptr noundef nonnull align 8 dereferenceable(40) %i48, ptr noundef %i51, i32 noundef %i33), !intel_dtrans_type !875, !_Intel.Devirt.Call !779
  br label %bb52

bb52:                                             ; preds = %bb44, %bb35
  ret void

bb53:                                             ; preds = %bb30, %bb28, %bb26
  %i54 = phi { ptr, i32 } [ %i31, %bb30 ], [ %i29, %bb28 ], [ %i27, %bb26 ]
  resume { ptr, i32 } %i54
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define hidden noundef i32 @_ZNK11xercesc_2_713FieldValueMap4sizeEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg) #30 align 2 !intel.dtrans.func.type !876 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap", ptr %arg, i64 0, i32 0, !intel-tbaa !799
  %i1 = load ptr, ptr %i, align 8, !tbaa !799
  %i2 = icmp eq ptr %i1, null
  br i1 %i2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
  %i4 = tail call noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(32) %i1)
  br label %bb5

bb5:                                              ; preds = %bb3, %bb
  %i6 = phi i32 [ %i4, %bb3 ], [ 0, %bb ]
  ret i32 %i6
}

; Function Attrs: mustprogress uwtable
define hidden noundef zeroext i1 @_ZN11xercesc_2_710ValueStore8containsEPKNS_13FieldValueMapE(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1) #0 align 2 !intel.dtrans.func.type !877 {
bb:
  %i = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %arg, i64 0, i32 4, !intel-tbaa !878
  %i2 = load ptr, ptr %i, align 8, !tbaa !878
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb40, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call noundef i32 @_ZNK11xercesc_2_713FieldValueMap4sizeEv(ptr noundef nonnull align 8 dereferenceable(32) %arg1)
  %i6 = load ptr, ptr %i, align 8, !tbaa !878
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %i6, i64 0, i32 2, !intel-tbaa !884
  %i8 = load i32, ptr %i7, align 4, !tbaa !884
  %i9 = icmp eq i32 %i8, 0
  br i1 %i9, label %bb40, label %bb10

bb10:                                             ; preds = %bb4
  %i11 = icmp eq i32 %i5, 0
  br i1 %i11, label %bb31, label %bb12

bb12:                                             ; preds = %bb28, %bb10
  %i13 = phi i32 [ %i29, %bb28 ], [ 0, %bb10 ]
  %i14 = load ptr, ptr %i, align 8, !tbaa !878
  %i15 = tail call noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i14, i32 noundef %i13)
  %i16 = tail call noundef i32 @_ZNK11xercesc_2_713FieldValueMap4sizeEv(ptr noundef nonnull align 8 dereferenceable(32) %i15)
  %i17 = icmp eq i32 %i5, %i16
  br i1 %i17, label %bb21, label %bb28

bb18:                                             ; preds = %bb21
  %i19 = add nuw i32 %i22, 1
  %i20 = icmp eq i32 %i19, %i5
  br i1 %i20, label %bb40, label %bb21, !llvm.loop !887

bb21:                                             ; preds = %bb18, %bb12
  %i22 = phi i32 [ %i19, %bb18 ], [ 0, %bb12 ]
  %i23 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i15, i32 noundef %i22)
  %i24 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %i15, i32 noundef %i22)
  %i25 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap22getDatatypeValidatorAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i22)
  %i26 = tail call noundef ptr @_ZNK11xercesc_2_713FieldValueMap10getValueAtEj(ptr noundef nonnull align 8 dereferenceable(32) %arg1, i32 noundef %i22)
  %i27 = tail call noundef zeroext i1 @_ZN11xercesc_2_710ValueStore13isDuplicateOfEPNS_17DatatypeValidatorEPKtS2_S4_(ptr noundef nonnull align 8 dereferenceable(80) %arg, ptr noundef %i23, ptr noundef %i24, ptr noundef %i25, ptr noundef %i26)
  br i1 %i27, label %bb18, label %bb28

bb28:                                             ; preds = %bb21, %bb12
  %i29 = add nuw i32 %i13, 1
  %i30 = icmp eq i32 %i29, %i8
  br i1 %i30, label %bb40, label %bb12, !llvm.loop !888

bb31:                                             ; preds = %bb37, %bb10
  %i32 = phi i32 [ %i38, %bb37 ], [ 0, %bb10 ]
  %i33 = load ptr, ptr %i, align 8, !tbaa !878
  %i34 = tail call noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr noundef nonnull align 8 dereferenceable(40) %i33, i32 noundef %i32)
  %i35 = tail call noundef i32 @_ZNK11xercesc_2_713FieldValueMap4sizeEv(ptr noundef nonnull align 8 dereferenceable(32) %i34)
  %i36 = icmp eq i32 %i35, 0
  br i1 %i36, label %bb40, label %bb37

bb37:                                             ; preds = %bb31
  %i38 = add nuw i32 %i32, 1
  %i39 = icmp eq i32 %i38, %i8
  br i1 %i39, label %bb40, label %bb31, !llvm.loop !888

bb40:                                             ; preds = %bb37, %bb31, %bb28, %bb18, %bb4, %bb
  %i41 = phi i1 [ false, %bb ], [ false, %bb4 ], [ true, %bb31 ], [ false, %bb37 ], [ true, %bb18 ], [ false, %bb28 ]
  ret i1 %i41
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #0 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !889 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !884
  %i2 = load i32, ptr %i, align 4, !tbaa !884
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 5, !intel-tbaa !891
  %i7 = load ptr, ptr %i6, align 8, !tbaa !891
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.2.2471, i32 noundef 249, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !892
  %i13 = load ptr, ptr %i12, align 8, !tbaa !892
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  %i16 = load ptr, ptr %i15, align 8, !tbaa !893
  ret ptr %i16
}

; Function Attrs: mustprogress uwtable
define hidden noundef zeroext i1 @_ZN11xercesc_2_710ValueStore13isDuplicateOfEPNS_17DatatypeValidatorEPKtS2_S4_(ptr nocapture noundef nonnull readonly align 8 dereferenceable(80) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, ptr noundef "intel_dtrans_func_index"="3" %arg2, ptr noundef "intel_dtrans_func_index"="4" %arg3, ptr noundef "intel_dtrans_func_index"="5" %arg4) #0 align 2 !intel.dtrans.func.type !895 {
bb:
  %i = icmp ne ptr %arg1, null
  %i5 = icmp ne ptr %arg3, null
  %i6 = and i1 %i, %i5
  %i7 = icmp eq ptr %arg2, null
  %i8 = icmp eq ptr %arg4, null
  br i1 %i6, label %bb35, label %bb9

bb9:                                              ; preds = %bb
  %i10 = or i1 %i7, %i8
  br i1 %i10, label %bb15, label %bb11

bb11:                                             ; preds = %bb9
  %i12 = load i16, ptr %arg2, align 2, !tbaa !787
  %i13 = load i16, ptr %arg4, align 2, !tbaa !787
  %i14 = icmp eq i16 %i12, %i13
  br i1 %i14, label %bb24, label %bb76

bb15:                                             ; preds = %bb9
  br i1 %i7, label %bb19, label %bb16

bb16:                                             ; preds = %bb15
  %i17 = load i16, ptr %arg2, align 2, !tbaa !787
  %i18 = icmp eq i16 %i17, 0
  br i1 %i18, label %bb19, label %bb76

bb19:                                             ; preds = %bb16, %bb15
  br i1 %i8, label %bb23, label %bb20

bb20:                                             ; preds = %bb19
  %i21 = load i16, ptr %arg4, align 2, !tbaa !787
  %i22 = icmp eq i16 %i21, 0
  br i1 %i22, label %bb23, label %bb76

bb23:                                             ; preds = %bb20, %bb19
  br label %bb76

bb24:                                             ; preds = %bb29, %bb11
  %i25 = phi i16 [ %i32, %bb29 ], [ %i12, %bb11 ]
  %i26 = phi ptr [ %i31, %bb29 ], [ %arg4, %bb11 ]
  %i27 = phi ptr [ %i30, %bb29 ], [ %arg2, %bb11 ]
  %i28 = icmp eq i16 %i25, 0
  br i1 %i28, label %bb76, label %bb29

bb29:                                             ; preds = %bb24
  %i30 = getelementptr inbounds i16, ptr %i27, i64 1
  %i31 = getelementptr inbounds i16, ptr %i26, i64 1
  %i32 = load i16, ptr %i30, align 2, !tbaa !787
  %i33 = load i16, ptr %i31, align 2, !tbaa !787
  %i34 = icmp eq i16 %i32, %i33
  br i1 %i34, label %bb24, label %bb76, !llvm.loop !896

bb35:                                             ; preds = %bb
  br i1 %i7, label %bb40, label %bb36

bb36:                                             ; preds = %bb35
  %i37 = load i16, ptr %arg2, align 2, !tbaa !787
  %i38 = icmp ne i16 %i37, 0
  %i39 = zext i1 %i38 to i32
  br label %bb40

bb40:                                             ; preds = %bb36, %bb35
  %i41 = phi i1 [ false, %bb35 ], [ %i38, %bb36 ]
  %i42 = phi i32 [ 0, %bb35 ], [ %i39, %bb36 ]
  br i1 %i8, label %bb47, label %bb43

bb43:                                             ; preds = %bb40
  %i44 = load i16, ptr %arg4, align 2, !tbaa !787
  %i45 = icmp ne i16 %i44, 0
  %i46 = zext i1 %i45 to i32
  br label %bb47

bb47:                                             ; preds = %bb43, %bb40
  %i48 = phi i1 [ false, %bb40 ], [ %i45, %bb43 ]
  %i49 = phi i32 [ 0, %bb40 ], [ %i46, %bb43 ]
  %i50 = or i32 %i49, %i42
  %i51 = icmp eq i32 %i50, 0
  br i1 %i51, label %bb52, label %bb54

bb52:                                             ; preds = %bb47
  %i53 = icmp eq ptr %arg1, %arg3
  br label %bb76

bb54:                                             ; preds = %bb47
  %i55 = and i1 %i41, %i48
  br i1 %i55, label %bb56, label %bb76

bb56:                                             ; preds = %bb54
  %i57 = icmp eq ptr %arg1, %arg3
  %i58 = getelementptr inbounds %"_DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore", ptr %arg, i64 0, i32 7
  %i59 = load ptr, ptr %i58, align 8, !tbaa !897
  br i1 %i57, label %bb60, label %bb68

bb60:                                             ; preds = %bb56
  %i61 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %arg1, i64 0, i32 0, i32 0
  %i62 = load ptr, ptr %i61, align 8, !tbaa !776
  %i63 = call i1 @llvm.type.test(ptr %i62, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i63)
  %i64 = getelementptr inbounds ptr, ptr %i62, i64 10
  %i65 = load ptr, ptr %i64, align 8
  %i66 = tail call noundef i32 %i65(ptr noundef nonnull align 8 dereferenceable(104) %arg1, ptr noundef %arg2, ptr noundef %arg4, ptr noundef %i59), !intel_dtrans_type !898
  %i67 = icmp eq i32 %i66, 0
  br label %bb76

bb68:                                             ; preds = %bb56
  %i69 = getelementptr %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator", ptr %arg3, i64 0, i32 0, i32 0
  %i70 = load ptr, ptr %i69, align 8, !tbaa !776
  %i71 = call i1 @llvm.type.test(ptr %i70, metadata !"_ZTSN11xercesc_2_717DatatypeValidatorE")
  tail call void @llvm.assume(i1 %i71)
  %i72 = getelementptr inbounds ptr, ptr %i70, i64 10
  %i73 = load ptr, ptr %i72, align 8
  %i74 = tail call noundef i32 %i73(ptr noundef nonnull align 8 dereferenceable(104) %arg3, ptr noundef %arg2, ptr noundef %arg4, ptr noundef %i59), !intel_dtrans_type !898
  %i75 = icmp eq i32 %i74, 0
  br label %bb76

bb76:                                             ; preds = %bb68, %bb60, %bb54, %bb52, %bb29, %bb24, %bb23, %bb20, %bb16, %bb11
  %i77 = phi i1 [ %i67, %bb60 ], [ %i75, %bb68 ], [ false, %bb54 ], [ %i53, %bb52 ], [ true, %bb23 ], [ false, %bb20 ], [ false, %bb16 ], [ false, %bb11 ], [ true, %bb24 ], [ false, %bb29 ]
  ret i1 %i77
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2EjPNS_13MemoryManagerEb(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, i1 noundef zeroext %arg3) unnamed_addr #18 align 2 !intel.dtrans.func.type !899 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !811
  store i8 0, ptr %i, align 8, !tbaa !811
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  store i32 0, ptr %i4, align 4, !tbaa !818
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !819
  store i32 1, ptr %i5, align 8, !tbaa !819
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !820
  store ptr null, ptr %i6, align 8, !tbaa !820
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !821
  store ptr %arg2, ptr %i7, align 8, !tbaa !821
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !776
  %i10 = call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 2
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i13, label %bb14, label %bb16

bb14:                                             ; preds = %bb
  %i15 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef 8), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb18

bb16:                                             ; preds = %bb
  %i17 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef 8), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb18

bb18:                                             ; preds = %bb16, %bb14
  %i19 = phi ptr [ %i15, %bb14 ], [ %i17, %bb16 ]
  br label %bb20

bb20:                                             ; preds = %bb18
  store ptr %i19, ptr %i6, align 8, !tbaa !820
  tail call void @llvm.memset.p0.i64(ptr align 8 %i19, i8 0, i64 8, i1 false)
  ret void
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2EjPNS_13MemoryManagerEb(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, i1 noundef zeroext %arg3) unnamed_addr #18 align 2 !intel.dtrans.func.type !900 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !826
  store i8 0, ptr %i, align 8, !tbaa !826
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  store i32 0, ptr %i4, align 4, !tbaa !829
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !830
  store i32 1, ptr %i5, align 8, !tbaa !830
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  store ptr null, ptr %i6, align 8, !tbaa !831
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  store ptr %arg2, ptr %i7, align 8, !tbaa !832
  %i8 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i9 = load ptr, ptr %i8, align 8, !tbaa !776
  %i10 = call i1 @llvm.type.test(ptr %i9, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i10)
  %i11 = getelementptr inbounds ptr, ptr %i9, i64 2
  %i12 = load ptr, ptr %i11, align 8
  %i13 = icmp eq ptr %i12, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i13, label %bb14, label %bb16

bb14:                                             ; preds = %bb
  %i15 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef 8), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb18

bb16:                                             ; preds = %bb
  %i17 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef 8), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb18

bb18:                                             ; preds = %bb16, %bb14
  %i19 = phi ptr [ %i15, %bb14 ], [ %i17, %bb16 ]
  br label %bb20

bb20:                                             ; preds = %bb18
  store ptr %i19, ptr %i6, align 8, !tbaa !831
  tail call void @llvm.memset.p0.i64(ptr align 8 %i19, i8 0, i64 8, i1 false)
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE10addElementERKS2_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg1) #21 align 2 !intel.dtrans.func.type !901 {
bb:
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr noundef nonnull align 8 dereferenceable(32) %arg, i32 noundef 1)
  %i = load ptr, ptr %arg1, align 8, !tbaa !822
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !820
  %i3 = load ptr, ptr %i2, align 8, !tbaa !820
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  %i5 = load i32, ptr %i4, align 4, !tbaa !818
  %i6 = zext i32 %i5 to i64
  %i7 = getelementptr inbounds ptr, ptr %i3, i64 %i6
  store ptr %i, ptr %i7, align 8, !tbaa !822
  %i8 = add i32 %i5, 1
  store i32 %i8, ptr %i4, align 4, !tbaa !818
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE10addElementERKS2_(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg1) #21 align 2 !intel.dtrans.func.type !902 {
bb:
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE19ensureExtraCapacityEj(ptr noundef nonnull align 8 dereferenceable(32) %arg, i32 noundef 1)
  %i = load ptr, ptr %arg1, align 8, !tbaa !833
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  %i3 = load ptr, ptr %i2, align 8, !tbaa !831
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  %i5 = load i32, ptr %i4, align 4, !tbaa !829
  %i6 = zext i32 %i5 to i64
  %i7 = getelementptr inbounds ptr, ptr %i3, i64 %i6
  store ptr %i, ptr %i7, align 8, !tbaa !833
  %i8 = add i32 %i5, 1
  store i32 %i8, ptr %i4, align 4, !tbaa !829
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE12setElementAtERKS2_j(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) #21 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !903 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  %i3 = load i32, ptr %i, align 4, !tbaa !829
  %i4 = icmp ugt i32 %i3, %arg2
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  %i8 = load ptr, ptr %i7, align 8, !tbaa !832
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.1.2472, i32 noundef 126, i32 noundef 116, ptr noundef %i8)
          to label %bb9 unwind label %bb10

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb10:                                             ; preds = %bb5
  %i11 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #36
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb
  %i13 = load ptr, ptr %arg1, align 8, !tbaa !833
  %i14 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  %i15 = load ptr, ptr %i14, align 8, !tbaa !831
  %i16 = zext i32 %arg2 to i64
  %i17 = getelementptr inbounds ptr, ptr %i15, i64 %i16
  store ptr %i13, ptr %i17, align 8, !tbaa !833
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE19ensureExtraCapacityEj(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) #21 align 2 !intel.dtrans.func.type !904 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  %i2 = load i32, ptr %i, align 4, !tbaa !829
  %i3 = add i32 %i2, 1
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !830
  %i5 = load i32, ptr %i4, align 8, !tbaa !830
  %i6 = icmp ugt i32 %i3, %i5
  br i1 %i6, label %bb7, label %bb55

bb7:                                              ; preds = %bb
  %i8 = uitofp i32 %i2 to double
  %i9 = fmul fast double %i8, 1.250000e+00
  %i10 = fptoui double %i9 to i32
  %i11 = icmp ult i32 %i3, %i10
  %i12 = select i1 %i11, i32 %i10, i32 %i3
  %i13 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  %i14 = load ptr, ptr %i13, align 8, !tbaa !832
  %i15 = zext i32 %i12 to i64
  %i16 = shl nuw nsw i64 %i15, 3
  %i17 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 8, !tbaa !776
  %i19 = call i1 @llvm.type.test(ptr %i18, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i19)
  %i20 = getelementptr inbounds ptr, ptr %i18, i64 2
  %i21 = load ptr, ptr %i20, align 8
  %i22 = icmp eq ptr %i21, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i22, label %bb23, label %bb25

bb23:                                             ; preds = %bb7
  %i24 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i14, i64 noundef %i16), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb27

bb25:                                             ; preds = %bb7
  %i26 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i14, i64 noundef %i16), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb27

bb27:                                             ; preds = %bb25, %bb23
  %i28 = phi ptr [ %i24, %bb23 ], [ %i26, %bb25 ]
  br label %bb29

bb29:                                             ; preds = %bb27
  %i30 = load i32, ptr %i, align 4, !tbaa !829
  %i31 = icmp eq i32 %i30, 0
  %i32 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3
  %i33 = load ptr, ptr %i32, align 8, !tbaa !831
  br i1 %i31, label %bb36, label %bb34

bb34:                                             ; preds = %bb29
  %i35 = zext i32 %i30 to i64
  br label %bb48

bb36:                                             ; preds = %bb48, %bb29
  %i37 = load ptr, ptr %i13, align 8, !tbaa !832
  %i38 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i37, i64 0, i32 0
  %i39 = load ptr, ptr %i38, align 8, !tbaa !776
  %i40 = call i1 @llvm.type.test(ptr %i39, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i40)
  %i41 = getelementptr inbounds ptr, ptr %i39, i64 3
  %i42 = load ptr, ptr %i41, align 8
  %i43 = icmp eq ptr %i42, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i43, label %bb44, label %bb45

bb44:                                             ; preds = %bb36
  tail call void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef %i33), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb46

bb45:                                             ; preds = %bb36
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef %i33), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb46

bb46:                                             ; preds = %bb45, %bb44
  br label %bb47

bb47:                                             ; preds = %bb46
  store ptr %i28, ptr %i32, align 8, !tbaa !831
  store i32 %i12, ptr %i4, align 8, !tbaa !830
  br label %bb55

bb48:                                             ; preds = %bb48, %bb34
  %i49 = phi i64 [ 0, %bb34 ], [ %i53, %bb48 ]
  %i50 = getelementptr inbounds ptr, ptr %i33, i64 %i49
  %i51 = load ptr, ptr %i50, align 8, !tbaa !833
  %i52 = getelementptr inbounds ptr, ptr %i28, i64 %i49
  store ptr %i51, ptr %i52, align 8, !tbaa !833
  %i53 = add nuw nsw i64 %i49, 1
  %i54 = icmp eq i64 %i53, %i35
  br i1 %i54, label %bb36, label %bb48, !llvm.loop !905

bb55:                                             ; preds = %bb47, %bb
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE19ensureExtraCapacityEj(ptr nocapture noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) #21 align 2 !intel.dtrans.func.type !906 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !818
  %i2 = load i32, ptr %i, align 4, !tbaa !818
  %i3 = add i32 %i2, 1
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !819
  %i5 = load i32, ptr %i4, align 8, !tbaa !819
  %i6 = icmp ugt i32 %i3, %i5
  br i1 %i6, label %bb7, label %bb55

bb7:                                              ; preds = %bb
  %i8 = uitofp i32 %i2 to double
  %i9 = fmul fast double %i8, 1.250000e+00
  %i10 = fptoui double %i9 to i32
  %i11 = icmp ult i32 %i3, %i10
  %i12 = select i1 %i11, i32 %i10, i32 %i3
  %i13 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !821
  %i14 = load ptr, ptr %i13, align 8, !tbaa !821
  %i15 = zext i32 %i12 to i64
  %i16 = shl nuw nsw i64 %i15, 3
  %i17 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i14, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 8, !tbaa !776
  %i19 = call i1 @llvm.type.test(ptr %i18, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i19)
  %i20 = getelementptr inbounds ptr, ptr %i18, i64 2
  %i21 = load ptr, ptr %i20, align 8
  %i22 = icmp eq ptr %i21, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i22, label %bb23, label %bb25

bb23:                                             ; preds = %bb7
  %i24 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i14, i64 noundef %i16), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb27

bb25:                                             ; preds = %bb7
  %i26 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i14, i64 noundef %i16), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb27

bb27:                                             ; preds = %bb25, %bb23
  %i28 = phi ptr [ %i24, %bb23 ], [ %i26, %bb25 ]
  br label %bb29

bb29:                                             ; preds = %bb27
  %i30 = load i32, ptr %i, align 4, !tbaa !818
  %i31 = icmp eq i32 %i30, 0
  %i32 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3
  %i33 = load ptr, ptr %i32, align 8, !tbaa !820
  br i1 %i31, label %bb36, label %bb34

bb34:                                             ; preds = %bb29
  %i35 = zext i32 %i30 to i64
  br label %bb48

bb36:                                             ; preds = %bb48, %bb29
  %i37 = load ptr, ptr %i13, align 8, !tbaa !821
  %i38 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i37, i64 0, i32 0
  %i39 = load ptr, ptr %i38, align 8, !tbaa !776
  %i40 = call i1 @llvm.type.test(ptr %i39, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i40)
  %i41 = getelementptr inbounds ptr, ptr %i39, i64 3
  %i42 = load ptr, ptr %i41, align 8
  %i43 = icmp eq ptr %i42, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i43, label %bb44, label %bb45

bb44:                                             ; preds = %bb36
  tail call void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef %i33), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb46

bb45:                                             ; preds = %bb36
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i37, ptr noundef %i33), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb46

bb46:                                             ; preds = %bb45, %bb44
  br label %bb47

bb47:                                             ; preds = %bb46
  store ptr %i28, ptr %i32, align 8, !tbaa !820
  store i32 %i12, ptr %i4, align 8, !tbaa !819
  br label %bb55

bb48:                                             ; preds = %bb48, %bb34
  %i49 = phi i64 [ 0, %bb34 ], [ %i53, %bb48 ]
  %i50 = getelementptr inbounds ptr, ptr %i33, i64 %i49
  %i51 = load ptr, ptr %i50, align 8, !tbaa !822
  %i52 = getelementptr inbounds ptr, ptr %i28, i64 %i49
  store ptr %i51, ptr %i52, align 8, !tbaa !822
  %i53 = add nuw nsw i64 %i49, 1
  %i54 = icmp eq i64 %i53, %i35
  br i1 %i54, label %bb36, label %bb48, !llvm.loop !907

bb55:                                             ; preds = %bb47, %bb
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEE9elementAtEj(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #21 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !908 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !829
  %i2 = load i32, ptr %i, align 4, !tbaa !829
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !832
  %i7 = load ptr, ptr %i6, align 8, !tbaa !832
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.1.2472, i32 noundef 206, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !831
  %i13 = load ptr, ptr %i12, align 8, !tbaa !831
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  ret ptr %i15
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv() #27 align 2

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg) unnamed_addr #31 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !909 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i32 0, i64 2), ptr %i, align 8, !tbaa !776
  %i1 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5, !intel-tbaa !911
  %i2 = load ptr, ptr %i1, align 8, !tbaa !911
  %i3 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !915
  %i4 = load ptr, ptr %i3, align 8, !tbaa !915
  %i5 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i2, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !776
  %i7 = call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = getelementptr inbounds ptr, ptr %i6, i64 3
  %i9 = load ptr, ptr %i8, align 8
  %i10 = icmp eq ptr %i9, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i10, label %bb11, label %bb12

bb11:                                             ; preds = %bb
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i2, ptr noundef %i4)
          to label %bb13 unwind label %bb28, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb12:                                             ; preds = %bb
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i2, ptr noundef %i4)
          to label %bb13 unwind label %bb28, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb13:                                             ; preds = %bb12, %bb11
  br label %bb14

bb14:                                             ; preds = %bb13
  %i15 = load ptr, ptr %i1, align 8, !tbaa !911
  %i16 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 2, !intel-tbaa !916
  %i17 = load ptr, ptr %i16, align 8, !tbaa !916
  %i18 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i15, i64 0, i32 0
  %i19 = load ptr, ptr %i18, align 8, !tbaa !776
  %i20 = call i1 @llvm.type.test(ptr %i19, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i20)
  %i21 = getelementptr inbounds ptr, ptr %i19, i64 3
  %i22 = load ptr, ptr %i21, align 8
  %i23 = icmp eq ptr %i22, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i23, label %bb24, label %bb25

bb24:                                             ; preds = %bb14
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i15, ptr noundef %i17)
          to label %bb26 unwind label %bb28, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb25:                                             ; preds = %bb14
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i15, ptr noundef %i17)
          to label %bb26 unwind label %bb28, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb26:                                             ; preds = %bb25, %bb24
  br label %bb27

bb27:                                             ; preds = %bb26
  ret void

bb28:                                             ; preds = %bb25, %bb24, %bb12, %bb11
  %i29 = landingpad { ptr, i32 }
          catch ptr null
  %i30 = extractvalue { ptr, i32 } %i29, 0
  tail call void @__clang_call_terminate(ptr %i30) #38
  unreachable
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2, ptr noundef "intel_dtrans_func_index"="3" %arg3) unnamed_addr #11 align 2 !intel.dtrans.func.type !917 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 0
  store ptr getelementptr inbounds ([5 x ptr], ptr @_ZTVN11xercesc_2_712XMLExceptionE.0, i32 0, i64 2), ptr %i, align 8, !tbaa !776
  %i4 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  store i32 0, ptr %i4, align 8, !tbaa !918
  %i5 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 2, !intel-tbaa !916
  store ptr null, ptr %i5, align 8, !tbaa !916
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 3, !intel-tbaa !919
  store i32 %arg2, ptr %i6, align 8, !tbaa !919
  %i7 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !915
  store ptr null, ptr %i7, align 8, !tbaa !915
  %i8 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5, !intel-tbaa !911
  store ptr %arg3, ptr %i8, align 8, !tbaa !911
  %i9 = icmp eq ptr %arg3, null
  br i1 %i9, label %bb10, label %bb12

bb10:                                             ; preds = %bb
  %i11 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !769
  store ptr %i11, ptr %i8, align 8, !tbaa !911
  br label %bb12

bb12:                                             ; preds = %bb10, %bb
  %i13 = phi ptr [ %i11, %bb10 ], [ %arg3, %bb ]
  %i14 = tail call noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKcPNS_13MemoryManagerE(ptr noundef %arg1, ptr noundef %i13)
  store ptr %i14, ptr %i5, align 8, !tbaa !916
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr nocapture noundef nonnull align 8 dereferenceable(48) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) #0 align 2 !intel.dtrans.func.type !920 {
bb:
  %i = alloca [2048 x i16], align 16, !intel_dtrans_type !921
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 1, !intel-tbaa !918
  store i32 %arg1, ptr %i2, align 8, !tbaa !918
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i) #36
  %i3 = tail call fastcc noundef nonnull align 8 dereferenceable(8) ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv()
  %i4 = getelementptr inbounds [2048 x i16], ptr %i, i64 0, i64 0
  %i5 = getelementptr %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader", ptr %i3, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !776
  %i7 = call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_712XMLMsgLoaderE")
  tail call void @llvm.assume(i1 %i7)
  %i8 = getelementptr inbounds ptr, ptr %i6, i64 2
  %i9 = load ptr, ptr %i8, align 8
  %i10 = call noundef zeroext i1 @_ZN11xercesc_2_714InMemMsgLoader7loadMsgEjPtj(ptr noundef nonnull align 8 dereferenceable(8) %i3, i32 noundef %arg1, ptr noundef nonnull %i4, i32 noundef 2047), !intel_dtrans_type !922, !_Intel.Devirt.Call !779
  %i11 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 5
  %i12 = load ptr, ptr %i11, align 8, !tbaa !911
  br i1 %i10, label %bb37, label %bb13

bb13:                                             ; preds = %bb13, %bb
  %i14 = phi ptr [ %i15, %bb13 ], [ @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, %bb ]
  %i15 = getelementptr inbounds i16, ptr %i14, i64 1
  %i16 = getelementptr [23 x i16], ptr %i15, i64 0, i32 0
  %i17 = load i16, ptr %i16, align 2, !tbaa !787
  %i18 = icmp eq i16 %i17, 0
  br i1 %i18, label %bb19, label %bb13, !llvm.loop !924

bb19:                                             ; preds = %bb13
  %i20 = ptrtoint ptr %i15 to i64
  %i21 = add i64 %i20, add (i64 sub (i64 0, i64 ptrtoint (ptr @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE to i64)), i64 2)
  %i22 = and i64 %i21, 8589934590
  %i23 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i12, i64 0, i32 0
  %i24 = load ptr, ptr %i23, align 8, !tbaa !776
  %i25 = call i1 @llvm.type.test(ptr %i24, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i25)
  %i26 = getelementptr inbounds ptr, ptr %i24, i64 2
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i28, label %bb29, label %bb31

bb29:                                             ; preds = %bb19
  %i30 = call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i22), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb33

bb31:                                             ; preds = %bb19
  %i32 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i22), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb33

bb33:                                             ; preds = %bb31, %bb29
  %i34 = phi ptr [ %i30, %bb29 ], [ %i32, %bb31 ]
  br label %bb35

bb35:                                             ; preds = %bb33
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i34, ptr nonnull align 2 @_ZN11xercesc_2_76XMLUni11fgDefErrMsgE, i64 %i22, i1 false)
  %i36 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !915
  store ptr %i34, ptr %i36, align 8, !tbaa !915
  br label %bb67

bb37:                                             ; preds = %bb
  %i38 = load i16, ptr %i4, align 16, !tbaa !787
  %i39 = icmp eq i16 %i38, 0
  br i1 %i39, label %bb51, label %bb40

bb40:                                             ; preds = %bb40, %bb37
  %i41 = phi ptr [ %i42, %bb40 ], [ %i4, %bb37 ]
  %i42 = getelementptr inbounds i16, ptr %i41, i64 1
  %i43 = load i16, ptr %i42, align 2, !tbaa !787
  %i44 = icmp eq i16 %i43, 0
  br i1 %i44, label %bb45, label %bb40, !llvm.loop !924

bb45:                                             ; preds = %bb40
  %i46 = ptrtoint ptr %i42 to i64
  %i47 = ptrtoint ptr %i to i64
  %i48 = sub i64 %i46, %i47
  %i49 = add i64 %i48, 2
  %i50 = and i64 %i49, 8589934590
  br label %bb51

bb51:                                             ; preds = %bb45, %bb37
  %i52 = phi i64 [ %i50, %bb45 ], [ 2, %bb37 ]
  %i53 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i12, i64 0, i32 0
  %i54 = load ptr, ptr %i53, align 8, !tbaa !776
  %i55 = call i1 @llvm.type.test(ptr %i54, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  call void @llvm.assume(i1 %i55)
  %i56 = getelementptr inbounds ptr, ptr %i54, i64 2
  %i57 = load ptr, ptr %i56, align 8
  %i58 = icmp eq ptr %i57, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i58, label %bb59, label %bb61

bb59:                                             ; preds = %bb51
  %i60 = call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i52), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb63

bb61:                                             ; preds = %bb51
  %i62 = call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i52), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb63

bb63:                                             ; preds = %bb61, %bb59
  %i64 = phi ptr [ %i60, %bb59 ], [ %i62, %bb61 ]
  br label %bb65

bb65:                                             ; preds = %bb63
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %i64, ptr nonnull align 16 %i4, i64 %i52, i1 false)
  %i66 = getelementptr inbounds %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException", ptr %arg, i64 0, i32 4, !intel-tbaa !915
  store ptr %i64, ptr %i66, align 8, !tbaa !915
  br label %bb67

bb67:                                             ; preds = %bb65, %bb35
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i) #36
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden fastcc noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_7L13gGetMsgLoaderEv() unnamed_addr #0 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !925 {
bb:
  %i = alloca %"class._ZTSN11xercesc_2_712XMLMutexLockE.xercesc_2_7::XMLMutexLock", align 8
  %i1 = alloca %"class._ZTSN11xercesc_2_712XMLMutexLockE.xercesc_2_7::XMLMutexLock", align 8
  %i2 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !926
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb4, label %bb37

bb4:                                              ; preds = %bb
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i1) #36
  %i5 = load i1, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br i1 %i5, label %bb23, label %bb6

bb6:                                              ; preds = %bb4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #36
  %i7 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils13fgAtomicMutexE, align 8, !tbaa !928
  call void @_ZN11xercesc_2_712XMLMutexLockC1EPNS_8XMLMutexE(ptr noundef nonnull align 8 dereferenceable(8) %i, ptr noundef %i7)
  %i8 = load i1, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br i1 %i8, label %bb18, label %bb9

bb9:                                              ; preds = %bb6
  %i10 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEm(i64 noundef 8)
          to label %bb11 unwind label %bb14

bb11:                                             ; preds = %bb9
  %i12 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !769
  invoke void @_ZN11xercesc_2_78XMLMutexC1EPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(8) %i10, ptr noundef %i12)
          to label %bb13 unwind label %bb16

bb13:                                             ; preds = %bb11
  store ptr %i10, ptr @_ZN11xercesc_2_7L9sMsgMutexE, align 8, !tbaa !928
  call void @_ZN11xercesc_2_718XMLRegisterCleanup15registerCleanupEPFvvE(ptr noundef nonnull align 8 dereferenceable(24) @_ZN11xercesc_2_7L15msgMutexCleanupE, ptr noundef nonnull @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv)
  store i1 true, ptr @_ZN11xercesc_2_7L23sScannerMutexRegisteredE, align 1
  br label %bb18

bb14:                                             ; preds = %bb9
  %i15 = landingpad { ptr, i32 }
          cleanup
  br label %bb21

bb16:                                             ; preds = %bb11
  %i17 = landingpad { ptr, i32 }
          cleanup
  call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef %i10) #36
  br label %bb21

bb18:                                             ; preds = %bb13, %bb6
  call void @_ZN11xercesc_2_712XMLMutexLockD1Ev(ptr noundef nonnull align 8 dereferenceable(8) %i) #36
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #36
  br label %bb23

bb19:                                             ; preds = %bb32, %bb21
  %i20 = phi { ptr, i32 } [ %i22, %bb21 ], [ %i33, %bb32 ]
  resume { ptr, i32 } %i20

bb21:                                             ; preds = %bb16, %bb14
  %i22 = phi { ptr, i32 } [ %i15, %bb14 ], [ %i17, %bb16 ]
  call void @_ZN11xercesc_2_712XMLMutexLockD1Ev(ptr noundef nonnull align 8 dereferenceable(8) %i) #36
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #36
  br label %bb19

bb23:                                             ; preds = %bb18, %bb4
  %i24 = load ptr, ptr @_ZN11xercesc_2_7L9sMsgMutexE, align 8, !tbaa !928
  call void @_ZN11xercesc_2_712XMLMutexLockC1EPNS_8XMLMutexE(ptr noundef nonnull align 8 dereferenceable(8) %i1, ptr noundef nonnull %i24)
  %i25 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !926
  %i26 = icmp eq ptr %i25, null
  br i1 %i26, label %bb27, label %bb35

bb27:                                             ; preds = %bb23
  %i28 = invoke noundef ptr @_ZN11xercesc_2_716XMLPlatformUtils10loadMsgSetEPKt(ptr noundef nonnull @_ZN11xercesc_2_76XMLUni14fgExceptDomainE)
          to label %bb29 unwind label %bb32

bb29:                                             ; preds = %bb27
  store ptr %i28, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !926
  %i30 = icmp eq ptr %i28, null
  br i1 %i30, label %bb31, label %bb34

bb31:                                             ; preds = %bb29
  invoke void @_ZN11xercesc_2_716XMLPlatformUtils5panicENS_12PanicHandler12PanicReasonsE(i32 noundef 4)
          to label %bb34 unwind label %bb32

bb32:                                             ; preds = %bb31, %bb27
  %i33 = landingpad { ptr, i32 }
          cleanup
  call void @_ZN11xercesc_2_712XMLMutexLockD1Ev(ptr noundef nonnull align 8 dereferenceable(8) %i1) #36
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i1) #36
  br label %bb19

bb34:                                             ; preds = %bb31, %bb29
  call void @_ZN11xercesc_2_718XMLRegisterCleanup15registerCleanupEPFvvE(ptr noundef nonnull align 8 dereferenceable(24) @_ZN11xercesc_2_7L16msgLoaderCleanupE, ptr noundef nonnull @_ZN11xercesc_2_712XMLException15reinitMsgLoaderEv)
  br label %bb35

bb35:                                             ; preds = %bb34, %bb23
  call void @_ZN11xercesc_2_712XMLMutexLockD1Ev(ptr noundef nonnull align 8 dereferenceable(8) %i1) #36
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i1) #36
  %i36 = load ptr, ptr @_ZN11xercesc_2_7L10sMsgLoaderE, align 8, !tbaa !926
  br label %bb37

bb37:                                             ; preds = %bb35, %bb
  %i38 = phi ptr [ %i36, %bb35 ], [ %i2, %bb ]
  ret ptr %i38
}

; Function Attrs: mustprogress nounwind uwtable
declare hidden void @_ZN11xercesc_2_712XMLException14reinitMsgMutexEv() #27 align 2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable
define hidden void @_ZN11xercesc_2_718XMLRegisterCleanup15registerCleanupEPFvvE(ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) #32 align 2 !intel.dtrans.func.type !930 {
bb:
  %i = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %arg, i64 0, i32 0, !intel-tbaa !931
  store ptr %arg1, ptr %i, align 8, !tbaa !931
  %i2 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %arg, i64 0, i32 1, !intel-tbaa !935
  %i3 = load ptr, ptr %i2, align 8, !tbaa !935
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb5, label %bb14

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %arg, i64 0, i32 2, !intel-tbaa !936
  %i7 = load ptr, ptr %i6, align 8, !tbaa !936
  %i8 = icmp eq ptr %i7, null
  br i1 %i8, label %bb9, label %bb14

bb9:                                              ; preds = %bb5
  %i10 = load ptr, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !937
  store ptr %i10, ptr %i2, align 8, !tbaa !935
  store ptr %arg, ptr @_ZN11xercesc_2_715gXMLCleanupListE, align 8, !tbaa !937
  %i11 = icmp eq ptr %i10, null
  br i1 %i11, label %bb14, label %bb12

bb12:                                             ; preds = %bb9
  %i13 = getelementptr inbounds %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup", ptr %i10, i64 0, i32 2, !intel-tbaa !936
  store ptr %arg, ptr %i13, align 8, !tbaa !936
  br label %bb14

bb14:                                             ; preds = %bb12, %bb9, %bb5, %bb
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_79XMLString9replicateEPKcPNS_13MemoryManagerE(ptr noundef readonly "intel_dtrans_func_index"="2" %arg, ptr noundef "intel_dtrans_func_index"="3" %arg1) #0 align 2 !intel.dtrans.func.type !938 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb19, label %bb2

bb2:                                              ; preds = %bb
  %i3 = tail call i64 @strlen(ptr noundef nonnull dereferenceable(1) %arg) #41
  %i4 = add i64 %i3, 1
  %i5 = and i64 %i4, 4294967295
  %i6 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg1, i64 0, i32 0
  %i7 = load ptr, ptr %i6, align 8, !tbaa !776
  %i8 = call i1 @llvm.type.test(ptr %i7, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i8)
  %i9 = getelementptr inbounds ptr, ptr %i7, i64 2
  %i10 = load ptr, ptr %i9, align 8
  %i11 = icmp eq ptr %i10, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb2
  %i13 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i5), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb16

bb14:                                             ; preds = %bb2
  %i15 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i5), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb16

bb16:                                             ; preds = %bb14, %bb12
  %i17 = phi ptr [ %i13, %bb12 ], [ %i15, %bb14 ]
  br label %bb18

bb18:                                             ; preds = %bb16
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 1 %i17, ptr nonnull align 1 %arg, i64 %i5, i1 false)
  br label %bb19

bb19:                                             ; preds = %bb18, %bb
  %i20 = phi ptr [ %i17, %bb18 ], [ null, %bb ]
  ret ptr %i20
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEm(i64 noundef %arg) #0 align 2 !intel.dtrans.func.type !939 {
bb:
  %i = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !769
  %i1 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i, i64 0, i32 0
  %i2 = load ptr, ptr %i1, align 8, !tbaa !776
  %i3 = call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = add nuw nsw i64 %arg, 8
  %i5 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i6 = load ptr, ptr %i5, align 8
  %i7 = icmp eq ptr %i6, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i7, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i, i64 noundef %i4), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb12

bb10:                                             ; preds = %bb
  %i11 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i, i64 noundef %i4), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb12

bb12:                                             ; preds = %bb10, %bb8
  %i13 = phi ptr [ %i9, %bb8 ], [ %i11, %bb10 ]
  br label %bb14

bb14:                                             ; preds = %bb12
  %i15 = load ptr, ptr @_ZN11xercesc_2_716XMLPlatformUtils15fgMemoryManagerE, align 8, !tbaa !769
  store ptr %i15, ptr %i13, align 8, !tbaa !769
  %i16 = getelementptr inbounds i8, ptr %i13, i64 8, !intel-tbaa !940
  ret ptr %i16
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1) #0 align 2 !intel.dtrans.func.type !941 {
bb:
  %i = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg1, i64 0, i32 0
  %i2 = load ptr, ptr %i, align 8, !tbaa !776
  %i3 = call i1 @llvm.type.test(ptr %i2, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i3)
  %i4 = add nuw nsw i64 %arg, 8
  %i5 = getelementptr inbounds ptr, ptr %i2, i64 2
  %i6 = load ptr, ptr %i5, align 8
  %i7 = icmp eq ptr %i6, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i7, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i4), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb12

bb10:                                             ; preds = %bb
  %i11 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg1, i64 noundef %i4), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb12

bb12:                                             ; preds = %bb10, %bb8
  %i13 = phi ptr [ %i9, %bb8 ], [ %i11, %bb10 ]
  br label %bb14

bb14:                                             ; preds = %bb12
  store ptr %arg1, ptr %i13, align 8, !tbaa !769
  %i15 = getelementptr inbounds i8, ptr %i13, i64 8, !intel-tbaa !940
  ret ptr %i15
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef "intel_dtrans_func_index"="1" %arg) #27 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !942 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb12, label %bb1

bb1:                                              ; preds = %bb
  %i2 = getelementptr inbounds i8, ptr %arg, i64 -8, !intel-tbaa !940
  %i3 = load ptr, ptr %i2, align 8, !tbaa !769
  %i4 = load ptr, ptr %i3, align 8, !tbaa !776
  %i5 = call i1 @llvm.type.test(ptr %i4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i5)
  %i6 = getelementptr inbounds ptr, ptr %i4, i64 3
  %i7 = load ptr, ptr %i6, align 8
  %i8 = icmp eq ptr %i7, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i8, label %bb9, label %bb10

bb9:                                              ; preds = %bb1
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i3, ptr noundef nonnull %i2)
          to label %bb11 unwind label %bb13, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb10:                                             ; preds = %bb1
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i3, ptr noundef nonnull %i2)
          to label %bb11 unwind label %bb13, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb11:                                             ; preds = %bb10, %bb9
  br label %bb12

bb12:                                             ; preds = %bb11, %bb
  ret void

bb13:                                             ; preds = %bb10, %bb9
  %i14 = landingpad { ptr, i32 }
          catch ptr null
  %i15 = extractvalue { ptr, i32 } %i14, 0
  tail call void @__clang_call_terminate(ptr %i15) #38
  unreachable
}

; Function Attrs: mustprogress nounwind uwtable
define hidden void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %arg1) #27 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !943 {
bb:
  %i = icmp eq ptr %arg, null
  br i1 %i, label %bb13, label %bb2

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds i8, ptr %arg, i64 -8, !intel-tbaa !940
  %i4 = load ptr, ptr %i3, align 8, !tbaa !769
  %i5 = load ptr, ptr %i4, align 8, !tbaa !776
  %i6 = call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  %i7 = getelementptr inbounds ptr, ptr %i5, i64 3
  %i8 = load ptr, ptr %i7, align 8
  %i9 = icmp eq ptr %i8, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i9, label %bb10, label %bb11

bb10:                                             ; preds = %bb2
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i4, ptr noundef nonnull %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb11:                                             ; preds = %bb2
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i4, ptr noundef nonnull %i3)
          to label %bb12 unwind label %bb14, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb12:                                             ; preds = %bb11, %bb10
  br label %bb13

bb13:                                             ; preds = %bb12, %bb
  ret void

bb14:                                             ; preds = %bb11, %bb10
  %i15 = landingpad { ptr, i32 }
          catch ptr null
  %i16 = extractvalue { ptr, i32 } %i15, 0
  tail call void @__clang_call_terminate(ptr %i16) #38
  unreachable
}

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !944 dso_local void @_ZNSt9bad_allocD1Ev(ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1") unnamed_addr #33

; Function Attrs: nofree noreturn uwtable
define hidden noalias noundef nonnull "intel_dtrans_func_index"="1" ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1) unnamed_addr #34 align 2 !intel.dtrans.func.type !946 !_Intel.Devirt.Target !868 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #36
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !776
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #37
  unreachable
}

; Function Attrs: nofree noreturn uwtable
define hidden void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr nocapture noundef nonnull readnone align 8 dereferenceable(8) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readnone "intel_dtrans_func_index"="2" %arg1) unnamed_addr #34 align 2 !intel.dtrans.func.type !948 !_Intel.Devirt.Target !868 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 8) #36
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2), ptr %i, align 8, !tbaa !776
  tail call void @__cxa_throw(ptr nonnull %i, ptr nonnull @_ZTISt9bad_alloc, ptr nonnull @_ZNSt9bad_allocD1Ev) #37
  unreachable
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #35

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define hidden noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv.5823(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg) #22 align 2 !intel.dtrans.func.type !949 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i1 = load i32, ptr %i, align 4, !tbaa !950
  ret i32 %i1
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE.5825(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, ptr noundef "intel_dtrans_func_index"="2" %arg2, i1 noundef zeroext %arg3) unnamed_addr #18 align 2 !intel.dtrans.func.type !953 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !954
  store i8 0, ptr %i, align 8, !tbaa !954
  %i4 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  store i32 0, ptr %i4, align 4, !tbaa !950
  %i5 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !955
  store i32 %arg1, ptr %i5, align 8, !tbaa !955
  %i6 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  store ptr null, ptr %i6, align 8, !tbaa !956
  %i7 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  store ptr %arg2, ptr %i7, align 8, !tbaa !957
  %i8 = zext i32 %arg1 to i64
  %i9 = shl nuw nsw i64 %i8, 3
  %i10 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %arg2, i64 0, i32 0
  %i11 = load ptr, ptr %i10, align 8, !tbaa !776
  %i12 = call i1 @llvm.type.test(ptr %i11, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i12)
  %i13 = getelementptr inbounds ptr, ptr %i11, i64 2
  %i14 = load ptr, ptr %i13, align 8
  %i15 = icmp eq ptr %i14, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i15, label %bb16, label %bb18

bb16:                                             ; preds = %bb
  %i17 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef %i9), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb20

bb18:                                             ; preds = %bb
  %i19 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %arg2, i64 noundef %i9), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb20

bb20:                                             ; preds = %bb18, %bb16
  %i21 = phi ptr [ %i17, %bb16 ], [ %i19, %bb18 ]
  br label %bb22

bb22:                                             ; preds = %bb20
  store ptr %i21, ptr %i6, align 8, !tbaa !956
  %i23 = icmp eq i32 %arg1, 0
  br i1 %i23, label %bb25, label %bb24

bb24:                                             ; preds = %bb22
  tail call void @llvm.memset.p0.i64(ptr align 8 %i21, i8 0, i64 %i9, i1 false), !tbaa !958
  br label %bb25

bb25:                                             ; preds = %bb24, %bb22
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden noundef "intel_dtrans_func_index"="1" ptr @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj.5826(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="2" %arg, i32 noundef %arg1) #0 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !959 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i2 = load i32, ptr %i, align 4, !tbaa !950
  %i3 = icmp ugt i32 %i2, %arg1
  br i1 %i3, label %bb11, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i6 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i7 = load ptr, ptr %i6, align 8, !tbaa !957
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i5, ptr noundef nonnull @.str.3, i32 noundef 249, i32 noundef 116, ptr noundef %i7)
          to label %bb8 unwind label %bb9

bb8:                                              ; preds = %bb4
  tail call void @__cxa_throw(ptr nonnull %i5, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb9:                                              ; preds = %bb4
  %i10 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i5) #36
  resume { ptr, i32 } %i10

bb11:                                             ; preds = %bb
  %i12 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  %i13 = load ptr, ptr %i12, align 8, !tbaa !956
  %i14 = zext i32 %arg1 to i64
  %i15 = getelementptr inbounds ptr, ptr %i13, i64 %i14
  %i16 = load ptr, ptr %i15, align 8, !tbaa !958
  ret ptr %i16
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt.5827(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %arg1) #21 align 2 !intel.dtrans.func.type !960 {
bb:
  tail call void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.5828(ptr noundef nonnull align 8 dereferenceable(40) %arg, i32 noundef 1)
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  %i2 = load ptr, ptr %i, align 8, !tbaa !956
  %i3 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i4 = load i32, ptr %i3, align 4, !tbaa !950
  %i5 = zext i32 %i4 to i64
  %i6 = getelementptr inbounds ptr, ptr %i2, i64 %i5
  %i7 = load ptr, ptr %arg1, align 8
  store ptr %i7, ptr %i6, align 8, !tbaa !958
  %i8 = add i32 %i4, 1
  store i32 %i8, ptr %i3, align 4, !tbaa !950
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj.5828(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1) #21 align 2 !intel.dtrans.func.type !961 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i2 = load i32, ptr %i, align 4, !tbaa !950
  %i3 = add i32 %i2, 1
  %i4 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !955
  %i5 = load i32, ptr %i4, align 8, !tbaa !955
  %i6 = icmp ugt i32 %i3, %i5
  br i1 %i6, label %bb7, label %bb65

bb7:                                              ; preds = %bb
  %i8 = lshr i32 %i5, 1
  %i9 = add i32 %i8, %i5
  %i10 = icmp ult i32 %i3, %i9
  %i11 = select i1 %i10, i32 %i9, i32 %i3
  %i12 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i13 = load ptr, ptr %i12, align 8, !tbaa !957
  %i14 = zext i32 %i11 to i64
  %i15 = shl nuw nsw i64 %i14, 3
  %i16 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i13, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 8, !tbaa !776
  %i18 = call i1 @llvm.type.test(ptr %i17, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i18)
  %i19 = getelementptr inbounds ptr, ptr %i17, i64 2
  %i20 = load ptr, ptr %i19, align 8
  %i21 = icmp eq ptr %i20, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i21, label %bb22, label %bb24

bb22:                                             ; preds = %bb7
  %i23 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb24:                                             ; preds = %bb7
  %i25 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i13, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb26:                                             ; preds = %bb24, %bb22
  %i27 = phi ptr [ %i23, %bb22 ], [ %i25, %bb24 ]
  br label %bb28

bb28:                                             ; preds = %bb26
  %i29 = load i32, ptr %i, align 4, !tbaa !950
  %i30 = icmp eq i32 %i29, 0
  %i31 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3
  %i32 = load ptr, ptr %i31, align 8, !tbaa !956
  br i1 %i30, label %bb35, label %bb33

bb33:                                             ; preds = %bb28
  %i34 = zext i32 %i29 to i64
  br label %bb46

bb35:                                             ; preds = %bb46, %bb28
  %i36 = icmp ult i32 %i29, %i11
  br i1 %i36, label %bb37, label %bb53

bb37:                                             ; preds = %bb35
  %i38 = zext i32 %i29 to i64
  %i39 = shl nuw nsw i64 %i38, 3
  %i40 = getelementptr i8, ptr %i27, i64 %i39
  %i41 = xor i32 %i29, -1
  %i42 = add i32 %i11, %i41
  %i43 = zext i32 %i42 to i64
  %i44 = add nuw nsw i64 %i43, 1
  %i45 = shl nuw nsw i64 %i44, 3
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %i40, i8 0, i64 %i45, i1 false), !tbaa !958
  br label %bb53

bb46:                                             ; preds = %bb46, %bb33
  %i47 = phi i64 [ 0, %bb33 ], [ %i51, %bb46 ]
  %i48 = getelementptr inbounds ptr, ptr %i32, i64 %i47
  %i49 = load ptr, ptr %i48, align 8, !tbaa !958
  %i50 = getelementptr inbounds ptr, ptr %i27, i64 %i47
  store ptr %i49, ptr %i50, align 8, !tbaa !958
  %i51 = add nuw nsw i64 %i47, 1
  %i52 = icmp eq i64 %i51, %i34
  br i1 %i52, label %bb35, label %bb46, !llvm.loop !962

bb53:                                             ; preds = %bb37, %bb35
  %i54 = load ptr, ptr %i12, align 8, !tbaa !957
  %i55 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i54, i64 0, i32 0
  %i56 = load ptr, ptr %i55, align 8, !tbaa !776
  %i57 = call i1 @llvm.type.test(ptr %i56, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i57)
  %i58 = getelementptr inbounds ptr, ptr %i56, i64 3
  %i59 = load ptr, ptr %i58, align 8
  %i60 = icmp eq ptr %i59, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i60, label %bb61, label %bb62

bb61:                                             ; preds = %bb53
  tail call void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i54, ptr noundef %i32), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb63

bb62:                                             ; preds = %bb53
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i54, ptr noundef %i32), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb63

bb63:                                             ; preds = %bb62, %bb61
  br label %bb64

bb64:                                             ; preds = %bb63
  store ptr %i27, ptr %i31, align 8, !tbaa !956
  store i32 %i11, ptr %i4, align 8, !tbaa !955
  br label %bb65

bb65:                                             ; preds = %bb64, %bb
  ret void
}

; Function Attrs: nounwind uwtable
define hidden void @_ZN11xercesc_2_716RefArrayVectorOfItED2Ev.5830(ptr nocapture noundef nonnull align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg) unnamed_addr #20 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !963 {
bb:
  %i = icmp eq i8 1, 0
  br i1 %i, label %bb28, label %bb1

bb1:                                              ; preds = %bb
  %i2 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i3 = load i32, ptr %i2, align 4, !tbaa !950
  %i4 = icmp eq i32 %i3, 0
  br i1 %i4, label %bb28, label %bb5

bb5:                                              ; preds = %bb1
  %i6 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i7 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  br label %bb8

bb8:                                              ; preds = %bb23, %bb5
  %i9 = phi i64 [ 0, %bb5 ], [ %i24, %bb23 ]
  %i10 = load ptr, ptr %i6, align 8, !tbaa !957
  %i11 = load ptr, ptr %i7, align 8, !tbaa !956
  %i12 = getelementptr inbounds ptr, ptr %i11, i64 %i9
  %i13 = load ptr, ptr %i12, align 8, !tbaa !958
  %i14 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i10, i64 0, i32 0
  %i15 = load ptr, ptr %i14, align 8, !tbaa !776
  %i16 = call i1 @llvm.type.test(ptr %i15, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i16)
  %i17 = getelementptr inbounds ptr, ptr %i15, i64 3
  %i18 = load ptr, ptr %i17, align 8
  %i19 = icmp eq ptr %i18, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i19, label %bb20, label %bb21

bb20:                                             ; preds = %bb8
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i10, ptr noundef %i13)
          to label %bb22 unwind label %bb43, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb21:                                             ; preds = %bb8
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i10, ptr noundef %i13)
          to label %bb22 unwind label %bb43, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb22:                                             ; preds = %bb21, %bb20
  br label %bb23

bb23:                                             ; preds = %bb22
  %i24 = add nuw nsw i64 %i9, 1
  %i25 = load i32, ptr %i2, align 4, !tbaa !950
  %i26 = zext i32 %i25 to i64
  %i27 = icmp ult i64 %i24, %i26
  br i1 %i27, label %bb8, label %bb28, !llvm.loop !964

bb28:                                             ; preds = %bb23, %bb1, %bb
  %i29 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i30 = load ptr, ptr %i29, align 8, !tbaa !957
  %i31 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  %i32 = load ptr, ptr %i31, align 8, !tbaa !956
  %i33 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i30, i64 0, i32 0
  %i34 = load ptr, ptr %i33, align 8, !tbaa !776
  %i35 = call i1 @llvm.type.test(ptr %i34, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i35)
  %i36 = getelementptr inbounds ptr, ptr %i34, i64 3
  %i37 = load ptr, ptr %i36, align 8
  %i38 = icmp eq ptr %i37, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i38, label %bb39, label %bb40

bb39:                                             ; preds = %bb28
  invoke void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i30, ptr noundef %i32)
          to label %bb41 unwind label %bb45, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb40:                                             ; preds = %bb28
  invoke void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i30, ptr noundef %i32)
          to label %bb41 unwind label %bb45, !intel_dtrans_type !838, !_Intel.Devirt.Call !779

bb41:                                             ; preds = %bb40, %bb39
  br label %bb42

bb42:                                             ; preds = %bb41
  ret void

bb43:                                             ; preds = %bb21, %bb20
  %i44 = landingpad { ptr, i32 }
          catch ptr null
  br label %bb47

bb45:                                             ; preds = %bb40, %bb39
  %i46 = landingpad { ptr, i32 }
          catch ptr null
  br label %bb47

bb47:                                             ; preds = %bb45, %bb43
  %i48 = phi { ptr, i32 } [ %i44, %bb43 ], [ %i46, %bb45 ]
  %i49 = extractvalue { ptr, i32 } %i48, 0
  tail call void @__clang_call_terminate(ptr %i49) #38
  unreachable
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) unnamed_addr #21 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !965 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  %i3 = load i32, ptr %i, align 4, !tbaa !950
  %i4 = icmp ugt i32 %i3, %arg2
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb
  %i6 = tail call ptr @__cxa_allocate_exception(i64 48) #36
  %i7 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i8 = load ptr, ptr %i7, align 8, !tbaa !957
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef nonnull align 8 dereferenceable(48) %i6, ptr noundef nonnull @.str.2.1138, i32 noundef 52, i32 noundef 116, ptr noundef %i8)
          to label %bb9 unwind label %bb10

bb9:                                              ; preds = %bb5
  tail call void @__cxa_throw(ptr nonnull %i6, ptr nonnull @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr nonnull @_ZN11xercesc_2_712XMLExceptionD2Ev) #37
  unreachable

bb10:                                             ; preds = %bb5
  %i11 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %i6) #36
  resume { ptr, i32 } %i11

bb12:                                             ; preds = %bb
  %i13 = icmp eq i8 1, 0
  br i1 %i13, label %bb14, label %bb16

bb14:                                             ; preds = %bb12
  %i15 = zext i32 %arg2 to i64
  br label %bb34

bb16:                                             ; preds = %bb12
  %i17 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  %i18 = load ptr, ptr %i17, align 8, !tbaa !957
  %i19 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  %i20 = load ptr, ptr %i19, align 8, !tbaa !956
  %i21 = zext i32 %arg2 to i64
  %i22 = getelementptr inbounds ptr, ptr %i20, i64 %i21
  %i23 = load ptr, ptr %i22, align 8, !tbaa !958
  %i24 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i18, i64 0, i32 0
  %i25 = load ptr, ptr %i24, align 8, !tbaa !776
  %i26 = call i1 @llvm.type.test(ptr %i25, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i26)
  %i27 = getelementptr inbounds ptr, ptr %i25, i64 3
  %i28 = load ptr, ptr %i27, align 8
  %i29 = icmp eq ptr %i28, @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv
  br i1 %i29, label %bb30, label %bb31

bb30:                                             ; preds = %bb16
  tail call void @_ZN11xercesc_2_717MemoryManagerImpl10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i18, ptr noundef %i23), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb32

bb31:                                             ; preds = %bb16
  tail call void @_ZN11xalanc_1_1023XalanDummyMemoryManager10deallocateEPv(ptr noundef nonnull align 8 dereferenceable(8) %i18, ptr noundef %i23), !intel_dtrans_type !838, !_Intel.Devirt.Call !779
  br label %bb32

bb32:                                             ; preds = %bb31, %bb30
  br label %bb33

bb33:                                             ; preds = %bb32
  br label %bb34

bb34:                                             ; preds = %bb33, %bb14
  %i35 = phi i64 [ %i15, %bb14 ], [ %i21, %bb33 ]
  %i36 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  %i37 = load ptr, ptr %i36, align 8, !tbaa !956
  %i38 = getelementptr inbounds ptr, ptr %i37, i64 %i35
  store ptr %arg1, ptr %i38, align 8, !tbaa !958
  ret void
}

; Function Attrs: mustprogress uwtable
define hidden void @_ZN11xercesc_2_716RefArrayVectorOfItE12setElementAtEPtj.5831.5832(ptr nocapture noundef nonnull readonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr noundef "intel_dtrans_func_index"="2" %arg1, i32 noundef %arg2) unnamed_addr #21 align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !966 !dtrans-soatoaosprepare !361 {
bb:
  %i = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3
  %i3 = load ptr, ptr %i, align 8
  %i4 = zext i32 %arg2 to i64
  %i5 = getelementptr inbounds ptr, ptr %i3, i64 %i4
  store ptr %arg1, ptr %i5, align 8
  ret void
}

; Function Attrs: uwtable
define hidden void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE.5825.5833.5834(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, ptr nocapture readonly "intel_dtrans_func_index"="2" %arg1) unnamed_addr #18 align 2 !intel.dtrans.func.type !967 !dtrans-soatoaosprepare !361 {
bb:
  %i = getelementptr %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg1, i64 0, i32 0
  %i2 = load i8, ptr %i, align 1
  %i3 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 0, !intel-tbaa !954
  store i8 %i2, ptr %i3, align 8, !tbaa !954
  %i4 = getelementptr %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg1, i64 0, i32 1
  %i5 = load i32, ptr %i4, align 4
  %i6 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 1, !intel-tbaa !950
  store i32 %i5, ptr %i6, align 4, !tbaa !950
  %i7 = getelementptr %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg1, i64 0, i32 2
  %i8 = load i32, ptr %i7, align 4
  %i9 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 2, !intel-tbaa !955
  store i32 %i8, ptr %i9, align 8, !tbaa !955
  %i10 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 3, !intel-tbaa !956
  store ptr null, ptr %i10, align 8, !tbaa !956
  %i11 = getelementptr %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg1, i64 0, i32 4
  %i12 = load ptr, ptr %i11, align 8
  %i13 = getelementptr inbounds %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf", ptr %arg, i64 0, i32 4, !intel-tbaa !957
  store ptr %i12, ptr %i13, align 8, !tbaa !957
  %i14 = zext i32 %i8 to i64
  %i15 = shl nuw nsw i64 %i14, 3
  %i16 = getelementptr %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager", ptr %i12, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 8, !tbaa !776
  %i18 = call i1 @llvm.type.test(ptr %i17, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i18)
  %i19 = getelementptr inbounds ptr, ptr %i17, i64 2
  %i20 = load ptr, ptr %i19, align 8
  %i21 = icmp eq ptr %i20, @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm
  br i1 %i21, label %bb22, label %bb24

bb22:                                             ; preds = %bb
  %i23 = tail call noundef ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb24:                                             ; preds = %bb
  %i25 = tail call noundef ptr @_ZN11xalanc_1_1023XalanDummyMemoryManager8allocateEm(ptr noundef nonnull align 8 dereferenceable(8) %i12, i64 noundef %i15), !intel_dtrans_type !791, !_Intel.Devirt.Call !779
  br label %bb26

bb26:                                             ; preds = %bb24, %bb22
  %i27 = phi ptr [ %i23, %bb22 ], [ %i25, %bb24 ]
  br label %bb28

bb28:                                             ; preds = %bb26
  store ptr %i27, ptr %i10, align 8, !tbaa !956
  %i29 = icmp eq i32 %i8, 0
  br i1 %i29, label %bb31, label %bb30

bb30:                                             ; preds = %bb28
  tail call void @llvm.memset.p0.i64(ptr align 8 %i27, i8 0, i64 %i15, i1 false), !tbaa !958
  br label %bb31

bb31:                                             ; preds = %bb30, %bb28
  ret void
}

declare !intel.dtrans.func.type !968 void @_ZN11xercesc_2_714InMemMsgLoaderC1EPKt(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2")

declare !intel.dtrans.func.type !969 void @_ZN11xercesc_2_78XMLMutexC1EPNS_13MemoryManagerE(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2")

declare !intel.dtrans.func.type !970 void @_ZN11xercesc_2_712XMLMutexLockC1EPNS_8XMLMutexE(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2")

declare !intel.dtrans.func.type !972 void @_ZN11xercesc_2_712XMLMutexLockD1Ev(ptr "intel_dtrans_func_index"="1")

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind memory(none) }
attributes #2 = { nofree }
attributes #3 = { nofree noinline noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nofree noreturn nounwind }
attributes #5 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #6 = { nofree noreturn }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #11 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #12 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { mustprogress nofree noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #18 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #19 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #20 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #21 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #22 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #23 = { mustprogress nofree nounwind willreturn memory(argmem: read) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #24 = { mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #25 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #26 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #27 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #28 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #29 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #30 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #31 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #32 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #33 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #34 = { nofree noreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #35 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #36 = { nounwind }
attributes #37 = { noreturn }
attributes #38 = { noreturn nounwind }
attributes #39 = { cold }
attributes #40 = { allocsize(0) }
attributes #41 = { nounwind willreturn memory(read) }

!intel.dtrans.types = !{!355, !359, !362, !363, !365, !366, !373, !374, !377, !378, !379, !383, !384, !386, !387, !388, !391, !392, !395, !398, !399, !400, !403, !404, !405, !406, !407, !408, !429, !430, !432, !434, !437, !443, !447, !449, !451, !456, !457, !458, !462, !463, !465, !466, !477, !480, !482, !490, !491, !494, !496, !497, !498, !505, !508, !509, !514, !516, !521, !523, !525, !528, !531, !532, !535, !536, !537, !538, !540, !542, !544, !546, !547, !549, !551, !554, !556, !558, !560, !563, !565, !568, !569, !570, !574, !575, !579, !581, !582, !583, !596, !598, !600, !603, !605, !607, !609, !611, !614, !615, !618, !620, !622, !624, !625, !626, !628, !630, !634, !638, !639, !641, !643, !645, !647, !648, !650, !653, !655, !657, !659, !660, !662, !664, !667, !669, !671, !673, !675, !677, !679, !681, !683, !686, !688, !691, !693, !696, !697, !699, !700, !702, !703, !705, !707, !709, !711, !713, !715, !717, !718, !720, !724, !726, !728, !730, !732, !734, !736, !738, !740, !742, !745, !747, !750, !751, !753, !754, !756, !757, !758, !759, !760}
!llvm.ident = !{!761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761, !761}
!llvm.module.flags = !{!762, !763, !764, !765, !766, !767}

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
!338 = !{i8 0, i32 1}
!339 = !{i64 16, !"_ZTSSt9bad_alloc"}
!340 = !{i64 32, !"_ZTSMSt9bad_allocKFPKcvE.virtual"}
!341 = !{i64 16, !"_ZTSSt9exception"}
!342 = !{i64 32, !"_ZTSMSt9exceptionKFPKcvE.virtual"}
!343 = !{!"L", i32 1, !344}
!344 = !{!"A", i32 5, !338}
!345 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}
!346 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!347 = !{!"L", i32 3, !338, !338, !338}
!348 = !{i32 16, !"_ZTSN11xercesc_2_712XMLExceptionE"}
!349 = !{i32 32, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPKtvE.virtual"}
!350 = !{i32 40, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPS0_vE.virtual"}
!351 = !{i32 16, !"_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE"}
!352 = !{i32 32, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPKtvE.virtual"}
!353 = !{i32 40, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!354 = !{!"A", i32 6, !338}
!355 = !{!"S", %"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 1, !356}
!356 = !{!357, i32 2}
!357 = !{!"F", i1 true, i32 0, !358}
!358 = !{i32 0, i32 0}
!359 = !{!"S", %"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 2, !360, !361}
!360 = !{%"class._ZTSN11xercesc_2_712XMLMsgLoaderE.xercesc_2_7::XMLMsgLoader" zeroinitializer, i32 0}
!361 = !{i16 0, i32 1}
!362 = !{!"S", %"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !356}
!363 = !{!"S", %"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1, !364}
!364 = !{i8 0, i32 0}
!365 = !{!"S", %"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 1, !356}
!366 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !358, !338, !338, !338, !338, !338, !338, !338, !338, !338, !338, !338, !367, !345, !358, !358, !368, !4, !364, !369, !338, !368, !370, !371, !345, !338, !368, !358, !372}
!367 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}
!368 = !{i64 0, i32 0}
!369 = !{!"A", i32 1, !364}
!370 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}
!371 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}
!372 = !{!"A", i32 20, !364}
!373 = !{!"S", %"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 6, !356, !358, !338, !358, !361, !346}
!374 = !{!"S", %"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 16, !375, !376, !376, !376, !376, !376, !376, !376, !376, !376, !376, !376, !376, !376, !376, !346}
!375 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!376 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 1}
!377 = !{!"S", %"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 3, !356, !4, !346}
!378 = !{!"S", %"class._ZTSN11xercesc_2_78XMLMutexE.xercesc_2_7::XMLMutex" zeroinitializer, i32 1, !338}
!379 = !{!"S", %"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 10, !380, !364, !364, !358, !358, !358, !381, !381, !382, !346}
!380 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 0}
!381 = !{i32 0, i32 1}
!382 = !{%"class._ZTSN11xercesc_2_710RangeTokenE.xercesc_2_7::RangeToken" zeroinitializer, i32 1}
!383 = !{!"S", %"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 1, !356}
!384 = !{!"S", %"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 2, !385, !346}
!385 = !{%"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 0}
!386 = !{!"S", %"class._ZTSN11xercesc_2_713XSerializableE.xercesc_2_7::XSerializable" zeroinitializer, i32 1, !356}
!387 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 10, !385, !358, !358, !361, !361, !361, !361, !361, !361, !346}
!388 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 6, !356, !358, !389, !390, !346, !358}
!389 = !{!"A", i32 4, !364}
!390 = !{%"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 1}
!391 = !{!"S", %"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1, !356}
!392 = !{!"S", %"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 6, !385, !346, !393, !394, !358, !358}
!393 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2}
!394 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!395 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 7, !385, !346, !396, !358, !358, !364, !397}
!396 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 1}
!397 = !{!"A", i32 7, !364}
!398 = !{!"S", %"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 6, !385, !346, !396, !358, !358, !364}
!399 = !{!"S", %"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 9, !385, !358, !358, !358, !358, !361, !361, !361, !346}
!400 = !{!"S", %"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 11, !385, !346, !396, !401, !402, !402, !358, !364, !364, !358, !358}
!401 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl" zeroinitializer, i32 1}
!402 = !{%"class._ZTSN11xercesc_2_715ContentSpecNodeE.xercesc_2_7::ContentSpecNode" zeroinitializer, i32 1}
!403 = !{!"S", %"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 10, !385, !358, !358, !358, !364, !364, !358, !361, !361, !346}
!404 = !{!"S", %"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1, !356}
!405 = !{!"S", %"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1, !356}
!406 = !{!"S", %"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1, !356}
!407 = !{!"S", %"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1, !356}
!408 = !{!"S", %"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 72, !409, !368, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !364, !358, !358, !358, !358, !358, !358, !358, !358, !410, !358, !358, !358, !358, !358, !411, !412, !413, !414, !415, !416, !417, !418, !419, !364, !420, !421, !358, !422, !346, !423, !423, !424, !361, !361, !361, !425, !358, !346, !426, !427, !427, !427, !427, !427, !427, !427, !428}
!409 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 0}
!410 = !{i32 0, i32 2}
!411 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!412 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!413 = !{%"class._ZTSN11xercesc_2_718XMLDocumentHandlerE.xercesc_2_7::XMLDocumentHandler" zeroinitializer, i32 1}
!414 = !{%"class._ZTSN11xercesc_2_714DocTypeHandlerE.xercesc_2_7::DocTypeHandler" zeroinitializer, i32 1}
!415 = !{%"class._ZTSN11xercesc_2_716XMLEntityHandlerE.xercesc_2_7::XMLEntityHandler" zeroinitializer, i32 1}
!416 = !{%"class._ZTSN11xercesc_2_716XMLErrorReporterE.xercesc_2_7::XMLErrorReporter" zeroinitializer, i32 1}
!417 = !{%"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1}
!418 = !{%"class._ZTSN11xercesc_2_711PSVIHandlerE.xercesc_2_7::PSVIHandler" zeroinitializer, i32 1}
!419 = !{%"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 1}
!420 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 0}
!421 = !{%"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 1}
!422 = !{%"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 1}
!423 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1}
!424 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 1}
!425 = !{%"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 1}
!426 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 0}
!427 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 0}
!428 = !{%"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 0}
!429 = !{!"S", %"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1, !356}
!430 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_7XMLAttrEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !431}
!431 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!432 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_7XMLAttrEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !433, !346}
!433 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 2}
!434 = !{!"S", %"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 11, !364, !435, !358, !358, !389, !361, !396, !346, !436, !364, !397}
!435 = !{!"A", i32 3, !364}
!436 = !{%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 1}
!437 = !{!"S", %"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 11, !438, !439, !440, !415, !441, !358, !442, !364, !358, !364, !346}
!438 = !{%"class._ZTSN11xercesc_2_77LocatorE.xercesc_2_7::Locator" zeroinitializer, i32 0}
!439 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 1}
!440 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 1}
!441 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!442 = !{%"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 1}
!443 = !{!"S", %"class._ZTSN11xercesc_2_712XMLValidatorE.xercesc_2_7::XMLValidator" zeroinitializer, i32 5, !356, !444, !416, !445, !446}
!444 = !{%"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 1}
!445 = !{%"class._ZTSN11xercesc_2_79ReaderMgrE.xercesc_2_7::ReaderMgr" zeroinitializer, i32 1}
!446 = !{%"class._ZTSN11xercesc_2_710XMLScannerE.xercesc_2_7::XMLScanner" zeroinitializer, i32 1}
!447 = !{!"S", %"class._ZTSN11xercesc_2_712XMLBufferMgrE.xercesc_2_7::XMLBufferMgr" zeroinitializer, i32 3, !358, !346, !448}
!448 = !{%"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 2}
!449 = !{!"S", %"class._ZTSN11xercesc_2_79XMLBufferE.xercesc_2_7::XMLBuffer" zeroinitializer, i32 7, !358, !358, !358, !364, !346, !450, !361}
!450 = !{%"class._ZTSN11xercesc_2_720XMLBufferFullHandlerE.xercesc_2_7::XMLBufferFullHandler" zeroinitializer, i32 1}
!451 = !{!"S", %"class._ZTSN11xercesc_2_715GrammarResolverE.xercesc_2_7::GrammarResolver" zeroinitializer, i32 12, !364, !364, !364, !424, !452, !452, !453, !346, !454, !390, !390, !455}
!452 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!453 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 1}
!454 = !{%"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 1}
!455 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!456 = !{!"S", %"class._ZTSN11xercesc_2_714XMLGrammarPoolE.xercesc_2_7::XMLGrammarPool" zeroinitializer, i32 4, !356, !346, !364, !397}
!457 = !{!"S", %"class._ZTSN11xercesc_2_715SecurityManagerE.xercesc_2_7::SecurityManager" zeroinitializer, i32 3, !356, !358, !389}
!458 = !{!"S", %"class._ZTSN11xercesc_2_79ElemStackE.xercesc_2_7::ElemStack" zeroinitializer, i32 13, !358, !358, !459, !460, !358, !358, !358, !358, !358, !358, !358, !461, !346}
!459 = !{%"class._ZTSN11xercesc_2_713XMLStringPoolE.xercesc_2_7::XMLStringPool" zeroinitializer, i32 0}
!460 = !{%"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 2}
!461 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!462 = !{!"S", %"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 8, !385, !358, !358, !361, !361, !361, !361, !346}
!463 = !{!"S", %"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1, !464}
!464 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 0}
!465 = !{!"S", %"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 7, !356, !358, !389, !390, !346, !358, !389}
!466 = !{!"S", %"class._ZTSN11xercesc_2_77XSModelE.xercesc_2_7::XSModel" zeroinitializer, i32 14, !346, !467, !468, !469, !471, !424, !473, !474, !475, !468, !390, !364, !364, !476}
!467 = !{%"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1}
!468 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!469 = !{!"A", i32 14, !470}
!470 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!471 = !{!"A", i32 14, !472}
!472 = !{%"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 1}
!473 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!474 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!475 = !{%"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 1}
!476 = !{!"A", i32 6, !364}
!477 = !{!"S", %"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 19, !385, !364, !364, !364, !364, !4, !358, !358, !358, !358, !358, !436, !478, !361, !479, !361, !361, !361, !346}
!478 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!479 = !{%"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 1}
!480 = !{!"S", %"class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1, !481}
!481 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!482 = !{!"S", %"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 24, !483, !435, !358, !358, !358, !358, !358, !358, !358, !358, !389, !361, !484, !485, !484, !436, !486, !487, !488, !436, !364, !364, !364, !489}
!483 = !{%"class._ZTSN11xercesc_2_714XMLElementDeclE.xercesc_2_7::XMLElementDecl.base" zeroinitializer, i32 0}
!484 = !{%"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 1}
!485 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!486 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!487 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 1}
!488 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 1}
!489 = !{!"A", i32 5, !364}
!490 = !{!"S", %"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 2, !358, !361}
!491 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !492, !358, !358, !358, !493}
!492 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!493 = !{%"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1}
!494 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !346, !364, !495, !358, !358, !493}
!495 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!496 = !{!"S", %"class._ZTSN11xercesc_2_717ValidationContextE.xercesc_2_7::ValidationContext" zeroinitializer, i32 2, !356, !346}
!497 = !{!"S", %"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 1, !385}
!498 = !{!"S", %"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 32, !358, !499, !358, !500, !501, !368, !368, !358, !361, !364, !364, !361, !358, !502, !358, !358, !358, !364, !358, !358, !364, !364, !361, !503, !364, !364, !504, !358, !338, !364, !358, !346}
!499 = !{!"A", i32 16384, !4}
!500 = !{!"A", i32 16384, !364}
!501 = !{!"A", i32 16384, !358}
!502 = !{!"A", i32 49152, !364}
!503 = !{%"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1}
!504 = !{%"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 1}
!505 = !{!"S", %"struct._ZTSN11xercesc_2_79ElemStack9StackElemE.xercesc_2_7::ElemStack::StackElem" zeroinitializer, i32 17, !401, !358, !358, !358, !506, !507, !358, !358, !364, !364, !364, !358, !423, !358, !361, !358, !358}
!506 = !{%"class._ZTSN11xercesc_2_75QNameE.xercesc_2_7::QName" zeroinitializer, i32 2}
!507 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 1}
!508 = !{!"S", %"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1, !356}
!509 = !{!"S", %"class._ZTSN11xercesc_2_717RegularExpressionE.xercesc_2_7::RegularExpression" zeroinitializer, i32 15, !364, !364, !358, !358, !358, !358, !510, !361, !361, !511, !376, !382, !512, !513, !346}
!510 = !{%"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 1}
!511 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 1}
!512 = !{%"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 0}
!513 = !{%"class._ZTSN11xercesc_2_712TokenFactoryE.xercesc_2_7::TokenFactory" zeroinitializer, i32 1}
!514 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfItEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !515, !346}
!515 = !{i16 0, i32 2}
!516 = !{!"S", %"class._ZTSN11xercesc_2_715ComplexTypeInfoE.xercesc_2_7::ComplexTypeInfo" zeroinitializer, i32 30, !385, !364, !364, !364, !364, !364, !358, !358, !358, !358, !358, !358, !358, !358, !361, !361, !361, !436, !436, !484, !402, !487, !517, !518, !485, !519, !361, !381, !520, !346}
!517 = !{%"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 1}
!518 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!519 = !{%"class._ZTSN11xercesc_2_715XMLContentModelE.xercesc_2_7::XMLContentModel" zeroinitializer, i32 1}
!520 = !{%"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 1}
!521 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !346, !364, !522, !358, !358, !493}
!522 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!523 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !524}
!524 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!525 = !{!"S", %"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 11, !526, !358, !358, !358, !358, !396, !436, !436, !436, !527, !487}
!526 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 0}
!527 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!528 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !529, !530, !338}
!529 = !{%"struct._ZTSN11xercesc_2_713XMLStringPool8PoolElemE.xercesc_2_7::XMLStringPool::PoolElem" zeroinitializer, i32 1}
!530 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_13XMLStringPool8PoolElemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!531 = !{!"S", %"class._ZTSN11xercesc_2_78HashBaseE.xercesc_2_7::HashBase" zeroinitializer, i32 1, !356}
!532 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !533, !534, !338, !358, !389}
!533 = !{%"class._ZTSN11xercesc_2_77XMLAttrE.xercesc_2_7::XMLAttr" zeroinitializer, i32 1}
!534 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_7XMLAttrEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!535 = !{!"S", %"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2, !358, !358}
!536 = !{!"S", %"class._ZTSN11xercesc_2_79BMPatternE.xercesc_2_7::BMPattern" zeroinitializer, i32 6, !364, !358, !381, !361, !361, !346}
!537 = !{!"S", %"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 4, !356, !346, !4, !511}
!538 = !{!"S", %"class._ZTSN11xercesc_2_79OpFactoryE.xercesc_2_7::OpFactory" zeroinitializer, i32 2, !539, !346}
!539 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!540 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !541}
!541 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!542 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !487, !543, !338, !358, !389}
!543 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!544 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18IdentityConstraintEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !545, !346}
!545 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 2}
!546 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIjEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !381, !346}
!547 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_2OpEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !548}
!548 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!549 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_17SchemaElementDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !550, !346}
!550 = !{%"class._ZTSN11xercesc_2_717SchemaElementDeclE.xercesc_2_7::SchemaElementDecl" zeroinitializer, i32 2}
!551 = !{!"S", %"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 8, !385, !361, !361, !552, !553, !346, !358, !389}
!552 = !{%"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 1}
!553 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!554 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_2OpEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !555, !346}
!555 = !{%"class._ZTSN11xercesc_2_72OpE.xercesc_2_7::Op" zeroinitializer, i32 2}
!556 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8IC_FieldEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !557}
!557 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!558 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8IC_FieldEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !559, !346}
!559 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 2}
!560 = !{!"S", %"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 3, !385, !561, !562}
!561 = !{%"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 1}
!562 = !{%"class._ZTSN11xercesc_2_718IdentityConstraintE.xercesc_2_7::IdentityConstraint" zeroinitializer, i32 1}
!563 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12KVStringPairEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !564, !358, !358, !358, !493}
!564 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!565 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !566, !567, !338}
!566 = !{%"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 1}
!567 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12KVStringPairEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!568 = !{!"S", %"class._ZTSN11xercesc_2_712KVStringPairE.xercesc_2_7::KVStringPair" zeroinitializer, i32 6, !385, !368, !368, !361, !361, !346}
!569 = !{!"S", %"class._ZTSN11xercesc_2_714BinInputStreamE.xercesc_2_7::BinInputStream" zeroinitializer, i32 1, !356}
!570 = !{!"S", %"class._ZTSN11xercesc_2_718XMLRegisterCleanupE.xercesc_2_7::XMLRegisterCleanup" zeroinitializer, i32 3, !571, !337, !337}
!571 = !{!572, i32 1}
!572 = !{!"F", i1 false, i32 0, !573}
!573 = !{!"void", i32 0}
!574 = !{!"S", %"class._ZTSN11xercesc_2_710XSDLocatorE.xercesc_2_7::XSDLocator" zeroinitializer, i32 5, !438, !368, !368, !361, !361}
!575 = !{!"S", %"class._ZTSN11xercesc_2_716SchemaAttDefListE.xercesc_2_7::SchemaAttDefList" zeroinitializer, i32 6, !576, !577, !485, !578, !358, !358}
!576 = !{%"class._ZTSN11xercesc_2_713XMLAttDefListE.xercesc_2_7::XMLAttDefList" zeroinitializer, i32 0}
!577 = !{%"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 1}
!578 = !{%"class._ZTSN11xercesc_2_712SchemaAttDefE.xercesc_2_7::SchemaAttDef" zeroinitializer, i32 2}
!579 = !{!"S", %"class._ZTSN11xercesc_2_729RefHash2KeysTableOfEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::RefHash2KeysTableOfEnumerator" zeroinitializer, i32 7, !580, !364, !543, !358, !485, !346, !338}
!580 = !{%"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 0}
!581 = !{!"S", %"class._ZTSN11xercesc_2_713XMLEnumeratorINS_12SchemaAttDefEEE.xercesc_2_7::XMLEnumerator" zeroinitializer, i32 1, !356}
!582 = !{!"S", %"class._ZTSN11xercesc_2_712XMLMutexLockE.xercesc_2_7::XMLMutexLock" zeroinitializer, i32 1, !336}
!583 = !{!"S", %"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 18, !584, !361, !585, !585, !585, !586, !587, !588, !589, !590, !591, !592, !419, !346, !593, !594, !364, !595}
!584 = !{%"class._ZTSN11xercesc_2_77GrammarE.xercesc_2_7::Grammar" zeroinitializer, i32 0}
!585 = !{%"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 1}
!586 = !{%"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 1}
!587 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!588 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!589 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!590 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!591 = !{%"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 1}
!592 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!593 = !{%"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1}
!594 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!595 = !{%"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 0}
!596 = !{!"S", %"class._ZTSN11xercesc_2_724DatatypeValidatorFactoryE.xercesc_2_7::DatatypeValidatorFactory" zeroinitializer, i32 3, !385, !597, !346}
!597 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!598 = !{!"S", %"class._ZTSN11xercesc_2_718RefHash3KeysIdPoolINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysIdPool" zeroinitializer, i32 8, !346, !364, !599, !358, !493, !550, !358, !358}
!599 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 2}
!600 = !{!"S", %"class._ZTSN11xercesc_2_710NameIdPoolINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPool" zeroinitializer, i32 7, !346, !601, !602, !358, !358, !358, !389}
!601 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2}
!602 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 2}
!603 = !{!"S", %"class._ZTSN11xercesc_2_720XMLSchemaDescriptionE.xercesc_2_7::XMLSchemaDescription" zeroinitializer, i32 1, !604}
!604 = !{%"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 0}
!605 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !606, !358, !358, !358, !493}
!606 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!607 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !608, !358, !358, !358, !493}
!608 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!609 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 5, !488, !610, !338, !358, !358}
!610 = !{%"struct._ZTSN11xercesc_2_727RefHash3KeysTableBucketElemINS_17SchemaElementDeclEEE.xercesc_2_7::RefHash3KeysTableBucketElem" zeroinitializer, i32 1}
!611 = !{!"S", %"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 2, !612, !613}
!612 = !{%"class._ZTSN11xercesc_2_715XMLNotationDeclE.xercesc_2_7::XMLNotationDecl" zeroinitializer, i32 1}
!613 = !{%"struct._ZTSN11xercesc_2_720NameIdPoolBucketElemINS_15XMLNotationDeclEEE.xercesc_2_7::NameIdPoolBucketElem" zeroinitializer, i32 1}
!614 = !{!"S", %"class._ZTSN11xercesc_2_721XMLGrammarDescriptionE.xercesc_2_7::XMLGrammarDescription" zeroinitializer, i32 2, !385, !346}
!615 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !616, !617, !338}
!616 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 1}
!617 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_12XSAnnotationEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!618 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !436, !619, !338}
!619 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_17DatatypeValidatorEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!620 = !{!"S", %"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 7, !385, !621, !361, !616, !361, !358, !358}
!621 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject.base" zeroinitializer, i32 0}
!622 = !{!"S", %"class._ZTSN11xercesc_2_711XercesXPathE.xercesc_2_7::XercesXPath" zeroinitializer, i32 5, !385, !358, !361, !623, !346}
!623 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!624 = !{!"S", %"class._ZTSN11xercesc_2_712ErrorHandlerE.xercesc_2_7::ErrorHandler" zeroinitializer, i32 1, !356}
!625 = !{!"S", %"class._ZTSN11xercesc_2_713XMLTranscoderE.xercesc_2_7::XMLTranscoder" zeroinitializer, i32 4, !356, !358, !361, !346}
!626 = !{!"S", %"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1, !627}
!627 = !{%"class._ZTSN11xercesc_2_712PanicHandlerE.xercesc_2_7::PanicHandler" zeroinitializer, i32 0}
!628 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_11PrefMapElemEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !629, !346}
!629 = !{%"struct._ZTSN11xercesc_2_711PrefMapElemE.xercesc_2_7::PrefMapElem" zeroinitializer, i32 2}
!630 = !{!"S", %"_DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 8, !364, !358, !562, !631, !632, !633, !446, !346}
!631 = !{%"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 0}
!632 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!633 = !{%"_DPRE_class._ZTSN11xercesc_2_710ValueStoreE.xercesc_2_7::ValueStore" zeroinitializer, i32 1}
!634 = !{!"S", %"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 4, !635, !636, !637, !346}
!635 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!636 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!637 = !{%"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 1}
!638 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !559, !346}
!639 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !640, !346}
!640 = !{%"class._ZTSN11xercesc_2_717DatatypeValidatorE.xercesc_2_7::DatatypeValidator" zeroinitializer, i32 2}
!641 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_7GrammarEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !642, !358, !358, !358, !493}
!642 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!643 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !423, !644, !338}
!644 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_7GrammarEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!645 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_13SchemaGrammarEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !646, !346}
!646 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 2}
!647 = !{!"S", %"class._ZTSN11xercesc_2_711IC_SelectorE.xercesc_2_7::IC_Selector" zeroinitializer, i32 3, !385, !561, !562}
!648 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !649, !358, !358, !358, !493}
!649 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!650 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !651, !652, !338}
!651 = !{%"class._ZTSN11xercesc_2_79XMLAttDefE.xercesc_2_7::XMLAttDef" zeroinitializer, i32 1}
!652 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_9XMLAttDefEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!653 = !{!"S", %"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1, !654}
!654 = !{%"class._ZTSN11xercesc_2_713MemoryManagerE.xercesc_2_7::MemoryManager" zeroinitializer, i32 0}
!655 = !{!"S", %"class._ZTSN11xercesc_2_714NamespaceScopeE.xercesc_2_7::NamespaceScope" zeroinitializer, i32 6, !358, !358, !358, !459, !656, !346}
!656 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 2}
!657 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope9StackElemE.xercesc_2_7::NamespaceScope::StackElem" zeroinitializer, i32 3, !658, !358, !358}
!658 = !{%"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 1}
!659 = !{!"S", %"struct._ZTSN11xercesc_2_714NamespaceScope11PrefMapElemE.xercesc_2_7::NamespaceScope::PrefMapElem" zeroinitializer, i32 2, !358, !358}
!660 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_5TokenEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !661}
!661 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!662 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_5TokenEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !663, !346}
!663 = !{%"class._ZTSN11xercesc_2_75TokenE.xercesc_2_7::Token" zeroinitializer, i32 2}
!664 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !665, !666}
!665 = !{!"A", i32 8, !364}
!666 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!667 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !668}
!668 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!669 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13XMLEntityDeclEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !670, !346}
!670 = !{%"class._ZTSN11xercesc_2_713XMLEntityDeclE.xercesc_2_7::XMLEntityDecl" zeroinitializer, i32 2}
!671 = !{!"S", %"class._ZTSN11xercesc_2_710RefStackOfINS_9XMLReaderEEE.xercesc_2_7::RefStackOf" zeroinitializer, i32 2, !665, !672}
!672 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 0}
!673 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_9XMLReaderEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !674}
!674 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!675 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_9XMLReaderEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !676, !346}
!676 = !{%"class._ZTSN11xercesc_2_79XMLReaderE.xercesc_2_7::XMLReader" zeroinitializer, i32 2}
!677 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !678, !358, !358, !358, !493}
!678 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!679 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !484, !680, !338}
!680 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15ComplexTypeInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!681 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !682, !358, !358, !358, !493}
!682 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!683 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !684, !685, !338}
!684 = !{%"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 1}
!685 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XercesGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!686 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !687, !358, !358, !358, !493}
!687 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!688 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !689, !690, !338}
!689 = !{%"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 1}
!690 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_18XercesAttGroupInfoEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!691 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !346, !364, !692, !358, !358, !493}
!692 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!693 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !694, !695, !338, !358, !389}
!694 = !{%"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 1}
!695 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_13ValueVectorOfIPNS_17SchemaElementDeclEEEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!696 = !{!"S", %"class._ZTSN11xercesc_2_715XercesGroupInfoE.xercesc_2_7::XercesGroupInfo" zeroinitializer, i32 9, !385, !364, !358, !358, !358, !402, !518, !684, !520}
!697 = !{!"S", %"class._ZTSN11xercesc_2_718XercesAttGroupInfoE.xercesc_2_7::XercesAttGroupInfo" zeroinitializer, i32 8, !385, !364, !358, !358, !698, !698, !487, !346}
!698 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!699 = !{!"S", %"class._ZTSN11xercesc_2_713ValueVectorOfIPNS_17SchemaElementDeclEEE.xercesc_2_7::ValueVectorOf" zeroinitializer, i32 5, !364, !358, !358, !550, !346}
!700 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !701}
!701 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!702 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12SchemaAttDefEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !578, !346}
!703 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !704}
!704 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!705 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !706, !346}
!706 = !{%"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 2}
!707 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !708}
!708 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!709 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_18XercesLocationPathEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !710, !346}
!710 = !{%"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2}
!711 = !{!"S", %"class._ZTSN11xercesc_2_718XercesLocationPathE.xercesc_2_7::XercesLocationPath" zeroinitializer, i32 2, !385, !712}
!712 = !{%"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1}
!713 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !714, !346}
!714 = !{%"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 2}
!715 = !{!"S", %"class._ZTSN11xercesc_2_710XercesStepE.xercesc_2_7::XercesStep" zeroinitializer, i32 3, !385, !4, !716}
!716 = !{%"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 1}
!717 = !{!"S", %"class._ZTSN11xercesc_2_714XercesNodeTestE.xercesc_2_7::XercesNodeTest" zeroinitializer, i32 3, !385, !4, !396}
!718 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_10XercesStepEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !719}
!719 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_10XercesStepEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!720 = !{!"S", %"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 7, !346, !721, !390, !471, !473, !722, !361}
!721 = !{%"class._ZTSN11xercesc_2_713SchemaGrammarE.xercesc_2_7::SchemaGrammar" zeroinitializer, i32 1}
!722 = !{!"A", i32 14, !723}
!723 = !{%"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 1}
!724 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !725}
!725 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!726 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_12XSAnnotationEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !727, !346}
!727 = !{%"class._ZTSN11xercesc_2_712XSAnnotationE.xercesc_2_7::XSAnnotation" zeroinitializer, i32 2}
!728 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !729}
!729 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!730 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_15XSNamespaceItemEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !731, !346}
!731 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 2}
!732 = !{!"S", %"class._ZTSN11xercesc_2_710XSNamedMapINS_8XSObjectEEE.xercesc_2_7::XSNamedMap" zeroinitializer, i32 4, !346, !424, !470, !733}
!733 = !{%"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 1}
!734 = !{!"S", %"class._ZTSN11xercesc_2_711RefVectorOfINS_8XSObjectEEE.xercesc_2_7::RefVectorOf" zeroinitializer, i32 1, !735}
!735 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 0}
!736 = !{!"S", %"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_8XSObjectEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 6, !356, !364, !358, !358, !737, !346}
!737 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 2}
!738 = !{!"S", %"class._ZTSN11xercesc_2_719RefHash2KeysTableOfINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableOf" zeroinitializer, i32 6, !346, !364, !739, !358, !358, !493}
!739 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 2}
!740 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_8XSObjectEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !741, !358, !358, !358, !493}
!741 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!742 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !743, !744, !338}
!743 = !{%"class._ZTSN11xercesc_2_78XSObjectE.xercesc_2_7::XSObject" zeroinitializer, i32 1}
!744 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!745 = !{!"S", %"class._ZTSN11xercesc_2_714RefHashTableOfINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableOf" zeroinitializer, i32 7, !346, !364, !746, !358, !358, !358, !493}
!746 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 2}
!747 = !{!"S", %"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 3, !748, !749, !338}
!748 = !{%"class._ZTSN11xercesc_2_715XSNamespaceItemE.xercesc_2_7::XSNamespaceItem" zeroinitializer, i32 1}
!749 = !{%"struct._ZTSN11xercesc_2_722RefHashTableBucketElemINS_15XSNamespaceItemEEE.xercesc_2_7::RefHashTableBucketElem" zeroinitializer, i32 1}
!750 = !{!"S", %"class._ZTSN11xercesc_2_715XSObjectFactoryE.xercesc_2_7::XSObjectFactory" zeroinitializer, i32 3, !346, !723, !470}
!751 = !{!"S", %"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 5, !743, !752, !338, !358, !389}
!752 = !{%"struct._ZTSN11xercesc_2_727RefHash2KeysTableBucketElemINS_8XSObjectEEE.xercesc_2_7::RefHash2KeysTableBucketElem" zeroinitializer, i32 1}
!753 = !{!"S", %"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1, !654}
!754 = !{!"S", %"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1, !755}
!755 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}
!756 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !356}
!757 = !{!"S", %"_DPRE__REP_class._ZTSN11xercesc_2_716RefArrayVectorOfItEE.xercesc_2_7::RefArrayVectorOf" zeroinitializer, i32 5, !364, !358, !358, !515, !346}
!758 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 -1}
!759 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 -1}
!760 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 -1}
!761 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)"}
!762 = !{i32 1, !"wchar_size", i32 4}
!763 = !{i32 1, !"Virtual Function Elim", i32 0}
!764 = !{i32 7, !"uwtable", i32 2}
!765 = !{i32 1, !"ThinLTO", i32 0}
!766 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!767 = !{i32 1, !"LTOPostLink", i32 1}
!768 = distinct !{!335, !361}
!769 = !{!770, !770, i64 0}
!770 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !771, i64 0}
!771 = !{!"omnipotent char", !772, i64 0}
!772 = !{!"Simple C++ TBAA"}
!773 = distinct !{!338, !338}
!774 = !{!775, !775, i64 0}
!775 = !{!"pointer@_ZTSPN11xercesc_2_712PanicHandlerE", !771, i64 0}
!776 = !{!777, !777, i64 0}
!777 = !{!"vtable pointer", !772, i64 0}
!778 = !{!"F", i1 false, i32 2, !573, !334, !358}
!779 = !{!"_Intel.Devirt.Call"}
!780 = distinct !{!338}
!781 = distinct !{!338, !338, !338}
!782 = distinct !{!338}
!783 = distinct !{!784, !338, !346}
!784 = !{%"class._ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE.xercesc_2_7::ArrayIndexOutOfBoundsException" zeroinitializer, i32 1}
!785 = distinct !{!338}
!786 = distinct !{!361, !361, !346}
!787 = !{!788, !788, i64 0}
!788 = !{!"short", !771, i64 0}
!789 = distinct !{!789, !790}
!790 = !{!"llvm.loop.mustprogress"}
!791 = !{!"F", i1 false, i32 2, !338, !346, !368}
!792 = distinct !{!793}
!793 = !{%"class._ZTSN11xercesc_2_719DefaultPanicHandlerE.xercesc_2_7::DefaultPanicHandler" zeroinitializer, i32 1}
!794 = !{!795, !795, i64 0}
!795 = !{!"pointer@_ZTSP8_IO_FILE", !771, i64 0}
!796 = distinct !{!345, !338}
!797 = distinct !{!798, !346}
!798 = !{%"_DPRE_class._ZTSN11xercesc_2_713FieldValueMapE.xercesc_2_7::FieldValueMap" zeroinitializer, i32 1}
!799 = !{!800, !801, i64 0}
!800 = !{!"struct@_ZTSN11xercesc_2_713FieldValueMapE", !801, i64 0, !802, i64 8, !803, i64 16, !770, i64 24}
!801 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !771, i64 0}
!802 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !771, i64 0}
!803 = !{!"pointer@_ZTSPN11xercesc_2_716RefArrayVectorOfItEE", !771, i64 0}
!804 = !{!800, !802, i64 8}
!805 = !{!800, !803, i64 16}
!806 = !{!800, !770, i64 24}
!807 = distinct !{!798, !798}
!808 = distinct !{!808, !790}
!809 = !{!"F", i1 false, i32 1, !573, !798}
!810 = distinct !{!635, !635}
!811 = !{!812, !813, i64 0}
!812 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !813, i64 0, !814, i64 4, !814, i64 8, !815, i64 16, !770, i64 24}
!813 = !{!"bool", !771, i64 0}
!814 = !{!"int", !771, i64 0}
!815 = !{!"pointer@_ZTSPPN11xercesc_2_78IC_FieldE", !771, i64 0}
!816 = !{i8 0, i8 2}
!817 = !{}
!818 = !{!812, !814, i64 4}
!819 = !{!812, !814, i64 8}
!820 = !{!812, !815, i64 16}
!821 = !{!812, !770, i64 24}
!822 = !{!823, !823, i64 0}
!823 = !{!"pointer@_ZTSPN11xercesc_2_78IC_FieldE", !771, i64 0}
!824 = distinct !{!824, !790}
!825 = distinct !{!636, !636}
!826 = !{!827, !813, i64 0}
!827 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !813, i64 0, !814, i64 4, !814, i64 8, !828, i64 16, !770, i64 24}
!828 = !{!"pointer@_ZTSPPN11xercesc_2_717DatatypeValidatorE", !771, i64 0}
!829 = !{!827, !814, i64 4}
!830 = !{!827, !814, i64 8}
!831 = !{!827, !828, i64 16}
!832 = !{!827, !770, i64 24}
!833 = !{!834, !834, i64 0}
!834 = !{!"pointer@_ZTSPN11xercesc_2_717DatatypeValidatorE", !771, i64 0}
!835 = distinct !{!835, !790}
!836 = distinct !{!798}
!837 = distinct !{!635}
!838 = !{!"F", i1 false, i32 2, !573, !346, !338}
!839 = distinct !{!636}
!840 = distinct !{!798}
!841 = distinct !{!798, !842}
!842 = !{%"class._ZTSN11xercesc_2_78IC_FieldE.xercesc_2_7::IC_Field" zeroinitializer, i32 1}
!843 = distinct !{!843, !790}
!844 = distinct !{!635}
!845 = distinct !{!559, !635}
!846 = distinct !{!338}
!847 = distinct !{!848, !361}
!848 = !{%"class._ZTSN11xercesc_2_714InMemMsgLoaderE.xercesc_2_7::InMemMsgLoader" zeroinitializer, i32 1}
!849 = !{!850, !852, i64 8}
!850 = !{!"struct@_ZTSN11xercesc_2_714InMemMsgLoaderE", !851, i64 0, !852, i64 8}
!851 = !{!"struct@_ZTSN11xercesc_2_712XMLMsgLoaderE"}
!852 = !{!"pointer@_ZTSPt", !771, i64 0}
!853 = distinct !{!853, !790}
!854 = !{!855, !856, i64 0}
!855 = !{!"array@_ZTSA311_A128_t", !856, i64 0}
!856 = !{!"array@_ZTSA128_t", !788, i64 0}
!857 = !{!858, !856, i64 0}
!858 = !{!"array@_ZTSA401_A128_t", !856, i64 0}
!859 = !{!860, !856, i64 0}
!860 = !{!"array@_ZTSA114_A128_t", !856, i64 0}
!861 = !{!862, !856, i64 0}
!862 = !{!"array@_ZTSA25_A128_t", !856, i64 0}
!863 = distinct !{!863, !790}
!864 = distinct !{!865}
!865 = !{%"class._ZTSN11xercesc_2_720OutOfMemoryExceptionE.xercesc_2_7::OutOfMemoryException" zeroinitializer, i32 1}
!866 = distinct !{!338, !867}
!867 = !{%"class._ZTSN11xercesc_2_717MemoryManagerImplE.xercesc_2_7::MemoryManagerImpl" zeroinitializer, i32 1}
!868 = !{!"_Intel.Devirt.Target"}
!869 = distinct !{!867, !338}
!870 = distinct !{!338}
!871 = distinct !{!335, !361}
!872 = distinct !{!436, !798}
!873 = distinct !{!361, !798}
!874 = distinct !{!798, !842, !436, !361}
!875 = !{!"F", i1 false, i32 3, !573, !637, !361, !358}
!876 = distinct !{!798}
!877 = distinct !{!633, !798}
!878 = !{!879, !881, i64 48}
!879 = !{!"struct@_ZTSN11xercesc_2_710ValueStoreE", !813, i64 0, !814, i64 4, !880, i64 8, !800, i64 16, !881, i64 48, !882, i64 56, !883, i64 64, !770, i64 72}
!880 = !{!"pointer@_ZTSPN11xercesc_2_718IdentityConstraintE", !771, i64 0}
!881 = !{!"pointer@_ZTSPN11xercesc_2_711RefVectorOfINS_13FieldValueMapEEE", !771, i64 0}
!882 = !{!"pointer@_ZTSPN11xercesc_2_710ValueStoreE", !771, i64 0}
!883 = !{!"pointer@_ZTSPN11xercesc_2_710XMLScannerE", !771, i64 0}
!884 = !{!885, !814, i64 12}
!885 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE", !813, i64 8, !814, i64 12, !814, i64 16, !886, i64 24, !770, i64 32}
!886 = !{!"pointer@_ZTSPPN11xercesc_2_713FieldValueMapE", !771, i64 0}
!887 = distinct !{!887, !790}
!888 = distinct !{!888, !790}
!889 = distinct !{!798, !890}
!890 = !{%"class._ZTSN11xercesc_2_715BaseRefVectorOfINS_13FieldValueMapEEE.xercesc_2_7::BaseRefVectorOf" zeroinitializer, i32 1}
!891 = !{!885, !770, i64 32}
!892 = !{!885, !886, i64 24}
!893 = !{!894, !894, i64 0}
!894 = !{!"pointer@_ZTSPN11xercesc_2_713FieldValueMapE", !771, i64 0}
!895 = distinct !{!633, !436, !361, !436, !361}
!896 = distinct !{!896, !790}
!897 = !{!879, !770, i64 72}
!898 = !{!"F", i1 false, i32 4, !358, !436, !361, !361, !346}
!899 = distinct !{!635, !346}
!900 = distinct !{!636, !346}
!901 = distinct !{!635, !559}
!902 = distinct !{!636, !640}
!903 = distinct !{!636, !640}
!904 = distinct !{!636}
!905 = distinct !{!905, !790}
!906 = distinct !{!635}
!907 = distinct !{!907, !790}
!908 = distinct !{!640, !636}
!909 = distinct !{!910}
!910 = !{%"class._ZTSN11xercesc_2_712XMLExceptionE.xercesc_2_7::XMLException" zeroinitializer, i32 1}
!911 = !{!912, !770, i64 40}
!912 = !{!"struct@_ZTSN11xercesc_2_712XMLExceptionE", !913, i64 8, !914, i64 16, !814, i64 24, !852, i64 32, !770, i64 40}
!913 = !{!"_ZTSN11xercesc_2_710XMLExcepts5CodesE", !771, i64 0}
!914 = !{!"pointer@_ZTSPc", !771, i64 0}
!915 = !{!912, !852, i64 32}
!916 = !{!912, !914, i64 16}
!917 = distinct !{!910, !338, !346}
!918 = !{!912, !913, i64 8}
!919 = !{!912, !814, i64 24}
!920 = distinct !{!910}
!921 = !{!"A", i32 2048, !4}
!922 = !{!"F", i1 false, i32 4, !923, !335, !358, !361, !358}
!923 = !{i1 false, i32 0}
!924 = distinct !{!924, !790}
!925 = distinct !{!335}
!926 = !{!927, !927, i64 0}
!927 = !{!"pointer@_ZTSPN11xercesc_2_712XMLMsgLoaderE", !771, i64 0}
!928 = !{!929, !929, i64 0}
!929 = !{!"pointer@_ZTSPN11xercesc_2_78XMLMutexE", !771, i64 0}
!930 = distinct !{!337, !571}
!931 = !{!932, !933, i64 0}
!932 = !{!"struct@_ZTSN11xercesc_2_718XMLRegisterCleanupE", !933, i64 0, !934, i64 8, !934, i64 16}
!933 = !{!"pointer@_ZTSPFvvE", !771, i64 0}
!934 = !{!"pointer@_ZTSPN11xercesc_2_718XMLRegisterCleanupE", !771, i64 0}
!935 = !{!932, !934, i64 8}
!936 = !{!932, !934, i64 16}
!937 = !{!934, !934, i64 0}
!938 = distinct !{!338, !338, !346}
!939 = distinct !{!338}
!940 = !{!771, !771, i64 0}
!941 = distinct !{!338, !346}
!942 = distinct !{!338}
!943 = distinct !{!338, !346}
!944 = distinct !{!945}
!945 = !{%"class._ZTSSt9bad_alloc.std::bad_alloc" zeroinitializer, i32 1}
!946 = distinct !{!338, !947}
!947 = !{%"class._ZTSN11xalanc_1_1023XalanDummyMemoryManagerE.xalanc_1_10::XalanDummyMemoryManager" zeroinitializer, i32 1}
!948 = distinct !{!947, !338}
!949 = distinct !{!637}
!950 = !{!951, !814, i64 12}
!951 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfItEE", !813, i64 8, !814, i64 12, !814, i64 16, !952, i64 24, !770, i64 32}
!952 = !{!"pointer@_ZTSPPt", !771, i64 0}
!953 = distinct !{!637, !346}
!954 = !{!951, !813, i64 8}
!955 = !{!951, !814, i64 16}
!956 = !{!951, !952, i64 24}
!957 = !{!951, !770, i64 32}
!958 = !{!852, !852, i64 0}
!959 = distinct !{!361, !637}
!960 = distinct !{!637, !515}
!961 = distinct !{!637}
!962 = distinct !{!962, !790}
!963 = distinct !{!637}
!964 = distinct !{!964, !790}
!965 = distinct !{!637, !361}
!966 = distinct !{!637, !361}
!967 = distinct !{!637, !637}
!968 = distinct !{!848, !361}
!969 = distinct !{!336, !346}
!970 = distinct !{!971, !336}
!971 = !{%"class._ZTSN11xercesc_2_712XMLMutexLockE.xercesc_2_7::XMLMutexLock" zeroinitializer, i32 1}
!972 = distinct !{!971}
