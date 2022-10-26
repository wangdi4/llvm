; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 2 are both valid transpose
; candidates even though field 1 is not a nested dope vector field. Check
; that the reading and writing of that field with memcpy, memset, and
; for_trim does not invalidate the dope vector analysis.

; Check that the array represented by field 0 of physpropmod_mp_physprop_
; can but should NOT be transposed because the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 0
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

; Check that the array represented by field 2 of physpropmod_mp_physprop_
; can and should be transposed to ensure that the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 2
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; Check that the array through which the indirect subscripting is occurring is
; a candidate for transposing but not profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", [32 x i8], %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit.1 = internal unnamed_addr constant [7 x i8] c"SULFATE"
@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal unnamed_addr global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.263b53731fe38c4199c4a10e662745ed.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca [32 x i8], align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.263b53731fe38c4199c4a10e662745ed.0) #5
  br label %bb2

bb2:                                              ; preds = %bb11, %bb
  %i3 = phi i64 [ %i12, %bb11 ], [ 1, %bb ]
  %i4 = trunc i64 %i3 to i32
  br label %bb5

bb5:                                              ; preds = %bb5, %bb2
  %i6 = phi i64 [ %i9, %bb5 ], [ 1, %bb2 ]
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i6)
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i7, i64 %i3)
  store i32 %i4, ptr %i8, align 1
  %i9 = add nuw nsw i64 %i6, 1
  %i10 = icmp eq i64 %i9, 1001
  br i1 %i10, label %bb11, label %bb5

bb11:                                             ; preds = %bb5
  %i12 = add nuw nsw i64 %i3, 1
  %i13 = icmp eq i64 %i12, 20
  br i1 %i13, label %bb14, label %bb2

bb14:                                             ; preds = %bb11
  %i15 = getelementptr inbounds [32 x i8], ptr %i, i64 0, i64 0
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 224, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i16, align 1
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, ptr %i17, align 1
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 224, ptr %i18, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i19 = tail call i32 @for_allocate_handle(i64 22400, ptr @physpropmod_mp_physprop_, i32 262144, ptr null) #5
  br label %bb20

bb20:                                             ; preds = %bb20, %bb14
  %i21 = phi i64 [ %i83, %bb20 ], [ 1, %bb14 ]
  %i22 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i23 = load i64, ptr %i18, align 1
  %i24 = load i64, ptr %i16, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i24, i64 %i23, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i22, i64 %i21)
  %i26 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i25, i64 0, i32 0
  %i27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 3
  store i64 5, ptr %i27, align 1
  %i28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 5
  store i64 0, ptr %i28, align 1
  %i29 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 1
  store i64 4, ptr %i29, align 1
  %i30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 4
  store i64 2, ptr %i30, align 1
  %i31 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 2
  store i64 0, ptr %i31, align 1
  %i32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 6, i64 0
  %i33 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 2
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 0)
  store i64 1, ptr %i34, align 1
  %i35 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 0
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 0)
  store i64 19, ptr %i36, align 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 1)
  store i64 1, ptr %i37, align 1
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 1)
  store i64 1000, ptr %i38, align 1
  %i39 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 1
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0)
  store i64 4, ptr %i40, align 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1)
  store i64 76, ptr %i41, align 1
  %i42 = load i64, ptr %i27, align 1
  %i43 = and i64 %i42, -68451041281
  %i44 = or i64 %i43, 1073741824
  store i64 %i44, ptr %i27, align 1
  %i45 = load i64, ptr %i28, align 1
  %i46 = inttoptr i64 %i45 to ptr
  %i47 = bitcast ptr %i25 to ptr
  %i48 = tail call i32 @for_allocate_handle(i64 76000, ptr %i47, i32 262144, ptr %i46) #5
  %i49 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i50 = load i64, ptr %i18, align 1
  %i51 = load i64, ptr %i16, align 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i51, i64 %i50, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i49, i64 %i21)
  %i53 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i52, i64 0, i32 2
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 3
  store i64 5, ptr %i54, align 1
  %i55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 5
  store i64 0, ptr %i55, align 1
  %i56 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 1
  store i64 4, ptr %i56, align 1
  %i57 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 4
  store i64 2, ptr %i57, align 1
  %i58 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 2
  store i64 0, ptr %i58, align 1
  %i59 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 6, i64 0
  %i60 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 2
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0)
  store i64 1, ptr %i61, align 1
  %i62 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 0
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 0)
  store i64 19, ptr %i63, align 1
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1)
  store i64 1, ptr %i64, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 1)
  store i64 1000, ptr %i65, align 1
  %i66 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 1
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 0)
  store i64 4, ptr %i67, align 1
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 1)
  store i64 76, ptr %i68, align 1
  %i69 = load i64, ptr %i54, align 1
  %i70 = and i64 %i69, -68451041281
  %i71 = or i64 %i70, 1073741824
  store i64 %i71, ptr %i54, align 1
  %i72 = load i64, ptr %i55, align 1
  %i73 = inttoptr i64 %i72 to ptr
  %i74 = bitcast ptr %i53 to ptr
  %i75 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i74, i32 262144, ptr %i73) #5
  %i76 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i77 = load i64, ptr %i18, align 1
  %i78 = load i64, ptr %i16, align 1
  %i79 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i78, i64 %i77, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i76, i64 %i21)
  %i80 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i79, i64 0, i32 1
  %i81 = getelementptr [32 x i8], ptr %i80, i64 0, i64 0
  %i82 = getelementptr i8, ptr %i81, i64 7
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(7) %i81, ptr noundef nonnull align 1 dereferenceable(7) @strlit.1, i64 7, i1 false)
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(25) %i82, i8 32, i64 25, i1 false)
  %i83 = add nuw nsw i64 %i21, 1
  %i84 = icmp eq i64 %i83, 101
  br i1 %i84, label %bb85, label %bb20

bb85:                                             ; preds = %bb162, %bb20
  %i86 = phi i64 [ %i163, %bb162 ], [ 1, %bb20 ]
  br label %bb87

bb87:                                             ; preds = %bb159, %bb85
  %i88 = phi i64 [ %i160, %bb159 ], [ 1, %bb85 ]
  %i89 = sub nsw i64 %i86, %i88
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i88)
  br label %bb91

bb91:                                             ; preds = %bb156, %bb87
  %i92 = phi i64 [ %i157, %bb156 ], [ 2, %bb87 ]
  %i93 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i94 = load i64, ptr %i18, align 1
  %i95 = load i64, ptr %i16, align 1
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i95, i64 %i94, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i93, i64 101)
  %i97 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i96, i64 0, i32 1
  %i98 = getelementptr [32 x i8], ptr %i97, i64 0, i64 0
  %i99 = call i64 @for_trim(ptr nonnull %i15, i32 32, ptr nonnull %i98, i32 32) #5
  %i100 = shl i64 %i99, 32
  %i101 = ashr exact i64 %i100, 32
  %i102 = call i64 @for_cpstr(ptr nonnull %i15, i64 %i101, ptr @strlit.1, i64 7, i64 2) #5
  %i103 = and i64 %i102, 1
  %i104 = icmp eq i64 %i103, 0
  br i1 %i104, label %bb156, label %bb105

bb105:                                            ; preds = %bb91
  %i106 = add nsw i64 %i89, %i92
  %i107 = trunc i64 %i106 to i32
  %i108 = sitofp i32 %i107 to float
  %i109 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i110 = load i64, ptr %i18, align 1
  %i111 = load i64, ptr %i16, align 1
  %i112 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i111, i64 %i110, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i109, i64 101)
  %i113 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i112, i64 0, i32 2
  %i114 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i113, i64 0, i32 0
  %i115 = load ptr, ptr %i114, align 1
  %i116 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i113, i64 0, i32 6, i64 0
  %i117 = getelementptr inbounds { i64, i64, i64 }, ptr %i116, i64 0, i32 1
  %i118 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i117, i32 0)
  %i119 = load i64, ptr %i118, align 1
  %i120 = getelementptr inbounds { i64, i64, i64 }, ptr %i116, i64 0, i32 2
  %i121 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i120, i32 0)
  %i122 = load i64, ptr %i121, align 1
  %i123 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i90, i64 %i92)
  %i124 = load i32, ptr %i123, align 1
  %i125 = add nsw i32 %i124, 1
  %i126 = sext i32 %i125 to i64
  %i127 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i117, i32 1)
  %i128 = load i64, ptr %i127, align 1
  %i129 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i120, i32 1)
  %i130 = load i64, ptr %i129, align 1
  %i131 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i130, i64 %i128, ptr elementtype(float) %i115, i64 %i88)
  %i132 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i122, i64 %i119, ptr elementtype(float) %i131, i64 %i126)
  store float %i108, ptr %i132, align 1
  %i133 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i134 = load i64, ptr %i18, align 1
  %i135 = load i64, ptr %i16, align 1
  %i136 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i135, i64 %i134, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i133, i64 101)
  %i137 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i136, i64 0, i32 0
  %i138 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i137, i64 0, i32 0
  %i139 = load ptr, ptr %i138, align 1
  %i140 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i137, i64 0, i32 6, i64 0
  %i141 = getelementptr inbounds { i64, i64, i64 }, ptr %i140, i64 0, i32 1
  %i142 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i141, i32 0)
  %i143 = load i64, ptr %i142, align 1
  %i144 = getelementptr inbounds { i64, i64, i64 }, ptr %i140, i64 0, i32 2
  %i145 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i144, i32 0)
  %i146 = load i64, ptr %i145, align 1
  %i147 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i141, i32 1)
  %i148 = load i64, ptr %i147, align 1
  %i149 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i144, i32 1)
  %i150 = load i64, ptr %i149, align 1
  %i151 = load i32, ptr %i123, align 1
  %i152 = add nsw i32 %i151, -1
  %i153 = sext i32 %i152 to i64
  %i154 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i150, i64 %i148, ptr elementtype(float) %i139, i64 %i153)
  %i155 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i146, i64 %i143, ptr elementtype(float) %i154, i64 %i88)
  store float %i108, ptr %i155, align 1
  br label %bb156

bb156:                                            ; preds = %bb105, %bb91
  %i157 = add nuw nsw i64 %i92, 1
  %i158 = icmp eq i64 %i157, 19
  br i1 %i158, label %bb159, label %bb91

bb159:                                            ; preds = %bb156
  %i160 = add nuw nsw i64 %i88, 1
  %i161 = icmp eq i64 %i160, 1001
  br i1 %i161, label %bb162, label %bb87

bb162:                                            ; preds = %bb159
  %i163 = add nuw nsw i64 %i86, 1
  %i164 = icmp eq i64 %i163, 101
  br i1 %i164, label %bb165, label %bb85

bb165:                                            ; preds = %bb162
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i64 @for_trim(ptr nocapture, i32, ptr nocapture readonly, i32) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i64 @for_cpstr(ptr nocapture readonly, i64, ptr nocapture readonly, i64, i64) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { argmemonly nocallback nofree nounwind willreturn }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
