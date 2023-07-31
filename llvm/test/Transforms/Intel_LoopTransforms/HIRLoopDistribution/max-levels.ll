
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-loopnest,hir-loop-distribute-memrec,print<hir>" -disable-output %s 2>&1 | FileCheck %s

; Check that distribution does not try to distribute loopnests which already have
; the maximum level.

;    BEGIN REGION { }
;          + DO i1 = 0, zext.i32.i64(%tmp202), 1   <DO_LOOP>
;          |   + DO i2 = 0, sext.i32.i64(%tmp213) + -1, 1   <DO_LOOP>
;          |   |   %tmp225 = -1 * i1 + zext.i32.i64(%tmp202)  -  i2;
;          |   |
;          |   |   + DO i3 = 0, smax(0, %tmp225), 1   <DO_LOOP>
;          |   |   |   %tmp235 = %tmp202  -  i1 + i2 + i3;
;          |   |   |
;          |   |   |   + DO i4 = 0, smax(0, %tmp235), 1   <DO_LOOP>
;          |   |   |   |   %tmp255 = zext.i32.i64(-1 * i1 + -1 * i2 + -1 * i3 + -1 * i4 + %tmp202);
;          |   |   |   |
;          |   |   |   |   + DO i5 = 0, %tmp255, 1   <DO_LOOP>
;          |   |   |   |   |   %tmp266 = zext.i32.i64(-1 * i1 + -1 * i2 + -1 * i3 + -1 * i4 + -1 * i5 + %tmp202);
;          |   |   |   |   |
;          |   |   |   |   |   + DO i6 = 0, %tmp266, 1   <DO_LOOP>
;          |   |   |   |   |   |   %tmp282 = i3 + i6  &  4294967295;
;          |   |   |   |   |   |   %tmp283 = zext.i32.i64(-1 * i1 + -1 * i2 + -1 * i3 + -1 * i4 + -1 * i5 + -1 * i6 + %tmp202);
;          |   |   |   |   |   |
;          |   |   |   |   |   |   + DO i7 = 0, %tmp283, 1   <DO_LOOP>
;          |   |   |   |   |   |   |   %tmp286 = -1 * i1 + -1 * i2 + -1 * i3 + -1 * i4 + -1 * i5 + -1 * i6 + %tmp202  -  i7;
;          |   |   |   |   |   |   |
;          |   |   |   |   |   |   |   + DO i8 = 0, smax(1, (1 + %tmp286)) + -1, 1   <DO_LOOP>
;          |   |   |   |   |   |   |   |   %tmp299 = -1 * i7 + %tmp283  -  i8;
;          |   |   |   |   |   |   |   |   %tmp304 = (null)[0];
;          |   |   |   |   |   |   |   |   %tmp305 = (null)[0];
;          |   |   |   |   |   |   |   |   %tmp306 = (%tmp198)[i3][1][1];
;          |   |   |   |   |   |   |   |   %tmp307 = (%tmp198)[i2][2][1];
;          |   |   |   |   |   |   |   |   %tmp308 = (%tmp198)[i1][3][1];
;          |   |   |   |   |   |   |   |   %tmp309 = (%tmp198)[i6][1][2];
;          |   |   |   |   |   |   |   |   %tmp310 = (%tmp198)[i5][2][2];
;          |   |   |   |   |   |   |   |   %tmp311 = (%tmp198)[i4][3][2];
;          |   |   |   |   |   |   |   |   %tmp315 = (%tmp198)[i8][2][3];
;          |   |   |   |   |   |   |   |   %tmp316 = (%tmp198)[i7][3][3];
;          |   |   |   |   |   |   |   |   %tmp317 = (null)[i1 + i2 + i3];
;          |   |   |   |   |   |   |   |   %tmp318 = (null)[i4 + i5 + i6];
;          |   |   |   |   |   |   |   |   %tmp321 = (null)[i6]  *  (null)[i3];
;          |   |   |   |   |   |   |   |   %tmp322 = (null)[i2];
;          |   |   |   |   |   |   |   |   %tmp323 = (null)[i5];
;          |   |   |   |   |   |   |   |   %tmp325 = (null)[i8];
;          |   |   |   |   |   |   |   |   %tmp326 = (null)[i1];
;          |   |   |   |   |   |   |   |   %tmp327 = (null)[i4];
;          |   |   |   |   |   |   |   |   %tmp328 = (null)[i7];
;          |   |   |   |   |   |   |   |
;          |   |   |   |   |   |   |   |   + DO i9 = 0, smax(1, (1 + %tmp299)) + -1, 1   <DO_LOOP>
;          |   |   |   |   |   |   |   |   |   %tmp335 = (%tmp116)[i1 + i4 + i7][i2 + i5 + i8][i9 + %tmp282];
;          |   |   |   |   |   |   |   |   |   %tmp342 = (%tmp116)[i7 + i8 + i9][i4 + i5 + i6][i1 + i2 + i3];

; CHECK: BEGIN REGION
; CHECK-NOT: modified
; CHECK: DO i9

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.pluto = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%struct.eggs = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%struct.eggs.0 = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%struct.zot = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%struct.wobble = type <{ ptr, i32, i32, i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], [3 x i32], %struct.zot, %struct.zot, %struct.zot, %struct.pluto }>
%struct.baz = type <{ i32, [4 x i8], ptr, i32, [4 x i8], i64, [3 x i32], [3 x i32], [3 x i32], i32, [3 x i32], [4 x i8], [3 x [3 x double]], [3 x [3 x double]], i32, i32, i32, i32, i32, i32, i32, [3 x i32], [3 x i32], [3 x i32], %struct.barney, %struct.barney, %struct.barney, %struct.eggs.0, %struct.zot, %struct.zot, %struct.zot, i32, [3 x i32], %struct.zot, %struct.zot }>
%struct.snork = type <{ i64, i64, [3 x [2 x i32]], [3 x i32], i32, i32, [3 x [2 x i32]], [3 x i32], [3 x double], [3 x [3 x double]], [3 x [3 x double]], i32, [4 x i8], double, double, double, %struct.foo, %struct.foo, %struct.foo, %struct.spam, %struct.wibble, %struct.eggs, %struct.barney, %struct.barney, i32, i32, i32, i32, i32, [4 x i8], %struct.zot, i32, i32, %struct.zot.2 }>
%struct.foo = type <{ %struct.zot, %struct.zot }>
%struct.spam = type <{ i32, i32, i32, i32, i32, i32, i32, i32, %struct.eggs.0, %struct.barney, %struct.zot, i32, i32, [2 x i32], [2 x i32], %struct.wobble.1, %struct.zot }>
%struct.wobble.1 = type { ptr, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }
%struct.wibble = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.zot.2 = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.snork.3 = type { double, double }
%struct.barney = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.baz.4 = type <{ i32, [4 x i8], [3 x double], double, [3 x [3 x double]], [3 x [3 x double]], i32, [4 x i8], %struct.barney, %struct.barney, %struct.wobble.5, %struct.zot, double }>
%struct.wobble.5 = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%struct.zot.6 = type <{ i32, i32, %struct.eggs }>

; Function Attrs: nounwind uwtable
define void @eggs() #0 {
bb:
  %tmp = alloca [3 x double], align 16
  %tmp1 = alloca [3 x double], align 16
  %tmp2 = alloca [3 x double], align 16
  %tmp3 = alloca [3 x [3 x double]], align 16
  %tmp4 = alloca [3 x double], align 16
  %tmp5 = alloca [3 x i32], align 16
  %tmp6 = alloca [3 x i32], align 16
  %tmp7 = alloca [3 x i32], align 16
  %tmp8 = alloca %struct.pluto, align 8
  %tmp9 = alloca %struct.eggs, align 8
  %tmp10 = alloca %struct.eggs.0, align 8
  %tmp11 = alloca %struct.zot, align 8
  %tmp12 = alloca i64, align 8
  %tmp13 = alloca i64, align 8
  %tmp14 = alloca [3 x double], align 16
  %tmp15 = alloca i64, align 8
  %tmp16 = alloca i64, align 8
  %tmp17 = getelementptr inbounds [3 x i32], ptr %tmp6, i64 0, i64 0
  %tmp18 = getelementptr inbounds [3 x i32], ptr %tmp5, i64 0, i64 0
  %tmp19 = load ptr, ptr null, align 1
  %tmp20 = load ptr, ptr null, align 1
  %tmp21 = load ptr, ptr null, align 1
  %tmp22 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 0
  %tmp23 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 1
  %tmp24 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 2
  %tmp25 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 3
  %tmp26 = bitcast ptr %tmp8 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %tmp26, i8 0, i64 24, i1 false)
  store i64 128, ptr %tmp25, align 8
  %tmp27 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 4
  store i64 3, ptr %tmp27, align 8
  %tmp28 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 5
  %tmp29 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 6, i64 0, i32 0
  %tmp30 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 6, i64 0, i32 1
  %tmp31 = getelementptr inbounds %struct.pluto, ptr %tmp8, i64 0, i32 6, i64 0, i32 2
  %tmp32 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 0
  %tmp33 = bitcast ptr %tmp28 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(80) %tmp33, i8 0, i64 80, i1 false)
  %tmp34 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 1
  %tmp35 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 2
  %tmp36 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 3
  %tmp37 = bitcast ptr %tmp9 to ptr
  store i64 0, ptr %tmp37, align 8
  %tmp38 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 4
  %tmp39 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 5
  %tmp40 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 6, i64 0, i32 0
  %tmp41 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 6, i64 0, i32 1
  %tmp42 = getelementptr inbounds %struct.eggs, ptr %tmp9, i64 0, i32 6, i64 0, i32 2
  %tmp43 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 0
  %tmp44 = getelementptr inbounds i64, ptr %tmp39, i64 1
  %tmp45 = bitcast ptr %tmp44 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %tmp45, i8 0, i64 24, i1 false)
  %tmp46 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 1
  %tmp47 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 2
  %tmp48 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 3
  %tmp49 = bitcast ptr %tmp10 to ptr
  store i64 0, ptr %tmp49, align 8
  %tmp50 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 4
  %tmp51 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 5
  %tmp52 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 6, i64 0, i32 0
  %tmp53 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 6, i64 0, i32 1
  %tmp54 = getelementptr inbounds %struct.eggs.0, ptr %tmp10, i64 0, i32 6, i64 0, i32 2
  %tmp55 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 0
  %tmp56 = getelementptr inbounds i64, ptr %tmp51, i64 1
  %tmp57 = bitcast ptr %tmp56 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(72) %tmp57, i8 0, i64 72, i1 false)
  %tmp58 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 1
  %tmp59 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 2
  %tmp60 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 3
  %tmp61 = bitcast ptr %tmp11 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %tmp61, i8 0, i64 24, i1 false)
  store i64 128, ptr %tmp60, align 8
  %tmp62 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 4
  store i64 1, ptr %tmp62, align 8
  %tmp63 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 5
  %tmp64 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 6, i64 0, i32 0
  %tmp65 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 6, i64 0, i32 1
  %tmp66 = getelementptr inbounds %struct.zot, ptr %tmp11, i64 0, i32 6, i64 0, i32 2
  %tmp67 = bitcast ptr %tmp63 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(32) %tmp67, i8 0, i64 32, i1 false)
  store i64 0, ptr %tmp39, align 8
  store i64 8, ptr %tmp34, align 8
  store i64 1, ptr %tmp38, align 8
  store i64 0, ptr %tmp35, align 8
  %tmp68 = load i32, ptr null, align 4
  %tmp69 = add nsw i32 %tmp68, 1
  %tmp70 = add nsw i32 %tmp68, 2
  %tmp71 = mul nsw i32 %tmp69, %tmp70
  %tmp72 = add nsw i32 %tmp68, 3
  %tmp73 = mul nsw i32 %tmp71, %tmp72
  %tmp74 = sdiv i32 %tmp73, 6
  %tmp75 = sext i32 %tmp74 to i64
  %tmp76 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp42, i32 0)
  store i64 1, ptr %tmp76, align 8
  %tmp77 = icmp sgt i64 %tmp75, 0
  %tmp78 = select i1 %tmp77, i64 %tmp75, i64 0
  %tmp79 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp40, i32 0)
  store i64 %tmp78, ptr %tmp79, align 8
  %tmp80 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp41, i32 0)
  store i64 8, ptr %tmp80, align 8
  %tmp81 = call i32 (ptr, i32, ...) null(ptr nonnull %tmp12, i32 2, i64 %tmp78, i64 8) #4
  %tmp82 = load i64, ptr %tmp12, align 8
  store i64 1073741957, ptr %tmp36, align 8
  %tmp83 = shl i32 %tmp81, 4
  %tmp84 = and i32 %tmp83, 16
  %tmp85 = or i32 %tmp84, 262146
  %tmp86 = bitcast ptr %tmp9 to ptr
  %tmp87 = call i32 null(i64 %tmp82, ptr nonnull %tmp86, i32 %tmp85, ptr null) #4
  store i64 0, ptr %tmp51, align 8
  store i64 4, ptr %tmp46, align 8
  store i64 3, ptr %tmp50, align 8
  store i64 0, ptr %tmp47, align 8
  %tmp88 = load i32, ptr null, align 4
  %tmp89 = sext i32 %tmp88 to i64
  %tmp90 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp54, i32 0)
  store i64 0, ptr %tmp90, align 8
  %tmp91 = add nsw i64 %tmp89, 1
  %tmp92 = icmp sgt i32 %tmp88, -1
  %tmp93 = select i1 %tmp92, i64 %tmp91, i64 0
  %tmp94 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp52, i32 0)
  store i64 %tmp93, ptr %tmp94, align 8
  %tmp95 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp54, i32 1)
  store i64 0, ptr %tmp95, align 8
  %tmp96 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp52, i32 1)
  store i64 %tmp93, ptr %tmp96, align 8
  %tmp97 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp54, i32 2)
  store i64 0, ptr %tmp97, align 8
  %tmp98 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp52, i32 2)
  store i64 %tmp93, ptr %tmp98, align 8
  %tmp99 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp53, i32 0)
  store i64 4, ptr %tmp99, align 8
  %tmp100 = shl nsw i64 %tmp93, 2
  %tmp101 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp53, i32 1)
  store i64 %tmp100, ptr %tmp101, align 8
  %tmp102 = mul nsw i64 %tmp100, %tmp93
  %tmp103 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp53, i32 2)
  store i64 %tmp102, ptr %tmp103, align 8
  %tmp104 = call i32 (ptr, i32, ...) null(ptr nonnull %tmp13, i32 4, i64 %tmp93, i64 %tmp93, i64 %tmp93, i64 4) #4
  %tmp105 = load i64, ptr %tmp13, align 8
  store i64 1073741957, ptr %tmp48, align 8
  %tmp106 = shl i32 %tmp104, 4
  %tmp107 = and i32 %tmp106, 16
  %tmp108 = or i32 %tmp107, 262146
  %tmp109 = bitcast ptr %tmp10 to ptr
  %tmp110 = call i32 null(i64 %tmp105, ptr nonnull %tmp109, i32 %tmp108, ptr null) #4
  %tmp111 = load i64, ptr %tmp90, align 8
  %tmp112 = load i64, ptr %tmp95, align 8
  %tmp113 = load i64, ptr %tmp101, align 8
  %tmp114 = load i64, ptr %tmp97, align 8
  %tmp115 = load i64, ptr %tmp103, align 8
  %tmp116 = load ptr, ptr %tmp43, align 8
  %tmp117 = load i64, ptr %tmp98, align 8
  %tmp118 = icmp slt i64 %tmp117, 1
  %tmp119 = bitcast ptr %tmp116 to ptr
  store i32 0, ptr null, align 4
  %tmp120 = load i32, ptr null, align 4
  store i32 0, ptr null, align 4
  %tmp121 = icmp slt i32 %tmp120, 0
  %tmp122 = load ptr, ptr %tmp19, align 1
  %tmp123 = getelementptr inbounds %struct.wobble, ptr %tmp122, i64 0, i32 0
  %tmp124 = load ptr, ptr %tmp123, align 1
  %tmp125 = getelementptr inbounds %struct.baz, ptr %tmp124, i64 0, i32 12, i64 0, i64 0
  %tmp126 = getelementptr inbounds [3 x [3 x double]], ptr %tmp3, i64 0, i64 0, i64 0
  %tmp127 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %tmp125, i64 1)
  %tmp128 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %tmp126, i64 1)
  %tmp129 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp127, i64 1)
  %tmp130 = load double, ptr %tmp129, align 1
  %tmp131 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp128, i64 1)
  store double %tmp130, ptr %tmp131, align 8
  %tmp132 = add nuw nsw i64 1, 1
  %tmp133 = icmp eq i64 %tmp132, 4
  %tmp134 = add nuw nsw i64 1, 1
  %tmp135 = icmp eq i64 %tmp134, 4
  %tmp136 = getelementptr inbounds [3 x double], ptr %tmp14, i64 0, i64 0
  %tmp137 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp136, i64 1)
  store double 0.000000e+00, ptr %tmp137, align 8
  %tmp138 = add nuw nsw i64 1, 1
  %tmp139 = icmp eq i64 %tmp138, 4
  %tmp140 = getelementptr inbounds %struct.baz, ptr %tmp124, i64 0, i32 13, i64 0, i64 0
  %tmp141 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %tmp140, i64 1)
  %tmp142 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) null, i64 1)
  %tmp143 = load double, ptr %tmp142, align 8
  %tmp144 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp141, i64 1)
  %tmp145 = load double, ptr %tmp144, align 1
  %tmp146 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp136, i64 1)
  %tmp147 = load double, ptr %tmp146, align 8
  %tmp148 = fmul reassoc ninf nsz arcp contract afn double %tmp143, %tmp145
  %tmp149 = fadd reassoc ninf nsz arcp contract afn double %tmp147, %tmp148
  store double %tmp149, ptr %tmp146, align 8
  %tmp150 = add nuw nsw i64 1, 1
  %tmp151 = icmp eq i64 %tmp150, 4
  %tmp152 = add nuw nsw i64 1, 1
  %tmp153 = icmp eq i64 %tmp152, 4
  %tmp154 = getelementptr inbounds [3 x double], ptr %tmp4, i64 0, i64 0
  %tmp155 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp136, i64 1)
  %tmp156 = load double, ptr %tmp155, align 8
  %tmp157 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp154, i64 1)
  store double %tmp156, ptr %tmp157, align 8
  %tmp158 = add nuw nsw i64 1, 1
  %tmp159 = icmp eq i64 %tmp158, 4
  %tmp160 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp154, i64 1)
  %tmp161 = load double, ptr %tmp160, align 8
  %tmp162 = tail call reassoc ninf nsz arcp contract afn double @llvm.floor.f64(double %tmp161)
  %tmp163 = fptosi double %tmp162 to i32
  %tmp164 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) null, i64 1)
  store i32 %tmp163, ptr %tmp164, align 4
  %tmp165 = add nuw nsw i64 1, 1
  %tmp166 = icmp eq i64 %tmp165, 4
  %tmp167 = load i64, ptr %tmp25, align 8
  %tmp168 = and i64 %tmp167, 1030792151296
  store i64 0, ptr %tmp28, align 8
  store i64 8, ptr %tmp23, align 8
  store i64 3, ptr %tmp27, align 8
  store i64 0, ptr %tmp24, align 8
  %tmp169 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp31, i32 0)
  store i64 1, ptr %tmp169, align 8
  %tmp170 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp29, i32 0)
  store i64 3, ptr %tmp170, align 8
  %tmp171 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp31, i32 1)
  store i64 1, ptr %tmp171, align 8
  %tmp172 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp29, i32 1)
  store i64 3, ptr %tmp172, align 8
  %tmp173 = sext i32 %tmp120 to i64
  %tmp174 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp31, i32 2)
  store i64 0, ptr %tmp174, align 8
  %tmp175 = add nsw i64 %tmp173, 1
  %tmp176 = icmp sgt i32 %tmp120, -1
  %tmp177 = select i1 %tmp176, i64 %tmp175, i64 0
  %tmp178 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp29, i32 2)
  store i64 %tmp177, ptr %tmp178, align 8
  %tmp179 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 0)
  store i64 8, ptr %tmp179, align 8
  %tmp180 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 1)
  store i64 24, ptr %tmp180, align 8
  %tmp181 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 2)
  store i64 72, ptr %tmp181, align 8
  %tmp182 = call i32 (ptr, i32, ...) null(ptr nonnull %tmp15, i32 2, i64 %tmp177, i64 72) #4
  %tmp183 = load i64, ptr %tmp15, align 8
  %tmp184 = or i64 %tmp168, 1073741957
  store i64 %tmp184, ptr %tmp25, align 8
  %tmp185 = shl i32 %tmp182, 4
  %tmp186 = and i32 %tmp185, 16
  %tmp187 = lshr i64 %tmp168, 15
  %tmp188 = trunc i64 %tmp187 to i32
  %tmp189 = or i32 %tmp186, %tmp188
  %tmp190 = or i32 %tmp189, 262146
  %tmp191 = bitcast ptr %tmp8 to ptr
  %tmp192 = call i32 null(i64 %tmp183, ptr nonnull %tmp191, i32 %tmp190, ptr null) #4
  %tmp193 = load i64, ptr %tmp169, align 8
  %tmp194 = load i64, ptr %tmp171, align 8
  %tmp195 = load i64, ptr %tmp180, align 8
  %tmp196 = load i64, ptr %tmp174, align 8
  %tmp197 = load i64, ptr %tmp181, align 8
  %tmp198 = load ptr, ptr %tmp22, align 8
  %tmp199 = load i64, ptr %tmp172, align 8
  %tmp200 = icmp slt i64 %tmp199, 1
  %tmp201 = bitcast ptr %tmp198 to ptr
  %tmp202 = load i32, ptr null, align 4
  %tmp203 = icmp slt i32 %tmp202, 1
  %tmp204 = icmp slt i32 %tmp202, 0
  %tmp205 = load ptr, ptr %tmp32, align 8
  %tmp206 = load i64, ptr %tmp76, align 8
  %tmp207 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) null, i32 0)
  %tmp208 = zext i32 %tmp202 to i64
  %tmp209 = add nuw nsw i32 %tmp202, 1
  %tmp210 = zext i32 %tmp209 to i64
  br label %bb211

bb211:                                            ; preds = %bb402, %bb
  %tmp212 = phi i64 [ 0, %bb ], [ %tmp403, %bb402 ]
  %tmp213 = phi i32 [ %tmp209, %bb ], [ %tmp404, %bb402 ]
  %tmp214 = sub nsw i64 %tmp208, %tmp212
  %tmp215 = icmp slt i64 %tmp214, 0
  %tmp216 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp212)
  %tmp217 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp216, i64 3)
  %tmp218 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp217, i64 1)
  %tmp219 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp212)
  %tmp220 = sext i32 %tmp213 to i64
  br label %bb221

bb221:                                            ; preds = %bb398, %bb211
  %tmp222 = phi i64 [ 0, %bb211 ], [ %tmp399, %bb398 ]
  %tmp223 = phi i32 [ 0, %bb211 ], [ %tmp400, %bb398 ]
  %tmp224 = zext i32 %tmp223 to i64
  %tmp225 = sub nsw i64 %tmp214, %tmp224
  %tmp226 = icmp slt i64 %tmp225, 0
  %tmp227 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp222)
  %tmp228 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp227, i64 2)
  %tmp229 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp228, i64 1)
  %tmp230 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp222)
  br label %bb231

bb231:                                            ; preds = %bb395, %bb221
  %tmp232 = phi i64 [ 0, %bb221 ], [ %tmp396, %bb395 ]
  %tmp233 = add nuw nsw i64 %tmp222, %tmp232
  %tmp234 = add nuw nsw i64 %tmp233, %tmp212
  %tmp235 = sub nsw i64 %tmp208, %tmp234
  %tmp236 = icmp slt i64 %tmp235, 0
  %tmp237 = trunc i64 %tmp234 to i32
  %tmp238 = and i64 %tmp234, 4294967295
  %tmp239 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp232)
  %tmp240 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp239, i64 1)
  %tmp241 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp240, i64 1)
  %tmp242 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp238)
  %tmp243 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp232)
  %tmp244 = trunc i64 %tmp235 to i32
  br label %bb245

bb245:                                            ; preds = %bb391, %bb231
  %tmp246 = phi i64 [ 0, %bb231 ], [ %tmp392, %bb391 ]
  %tmp247 = phi i32 [ 0, %bb231 ], [ %tmp393, %bb391 ]
  %tmp248 = sub nsw i32 %tmp244, %tmp247
  %tmp249 = icmp slt i32 %tmp248, 0
  %tmp250 = add nuw nsw i64 %tmp212, %tmp246
  %tmp251 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp246)
  %tmp252 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp251, i64 3)
  %tmp253 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp252, i64 2)
  %tmp254 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp246)
  %tmp255 = zext i32 %tmp248 to i64
  br label %bb256

bb256:                                            ; preds = %bb387, %bb245
  %tmp257 = phi i64 [ 0, %bb245 ], [ %tmp388, %bb387 ]
  %tmp258 = phi i32 [ 0, %bb245 ], [ %tmp389, %bb387 ]
  %tmp259 = sub nsw i32 %tmp248, %tmp258
  %tmp260 = icmp slt i32 %tmp259, 0
  %tmp261 = add nuw nsw i64 %tmp222, %tmp257
  %tmp262 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp257)
  %tmp263 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp262, i64 2)
  %tmp264 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp263, i64 2)
  %tmp265 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp257)
  %tmp266 = zext i32 %tmp259 to i64
  br label %bb267

bb267:                                            ; preds = %bb384, %bb256
  %tmp268 = phi i64 [ 0, %bb256 ], [ %tmp385, %bb384 ]
  %tmp269 = add nuw nsw i64 %tmp257, %tmp268
  %tmp270 = add nuw nsw i64 %tmp269, %tmp246
  %tmp271 = trunc i64 %tmp270 to i32
  %tmp272 = add nuw i32 %tmp237, %tmp271
  %tmp273 = sub i32 %tmp202, %tmp272
  %tmp274 = icmp slt i32 %tmp273, 0
  %tmp275 = add nuw nsw i64 %tmp232, %tmp268
  %tmp276 = and i64 %tmp270, 4294967295
  %tmp277 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp268)
  %tmp278 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp277, i64 1)
  %tmp279 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp278, i64 2)
  %tmp280 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp276)
  %tmp281 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp268)
  %tmp282 = and i64 %tmp275, 4294967295
  %tmp283 = zext i32 %tmp273 to i64
  br label %bb284

bb284:                                            ; preds = %bb381, %bb267
  %tmp285 = phi i64 [ 0, %bb267 ], [ %tmp382, %bb381 ]
  %tmp286 = sub nsw i64 %tmp283, %tmp285
  %tmp287 = icmp slt i64 %tmp286, 0
  %tmp288 = add nuw nsw i64 %tmp250, %tmp285
  %tmp289 = and i64 %tmp288, 4294967295
  %tmp290 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp114, i64 %tmp115, ptr elementtype(i32) %tmp116, i64 %tmp289)
  %tmp291 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp285)
  %tmp292 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp291, i64 3)
  %tmp293 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp292, i64 3)
  %tmp294 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp285)
  br label %bb295

bb295:                                            ; preds = %bb377, %bb284
  %tmp296 = phi i64 [ 0, %bb284 ], [ %tmp378, %bb377 ]
  %tmp297 = phi i32 [ 0, %bb284 ], [ %tmp379, %bb377 ]
  %tmp298 = zext i32 %tmp297 to i64
  %tmp299 = sub nsw i64 %tmp286, %tmp298
  %tmp300 = icmp slt i64 %tmp299, 0
  %tmp301 = add nuw nsw i64 %tmp261, %tmp296
  %tmp302 = and i64 %tmp301, 4294967295
  %tmp303 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp112, i64 %tmp113, ptr elementtype(i32) %tmp290, i64 %tmp302)
  %tmp304 = load ptr, ptr null, align 8
  %tmp305 = load i64, ptr %tmp207, align 8
  %tmp306 = load double, ptr %tmp241, align 8
  %tmp307 = load double, ptr %tmp229, align 8
  %tmp308 = load double, ptr %tmp218, align 8
  %tmp309 = load double, ptr %tmp279, align 8
  %tmp310 = load double, ptr %tmp264, align 8
  %tmp311 = load double, ptr %tmp253, align 8
  %tmp312 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr elementtype(double) %tmp198, i64 %tmp296)
  %tmp313 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp312, i64 2)
  %tmp314 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp313, i64 3)
  %tmp315 = load double, ptr %tmp314, align 8
  %tmp316 = load double, ptr %tmp293, align 8
  %tmp317 = load double, ptr %tmp242, align 8
  %tmp318 = load double, ptr %tmp280, align 8
  %tmp319 = load double, ptr %tmp243, align 8
  %tmp320 = load double, ptr %tmp281, align 8
  %tmp321 = fmul reassoc ninf nsz arcp contract afn double %tmp320, %tmp319
  %tmp322 = load double, ptr %tmp230, align 8
  %tmp323 = load double, ptr %tmp265, align 8
  %tmp324 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp296)
  %tmp325 = load double, ptr %tmp324, align 8
  %tmp326 = load double, ptr %tmp219, align 8
  %tmp327 = load double, ptr %tmp254, align 8
  %tmp328 = load double, ptr %tmp294, align 8
  br label %bb329

bb329:                                            ; preds = %bb329, %bb295
  %tmp330 = phi i64 [ 0, %bb295 ], [ %tmp375, %bb329 ]
  %tmp331 = add nuw nsw i64 %tmp296, %tmp330
  %tmp332 = add nuw nsw i64 %tmp331, %tmp285
  %tmp333 = add nuw nsw i64 %tmp282, %tmp330
  %tmp334 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp111, i64 4, ptr elementtype(i32) %tmp303, i64 %tmp333)
  %tmp335 = load i32, ptr %tmp334, align 4, !tbaa !0
  %tmp336 = sext i32 %tmp335 to i64
  %tmp337 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp206, i64 8, ptr elementtype(double) %tmp205, i64 %tmp336)
  %tmp338 = load double, ptr %tmp337, align 8
  %tmp339 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp114, i64 %tmp115, ptr elementtype(i32) %tmp116, i64 %tmp332)
  %tmp340 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp112, i64 %tmp113, ptr elementtype(i32) %tmp339, i64 %tmp276)
  %tmp341 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp111, i64 4, ptr elementtype(i32) %tmp340, i64 %tmp238)
  %tmp342 = load i32, ptr %tmp341, align 4, !tbaa !0
  %tmp343 = sext i32 %tmp342 to i64
  %tmp344 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp305, i64 8, ptr elementtype(double) %tmp304, i64 %tmp343)
  %tmp345 = load double, ptr %tmp344, align 1, !tbaa !5
  %tmp346 = fmul reassoc ninf nsz arcp contract afn double %tmp306, %tmp345
  %tmp347 = fmul reassoc ninf nsz arcp contract afn double %tmp346, %tmp307
  %tmp348 = fmul reassoc ninf nsz arcp contract afn double %tmp347, %tmp308
  %tmp349 = fmul reassoc ninf nsz arcp contract afn double %tmp348, %tmp309
  %tmp350 = fmul reassoc ninf nsz arcp contract afn double %tmp349, %tmp310
  %tmp351 = fmul reassoc ninf nsz arcp contract afn double %tmp350, %tmp311
  %tmp352 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %tmp196, i64 %tmp197, ptr nonnull elementtype(double) %tmp198, i64 %tmp330)
  %tmp353 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp194, i64 %tmp195, ptr elementtype(double) %tmp352, i64 1)
  %tmp354 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp193, i64 8, ptr elementtype(double) %tmp353, i64 3)
  %tmp355 = load double, ptr %tmp354, align 8, !tbaa !7
  %tmp356 = fmul reassoc ninf nsz arcp contract afn double %tmp351, %tmp355
  %tmp357 = fmul reassoc ninf nsz arcp contract afn double %tmp356, %tmp315
  %tmp358 = fmul reassoc ninf nsz arcp contract afn double %tmp357, %tmp316
  %tmp359 = fmul reassoc ninf nsz arcp contract afn double %tmp358, %tmp317
  %tmp360 = fmul reassoc ninf nsz arcp contract afn double %tmp359, %tmp318
  %tmp361 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp332)
  %tmp362 = load double, ptr %tmp361, align 8
  %tmp363 = fmul reassoc ninf nsz arcp contract afn double %tmp360, %tmp362
  %tmp364 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) null, i64 %tmp330)
  %tmp365 = load double, ptr %tmp364, align 8
  %tmp366 = fmul reassoc ninf nsz arcp contract afn double %tmp321, %tmp365
  %tmp367 = fmul reassoc ninf nsz arcp contract afn double %tmp366, %tmp322
  %tmp368 = fmul reassoc ninf nsz arcp contract afn double %tmp367, %tmp323
  %tmp369 = fmul reassoc ninf nsz arcp contract afn double %tmp368, %tmp325
  %tmp370 = fmul reassoc ninf nsz arcp contract afn double %tmp369, %tmp326
  %tmp371 = fmul reassoc ninf nsz arcp contract afn double %tmp370, %tmp327
  %tmp372 = fmul reassoc ninf nsz arcp contract afn double %tmp371, %tmp328
  %tmp373 = fdiv reassoc ninf nsz arcp contract afn double %tmp363, %tmp372
  %tmp374 = fadd reassoc ninf nsz arcp contract afn double %tmp373, %tmp338
  store double %tmp374, ptr %tmp337, align 8, !tbaa !9
  %tmp375 = add nuw nsw i64 %tmp330, 1
  %tmp376 = icmp sgt i64 %tmp375, %tmp299
  br i1 %tmp376, label %bb377, label %bb329

bb377:                                            ; preds = %bb329
  %tmp378 = add nuw nsw i64 %tmp296, 1
  %tmp379 = add nuw nsw i32 %tmp297, 1
  %tmp380 = icmp sgt i64 %tmp378, %tmp286
  br i1 %tmp380, label %bb381, label %bb295

bb381:                                            ; preds = %bb377
  %tmp382 = add nuw nsw i64 %tmp285, 1
  %tmp383 = icmp ugt i64 %tmp382, %tmp283
  br i1 %tmp383, label %bb384, label %bb284

bb384:                                            ; preds = %bb381
  %tmp385 = add nuw nsw i64 %tmp268, 1
  %tmp386 = icmp ugt i64 %tmp385, %tmp266
  br i1 %tmp386, label %bb387, label %bb267

bb387:                                            ; preds = %bb384
  %tmp388 = add nuw nsw i64 %tmp257, 1
  %tmp389 = add nuw nsw i32 %tmp258, 1
  %tmp390 = icmp ugt i64 %tmp388, %tmp255
  br i1 %tmp390, label %bb391, label %bb256

bb391:                                            ; preds = %bb387
  %tmp392 = add nuw nsw i64 %tmp246, 1
  %tmp393 = add nuw nsw i32 %tmp247, 1
  %tmp394 = icmp sgt i64 %tmp392, %tmp235
  br i1 %tmp394, label %bb395, label %bb245

bb395:                                            ; preds = %bb391
  %tmp396 = add nuw nsw i64 %tmp232, 1
  %tmp397 = icmp sgt i64 %tmp396, %tmp225
  br i1 %tmp397, label %bb398, label %bb231

bb398:                                            ; preds = %bb395
  %tmp399 = add nuw nsw i64 %tmp222, 1
  %tmp400 = add nuw nsw i32 %tmp223, 1
  %tmp401 = icmp eq i64 %tmp399, %tmp220
  br i1 %tmp401, label %bb402, label %bb221

bb402:                                            ; preds = %bb398
  %tmp403 = add nuw nsw i64 %tmp212, 1
  %tmp404 = add i32 %tmp213, -1
  %tmp405 = icmp eq i64 %tmp403, %tmp210
  br i1 %tmp405, label %bb406, label %bb211

bb406:                                            ; preds = %bb402
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.floor.f64(double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.ceil.f64(double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.exp.f64(double) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #4 = { nounwind }

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$696", !2, i64 0}
!2 = !{!"Fortran Data Symbol", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$18$qs_collocate_densitycollocate_pgf_product_rspace_mp_collocate_general_opt_"}
!5 = !{!6, !6, i64 0}
!6 = !{!"ifx$unique_sym$727", !2, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$707", !2, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$701", !2, i64 0}
