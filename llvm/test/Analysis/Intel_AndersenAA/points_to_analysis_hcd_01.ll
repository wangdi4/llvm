; CMPLRLLVM-30376 & CMPLRLLVM-30436: This test verifies that Andersens
; analysis shouldn't crash due to collapsing nodes in mapping (offline
; cycles) data that is collected by HCD.

; RUN: opt < %s  -passes='require<anders-aa>' -disable-output 2>/dev/null

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.config_s = type { i16, i16, i16, i16, ptr }
%struct.MYSQL = type { %struct.NET, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i64, i32, i64, i64, i32, i32, i32, i32, i32, %struct.st_mysql_options, i32, i32, i8, i8, [21 x i8], ptr, ptr, ptr, ptr, ptr }
%struct.NET = type { ptr, ptr, ptr, ptr, ptr, i32, i64, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, i32, ptr, i8, i8, i8, i32, i8, [512 x i8], [6 x i8], ptr }
%struct.st_mysql_options = type { i32, i32, i32, i32, i32, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i8, i8, ptr, i8, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.LIST = type { ptr, ptr, ptr }
%struct.mysql_async_connect = type { ptr, ptr, ptr, ptr, ptr, i32, ptr, i64, i8, i64, ptr, [388 x i8], i32, ptr, ptr, ptr, i8, ptr, i8, ptr, i32, ptr, ptr }
%struct.z_stream_s = type { ptr, i32, i64, ptr, i32, i64, ptr, ptr, ptr, ptr, ptr, i32, i64, i64 }
%struct.internal_state = type { ptr, i32, ptr, i64, ptr, i64, i32, ptr, i64, i8, i32, i32, i32, i32, ptr, i64, ptr, ptr, i32, i32, i32, i32, i32, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [573 x %struct.ct_data_s], [61 x %struct.ct_data_s], [39 x %struct.ct_data_s], %struct.tree_desc_s, %struct.tree_desc_s, %struct.tree_desc_s, [16 x i16], [573 x i32], i32, i32, [573 x i8], ptr, i32, i32, ptr, i64, i64, i32, i32, i16, i32, i64 }
%struct.ct_data_s = type { %union.anon.4374, %union.anon.4374 }
%union.anon.4374 = type { i16 }
%struct.tree_desc_s = type { ptr, i32, ptr }

@configuration_table = internal unnamed_addr constant [10 x %struct.config_s] [%struct.config_s { i16 0, i16 0, i16 0, i16 0, ptr @deflate_stored }, %struct.config_s { i16 4, i16 4, i16 8, i16 4, ptr undef }, %struct.config_s { i16 4, i16 5, i16 16, i16 8, ptr undef }, %struct.config_s { i16 4, i16 6, i16 32, i16 32, ptr undef }, %struct.config_s { i16 4, i16 4, i16 16, i16 16, ptr undef }, %struct.config_s { i16 8, i16 16, i16 32, i16 32, ptr undef }, %struct.config_s { i16 8, i16 16, i16 128, i16 128, ptr undef }, %struct.config_s { i16 8, i16 32, i16 128, i16 256, ptr undef }, %struct.config_s { i16 32, i16 128, i16 258, i16 1024, ptr undef }, %struct.config_s { i16 32, i16 258, i16 258, i16 4096, ptr undef }], align 16

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #1

define internal void @set_mysql_extended_error(ptr %arg, ...) unnamed_addr {
bb:
  %i = getelementptr inbounds %struct.MYSQL, ptr %arg, i64 0, i32 36
  %i1 = load ptr, ptr %i, align 8
  br i1 undef, label %bb2, label %bb7

bb2:                                              ; preds = %bb
  %i3 = call ptr @calloc()
  br label %bb4

bb4:                                              ; preds = %bb2
  %i5 = getelementptr inbounds i8, ptr %i3, i64 32
  ret void

bb6:                                              ; No predecessors!
  store ptr %i5, ptr %i, align 8
  br label %bb7

bb7:                                              ; preds = %bb6, %bb
  %i8 = phi ptr [ %i5, %bb6 ], [ %i1, %bb ]
  ret void
}

define internal fastcc void @end_server(ptr %arg) unnamed_addr personality ptr undef {
bb:
  %i = getelementptr inbounds %struct.MYSQL, ptr %arg, i64 0, i32 32
  br label %bb1

bb1:                                              ; preds = %bb9, %bb
  %i2 = phi ptr [ %i10, %bb9 ], [ undef, %bb ]
  %i3 = phi ptr [ %i2, %bb9 ], [ null, %bb ]
  %i4 = getelementptr inbounds %struct.LIST, ptr %i2, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  ret void

bb6:                                              ; No predecessors!
  store ptr %i5, ptr %i, align 8
  ret void

bb7:                                              ; No predecessors!
  %i8 = getelementptr inbounds %struct.LIST, ptr %i2, i64 0, i32 1
  store ptr %i3, ptr %i8, align 8
  br label %bb9

bb9:                                              ; preds = %bb7
  %i10 = load ptr, ptr %i, align 8
  br label %bb1
}

define internal i1 @cli_advanced_command() {
bb:
  ret i1 undef
}

define internal fastcc i1 @mysql_reconnect() unnamed_addr personality ptr undef {
bb:
  %i = alloca %struct.MYSQL, align 8
  ret i1 undef

bb1:                                              ; No predecessors!
  call fastcc void @end_server(ptr %i)
  ret i1 undef

bb2:                                              ; No predecessors!
  call void (ptr, ...) @set_mysql_extended_error(ptr %i)
  ret i1 undef
}

define internal i32 @_ZL19csm_parse_handshakeP19mysql_async_connect() {
bb:
  store ptr @_ZL17csm_establish_sslP19mysql_async_connect, ptr undef, align 8
  ret i32 undef
}

define internal i32 @_ZL17csm_establish_sslP19mysql_async_connect(ptr %arg) {
bb:
  %i = getelementptr inbounds %struct.mysql_async_connect, ptr %arg, i64 0, i32 0
  %i1 = load ptr, ptr %i, align 8
  br label %bb2

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds %struct.MYSQL, ptr %i1, i64 0, i32 36
  %i4 = load ptr, ptr %i3, align 8
  br label %bb5

bb5:                                              ; preds = %bb2
  br label %bb17

bb6:                                              ; No predecessors!
  %i7 = call ptr @calloc()
  br label %bb8

bb8:                                              ; preds = %bb6
  %i9 = getelementptr inbounds i8, ptr %i7, i64 32
  br label %bb10

bb10:                                             ; preds = %bb8
  %i11 = call ptr @calloc()
  br label %bb12

bb12:                                             ; preds = %bb10
  %i13 = getelementptr inbounds i8, ptr %i11, i64 32
  br label %bb14

bb14:                                             ; preds = %bb12
  %i15 = getelementptr inbounds i8, ptr %i9, i64 104
  %i16 = bitcast ptr %i15 to ptr
  store ptr %i13, ptr %i16, align 8
  store ptr %i9, ptr %i3, align 8
  br label %bb17

bb17:                                             ; preds = %bb14, %bb5
  %i18 = phi ptr [ %i9, %bb14 ], [ %i4, %bb5 ]
  br label %bb19

bb19:                                             ; preds = %bb17
  br label %bb20

bb20:                                             ; preds = %bb19
  ret i32 undef

bb21:                                             ; No predecessors!
  %i22 = call ptr @malloc()
  br label %bb23

bb23:                                             ; preds = %bb21
  %i24 = getelementptr inbounds i8, ptr %i22, i64 32
  %i25 = getelementptr inbounds %struct.mysql_async_connect, ptr %arg, i64 0, i32 15
  store ptr %i24, ptr %i25, align 8
  ret i32 undef

bb26:                                             ; No predecessors!
  %i27 = getelementptr inbounds %struct.mysql_async_connect, ptr %arg, i64 0, i32 13
  %i28 = load ptr, ptr %i27, align 8
  %i29 = getelementptr inbounds %struct.mysql_async_connect, ptr %arg, i64 0, i32 15
  store ptr %i28, ptr %i29, align 8
  ret i32 undef

bb30:                                             ; No predecessors!
  %i31 = getelementptr inbounds %struct.MYSQL, ptr %i1, i64 0, i32 0, i32 1
  %i32 = load ptr, ptr %i31, align 8
  br label %bb33

bb33:                                             ; preds = %bb30
  %i34 = call fastcc i1 @_Z16net_write_packetP3NETPKhm(ptr %i32, i64 undef)
  ret i32 undef

bb35:                                             ; No predecessors!
  call void (ptr, ...) @set_mysql_extended_error(ptr %i1)
  ret i32 undef
}

declare i32 @_ZL16csm_authenticateP19mysql_async_connect()

define internal fastcc i1 @_Z16net_write_packetP3NETPKhm(ptr %arg, i64 %arg1) unnamed_addr {
bb:
  %i = call ptr @malloc()
  br label %bb2

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds i8, ptr %i, i64 32
  %i4 = getelementptr inbounds i8, ptr %i3, i64 7
  tail call void @llvm.memcpy.p0.p0.i64(ptr %i4, ptr %arg, i64 %arg1, i1 false)
  %i5 = call fastcc i1 @_Z11my_compressP22mysql_compress_contextPhPmS2_(ptr %i4)
  ret i1 undef
}

declare ptr @malloc() local_unnamed_addr

declare void @SSL_CTX_free() local_unnamed_addr

define internal fastcc i1 @_Z11my_compressP22mysql_compress_contextPhPmS2_(ptr %arg) unnamed_addr {
bb:
  %i = alloca %struct.z_stream_s, align 8
  ret i1 undef

bb1:                                              ; No predecessors!
  %i2 = getelementptr inbounds %struct.z_stream_s, ptr %i, i64 0, i32 0
  store ptr %arg, ptr %i2, align 8
  ret i1 undef

bb3:                                              ; No predecessors!
  %i4 = call fastcc i32 @deflate(ptr %i)
  ret i1 undef
}

declare ptr @calloc() local_unnamed_addr

define internal i32 @deflate_stored(ptr %arg) {
bb:
  %i = getelementptr inbounds %struct.internal_state, ptr %arg, i64 0, i32 0
  %i1 = getelementptr inbounds %struct.internal_state, ptr %arg, i64 0, i32 14
  ret i32 undef

bb2:                                              ; No predecessors!
  %i3 = load ptr, ptr %i, align 8
  %i4 = getelementptr inbounds %struct.z_stream_s, ptr %i3, i64 0, i32 7
  %i5 = load ptr, ptr %i4, align 8
  ret i32 undef

bb6:                                              ; No predecessors!
  %i7 = getelementptr inbounds %struct.z_stream_s, ptr %i3, i64 0, i32 3
  %i8 = load ptr, ptr %i7, align 8
  %i9 = getelementptr inbounds %struct.internal_state, ptr %i5, i64 0, i32 4
  %i10 = load ptr, ptr %i9, align 8
  tail call void @llvm.memcpy.p0.p0.i64(ptr %i8, ptr %i10, i64 undef, i1 false)
  %i11 = load ptr, ptr %i7, align 8
  %i12 = getelementptr inbounds i8, ptr %i11, i64 undef
  store ptr %i12, ptr %i7, align 8
  ret i32 undef

bb13:                                             ; No predecessors!
  %i14 = load ptr, ptr %i, align 8
  %i15 = getelementptr inbounds %struct.z_stream_s, ptr %i14, i64 0, i32 3
  %i16 = load ptr, ptr %i15, align 8
  br i1 undef, label %bb25, label %bb17

bb17:                                             ; preds = %bb13
  %i18 = getelementptr inbounds %struct.z_stream_s, ptr %i14, i64 0, i32 0
  %i19 = load ptr, ptr %i18, align 8
  tail call void @llvm.memcpy.p0.p0.i64(ptr %i16, ptr %i19, i64 undef, i1 false)
  switch i32 undef, label %bb20 [
  ]

bb20:                                             ; preds = %bb17
  %i21 = load ptr, ptr %i18, align 8
  %i22 = getelementptr inbounds i8, ptr %i21, i64 undef
  store ptr %i22, ptr %i18, align 8
  %i23 = getelementptr inbounds %struct.z_stream_s, ptr %i14, i64 0, i32 2
  %i24 = add i64 undef, undef
  store i64 %i24, ptr %i23, align 8
  br label %bb25

bb25:                                             ; preds = %bb20, %bb13
  %i26 = phi ptr [ %i16, %bb13 ], [ undef, %bb20 ]
  %i27 = phi ptr [ %i14, %bb13 ], [ undef, %bb20 ]
  %i28 = getelementptr inbounds %struct.z_stream_s, ptr %i27, i64 0, i32 3
  %i29 = getelementptr inbounds i8, ptr %i26, i64 undef
  store ptr %i29, ptr %i28, align 8
  ret i32 undef

bb30:                                             ; No predecessors!
  %i31 = load ptr, ptr %i, align 8
  ret i32 undef

bb32:                                             ; No predecessors!
  %i33 = load ptr, ptr %i1, align 8
  %i34 = getelementptr inbounds i8, ptr %i33, i64 undef
  %i35 = getelementptr inbounds %struct.z_stream_s, ptr %i31, i64 0, i32 0
  %i36 = load ptr, ptr %i35, align 8
  tail call void @llvm.memcpy.p0.p0.i64(ptr %i34, ptr %i36, i64 undef, i1 false)
  switch i32 undef, label %bb37 [
  ]

bb37:                                             ; preds = %bb32
  %i38 = load ptr, ptr %i35, align 8
  %i39 = getelementptr inbounds i8, ptr %i38, i64 undef
  store ptr %i39, ptr %i35, align 8
  ret i32 undef
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.usub.sat.i32(i32, i32) #2

define internal fastcc i32 @deflate(ptr %arg) unnamed_addr {
bb:
  %i = getelementptr inbounds %struct.z_stream_s, ptr %arg, i64 0, i32 7
  %i1 = load ptr, ptr %i, align 8
  ret i32 undef

bb2:                                              ; No predecessors!
  %i3 = call i32 @deflate_stored(ptr %i1)
  ret i32 undef
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.ctlz.i32(i32, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <4 x i32> @llvm.usub.sat.v4i32(<4 x i32>, <4 x i32>) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.cttz.i64(i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }
