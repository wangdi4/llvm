; CMPLRLLVM-30376 & CMPLRLLVM-30436: This test verifies that Andersens
; analysis shouldn't crash due to collapsing nodes in mapping (offline
; cycles) data that is collected by HCD.

; RUN: opt < %s  -passes='require<anders-aa>' -disable-output 2>/dev/null

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.config_s = type { i16, i16, i16, i16, i32 (%struct.internal_state*, i32)* }
%struct.internal_state = type { %struct.z_stream_s*, i32, i8*, i64, i8*, i64, i32, %struct.gz_header_s*, i64, i8, i32, i32, i32, i32, i8*, i64, i16*, i16*, i32, i32, i32, i32, i32, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [573 x %struct.ct_data_s], [61 x %struct.ct_data_s], [39 x %struct.ct_data_s], %struct.tree_desc_s, %struct.tree_desc_s, %struct.tree_desc_s, [16 x i16], [573 x i32], i32, i32, [573 x i8], i8*, i32, i32, i16*, i64, i64, i32, i32, i16, i32, i64 }
%struct.z_stream_s = type { i8*, i32, i64, i8*, i32, i64, i8*, %struct.internal_state*, i8* (i8*, i32, i32)*, void (i8*, i8*)*, i8*, i32, i64, i64 }
%struct.gz_header_s = type { i32, i64, i32, i32, i8*, i32, i32, i8*, i32, i8*, i32, i32, i32 }
%struct.ct_data_s = type { %union.anon.4374, %union.anon.4374 }
%union.anon.4374 = type { i16 }
%struct.tree_desc_s = type { %struct.ct_data_s*, i32, %struct.static_tree_desc_s* }
%struct.static_tree_desc_s = type { %struct.ct_data_s*, i32*, i32, i32, i32 }
%struct.MYSQL = type { %struct.NET, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct.CHARSET_INFO*, %struct.MYSQL_FIELD*, %struct.MEM_ROOT*, i64, i64, i64, i64, i64, i32, i64, i64, i32, i32, i32, i32, i32, %struct.st_mysql_options, i32, i32, i8, i8, [21 x i8], %struct.LIST*, %struct.MYSQL_METHODS*, i8*, i8*, i8* }
%struct.NET = type { %struct.Vio*, i8*, i8*, i8*, i8*, i32, i64, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, i32, i32*, i8, i8, i8, i32, i8, [512 x i8], [6 x i8], i8* }
%struct.Vio = type <{ %struct.MYSQL_SOCKET, i8, [3 x i8], i32, i32, i32, i32, i8, [3 x i8], %struct.sockaddr_storage, %struct.sockaddr_storage, i64, i8*, i8*, i8*, i64, %struct.__sigset_t, %"struct.std::_Rb_tree_key_compare", [256 x i8], [7 x i8], void (%struct.Vio*)*, i32 (%struct.Vio*)*, i64 (%struct.Vio*, i8*, i64)*, i64 (%struct.Vio*, i8*, i64)*, i32 (%struct.Vio*, i32, i1)*, i32 (%struct.Vio*, i1)*, i32 (%struct.Vio*)*, i1 (%struct.Vio*, i8*, i16*, i64)*, void (%struct.Vio*, %struct.sockaddr_storage*)*, i1 (%struct.Vio*)*, i1 (%struct.Vio*)*, i32 (%struct.Vio*)*, i1 (%struct.Vio*)*, i1 (%struct.Vio*)*, i32 (%struct.Vio*, i32, i32)*, i1 (%struct.Vio*, %struct.sockaddr*, i32, i32)*, i8*, %struct.PSI_socket_locker*, %struct.PSI_socket_locker_state_v1, %struct.PSI_socket_locker*, %struct.PSI_socket_locker_state_v1, i1 (%struct.Vio*)*, i32 (%struct.Vio*, i1)*, i32 (%struct.Vio*, i1)*, i8, [7 x i8] }>
%struct.MYSQL_SOCKET = type { i32, %struct.PSI_socket* }
%struct.PSI_socket = type opaque
%struct.sockaddr_storage = type { i16, [118 x i8], i64 }
%struct.__sigset_t = type { [16 x i64] }
%"struct.std::_Rb_tree_key_compare" = type { %"class.std::ios_base::Init" }
%"class.std::ios_base::Init" = type { i8 }
%struct.sockaddr = type { i16, [14 x i8] }
%struct.PSI_socket_locker = type opaque
%struct.PSI_socket_locker_state_v1 = type { i32, %struct.PSI_socket*, %struct.PSI_thread*, i64, i64, i64 ()*, i32, i8*, i32, i8* }
%struct.PSI_thread = type opaque
%struct.CHARSET_INFO = type { i32, i32, i32, i32, i8*, i8*, i8*, i8*, %struct.Coll_param*, i8*, i8*, i8*, i8*, %struct.MY_UCA_INFO*, i16*, %struct.MY_UNI_IDX*, %struct.MY_UNICASE_INFO*, %struct.lex_state_maps_st*, i8*, i32, i8, i8, i32, i32, i32, i64, i64, i8, i8, i8, %struct.MY_CHARSET_HANDLER*, %struct.MY_COLLATION_HANDLER*, i32 }
%struct.Coll_param = type { %struct.Reorder_param*, i8, i32 }
%struct.Reorder_param = type { [4 x i32], [8 x %struct.Reorder_wt_rec], i32, i16 }
%struct.Reorder_wt_rec = type { %struct.Weight_boundary, %struct.Weight_boundary }
%struct.Weight_boundary = type { i16, i16 }
%struct.MY_UCA_INFO = type { i32, i64, i8*, i16**, i8, %"class.std::vector.3724"*, i8*, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i16, i16, i16 }
%"class.std::vector.3724" = type { %"struct.std::_Vector_base.3723" }
%"struct.std::_Vector_base.3723" = type { %"struct.std::_Vector_base<MY_CONTRACTION, std::allocator<MY_CONTRACTION>>::_Vector_impl" }
%"struct.std::_Vector_base<MY_CONTRACTION, std::allocator<MY_CONTRACTION>>::_Vector_impl" = type { %"struct.std::_Vector_base<MY_CONTRACTION, std::allocator<MY_CONTRACTION>>::_Vector_impl_data" }
%"struct.std::_Vector_base<MY_CONTRACTION, std::allocator<MY_CONTRACTION>>::_Vector_impl_data" = type { %struct.MY_CONTRACTION*, %struct.MY_CONTRACTION*, %struct.MY_CONTRACTION* }
%struct.MY_CONTRACTION = type { i64, %"class.std::vector.3724", %"class.std::vector.3724", [25 x i16], i8, i64 }
%struct.MY_UNI_IDX = type { i16, i16, i8* }
%struct.MY_UNICASE_INFO = type { i64, %struct.MY_UNICASE_CHARACTER** }
%struct.MY_UNICASE_CHARACTER = type { i32, i32, i32 }
%struct.lex_state_maps_st = type { [256 x i8], [256 x i8] }
%struct.MY_CHARSET_HANDLER = type { i1 (%struct.CHARSET_INFO*, %struct.MY_CHARSET_LOADER*)*, i32 (%struct.CHARSET_INFO*, i8*, i8*)*, i32 (%struct.CHARSET_INFO*, i32)*, i64 (%struct.CHARSET_INFO*, i8*, i8*)*, i64 (%struct.CHARSET_INFO*, i8*, i8*, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i8*, i64, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i8*)*, i32 (%struct.CHARSET_INFO*, i64*, i8*, i8*)*, i32 (%struct.CHARSET_INFO*, i64, i8*, i8*)*, i32 (%struct.CHARSET_INFO*, i32*, i8*, i8*)*, i64 (%struct.CHARSET_INFO*, i8*)*, i64 (%struct.CHARSET_INFO*, i8*)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i8*, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i8*, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i8*, ...)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i64)*, void (%struct.CHARSET_INFO*, i8*, i64, i32)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8**, i32*)*, double (%struct.CHARSET_INFO*, i8*, i64, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8**, i32*)*, i64 (%struct.CHARSET_INFO*, i8*, i8*, i32)* }
%struct.MY_CHARSET_LOADER = type { i32, [192 x i8], i8* (i64)*, i8* (i64)*, i8* (i8*, i64)*, void (i8*)*, void (i32, i32, ...)*, i32 (%struct.CHARSET_INFO*)* }
%struct.MY_COLLATION_HANDLER = type { i1 (%struct.CHARSET_INFO*, %struct.MY_CHARSET_LOADER*)*, void (%struct.CHARSET_INFO*)*, i32 (%struct.CHARSET_INFO*, i8*, i64, i8*, i64, i1)*, i32 (%struct.CHARSET_INFO*, i8*, i64, i8*, i64)*, i64 (%struct.CHARSET_INFO*, i8*, i64, i32, i8*, i64, i32)*, i64 (%struct.CHARSET_INFO*, i64)*, i1 (%struct.CHARSET_INFO*, i8*, i64, i8, i8, i8, i64, i8*, i8*, i64*, i64*)*, i32 (%struct.CHARSET_INFO*, i8*, i8*, i8*, i8*, i32, i32, i32)*, i32 (%struct.CHARSET_INFO*, i8*, i8*)*, i32 (%struct.CHARSET_INFO*, i8*, i64, i8*, i64, %struct.MY_UNICASE_CHARACTER*, i32)*, void (%struct.CHARSET_INFO*, i8*, i64, i64*, i64*)*, i1 (%struct.CHARSET_INFO*, i8*, i64)* }
%struct.MYSQL_FIELD = type { i8*, i8*, i8*, i8*, i8*, i8*, i8*, i64, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8* }
%struct.MEM_ROOT = type <{ %"struct.MEM_ROOT::Block"*, i8*, i8*, i64, i64, i64, i64, i8, [7 x i8], void ()*, i32, [4 x i8] }>
%"struct.MEM_ROOT::Block" = type { %"struct.MEM_ROOT::Block"* }
%struct.st_mysql_options = type { i32, i32, i32, i32, i32, i64, i8*, i8*, i8*, i8*, i8*, %struct.Init_commands_array*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i64, i8, i8, i8*, i8, i32 (i8**, i8*, i8*)*, i32 (i8*, i8*, i32)*, void (i8*)*, i32 (i8*, i8*, i32)*, i8*, %struct.st_mysql_options_extention* }
%struct.Init_commands_array = type { %class.Prealloced_array.base, [4 x i8] }
%class.Prealloced_array.base = type <{ i64, i64, [40 x i8], i8**, i32 }>
%struct.st_mysql_options_extention = type { i8*, i8*, i8*, i8*, %struct.My_hash*, i8*, i64, i8, i8, i8*, i64, i32, i32, i32, i8*, i8*, i32, i32, i8, i8* }
%struct.My_hash = type { %class.malloc_unordered_map }
%class.malloc_unordered_map = type { %"class.std::unordered_map.3429" }
%"class.std::unordered_map.3429" = type { %"class.std::_Hashtable.3428" }
%"class.std::_Hashtable.3428" = type { %"struct.std::__detail::_Hashtable_alloc", %"struct.std::__detail::_Hash_node_base"**, i64, %"struct.std::__detail::_Hash_node_base", i64, %"struct.std::__detail::_Prime_rehash_policy", %"struct.std::__detail::_Hash_node_base"* }
%"struct.std::__detail::_Hashtable_alloc" = type { %"struct.std::atomic" }
%"struct.std::atomic" = type { %"struct.std::__atomic_base" }
%"struct.std::__atomic_base" = type { i32 }
%"struct.std::__detail::_Hash_node_base" = type { %"struct.std::__detail::_Hash_node_base"* }
%"struct.std::__detail::_Prime_rehash_policy" = type { float, i64 }
%struct.LIST = type { %struct.LIST*, %struct.LIST*, i8* }
%struct.MYSQL_METHODS = type { i1 (%struct.MYSQL*)*, i1 (%struct.MYSQL*, i32, i8*, i64, i8*, i64, i1, %struct.MYSQL_STMT*)*, %struct.MYSQL_DATA* (%struct.MYSQL*, %struct.MYSQL_FIELD*, i32)*, %struct.MYSQL_RES* (%struct.MYSQL*)*, void (i64*, i8**, i32)*, void (%struct.MYSQL*, i1)*, i32 (%struct.MYSQL*)*, %struct.MYSQL_FIELD* (%struct.MYSQL*)*, i1 (%struct.MYSQL*, %struct.MYSQL_STMT*)*, i32 (%struct.MYSQL_STMT*)*, i32 (%struct.MYSQL_STMT*)*, i32 (%struct.MYSQL*, i8**)*, i8* (%struct.MYSQL*)*, i1 (%struct.MYSQL*)*, i32 (%struct.MYSQL_STMT*)*, void (%struct.MYSQL_DATA*)*, i32 (%struct.MYSQL*)*, i32 (%struct.MYSQL*, i32, i8*, i64, i8*, i64, i1, %struct.MYSQL_STMT*, i8*)*, i32 (%struct.MYSQL*, %struct.MYSQL_FIELD*, i32, %struct.MYSQL_DATA**)*, i32 (%struct.MYSQL*, i1)*, i32 (%struct.MYSQL*)*, i32 (%struct.MYSQL*, i64*)* }
%struct.MYSQL_STMT = type { %struct.MEM_ROOT*, %struct.LIST, %struct.MYSQL*, %struct.MYSQL_BIND*, %struct.MYSQL_BIND*, %struct.MYSQL_FIELD*, %struct.MYSQL_DATA, %struct.MYSQL_ROWS*, i32 (%struct.MYSQL_STMT*, i8**)*, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, [512 x i8], [6 x i8], i8, i8, i8, i8, i8, %struct.MYSQL_STMT_EXT* }
%struct.MYSQL_BIND = type { i64*, i8*, i8*, i8*, i8*, void (%struct.NET*, %struct.MYSQL_BIND*)*, void (%struct.MYSQL_BIND*, %struct.MYSQL_FIELD*, i8**)*, void (%struct.MYSQL_BIND*, %struct.MYSQL_FIELD*, i8**)*, i64, i64, i64, i32, i32, i32, i8, i8, i8, i8, i8* }
%struct.MYSQL_DATA = type { %struct.MYSQL_ROWS*, %struct.MEM_ROOT*, i64, i32 }
%struct.MYSQL_ROWS = type { %struct.MYSQL_ROWS*, i8**, i64 }
%struct.MYSQL_STMT_EXT = type { %struct.MEM_ROOT }
%struct.MYSQL_RES = type { i64, %struct.MYSQL_FIELD*, %struct.MYSQL_DATA*, %struct.MYSQL_ROWS*, i64*, %struct.MYSQL*, %struct.MYSQL_METHODS*, i8**, i8**, %struct.MEM_ROOT*, i32, i32, i8, i8, i32, i8* }
%struct.mysql_async_connect = type { %struct.MYSQL*, i8*, i8*, i8*, i8*, i32, i8*, i64, i8, i64, i8*, [388 x i8], i32, i8*, i8*, i8*, i8, %struct.mysql_async_auth*, i8, i8**, i32, %struct.ssl_st*, i32 (%struct.mysql_async_connect*)* }
%struct.mysql_async_auth = type { %struct.MYSQL*, i8, i8*, i32, i8*, i8*, i8*, %struct.auth_plugin_t*, %struct.MCPVIO_EXT, i64, i32, i8*, i32, i32, i32 (%struct.mysql_async_auth*)* }
%struct.auth_plugin_t = type { i32, i32, i8*, i8*, i8*, [3 x i32], i8*, i8*, i32 (i8*, i64, i32, %struct.__va_list_tag*)*, i32 ()*, i32 (i8*, i8*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, %struct.MYSQL*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, %struct.MYSQL*, i32*)* }
%struct.__va_list_tag = type { i32, i32, i8*, i8* }
%struct.MYSQL_PLUGIN_VIO = type { i32 (%struct.MYSQL_PLUGIN_VIO*, i8**)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8*, i32)*, void (%struct.MYSQL_PLUGIN_VIO*, %"struct.std::pair.60"*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8**, i32*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8*, i32, i32*)* }
%"struct.std::pair.60" = type { i32, i32 }
%struct.MCPVIO_EXT = type { i32 (%struct.MYSQL_PLUGIN_VIO*, i8**)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8*, i32)*, void (%struct.MYSQL_PLUGIN_VIO*, %"struct.std::pair.60"*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8**, i32*)*, i32 (%struct.MYSQL_PLUGIN_VIO*, i8*, i32, i32*)*, %struct.MYSQL*, %struct.auth_plugin_t*, i8*, %struct.anon, i32, i32, i32, i32 }
%struct.anon = type { i8*, i32 }
%struct.ssl_st = type opaque

@configuration_table = internal unnamed_addr constant [10 x %struct.config_s] [%struct.config_s { i16 0, i16 0, i16 0, i16 0, i32 (%struct.internal_state*, i32)* bitcast (i32 (%struct.internal_state*)* @deflate_stored to i32 (%struct.internal_state*, i32)*) }, %struct.config_s { i16 4, i16 4, i16 8, i16 4, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 4, i16 5, i16 16, i16 8, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 4, i16 6, i16 32, i16 32, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 4, i16 4, i16 16, i16 16, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 8, i16 16, i16 32, i16 32, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 8, i16 16, i16 128, i16 128, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 8, i16 32, i16 128, i16 256, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 32, i16 128, i16 258, i16 1024, i32 (%struct.internal_state*, i32)* undef }, %struct.config_s { i16 32, i16 258, i16 258, i16 4096, i32 (%struct.internal_state*, i32)* undef }], align 16

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata)

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_start(i8*)

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_end(i8*)

define internal void @set_mysql_extended_error(%struct.MYSQL* %0, ...) unnamed_addr {
  %2 = getelementptr inbounds %struct.MYSQL, %struct.MYSQL* %0, i64 0, i32 36
  %3 = load i8*, i8** %2, align 8
  br i1 undef, label %4, label %9

4:                                                ; preds = %1
  %5 = call i8* @calloc()
  br label %6

6:                                                ; preds = %4
  %7 = getelementptr inbounds i8, i8* %5, i64 32
  ret void

8:                                                ; No predecessors!
  store i8* %7, i8** %2, align 8
  br label %9

9:                                                ; preds = %8, %1
  %10 = phi i8* [ %7, %8 ], [ %3, %1 ]
  ret void
}

define internal fastcc void @end_server(%struct.MYSQL* %0) unnamed_addr personality i32 (...)* undef {
  %2 = getelementptr inbounds %struct.MYSQL, %struct.MYSQL* %0, i64 0, i32 32
  br label %3

3:                                                ; preds = %11, %1
  %4 = phi %struct.LIST* [ %12, %11 ], [ undef, %1 ]
  %5 = phi %struct.LIST* [ %4, %11 ], [ null, %1 ]
  %6 = getelementptr inbounds %struct.LIST, %struct.LIST* %4, i64 0, i32 1
  %7 = load %struct.LIST*, %struct.LIST** %6, align 8
  ret void

8:                                                ; No predecessors!
  store %struct.LIST* %7, %struct.LIST** %2, align 8
  ret void

9:                                                ; No predecessors!
  %10 = getelementptr inbounds %struct.LIST, %struct.LIST* %4, i64 0, i32 1
  store %struct.LIST* %5, %struct.LIST** %10, align 8
  br label %11

11:                                               ; preds = %9
  %12 = load %struct.LIST*, %struct.LIST** %2, align 8
  br label %3
}

define internal i1 @cli_advanced_command() {
  ret i1 undef
}

define internal fastcc i1 @mysql_reconnect() unnamed_addr personality i32 (...)* undef {
  %1 = alloca %struct.MYSQL, align 8
  ret i1 undef

2:                                                ; No predecessors!
  call fastcc void @end_server(%struct.MYSQL* %1)
  ret i1 undef

3:                                                ; No predecessors!
  call void (%struct.MYSQL*, ...) @set_mysql_extended_error(%struct.MYSQL* %1)
  ret i1 undef
}

define internal i32 @_ZL19csm_parse_handshakeP19mysql_async_connect() {
  store i32 (%struct.mysql_async_connect*)* @_ZL17csm_establish_sslP19mysql_async_connect, i32 (%struct.mysql_async_connect*)** undef, align 8
  ret i32 undef
}

define internal i32 @_ZL17csm_establish_sslP19mysql_async_connect(%struct.mysql_async_connect* %0) {
  %2 = getelementptr inbounds %struct.mysql_async_connect, %struct.mysql_async_connect* %0, i64 0, i32 0
  %3 = load %struct.MYSQL*, %struct.MYSQL** %2, align 8
  br label %4

4:                                                ; preds = %1
  %5 = getelementptr inbounds %struct.MYSQL, %struct.MYSQL* %3, i64 0, i32 36
  %6 = load i8*, i8** %5, align 8
  br label %7

7:                                                ; preds = %4
  br label %19

8:                                                ; No predecessors!
  %9 = call i8* @calloc()
  br label %10

10:                                               ; preds = %8
  %11 = getelementptr inbounds i8, i8* %9, i64 32
  br label %12

12:                                               ; preds = %10
  %13 = call i8* @calloc()
  br label %14

14:                                               ; preds = %12
  %15 = getelementptr inbounds i8, i8* %13, i64 32
  br label %16

16:                                               ; preds = %14
  %17 = getelementptr inbounds i8, i8* %11, i64 104
  %18 = bitcast i8* %17 to i8**
  store i8* %15, i8** %18, align 8
  store i8* %11, i8** %5, align 8
  br label %19

19:                                               ; preds = %16, %7
  %20 = phi i8* [ %11, %16 ], [ %6, %7 ]
  br label %21

21:                                               ; preds = %19
  br label %22

22:                                               ; preds = %21
  ret i32 undef

23:                                               ; No predecessors!
  %24 = call i8* @malloc()
  br label %25

25:                                               ; preds = %23
  %26 = getelementptr inbounds i8, i8* %24, i64 32
  %27 = getelementptr inbounds %struct.mysql_async_connect, %struct.mysql_async_connect* %0, i64 0, i32 15
  store i8* %26, i8** %27, align 8
  ret i32 undef

28:                                               ; No predecessors!
  %29 = getelementptr inbounds %struct.mysql_async_connect, %struct.mysql_async_connect* %0, i64 0, i32 13
  %30 = load i8*, i8** %29, align 8
  %31 = getelementptr inbounds %struct.mysql_async_connect, %struct.mysql_async_connect* %0, i64 0, i32 15
  store i8* %30, i8** %31, align 8
  ret i32 undef

32:                                               ; No predecessors!
  %33 = getelementptr inbounds %struct.MYSQL, %struct.MYSQL* %3, i64 0, i32 0, i32 1
  %34 = load i8*, i8** %33, align 8
  br label %35

35:                                               ; preds = %32
  %36 = call fastcc i1 @_Z16net_write_packetP3NETPKhm(i8* %34, i64 undef)
  ret i32 undef

37:                                               ; No predecessors!
  call void (%struct.MYSQL*, ...) @set_mysql_extended_error(%struct.MYSQL* %3)
  ret i32 undef
}

declare i32 @_ZL16csm_authenticateP19mysql_async_connect()

define internal fastcc i1 @_Z16net_write_packetP3NETPKhm(i8* %0, i64 %1) unnamed_addr {
  %3 = call i8* @malloc()
  br label %4

4:                                                ; preds = %2
  %5 = getelementptr inbounds i8, i8* %3, i64 32
  %6 = getelementptr inbounds i8, i8* %5, i64 7
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %6, i8* %0, i64 %1, i1 false)
  %7 = call fastcc i1 @_Z11my_compressP22mysql_compress_contextPhPmS2_(i8* %6)
  ret i1 undef
}

declare i8* @malloc() local_unnamed_addr

declare void @SSL_CTX_free() local_unnamed_addr

define internal fastcc i1 @_Z11my_compressP22mysql_compress_contextPhPmS2_(i8* %0) unnamed_addr {
  %2 = alloca %struct.z_stream_s, align 8
  ret i1 undef

3:                                                ; No predecessors!
  %4 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %2, i64 0, i32 0
  store i8* %0, i8** %4, align 8
  ret i1 undef

5:                                                ; No predecessors!
  %6 = call fastcc i32 @deflate(%struct.z_stream_s* %2)
  ret i1 undef
}

declare i8* @calloc() local_unnamed_addr

define internal i32 @deflate_stored(%struct.internal_state* %0) {
  %2 = getelementptr inbounds %struct.internal_state, %struct.internal_state* %0, i64 0, i32 0
  %3 = getelementptr inbounds %struct.internal_state, %struct.internal_state* %0, i64 0, i32 14
  ret i32 undef

4:                                                ; No predecessors!
  %5 = load %struct.z_stream_s*, %struct.z_stream_s** %2, align 8
  %6 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %5, i64 0, i32 7
  %7 = load %struct.internal_state*, %struct.internal_state** %6, align 8
  ret i32 undef

8:                                                ; No predecessors!
  %9 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %5, i64 0, i32 3
  %10 = load i8*, i8** %9, align 8
  %11 = getelementptr inbounds %struct.internal_state, %struct.internal_state* %7, i64 0, i32 4
  %12 = load i8*, i8** %11, align 8
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %10, i8* %12, i64 undef, i1 false)
  %13 = load i8*, i8** %9, align 8
  %14 = getelementptr inbounds i8, i8* %13, i64 undef
  store i8* %14, i8** %9, align 8
  ret i32 undef

15:                                               ; No predecessors!
  %16 = load %struct.z_stream_s*, %struct.z_stream_s** %2, align 8
  %17 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %16, i64 0, i32 3
  %18 = load i8*, i8** %17, align 8
  br i1 undef, label %27, label %19

19:                                               ; preds = %15
  %20 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %16, i64 0, i32 0
  %21 = load i8*, i8** %20, align 8
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %18, i8* %21, i64 undef, i1 false)
  switch i32 undef, label %22 [
  ]

22:                                               ; preds = %19
  %23 = load i8*, i8** %20, align 8
  %24 = getelementptr inbounds i8, i8* %23, i64 undef
  store i8* %24, i8** %20, align 8
  %25 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %16, i64 0, i32 2
  %26 = add i64 undef, undef
  store i64 %26, i64* %25, align 8
  br label %27

27:                                               ; preds = %22, %15
  %28 = phi i8* [ %18, %15 ], [ undef, %22 ]
  %29 = phi %struct.z_stream_s* [ %16, %15 ], [ undef, %22 ]
  %30 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %29, i64 0, i32 3
  %31 = getelementptr inbounds i8, i8* %28, i64 undef
  store i8* %31, i8** %30, align 8
  ret i32 undef

32:                                               ; No predecessors!
  %33 = load %struct.z_stream_s*, %struct.z_stream_s** %2, align 8
  ret i32 undef

34:                                               ; No predecessors!
  %35 = load i8*, i8** %3, align 8
  %36 = getelementptr inbounds i8, i8* %35, i64 undef
  %37 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %33, i64 0, i32 0
  %38 = load i8*, i8** %37, align 8
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %36, i8* %38, i64 undef, i1 false)
  switch i32 undef, label %39 [
  ]

39:                                               ; preds = %34
  %40 = load i8*, i8** %37, align 8
  %41 = getelementptr inbounds i8, i8* %40, i64 undef
  store i8* %41, i8** %37, align 8
  ret i32 undef
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.usub.sat.i32(i32, i32)

define internal fastcc i32 @deflate(%struct.z_stream_s* %0) unnamed_addr {
  %2 = getelementptr inbounds %struct.z_stream_s, %struct.z_stream_s* %0, i64 0, i32 7
  %3 = load %struct.internal_state*, %struct.internal_state** %2, align 8
  ret i32 undef

4:                                                ; No predecessors!
  %5 = call i32 @deflate_stored(%struct.internal_state* %3)
  ret i32 undef
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.ctlz.i32(i32, i1 immarg)

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare <4 x i32> @llvm.usub.sat.v4i32(<4 x i32>, <4 x i32>)

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @llvm.cttz.i64(i64, i1 immarg)
