; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,hir-vec-dir-insert,print<hir>" -hir-loop-distribute-max-scalar-expanded-temps=9 -hir-loop-distribute-skip-vectorization-profitability-check=true -disable-output < %s 2>&1 | FileCheck %s

; Check that distribution succeeds for loops with liveout temps under control flow.
; %"prec4neg_1_$IJKL1.0" is loaded inside inside the if of the 2nd distributed loop.

; Previously, the loop was distributes into 2 chunks and the small chunk was
; vectorizable. Now we distribute it into 7 chunk 4 of which are vectorizable.

; Scalar Expansion analysis:
; %mul.156 (sb:42) (In/Out 0/0) (24) -> (88) Recompute: 0
; ( 24 -> 88 )
; %"prec4neg_1_$IJKL1.0" (sb:7) (In/Out 1/1) (30) -> (77) Recompute: 0
; ( 30 -> 77 )
; %"prec4neg_1_$SQRPOLD.0" (sb:3) (In/Out 1/0) (68) -> (75) Recompute: 0
; ( 68 -> 75 )
; %mul.177 (sb:90) (In/Out 0/0) (74) -> (75) Recompute: 0
; ( 74 -> 75 )

; Before Distribution:

;      BEGIN REGION { }
; + DO i1 = 0, %"prec4neg_1_$NBLS_N0_fetch.1124" + -1, 1   <DO_LOOP>
; |   %"prec4neg_1_$MAP_N0[]_fetch.1127" = (%"prec4neg_1_$MAP_N0")[i1];
; |   %"prec4neg_1_$MAP_IJ[]_fetch.1129" = (%"prec4neg_1_$MAP_IJ")[%"prec4neg_1_$MAP_N0[]_fetch.1127" + -1];
; |   %"prec4neg_1_$MAP_KL[]_fetch.1131" = (%"prec4neg_1_$MAP_KL")[%"prec4neg_1_$MAP_N0[]_fetch.1127" + -1];
; |   %"prec4neg_1_$APB[][]_fetch.1138" = (%"prec4neg_1_$APB")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1];
; |   %"prec4neg_1_$CPD[][]_fetch.1145" = (%"prec4neg_1_$CPD")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |   %add.100 = %"prec4neg_1_$APB[][]_fetch.1138"  +  %"prec4neg_1_$CPD[][]_fetch.1145";
; |   %div.8 = 1.000000e+00  /  %add.100;
; |   %"prec4neg_1_$ABPCDR.0" = (%add.100 !=u %"prec4neg_1_$ABPCD1R.0") ? %div.8 : %"prec4neg_1_$ABPCDR.0";
; |   %"prec4neg_1_$ABPCD1R.0" = (%add.100 !=u %"prec4neg_1_$ABPCD1R.0") ? %add.100 : %"prec4neg_1_$ABPCD1R.0";
; |   %mul.155 = (%"prec4neg_1_$ESTAB")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  *  (%"prec4neg_1_$ESTCD")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |   %mul.156 = %mul.155  *  %"prec4neg_1_$ABPCDR.0";
; |   if (%mul.156 > %fetch.1169)
; |   {
; |      %"prec4neg_1_$IJKL1.0" = %"prec4neg_1_$IJKL1.0"  +  1;
; |      (%"prec4neg_1_$INDEX")[%"prec4neg_1_$IJKL1.0" + -1] = i1 + 1;
; |      (%"prec4neg_1_$RPPQ")[%"prec4neg_1_$IJKL1.0" + -1] = %"prec4neg_1_$ABPCDR.0";
; |      %mul.157 = %"prec4neg_1_$CPD[][]_fetch.1145"  *  %"prec4neg_1_$ABPCDR.0";
; |      (%"prec4neg_1_$RHOAPB")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.157;
; |      %mul.158 = %"prec4neg_1_$APB[][]_fetch.1138"  *  %"prec4neg_1_$ABPCDR.0";
; |      (%"prec4neg_1_$RHOCPD")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.158;
; |      %mul.159 = %"prec4neg_1_$APB[][]_fetch.1138"  *  %"prec4neg_1_$CPD[][]_fetch.1145";
; |      %mul.160 = %mul.159  *  %"prec4neg_1_$ABPCDR.0";
; |      %sub.11 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][0][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][0][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |      %sub.12 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |      %sub.13 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][2][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][2][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |      %mul.169 = %sub.11  *  %sub.11;
; |      %mul.170 = %sub.12  *  %sub.12;
; |      %add.108 = %mul.169  +  %mul.170;
; |      %mul.171 = %sub.13  *  %sub.13;
; |      %add.109 = %add.108  +  %mul.171;
; |      %mul.172 = %mul.160  *  %add.109;
; |      (%"prec4neg_1_$RYS")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.172;
; |      %func_result = @llvm.sqrt.f64(%"prec4neg_1_$ABPCDR.0");
; |      %"prec4neg_1_$SQRPOLD.0" = (%"prec4neg_1_$ABPCDR.0" !=u %"prec4neg_1_$RPOLD.0") ? %func_result : %"prec4neg_1_$SQRPOLD.0";
; |      %"prec4neg_1_$RPOLD.0" = (%"prec4neg_1_$ABPCDR.0" !=u %"prec4neg_1_$RPOLD.0") ? %"prec4neg_1_$ABPCDR.0" : %"prec4neg_1_$RPOLD.0";
; |      %mul.177 = (%"prec4neg_1_$COEFIJ")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  *  (%"prec4neg_1_$COEFKL")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; |      %mul.178 = %"prec4neg_1_$SQRPOLD.0"  *  %mul.177;
; |      (%"prec4neg_1_$CONST")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.178;
; |   }
; + END LOOP
; END REGION

; After Distribution:

; BEGIN REGION { modified }
; CHECK: + DO i1 = 0, (%"prec4neg_1_$NBLS_N0_fetch.1124" + -1)/u64, 1   <DO_LOOP>
; CHECK: |   %min = (-64 * i1 + %"prec4neg_1_$NBLS_N0_fetch.1124" + -1 <= 63) ? -64 * i1 + %"prec4neg_1_$NBLS_N0_fetch.1124" + -1 : 63;
; CHECK: |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %"prec4neg_1_$MAP_N0[]_fetch.1127" = (%"prec4neg_1_$MAP_N0")[64 * i1 + i2];
; CHECK: |   |   %"prec4neg_1_$MAP_IJ[]_fetch.1129" = (%"prec4neg_1_$MAP_IJ")[%"prec4neg_1_$MAP_N0[]_fetch.1127" + -1];
; CHECK: |   |   (%.TempArray)[0][i2] = %"prec4neg_1_$MAP_IJ[]_fetch.1129";
; CHECK: |   |   %"prec4neg_1_$MAP_KL[]_fetch.1131" = (%"prec4neg_1_$MAP_KL")[%"prec4neg_1_$MAP_N0[]_fetch.1127" + -1];
; CHECK: |   |   (%.TempArray3)[0][i2] = %"prec4neg_1_$MAP_KL[]_fetch.1131";
; CHECK: |   |   %"prec4neg_1_$APB[][]_fetch.1138" = (%"prec4neg_1_$APB")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1];
; CHECK: |   |   %"prec4neg_1_$CPD[][]_fetch.1145" = (%"prec4neg_1_$CPD")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |   %add.100 = %"prec4neg_1_$APB[][]_fetch.1138"  +  %"prec4neg_1_$CPD[][]_fetch.1145";
; CHECK: |   |   (%.TempArray7)[0][i2] = %add.100;
; CHECK: |   |   %div.8 = 1.000000e+00  /  %add.100;
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %add.100 = (%.TempArray7)[0][i2];
; CHECK: |   |   %div.8 = 1.000000e+00  /  %add.100;
; CHECK: |   |   %"prec4neg_1_$ABPCDR.0" = (%add.100 !=u %"prec4neg_1_$ABPCD1R.0") ? %div.8 : %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |   (%.TempArray9)[0][i2] = %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |   %"prec4neg_1_$ABPCD1R.0" = (%add.100 !=u %"prec4neg_1_$ABPCD1R.0") ? %add.100 : %"prec4neg_1_$ABPCD1R.0";
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   %entry.region25 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %"prec4neg_1_$MAP_IJ[]_fetch.1129" = (%.TempArray)[0][i2];
; CHECK: |   |   %"prec4neg_1_$MAP_KL[]_fetch.1131" = (%.TempArray3)[0][i2];
; CHECK: |   |   %"prec4neg_1_$ABPCDR.0" = (%.TempArray9)[0][i2];
; CHECK: |   |   %mul.155 = (%"prec4neg_1_$ESTAB")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  *  (%"prec4neg_1_$ESTCD")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |   %mul.156 = %mul.155  *  %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |   (%.TempArray13)[0][i2] = %mul.156;
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   @llvm.directive.region.exit(%entry.region25); [ DIR.VPO.END.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %mul.156 = (%.TempArray13)[0][i2];
; CHECK: |   |   if (%mul.156 > %fetch.1169)
; CHECK: |   |   {
; CHECK: |   |      %"prec4neg_1_$IJKL1.0" = %"prec4neg_1_$IJKL1.0"  +  1;
; CHECK: |   |      (%.TempArray18)[0][i2] = %"prec4neg_1_$IJKL1.0";
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   %entry.region26 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %"prec4neg_1_$MAP_IJ[]_fetch.1129" = (%.TempArray)[0][i2];
; CHECK: |   |   %"prec4neg_1_$MAP_KL[]_fetch.1131" = (%.TempArray3)[0][i2];
; CHECK: |   |   %"prec4neg_1_$APB[][]_fetch.1138" = (%"prec4neg_1_$APB")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1];
; CHECK: |   |   %"prec4neg_1_$CPD[][]_fetch.1145" = (%"prec4neg_1_$CPD")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |   %"prec4neg_1_$ABPCDR.0" = (%.TempArray9)[0][i2];
; CHECK: |   |   %mul.156 = (%.TempArray13)[0][i2];
; CHECK: |   |   if (%mul.156 > %fetch.1169)
; CHECK: |   |   {
; CHECK: |   |      %"prec4neg_1_$IJKL1.0" = (%.TempArray18)[0][i2];
; CHECK: |   |      (%"prec4neg_1_$INDEX")[%"prec4neg_1_$IJKL1.0" + -1] = 64 * i1 + i2 + 1;
; CHECK: |   |      (%"prec4neg_1_$RPPQ")[%"prec4neg_1_$IJKL1.0" + -1] = %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |      %mul.157 = %"prec4neg_1_$CPD[][]_fetch.1145"  *  %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |      (%"prec4neg_1_$RHOAPB")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.157;
; CHECK: |   |      %mul.158 = %"prec4neg_1_$APB[][]_fetch.1138"  *  %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |      (%"prec4neg_1_$RHOCPD")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.158;
; CHECK: |   |      %mul.159 = %"prec4neg_1_$APB[][]_fetch.1138"  *  %"prec4neg_1_$CPD[][]_fetch.1145";
; CHECK: |   |      %mul.160 = %mul.159  *  %"prec4neg_1_$ABPCDR.0";
; CHECK: |   |      %sub.11 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][0][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][0][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |      %sub.12 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |      %sub.13 = (%"prec4neg_1_$XP")[%"prec4neg_1_$IJ_fetch.1135" + -1][2][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  -  (%"prec4neg_1_$XQ")[%"prec4neg_1_$KL_fetch.1142" + -1][2][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |      %mul.169 = %sub.11  *  %sub.11;
; CHECK: |   |      %mul.170 = %sub.12  *  %sub.12;
; CHECK: |   |      %add.108 = %mul.169  +  %mul.170;
; CHECK: |   |      %mul.171 = %sub.13  *  %sub.13;
; CHECK: |   |      %add.109 = %add.108  +  %mul.171;
; CHECK: |   |      %mul.172 = %mul.160  *  %add.109;
; CHECK: |   |      (%"prec4neg_1_$RYS")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.172;
; CHECK: |   |      %func_result = @llvm.sqrt.f64(%"prec4neg_1_$ABPCDR.0");
; CHECK: |   |      (%.TempArray21)[0][i2] = %func_result;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   @llvm.directive.region.exit(%entry.region26); [ DIR.VPO.END.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %"prec4neg_1_$ABPCDR.0" = (%.TempArray9)[0][i2];
; CHECK: |   |   %mul.156 = (%.TempArray13)[0][i2];
; CHECK: |   |   if (%mul.156 > %fetch.1169)
; CHECK: |   |   {
; CHECK: |   |      %func_result = (%.TempArray21)[0][i2];
; CHECK: |   |      %"prec4neg_1_$SQRPOLD.0" = (%"prec4neg_1_$ABPCDR.0" !=u %"prec4neg_1_$RPOLD.0") ? %func_result : %"prec4neg_1_$SQRPOLD.0";
; CHECK: |   |      (%.TempArray23)[0][i2] = %"prec4neg_1_$SQRPOLD.0";
; CHECK: |   |      %"prec4neg_1_$RPOLD.0" = (%"prec4neg_1_$ABPCDR.0" !=u %"prec4neg_1_$RPOLD.0") ? %"prec4neg_1_$ABPCDR.0" : %"prec4neg_1_$RPOLD.0";
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   %entry.region27 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
; CHECK: |   
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %"prec4neg_1_$MAP_IJ[]_fetch.1129" = (%.TempArray)[0][i2];
; CHECK: |   |   %"prec4neg_1_$MAP_KL[]_fetch.1131" = (%.TempArray3)[0][i2];
; CHECK: |   |   %mul.156 = (%.TempArray13)[0][i2];
; CHECK: |   |   if (%mul.156 > %fetch.1169)
; CHECK: |   |   {
; CHECK: |   |      %"prec4neg_1_$IJKL1.0" = (%.TempArray18)[0][i2];
; CHECK: |   |      %"prec4neg_1_$SQRPOLD.0" = (%.TempArray23)[0][i2];
; CHECK: |   |      %mul.177 = (%"prec4neg_1_$COEFIJ")[%"prec4neg_1_$IJ_fetch.1135" + -1][%"prec4neg_1_$MAP_IJ[]_fetch.1129" + -1]  *  (%"prec4neg_1_$COEFKL")[%"prec4neg_1_$KL_fetch.1142" + -1][%"prec4neg_1_$MAP_KL[]_fetch.1131" + -1];
; CHECK: |   |      %mul.178 = %"prec4neg_1_$SQRPOLD.0"  *  %mul.177;
; CHECK: |   |      (%"prec4neg_1_$CONST")[%"prec4neg_1_$IJKL1.0" + -1] = %mul.178;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |   
; CHECK: |   @llvm.directive.region.exit(%entry.region27); [ DIR.VPO.END.AUTO.VEC() ] 
; CHECK: + END LOOP
; END REGION

@neglect_ = external unnamed_addr global [24 x i8], align 32

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind readnone speculatable

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.sqrt.f64(double) #1

; Function Attrs: nofree nosync nounwind uwtable
define void @prec4neg_1_(ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$NBLS_N0", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$MAP_N0", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$IJ", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$KL", ptr noalias nocapture readnone dereferenceable(8) %"prec4neg_1_$LC12", ptr noalias nocapture readnone dereferenceable(8) %"prec4neg_1_$LC34", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$ESTAB", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$ESTCD", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$APB", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$CPD", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$COEFIJ", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$COEFKL", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$XP", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$XQ", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$MAP_IJ", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$MAP_KL", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$NIJ_UNIQE", ptr noalias nocapture readonly dereferenceable(8) %"prec4neg_1_$NKL_UNIQE", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$RPPQ", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$RHOAPB", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$RHOCPD", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$RYS", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$CONST", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$NBLS1", ptr noalias nocapture dereferenceable(8) %"prec4neg_1_$INDEX") local_unnamed_addr #2 {
alloca_9:
  %"prec4neg_1_$NIJ_UNIQE_fetch.1116" = load i64, ptr %"prec4neg_1_$NIJ_UNIQE", align 1, !tbaa !0
  %"prec4neg_1_$NKL_UNIQE_fetch.1118" = load i64, ptr %"prec4neg_1_$NKL_UNIQE", align 1, !tbaa !4
  %mul.147 = shl nsw i64 %"prec4neg_1_$NIJ_UNIQE_fetch.1116", 3
  %mul.149 = shl nsw i64 %"prec4neg_1_$NKL_UNIQE_fetch.1118", 3
  %"prec4neg_1_$NBLS_N0_fetch.1124" = load i64, ptr %"prec4neg_1_$NBLS_N0", align 1, !tbaa !6
  %rel.108 = icmp slt i64 %"prec4neg_1_$NBLS_N0_fetch.1124", 1
  br i1 %rel.108, label %bb209, label %bb208.preheader

bb208.preheader:                                  ; preds = %alloca_9
  %mul.166 = mul nsw i64 %"prec4neg_1_$NKL_UNIQE_fetch.1118", 24
  %mul.162 = mul nsw i64 %"prec4neg_1_$NIJ_UNIQE_fetch.1116", 24
  %"prec4neg_1_$IJ_fetch.1135" = load i64, ptr %"prec4neg_1_$IJ", align 1, !tbaa !8
  %"prec4neg_1_$APB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$APB", i64 %"prec4neg_1_$IJ_fetch.1135")
  %"prec4neg_1_$KL_fetch.1142" = load i64, ptr %"prec4neg_1_$KL", align 1, !tbaa !10
  %"prec4neg_1_$CPD[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$CPD", i64 %"prec4neg_1_$KL_fetch.1142")
  %"prec4neg_1_$ESTAB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$ESTAB", i64 %"prec4neg_1_$IJ_fetch.1135")
  %"prec4neg_1_$ESTCD[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$ESTCD", i64 %"prec4neg_1_$KL_fetch.1142")
  %fetch.1169 = load double, ptr getelementptr inbounds ([24 x i8], ptr @neglect_, i64 0, i64 16), align 16, !tbaa !12
  %"prec4neg_1_$XP[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.162, ptr nonnull elementtype(double) %"prec4neg_1_$XP", i64 %"prec4neg_1_$IJ_fetch.1135")
  %"prec4neg_1_$XP[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$XP[]", i64 1)
  %"prec4neg_1_$XQ[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.166, ptr nonnull elementtype(double) %"prec4neg_1_$XQ", i64 %"prec4neg_1_$KL_fetch.1142")
  %"prec4neg_1_$XQ[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[]", i64 1)
  %"prec4neg_1_$XP[][]3" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$XP[]", i64 2)
  %"prec4neg_1_$XQ[][]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[]", i64 2)
  %"prec4neg_1_$XP[][]9" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$XP[]", i64 3)
  %"prec4neg_1_$XQ[][]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[]", i64 3)
  %"prec4neg_1_$COEFIJ[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.147, ptr nonnull elementtype(double) %"prec4neg_1_$COEFIJ", i64 %"prec4neg_1_$IJ_fetch.1135")
  %"prec4neg_1_$COEFKL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.149, ptr nonnull elementtype(double) %"prec4neg_1_$COEFKL", i64 %"prec4neg_1_$KL_fetch.1142")
  %0 = add nuw nsw i64 %"prec4neg_1_$NBLS_N0_fetch.1124", 1
  br label %bb208

bb208:                                            ; preds = %bb217_endif, %bb208.preheader
  %"prec4neg_1_$SQRPOLD.0" = phi double [ %"prec4neg_1_$SQRPOLD.2", %bb217_endif ], [ 1.000000e+00, %bb208.preheader ]
  %"prec4neg_1_$RPOLD.0" = phi double [ %"prec4neg_1_$RPOLD.2", %bb217_endif ], [ 1.000000e+00, %bb208.preheader ]
  %"prec4neg_1_$ABPCD1R.0" = phi double [ %1, %bb217_endif ], [ 0.000000e+00, %bb208.preheader ]
  %"prec4neg_1_$ABPCDR.0" = phi double [ %2, %bb217_endif ], [ 0.000000e+00, %bb208.preheader ]
  %"prec4neg_1_$IJKL1.0" = phi i64 [ %"prec4neg_1_$IJKL1.1", %bb217_endif ], [ 0, %bb208.preheader ]
  %"prec4neg_1_$IJKLP.0" = phi i64 [ %add.112, %bb217_endif ], [ 1, %bb208.preheader ]
  %"prec4neg_1_$MAP_N0[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(i64) %"prec4neg_1_$MAP_N0", i64 %"prec4neg_1_$IJKLP.0")
  %"prec4neg_1_$MAP_N0[]_fetch.1127" = load i64, ptr %"prec4neg_1_$MAP_N0[]", align 1, !tbaa !14
  %"prec4neg_1_$MAP_IJ[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(i64) %"prec4neg_1_$MAP_IJ", i64 %"prec4neg_1_$MAP_N0[]_fetch.1127")
  %"prec4neg_1_$MAP_IJ[]_fetch.1129" = load i64, ptr %"prec4neg_1_$MAP_IJ[]", align 1, !tbaa !16
  %"prec4neg_1_$MAP_KL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(i64) %"prec4neg_1_$MAP_KL", i64 %"prec4neg_1_$MAP_N0[]_fetch.1127")
  %"prec4neg_1_$MAP_KL[]_fetch.1131" = load i64, ptr %"prec4neg_1_$MAP_KL[]", align 1, !tbaa !18
  %"prec4neg_1_$APB[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$APB[]", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$APB[][]_fetch.1138" = load double, ptr %"prec4neg_1_$APB[][]", align 1, !tbaa !20
  %"prec4neg_1_$CPD[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$CPD[]", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$CPD[][]_fetch.1145" = load double, ptr %"prec4neg_1_$CPD[][]", align 1, !tbaa !22
  %add.100 = fadd reassoc ninf nsz arcp contract afn double %"prec4neg_1_$APB[][]_fetch.1138", %"prec4neg_1_$CPD[][]_fetch.1145"
  %rel.109 = fcmp reassoc ninf nsz arcp contract afn une double %add.100, %"prec4neg_1_$ABPCD1R.0"
  %div.8 = fdiv reassoc ninf nsz arcp contract afn double 1.000000e+00, %add.100
  %1 = select i1 %rel.109, double %add.100, double %"prec4neg_1_$ABPCD1R.0"
  %2 = select i1 %rel.109, double %div.8, double %"prec4neg_1_$ABPCDR.0"
  %"prec4neg_1_$ESTAB[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$ESTAB[]", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$ESTAB[][]_fetch.1159" = load double, ptr %"prec4neg_1_$ESTAB[][]", align 1, !tbaa !24
  %"prec4neg_1_$ESTCD[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$ESTCD[]", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$ESTCD[][]_fetch.1166" = load double, ptr %"prec4neg_1_$ESTCD[][]", align 1, !tbaa !26
  %mul.155 = fmul reassoc ninf nsz arcp contract afn double %"prec4neg_1_$ESTAB[][]_fetch.1159", %"prec4neg_1_$ESTCD[][]_fetch.1166"
  %mul.156 = fmul reassoc ninf nsz arcp contract afn double %mul.155, %2
  %rel.110 = fcmp reassoc ninf nsz arcp contract afn ogt double %mul.156, %fetch.1169
  br i1 %rel.110, label %bb_new543_then, label %bb217_endif

bb_new543_then:                                   ; preds = %bb208
  %add.103 = add nsw i64 %"prec4neg_1_$IJKL1.0", 1
  %"prec4neg_1_$INDEX[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(i64) %"prec4neg_1_$INDEX", i64 %add.103)
  store i64 %"prec4neg_1_$IJKLP.0", ptr %"prec4neg_1_$INDEX[]", align 1, !tbaa !28
  %"prec4neg_1_$RPPQ[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$RPPQ", i64 %add.103)
  store double %2, ptr %"prec4neg_1_$RPPQ[]", align 1, !tbaa !30
  %mul.157 = fmul reassoc ninf nsz arcp contract afn double %"prec4neg_1_$CPD[][]_fetch.1145", %2
  %"prec4neg_1_$RHOAPB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$RHOAPB", i64 %add.103)
  store double %mul.157, ptr %"prec4neg_1_$RHOAPB[]", align 1, !tbaa !32
  %mul.158 = fmul reassoc ninf nsz arcp contract afn double %"prec4neg_1_$APB[][]_fetch.1138", %2
  %"prec4neg_1_$RHOCPD[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$RHOCPD", i64 %add.103)
  store double %mul.158, ptr %"prec4neg_1_$RHOCPD[]", align 1, !tbaa !34
  %mul.159 = fmul reassoc ninf nsz arcp contract afn double %"prec4neg_1_$APB[][]_fetch.1138", %"prec4neg_1_$CPD[][]_fetch.1145"
  %mul.160 = fmul reassoc ninf nsz arcp contract afn double %mul.159, %2
  %"prec4neg_1_$XP[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XP[][]", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$XP[][][]_fetch.1193" = load double, ptr %"prec4neg_1_$XP[][][]", align 1, !tbaa !36
  %"prec4neg_1_$XQ[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[][]", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$XQ[][][]_fetch.1202" = load double, ptr %"prec4neg_1_$XQ[][][]", align 1, !tbaa !38
  %sub.11 = fsub reassoc ninf nsz arcp contract afn double %"prec4neg_1_$XP[][][]_fetch.1193", %"prec4neg_1_$XQ[][][]_fetch.1202"
  %"prec4neg_1_$XP[][][]4" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XP[][]3", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$XP[][][]_fetch.1208" = load double, ptr %"prec4neg_1_$XP[][][]4", align 1, !tbaa !36
  %"prec4neg_1_$XQ[][][]7" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[][]6", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$XQ[][][]_fetch.1214" = load double, ptr %"prec4neg_1_$XQ[][][]7", align 1, !tbaa !38
  %sub.12 = fsub reassoc ninf nsz arcp contract afn double %"prec4neg_1_$XP[][][]_fetch.1208", %"prec4neg_1_$XQ[][][]_fetch.1214"
  %"prec4neg_1_$XP[][][]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XP[][]9", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$XP[][][]_fetch.1220" = load double, ptr %"prec4neg_1_$XP[][][]10", align 1, !tbaa !36
  %"prec4neg_1_$XQ[][][]13" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$XQ[][]12", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$XQ[][][]_fetch.1226" = load double, ptr %"prec4neg_1_$XQ[][][]13", align 1, !tbaa !38
  %sub.13 = fsub reassoc ninf nsz arcp contract afn double %"prec4neg_1_$XP[][][]_fetch.1220", %"prec4neg_1_$XQ[][][]_fetch.1226"
  %mul.169 = fmul reassoc ninf nsz arcp contract afn double %sub.11, %sub.11
  %mul.170 = fmul reassoc ninf nsz arcp contract afn double %sub.12, %sub.12
  %add.108 = fadd reassoc ninf nsz arcp contract afn double %mul.169, %mul.170
  %mul.171 = fmul reassoc ninf nsz arcp contract afn double %sub.13, %sub.13
  %add.109 = fadd reassoc ninf nsz arcp contract afn double %add.108, %mul.171
  %mul.172 = fmul reassoc ninf nsz arcp contract afn double %mul.160, %add.109
  %"prec4neg_1_$RYS[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$RYS", i64 %add.103)
  store double %mul.172, ptr %"prec4neg_1_$RYS[]", align 1, !tbaa !40
  %rel.111 = fcmp reassoc ninf nsz arcp contract afn une double %2, %"prec4neg_1_$RPOLD.0"
  %func_result = tail call reassoc ninf nsz arcp contract afn double @llvm.sqrt.f64(double %2)
  %3 = select i1 %rel.111, double %func_result, double %"prec4neg_1_$SQRPOLD.0"
  %4 = select i1 %rel.111, double %2, double %"prec4neg_1_$RPOLD.0"
  %"prec4neg_1_$COEFIJ[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$COEFIJ[]", i64 %"prec4neg_1_$MAP_IJ[]_fetch.1129")
  %"prec4neg_1_$COEFIJ[][]_fetch.1246" = load double, ptr %"prec4neg_1_$COEFIJ[][]", align 1, !tbaa !42
  %"prec4neg_1_$COEFKL[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$COEFKL[]", i64 %"prec4neg_1_$MAP_KL[]_fetch.1131")
  %"prec4neg_1_$COEFKL[][]_fetch.1253" = load double, ptr %"prec4neg_1_$COEFKL[][]", align 1, !tbaa !44
  %mul.177 = fmul reassoc ninf nsz arcp contract afn double %"prec4neg_1_$COEFIJ[][]_fetch.1246", %"prec4neg_1_$COEFKL[][]_fetch.1253"
  %mul.178 = fmul reassoc ninf nsz arcp contract afn double %3, %mul.177
  %"prec4neg_1_$CONST[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"prec4neg_1_$CONST", i64 %add.103)
  store double %mul.178, ptr %"prec4neg_1_$CONST[]", align 1, !tbaa !46
  br label %bb217_endif

bb217_endif:                                      ; preds = %bb_new543_then, %bb208
  %"prec4neg_1_$SQRPOLD.2" = phi double [ %3, %bb_new543_then ], [ %"prec4neg_1_$SQRPOLD.0", %bb208 ]
  %"prec4neg_1_$RPOLD.2" = phi double [ %4, %bb_new543_then ], [ %"prec4neg_1_$RPOLD.0", %bb208 ]
  %"prec4neg_1_$IJKL1.1" = phi i64 [ %add.103, %bb_new543_then ], [ %"prec4neg_1_$IJKL1.0", %bb208 ]
  %add.112 = add nuw nsw i64 %"prec4neg_1_$IJKLP.0", 1
  %exitcond = icmp eq i64 %add.112, %0
  br i1 %exitcond, label %bb209.loopexit, label %bb208

bb209.loopexit:                                   ; preds = %bb217_endif
  %"prec4neg_1_$IJKL1.1.lcssa" = phi i64 [ %"prec4neg_1_$IJKL1.1", %bb217_endif ]
  br label %bb209

bb209:                                            ; preds = %bb209.loopexit, %alloca_9
  %"prec4neg_1_$IJKL1.2" = phi i64 [ 0, %alloca_9 ], [ %"prec4neg_1_$IJKL1.1.lcssa", %bb209.loopexit ]
  store i64 %"prec4neg_1_$IJKL1.2", ptr %"prec4neg_1_$NBLS1", align 1, !tbaa !48
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$278", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"Simple Fortran Alias Analysis 10"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$280", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$289", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$297", !2, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$300", !2, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$307", !2, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$291", !2, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$293", !2, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$295", !2, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$298", !2, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$301", !2, i64 0}
!24 = !{!25, !25, i64 0}
!25 = !{!"ifx$unique_sym$304", !2, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"ifx$unique_sym$305", !2, i64 0}
!28 = !{!29, !29, i64 0}
!29 = !{!"ifx$unique_sym$308", !2, i64 0}
!30 = !{!31, !31, i64 0}
!31 = !{!"ifx$unique_sym$309", !2, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"ifx$unique_sym$310", !2, i64 0}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$311", !2, i64 0}
!36 = !{!37, !37, i64 0}
!37 = !{!"ifx$unique_sym$314", !2, i64 0}
!38 = !{!39, !39, i64 0}
!39 = !{!"ifx$unique_sym$315", !2, i64 0}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$320", !2, i64 0}
!42 = !{!43, !43, i64 0}
!43 = !{!"ifx$unique_sym$321", !2, i64 0}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$322", !2, i64 0}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$323", !2, i64 0}
!48 = !{!49, !49, i64 0}
!49 = !{!"ifx$unique_sym$324", !2, i64 0}
