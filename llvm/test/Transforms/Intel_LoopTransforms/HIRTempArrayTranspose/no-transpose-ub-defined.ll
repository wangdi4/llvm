; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose" -print-changed -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Check that transposing does not occur for the i2 Loop.
; it has a ztt that is based on %tmp98, which is defined in i1 loop.
; Note: we could remove the ztt(), but that would change the control flow.

; ZTT for the i2 loop

;     |   + Ztt: if (%tmp98 == 0)
;     |   + NumExits: 1
;     |   + Innermost: No
;     |   + HasSignedIV: Yes
;     |   + LiveIn symbases: 11, 19, 26, 28, 30, 33, 35, 40, 44, 49
;     |   + LiveOut symbases:
;     |   + Loop metadata: No
;     |   + DO i64 i2 = 0, sext.i32.i64(%tmp50) + -1, 1   <DO_LOOP>

;  BEGIN REGION { }
;        + DO i1 = 0, zext.i32.i64(%tmp70) + -1, 1   <DO_LOOP>
;        |   %tmp95 = (%tmp75)[i1 + 1];
;        |   %tmp98 = (0 != %tmp95) ? -1 : %tmp77;
;        |
;        |      %tmp143 = (null)[0];
;        |      %tmp144 = (null)[0];
;        |   + DO i2 = 0, sext.i32.i64(%tmp50) + -1, 1   <DO_LOOP>
;        |   |   if (undef true undef)
;        |   |   {
;        |   |      %tmp102 = (null)[0];
;        |   |      %tmp120.in = (%arg1)[i1][%tmp102];
;        |   |
;        |   |      + DO i3 = 0, sext.i32.i64(%tmp78) + -1, 1   <DO_LOOP>
;        |   |      |   %tmp126 = 0.000000e+00  *  (%tmp83)[i3][i2];
;        |   |      |   %tmp120.in = 0.000000e+00;
;        |   |      + END LOOP
;        |   |   }
;        |   + END LOOP
;        + END LOOP
;  END REGION

; CHECK:  BEGIN REGION
; CHECK-NOT: modified

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRTempArrayTranspose

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wombat.0 = type { i32, i32, %struct.wombat, i32, [10 x i32], %struct.quux.1, %struct.quux.1, %struct.barney }
%struct.wombat = type <{ %struct.barney, %struct.quux, %struct.zot, i32, i32, i32, i32, i32, i32 }>
%struct.quux = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.eggs = type { double, double }
%struct.zot = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%struct.quux.1 = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%struct.barney = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@global.1 = external global i32

; Function Attrs: nofree nounwind uwtable
define void @bar(ptr nocapture readonly %arg, ptr noalias nocapture dereferenceable(8) %arg1) #0 {
bb:
  br label %bb16

bb16:                                             ; preds = %bb
  %tmp34 = getelementptr inbounds %struct.wombat.0, ptr %arg, i64 0, i32 2, i32 0, i32 6, i64 0, i32 2
  %tmp36 = getelementptr inbounds %struct.wombat.0, ptr %arg, i64 0, i32 2, i32 0, i32 6, i64 0, i32 1
  %tmp37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp36, i32 1)
  %tmp38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp34, i32 1)
  %tmp39 = getelementptr inbounds %struct.wombat.0, ptr %arg, i64 0, i32 7, i32 0
  %tmp40 = getelementptr inbounds %struct.wombat.0, ptr %arg, i64 0, i32 7, i32 6, i64 0, i32 2
  %tmp42 = getelementptr inbounds %struct.wombat.0, ptr %arg, i64 0, i32 7, i32 6, i64 0, i32 1
  %tmp43 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp42, i32 1)
  %tmp44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp40, i32 1)
  br label %bb45

bb45:                                             ; preds = %bb16
  %tmp50 = load i32, ptr null, align 4
  br label %bb52

bb52:                                             ; preds = %bb45
  %tmp70 = load i32, ptr @global.1, align 8
  br label %bb74

bb74:                                             ; preds = %bb52
  %tmp75 = load ptr, ptr null, align 8
  %tmp76 = load i64, ptr null, align 8
  %tmp77 = icmp slt i32 %tmp50, 1
  %tmp78 = load i32, ptr getelementptr inbounds (%struct.wombat, ptr null, i64 0, i32 7), align 8
  %tmp83 = load ptr, ptr null, align 8
  %tmp85 = add nuw nsw i32 %tmp78, 1
  %tmp86 = add nuw nsw i32 %tmp50, 1
  %tmp87 = add nuw nsw i32 %tmp70, 1
  %tmp88 = zext i32 %tmp87 to i64
  %tmp90 = sext i32 %tmp86 to i64
  %tmp91 = sext i32 %tmp85 to i64
  br label %bb92

bb92:                                             ; preds = %bb150, %bb74
  %tmp93 = phi i64 [ 1, %bb74 ], [ %tmp151, %bb150 ]
  %tmp94 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp76, i64 4, ptr elementtype(i32) %tmp75, i64 %tmp93)
  %tmp95 = load i32, ptr %tmp94, align 1
  %tmp96 = zext i32 %tmp95 to i64
  %tmp97 = icmp ne i64 0, %tmp96
  %tmp98 = select i1 %tmp97, i1 true, i1 %tmp77
  br i1 %tmp98, label %bb150, label %bb142

bb99:                                             ; preds = %bb142, %bb139
  %tmp100 = phi i64 [ 1, %bb142 ], [ %tmp140, %bb139 ]
  br i1 false, label %bb139, label %bb101

bb101:                                            ; preds = %bb99
  %tmp102 = load i32, ptr null, align 8
  %tmp103 = sext i32 %tmp102 to i64
  %tmp104 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %tmp147, i64 %tmp103)
  %tmp105 = load ptr, ptr null, align 8
  %tmp106 = load i64, ptr null, align 8
  %tmp107 = add nsw i64 %tmp100, %tmp148
  %tmp108 = load i64, ptr %tmp37, align 8
  %tmp109 = load i64, ptr %tmp38, align 8
  %tmp110 = load ptr, ptr %tmp39, align 8
  %tmp111 = load i64, ptr null, align 8
  %tmp112 = load i64, ptr %tmp43, align 8
  %tmp113 = load i64, ptr %tmp44, align 8
  %tmp114 = load i32, ptr null, align 8
  %tmp115 = sext i32 %tmp114 to i64
  %tmp116 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %tmp113, i64 %tmp112, ptr elementtype(double) %tmp110, i64 %tmp115)
  %tmp117 = load double, ptr %tmp104, align 1
  br label %bb118

bb118:                                            ; preds = %bb118, %bb101
  %tmp119 = phi i64 [ 1, %bb101 ], [ %tmp135, %bb118 ]
  %tmp120 = phi double [ %tmp117, %bb101 ], [ 0.000000e+00, %bb118 ]
  %tmp121 = add nsw i64 %tmp119, 0
  %tmp122 = add nsw i64 %tmp121, -1
  %tmp123 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 0, ptr elementtype(double) %tmp83, i64 %tmp119)
  %tmp124 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %tmp123, i64 %tmp100)
  %tmp125 = load double, ptr %tmp124, align 8
  %tmp126 = fmul reassoc ninf nsz arcp contract afn double 0.000000e+00, %tmp125
  %tmp135 = add nuw nsw i64 %tmp119, 1
  %tmp136 = icmp eq i64 %tmp135, %tmp91
  br i1 %tmp136, label %bb137, label %bb118

bb137:                                            ; preds = %bb118
  br label %bb139

bb139:                                            ; preds = %bb137, %bb99
  %tmp140 = add nuw nsw i64 %tmp100, 1
  %tmp141 = icmp eq i64 %tmp140, %tmp90
  br i1 %tmp141, label %bb149, label %bb99

bb142:                                            ; preds = %bb92
  %tmp143 = load i64, ptr null, align 8
  %tmp144 = load ptr, ptr null, align 8
  %tmp145 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %tmp143, i64 4, ptr elementtype(i32) %tmp144, i64 %tmp93)
  %tmp146 = load i32, ptr %tmp145, align 1
  %tmp147 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %arg1, i64 %tmp93)
  %tmp148 = sext i32 %tmp146 to i64
  br label %bb99

bb149:                                            ; preds = %bb139
  br label %bb150

bb150:                                            ; preds = %bb149, %bb92
  %tmp151 = add nuw nsw i64 %tmp93, 1
  %tmp152 = icmp eq i64 %tmp151, %tmp88
  br i1 %tmp152, label %bb153, label %bb92

bb153:                                            ; preds = %bb150
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #2

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3
