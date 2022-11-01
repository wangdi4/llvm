; RUN: opt -passes='function(gvn),print<inline-report>' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='function(gvn)' -inline-report=0xea86 -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that a call to __ctype_b_loc is deleted as dead code.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: COMPILE FUNC: wait_for_start_signal
; CHECK: EXTERN: fgetc
; CHECK: EXTERN: t_exit
; CHECK: EXTERN: fputc
; CHECK: llvm.lifetime.start.p0i8 {{.*}}Callee is intrinsic
; CHECK: EXTERN: __ctype_b_loc
; CHECK: EXTERN: report_info
; CHECK: DELETE: __ctype_b_loc {{.*}}Dead code
; CHECK: llvm.lifetime.end.p0i8 {{.*}}Callee is intrinsic
; CHECK: llvm.lifetime.end.p0i8 {{.*}}Callee is intrinsic
; CHECK: llvm.lifetime.end.p0i8 {{.*}}Callee is intrinsic
; CHECK: EXTERN: report_info

%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i64, i32 (i64, i32, i8**)*, {}*, i32 (i32, i8**)*, void ()* }
%struct.version_number = type { i8, i8, i8, i8 }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%struct.__va_list_tag = type { i32, i32, i8*, i8* }
%struct.THTestResults = type { i64, i64, i16, i64, i64, i64, i64, i8* }
%struct.FileDef = type { [128 x i8], i64, i8*, i64, i64 }
%struct.TCDef.7 = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef.7*, %struct.version_number, %struct.version_number, %struct.version_number, i64, i32 (i64, i32, i8**)*, i32 (%struct.TCDef.7**, i32, i8**)*, i32 (i32, i8**)*, void ()* }

@wait_for_start_signal.state = internal unnamed_addr global i1 false, align 4
@argca = internal unnamed_addr global i32 0, align 4
@argva = internal global [128 x i8*] zeroinitializer, align 16
@argv0_pgm = internal unnamed_addr global i8* null, align 8
@autogo = internal unnamed_addr global i1 false, align 4
@clbuf = internal global [1041 x i8] zeroinitializer, align 16
@inbuf = internal global [1041 x i8] zeroinitializer, align 16
@iterations = internal unnamed_addr global i64 0, align 8
@mem_base = internal unnamed_addr global i8* null, align 8
@mem_size = internal unnamed_addr global i1 false, align 8
@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@stdin = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@the_tcdef_ptr = internal unnamed_addr global %struct.TCDef.7* null, align 8

@t_run_test.info = internal global [64 x i8] zeroinitializer, align 16
@__const.t_run_test.CM_ONE = private unnamed_addr constant [3 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@__const.t_run_test.CM_THREE = private unnamed_addr constant [5 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\00\01", [2 x i8] c"\01\00", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@.str.7 = private unnamed_addr constant [19 x i8] c"\0AInput Stream EOF\0A\00", align 1
@.str.30 = private unnamed_addr constant [3 x i8] c"TH\00", align 1
@.str.33 = private unnamed_addr constant [2 x i8] c"n\00", align 1
@.str.35 = private unnamed_addr constant [2 x i8] c"g\00", align 1
@.str.36 = private unnamed_addr constant [2 x i8] c"i\00", align 1
@.str.37.63 = private unnamed_addr constant [4 x i8] c"dir\00", align 1
@.str.38 = private unnamed_addr constant [4 x i8] c"dnf\00", align 1
@.str.40 = private unnamed_addr constant [4 x i8] c"daf\00", align 1
@.str.43 = private unnamed_addr constant [4 x i8] c"mem\00", align 1
@.str.44 = private unnamed_addr constant [5 x i8] c"exit\00", align 1
@.str.45 = private unnamed_addr constant [3 x i8] c"cl\00", align 1
@.str.46 = private unnamed_addr constant [38 x i8] c"\0A-- Benchmark Command Line, ARGS:%d\0A\0A\00", align 1
@.str.49 = private unnamed_addr constant [4 x i8] c"zcl\00", align 1
@.str.52 = private unnamed_addr constant [4 x i8] c"ver\00", align 1
@.str.54 = private unnamed_addr constant [4 x i8] c"sff\00", align 1
@.str.57 = private unnamed_addr constant [2 x i8] c"h\00", align 1
@.str.58 = private unnamed_addr constant [5 x i8] c"help\00", align 1
@.str.59 = private unnamed_addr constant [2 x i8] c"?\00", align 1

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0
declare noundef i32 @fgetc(%struct._IO_FILE* nocapture noundef)
declare i32 @i_printf(i8* nocapture noundef readonly, %struct.__va_list_tag* noundef)
declare i32 @i_sprintf(i8* nocapture noundef, i8* nocapture noundef readonly, %struct.__va_list_tag* noundef)
declare i32 @i_sends(i8* noundef readonly)
declare i32 @i_putchar(i8 noundef signext)
declare i32 @i_write_con(i8* nocapture noundef, i64 noundef)
declare i64 @i_read_con(i8* nocapture noundef writeonly, i64 noundef returned)
declare i64 @i_con_chars_avail()
declare i64 @i_ticks_per_sec()
declare i64 @i_tick_granularity()
declare i8* @i_malloc(i64 noundef, i8* nocapture readnone, i32)
declare void @i_free(i8* nocapture noundef, i8* nocapture readnone, i32)
declare void @free(i8* allocptr nocapture noundef)
declare void @i_heap_reset()
declare void @i_signal_start()
declare void @t_printf(i8* nocapture noundef readonly, ...)
declare i64 @i_signal_finished()
declare void @i_exit(i32 noundef, i8* noundef, %struct.__va_list_tag* noundef)
declare %struct.FileDef* @i_get_file_def(i8* nocapture readnone)
declare %struct.FileDef* @i_get_file_num(i32)
declare i32 @i_send_buf_as_file(i8* noundef readonly, i64 noundef, i8* noundef)
declare i32 @i_report_results(%struct.THTestResults* nocapture noundef readonly, i16 noundef zeroext)
declare i32 @i_harness_poll()
declare void @t_exit(i32 noundef, i8* noundef, ...)
declare i16** @__ctype_b_loc()
declare void @report_info()
declare noundef i32 @fputc(i32 noundef, %struct._IO_FILE* nocapture noundef) #23

define i32 @wait_for_start_signal() unnamed_addr #21 {
bb:
  %i = alloca [16 x i8], align 16
  %i1 = load i1, i1* @autogo, align 4
  br i1 %i1, label %bb4, label %bb2

bb2:                                              ; preds = %bb
  %i3 = getelementptr inbounds [16 x i8], [16 x i8]* %i, i64 0, i64 0
  br label %bb7

bb4:                                              ; preds = %bb
  %i5 = load i1, i1* @wait_for_start_signal.state, align 4
  br i1 %i5, label %bb738, label %bb6

bb6:                                              ; preds = %bb4
  store i1 true, i1* @wait_for_start_signal.state, align 4
  br label %bb738

bb7:                                              ; preds = %bb734, %bb2
  br label %bb8

bb8:                                              ; preds = %bb30, %bb7
  %i9 = phi i32 [ 0, %bb7 ], [ %i31, %bb30 ]
  %i10 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !tbaa !6
  %i11 = tail call i32 @fgetc(%struct._IO_FILE* noundef %i10)
  %i12 = icmp eq i32 %i11, -1
  br i1 %i12, label %bb13, label %bb14

bb13:                                             ; preds = %bb8
  tail call void (i32, i8*, ...) @t_exit(i32 noundef 1, i8* noundef getelementptr inbounds ([19 x i8], [19 x i8]* @.str.7, i64 0, i64 0)) #24
  unreachable

bb14:                                             ; preds = %bb8
  %i15 = trunc i32 %i11 to i8
  %i16 = icmp sgt i8 %i15, 31
  br i1 %i16, label %bb17, label %bb30

bb17:                                             ; preds = %bb14
  %i18 = icmp ne i8 %i15, 127
  %i19 = icmp slt i32 %i9, 1040
  %i20 = select i1 %i18, i1 %i19, i1 false
  br i1 %i20, label %bb21, label %bb30

bb21:                                             ; preds = %bb17
  %i22 = add nsw i32 %i9, 1
  %i23 = sext i32 %i9 to i64
  %i24 = getelementptr inbounds [1041 x i8], [1041 x i8]* @inbuf, i64 0, i64 %i23, !intel-tbaa !10
  store i8 %i15, i8* %i24, align 1, !tbaa !10
  %i25 = sext i32 %i22 to i64
  %i26 = getelementptr inbounds [1041 x i8], [1041 x i8]* @inbuf, i64 0, i64 %i25, !intel-tbaa !10
  store i8 0, i8* %i26, align 1, !tbaa !10
  %i27 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8, !tbaa !6
  %i28 = zext i8 %i15 to i32
  %i29 = tail call i32 @fputc(i32 %i28, %struct._IO_FILE* %i27)
  br label %bb30

bb30:                                             ; preds = %bb21, %bb17, %bb14
  %i31 = phi i32 [ %i9, %bb14 ], [ %i9, %bb17 ], [ %i22, %bb21 ]
  switch i8 %i15, label %bb8 [
    i8 10, label %bb32
    i8 13, label %bb32
    i8 27, label %bb737
  ]

bb32:                                             ; preds = %bb30, %bb30
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %i3) #24
  %i33 = tail call i16** @__ctype_b_loc() #25
  %i34 = load i16*, i16** %i33, align 8, !tbaa !12
  br label %bb35

bb35:                                             ; preds = %bb35, %bb32
  %i36 = phi i8* [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @inbuf, i64 0, i64 0), %bb32 ], [ %i43, %bb35 ]
  %i37 = load i8, i8* %i36, align 1, !tbaa !14
  %i38 = sext i8 %i37 to i64
  %i39 = getelementptr inbounds i16, i16* %i34, i64 %i38
  %i40 = load i16, i16* %i39, align 2, !tbaa !15
  %i41 = and i16 %i40, 8192
  %i42 = icmp eq i16 %i41, 0
  %i43 = getelementptr inbounds i8, i8* %i36, i64 1
  br i1 %i42, label %bb44, label %bb35

bb44:                                             ; preds = %bb35
  %i45 = phi i8* [ %i36, %bb35 ]
  %i46 = phi i8 [ %i37, %bb35 ]
  %i47 = icmp eq i8 %i46, 0
  br i1 %i47, label %bb52, label %bb48

bb48:                                             ; preds = %bb44
  %i49 = add i8 %i46, -127
  %i50 = icmp ult i8 %i49, -94
  br i1 %i50, label %bb71, label %bb51

bb51:                                             ; preds = %bb48
  br label %bb53

bb52:                                             ; preds = %bb44
  store i8 0, i8* %i3, align 16, !tbaa !14
  br label %bb74

bb53:                                             ; preds = %bb62, %bb51
  %i54 = phi i8 [ %i65, %bb62 ], [ %i46, %bb51 ]
  %i55 = phi i32 [ %i58, %bb62 ], [ 0, %bb51 ]
  %i56 = phi i8* [ %i64, %bb62 ], [ %i45, %bb51 ]
  %i57 = phi i8* [ %i63, %bb62 ], [ %i3, %bb51 ]
  %i58 = add nuw nsw i32 %i55, 1
  %i59 = icmp ult i32 %i55, 15
  br i1 %i59, label %bb60, label %bb62

bb60:                                             ; preds = %bb53
  %i61 = getelementptr inbounds i8, i8* %i57, i64 1
  store i8 %i54, i8* %i57, align 1, !tbaa !14
  br label %bb62

bb62:                                             ; preds = %bb60, %bb53
  %i63 = phi i8* [ %i61, %bb60 ], [ %i57, %bb53 ]
  %i64 = getelementptr inbounds i8, i8* %i56, i64 1
  %i65 = load i8, i8* %i64, align 1, !tbaa !14
  %i66 = add i8 %i65, -127
  %i67 = icmp ult i8 %i66, -94
  br i1 %i67, label %bb68, label %bb53

bb68:                                             ; preds = %bb62
  %i69 = phi i8* [ %i63, %bb62 ]
  %i70 = phi i8* [ %i64, %bb62 ]
  br label %bb71

bb71:                                             ; preds = %bb68, %bb48
  %i72 = phi i8* [ %i3, %bb48 ], [ %i69, %bb68 ]
  %i73 = phi i8* [ %i45, %bb48 ], [ %i70, %bb68 ]
  store i8 0, i8* %i72, align 1, !tbaa !14
  br label %bb74

bb74:                                             ; preds = %bb71, %bb52
  %i75 = phi i8* [ null, %bb52 ], [ %i73, %bb71 ]
  %i76 = load i8, i8* %i3, align 16, !tbaa !14
  %i77 = sext i8 %i76 to i32
  %i78 = add i8 %i76, -97
  %i79 = icmp ult i8 %i78, 26
  %i80 = add nsw i32 %i77, -32
  %i81 = select i1 %i79, i32 %i80, i32 %i77
  %i82 = icmp eq i32 %i81, 78
  br i1 %i82, label %bb83, label %bb158

bb83:                                             ; preds = %bb74
  br label %bb84

bb84:                                             ; preds = %bb89, %bb83
  %i85 = phi i8 [ %i92, %bb89 ], [ %i76, %bb83 ]
  %i86 = phi i8* [ %i91, %bb89 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.33, i64 0, i64 0), %bb83 ]
  %i87 = phi i8* [ %i90, %bb89 ], [ %i3, %bb83 ]
  %i88 = icmp eq i8 %i85, 0
  br i1 %i88, label %bb105, label %bb89

bb89:                                             ; preds = %bb84
  %i90 = getelementptr inbounds i8, i8* %i87, i64 1
  %i91 = getelementptr inbounds i8, i8* %i86, i64 1
  %i92 = load i8, i8* %i90, align 1, !tbaa !14
  %i93 = sext i8 %i92 to i32
  %i94 = add i8 %i92, -97
  %i95 = icmp ult i8 %i94, 26
  %i96 = add nsw i32 %i93, -32
  %i97 = select i1 %i95, i32 %i96, i32 %i93
  %i98 = load i8, i8* %i91, align 1, !tbaa !14
  %i99 = sext i8 %i98 to i32
  %i100 = add i8 %i98, -97
  %i101 = icmp ult i8 %i100, 26
  %i102 = add nsw i32 %i99, -32
  %i103 = select i1 %i101, i32 %i102, i32 %i99
  %i104 = icmp eq i32 %i97, %i103
  br i1 %i104, label %bb84, label %bb157

bb105:                                            ; preds = %bb84
  %i106 = load i8, i8* %i75, align 1, !tbaa !14
  %i107 = icmp eq i8 %i106, 0
  br i1 %i107, label %bb158, label %bb108

bb108:                                            ; preds = %bb105
  br label %bb109

bb109:                                            ; preds = %bb114, %bb108
  %i110 = phi i8 [ %i116, %bb114 ], [ %i106, %bb108 ]
  %i111 = phi i8* [ %i115, %bb114 ], [ %i75, %bb108 ]
  %i112 = add i8 %i110, -48
  %i113 = icmp ult i8 %i112, 10
  br i1 %i113, label %bb118, label %bb114

bb114:                                            ; preds = %bb109
  %i115 = getelementptr inbounds i8, i8* %i111, i64 1
  %i116 = load i8, i8* %i115, align 1, !tbaa !14
  %i117 = icmp eq i8 %i116, 0
  br i1 %i117, label %bb156, label %bb109

bb118:                                            ; preds = %bb109
  %i119 = phi i8 [ %i110, %bb109 ]
  %i120 = phi i8* [ %i111, %bb109 ]
  %i121 = load i16*, i16** %i33, align 8, !tbaa !12
  %i122 = zext i8 %i119 to i64
  %i123 = getelementptr inbounds i16, i16* %i121, i64 %i122
  %i124 = load i16, i16* %i123, align 2, !tbaa !15
  %i125 = and i16 %i124, 2048
  %i126 = icmp eq i16 %i125, 0
  br i1 %i126, label %bb149, label %bb127

bb127:                                            ; preds = %bb118
  br label %bb128

bb128:                                            ; preds = %bb128, %bb127
  %i129 = phi i64 [ %i139, %bb128 ], [ %i122, %bb127 ]
  %i130 = phi i64 [ %i135, %bb128 ], [ 0, %bb127 ]
  %i131 = phi i32 [ %i137, %bb128 ], [ 0, %bb127 ]
  %i132 = phi i8* [ %i136, %bb128 ], [ %i120, %bb127 ]
  %i133 = mul i64 %i130, 10
  %i134 = add nsw i64 %i129, -48
  %i135 = add i64 %i134, %i133
  %i136 = getelementptr inbounds i8, i8* %i132, i64 1
  %i137 = add nuw nsw i32 %i131, 1
  %i138 = load i8, i8* %i136, align 1, !tbaa !14
  %i139 = sext i8 %i138 to i64
  %i140 = getelementptr inbounds i16, i16* %i121, i64 %i139
  %i141 = load i16, i16* %i140, align 2, !tbaa !15
  %i142 = and i16 %i141, 2048
  %i143 = icmp ne i16 %i142, 0
  %i144 = icmp ult i32 %i131, 9
  %i145 = select i1 %i143, i1 %i144, i1 false
  br i1 %i145, label %bb128, label %bb146

bb146:                                            ; preds = %bb128
  %i147 = phi i64 [ %i135, %bb128 ]
  %i148 = icmp eq i64 %i147, 0
  br i1 %i148, label %bb149, label %bb153

bb149:                                            ; preds = %bb146, %bb118
  %i150 = load %struct.TCDef.7*, %struct.TCDef.7** @the_tcdef_ptr, align 8, !tbaa !17
  %i151 = getelementptr inbounds %struct.TCDef.7, %struct.TCDef.7* %i150, i64 0, i32 10, !intel-tbaa !19
  %i152 = load i64, i64* %i151, align 8, !tbaa !19
  br label %bb153

bb153:                                            ; preds = %bb149, %bb146
  %i154 = phi i8* [ %i120, %bb149 ], [ %i120, %bb146 ]
  %i155 = phi i64 [ %i152, %bb149 ], [ %i147, %bb146 ]
  store i64 %i155, i64* @iterations, align 8, !tbaa !29
  br label %bb158

bb156:                                            ; preds = %bb114
  br label %bb158

bb157:                                            ; preds = %bb89
  br label %bb158

bb158:                                            ; preds = %bb157, %bb156, %bb153, %bb105, %bb74
  %i159 = phi i8* [ %i154, %bb153 ], [ null, %bb105 ], [ %i75, %bb74 ], [ null, %bb156 ], [ %i75, %bb157 ]
  %i160 = load i8, i8* %i3, align 16, !tbaa !14
  %i161 = sext i8 %i160 to i32
  %i162 = add i8 %i160, -97
  %i163 = icmp ult i8 %i162, 26
  %i164 = add nsw i32 %i161, -32
  %i165 = select i1 %i163, i32 %i164, i32 %i161
  %i166 = icmp eq i32 %i165, 71
  br i1 %i166, label %bb167, label %bb190

bb167:                                            ; preds = %bb158
  br label %bb168

bb168:                                            ; preds = %bb173, %bb167
  %i169 = phi i8 [ %i176, %bb173 ], [ %i160, %bb167 ]
  %i170 = phi i8* [ %i175, %bb173 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.35, i64 0, i64 0), %bb167 ]
  %i171 = phi i8* [ %i174, %bb173 ], [ %i3, %bb167 ]
  %i172 = icmp eq i8 %i169, 0
  br i1 %i172, label %bb736, label %bb173

bb173:                                            ; preds = %bb168
  %i174 = getelementptr inbounds i8, i8* %i171, i64 1
  %i175 = getelementptr inbounds i8, i8* %i170, i64 1
  %i176 = load i8, i8* %i174, align 1, !tbaa !14
  %i177 = sext i8 %i176 to i32
  %i178 = add i8 %i176, -97
  %i179 = icmp ult i8 %i178, 26
  %i180 = add nsw i32 %i177, -32
  %i181 = select i1 %i179, i32 %i180, i32 %i177
  %i182 = load i8, i8* %i175, align 1, !tbaa !14
  %i183 = sext i8 %i182 to i32
  %i184 = add i8 %i182, -97
  %i185 = icmp ult i8 %i184, 26
  %i186 = add nsw i32 %i183, -32
  %i187 = select i1 %i185, i32 %i186, i32 %i183
  %i188 = icmp eq i32 %i181, %i187
  br i1 %i188, label %bb168, label %bb189

bb189:                                            ; preds = %bb173
  br label %bb190

bb190:                                            ; preds = %bb189, %bb158
  %i191 = load i8, i8* %i3, align 16, !tbaa !14
  %i192 = sext i8 %i191 to i32
  %i193 = add i8 %i191, -97
  %i194 = icmp ult i8 %i193, 26
  %i195 = add nsw i32 %i192, -32
  %i196 = select i1 %i194, i32 %i195, i32 %i192
  %i197 = icmp eq i32 %i196, 73
  br i1 %i197, label %bb198, label %bb222

bb198:                                            ; preds = %bb190
  br label %bb199

bb199:                                            ; preds = %bb204, %bb198
  %i200 = phi i8 [ %i207, %bb204 ], [ %i191, %bb198 ]
  %i201 = phi i8* [ %i206, %bb204 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.36, i64 0, i64 0), %bb198 ]
  %i202 = phi i8* [ %i205, %bb204 ], [ %i3, %bb198 ]
  %i203 = icmp eq i8 %i200, 0
  br i1 %i203, label %bb220, label %bb204

bb204:                                            ; preds = %bb199
  %i205 = getelementptr inbounds i8, i8* %i202, i64 1
  %i206 = getelementptr inbounds i8, i8* %i201, i64 1
  %i207 = load i8, i8* %i205, align 1, !tbaa !14
  %i208 = sext i8 %i207 to i32
  %i209 = add i8 %i207, -97
  %i210 = icmp ult i8 %i209, 26
  %i211 = add nsw i32 %i208, -32
  %i212 = select i1 %i210, i32 %i211, i32 %i208
  %i213 = load i8, i8* %i206, align 1, !tbaa !14
  %i214 = sext i8 %i213 to i32
  %i215 = add i8 %i213, -97
  %i216 = icmp ult i8 %i215, 26
  %i217 = add nsw i32 %i214, -32
  %i218 = select i1 %i216, i32 %i217, i32 %i214
  %i219 = icmp eq i32 %i212, %i218
  br i1 %i219, label %bb199, label %bb221

bb220:                                            ; preds = %bb199
  tail call fastcc void @report_info()
  br label %bb222

bb221:                                            ; preds = %bb204
  br label %bb222

bb222:                                            ; preds = %bb221, %bb220, %bb190
  %i223 = load i8, i8* %i3, align 16, !tbaa !14
  %i224 = sext i8 %i223 to i32
  %i225 = add i8 %i223, -97
  %i226 = icmp ult i8 %i225, 26
  %i227 = add nsw i32 %i224, -32
  %i228 = select i1 %i226, i32 %i227, i32 %i224
  %i229 = icmp eq i32 %i228, 68
  br i1 %i229, label %bb230, label %bb254

bb230:                                            ; preds = %bb222
  br label %bb231

bb231:                                            ; preds = %bb236, %bb230
  %i232 = phi i8 [ %i239, %bb236 ], [ %i223, %bb230 ]
  %i233 = phi i8* [ %i238, %bb236 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.37.63, i64 0, i64 0), %bb230 ]
  %i234 = phi i8* [ %i237, %bb236 ], [ %i3, %bb230 ]
  %i235 = icmp eq i8 %i232, 0
  br i1 %i235, label %bb252, label %bb236

bb236:                                            ; preds = %bb231
  %i237 = getelementptr inbounds i8, i8* %i234, i64 1
  %i238 = getelementptr inbounds i8, i8* %i233, i64 1
  %i239 = load i8, i8* %i237, align 1, !tbaa !14
  %i240 = sext i8 %i239 to i32
  %i241 = add i8 %i239, -97
  %i242 = icmp ult i8 %i241, 26
  %i243 = add nsw i32 %i240, -32
  %i244 = select i1 %i242, i32 %i243, i32 %i240
  %i245 = load i8, i8* %i238, align 1, !tbaa !14
  %i246 = sext i8 %i245 to i32
  %i247 = add i8 %i245, -97
  %i248 = icmp ult i8 %i247, 26
  %i249 = add nsw i32 %i246, -32
  %i250 = select i1 %i248, i32 %i249, i32 %i246
  %i251 = icmp eq i32 %i244, %i250
  br i1 %i251, label %bb231, label %bb253

bb252:                                            ; preds = %bb231
  br label %bb254

bb253:                                            ; preds = %bb236
  br label %bb254

bb254:                                            ; preds = %bb253, %bb252, %bb222
  %i255 = load i8, i8* %i3, align 16, !tbaa !14
  %i256 = sext i8 %i255 to i32
  %i257 = add i8 %i255, -97
  %i258 = icmp ult i8 %i257, 26
  %i259 = add nsw i32 %i256, -32
  %i260 = select i1 %i258, i32 %i259, i32 %i256
  %i261 = icmp eq i32 %i260, 68
  br i1 %i261, label %bb262, label %bb286

bb262:                                            ; preds = %bb254
  br label %bb263

bb263:                                            ; preds = %bb268, %bb262
  %i264 = phi i8 [ %i271, %bb268 ], [ %i255, %bb262 ]
  %i265 = phi i8* [ %i270, %bb268 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.38, i64 0, i64 0), %bb262 ]
  %i266 = phi i8* [ %i269, %bb268 ], [ %i3, %bb262 ]
  %i267 = icmp eq i8 %i264, 0
  br i1 %i267, label %bb284, label %bb268

bb268:                                            ; preds = %bb263
  %i269 = getelementptr inbounds i8, i8* %i266, i64 1
  %i270 = getelementptr inbounds i8, i8* %i265, i64 1
  %i271 = load i8, i8* %i269, align 1, !tbaa !14
  %i272 = sext i8 %i271 to i32
  %i273 = add i8 %i271, -97
  %i274 = icmp ult i8 %i273, 26
  %i275 = add nsw i32 %i272, -32
  %i276 = select i1 %i274, i32 %i275, i32 %i272
  %i277 = load i8, i8* %i270, align 1, !tbaa !14
  %i278 = sext i8 %i277 to i32
  %i279 = add i8 %i277, -97
  %i280 = icmp ult i8 %i279, 26
  %i281 = add nsw i32 %i278, -32
  %i282 = select i1 %i280, i32 %i281, i32 %i278
  %i283 = icmp eq i32 %i276, %i282
  br i1 %i283, label %bb263, label %bb285

bb284:                                            ; preds = %bb263
  br label %bb286

bb285:                                            ; preds = %bb268
  br label %bb286

bb286:                                            ; preds = %bb285, %bb284, %bb254
  %i287 = load i8, i8* %i3, align 16, !tbaa !14
  %i288 = sext i8 %i287 to i32
  %i289 = add i8 %i287, -97
  %i290 = icmp ult i8 %i289, 26
  %i291 = add nsw i32 %i288, -32
  %i292 = select i1 %i290, i32 %i291, i32 %i288
  %i293 = icmp eq i32 %i292, 68
  br i1 %i293, label %bb294, label %bb318

bb294:                                            ; preds = %bb286
  br label %bb295

bb295:                                            ; preds = %bb300, %bb294
  %i296 = phi i8 [ %i303, %bb300 ], [ %i287, %bb294 ]
  %i297 = phi i8* [ %i302, %bb300 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.40, i64 0, i64 0), %bb294 ]
  %i298 = phi i8* [ %i301, %bb300 ], [ %i3, %bb294 ]
  %i299 = icmp eq i8 %i296, 0
  br i1 %i299, label %bb316, label %bb300

bb300:                                            ; preds = %bb295
  %i301 = getelementptr inbounds i8, i8* %i298, i64 1
  %i302 = getelementptr inbounds i8, i8* %i297, i64 1
  %i303 = load i8, i8* %i301, align 1, !tbaa !14
  %i304 = sext i8 %i303 to i32
  %i305 = add i8 %i303, -97
  %i306 = icmp ult i8 %i305, 26
  %i307 = add nsw i32 %i304, -32
  %i308 = select i1 %i306, i32 %i307, i32 %i304
  %i309 = load i8, i8* %i302, align 1, !tbaa !14
  %i310 = sext i8 %i309 to i32
  %i311 = add i8 %i309, -97
  %i312 = icmp ult i8 %i311, 26
  %i313 = add nsw i32 %i310, -32
  %i314 = select i1 %i312, i32 %i313, i32 %i310
  %i315 = icmp eq i32 %i308, %i314
  br i1 %i315, label %bb295, label %bb317

bb316:                                            ; preds = %bb295
  br label %bb318

bb317:                                            ; preds = %bb300
  br label %bb318

bb318:                                            ; preds = %bb317, %bb316, %bb286
  %i319 = load i8, i8* %i3, align 16, !tbaa !14
  %i320 = sext i8 %i319 to i32
  %i321 = add i8 %i319, -97
  %i322 = icmp ult i8 %i321, 26
  %i323 = add nsw i32 %i320, -32
  %i324 = select i1 %i322, i32 %i323, i32 %i320
  %i325 = icmp eq i32 %i324, 77
  br i1 %i325, label %bb326, label %bb355

bb326:                                            ; preds = %bb318
  br label %bb327

bb327:                                            ; preds = %bb332, %bb326
  %i328 = phi i8 [ %i335, %bb332 ], [ %i319, %bb326 ]
  %i329 = phi i8* [ %i334, %bb332 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.43, i64 0, i64 0), %bb326 ]
  %i330 = phi i8* [ %i333, %bb332 ], [ %i3, %bb326 ]
  %i331 = icmp eq i8 %i328, 0
  br i1 %i331, label %bb348, label %bb332

bb332:                                            ; preds = %bb327
  %i333 = getelementptr inbounds i8, i8* %i330, i64 1
  %i334 = getelementptr inbounds i8, i8* %i329, i64 1
  %i335 = load i8, i8* %i333, align 1, !tbaa !14
  %i336 = sext i8 %i335 to i32
  %i337 = add i8 %i335, -97
  %i338 = icmp ult i8 %i337, 26
  %i339 = add nsw i32 %i336, -32
  %i340 = select i1 %i338, i32 %i339, i32 %i336
  %i341 = load i8, i8* %i334, align 1, !tbaa !14
  %i342 = sext i8 %i341 to i32
  %i343 = add i8 %i341, -97
  %i344 = icmp ult i8 %i343, 26
  %i345 = add nsw i32 %i342, -32
  %i346 = select i1 %i344, i32 %i345, i32 %i342
  %i347 = icmp eq i32 %i340, %i346
  br i1 %i347, label %bb327, label %bb354

bb348:                                            ; preds = %bb327
  %i349 = load i8*, i8** @mem_base, align 8, !tbaa !30
  %i350 = load i1, i1* @mem_size, align 8
  %i351 = select i1 %i350, i64 4194304, i64 0
  %i352 = load i1, i1* @mem_size, align 8
  %i353 = select i1 %i352, i64 4194304, i64 0
  br label %bb355

bb354:                                            ; preds = %bb332
  br label %bb355

bb355:                                            ; preds = %bb354, %bb348, %bb318
  %i356 = load i8, i8* %i3, align 16, !tbaa !14
  %i357 = sext i8 %i356 to i32
  %i358 = add i8 %i356, -97
  %i359 = icmp ult i8 %i358, 26
  %i360 = add nsw i32 %i357, -32
  %i361 = select i1 %i359, i32 %i360, i32 %i357
  %i362 = icmp eq i32 %i361, 69
  br i1 %i362, label %bb363, label %bb386

bb363:                                            ; preds = %bb355
  br label %bb364

bb364:                                            ; preds = %bb369, %bb363
  %i365 = phi i8 [ %i372, %bb369 ], [ %i356, %bb363 ]
  %i366 = phi i8* [ %i371, %bb369 ], [ getelementptr inbounds ([5 x i8], [5 x i8]* @.str.44, i64 0, i64 0), %bb363 ]
  %i367 = phi i8* [ %i370, %bb369 ], [ %i3, %bb363 ]
  %i368 = icmp eq i8 %i365, 0
  br i1 %i368, label %bb735, label %bb369

bb369:                                            ; preds = %bb364
  %i370 = getelementptr inbounds i8, i8* %i367, i64 1
  %i371 = getelementptr inbounds i8, i8* %i366, i64 1
  %i372 = load i8, i8* %i370, align 1, !tbaa !14
  %i373 = sext i8 %i372 to i32
  %i374 = add i8 %i372, -97
  %i375 = icmp ult i8 %i374, 26
  %i376 = add nsw i32 %i373, -32
  %i377 = select i1 %i375, i32 %i376, i32 %i373
  %i378 = load i8, i8* %i371, align 1, !tbaa !14
  %i379 = sext i8 %i378 to i32
  %i380 = add i8 %i378, -97
  %i381 = icmp ult i8 %i380, 26
  %i382 = add nsw i32 %i379, -32
  %i383 = select i1 %i381, i32 %i382, i32 %i379
  %i384 = icmp eq i32 %i377, %i383
  br i1 %i384, label %bb364, label %bb385

bb385:                                            ; preds = %bb369
  br label %bb386

bb386:                                            ; preds = %bb385, %bb355
  %i387 = load i8, i8* %i3, align 16, !tbaa !14
  %i388 = sext i8 %i387 to i32
  %i389 = add i8 %i387, -97
  %i390 = icmp ult i8 %i389, 26
  %i391 = add nsw i32 %i388, -32
  %i392 = select i1 %i390, i32 %i391, i32 %i388
  %i393 = icmp eq i32 %i392, 67
  br i1 %i393, label %bb394, label %bb532

bb394:                                            ; preds = %bb386
  br label %bb395

bb395:                                            ; preds = %bb400, %bb394
  %i396 = phi i8 [ %i403, %bb400 ], [ %i387, %bb394 ]
  %i397 = phi i8* [ %i402, %bb400 ], [ getelementptr inbounds ([3 x i8], [3 x i8]* @.str.45, i64 0, i64 0), %bb394 ]
  %i398 = phi i8* [ %i401, %bb400 ], [ %i3, %bb394 ]
  %i399 = icmp eq i8 %i396, 0
  br i1 %i399, label %bb416, label %bb400

bb400:                                            ; preds = %bb395
  %i401 = getelementptr inbounds i8, i8* %i398, i64 1
  %i402 = getelementptr inbounds i8, i8* %i397, i64 1
  %i403 = load i8, i8* %i401, align 1, !tbaa !14
  %i404 = sext i8 %i403 to i32
  %i405 = add i8 %i403, -97
  %i406 = icmp ult i8 %i405, 26
  %i407 = add nsw i32 %i404, -32
  %i408 = select i1 %i406, i32 %i407, i32 %i404
  %i409 = load i8, i8* %i402, align 1, !tbaa !14
  %i410 = sext i8 %i409 to i32
  %i411 = add i8 %i409, -97
  %i412 = icmp ult i8 %i411, 26
  %i413 = add nsw i32 %i410, -32
  %i414 = select i1 %i412, i32 %i413, i32 %i410
  %i415 = icmp eq i32 %i408, %i414
  br i1 %i415, label %bb395, label %bb531

bb416:                                            ; preds = %bb395
  %i417 = icmp eq i8* %i159, null
  br i1 %i417, label %bb437, label %bb418

bb418:                                            ; preds = %bb416
  %i419 = load i8, i8* %i159, align 1, !tbaa !14
  %i420 = icmp eq i8 %i419, 0
  br i1 %i420, label %bb437, label %bb421

bb421:                                            ; preds = %bb418
  %i422 = tail call i16** @__ctype_b_loc() #25
  %i423 = load i16*, i16** %i422, align 8, !tbaa !12
  br label %bb428

bb424:                                            ; preds = %bb428
  %i425 = getelementptr inbounds i8, i8* %i429, i64 1
  %i426 = load i8, i8* %i425, align 1, !tbaa !14
  %i427 = icmp eq i8 %i426, 0
  br i1 %i427, label %bb436, label %bb428

bb428:                                            ; preds = %bb424, %bb421
  %i429 = phi i8* [ %i159, %bb421 ], [ %i425, %bb424 ]
  %i430 = phi i8 [ %i419, %bb421 ], [ %i426, %bb424 ]
  %i431 = sext i8 %i430 to i64
  %i432 = getelementptr inbounds i16, i16* %i423, i64 %i431
  %i433 = load i16, i16* %i432, align 2, !tbaa !15
  %i434 = and i16 %i433, 8192
  %i435 = icmp eq i16 %i434, 0
  br i1 %i435, label %bb451, label %bb424

bb436:                                            ; preds = %bb424
  br label %bb437

bb437:                                            ; preds = %bb436, %bb418, %bb416
  %i438 = load i32, i32* @argca, align 4, !tbaa !32
  %i439 = load i32, i32* @argca, align 4, !tbaa !32
  %i440 = icmp sgt i32 %i439, 0
  br i1 %i440, label %bb441, label %bb734

bb441:                                            ; preds = %bb437
  br label %bb442

bb442:                                            ; preds = %bb442, %bb441
  %i443 = phi i64 [ %i447, %bb442 ], [ 0, %bb441 ]
  %i444 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i64 0, i64 %i443, !intel-tbaa !34
  %i445 = load i8*, i8** %i444, align 8, !tbaa !34
  %i446 = trunc i64 %i443 to i32
  %i447 = add nuw nsw i64 %i443, 1
  %i448 = load i32, i32* @argca, align 4, !tbaa !32
  %i449 = sext i32 %i448 to i64
  %i450 = icmp slt i64 %i447, %i449
  br i1 %i450, label %bb442, label %bb732

bb451:                                            ; preds = %bb428
  store i32 1, i32* @argca, align 4, !tbaa !32
  %i452 = load i8*, i8** @argv0_pgm, align 8, !tbaa !30
  %i453 = icmp eq i8* %i452, null
  %i454 = select i1 %i453, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i64 0, i64 0), i8* %i452
  store i8* %i454, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i64 0, i64 0), align 16, !tbaa !34
  store i8* null, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i64 0, i64 1), align 8, !tbaa !34
  %i455 = icmp eq i8* %i159, null
  br i1 %i455, label %bb530, label %bb456

bb456:                                            ; preds = %bb451
  %i457 = load i8, i8* %i159, align 1, !tbaa !14
  %i458 = icmp eq i8 %i457, 0
  br i1 %i458, label %bb530, label %bb459

bb459:                                            ; preds = %bb456
  %i460 = load i8, i8* %i159, align 1, !tbaa !14
  %i461 = icmp eq i8 %i460, 0
  br i1 %i461, label %bb480, label %bb462

bb462:                                            ; preds = %bb459
  br label %bb463

bb463:                                            ; preds = %bb470, %bb462
  %i464 = phi i8 [ %i476, %bb470 ], [ %i460, %bb462 ]
  %i465 = phi i8* [ %i474, %bb470 ], [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i64 0, i64 0), %bb462 ]
  %i466 = phi i8* [ %i475, %bb470 ], [ %i159, %bb462 ]
  %i467 = phi i32 [ %i468, %bb470 ], [ 1041, %bb462 ]
  store i8 %i464, i8* %i465, align 1, !tbaa !14
  %i468 = add nsw i32 %i467, -1
  %i469 = icmp eq i32 %i468, 0
  br i1 %i469, label %bb478, label %bb470

bb470:                                            ; preds = %bb463
  %i471 = load i8, i8* %i466, align 1, !tbaa !14
  %i472 = icmp ne i8 %i471, 0
  %i473 = zext i1 %i472 to i64
  %i474 = getelementptr i8, i8* %i465, i64 %i473
  %i475 = getelementptr i8, i8* %i466, i64 %i473
  %i476 = load i8, i8* %i475, align 1, !tbaa !14
  %i477 = icmp eq i8 %i476, 0
  br i1 %i477, label %bb478, label %bb463

bb478:                                            ; preds = %bb470, %bb463
  %i479 = phi i8* [ %i474, %bb470 ], [ %i465, %bb463 ]
  br label %bb480

bb480:                                            ; preds = %bb478, %bb459
  %i481 = phi i8* [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i64 0, i64 0), %bb459 ], [ %i479, %bb478 ]
  store i8 0, i8* %i481, align 1, !tbaa !14
  %i482 = load i32, i32* @argca, align 4, !tbaa !32
  %i483 = sext i32 %i482 to i64
  br label %bb484

bb484:                                            ; preds = %bb522, %bb480
  %i485 = phi i64 [ %i503, %bb522 ], [ %i483, %bb480 ]
  %i486 = phi i8* [ %i524, %bb522 ], [ getelementptr inbounds ([1041 x i8], [1041 x i8]* @clbuf, i64 0, i64 0), %bb480 ]
  %i487 = load i16*, i16** %i33, align 8, !tbaa !12
  br label %bb488

bb488:                                            ; preds = %bb488, %bb484
  %i489 = phi i8* [ %i486, %bb484 ], [ %i496, %bb488 ]
  %i490 = load i8, i8* %i489, align 1, !tbaa !14
  %i491 = sext i8 %i490 to i64
  %i492 = getelementptr inbounds i16, i16* %i487, i64 %i491
  %i493 = load i16, i16* %i492, align 2, !tbaa !15
  %i494 = and i16 %i493, 8192
  %i495 = icmp eq i16 %i494, 0
  %i496 = getelementptr inbounds i8, i8* %i489, i64 1
  br i1 %i495, label %bb497, label %bb488

bb497:                                            ; preds = %bb488
  %i498 = phi i8* [ %i489, %bb488 ]
  %i499 = phi i8 [ %i490, %bb488 ]
  %i500 = icmp eq i8 %i499, 0
  br i1 %i500, label %bb525, label %bb501

bb501:                                            ; preds = %bb497
  %i502 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i64 0, i64 %i485, !intel-tbaa !34
  store i8* %i498, i8** %i502, align 8, !tbaa !34
  %i503 = add i64 %i485, 1
  %i504 = trunc i64 %i503 to i32
  store i32 %i504, i32* @argca, align 4, !tbaa !32
  %i505 = load i8, i8* %i498, align 1, !tbaa !14
  %i506 = icmp eq i8 %i505, 0
  br i1 %i506, label %bb522, label %bb507

bb507:                                            ; preds = %bb501
  br label %bb508

bb508:                                            ; preds = %bb516, %bb507
  %i509 = phi i8 [ %i518, %bb516 ], [ %i505, %bb507 ]
  %i510 = phi i8* [ %i517, %bb516 ], [ %i498, %bb507 ]
  %i511 = sext i8 %i509 to i64
  %i512 = getelementptr inbounds i16, i16* %i487, i64 %i511
  %i513 = load i16, i16* %i512, align 2, !tbaa !15
  %i514 = and i16 %i513, 8192
  %i515 = icmp eq i16 %i514, 0
  br i1 %i515, label %bb516, label %bb520

bb516:                                            ; preds = %bb508
  %i517 = getelementptr inbounds i8, i8* %i510, i64 1
  %i518 = load i8, i8* %i517, align 1, !tbaa !14
  %i519 = icmp eq i8 %i518, 0
  br i1 %i519, label %bb520, label %bb508

bb520:                                            ; preds = %bb516, %bb508
  %i521 = phi i8* [ %i517, %bb516 ], [ %i510, %bb508 ]
  br label %bb522

bb522:                                            ; preds = %bb520, %bb501
  %i523 = phi i8* [ %i498, %bb501 ], [ %i521, %bb520 ]
  %i524 = getelementptr inbounds i8, i8* %i523, i64 1
  store i8 0, i8* %i523, align 1, !tbaa !14
  br label %bb484

bb525:                                            ; preds = %bb497
  %i526 = phi i64 [ %i485, %bb497 ]
  %i527 = shl i64 %i526, 32
  %i528 = ashr exact i64 %i527, 32
  %i529 = getelementptr inbounds [128 x i8*], [128 x i8*]* @argva, i64 0, i64 %i528, !intel-tbaa !34
  store i8* null, i8** %i529, align 8, !tbaa !34
  br label %bb530

bb530:                                            ; preds = %bb525, %bb456, %bb451
  br label %bb734

bb531:                                            ; preds = %bb400
  br label %bb532

bb532:                                            ; preds = %bb531, %bb386
  %i533 = load i8, i8* %i3, align 16, !tbaa !14
  %i534 = sext i8 %i533 to i32
  %i535 = add i8 %i533, -97
  %i536 = icmp ult i8 %i535, 26
  %i537 = add nsw i32 %i534, -32
  %i538 = select i1 %i536, i32 %i537, i32 %i534
  %i539 = icmp eq i32 %i538, 90
  br i1 %i539, label %bb540, label %bb568

bb540:                                            ; preds = %bb532
  br label %bb541

bb541:                                            ; preds = %bb546, %bb540
  %i542 = phi i8 [ %i549, %bb546 ], [ %i533, %bb540 ]
  %i543 = phi i8* [ %i548, %bb546 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.49, i64 0, i64 0), %bb540 ]
  %i544 = phi i8* [ %i547, %bb546 ], [ %i3, %bb540 ]
  %i545 = icmp eq i8 %i542, 0
  br i1 %i545, label %bb562, label %bb546

bb546:                                            ; preds = %bb541
  %i547 = getelementptr inbounds i8, i8* %i544, i64 1
  %i548 = getelementptr inbounds i8, i8* %i543, i64 1
  %i549 = load i8, i8* %i547, align 1, !tbaa !14
  %i550 = sext i8 %i549 to i32
  %i551 = add i8 %i549, -97
  %i552 = icmp ult i8 %i551, 26
  %i553 = add nsw i32 %i550, -32
  %i554 = select i1 %i552, i32 %i553, i32 %i550
  %i555 = load i8, i8* %i548, align 1, !tbaa !14
  %i556 = sext i8 %i555 to i32
  %i557 = add i8 %i555, -97
  %i558 = icmp ult i8 %i557, 26
  %i559 = add nsw i32 %i556, -32
  %i560 = select i1 %i558, i32 %i559, i32 %i556
  %i561 = icmp eq i32 %i554, %i560
  br i1 %i561, label %bb541, label %bb567

bb562:                                            ; preds = %bb541
  %i563 = load i32, i32* @argca, align 4, !tbaa !32
  store i32 1, i32* @argca, align 4, !tbaa !32
  %i564 = load i8*, i8** @argv0_pgm, align 8, !tbaa !30
  %i565 = icmp eq i8* %i564, null
  %i566 = select i1 %i565, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.30, i64 0, i64 0), i8* %i564
  store i8* %i566, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i64 0, i64 0), align 16, !tbaa !34
  store i8* null, i8** getelementptr inbounds ([128 x i8*], [128 x i8*]* @argva, i64 0, i64 1), align 8, !tbaa !34
  br label %bb734

bb567:                                            ; preds = %bb546
  br label %bb568

bb568:                                            ; preds = %bb567, %bb532
  %i569 = load i8, i8* %i3, align 16, !tbaa !14
  %i570 = sext i8 %i569 to i32
  %i571 = add i8 %i569, -97
  %i572 = icmp ult i8 %i571, 26
  %i573 = add nsw i32 %i570, -32
  %i574 = select i1 %i572, i32 %i573, i32 %i570
  %i575 = icmp eq i32 %i574, 86
  br i1 %i575, label %bb576, label %bb604

bb576:                                            ; preds = %bb568
  br label %bb577

bb577:                                            ; preds = %bb582, %bb576
  %i578 = phi i8 [ %i585, %bb582 ], [ %i569, %bb576 ]
  %i579 = phi i8* [ %i584, %bb582 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.52, i64 0, i64 0), %bb576 ]
  %i580 = phi i8* [ %i583, %bb582 ], [ %i3, %bb576 ]
  %i581 = icmp eq i8 %i578, 0
  br i1 %i581, label %bb598, label %bb582

bb582:                                            ; preds = %bb577
  %i583 = getelementptr inbounds i8, i8* %i580, i64 1
  %i584 = getelementptr inbounds i8, i8* %i579, i64 1
  %i585 = load i8, i8* %i583, align 1, !tbaa !14
  %i586 = sext i8 %i585 to i32
  %i587 = add i8 %i585, -97
  %i588 = icmp ult i8 %i587, 26
  %i589 = add nsw i32 %i586, -32
  %i590 = select i1 %i588, i32 %i589, i32 %i586
  %i591 = load i8, i8* %i584, align 1, !tbaa !14
  %i592 = sext i8 %i591 to i32
  %i593 = add i8 %i591, -97
  %i594 = icmp ult i8 %i593, 26
  %i595 = add nsw i32 %i592, -32
  %i596 = select i1 %i594, i32 %i595, i32 %i592
  %i597 = icmp eq i32 %i590, %i596
  br i1 %i597, label %bb577, label %bb603

bb598:                                            ; preds = %bb577
  %i599 = load %struct.TCDef.7*, %struct.TCDef.7** @the_tcdef_ptr, align 8, !tbaa !17
  %i600 = getelementptr inbounds %struct.TCDef.7, %struct.TCDef.7* %i599, i64 0, i32 4, i64 0
  %i601 = load %struct.TCDef.7*, %struct.TCDef.7** @the_tcdef_ptr, align 8, !tbaa !17
  %i602 = getelementptr inbounds %struct.TCDef.7, %struct.TCDef.7* %i601, i64 0, i32 0, i64 0
  br label %bb734

bb603:                                            ; preds = %bb582
  br label %bb604

bb604:                                            ; preds = %bb603, %bb568
  %i605 = load i8, i8* %i3, align 16, !tbaa !14
  %i606 = sext i8 %i605 to i32
  %i607 = add i8 %i605, -97
  %i608 = icmp ult i8 %i607, 26
  %i609 = add nsw i32 %i606, -32
  %i610 = select i1 %i608, i32 %i609, i32 %i606
  %i611 = icmp eq i32 %i610, 83
  br i1 %i611, label %bb612, label %bb636

bb612:                                            ; preds = %bb604
  br label %bb613

bb613:                                            ; preds = %bb618, %bb612
  %i614 = phi i8 [ %i621, %bb618 ], [ %i605, %bb612 ]
  %i615 = phi i8* [ %i620, %bb618 ], [ getelementptr inbounds ([4 x i8], [4 x i8]* @.str.54, i64 0, i64 0), %bb612 ]
  %i616 = phi i8* [ %i619, %bb618 ], [ %i3, %bb612 ]
  %i617 = icmp eq i8 %i614, 0
  br i1 %i617, label %bb634, label %bb618

bb618:                                            ; preds = %bb613
  %i619 = getelementptr inbounds i8, i8* %i616, i64 1
  %i620 = getelementptr inbounds i8, i8* %i615, i64 1
  %i621 = load i8, i8* %i619, align 1, !tbaa !14
  %i622 = sext i8 %i621 to i32
  %i623 = add i8 %i621, -97
  %i624 = icmp ult i8 %i623, 26
  %i625 = add nsw i32 %i622, -32
  %i626 = select i1 %i624, i32 %i625, i32 %i622
  %i627 = load i8, i8* %i620, align 1, !tbaa !14
  %i628 = sext i8 %i627 to i32
  %i629 = add i8 %i627, -97
  %i630 = icmp ult i8 %i629, 26
  %i631 = add nsw i32 %i628, -32
  %i632 = select i1 %i630, i32 %i631, i32 %i628
  %i633 = icmp eq i32 %i626, %i632
  br i1 %i633, label %bb613, label %bb635

bb634:                                            ; preds = %bb613
  br label %bb734

bb635:                                            ; preds = %bb618
  br label %bb636

bb636:                                            ; preds = %bb635, %bb604
  %i637 = load i8, i8* %i3, align 16, !tbaa !14
  %i638 = sext i8 %i637 to i32
  %i639 = add i8 %i637, -97
  %i640 = icmp ult i8 %i639, 26
  %i641 = add nsw i32 %i638, -32
  %i642 = select i1 %i640, i32 %i641, i32 %i638
  %i643 = icmp eq i32 %i642, 72
  br i1 %i643, label %bb644, label %bb667

bb644:                                            ; preds = %bb636
  br label %bb645

bb645:                                            ; preds = %bb650, %bb644
  %i646 = phi i8 [ %i653, %bb650 ], [ %i637, %bb644 ]
  %i647 = phi i8* [ %i652, %bb650 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.57, i64 0, i64 0), %bb644 ]
  %i648 = phi i8* [ %i651, %bb650 ], [ %i3, %bb644 ]
  %i649 = icmp eq i8 %i646, 0
  br i1 %i649, label %bb730, label %bb650

bb650:                                            ; preds = %bb645
  %i651 = getelementptr inbounds i8, i8* %i648, i64 1
  %i652 = getelementptr inbounds i8, i8* %i647, i64 1
  %i653 = load i8, i8* %i651, align 1, !tbaa !14
  %i654 = sext i8 %i653 to i32
  %i655 = add i8 %i653, -97
  %i656 = icmp ult i8 %i655, 26
  %i657 = add nsw i32 %i654, -32
  %i658 = select i1 %i656, i32 %i657, i32 %i654
  %i659 = load i8, i8* %i652, align 1, !tbaa !14
  %i660 = sext i8 %i659 to i32
  %i661 = add i8 %i659, -97
  %i662 = icmp ult i8 %i661, 26
  %i663 = add nsw i32 %i660, -32
  %i664 = select i1 %i662, i32 %i663, i32 %i660
  %i665 = icmp eq i32 %i658, %i664
  br i1 %i665, label %bb645, label %bb666

bb666:                                            ; preds = %bb650
  br label %bb667

bb667:                                            ; preds = %bb666, %bb636
  %i668 = load i8, i8* %i3, align 16, !tbaa !14
  %i669 = sext i8 %i668 to i32
  %i670 = add i8 %i668, -97
  %i671 = icmp ult i8 %i670, 26
  %i672 = add nsw i32 %i669, -32
  %i673 = select i1 %i671, i32 %i672, i32 %i669
  %i674 = icmp eq i32 %i673, 72
  br i1 %i674, label %bb675, label %bb698

bb675:                                            ; preds = %bb667
  br label %bb676

bb676:                                            ; preds = %bb681, %bb675
  %i677 = phi i8 [ %i684, %bb681 ], [ %i668, %bb675 ]
  %i678 = phi i8* [ %i683, %bb681 ], [ getelementptr inbounds ([5 x i8], [5 x i8]* @.str.58, i64 0, i64 0), %bb675 ]
  %i679 = phi i8* [ %i682, %bb681 ], [ %i3, %bb675 ]
  %i680 = icmp eq i8 %i677, 0
  br i1 %i680, label %bb729, label %bb681

bb681:                                            ; preds = %bb676
  %i682 = getelementptr inbounds i8, i8* %i679, i64 1
  %i683 = getelementptr inbounds i8, i8* %i678, i64 1
  %i684 = load i8, i8* %i682, align 1, !tbaa !14
  %i685 = sext i8 %i684 to i32
  %i686 = add i8 %i684, -97
  %i687 = icmp ult i8 %i686, 26
  %i688 = add nsw i32 %i685, -32
  %i689 = select i1 %i687, i32 %i688, i32 %i685
  %i690 = load i8, i8* %i683, align 1, !tbaa !14
  %i691 = sext i8 %i690 to i32
  %i692 = add i8 %i690, -97
  %i693 = icmp ult i8 %i692, 26
  %i694 = add nsw i32 %i691, -32
  %i695 = select i1 %i693, i32 %i694, i32 %i691
  %i696 = icmp eq i32 %i689, %i695
  br i1 %i696, label %bb676, label %bb697

bb697:                                            ; preds = %bb681
  br label %bb698

bb698:                                            ; preds = %bb697, %bb667
  %i699 = load i8, i8* %i3, align 16, !tbaa !14
  %i700 = sext i8 %i699 to i32
  %i701 = add i8 %i699, -97
  %i702 = icmp ult i8 %i701, 26
  %i703 = add nsw i32 %i700, -32
  %i704 = select i1 %i702, i32 %i703, i32 %i700
  %i705 = icmp eq i32 %i704, 63
  br i1 %i705, label %bb706, label %bb734

bb706:                                            ; preds = %bb698
  br label %bb707

bb707:                                            ; preds = %bb712, %bb706
  %i708 = phi i8 [ %i715, %bb712 ], [ %i699, %bb706 ]
  %i709 = phi i8* [ %i714, %bb712 ], [ getelementptr inbounds ([2 x i8], [2 x i8]* @.str.59, i64 0, i64 0), %bb706 ]
  %i710 = phi i8* [ %i713, %bb712 ], [ %i3, %bb706 ]
  %i711 = icmp eq i8 %i708, 0
  br i1 %i711, label %bb728, label %bb712

bb712:                                            ; preds = %bb707
  %i713 = getelementptr inbounds i8, i8* %i710, i64 1
  %i714 = getelementptr inbounds i8, i8* %i709, i64 1
  %i715 = load i8, i8* %i713, align 1, !tbaa !14
  %i716 = sext i8 %i715 to i32
  %i717 = add i8 %i715, -97
  %i718 = icmp ult i8 %i717, 26
  %i719 = add nsw i32 %i716, -32
  %i720 = select i1 %i718, i32 %i719, i32 %i716
  %i721 = load i8, i8* %i714, align 1, !tbaa !14
  %i722 = sext i8 %i721 to i32
  %i723 = add i8 %i721, -97
  %i724 = icmp ult i8 %i723, 26
  %i725 = add nsw i32 %i722, -32
  %i726 = select i1 %i724, i32 %i725, i32 %i722
  %i727 = icmp eq i32 %i720, %i726
  br i1 %i727, label %bb707, label %bb733

bb728:                                            ; preds = %bb707
  br label %bb731

bb729:                                            ; preds = %bb676
  br label %bb731

bb730:                                            ; preds = %bb645
  br label %bb731

bb731:                                            ; preds = %bb730, %bb729, %bb728
  br label %bb734

bb732:                                            ; preds = %bb442
  br label %bb734

bb733:                                            ; preds = %bb712
  br label %bb734

bb734:                                            ; preds = %bb733, %bb732, %bb731, %bb698, %bb634, %bb598, %bb562, %bb530, %bb437
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %i3) #24
  br label %bb7

bb735:                                            ; preds = %bb364
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %i3) #24
  br label %bb738

bb736:                                            ; preds = %bb168
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %i3) #24
  tail call fastcc void @report_info()
  br label %bb738

bb737:                                            ; preds = %bb30
  br label %bb738

bb738:                                            ; preds = %bb737, %bb736, %bb735, %bb6, %bb4
  %i739 = phi i32 [ 2, %bb6 ], [ 2, %bb736 ], [ 3, %bb735 ], [ 3, %bb4 ], [ 3, %bb737 ]
  ret i32 %i739
}

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #23 = { nofree nounwind }
attributes #24 = { nounwind }
attributes #25 = { nounwind readnone willreturn }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSP8_IO_FILE", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !8, i64 0}
!11 = !{!"array@_ZTSA1041_c", !8, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPt", !8, i64 0}
!14 = !{!8, !8, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"short", !8, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"pointer@_ZTSP5TCDef", !8, i64 0}
!19 = !{!20, !24, i64 160}
!20 = !{!"struct@TCDef", !21, i64 0, !21, i64 16, !21, i64 32, !21, i64 48, !22, i64 64, !16, i64 128, !18, i64 136, !23, i64 144, !23, i64 148, !23, i64 152, !24, i64 160, !25, i64 168, !26, i64 176, !27, i64 184, !28, i64 192}
!21 = !{!"array@_ZTSA16_c", !8, i64 0}
!22 = !{!"array@_ZTSA64_c", !8, i64 0}
!23 = !{!"struct@", !8, i64 0, !8, i64 1, !8, i64 2, !8, i64 3}
!24 = !{!"long", !8, i64 0}
!25 = !{!"pointer@_ZTSPFimiPPKcE", !8, i64 0}
!26 = !{!"pointer@_ZTSPFiPP5TCDefiPPKcE", !8, i64 0}
!27 = !{!"pointer@_ZTSPFiiPPcE", !8, i64 0}
!28 = !{!"pointer@_ZTSPFvvE", !8, i64 0}
!29 = !{!24, !24, i64 0}
!30 = !{!31, !31, i64 0}
!31 = !{!"pointer@_ZTSPc", !8, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"int", !8, i64 0}
!34 = !{!35, !31, i64 0}
!35 = !{!"array@_ZTSA128_PKc", !31, i64 0}
