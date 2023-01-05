; CMPLRLLVM-35401: This test verifies that store to %notconst is not
; eliminated by InstCombine using AndersensAA.
; AndersensAA was incorrectly computing points-to info for %1, %add.ptr.i.i334
; and %notconst.

; RUN: opt < %s -passes="require<anders-aa>,instcombine" -S  2>&1 | FileCheck %s

; CHECK: store %"class.clang::NamedDecl"* null, %"class.clang::NamedDecl"** %notconst

%"class.clang::Scope" = type { %"class.clang::Scope"*, i32, i16, i16, i16, i16, i16, %"class.clang::Scope"*, %"class.clang::Scope"*, %"class.clang::Scope"*, %"class.clang::Scope"*, %"class.clang::Scope"*, %"class.clang::Scope"*, %"class.llvm::SmallPtrSet.1438", %"class.clang::DeclContext"*, %"class.llvm::SmallVector.1441", %"class.clang::DiagnosticErrorTrap", %"class.llvm::PointerIntPair.1446" }
%"class.llvm::SmallPtrSet.1438" = type { %"class.llvm::SmallPtrSetImpl.base.1440", [32 x i8*] }
%"class.llvm::SmallPtrSetImpl.base.1440" = type { %"class.llvm::SmallPtrSetImplBase.base" }
%"class.llvm::SmallPtrSetImplBase.base" = type <{ %"class.llvm::DebugEpochBase", i8**, i8**, i32, i32, i32 }>
%"class.llvm::DebugEpochBase" = type { i64 }
%"class.clang::DeclContext" = type { %"class.clang::StoredDeclsMap"*, %union.anon.245, %"class.clang::Decl"*, %"class.clang::Decl"* }
%"class.clang::StoredDeclsMap" = type opaque
%union.anon.245 = type { %"class.clang::DeclContext::CXXConstructorDeclBitfields" }
%"class.clang::DeclContext::CXXConstructorDeclBitfields" = type { i64 }
%"class.clang::Decl" = type <{ i32 (...)**, %"class.llvm::PointerIntPair.235", %"class.llvm::PointerUnion", %"class.clang::SourceLocation", i32, i8, [7 x i8] }>
%"class.llvm::PointerIntPair.235" = type { i64 }
%"class.llvm::PointerUnion" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers" }
%"class.llvm::pointer_union_detail::PointerUnionMembers" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.236" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.236" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.237" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.237" = type { %"class.llvm::PointerIntPair.238" }
%"class.llvm::PointerIntPair.238" = type { i64 }
%"class.clang::SourceLocation" = type { i32 }
%"class.llvm::SmallVector.1441" = type { %"class.llvm::SmallVectorImpl.1442", %"struct.llvm::SmallVectorStorage.1445" }
%"class.llvm::SmallVectorImpl.1442" = type { %"class.llvm::SmallVectorTemplateBase.1443" }
%"class.llvm::SmallVectorTemplateBase.1443" = type { %"class.llvm::SmallVectorTemplateCommon.1444" }
%"class.llvm::SmallVectorTemplateCommon.1444" = type { %"class.llvm::SmallVectorBase" }
%"class.llvm::SmallVectorBase" = type { i8*, i32, i32 }
%"struct.llvm::SmallVectorStorage.1445" = type { [16 x i8] }
%"class.clang::DiagnosticErrorTrap" = type { %"class.clang::DiagnosticsEngine"*, i32, i32 }
%"class.clang::DiagnosticsEngine" = type { %"class.llvm::RefCountedBase.35", i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, %"class.llvm::IntrusiveRefCntPtr", %"class.llvm::IntrusiveRefCntPtr.39", %"class.clang::DiagnosticConsumer"*, %"class.std::unique_ptr.41", %"class.clang::SourceManager"*, %"class.clang::SyclOptReportHandler", %"class.std::__cxx11::list", %"class.clang::DiagnosticsEngine::DiagStateMap", %"class.std::vector.289", i8, i8, i8, i8, i32, i32, i32, i32, i32, i8*, void (i32, i64, i8*, i64, i8*, i64, %"class.llvm::ArrayRef"*, %"class.llvm::SmallVectorImpl.297"*, i8*, %"class.llvm::ArrayRef.301"*)*, i32, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", %"class.clang::OpenMPOptReportHandler", %"class.clang::SourceLocation", i32, %"struct.clang::DiagnosticStorage" }
%"class.llvm::RefCountedBase.35" = type { i32 }
%"class.llvm::IntrusiveRefCntPtr" = type { %"class.clang::DiagnosticIDs"* }
%"class.clang::DiagnosticIDs" = type { %"class.llvm::RefCountedBase.36", %"class.std::unique_ptr" }
%"class.llvm::RefCountedBase.36" = type { i32 }
%"class.std::unique_ptr" = type { %"struct.std::__uniq_ptr_data" }
%"struct.std::__uniq_ptr_data" = type { %"class.std::__uniq_ptr_impl" }
%"class.std::__uniq_ptr_impl" = type { %"class.std::tuple" }
%"class.std::tuple" = type { %"struct.std::_Tuple_impl" }
%"struct.std::_Tuple_impl" = type { %"struct.std::_Head_base.38" }
%"struct.std::_Head_base.38" = type { %"class.clang::diag::CustomDiagInfo"* }
%"class.clang::diag::CustomDiagInfo" = type opaque
%"class.llvm::IntrusiveRefCntPtr.39" = type { %"class.clang::DiagnosticOptions"* }
%"class.clang::DiagnosticOptions" = type { %"class.llvm::RefCountedBase.40", i24, i264, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", %"class.std::vector.18", %"class.std::vector.18", %"class.std::vector.18", %"class.std::vector.18" }
%"class.llvm::RefCountedBase.40" = type { i32 }
%"class.std::vector.18" = type { %"struct.std::_Vector_base.19" }
%"struct.std::_Vector_base.19" = type { %"struct.std::_Vector_base<std::__cxx11::basic_string<char>, std::allocator<std::__cxx11::basic_string<char>>>::_Vector_impl" }
%"struct.std::_Vector_base<std::__cxx11::basic_string<char>, std::allocator<std::__cxx11::basic_string<char>>>::_Vector_impl" = type { %"struct.std::_Vector_base<std::__cxx11::basic_string<char>, std::allocator<std::__cxx11::basic_string<char>>>::_Vector_impl_data" }
%"struct.std::_Vector_base<std::__cxx11::basic_string<char>, std::allocator<std::__cxx11::basic_string<char>>>::_Vector_impl_data" = type { %"class.std::__cxx11::basic_string"*, %"class.std::__cxx11::basic_string"*, %"class.std::__cxx11::basic_string"* }
%"class.clang::DiagnosticConsumer" = type { i32 (...)**, i32, i32 }
%"class.std::unique_ptr.41" = type { %"struct.std::__uniq_ptr_data.42" }
%"struct.std::__uniq_ptr_data.42" = type { %"class.std::__uniq_ptr_impl.43" }
%"class.std::__uniq_ptr_impl.43" = type { %"class.std::tuple.44" }
%"class.std::tuple.44" = type { %"struct.std::_Tuple_impl.45" }
%"struct.std::_Tuple_impl.45" = type { %"struct.std::_Head_base.50" }
%"struct.std::_Head_base.50" = type { %"class.clang::DiagnosticConsumer"* }
%"class.clang::SourceManager" = type { %"class.llvm::RefCountedBase.51", %"class.clang::DiagnosticsEngine"*, %"class.clang::FileManager"*, %"class.llvm::BumpPtrAllocatorImpl", %"class.llvm::DenseMap.120", i8, i8, i8, %"class.std::unique_ptr.145", %"class.std::vector.165", %"class.llvm::SmallVector.170", %"class.llvm::SmallVector.170", i32, i32, %"class.llvm::BitVector", %"class.clang::ExternalSLocEntrySource"*, %"class.clang::FileID", %"class.std::unique_ptr.180", %"class.clang::FileID", %"class.clang::SrcMgr::ContentCache"*, i32, i32, %"class.clang::FileID", %"class.clang::FileID", i32, i32, %"class.llvm::DenseMap.190", %"class.llvm::DenseMap.194", %"class.clang::InBeforeInTUCacheEntry", %"class.std::unique_ptr.134", %"class.std::unique_ptr.198", %"class.std::unique_ptr.208", %"class.llvm::DenseMap.220", %"class.llvm::SmallVector.224" }
%"class.llvm::RefCountedBase.51" = type { i32 }
%"class.clang::FileManager" = type { %"class.llvm::RefCountedBase.52", %"class.llvm::IntrusiveRefCntPtr.53", %"class.clang::FileSystemOptions", %"class.std::map.54", %"class.std::map.61", %"class.llvm::SmallVector.66", %"class.llvm::SmallVector.71", %"class.llvm::SmallVector.76", %"class.llvm::StringMap.78", %"class.llvm::StringMap.90", %"class.std::unique_ptr.91", %"class.llvm::Optional.102", %"class.llvm::DenseMap.106", %"class.llvm::BumpPtrAllocatorImpl", i32, %"class.std::unique_ptr.110" }
%"class.llvm::RefCountedBase.52" = type { i32 }
%"class.llvm::IntrusiveRefCntPtr.53" = type { %"class.llvm::vfs::FileSystem"* }
%"class.llvm::vfs::FileSystem" = type <{ i32 (...)**, %"class.llvm::ThreadSafeRefCountedBase", [4 x i8] }>
%"class.llvm::ThreadSafeRefCountedBase" = type { %"struct.std::atomic" }
%"struct.std::atomic" = type { %"struct.std::__atomic_base" }
%"struct.std::__atomic_base" = type { i32 }
%"class.clang::FileSystemOptions" = type { %"class.std::__cxx11::basic_string" }
%"class.std::map.54" = type { %"class.std::_Rb_tree.55" }
%"class.std::_Rb_tree.55" = type { %"struct.std::_Rb_tree<llvm::sys::fs::UniqueID, std::pair<const llvm::sys::fs::UniqueID, clang::DirectoryEntry>, std::_Select1st<std::pair<const llvm::sys::fs::UniqueID, clang::DirectoryEntry>>, std::less<llvm::sys::fs::UniqueID>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<llvm::sys::fs::UniqueID, std::pair<const llvm::sys::fs::UniqueID, clang::DirectoryEntry>, std::_Select1st<std::pair<const llvm::sys::fs::UniqueID, clang::DirectoryEntry>>, std::less<llvm::sys::fs::UniqueID>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.59", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare.59" = type { %"struct.std::less" }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_header" = type { %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::_Rb_tree_node_base" = type { i32, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"* }
%"class.std::map.61" = type { %"class.std::_Rb_tree.62" }
%"class.std::_Rb_tree.62" = type { %"struct.std::_Rb_tree<llvm::sys::fs::UniqueID, std::pair<const llvm::sys::fs::UniqueID, clang::FileEntry>, std::_Select1st<std::pair<const llvm::sys::fs::UniqueID, clang::FileEntry>>, std::less<llvm::sys::fs::UniqueID>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<llvm::sys::fs::UniqueID, std::pair<const llvm::sys::fs::UniqueID, clang::FileEntry>, std::_Select1st<std::pair<const llvm::sys::fs::UniqueID, clang::FileEntry>>, std::less<llvm::sys::fs::UniqueID>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.59", %"struct.std::_Rb_tree_header" }
%"class.llvm::SmallVector.66" = type { %"class.llvm::SmallVectorImpl.67", %"struct.llvm::SmallVectorStorage.70" }
%"class.llvm::SmallVectorImpl.67" = type { %"class.llvm::SmallVectorTemplateBase.68" }
%"class.llvm::SmallVectorTemplateBase.68" = type { %"class.llvm::SmallVectorTemplateCommon.69" }
%"class.llvm::SmallVectorTemplateCommon.69" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.70" = type { [32 x i8] }
%"class.llvm::SmallVector.71" = type { %"class.llvm::SmallVectorImpl.72", %"struct.llvm::SmallVectorStorage.75" }
%"class.llvm::SmallVectorImpl.72" = type { %"class.llvm::SmallVectorTemplateBase.73" }
%"class.llvm::SmallVectorTemplateBase.73" = type { %"class.llvm::SmallVectorTemplateCommon.74" }
%"class.llvm::SmallVectorTemplateCommon.74" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.75" = type { [32 x i8] }
%"class.llvm::SmallVector.76" = type { %"class.llvm::SmallVectorImpl.72" }
%"class.llvm::StringMap.78" = type { %"class.llvm::StringMapImpl", %"class.llvm::BumpPtrAllocatorImpl" }
%"class.llvm::StringMapImpl" = type { %"class.llvm::StringMapEntryBase"**, i32, i32, i32, i32 }
%"class.llvm::StringMapEntryBase" = type { i64 }
%"class.llvm::StringMap.90" = type { %"class.llvm::StringMapImpl", %"class.llvm::BumpPtrAllocatorImpl" }
%"class.std::unique_ptr.91" = type { %"struct.std::__uniq_ptr_data.92" }
%"struct.std::__uniq_ptr_data.92" = type { %"class.std::__uniq_ptr_impl.93" }
%"class.std::__uniq_ptr_impl.93" = type { %"class.std::tuple.94" }
%"class.std::tuple.94" = type { %"struct.std::_Tuple_impl.95" }
%"struct.std::_Tuple_impl.95" = type { %"struct.std::_Head_base.100" }
%"struct.std::_Head_base.100" = type { %"class.llvm::StringMap.101"* }
%"class.llvm::StringMap.101" = type opaque
%"class.llvm::Optional.102" = type { %"class.llvm::optional_detail::OptionalStorage.103" }
%"class.llvm::optional_detail::OptionalStorage.103" = type { %"class.clang::FileMgr::MapEntryOptionalStorage" }
%"class.clang::FileMgr::MapEntryOptionalStorage" = type { %"class.clang::FileEntryRef" }
%"class.clang::FileEntryRef" = type { %"class.llvm::StringMapEntry"* }
%"class.llvm::StringMapEntry" = type { %"class.llvm::StringMapEntryStorage" }
%"class.llvm::StringMapEntryStorage" = type { %"class.llvm::StringMapEntryBase", %"class.llvm::ErrorOr" }
%"class.llvm::ErrorOr" = type { %union.anon.104, i8, [7 x i8] }
%union.anon.104 = type { %"struct.llvm::AlignedCharArrayUnion" }
%"struct.llvm::AlignedCharArrayUnion" = type { [16 x i8] }
%"class.llvm::DenseMap.106" = type <{ %"class.llvm::DenseMapBase.107", %"struct.llvm::detail::DenseMapPair.108"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.107" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.108" = type opaque
%"class.std::unique_ptr.110" = type { %"struct.std::__uniq_ptr_data.111" }
%"struct.std::__uniq_ptr_data.111" = type { %"class.std::__uniq_ptr_impl.112" }
%"class.std::__uniq_ptr_impl.112" = type { %"class.std::tuple.113" }
%"class.std::tuple.113" = type { %"struct.std::_Tuple_impl.114" }
%"struct.std::_Tuple_impl.114" = type { %"struct.std::_Head_base.119" }
%"struct.std::_Head_base.119" = type { %"class.clang::FileSystemStatCache"* }
%"class.clang::FileSystemStatCache" = type opaque
%"class.llvm::BumpPtrAllocatorImpl" = type { i8*, i8*, %"class.llvm::SmallVector.80", %"class.llvm::SmallVector.85", i64, i64 }
%"class.llvm::SmallVector.80" = type { %"class.llvm::SmallVectorImpl.81", %"struct.llvm::SmallVectorStorage.84" }
%"class.llvm::SmallVectorImpl.81" = type { %"class.llvm::SmallVectorTemplateBase.82" }
%"class.llvm::SmallVectorTemplateBase.82" = type { %"class.llvm::SmallVectorTemplateCommon.83" }
%"class.llvm::SmallVectorTemplateCommon.83" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.84" = type { [32 x i8] }
%"class.llvm::SmallVector.85" = type { %"class.llvm::SmallVectorImpl.86" }
%"class.llvm::SmallVectorImpl.86" = type { %"class.llvm::SmallVectorTemplateBase.87" }
%"class.llvm::SmallVectorTemplateBase.87" = type { %"class.llvm::SmallVectorTemplateCommon.88" }
%"class.llvm::SmallVectorTemplateCommon.88" = type { %"class.llvm::SmallVectorBase" }
%"class.llvm::DenseMap.120" = type <{ %"class.llvm::DenseMapBase.121", %"struct.llvm::detail::DenseMapPair.122"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.121" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.122" = type { %"struct.std::pair.123" }
%"struct.std::pair.123" = type { %"class.clang::FileEntry"*, %"class.clang::SrcMgr::ContentCache"* }
%"class.clang::FileEntry" = type { %"class.std::__cxx11::basic_string", i64, i64, %"class.clang::DirectoryEntry"*, %"class.llvm::sys::fs::UniqueID", i32, i8, i8, %"class.std::unique_ptr.124", %"class.std::unique_ptr.134", %"class.llvm::Optional.102" }
%"class.clang::DirectoryEntry" = type { %"class.llvm::StringRef" }
%"class.llvm::StringRef" = type { i8*, i64 }
%"class.llvm::sys::fs::UniqueID" = type { i64, i64 }
%"class.std::unique_ptr.124" = type { %"struct.std::__uniq_ptr_data.125" }
%"struct.std::__uniq_ptr_data.125" = type { %"class.std::__uniq_ptr_impl.126" }
%"class.std::__uniq_ptr_impl.126" = type { %"class.std::tuple.127" }
%"class.std::tuple.127" = type { %"struct.std::_Tuple_impl.128" }
%"struct.std::_Tuple_impl.128" = type { %"struct.std::_Head_base.133" }
%"struct.std::_Head_base.133" = type { %"class.llvm::vfs::File"* }
%"class.llvm::vfs::File" = type { i32 (...)** }
%"class.std::unique_ptr.145" = type { %"struct.std::__uniq_ptr_data.146" }
%"struct.std::__uniq_ptr_data.146" = type { %"class.std::__uniq_ptr_impl.147" }
%"class.std::__uniq_ptr_impl.147" = type { %"class.std::tuple.148" }
%"class.std::tuple.148" = type { %"struct.std::_Tuple_impl.149" }
%"struct.std::_Tuple_impl.149" = type { %"struct.std::_Head_base.154" }
%"struct.std::_Head_base.154" = type { %"struct.clang::SourceManager::OverriddenFilesInfoTy"* }
%"struct.clang::SourceManager::OverriddenFilesInfoTy" = type { %"class.llvm::DenseMap.155", %"class.llvm::DenseSet" }
%"class.llvm::DenseMap.155" = type <{ %"class.llvm::DenseMapBase.156", %"struct.llvm::detail::DenseMapPair.157"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.156" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.157" = type { %"struct.std::pair.158" }
%"struct.std::pair.158" = type { %"class.clang::FileEntry"*, %"class.clang::FileEntry"* }
%"class.llvm::DenseSet" = type { %"class.llvm::detail::DenseSetImpl" }
%"class.llvm::detail::DenseSetImpl" = type { %"class.llvm::DenseMap.162" }
%"class.llvm::DenseMap.162" = type <{ %"class.llvm::DenseMapBase.163", %"class.llvm::detail::DenseSetPair"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.163" = type { %"class.llvm::DebugEpochBase" }
%"class.llvm::detail::DenseSetPair" = type { %"class.clang::FileEntry"* }
%"class.std::vector.165" = type { %"struct.std::_Vector_base.166" }
%"struct.std::_Vector_base.166" = type { %"struct.std::_Vector_base<clang::SrcMgr::ContentCache *, std::allocator<clang::SrcMgr::ContentCache *>>::_Vector_impl" }
%"struct.std::_Vector_base<clang::SrcMgr::ContentCache *, std::allocator<clang::SrcMgr::ContentCache *>>::_Vector_impl" = type { %"struct.std::_Vector_base<clang::SrcMgr::ContentCache *, std::allocator<clang::SrcMgr::ContentCache *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<clang::SrcMgr::ContentCache *, std::allocator<clang::SrcMgr::ContentCache *>>::_Vector_impl_data" = type { %"class.clang::SrcMgr::ContentCache"**, %"class.clang::SrcMgr::ContentCache"**, %"class.clang::SrcMgr::ContentCache"** }
%"class.llvm::SmallVector.170" = type { %"class.llvm::SmallVectorImpl.171" }
%"class.llvm::SmallVectorImpl.171" = type { %"class.llvm::SmallVectorTemplateBase.172" }
%"class.llvm::SmallVectorTemplateBase.172" = type { %"class.llvm::SmallVectorTemplateCommon.173" }
%"class.llvm::SmallVectorTemplateCommon.173" = type { %"class.llvm::SmallVectorBase" }
%"class.llvm::BitVector" = type <{ %"class.llvm::SmallVector.175", i32, [4 x i8] }>
%"class.llvm::SmallVector.175" = type { %"class.llvm::SmallVectorImpl.176", %"struct.llvm::SmallVectorStorage.179" }
%"class.llvm::SmallVectorImpl.176" = type { %"class.llvm::SmallVectorTemplateBase.177" }
%"class.llvm::SmallVectorTemplateBase.177" = type { %"class.llvm::SmallVectorTemplateCommon.178" }
%"class.llvm::SmallVectorTemplateCommon.178" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.179" = type { [48 x i8] }
%"class.clang::ExternalSLocEntrySource" = type { i32 (...)** }
%"class.std::unique_ptr.180" = type { %"struct.std::__uniq_ptr_data.181" }
%"struct.std::__uniq_ptr_data.181" = type { %"class.std::__uniq_ptr_impl.182" }
%"class.std::__uniq_ptr_impl.182" = type { %"class.std::tuple.183" }
%"class.std::tuple.183" = type { %"struct.std::_Tuple_impl.184" }
%"struct.std::_Tuple_impl.184" = type { %"struct.std::_Head_base.189" }
%"struct.std::_Head_base.189" = type { %"class.clang::LineTableInfo"* }
%"class.clang::LineTableInfo" = type opaque
%"class.clang::SrcMgr::ContentCache" = type <{ %"class.std::unique_ptr.134", %"class.clang::FileEntry"*, %"class.clang::FileEntry"*, %"class.llvm::StringRef", %"class.clang::SrcMgr::LineOffsetMapping", i8, [7 x i8] }>
%"class.clang::SrcMgr::LineOffsetMapping" = type { i32* }
%"class.clang::FileID" = type { i32 }
%"class.llvm::DenseMap.190" = type <{ %"class.llvm::DenseMapBase.191", %"struct.llvm::detail::DenseMapPair.192"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.191" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.192" = type opaque
%"class.llvm::DenseMap.194" = type <{ %"class.llvm::DenseMapBase.195", %"struct.llvm::detail::DenseMapPair.196"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.195" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.196" = type opaque
%"class.clang::InBeforeInTUCacheEntry" = type { %"class.clang::FileID", %"class.clang::FileID", i8, %"class.clang::FileID", i32, i32 }
%"class.std::unique_ptr.134" = type { %"struct.std::__uniq_ptr_data.135" }
%"struct.std::__uniq_ptr_data.135" = type { %"class.std::__uniq_ptr_impl.136" }
%"class.std::__uniq_ptr_impl.136" = type { %"class.std::tuple.137" }
%"class.std::tuple.137" = type { %"struct.std::_Tuple_impl.138" }
%"struct.std::_Tuple_impl.138" = type { %"struct.std::_Head_base.143" }
%"struct.std::_Head_base.143" = type { %"class.llvm::MemoryBuffer"* }
%"class.llvm::MemoryBuffer" = type { i32 (...)**, i8*, i8* }
%"class.std::unique_ptr.198" = type { %"struct.std::__uniq_ptr_data.199" }
%"struct.std::__uniq_ptr_data.199" = type { %"class.std::__uniq_ptr_impl.200" }
%"class.std::__uniq_ptr_impl.200" = type { %"class.std::tuple.201" }
%"class.std::tuple.201" = type { %"struct.std::_Tuple_impl.202" }
%"struct.std::_Tuple_impl.202" = type { %"struct.std::_Head_base.207" }
%"struct.std::_Head_base.207" = type { %"class.clang::SrcMgr::ContentCache"* }
%"class.std::unique_ptr.208" = type { %"struct.std::__uniq_ptr_data.209" }
%"struct.std::__uniq_ptr_data.209" = type { %"class.std::__uniq_ptr_impl.210" }
%"class.std::__uniq_ptr_impl.210" = type { %"class.std::tuple.211" }
%"class.std::tuple.211" = type { %"struct.std::_Tuple_impl.212" }
%"struct.std::_Tuple_impl.212" = type { %"struct.std::_Head_base.217" }
%"struct.std::_Head_base.217" = type { %"class.clang::SrcMgr::SLocEntry"* }
%"class.clang::SrcMgr::SLocEntry" = type { i32, %union.anon.218 }
%union.anon.218 = type { %"class.clang::SrcMgr::FileInfo" }
%"class.clang::SrcMgr::FileInfo" = type { %"class.clang::SourceLocation", i32, %"class.llvm::PointerIntPair.219" }
%"class.llvm::PointerIntPair.219" = type { i64 }
%"class.llvm::DenseMap.220" = type <{ %"class.llvm::DenseMapBase.221", %"struct.llvm::detail::DenseMapPair.222"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.221" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.222" = type opaque
%"class.llvm::SmallVector.224" = type { %"class.llvm::SmallVectorImpl.225", %"struct.llvm::SmallVectorStorage.228" }
%"class.llvm::SmallVectorImpl.225" = type { %"class.llvm::SmallVectorTemplateBase.226" }
%"class.llvm::SmallVectorTemplateBase.226" = type { %"class.llvm::SmallVectorTemplateCommon.227" }
%"class.llvm::SmallVectorTemplateCommon.227" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.228" = type { [96 x i8] }
%"class.clang::SyclOptReportHandler" = type { %"class.llvm::DenseMap.229" }
%"class.llvm::DenseMap.229" = type <{ %"class.llvm::DenseMapBase.230", %"struct.llvm::detail::DenseMapPair.231"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.230" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.231" = type { %"struct.std::pair.232" }
%"struct.std::pair.232" = type { %"class.clang::FunctionDecl"*, %"class.llvm::SmallVector.268" }
%"class.clang::FunctionDecl" = type { %"class.clang::DeclaratorDecl.base", %"class.clang::DeclContext", %"class.clang::Redeclarable", %"class.clang::ParmVarDecl"**, %union.anon.258, i32, %"class.clang::SourceLocation", %"class.llvm::PointerUnion.260", %"class.clang::DeclarationNameLoc" }
%"class.clang::DeclaratorDecl.base" = type <{ %"class.clang::ValueDecl", %"class.llvm::PointerUnion.240", %"class.clang::SourceLocation" }>
%"class.clang::ValueDecl" = type { %"class.clang::NamedDecl", %"class.clang::QualType" }
%"class.clang::NamedDecl" = type { %"class.clang::Decl.base", %"class.clang::DeclarationName" }
%"class.clang::Decl.base" = type <{ i32 (...)**, %"class.llvm::PointerIntPair.235", %"class.llvm::PointerUnion", %"class.clang::SourceLocation", i32, i8 }>
%"class.clang::DeclarationName" = type { i64 }
%"class.clang::QualType" = type { %"class.llvm::PointerIntPair.239" }
%"class.llvm::PointerIntPair.239" = type { i64 }
%"class.llvm::PointerUnion.240" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.241" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.241" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.242" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.242" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.243" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.243" = type { %"class.llvm::PointerIntPair.244" }
%"class.llvm::PointerIntPair.244" = type { i64 }
%"class.clang::Redeclarable" = type { %"class.clang::Redeclarable<clang::FunctionDecl>::DeclLink", %"class.clang::FunctionDecl"* }
%"class.clang::Redeclarable<clang::FunctionDecl>::DeclLink" = type { %"class.llvm::PointerUnion.246" }
%"class.llvm::PointerUnion.246" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.247" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.247" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.248" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.248" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.249" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.249" = type { %"class.llvm::PointerIntPair.250" }
%"class.llvm::PointerIntPair.250" = type { i64 }
%"class.clang::ParmVarDecl" = type { %"class.clang::VarDecl.base", [4 x i8] }
%"class.clang::VarDecl.base" = type <{ %"class.clang::DeclaratorDecl.base", [4 x i8], %"class.clang::Redeclarable.251", %"class.llvm::PointerUnion.252", %union.anon.257 }>
%"class.clang::Redeclarable.251" = type { %"class.clang::Redeclarable<clang::VarDecl>::DeclLink", %"class.clang::VarDecl"* }
%"class.clang::Redeclarable<clang::VarDecl>::DeclLink" = type { %"class.llvm::PointerUnion.246" }
%"class.clang::VarDecl" = type <{ %"class.clang::DeclaratorDecl.base", [4 x i8], %"class.clang::Redeclarable.251", %"class.llvm::PointerUnion.252", %union.anon.257, [4 x i8] }>
%"class.llvm::PointerUnion.252" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.253" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.253" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.254" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.254" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.255" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.255" = type { %"class.llvm::PointerIntPair.256" }
%"class.llvm::PointerIntPair.256" = type { i64 }
%union.anon.257 = type { i32 }
%union.anon.258 = type { %"struct.clang::LazyOffsetPtr" }
%"struct.clang::LazyOffsetPtr" = type { i64 }
%"class.llvm::PointerUnion.260" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.261" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.261" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.262" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.262" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.263" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.263" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.264" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.264" = type { %"class.llvm::pointer_union_detail::PointerUnionMembers.265" }
%"class.llvm::pointer_union_detail::PointerUnionMembers.265" = type { %"class.llvm::PointerIntPair.266" }
%"class.llvm::PointerIntPair.266" = type { i64 }
%"class.clang::DeclarationNameLoc" = type { %union.anon.267 }
%union.anon.267 = type { %"struct.clang::DeclarationNameLoc::NT" }
%"struct.clang::DeclarationNameLoc::NT" = type { %"class.clang::TypeSourceInfo"* }
%"class.clang::TypeSourceInfo" = type { %"class.clang::QualType" }
%"class.llvm::SmallVector.268" = type { %"class.llvm::SmallVectorImpl.269", %"struct.llvm::SmallVectorStorage.272" }
%"class.llvm::SmallVectorImpl.269" = type { %"class.llvm::SmallVectorTemplateBase.270" }
%"class.llvm::SmallVectorTemplateBase.270" = type { %"class.llvm::SmallVectorTemplateCommon.271" }
%"class.llvm::SmallVectorTemplateCommon.271" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.272" = type { [136 x i8] }
%"class.std::__cxx11::list" = type { %"class.std::__cxx11::_List_base" }
%"class.std::__cxx11::_List_base" = type { %"struct.std::__cxx11::_List_base<clang::DiagnosticsEngine::DiagState, std::allocator<clang::DiagnosticsEngine::DiagState>>::_List_impl" }
%"struct.std::__cxx11::_List_base<clang::DiagnosticsEngine::DiagState, std::allocator<clang::DiagnosticsEngine::DiagState>>::_List_impl" = type { %"struct.std::__detail::_List_node_header" }
%"struct.std::__detail::_List_node_header" = type { %"struct.std::__detail::_List_node_base", i64 }
%"struct.std::__detail::_List_node_base" = type { %"struct.std::__detail::_List_node_base"*, %"struct.std::__detail::_List_node_base"* }
%"class.clang::DiagnosticsEngine::DiagStateMap" = type <{ %"class.std::map.277", %"class.clang::DiagnosticsEngine::DiagState"*, %"class.clang::DiagnosticsEngine::DiagState"*, %"class.clang::SourceLocation", [4 x i8] }>
%"class.std::map.277" = type { %"class.std::_Rb_tree.278" }
%"class.std::_Rb_tree.278" = type { %"struct.std::_Rb_tree<clang::FileID, std::pair<const clang::FileID, clang::DiagnosticsEngine::DiagStateMap::File>, std::_Select1st<std::pair<const clang::FileID, clang::DiagnosticsEngine::DiagStateMap::File>>, std::less<clang::FileID>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<clang::FileID, std::pair<const clang::FileID, clang::DiagnosticsEngine::DiagStateMap::File>, std::_Select1st<std::pair<const clang::FileID, clang::DiagnosticsEngine::DiagStateMap::File>>, std::less<clang::FileID>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare.282", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare.282" = type { %"struct.std::less.283" }
%"struct.std::less.283" = type { i8 }
%"class.clang::DiagnosticsEngine::DiagState" = type { %"class.llvm::DenseMap.285", i8, i32 }
%"class.llvm::DenseMap.285" = type <{ %"class.llvm::DenseMapBase.286", %"struct.llvm::detail::DenseMapPair.287"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.286" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.287" = type { %"struct.std::pair.2446" }
%"struct.std::pair.2446" = type { i32, %"class.clang::DiagnosticMapping" }
%"class.clang::DiagnosticMapping" = type { i8, [3 x i8] }
%"class.std::vector.289" = type { %"struct.std::_Vector_base.290" }
%"struct.std::_Vector_base.290" = type { %"struct.std::_Vector_base<clang::DiagnosticsEngine::DiagState *, std::allocator<clang::DiagnosticsEngine::DiagState *>>::_Vector_impl" }
%"struct.std::_Vector_base<clang::DiagnosticsEngine::DiagState *, std::allocator<clang::DiagnosticsEngine::DiagState *>>::_Vector_impl" = type { %"struct.std::_Vector_base<clang::DiagnosticsEngine::DiagState *, std::allocator<clang::DiagnosticsEngine::DiagState *>>::_Vector_impl_data" }
%"struct.std::_Vector_base<clang::DiagnosticsEngine::DiagState *, std::allocator<clang::DiagnosticsEngine::DiagState *>>::_Vector_impl_data" = type { %"class.clang::DiagnosticsEngine::DiagState"**, %"class.clang::DiagnosticsEngine::DiagState"**, %"class.clang::DiagnosticsEngine::DiagState"** }
%"class.llvm::ArrayRef" = type { %"struct.std::pair.294"*, i64 }
%"struct.std::pair.294" = type { i32, i64 }
%"class.llvm::SmallVectorImpl.297" = type { %"class.llvm::SmallVectorTemplateBase.298" }
%"class.llvm::SmallVectorTemplateBase.298" = type { %"class.llvm::SmallVectorTemplateCommon.299" }
%"class.llvm::SmallVectorTemplateCommon.299" = type { %"class.llvm::SmallVectorBase.300" }
%"class.llvm::SmallVectorBase.300" = type { i8*, i64, i64 }
%"class.llvm::ArrayRef.301" = type { i64*, i64 }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { i8* }
%union.anon = type { i64, [8 x i8] }
%"class.clang::OpenMPOptReportHandler" = type { %"class.llvm::DenseMap.302" }
%"class.llvm::DenseMap.302" = type <{ %"class.llvm::DenseMapBase.303", %"struct.llvm::detail::DenseMapPair.304"*, i32, i32, i32, [4 x i8] }>
%"class.llvm::DenseMapBase.303" = type { %"class.llvm::DebugEpochBase" }
%"struct.llvm::detail::DenseMapPair.304" = type { %"struct.std::pair.305" }
%"struct.std::pair.305" = type { %"class.clang::FunctionDecl"*, %"class.llvm::SmallVector.308" }
%"class.llvm::SmallVector.308" = type { %"class.llvm::SmallVectorImpl.309", %"struct.llvm::SmallVectorStorage.312" }
%"class.llvm::SmallVectorImpl.309" = type { %"class.llvm::SmallVectorTemplateBase.310" }
%"class.llvm::SmallVectorTemplateBase.310" = type { %"class.llvm::SmallVectorTemplateCommon.311" }
%"class.llvm::SmallVectorTemplateCommon.311" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.312" = type { [40 x i8] }
%"struct.clang::DiagnosticStorage" = type { i8, [10 x i8], [10 x i64], [10 x %"class.std::__cxx11::basic_string"], %"class.llvm::SmallVector.314", %"class.llvm::SmallVector.319" }
%"class.llvm::SmallVector.314" = type { %"class.llvm::SmallVectorImpl.315", %"struct.llvm::SmallVectorStorage.318" }
%"class.llvm::SmallVectorImpl.315" = type { %"class.llvm::SmallVectorTemplateBase.316" }
%"class.llvm::SmallVectorTemplateBase.316" = type { %"class.llvm::SmallVectorTemplateCommon.317" }
%"class.llvm::SmallVectorTemplateCommon.317" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.318" = type { [96 x i8] }
%"class.llvm::SmallVector.319" = type { %"class.llvm::SmallVectorImpl.320", %"struct.llvm::SmallVectorStorage.323" }
%"class.llvm::SmallVectorImpl.320" = type { %"class.llvm::SmallVectorTemplateBase.321" }
%"class.llvm::SmallVectorTemplateBase.321" = type { %"class.llvm::SmallVectorTemplateCommon.322" }
%"class.llvm::SmallVectorTemplateCommon.322" = type { %"class.llvm::SmallVectorBase" }
%"struct.llvm::SmallVectorStorage.323" = type { [384 x i8] }
%"class.llvm::PointerIntPair.1446" = type { i64 }

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

define i32 @foo(%"class.clang::Scope"* (i64)* %fp_ptr, i64 %inp) {
for.body70.preheader:
  br label %for.body70

for.body70:                                       ; preds = %cleanup, %for.body70.preheader
  %InnermostTemplateScope.0359 = phi %"class.clang::Scope"* [ %InnermostTemplateScope.1, %cleanup ], [ null, %for.body70.preheader ]
  br i1 false, label %cleanup, label %if.end74

if.end74:                                         ; preds = %for.body70
  %call.i = call %"class.clang::Scope"* %fp_ptr(i64 0)
  %0 = getelementptr inbounds %"class.clang::Scope", %"class.clang::Scope"* %call.i, i64 0, i32 13, i32 0, i32 0, i32 2
  br i1 false, label %for.cond.cleanup80, label %for.body81

for.cond.cleanup80:                               ; preds = %if.end74
  br label %cleanup

for.body81:                                       ; preds = %if.end74
  br label %if.then84

if.then84:                                        ; preds = %for.body81
  %1 = load i8**, i8*** %0, align 8
  br label %if.then.i.i335

if.then.i.i335:                                   ; preds = %if.then84
  %add.ptr.i.i334 = getelementptr inbounds i8*, i8** %1, i64 undef
  br label %if.end19.i.i

if.end19.i.i:                                     ; preds = %if.then.i.i335
  br label %if.then22.i.i

if.then22.i.i:                                    ; preds = %if.end19.i.i
  %notconst = bitcast i8** %add.ptr.i.i334 to %"class.clang::NamedDecl"**
  store %"class.clang::NamedDecl"* null, %"class.clang::NamedDecl"** %notconst, align 8
  ret i32 0

cleanup:                                          ; preds = %for.cond.cleanup80, %for.body70
  %InnermostTemplateScope.1 = phi %"class.clang::Scope"* [ %call.i, %for.cond.cleanup80 ], [ null, %for.body70 ]
  br label %for.body70
}

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
