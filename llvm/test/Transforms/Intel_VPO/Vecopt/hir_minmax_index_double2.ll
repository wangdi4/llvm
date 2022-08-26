; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s

@g = global i32 0
@a = global i32 0

define i32 @main() {
; CHECK:       BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, {{.*}}, 4   <DO_LOOP>
; CHECK-NEXT:           |   [[DOTVEC0:%.*]] = [[PHI_TEMP100:%.*]] > 0.000000e+00
; CHECK-NEXT:           |   [[DOTVEC160:%.*]] = ([[PHI_TEMP100]] > 0.000000e+00) ? i1 + <i64 0, i64 1, i64 2, i64 3> : [[PHI_TEMP120:%.*]];
; CHECK-NEXT:           |   [[DOTVEC170:%.*]] = ([[PHI_TEMP100]] > 0.000000e+00) ? i1 + sext.i32.i64([[TMP3:%.*]]) + <i64 0, i64 1, i64 2, i64 3> : [[PHI_TEMP80:%.*]];
; CHECK-NEXT:           |   [[DOTVEC180:%.*]] = ([[PHI_TEMP100]] > 0.000000e+00) ? 0.000000e+00 : [[PHI_TEMP100]]
; CHECK-NEXT:           |   [[DOTVEC190:%.*]] = [[PHI_TEMP60:%.*]] > 0.000000e+00
; CHECK-NEXT:           |   [[DOTVEC200:%.*]] = ([[PHI_TEMP60]] > 0.000000e+00) ? i1 + <i64 0, i64 1, i64 2, i64 3> : [[PHI_TEMP140:%.*]];
; CHECK-NEXT:           |   [[DOTVEC210:%.*]] = ([[PHI_TEMP60]] > 0.000000e+00) ? i1 + sext.i32.i64([[TMP3]]) + <i64 0, i64 1, i64 2, i64 3> : [[PHI_TEMP0:%.*]];
; CHECK-NEXT:           |   [[DOTVEC220:%.*]] = ([[PHI_TEMP60]] > 0.000000e+00) ? 0.000000e+00 : [[PHI_TEMP60]]
; CHECK-NEXT:           |   [[PHI_TEMP0]] = [[DOTVEC210]]
; CHECK-NEXT:           |   [[PHI_TEMP60]] = [[DOTVEC220]]
; CHECK-NEXT:           |   [[PHI_TEMP80]] = [[DOTVEC170]]
; CHECK-NEXT:           |   [[PHI_TEMP100]] = [[DOTVEC180]]
; CHECK-NEXT:           |   [[PHI_TEMP120]] = [[DOTVEC160]]
; CHECK-NEXT:           |   [[PHI_TEMP140]] = [[DOTVEC200]]
; CHECK-NEXT:           + END LOOP
;
; CHECK:                [[TMP13:%.*]] = @llvm.vector.reduce.fmin.v4f64([[DOTVEC180]])
; CHECK-NEXT:           [[IDX_BLEND0:%.*]] = ([[TMP13]] == [[DOTVEC180]]) ? [[DOTVEC160]] : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>
; CHECK-NEXT:           [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.smin.v4i64([[IDX_BLEND0]])
; CHECK-NEXT:           [[MMIDX_CMP_0:%.*]] = [[VEC_REDUCE0]] == [[DOTVEC160]]
; CHECK-NEXT:           [[BSFINTMASK0:%.*]] = bitcast.<4 x i1>.i4([[MMIDX_CMP_0]])
; CHECK-NEXT:           [[BSF0:%.*]] = @llvm.cttz.i4([[BSFINTMASK0]],  1)
; CHECK-NEXT:           [[TMP10:%.*]] = extractelement [[DOTVEC170]],  [[BSF0]]
; CHECK-NEXT:           [[TMP12:%.*]] = @llvm.vector.reduce.fmin.v4f64([[DOTVEC220]])
; CHECK-NEXT:           [[IDX_BLEND300:%.*]] = ([[TMP12]] == [[DOTVEC220]]) ? [[DOTVEC200]] : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>
; CHECK-NEXT:           [[VEC_REDUCE310:%.*]] = @llvm.vector.reduce.smin.v4i64([[IDX_BLEND300]])
; CHECK-NEXT:           [[MMIDX_CMP_320:%.*]] = [[VEC_REDUCE310]] == [[DOTVEC200]]
; CHECK-NEXT:           [[BSFINTMASK330:%.*]] = bitcast.<4 x i1>.i4([[MMIDX_CMP_320]])
; CHECK-NEXT:           [[BSF340:%.*]] = @llvm.cttz.i4([[BSFINTMASK330]],  1)
; CHECK-NEXT:           [[TMP9:%.*]] = extractelement [[DOTVEC210]],  [[BSF340]]
; CHECK:             }
;
; CHECK:             + DO i1 = {{.*}}, sext.i32.i64([[TMP2:%.*]]) + -1 * sext.i32.i64([[TMP3]]) + -1, 1   <DO_LOOP>
; CHECK-NEXT:        |   [[TMP10]] = ([[TMP13]] > 0.000000e+00) ? i1 + sext.i32.i64([[TMP3]]) : [[TMP10]]
; CHECK-NEXT:        |   [[TMP13]] = ([[TMP13]] > 0.000000e+00) ? 0.000000e+00 : [[TMP13]]
; CHECK-NEXT:        |   [[TMP9]] = ([[TMP12]] > 0.000000e+00) ? i1 + sext.i32.i64([[TMP3]]) : [[TMP9]]
; CHECK-NEXT:        |   [[TMP12]] = ([[TMP12]] > 0.000000e+00) ? 0.000000e+00 : [[TMP12]]
; CHECK-NEXT:        + END LOOP
; CHECK:       END REGION
;
  %1 = tail call i32 (i32*, i8*, ...) @fscanf(i32* undef, i8* undef)
  %2 = load i32, i32* @a
  %3 = load i32, i32* @g
  %4 = icmp slt i32 %3, %2
  br i1 %4, label %5, label %27

5:                                                ; preds = %0
  %6 = sext i32 %3 to i64
  %7 = sext i32 %2 to i64
  br label %8

8:                                                ; preds = %8, %5
  %9 = phi i32 [ 1, %5 ], [ %20, %8 ]
  %10 = phi i32 [ 1, %5 ], [ %16, %8 ]
  %11 = phi i64 [ %6, %5 ], [ %22, %8 ]
  %12 = phi double [ undef, %5 ], [ %21, %8 ]
  %13 = phi double [ undef, %5 ], [ %17, %8 ]
  %14 = fcmp fast ogt double %13, 0.000000e+00
  %15 = trunc i64 %11 to i32
  %16 = select i1 %14, i32 %15, i32 %10
  %17 = select i1 %14, double 0.000000e+00, double %13
  %18 = fcmp fast ogt double %12, 0.000000e+00
  %19 = trunc i64 %11 to i32
  %20 = select i1 %18, i32 %19, i32 %9
  %21 = select i1 %18, double 0.000000e+00, double %12
  %22 = add nsw i64 %11, 1
  %23 = icmp eq i64 %22, %7
  br i1 %23, label %24, label %8

24:                                               ; preds = %8
  %25 = phi i32 [ %16, %8 ]
  %26 = phi i32 [ %20, %8 ]
  store i32 %2, i32* @g, align 4
  br label %27

27:
  ret i32 0
}

declare i32 @fscanf(i32*, i8*, ...)
