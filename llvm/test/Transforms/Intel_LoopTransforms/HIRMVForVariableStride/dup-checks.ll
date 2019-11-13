; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-mv-variable-stride -print-before=hir-mv-variable-stride -print-after=hir-mv-variable-stride -hir-details-refs  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-mv-variable-stride,print<hir>" -hir-details-refs -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check if duplicate checks are not generated with unrolled input.

; CHECK: Function: step2d_tile_
; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, %div421 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:              |   if (%"step2d_tile_$M2_fetch" + 1 >= %"step2d_tile_$L2_fetch" + -1)
; CHECK:              |   {
; CHECK:              |      + DO i2 = 0, -1 * sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + smax(sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")), (1 + sext.i32.i64(%"step2d_tile_$M2_fetch"))), 1   <DO_LOOP>
; CHECK:              |      + END LOOP
; CHECK:              |   }
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK: Function: step2d_tile_
; CHECK:        BEGIN REGION { }
; CHECK:              if (%func_result435_fetch == 8 && %func_result483_fetch == 8)
; CHECK:              {
; CHECK:                 + DO i1 = 0, %div421 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:                 |   if (%"step2d_tile_$M2_fetch" + 1 >= %"step2d_tile_$L2_fetch" + -1)
; CHECK:                 |   {
; CHECK:                 |      + DO i2 = 0, -1 * sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + smax(sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")), (1 + sext.i32.i64(%"step2d_tile_$M2_fetch"))), 1   <DO_LOOP>
; CHECK:                 |      + END LOOP
; CHECK:                 |   }
; CHECK:                 + END LOOP
; CHECK:              }
; CHECK:              else
; CHECK:              {
; CHECK:                 + DO i1 = 0, %div421 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:                 |   if (%"step2d_tile_$M2_fetch" + 1 >= %"step2d_tile_$L2_fetch" + -1)
; CHECK:                 |   {
; CHECK:                 |      + DO i2 = 0, -1 * sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + smax(sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")), (1 + sext.i32.i64(%"step2d_tile_$M2_fetch"))), 1   <DO_LOOP>
; CHECK:                 |      + END LOOP
; CHECK:                 |   }
; CHECK:                 + END LOOP
; CHECK:              }
; CHECK:        END REGION

; Following shows hir-details-refs input.
;        BEGIN REGION { }
;              + DO i1 = 0, %div421 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;              |   if (%"step2d_tile_$M2_fetch" + 1 >= %"step2d_tile_$L2_fetch" + -1)
;              |   {
;              |      + DO i2 = 0, -1 * sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + smax(sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")), (1 + sext.i32.i64(%"step2d_tile_$M2_fetch"))), 1   <DO_LOOP>
;              |      |   %add525 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:1:%func_result451_fetch(double*:0)][%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393):%func_result443_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result435_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393):%func_result493_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result483_fetch(double*:0)];
;              |      |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %add525;
;              |      |   %mul567 = (%"step2d_tile_$OM_V")[1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add605 = %add525  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + -1:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)];
;              |      |   %mul607 = %mul567  *  %add605;
;              |      |   %mul625 = (%"step2d_tile_$VBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul607;
;              |      |   (%"step2d_tile_$DVOM")[1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul625;
;              |      |   %mul656 = (%"step2d_tile_$ON_U")[1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add694 = %add525  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + -1:8(double*:0)];
;              |      |   %mul696 = %mul656  *  %add694;
;              |      |   %mul717 = (%"step2d_tile_$UBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul696;
;              |      |   (%"step2d_tile_$DUON")[1:4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul717;
;              |      |   %add844 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:1:%func_result451_fetch(double*:0)][%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 1:%func_result443_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result435_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 1:%func_result493_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result483_fetch(double*:0)];
;              |      |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %add844;
;              |      |   %mul875 = (%"step2d_tile_$OM_V")[1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add913 = %add844  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)];
;              |      |   %mul915 = %mul875  *  %add913;
;              |      |   %mul936 = (%"step2d_tile_$VBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul915;
;              |      |   (%"step2d_tile_$DVOM")[1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul936;
;              |      |   %mul967 = (%"step2d_tile_$ON_U")[1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add1005 = %add844  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + -1:8(double*:0)];
;              |      |   %mul1007 = %mul967  *  %add1005;
;              |      |   %mul1028 = (%"step2d_tile_$UBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul1007;
;              |      |   (%"step2d_tile_$DUON")[1:4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul1028;
;              |      |   %add1155 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:1:%func_result451_fetch(double*:0)][%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 2:%func_result443_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result435_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 2:%func_result493_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result483_fetch(double*:0)];
;              |      |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %add1155;
;              |      |   %mul1186 = (%"step2d_tile_$OM_V")[1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add1224 = %add1155  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 1:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)];
;              |      |   %mul1226 = %mul1186  *  %add1224;
;              |      |   %mul1247 = (%"step2d_tile_$VBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul1226;
;              |      |   (%"step2d_tile_$DVOM")[1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul1247;
;              |      |   %mul1278 = (%"step2d_tile_$ON_U")[1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add1316 = %add1155  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + -1:8(double*:0)];
;              |      |   %mul1318 = %mul1278  *  %add1316;
;              |      |   %mul1339 = (%"step2d_tile_$UBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul1318;
;              |      |   (%"step2d_tile_$DUON")[1:4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul1339;
;              |      |   %add1466 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:1:%func_result451_fetch(double*:0)][%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 3:%func_result443_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result435_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 3:%func_result493_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):%func_result483_fetch(double*:0)];
;              |      |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %add1466;
;              |      |   %mul1497 = (%"step2d_tile_$OM_V")[1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add1535 = %add1466  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 2:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)];
;              |      |   %mul1537 = %mul1497  *  %add1535;
;              |      |   %mul1558 = (%"step2d_tile_$VBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul1537;
;              |      |   (%"step2d_tile_$DVOM")[1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul1558;
;              |      |   %mul1589 = (%"step2d_tile_$ON_U")[1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  5.000000e-01;
;              |      |   %add1627 = %add1466  +  (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")) + -1:8(double*:0)];
;              |      |   %mul1629 = %mul1589  *  %add1627;
;              |      |   %mul1650 = (%"step2d_tile_$UBAR")[1:1:8 * (sext.i32.i64(%"step2d_tile_$M2_fetch") * sext.i32.i64(%"step2d_tile_$M1_fetch"))(double*:0)][1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)]  *  %mul1629;
;              |      |   (%"step2d_tile_$DUON")[1:4 * i1 + sext.i32.i64(%add393) + 3:8 * sext.i32.i64(%"step2d_tile_$M1_fetch")(double*:0)][1:i2 + sext.i32.i64((-1 + %"step2d_tile_$L2_fetch")):8(double*:0)] = %mul1650;
;              |      + END LOOP
;              |   }
;              + END LOOP
;        END REGION

; ModuleID = 'small.step2d.ll'
source_filename = "small.step2d.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly
define void @step2d_tile_({ double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias nocapture dereferenceable(120) %"step2d_tile_$ZETA", { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture dereferenceable(96) %"step2d_tile_$H", i32* noalias nocapture %"step2d_tile_$L1", i32* noalias nocapture %"step2d_tile_$L2", i32* noalias nocapture %"step2d_tile_$U1", i32* noalias nocapture %"step2d_tile_$U2", i32* noalias nocapture %"step2d_tile_$M1", i32* noalias nocapture %"step2d_tile_$M2", i32* noalias nocapture %"step2d_tile_$M3") local_unnamed_addr #0 {
alloca:
  %"step2d_tile_$ZETA_$field0$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"step2d_tile_$ZETA", i64 0, i32 0
  %"step2d_tile_$ZETA_$field6$_$field1$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"step2d_tile_$ZETA", i64 0, i32 6, i64 0, i32 1
  %"step2d_tile_$H_$field0$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"step2d_tile_$H", i64 0, i32 0
  %"step2d_tile_$H_$field6$_$field1$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"step2d_tile_$H", i64 0, i32 6, i64 0, i32 1
  %"step2d_tile_$L1_fetch" = load i32, i32* %"step2d_tile_$L1", align 4
  %"step2d_tile_$L2_fetch" = load i32, i32* %"step2d_tile_$L2", align 4
  %"step2d_tile_$U1_fetch" = load i32, i32* %"step2d_tile_$U1", align 4
  %"step2d_tile_$U2_fetch" = load i32, i32* %"step2d_tile_$U2", align 4
  %"step2d_tile_$M1_fetch" = load i32, i32* %"step2d_tile_$M1", align 4
  %"step2d_tile_$M2_fetch" = load i32, i32* %"step2d_tile_$M2", align 4
  %int_sext = sext i32 %"step2d_tile_$M1_fetch" to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %mul = shl nsw i64 %slct, 3
  %int_sext2 = sext i32 %"step2d_tile_$M2_fetch" to i64
  %rel4 = icmp sgt i64 %int_sext2, 0
  %slct6 = select i1 %rel4, i64 %int_sext2, i64 0
  %mul8 = mul nsw i64 %mul, %slct6
  %div = lshr exact i64 %mul8, 3
  %"step2d_tile_$DUON" = alloca double, i64 %div, align 8
  %"step2d_tile_$ON_U" = alloca double, i64 %div, align 8
  %mul52 = mul nuw nsw i64 %mul8, 100
  %div54 = lshr exact i64 %mul52, 3
  %"step2d_tile_$VBAR" = alloca double, i64 %div54, align 8
  %"step2d_tile_$UBAR" = alloca double, i64 %div54, align 8
  %"step2d_tile_$DVOM" = alloca double, i64 %div, align 8
  %"step2d_tile_$OM_V" = alloca double, i64 %div, align 8
  %sub = sub i32 1, %"step2d_tile_$L1_fetch"
  %add = add i32 %sub, %"step2d_tile_$U1_fetch"
  %int_sext124 = sext i32 %add to i64
  %rel126 = icmp sgt i64 %int_sext124, 0
  %slct128 = select i1 %rel126, i64 %int_sext124, i64 0
  %mul130 = shl nsw i64 %slct128, 3
  %sub132 = sub i32 1, %"step2d_tile_$L2_fetch"
  %add134 = add i32 %sub132, %"step2d_tile_$U2_fetch"
  %int_sext136 = sext i32 %add134 to i64
  %rel138 = icmp sgt i64 %int_sext136, 0
  %slct140 = select i1 %rel138, i64 %int_sext136, i64 0
  %mul142 = mul nsw i64 %mul130, %slct140
  %div144 = lshr exact i64 %mul142, 3
  %"step2d_tile_$DRHS" = alloca double, i64 %div144, align 8
  %mul147 = shl nsw i64 %int_sext, 3
  %mul220 = mul nsw i64 %mul147, %int_sext2
  %mul531 = shl nsw i64 %int_sext124, 3
  %rel1672091 = icmp slt i32 %"step2d_tile_$M2_fetch", 1
  br i1 %rel1672091, label %bb73.preheader, label %bb23.preheader.lr.ph

bb23.preheader.lr.ph:                             ; preds = %alloca
  %rel1592089 = icmp slt i32 %"step2d_tile_$M1_fetch", 1
  br label %bb23.preheader

bb22:                                             ; preds = %bb22.lr.ph, %bb22
  %"var$10.02090" = phi i64 [ 1, %bb22.lr.ph ], [ %add163, %bb22 ]
  %func_result155 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result153, i64 %"var$10.02090")
  store double 2.000000e+00, double* %func_result155, align 8
  %add163 = add nuw nsw i64 %"var$10.02090", 1
  %exitcond2112 = icmp eq i64 %"var$10.02090", %int_sext
  br i1 %exitcond2112, label %bb25.loopexit, label %bb22

bb25.loopexit:                                    ; preds = %bb22
  br label %bb25

bb25:                                             ; preds = %bb23.preheader, %bb25.loopexit
  %add171 = add nuw nsw i64 %"var$11.02092", 1
  %exitcond2113 = icmp eq i64 %"var$11.02092", %int_sext2
  br i1 %exitcond2113, label %bb47.preheader, label %bb23.preheader

bb23.preheader:                                   ; preds = %bb25, %bb23.preheader.lr.ph
  %"var$11.02092" = phi i64 [ 1, %bb23.preheader.lr.ph ], [ %add171, %bb25 ]
  br i1 %rel1592089, label %bb25, label %bb22.lr.ph

bb22.lr.ph:                                       ; preds = %bb23.preheader
  %func_result153 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$OM_V", i64 %"var$11.02092")
  br label %bb22

bb47.preheader:                                   ; preds = %bb25
  br i1 %rel1672091, label %bb73.preheader, label %bb43.preheader.lr.ph

bb43.preheader.lr.ph:                             ; preds = %bb47.preheader
  %rel1942084 = icmp slt i32 %"step2d_tile_$M1_fetch", 1
  br label %bb43.preheader

bb42:                                             ; preds = %bb42.lr.ph, %bb42
  %"var$14.02085" = phi i64 [ 1, %bb42.lr.ph ], [ %add198, %bb42 ]
  %func_result190 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result188, i64 %"var$14.02085")
  store double 3.000000e+00, double* %func_result190, align 8
  %add198 = add nuw nsw i64 %"var$14.02085", 1
  %exitcond2110 = icmp eq i64 %"var$14.02085", %int_sext
  br i1 %exitcond2110, label %bb45.loopexit, label %bb42

bb45.loopexit:                                    ; preds = %bb42
  br label %bb45

bb45:                                             ; preds = %bb43.preheader, %bb45.loopexit
  %add206 = add nuw nsw i64 %"var$15.02087", 1
  %exitcond2111 = icmp eq i64 %"var$15.02087", %int_sext2
  br i1 %exitcond2111, label %bb73.preheader.loopexit, label %bb43.preheader

bb43.preheader:                                   ; preds = %bb45, %bb43.preheader.lr.ph
  %"var$15.02087" = phi i64 [ 1, %bb43.preheader.lr.ph ], [ %add206, %bb45 ]
  br i1 %rel1942084, label %bb45, label %bb42.lr.ph

bb42.lr.ph:                                       ; preds = %bb43.preheader
  %func_result188 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DVOM", i64 %"var$15.02087")
  br label %bb42

bb73.preheader.loopexit:                          ; preds = %bb45
  br label %bb73.preheader

bb73.preheader:                                   ; preds = %bb73.preheader.loopexit, %bb47.preheader, %alloca
  %rel2372077 = icmp slt i32 %"step2d_tile_$M1_fetch", 1
  br label %bb69.preheader

bb64:                                             ; preds = %bb64.lr.ph, %bb64
  %"var$18.02078" = phi i64 [ 1, %bb64.lr.ph ], [ %add241, %bb64 ]
  %func_result233 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result231, i64 %"var$18.02078")
  store double 4.000000e+00, double* %func_result233, align 8
  %add241 = add nuw nsw i64 %"var$18.02078", 1
  %exitcond2107 = icmp eq i64 %"var$18.02078", %int_sext
  br i1 %exitcond2107, label %bb67.loopexit, label %bb64

bb67.loopexit:                                    ; preds = %bb64
  br label %bb67

bb67:                                             ; preds = %bb65.preheader, %bb67.loopexit
  %add249 = add nuw nsw i64 %"var$19.02080", 1
  %exitcond2108 = icmp eq i64 %"var$19.02080", %int_sext2
  br i1 %exitcond2108, label %bb71.loopexit, label %bb65.preheader

bb65.preheader:                                   ; preds = %bb65.preheader.lr.ph, %bb67
  %"var$19.02080" = phi i64 [ 1, %bb65.preheader.lr.ph ], [ %add249, %bb67 ]
  br i1 %rel2372077, label %bb67, label %bb64.lr.ph

bb64.lr.ph:                                       ; preds = %bb65.preheader
  %func_result231 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result229, i64 %"var$19.02080")
  br label %bb64

bb71.loopexit:                                    ; preds = %bb67
  br label %bb71

bb71:                                             ; preds = %bb69.preheader, %bb71.loopexit
  %add257 = add nuw nsw i64 %"var$20.02082", 1
  %exitcond2109 = icmp eq i64 %add257, 101
  br i1 %exitcond2109, label %bb95.preheader.preheader, label %bb69.preheader

bb95.preheader.preheader:                         ; preds = %bb71
  br label %bb95.preheader

bb69.preheader:                                   ; preds = %bb71, %bb73.preheader
  %"var$20.02082" = phi i64 [ 1, %bb73.preheader ], [ %add257, %bb71 ]
  br i1 %rel1672091, label %bb71, label %bb65.preheader.lr.ph

bb65.preheader.lr.ph:                             ; preds = %bb69.preheader
  %func_result229 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul220, double* nonnull %"step2d_tile_$UBAR", i64 %"var$20.02082")
  br label %bb65.preheader

bb90:                                             ; preds = %bb90.lr.ph, %bb90
  %"var$24.02071" = phi i64 [ 1, %bb90.lr.ph ], [ %add293, %bb90 ]
  %func_result285 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result283, i64 %"var$24.02071")
  store double 5.000000e+00, double* %func_result285, align 8
  %add293 = add nuw nsw i64 %"var$24.02071", 1
  %exitcond2104 = icmp eq i64 %"var$24.02071", %int_sext
  br i1 %exitcond2104, label %bb93.loopexit, label %bb90

bb93.loopexit:                                    ; preds = %bb90
  br label %bb93

bb93:                                             ; preds = %bb91.preheader, %bb93.loopexit
  %add301 = add nuw nsw i64 %"var$25.02073", 1
  %exitcond2105 = icmp eq i64 %"var$25.02073", %int_sext2
  br i1 %exitcond2105, label %bb97.loopexit, label %bb91.preheader

bb91.preheader:                                   ; preds = %bb91.preheader.lr.ph, %bb93
  %"var$25.02073" = phi i64 [ 1, %bb91.preheader.lr.ph ], [ %add301, %bb93 ]
  br i1 %rel2372077, label %bb93, label %bb90.lr.ph

bb90.lr.ph:                                       ; preds = %bb91.preheader
  %func_result283 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result281, i64 %"var$25.02073")
  br label %bb90

bb97.loopexit:                                    ; preds = %bb93
  br label %bb97

bb97:                                             ; preds = %bb95.preheader, %bb97.loopexit
  %add309 = add nuw nsw i64 %"var$26.02075", 1
  %exitcond2106 = icmp eq i64 %add309, 101
  br i1 %exitcond2106, label %bb119.preheader, label %bb95.preheader

bb95.preheader:                                   ; preds = %bb97, %bb95.preheader.preheader
  %"var$26.02075" = phi i64 [ %add309, %bb97 ], [ 1, %bb95.preheader.preheader ]
  br i1 %rel1672091, label %bb97, label %bb91.preheader.lr.ph

bb91.preheader.lr.ph:                             ; preds = %bb95.preheader
  %func_result281 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul220, double* nonnull %"step2d_tile_$VBAR", i64 %"var$26.02075")
  br label %bb91.preheader

bb119.preheader:                                  ; preds = %bb97
  br i1 %rel1672091, label %bb142, label %bb115.preheader.preheader

bb115.preheader.preheader:                        ; preds = %bb119.preheader
  br label %bb115.preheader

bb114:                                            ; preds = %bb114.lr.ph, %bb114
  %"var$30.02066" = phi i64 [ 1, %bb114.lr.ph ], [ %add337, %bb114 ]
  %func_result329 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result327, i64 %"var$30.02066")
  store double 6.000000e+00, double* %func_result329, align 8
  %add337 = add nuw nsw i64 %"var$30.02066", 1
  %exitcond2102 = icmp eq i64 %"var$30.02066", %int_sext
  br i1 %exitcond2102, label %bb117.loopexit, label %bb114

bb117.loopexit:                                   ; preds = %bb114
  br label %bb117

bb117:                                            ; preds = %bb115.preheader, %bb117.loopexit
  %add345 = add nuw nsw i64 %"var$31.02068", 1
  %exitcond2103 = icmp eq i64 %"var$31.02068", %int_sext2
  br i1 %exitcond2103, label %bb139.preheader, label %bb115.preheader

bb115.preheader:                                  ; preds = %bb117, %bb115.preheader.preheader
  %"var$31.02068" = phi i64 [ %add345, %bb117 ], [ 1, %bb115.preheader.preheader ]
  br i1 %rel2372077, label %bb117, label %bb114.lr.ph

bb114.lr.ph:                                      ; preds = %bb115.preheader
  %func_result327 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$ON_U", i64 %"var$31.02068")
  br label %bb114

bb139.preheader:                                  ; preds = %bb117
  br i1 %rel1672091, label %bb142, label %bb135.preheader.preheader

bb135.preheader.preheader:                        ; preds = %bb139.preheader
  br label %bb135.preheader

bb134:                                            ; preds = %bb134.lr.ph, %bb134
  %"var$34.02061" = phi i64 [ 1, %bb134.lr.ph ], [ %add372, %bb134 ]
  %func_result364 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result362, i64 %"var$34.02061")
  store double 7.000000e+00, double* %func_result364, align 8
  %add372 = add nuw nsw i64 %"var$34.02061", 1
  %exitcond = icmp eq i64 %"var$34.02061", %int_sext
  br i1 %exitcond, label %bb137.loopexit, label %bb134

bb137.loopexit:                                   ; preds = %bb134
  br label %bb137

bb137:                                            ; preds = %bb135.preheader, %bb137.loopexit
  %add380 = add nuw nsw i64 %"var$35.02063", 1
  %exitcond2101 = icmp eq i64 %"var$35.02063", %int_sext2
  br i1 %exitcond2101, label %bb142.loopexit, label %bb135.preheader

bb135.preheader:                                  ; preds = %bb137, %bb135.preheader.preheader
  %"var$35.02063" = phi i64 [ %add380, %bb137 ], [ 1, %bb135.preheader.preheader ]
  br i1 %rel2372077, label %bb137, label %bb134.lr.ph

bb134.lr.ph:                                      ; preds = %bb135.preheader
  %func_result362 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DUON", i64 %"var$35.02063")
  br label %bb134

bb142.loopexit:                                   ; preds = %bb137
  br label %bb142

bb142:                                            ; preds = %bb142.loopexit, %bb139.preheader, %bb119.preheader
  %rel391 = icmp slt i32 %"step2d_tile_$L1_fetch", 4
  %sub390.op = add i32 %"step2d_tile_$L1_fetch", -2
  %add393 = select i1 %rel391, i32 1, i32 %sub390.op
  %"step2d_tile_$M3_fetch" = load i32, i32* %"step2d_tile_$M3", align 4
  %rel395 = icmp sgt i32 %"step2d_tile_$M2_fetch", %"step2d_tile_$M3_fetch"
  %slct396 = select i1 %rel395, i32 %"step2d_tile_$M3_fetch", i32 %"step2d_tile_$M2_fetch"
  %sub399 = sub i32 2, %add393
  %add400 = add i32 %sub399, %slct396
  %div401 = sdiv i32 %add400, 4
  %sub402 = shl nsw i32 %div401, 2
  %mul404 = add i32 %add393, -4
  %add405 = add i32 %mul404, %sub402
  %int_sext411 = sext i32 %add393 to i64
  %int_sext413 = sext i32 %add405 to i64
  %sub417 = sub nsw i64 4, %int_sext411
  %add419 = add nsw i64 %sub417, %int_sext413
  %div421 = sdiv i64 %add419, 4
  %rel423 = icmp slt i64 %div421, 1
  br i1 %rel423, label %bb502, label %bb147.preheader

bb147.preheader:                                  ; preds = %bb142
  %add427 = add i32 %"step2d_tile_$L2_fetch", -1
  %add431 = add nsw i32 %"step2d_tile_$M2_fetch", 1
  %rel433 = icmp slt i32 %add431, %add427
  %func_result435 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 0)
  %int_sext439 = sext i32 %"step2d_tile_$L1_fetch" to i64
  %func_result443 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 1)
  %int_sext447 = sext i32 %"step2d_tile_$L2_fetch" to i64
  %func_result451 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 2)
  %func_result483 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$H_$field6$_$field1$", i32 0)
  %func_result493 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$H_$field6$_$field1$", i32 1)
  %func_result619 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul220, double* nonnull %"step2d_tile_$VBAR", i64 1)
  %func_result709 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul220, double* nonnull %"step2d_tile_$UBAR", i64 1)
  %0 = sext i32 %add427 to i64
  %"step2d_tile_$ZETA_$field0$_fetch" = load double*, double** %"step2d_tile_$ZETA_$field0$", align 8
  %func_result435_fetch = load i64, i64* %func_result435, align 8
  %func_result443_fetch = load i64, i64* %func_result443, align 8
  %func_result451_fetch = load i64, i64* %func_result451, align 8
  %func_result477 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %func_result451_fetch, double* %"step2d_tile_$ZETA_$field0$_fetch", i64 1)
  %"step2d_tile_$H_$field0$_fetch" = load double*, double** %"step2d_tile_$H_$field0$", align 8
  %func_result483_fetch = load i64, i64* %func_result483, align 8
  %func_result493_fetch = load i64, i64* %func_result493, align 8
  br label %bb147

bb147:                                            ; preds = %bb152, %bb147.preheader
  %indvars.iv2095 = phi i64 [ %int_sext411, %bb147.preheader ], [ %indvars.iv.next2096, %bb152 ]
  %"var$39.0" = phi i64 [ %div421, %bb147.preheader ], [ %sub1693, %bb152 ]
  br i1 %rel433, label %bb152, label %bb151.preheader

bb151.preheader:                                  ; preds = %bb147
  %func_result479 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result443_fetch, double* %func_result477, i64 %indvars.iv2095)
  %func_result521 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result493_fetch, double* %"step2d_tile_$H_$field0$_fetch", i64 %indvars.iv2095)
  %func_result559 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %mul531, double* nonnull %"step2d_tile_$DRHS", i64 %indvars.iv2095)
  %func_result563 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$OM_V", i64 %indvars.iv2095)
  %1 = add nsw i64 %indvars.iv2095, -1
  %func_result601 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %mul531, double* nonnull %"step2d_tile_$DRHS", i64 %1)
  %func_result621 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result619, i64 %indvars.iv2095)
  %func_result639 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DVOM", i64 %indvars.iv2095)
  %func_result652 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$ON_U", i64 %indvars.iv2095)
  %func_result711 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result709, i64 %indvars.iv2095)
  %func_result731 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DUON", i64 %indvars.iv2095)
  %2 = add nsw i64 %indvars.iv2095, 1
  %func_result796 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result443_fetch, double* %func_result477, i64 %2)
  %func_result840 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result493_fetch, double* %"step2d_tile_$H_$field0$_fetch", i64 %2)
  %func_result866 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %mul531, double* nonnull %"step2d_tile_$DRHS", i64 %2)
  %func_result871 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$OM_V", i64 %2)
  %func_result930 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result619, i64 %2)
  %func_result950 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DVOM", i64 %2)
  %func_result963 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$ON_U", i64 %2)
  %func_result1022 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result709, i64 %2)
  %func_result1042 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DUON", i64 %2)
  %3 = add nsw i64 %indvars.iv2095, 2
  %func_result1107 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result443_fetch, double* %func_result477, i64 %3)
  %func_result1151 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result493_fetch, double* %"step2d_tile_$H_$field0$_fetch", i64 %3)
  %func_result1177 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %mul531, double* nonnull %"step2d_tile_$DRHS", i64 %3)
  %func_result1182 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$OM_V", i64 %3)
  %func_result1241 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result619, i64 %3)
  %func_result1261 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DVOM", i64 %3)
  %func_result1274 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$ON_U", i64 %3)
  %func_result1333 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result709, i64 %3)
  %func_result1353 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DUON", i64 %3)
  %4 = add nsw i64 %indvars.iv2095, 3
  %func_result1418 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result443_fetch, double* %func_result477, i64 %4)
  %func_result1462 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %func_result493_fetch, double* %"step2d_tile_$H_$field0$_fetch", i64 %4)
  %func_result1488 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext447, i64 %mul531, double* nonnull %"step2d_tile_$DRHS", i64 %4)
  %func_result1493 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$OM_V", i64 %4)
  %func_result1552 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result619, i64 %4)
  %func_result1572 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DVOM", i64 %4)
  %func_result1585 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$ON_U", i64 %4)
  %func_result1644 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %func_result709, i64 %4)
  %func_result1664 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul147, double* nonnull %"step2d_tile_$DUON", i64 %4)
  br label %bb151

bb151:                                            ; preds = %bb151, %bb151.preheader
  %indvars.iv = phi i64 [ %0, %bb151.preheader ], [ %indvars.iv.next, %bb151 ]
  %func_result481 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result435_fetch, double* %func_result479, i64 %indvars.iv)
  %func_result481_fetch = load double, double* %func_result481, align 8
  %func_result523 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result483_fetch, double* %func_result521, i64 %indvars.iv)
  %func_result523_fetch = load double, double* %func_result523, align 8
  %add525 = fadd double %func_result481_fetch, %func_result523_fetch
  %func_result561 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result559, i64 %indvars.iv)
  store double %add525, double* %func_result561, align 8
  %func_result565 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result563, i64 %indvars.iv)
  %func_result565_fetch = load double, double* %func_result565, align 8
  %mul567 = fmul double %func_result565_fetch, 5.000000e-01
  %func_result603 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result601, i64 %indvars.iv)
  %func_result603_fetch = load double, double* %func_result603, align 8
  %add605 = fadd double %add525, %func_result603_fetch
  %mul607 = fmul double %mul567, %add605
  %func_result623 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result621, i64 %indvars.iv)
  %func_result623_fetch = load double, double* %func_result623, align 8
  %mul625 = fmul double %func_result623_fetch, %mul607
  %func_result641 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result639, i64 %indvars.iv)
  store double %mul625, double* %func_result641, align 8
  %func_result654 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result652, i64 %indvars.iv)
  %func_result654_fetch = load double, double* %func_result654, align 8
  %mul656 = fmul double %func_result654_fetch, 5.000000e-01
  %5 = add nsw i64 %indvars.iv, -1
  %func_result692 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result559, i64 %5)
  %func_result692_fetch = load double, double* %func_result692, align 8
  %add694 = fadd double %add525, %func_result692_fetch
  %mul696 = fmul double %mul656, %add694
  %func_result713 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result711, i64 %indvars.iv)
  %func_result713_fetch = load double, double* %func_result713, align 8
  %mul717 = fmul double %func_result713_fetch, %mul696
  %func_result733 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result731, i64 %indvars.iv)
  store double %mul717, double* %func_result733, align 8
  %func_result798 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result435_fetch, double* %func_result796, i64 %indvars.iv)
  %func_result798_fetch = load double, double* %func_result798, align 8
  %func_result842 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result483_fetch, double* %func_result840, i64 %indvars.iv)
  %func_result842_fetch = load double, double* %func_result842, align 8
  %add844 = fadd double %func_result798_fetch, %func_result842_fetch
  %func_result868 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result866, i64 %indvars.iv)
  store double %add844, double* %func_result868, align 8
  %func_result873 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result871, i64 %indvars.iv)
  %func_result873_fetch = load double, double* %func_result873, align 8
  %mul875 = fmul double %func_result873_fetch, 5.000000e-01
  %func_result911_fetch = load double, double* %func_result561, align 8
  %add913 = fadd double %add844, %func_result911_fetch
  %mul915 = fmul double %mul875, %add913
  %func_result932 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result930, i64 %indvars.iv)
  %func_result932_fetch = load double, double* %func_result932, align 8
  %mul936 = fmul double %func_result932_fetch, %mul915
  %func_result952 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result950, i64 %indvars.iv)
  store double %mul936, double* %func_result952, align 8
  %func_result965 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result963, i64 %indvars.iv)
  %func_result965_fetch = load double, double* %func_result965, align 8
  %mul967 = fmul double %func_result965_fetch, 5.000000e-01
  %func_result1003 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result866, i64 %5)
  %func_result1003_fetch = load double, double* %func_result1003, align 8
  %add1005 = fadd double %add844, %func_result1003_fetch
  %mul1007 = fmul double %mul967, %add1005
  %func_result1024 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1022, i64 %indvars.iv)
  %func_result1024_fetch = load double, double* %func_result1024, align 8
  %mul1028 = fmul double %func_result1024_fetch, %mul1007
  %func_result1044 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1042, i64 %indvars.iv)
  store double %mul1028, double* %func_result1044, align 8
  %func_result1109 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result435_fetch, double* %func_result1107, i64 %indvars.iv)
  %func_result1109_fetch = load double, double* %func_result1109, align 8
  %func_result1153 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result483_fetch, double* %func_result1151, i64 %indvars.iv)
  %func_result1153_fetch = load double, double* %func_result1153, align 8
  %add1155 = fadd double %func_result1109_fetch, %func_result1153_fetch
  %func_result1179 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result1177, i64 %indvars.iv)
  store double %add1155, double* %func_result1179, align 8
  %func_result1184 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1182, i64 %indvars.iv)
  %func_result1184_fetch = load double, double* %func_result1184, align 8
  %mul1186 = fmul double %func_result1184_fetch, 5.000000e-01
  %func_result1222_fetch = load double, double* %func_result868, align 8
  %add1224 = fadd double %add1155, %func_result1222_fetch
  %mul1226 = fmul double %mul1186, %add1224
  %func_result1243 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1241, i64 %indvars.iv)
  %func_result1243_fetch = load double, double* %func_result1243, align 8
  %mul1247 = fmul double %func_result1243_fetch, %mul1226
  %func_result1263 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1261, i64 %indvars.iv)
  store double %mul1247, double* %func_result1263, align 8
  %func_result1276 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1274, i64 %indvars.iv)
  %func_result1276_fetch = load double, double* %func_result1276, align 8
  %mul1278 = fmul double %func_result1276_fetch, 5.000000e-01
  %func_result1314 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result1177, i64 %5)
  %func_result1314_fetch = load double, double* %func_result1314, align 8
  %add1316 = fadd double %add1155, %func_result1314_fetch
  %mul1318 = fmul double %mul1278, %add1316
  %func_result1335 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1333, i64 %indvars.iv)
  %func_result1335_fetch = load double, double* %func_result1335, align 8
  %mul1339 = fmul double %func_result1335_fetch, %mul1318
  %func_result1355 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1353, i64 %indvars.iv)
  store double %mul1339, double* %func_result1355, align 8
  %func_result1420 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result435_fetch, double* %func_result1418, i64 %indvars.iv)
  %func_result1420_fetch = load double, double* %func_result1420, align 8
  %func_result1464 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 %func_result483_fetch, double* %func_result1462, i64 %indvars.iv)
  %func_result1464_fetch = load double, double* %func_result1464, align 8
  %add1466 = fadd double %func_result1420_fetch, %func_result1464_fetch
  %func_result1490 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result1488, i64 %indvars.iv)
  store double %add1466, double* %func_result1490, align 8
  %func_result1495 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1493, i64 %indvars.iv)
  %func_result1495_fetch = load double, double* %func_result1495, align 8
  %mul1497 = fmul double %func_result1495_fetch, 5.000000e-01
  %func_result1533_fetch = load double, double* %func_result1179, align 8
  %add1535 = fadd double %add1466, %func_result1533_fetch
  %mul1537 = fmul double %mul1497, %add1535
  %func_result1554 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1552, i64 %indvars.iv)
  %func_result1554_fetch = load double, double* %func_result1554, align 8
  %mul1558 = fmul double %func_result1554_fetch, %mul1537
  %func_result1574 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1572, i64 %indvars.iv)
  store double %mul1558, double* %func_result1574, align 8
  %func_result1587 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1585, i64 %indvars.iv)
  %func_result1587_fetch = load double, double* %func_result1587, align 8
  %mul1589 = fmul double %func_result1587_fetch, 5.000000e-01
  %func_result1625 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext439, i64 8, double* nonnull %func_result1488, i64 %5)
  %func_result1625_fetch = load double, double* %func_result1625, align 8
  %add1627 = fadd double %add1466, %func_result1625_fetch
  %mul1629 = fmul double %mul1589, %add1627
  %func_result1646 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1644, i64 %indvars.iv)
  %func_result1646_fetch = load double, double* %func_result1646, align 8
  %mul1650 = fmul double %func_result1646_fetch, %mul1629
  %func_result1666 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %func_result1664, i64 %indvars.iv)
  store double %mul1650, double* %func_result1666, align 8
  %indvars.iv.next = add i64 %indvars.iv, 1
  %rel1685 = icmp sgt i64 %indvars.iv, %int_sext2
  br i1 %rel1685, label %bb152.loopexit, label %bb151

bb152.loopexit:                                   ; preds = %bb151
  br label %bb152

bb152:                                            ; preds = %bb152.loopexit, %bb147
  %indvars.iv.next2096 = add nsw i64 %indvars.iv2095, 4
  %sub1693 = add nsw i64 %"var$39.0", -1
  %rel1697 = icmp sgt i64 %sub1693, 0
  br i1 %rel1697, label %bb147, label %bb502.loopexit

bb502.loopexit:                                   ; preds = %bb152
  br label %bb502

bb502:                                            ; preds = %bb502.loopexit, %bb142
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

attributes #0 = { nounwind readonly "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
