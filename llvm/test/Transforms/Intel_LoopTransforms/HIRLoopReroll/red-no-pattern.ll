; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s
; INTEL Re-enable after CMPLRLLVM-21845 fixed.
; XFAIL: *

; Make sure a pattern of safe reduction chains, that are not handled by HIR Loop Reroll currently, does not cause a compilation failure.
; The safe reduction pattern was in the in the i5-loop of else part of "if (trunc.i32.i1(%mod_mp_noncolin__fetch) == 0)"
;
; Reroll can only handle a reduction chain, where the first reduction instruction is a binary operation.
; In the following example, the first reduction instruction is a copy-like instruction, not a binary operation.
; It contains two reduction chains, each chain containing 3 reduction instructions.

; %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1"; <safe reduction chain 1 - 1>
; %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1"; <safe reduction chain 2 - 1>
; ...
; %sub1175 = %"c_phase_$PREF.sroa.0.1.out11"  -  %31;        <safe reduction chain 2 - 2>
; %"c_phase_$PREF.sroa.0.1" = %sub1175  +  %mul1173;           <safe reduction chain 2 - 3>
; %add1181 = %mul1177  +  %"c_phase_$PREF.sroa.6.1.out12";     <safe reduction chain 1 - 2>
; %"c_phase_$PREF.sroa.6.1" = %add1181  +  %mul1179;           <safe reduction chain 1 - 3>


; CHECK: Function: c_phase_

; CHECK:       BEGIN REGION { modified }
; CHECK:             + DO i1 = 0, %mod_mp_nppstr__fetch + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:             |   if (i1 + 1 != 1)
; CHECK:             |   {
; CHECK:             |      + DO i2 = 0, sext.i32.i64(%mod_mp_npwx__fetch) + -1, 1   <DO_LOOP>
; CHECK:             |      |   (%"c_phase_$IGK0")[i2] = (%_fetch1476)[0][i2 + %"[]_fetch"];
; CHECK:             |      + END LOOP
; CHECK:             |
; CHECK:             |
; CHECK:             |      + DO i2 = 0, sext.i32.i64(%mod_mp_nbnd__fetch158) + -1, 1   <DO_LOOP>
; CHECK:             |      |   %"c_phase_$L_CAL.addr_a0$_fetch[]_fetch" = (%"c_phase_$L_CAL.addr_a0$_fetch1449")[i2 + 1];
; CHECK:             |      |
; CHECK:             |      |   + DO i3 = 0, sext.i32.i64(%mod_mp_nbnd__fetch158) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |   %17 = trunc.i32.i1(%"c_phase_$L_CAL.addr_a0$_fetch[]_fetch")  &  (%"c_phase_$L_CAL.addr_a0$_fetch1449")[i3 + 1];
; CHECK:             |      |   |   %.not = %17 == 0;
; CHECK:             |      |   |   %brmerge = %.not  |  %rel1446.not;
; CHECK:             |      |   |   if (%brmerge == 0)
; CHECK:             |      |   |   {
; CHECK:             |      |   |      %"c_phase_$PREF.sroa.0.4" = 0.000000e+00;
; CHECK:             |      |   |      %"c_phase_$PREF.sroa.6.4" = 0.000000e+00;
; CHECK:             |      |   |      if (%mod_mp_nkb__fetch1440 >= 1)
; CHECK:             |      |   |      {
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.0.0" = 0.000000e+00;
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.6.0" = 0.000000e+00;
; CHECK:             |      |   |         if (trunc.i32.i1(%mod_mp_noncolin__fetch) == 0)
; CHECK:             |      |   |         {
; CHECK:             |      |   |            + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |            |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |            |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |            |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |            |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |            |
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |            |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |   |   %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack" = (null)[i2 + 1][i4 + 1].0;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778" = (null)[i2 + 1][i4 + 1].1;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack" = (null)[i3 + 1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781" = (null)[i3 + 1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |            |   |   %mul1276 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %33 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778";
; CHECK:             |      |   |            |   |   %sub1278 = %33  +  %mul1276;
; CHECK:             |      |   |            |   |   %mul1282 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %34 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778";
; CHECK:             |      |   |            |   |   %add1284 = %mul1282  -  %34;
; CHECK:             |      |   |            |   |   %"c_phase_$Q_DK[][][]_fetch.unpack" = (%"c_phase_$Q_DK")[%"_fetch[]224_fetch"][i5][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |            |   |   %"c_phase_$Q_DK[][][]_fetch.unpack1784" = (%"c_phase_$Q_DK")[%"_fetch[]224_fetch"][i5][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |            |   |   %mul1323 = %add1284  *  %"c_phase_$Q_DK[][][]_fetch.unpack1784";
; CHECK:             |      |   |            |   |   %mul1325 = %sub1278  *  %"c_phase_$Q_DK[][][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %sub1327 = %mul1325  -  %mul1323;
; CHECK:             |      |   |            |   |   %mul1329 = %add1284  *  %"c_phase_$Q_DK[][][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1331 = %sub1278  *  %"c_phase_$Q_DK[][][]_fetch.unpack1784";
; CHECK:             |      |   |            |   |   %add1333 = %mul1331  +  %mul1329;
; CHECK:             |      |   |            |   |   %"c_phase_$STRUC[]1344_fetch.unpack" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].0;
; CHECK:             |      |   |            |   |   %"c_phase_$STRUC[]1344_fetch.unpack1787" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].1;
; CHECK:             |      |   |            |   |   %mul1348 = %sub1327  *  %"c_phase_$STRUC[]1344_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1352 = %add1333  *  %"c_phase_$STRUC[]1344_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1354 = %sub1327  *  %"c_phase_$STRUC[]1344_fetch.unpack1787";
; CHECK:             |      |   |            |   |   %sub1350 = %mul1348  +  %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |   |   %35 = %add1333  *  %"c_phase_$STRUC[]1344_fetch.unpack1787";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.0.1" = %sub1350  -  %35;
; CHECK:             |      |   |            |   |   %add1356 = %mul1352  +  %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.6.1" = %add1356  +  %mul1354;
; CHECK:             |      |   |            |   + END LOOP
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            + END LOOP
; CHECK:             |      |   |         }
; CHECK:             |      |   |         else
; CHECK:             |      |   |         {
; CHECK:             |      |   |            if (trunc.i32.i1(%mod_mp_lspinorb__fetch) != 0)
; CHECK:             |      |   |            {
; CHECK:             |      |   |               + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |               |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |               |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |               |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |               |
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |               |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack" = (null)[i2 + 1][1][i4 + 1].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795" = (null)[i2 + 1][1][i4 + 1].1;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack" = (null)[i3 + 1][1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798" = (null)[i3 + 1][1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |               |   |   %mul342 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %21 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %sub344 = %21  +  %mul342;
; CHECK:             |      |   |               |   |   %mul348 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %22 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %add350 = %mul348  -  %22;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][1][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][1][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul432 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack"  *  %sub344;
; CHECK:             |      |   |               |   |   %mul430.neg = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801"  *  %add350;
; CHECK:             |      |   |               |   |   %mul436 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack"  *  %add350;
; CHECK:             |      |   |               |   |   %mul438 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801"  *  %sub344;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack" = (null)[i3 + 1][2][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804" = (null)[i3 + 1][2][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |               |   |   %mul569 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %23 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %sub571 = %23  +  %mul569;
; CHECK:             |      |   |               |   |   %mul575 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %24 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %add577 = %mul575  -  %24;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][2][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][2][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul663 = %sub571  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul661.neg = %add577  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807";
; CHECK:             |      |   |               |   |   %mul667 = %add577  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul669 = %sub571  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807";
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack" = (null)[i2 + 1][2][i4 + 1].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810" = (null)[i2 + 1][2][i4 + 1].1;
; CHECK:             |      |   |               |   |   %mul809 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %25 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798";
; CHECK:             |      |   |               |   |   %sub811 = %25  +  %mul809;
; CHECK:             |      |   |               |   |   %mul815 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798";
; CHECK:             |      |   |               |   |   %26 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %add817 = %mul815  -  %26;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][3][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][3][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul903 = %sub811  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul901.neg = %add817  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813";
; CHECK:             |      |   |               |   |   %mul907 = %add817  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul909 = %sub811  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813";
; CHECK:             |      |   |               |   |   %mul1049 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %27 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804";
; CHECK:             |      |   |               |   |   %sub1051 = %27  +  %mul1049;
; CHECK:             |      |   |               |   |   %mul1055 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804";
; CHECK:             |      |   |               |   |   %28 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %add1057 = %mul1055  -  %28;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][4][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][4][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul1141 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816"  *  %add1057;
; CHECK:             |      |   |               |   |   %mul1143 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack"  *  %sub1051;
; CHECK:             |      |   |               |   |   %mul1147 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack"  *  %add1057;
; CHECK:             |      |   |               |   |   %mul1149 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816"  *  %sub1051;
; CHECK:             |      |   |               |   |   %reass.add = %mul661.neg  +  %mul430.neg;
; CHECK:             |      |   |               |   |   %reass.add1825 = %reass.add  +  %mul901.neg;
; CHECK:             |      |   |               |   |   %sub434 = %mul663  +  %mul432;
; CHECK:             |      |   |               |   |   %add677 = %sub434  +  %mul903;
; CHECK:             |      |   |               |   |   %29 = %mul1143  +  %add677;
; CHECK:             |      |   |               |   |   %30 = %mul1141  +  %reass.add1825;
; CHECK:             |      |   |               |   |   %add1157 = %29  -  %30;
; CHECK:             |      |   |               |   |   %add1151 = %mul438  +  %mul436;
; CHECK:             |      |   |               |   |   %add911 = %add1151  +  %mul667;
; CHECK:             |      |   |               |   |   %add671 = %add911  +  %mul669;
; CHECK:             |      |   |               |   |   %add440 = %add671  +  %mul907;
; CHECK:             |      |   |               |   |   %add679 = %add440  +  %mul909;
; CHECK:             |      |   |               |   |   %add919 = %add679  +  %mul1147;
; CHECK:             |      |   |               |   |   %add1159 = %add919  +  %mul1149;
; CHECK:             |      |   |               |   |   %"c_phase_$STRUC[]_fetch.unpack" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$STRUC[]_fetch.unpack1819" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul1173 = %add1157  *  %"c_phase_$STRUC[]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul1177 = %add1159  *  %"c_phase_$STRUC[]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul1179 = %add1157  *  %"c_phase_$STRUC[]_fetch.unpack1819";
; CHECK:             |      |   |               |   |   %31 = %add1159  *  %"c_phase_$STRUC[]_fetch.unpack1819";
; CHECK:             |      |   |               |   |   %sub1175 = %"c_phase_$PREF.sroa.0.1.out11"  -  %31;
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1" = %sub1175  +  %mul1173;
; CHECK:             |      |   |               |   |   %add1181 = %mul1177  +  %"c_phase_$PREF.sroa.6.1.out12";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1" = %add1181  +  %mul1179;
; CHECK:             |      |   |               |   + END LOOP
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               + END LOOP
; CHECK:             |      |   |            }
; CHECK:             |      |   |            else
; CHECK:             |      |   |            {
; CHECK:             |      |   |               + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |               |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |               |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |               |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |               |
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |               |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |   + END LOOP
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               + END LOOP
; CHECK:             |      |   |            }
; CHECK:             |      |   |         }
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.0.4" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.6.4" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |      }
; CHECK:             |      |   |      %add1414 = (%"c_phase_$MAT")[i3][i2].0  +  %"c_phase_$PREF.sroa.0.4";
; CHECK:             |      |   |      %add1416 = (%"c_phase_$MAT")[i3][i2].1  +  %"c_phase_$PREF.sroa.6.4";
; CHECK:             |      |   |      (%"c_phase_$MAT")[i3][i2].0 = %add1414;
; CHECK:             |      |   |      (%"c_phase_$MAT")[i3][i2].1 = %add1416;
; CHECK:             |      |   |   }
; CHECK:             |      |   + END LOOP
; CHECK:             |      + END LOOP
; CHECK:             |   }
; CHECK:             + END LOOP
; CHECK:       END REGION

; CHECK:Function: c_phase_

; CHECK:       BEGIN REGION { modified }
; CHECK:             + DO i1 = 0, %mod_mp_nppstr__fetch + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:             |   if (i1 + 1 != 1)
; CHECK:             |   {
; CHECK:             |      + DO i2 = 0, sext.i32.i64(%mod_mp_npwx__fetch) + -1, 1   <DO_LOOP>
; CHECK:             |      |   (%"c_phase_$IGK0")[i2] = (%_fetch1476)[0][i2 + %"[]_fetch"];
; CHECK:             |      + END LOOP
; CHECK:             |
; CHECK:             |
; CHECK:             |      + DO i2 = 0, sext.i32.i64(%mod_mp_nbnd__fetch158) + -1, 1   <DO_LOOP>
; CHECK:             |      |   %"c_phase_$L_CAL.addr_a0$_fetch[]_fetch" = (%"c_phase_$L_CAL.addr_a0$_fetch1449")[i2 + 1];
; CHECK:             |      |
; CHECK:             |      |   + DO i3 = 0, sext.i32.i64(%mod_mp_nbnd__fetch158) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |   %17 = trunc.i32.i1(%"c_phase_$L_CAL.addr_a0$_fetch[]_fetch")  &  (%"c_phase_$L_CAL.addr_a0$_fetch1449")[i3 + 1];
; CHECK:             |      |   |   %.not = %17 == 0;
; CHECK:             |      |   |   %brmerge = %.not  |  %rel1446.not;
; CHECK:             |      |   |   if (%brmerge == 0)
; CHECK:             |      |   |   {
; CHECK:             |      |   |      %"c_phase_$PREF.sroa.0.4" = 0.000000e+00;
; CHECK:             |      |   |      %"c_phase_$PREF.sroa.6.4" = 0.000000e+00;
; CHECK:             |      |   |      if (%mod_mp_nkb__fetch1440 >= 1)
; CHECK:             |      |   |      {
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.0.0" = 0.000000e+00;
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.6.0" = 0.000000e+00;
; CHECK:             |      |   |         if (trunc.i32.i1(%mod_mp_noncolin__fetch) == 0)
; CHECK:             |      |   |         {
; CHECK:             |      |   |            + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |            |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |            |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |            |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |            |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |            |
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |            |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |   |   %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack" = (null)[i2 + 1][i4 + 1].0;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778" = (null)[i2 + 1][i4 + 1].1;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack" = (null)[i3 + 1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |            |   |   %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781" = (null)[i3 + 1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |            |   |   %mul1276 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %33 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778";
; CHECK:             |      |   |            |   |   %sub1278 = %33  +  %mul1276;
; CHECK:             |      |   |            |   |   %mul1282 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %34 = %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack"  *  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778";
; CHECK:             |      |   |            |   |   %add1284 = %mul1282  -  %34;
; CHECK:             |      |   |            |   |   %"c_phase_$Q_DK[][][]_fetch.unpack" = (%"c_phase_$Q_DK")[%"_fetch[]224_fetch"][i5][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |            |   |   %"c_phase_$Q_DK[][][]_fetch.unpack1784" = (%"c_phase_$Q_DK")[%"_fetch[]224_fetch"][i5][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |            |   |   %mul1323 = %add1284  *  %"c_phase_$Q_DK[][][]_fetch.unpack1784";
; CHECK:             |      |   |            |   |   %mul1325 = %sub1278  *  %"c_phase_$Q_DK[][][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %sub1327 = %mul1325  -  %mul1323;
; CHECK:             |      |   |            |   |   %mul1329 = %add1284  *  %"c_phase_$Q_DK[][][]_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1331 = %sub1278  *  %"c_phase_$Q_DK[][][]_fetch.unpack1784";
; CHECK:             |      |   |            |   |   %add1333 = %mul1331  +  %mul1329;
; CHECK:             |      |   |            |   |   %"c_phase_$STRUC[]1344_fetch.unpack" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].0;
; CHECK:             |      |   |            |   |   %"c_phase_$STRUC[]1344_fetch.unpack1787" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].1;
; CHECK:             |      |   |            |   |   %mul1348 = %sub1327  *  %"c_phase_$STRUC[]1344_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1352 = %add1333  *  %"c_phase_$STRUC[]1344_fetch.unpack";
; CHECK:             |      |   |            |   |   %mul1354 = %sub1327  *  %"c_phase_$STRUC[]1344_fetch.unpack1787";
; CHECK:             |      |   |            |   |   %sub1350 = %mul1348  +  %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |   |   %35 = %add1333  *  %"c_phase_$STRUC[]1344_fetch.unpack1787";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.0.1" = %sub1350  -  %35;
; CHECK:             |      |   |            |   |   %add1356 = %mul1352  +  %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            |   |   %"c_phase_$PREF.sroa.6.1" = %add1356  +  %mul1354;
; CHECK:             |      |   |            |   + END LOOP
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |            |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |            + END LOOP
; CHECK:             |      |   |         }
; CHECK:             |      |   |         else
; CHECK:             |      |   |         {
; CHECK:             |      |   |            if (trunc.i32.i1(%mod_mp_lspinorb__fetch) != 0)
; CHECK:             |      |   |            {
; CHECK:             |      |   |               + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |               |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |               |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |               |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |               |
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |               |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack" = (null)[i2 + 1][1][i4 + 1].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795" = (null)[i2 + 1][1][i4 + 1].1;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack" = (null)[i3 + 1][1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798" = (null)[i3 + 1][1][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |               |   |   %mul342 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %21 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %sub344 = %21  +  %mul342;
; CHECK:             |      |   |               |   |   %mul348 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %22 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %add350 = %mul348  -  %22;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][1][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][1][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul432 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack"  *  %sub344;
; CHECK:             |      |   |               |   |   %mul430.neg = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801"  *  %add350;
; CHECK:             |      |   |               |   |   %mul436 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack"  *  %add350;
; CHECK:             |      |   |               |   |   %mul438 = %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801"  *  %sub344;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack" = (null)[i3 + 1][2][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804" = (null)[i3 + 1][2][i4 + i5 + -1 * %"c_phase_$NKBTONH[]_fetch" + 2].1;
; CHECK:             |      |   |               |   |   %mul569 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %23 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %sub571 = %23  +  %mul569;
; CHECK:             |      |   |               |   |   %mul575 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %24 = %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"  *  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795";
; CHECK:             |      |   |               |   |   %add577 = %mul575  -  %24;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][2][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][2][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul663 = %sub571  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul661.neg = %add577  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807";
; CHECK:             |      |   |               |   |   %mul667 = %add577  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul669 = %sub571  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807";
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack" = (null)[i2 + 1][2][i4 + 1].0;
; CHECK:             |      |   |               |   |   %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810" = (null)[i2 + 1][2][i4 + 1].1;
; CHECK:             |      |   |               |   |   %mul809 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %25 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798";
; CHECK:             |      |   |               |   |   %sub811 = %25  +  %mul809;
; CHECK:             |      |   |               |   |   %mul815 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798";
; CHECK:             |      |   |               |   |   %26 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %add817 = %mul815  -  %26;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][3][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][3][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul903 = %sub811  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul901.neg = %add817  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813";
; CHECK:             |      |   |               |   |   %mul907 = %add817  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul909 = %sub811  *  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813";
; CHECK:             |      |   |               |   |   %mul1049 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %27 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804";
; CHECK:             |      |   |               |   |   %sub1051 = %27  +  %mul1049;
; CHECK:             |      |   |               |   |   %mul1055 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804";
; CHECK:             |      |   |               |   |   %28 = %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810"  *  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack";
; CHECK:             |      |   |               |   |   %add1057 = %mul1055  -  %28;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][4][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816" = (%"c_phase_$Q_DK_SO.addr_a0$_fetch")[%"_fetch[]224_fetch"][4][i5 + 1][%"c_phase_$NKBTONH[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul1141 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816"  *  %add1057;
; CHECK:             |      |   |               |   |   %mul1143 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack"  *  %sub1051;
; CHECK:             |      |   |               |   |   %mul1147 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack"  *  %add1057;
; CHECK:             |      |   |               |   |   %mul1149 = %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816"  *  %sub1051;
; CHECK:             |      |   |               |   |   %reass.add = %mul661.neg  +  %mul430.neg;
; CHECK:             |      |   |               |   |   %reass.add1825 = %reass.add  +  %mul901.neg;
; CHECK:             |      |   |               |   |   %sub434 = %mul663  +  %mul432;
; CHECK:             |      |   |               |   |   %add677 = %sub434  +  %mul903;
; CHECK:             |      |   |               |   |   %29 = %mul1143  +  %add677;
; CHECK:             |      |   |               |   |   %30 = %mul1141  +  %reass.add1825;
; CHECK:             |      |   |               |   |   %add1157 = %29  -  %30;
; CHECK:             |      |   |               |   |   %add1151 = %mul438  +  %mul436;
; CHECK:             |      |   |               |   |   %add911 = %add1151  +  %mul667;
; CHECK:             |      |   |               |   |   %add671 = %add911  +  %mul669;
; CHECK:             |      |   |               |   |   %add440 = %add671  +  %mul907;
; CHECK:             |      |   |               |   |   %add679 = %add440  +  %mul909;
; CHECK:             |      |   |               |   |   %add919 = %add679  +  %mul1147;
; CHECK:             |      |   |               |   |   %add1159 = %add919  +  %mul1149;
; CHECK:             |      |   |               |   |   %"c_phase_$STRUC[]_fetch.unpack" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].0;
; CHECK:             |      |   |               |   |   %"c_phase_$STRUC[]_fetch.unpack1819" = (%"c_phase_$STRUC")[%"c_phase_$NKBTONA[]_fetch"].1;
; CHECK:             |      |   |               |   |   %mul1173 = %add1157  *  %"c_phase_$STRUC[]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul1177 = %add1159  *  %"c_phase_$STRUC[]_fetch.unpack";
; CHECK:             |      |   |               |   |   %mul1179 = %add1157  *  %"c_phase_$STRUC[]_fetch.unpack1819";
; CHECK:             |      |   |               |   |   %31 = %add1159  *  %"c_phase_$STRUC[]_fetch.unpack1819";
; CHECK:             |      |   |               |   |   %sub1175 = %"c_phase_$PREF.sroa.0.1.out11"  -  %31;
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1" = %sub1175  +  %mul1173;
; CHECK:             |      |   |               |   |   %add1181 = %mul1177  +  %"c_phase_$PREF.sroa.6.1.out12";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1" = %add1181  +  %mul1179;
; CHECK:             |      |   |               |   + END LOOP
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               + END LOOP
; CHECK:             |      |   |            }
; CHECK:             |      |   |            else
; CHECK:             |      |   |            {
; CHECK:             |      |   |               + DO i4 = 0, sext.i32.i64(%mod_mp_nkb__fetch1440) + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   %"c_phase_$NKBTONH[]_fetch" = (%"c_phase_$NKBTONH")[i4];
; CHECK:             |      |   |               |   %"c_phase_$NKBTONA[]_fetch" = (%"c_phase_$NKBTONA")[i4];
; CHECK:             |      |   |               |   %"_fetch[]224_fetch" = (%_fetch225)[%"c_phase_$NKBTONA[]_fetch"];
; CHECK:             |      |   |               |   %"mod_mp_nh_[]_fetch" = (@mod_mp_nh_)[0][%"_fetch[]224_fetch"];
; CHECK:             |      |   |               |
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.1" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.1" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |               |   + DO i5 = 0, sext.i32.i64(%"mod_mp_nh_[]_fetch") + -1, 1   <DO_LOOP>
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.6.1.out12" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               |   |   %"c_phase_$PREF.sroa.0.1.out11" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |   + END LOOP
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.0.0" = %"c_phase_$PREF.sroa.0.1";
; CHECK:             |      |   |               |      %"c_phase_$PREF.sroa.6.0" = %"c_phase_$PREF.sroa.6.1";
; CHECK:             |      |   |               + END LOOP
; CHECK:             |      |   |            }
; CHECK:             |      |   |         }
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.0.4" = %"c_phase_$PREF.sroa.0.0";
; CHECK:             |      |   |         %"c_phase_$PREF.sroa.6.4" = %"c_phase_$PREF.sroa.6.0";
; CHECK:             |      |   |      }
; CHECK:             |      |   |      %add1414 = (%"c_phase_$MAT")[i3][i2].0  +  %"c_phase_$PREF.sroa.0.4";
; CHECK:             |      |   |      %add1416 = (%"c_phase_$MAT")[i3][i2].1  +  %"c_phase_$PREF.sroa.6.4";
; CHECK:             |      |   |      (%"c_phase_$MAT")[i3][i2].0 = %add1414;
; CHECK:             |      |   |      (%"c_phase_$MAT")[i3][i2].1 = %add1416;
; CHECK:             |      |   |   }
; CHECK:             |      |   + END LOOP
; CHECK:             |      + END LOOP
; CHECK:             |   }
; CHECK:             + END LOOP
; CHECK:       END REGION

;Module Before HIR
; ModuleID = 'bp_c_phase.i90'
source_filename = "bp_c_phase.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank1$.0" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"var$36" = type { %"QNCA_a0$ptr$rank0$", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank0$" = type { ptr, i64, i64, i64, i64, i64 }
%dyn_typ = type { ptr, ptr }
%"var$38" = type { %"QNCA_a0$ptr$rank2$", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"var$40" = type { %"QNCA_a0$ptr$rank2$.4", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank2$.4" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"var$42" = type { %"QNCA_a0$ptr$rank3$", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"MOD$.btBEC_TYPE" = type { %"QNCA_a0$ptr$rank2$", %"QNCA_a0$ptr$rank2$", %"QNCA_a0$ptr$rank3$", i32, i32, i32, i32, i32, i32 }
%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%complex_128bit = type { double, double }
%"QNCA_a0$ptr$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"var$45" = type { %"QNCA_a0$ptr$rank0$.5", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank0$.5" = type { ptr, i64, i64, i64, i64, i64 }
%"QNCA_a0$ptr$rank4$" = type { ptr, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }
%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@mod_mp_lspinorb_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_noncolin_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nh_ = external dso_local local_unnamed_addr global [10 x i32], align 8
@mod_mp_ityp_ = external dso_local local_unnamed_addr global %"QNCA_a0$ptr$rank1$.0"
@mod_mp_okvan_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_igk_k_ = external dso_local local_unnamed_addr global %"QNCA_a0$ptr$rank2$"
@mod_mp_nppstr_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nspin_lsda_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nat_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_ntyp_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nhm_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nbnd_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_nkb_ = external dso_local local_unnamed_addr global i32, align 8
@mod_mp_npwx_ = external dso_local local_unnamed_addr global i32, align 8
@"var$37" = internal global %"var$36" zeroinitializer
@_DYNTYPE_RECORD_0 = internal global %dyn_typ { ptr @0, ptr null }
@0 = internal unnamed_addr constant [13 x i8] c"MOD#BEC_TYPE\00"
@"var$39" = internal global %"var$38" zeroinitializer
@_DYNTYPE_RECORD_1 = internal global %dyn_typ { ptr @1, ptr null }
@1 = internal unnamed_addr constant [12 x i8] c"intr#real#8\00"
@"var$41" = internal global %"var$40" zeroinitializer
@_DYNTYPE_RECORD_2 = internal global %dyn_typ { ptr @2, ptr null }
@2 = internal unnamed_addr constant [16 x i8] c"intr#complex#16\00"
@"var$43" = internal global %"var$42" zeroinitializer
@_ALLOC_RECORD_LIST_VAR_0 = internal global [10 x ptr] [ptr inttoptr (i64 3 to ptr), ptr inttoptr (i64 2 to ptr), ptr null, ptr @"var$39", ptr inttoptr (i64 2 to ptr), ptr inttoptr (i64 96 to ptr), ptr @"var$41", ptr inttoptr (i64 2 to ptr), ptr inttoptr (i64 192 to ptr), ptr @"var$43"], align 8
@"c_phase_$blk.var$44" = internal global %"MOD$.btBEC_TYPE" { %"QNCA_a0$ptr$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }, %"QNCA_a0$ptr$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }, %"QNCA_a0$ptr$rank3$" { ptr null, i64 0, i64 0, i64 1073741952, i64 3, i64 0, [3 x { i64, i64, i64 }] zeroinitializer }, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0 }, align 8
@_INFO_LIST_VAR_0 = internal global [4 x ptr] [ptr inttoptr (i64 1 to ptr), ptr null, ptr @"c_phase_$blk.var$44", ptr null], align 8
@"var$46" = internal global %"var$45" zeroinitializer
@_ALLOC_RECORD_LIST_VAR_1 = internal global [10 x ptr] [ptr inttoptr (i64 3 to ptr), ptr inttoptr (i64 2 to ptr), ptr null, ptr @"var$39", ptr inttoptr (i64 2 to ptr), ptr inttoptr (i64 96 to ptr), ptr @"var$41", ptr inttoptr (i64 2 to ptr), ptr inttoptr (i64 192 to ptr), ptr @"var$43"], align 8

; Function Attrs: nounwind uwtable
define void @c_phase_() local_unnamed_addr #0 {
alloca_0:
  %"c_phase_$BECP_BP" = alloca %"MOD$.btBEC_TYPE", align 8
  %"c_phase_$BECP0" = alloca %"MOD$.btBEC_TYPE", align 8
  %"c_phase_$Q_DK_SO" = alloca %"QNCA_a0$ptr$rank4$", align 8
  %"c_phase_$L_CAL" = alloca %"QNCA_a0$ptr$rank1$", align 8
  %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 1, i32 6, i64 0, i32 2
  %"c_phase_$BECP_BP.NC$.dim_info$.spacing$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 2, i32 6, i64 0, i32 1
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 2, i32 6, i64 0, i32 2
  %"c_phase_$BECP0.K$.dim_info$.lower_bound$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 1, i32 6, i64 0, i32 2
  %"c_phase_$BECP0.NC$.dim_info$.spacing$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 2, i32 6, i64 0, i32 1
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 2, i32 6, i64 0, i32 2
  %"c_phase_$L_CAL.flags$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"c_phase_$L_CAL", i64 0, i32 3
  %"c_phase_$Q_DK_SO.flags$" = getelementptr inbounds %"QNCA_a0$ptr$rank4$", ptr %"c_phase_$Q_DK_SO", i64 0, i32 3
  %"c_phase_$Q_DK_SO.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank4$", ptr %"c_phase_$Q_DK_SO", i64 0, i32 6, i64 0, i32 1
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$ptr$rank4$", ptr %"c_phase_$Q_DK_SO", i64 0, i32 6, i64 0, i32 2
  %mod_mp_npwx__fetch = load i32, ptr @mod_mp_npwx_, align 8
  %mod_mp_nkb__fetch = load i32, ptr @mod_mp_nkb_, align 8
  %mod_mp_nbnd__fetch = load i32, ptr @mod_mp_nbnd_, align 8
  %mod_mp_nhm__fetch = load i32, ptr @mod_mp_nhm_, align 8
  %mod_mp_ntyp__fetch = load i32, ptr @mod_mp_ntyp_, align 8
  %mod_mp_nat__fetch = load i32, ptr @mod_mp_nat_, align 8
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(24) %"c_phase_$Q_DK_SO", i8 0, i64 24, i1 false)
  store i64 128, ptr %"c_phase_$Q_DK_SO.flags$", align 8
  %_fetch.fca.4.gep = getelementptr inbounds %"QNCA_a0$ptr$rank4$", ptr %"c_phase_$Q_DK_SO", i64 0, i32 4
  store i64 4, ptr %_fetch.fca.4.gep, align 8
  %_fetch.fca.5.gep = getelementptr inbounds %"QNCA_a0$ptr$rank4$", ptr %"c_phase_$Q_DK_SO", i64 0, i32 5
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(104) %_fetch.fca.5.gep, i8 0, i64 104, i1 false)
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(24) %"c_phase_$L_CAL", i8 0, i64 24, i1 false)
  store i64 128, ptr %"c_phase_$L_CAL.flags$", align 8
  %_fetch93.fca.4.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"c_phase_$L_CAL", i64 0, i32 4
  store i64 1, ptr %_fetch93.fca.4.gep, align 8
  %_fetch93.fca.5.gep = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"c_phase_$L_CAL", i64 0, i32 5
  %int_sext = sext i32 %mod_mp_nat__fetch to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %"c_phase_$STRUC" = alloca %complex_128bit, i64 %slct, align 1
  %int_sext2 = sext i32 %mod_mp_nhm__fetch to i64
  %rel4 = icmp sgt i64 %int_sext2, 0
  %slct6 = select i1 %rel4, i64 %int_sext2, i64 0
  %mul8 = shl nuw nsw i64 %slct6, 4
  %mul18 = mul nsw i64 %mul8, %slct6
  %int_sext20 = sext i32 %mod_mp_ntyp__fetch to i64
  %rel22 = icmp sgt i64 %int_sext20, 0
  %slct24 = select i1 %rel22, i64 %int_sext20, i64 0
  %mul26 = mul nsw i64 %mul18, %slct24
  %div28 = lshr exact i64 %mul26, 4
  %"c_phase_$Q_DK" = alloca %complex_128bit, i64 %div28, align 1
  %int_sext30 = sext i32 %mod_mp_nbnd__fetch to i64
  %rel32 = icmp sgt i64 %int_sext30, 0
  %slct34 = select i1 %rel32, i64 %int_sext30, i64 0
  %mul36 = shl nuw nsw i64 %slct34, 4
  %mul46 = mul nsw i64 %mul36, %slct34
  %div48 = lshr exact i64 %mul46, 4
  %"c_phase_$MAT" = alloca %complex_128bit, i64 %div48, align 1
  %int_sext50 = sext i32 %mod_mp_nkb__fetch to i64
  %rel52 = icmp sgt i64 %int_sext50, 0
  %slct54 = select i1 %rel52, i64 %int_sext50, i64 0
  %"c_phase_$NKBTONH" = alloca i32, i64 %slct54, align 1
  %"c_phase_$NKBTONA" = alloca i32, i64 %slct54, align 1
  %int_sext72 = sext i32 %mod_mp_npwx__fetch to i64
  %rel74 = icmp sgt i64 %int_sext72, 0
  %slct76 = select i1 %rel74, i64 %int_sext72, i64 0
  %"c_phase_$IGK0" = alloca i32, i64 %slct76, align 1
  %mul1291 = shl nsw i64 %int_sext2, 4
  %mul1294 = mul nsw i64 %mul1291, %int_sext2
  %mul1402 = shl nsw i64 %int_sext30, 4
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(32) %_fetch93.fca.5.gep, i8 0, i64 32, i1 false)
  store i64 1248, ptr getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 0, i32 3), align 8
  store i64 2, ptr getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 0, i32 2), align 16
  store ptr @_DYNTYPE_RECORD_1, ptr getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 1), align 16
  store ptr null, ptr getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 2), align 8
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(48) getelementptr inbounds (%"var$38", ptr @"var$39", i64 0, i32 4), i8 0, i64 48, i1 false)
  store i64 1248, ptr getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 0, i32 3), align 8
  store i64 2, ptr getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 0, i32 2), align 16
  store ptr @_DYNTYPE_RECORD_2, ptr getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 1), align 16
  store ptr null, ptr getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 2), align 8
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(48) getelementptr inbounds (%"var$40", ptr @"var$41", i64 0, i32 4), i8 0, i64 48, i1 false)
  store i64 1248, ptr getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 0, i32 3), align 8
  store i64 3, ptr getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 0, i32 2), align 16
  store ptr @_DYNTYPE_RECORD_2, ptr getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 1), align 8
  store ptr null, ptr getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 2), align 16
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(48) getelementptr inbounds (%"var$42", ptr @"var$43", i64 0, i32 4), i8 0, i64 48, i1 false)
  store i64 336, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 1), align 8
  store i64 0, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 4), align 16
  store ptr null, ptr @"var$37", align 16
  store i64 1091, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 2), align 16
  store ptr @_DYNTYPE_RECORD_0, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 1), align 16
  store ptr null, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 2), align 8
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(24) getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 5), i8 0, i64 24, i1 false)
  store ptr @_ALLOC_RECORD_LIST_VAR_0, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 4), align 8
  store ptr @_INFO_LIST_VAR_0, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 8), align 8
  store ptr null, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 9), align 16
  store i64 336, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 1), align 8
  store i64 0, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 4), align 16
  store ptr null, ptr @"var$46", align 16
  store i64 1091, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 2), align 16
  store ptr @_DYNTYPE_RECORD_0, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 1), align 16
  store ptr null, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 2), align 8
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(24) getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 5), i8 0, i64 24, i1 false)
  store ptr @_ALLOC_RECORD_LIST_VAR_1, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 4), align 8
  store ptr @_INFO_LIST_VAR_0, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 8), align 8
  store ptr null, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 9), align 16
  %"c_phase_$blk.var$20_fetch.fca.0.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 0, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(24) %"c_phase_$BECP0", i8 0, i64 24, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$20_fetch.fca.0.3.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.0.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 0, i32 4
  store i64 2, ptr %"c_phase_$blk.var$20_fetch.fca.0.4.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.0.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 0, i32 5
  %"c_phase_$blk.var$20_fetch.fca.1.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 1, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(80) %"c_phase_$blk.var$20_fetch.fca.0.5.gep", i8 0, i64 80, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$20_fetch.fca.1.3.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.1.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 1, i32 4
  store i64 2, ptr %"c_phase_$blk.var$20_fetch.fca.1.4.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.1.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 1, i32 5
  %"c_phase_$blk.var$20_fetch.fca.2.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 2, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(80) %"c_phase_$blk.var$20_fetch.fca.1.5.gep", i8 0, i64 80, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$20_fetch.fca.2.3.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.2.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 2, i32 4
  store i64 3, ptr %"c_phase_$blk.var$20_fetch.fca.2.4.gep", align 8
  %"c_phase_$blk.var$20_fetch.fca.2.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 2, i32 5
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(104) %"c_phase_$blk.var$20_fetch.fca.2.5.gep", i8 0, i64 104, i1 false)
  %"c_phase_$blk.var$21_fetch.fca.0.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 0, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(24) %"c_phase_$BECP_BP", i8 0, i64 24, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$21_fetch.fca.0.3.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.0.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 0, i32 4
  store i64 2, ptr %"c_phase_$blk.var$21_fetch.fca.0.4.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.0.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 0, i32 5
  %"c_phase_$blk.var$21_fetch.fca.1.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 1, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(80) %"c_phase_$blk.var$21_fetch.fca.0.5.gep", i8 0, i64 80, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$21_fetch.fca.1.3.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.1.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 1, i32 4
  store i64 2, ptr %"c_phase_$blk.var$21_fetch.fca.1.4.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.1.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 1, i32 5
  %"c_phase_$blk.var$21_fetch.fca.2.3.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 2, i32 3
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(80) %"c_phase_$blk.var$21_fetch.fca.1.5.gep", i8 0, i64 80, i1 false)
  store i64 1073741952, ptr %"c_phase_$blk.var$21_fetch.fca.2.3.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.2.4.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 2, i32 4
  store i64 3, ptr %"c_phase_$blk.var$21_fetch.fca.2.4.gep", align 8
  %"c_phase_$blk.var$21_fetch.fca.2.5.gep" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 2, i32 5
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(104) %"c_phase_$blk.var$21_fetch.fca.2.5.gep", i8 0, i64 104, i1 false)
  %mod_mp_nspin_lsda__fetch = load i32, ptr @mod_mp_nspin_lsda_, align 8
  %rel95 = icmp slt i32 %mod_mp_nspin_lsda__fetch, 1
  br i1 %rel95, label %bb6, label %bb17.preheader

bb17.preheader:                                   ; preds = %alloca_0
  %"c_phase_$L_CAL.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"c_phase_$L_CAL", i64 0, i32 6, i64 0, i32 2
  %"c_phase_$BECP0.K$.dim_info$.spacing$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP0", i64 0, i32 1, i32 6, i64 0, i32 1
  %"c_phase_$BECP_BP.K$.dim_info$.spacing$" = getelementptr inbounds %"MOD$.btBEC_TYPE", ptr %"c_phase_$BECP_BP", i64 0, i32 1, i32 6, i64 0, i32 1
  %mod_mp_nppstr__fetch = load i32, ptr @mod_mp_nppstr_, align 8
  %rel99 = icmp slt i32 %mod_mp_nppstr__fetch, 1
  %_fetch1476 = load ptr, ptr @mod_mp_igk_k_, align 8
  %"[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank2$", ptr elementtype(%"QNCA_a0$ptr$rank2$") @mod_mp_igk_k_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %"[]_fetch" = load i64, ptr %"[]", align 1
  %"[]118" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank2$", ptr elementtype(%"QNCA_a0$ptr$rank2$") @mod_mp_igk_k_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %"[]118_fetch" = load i64, ptr %"[]118", align 1
  %"[]121" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank2$", ptr elementtype(%"QNCA_a0$ptr$rank2$") @mod_mp_igk_k_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %"[]121_fetch" = load i64, ptr %"[]121", align 1
  %rel148.not1826 = icmp slt i32 %mod_mp_npwx__fetch, 1
  %"_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"[]121_fetch", i64 %"[]118_fetch", ptr elementtype(i32) %_fetch1476, i64 0)
  %mod_mp_nbnd__fetch158 = load i32, ptr @mod_mp_nbnd_, align 8
  %rel160 = icmp slt i32 %mod_mp_nbnd__fetch158, 1
  %"c_phase_$L_CAL.addr_a0$_fetch1449" = load ptr, ptr %"c_phase_$L_CAL", align 8
  %"c_phase_$L_CAL.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$L_CAL.dim_info$.lower_bound$", i32 0)
  %"c_phase_$L_CAL.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$L_CAL.dim_info$.lower_bound$[]", align 1
  %mod_mp_okvan__fetch = load i32, ptr @mod_mp_okvan_, align 8
  %and1445 = and i32 %mod_mp_okvan__fetch, 1
  %rel1446.not = icmp eq i32 %and1445, 0
  %mod_mp_nkb__fetch1440 = load i32, ptr @mod_mp_nkb_, align 8
  %rel1442 = icmp slt i32 %mod_mp_nkb__fetch1440, 1
  %_fetch225 = load ptr, ptr @mod_mp_ityp_, align 8
  %"[]214" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$.0", ptr elementtype(%"QNCA_a0$ptr$rank1$.0") @mod_mp_ityp_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %"[]214_fetch" = load i64, ptr %"[]214", align 1
  %mod_mp_noncolin__fetch = load i32, ptr @mod_mp_noncolin_, align 8
  %and1378 = and i32 %mod_mp_noncolin__fetch, 1
  %rel1379.not = icmp eq i32 %and1378, 0
  %mod_mp_lspinorb__fetch = load i32, ptr @mod_mp_lspinorb_, align 8
  %and1373 = and i32 %mod_mp_lspinorb__fetch, 1
  %rel1374.not = icmp eq i32 %and1373, 0
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.NC$.dim_info$.lower_bound$", i32 0)
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]", align 1
  %"c_phase_$BECP0.NC$.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.NC$.dim_info$.spacing$", i32 1)
  %"c_phase_$BECP0.NC$.dim_info$.spacing$[]_fetch" = load i64, ptr %"c_phase_$BECP0.NC$.dim_info$.spacing$[]", align 1
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]243" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.NC$.dim_info$.lower_bound$", i32 1)
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]243_fetch" = load i64, ptr %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]243", align 1
  %"c_phase_$BECP0.NC$.dim_info$.spacing$[]246" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.NC$.dim_info$.spacing$", i32 2)
  %"c_phase_$BECP0.NC$.dim_info$.spacing$[]246_fetch" = load i64, ptr %"c_phase_$BECP0.NC$.dim_info$.spacing$[]246", align 1
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]249" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.NC$.dim_info$.lower_bound$", i32 2)
  %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]249_fetch" = load i64, ptr %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]249", align 1
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$", i32 0)
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]", align 1
  %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.NC$.dim_info$.spacing$", i32 1)
  %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]_fetch" = load i64, ptr %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]", align 1
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]297" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$", i32 1)
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]297_fetch" = load i64, ptr %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]297", align 1
  %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]300" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.NC$.dim_info$.spacing$", i32 2)
  %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]300_fetch" = load i64, ptr %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]300", align 1
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]303" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$", i32 2)
  %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]303_fetch" = load i64, ptr %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]303", align 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch" = load ptr, ptr %"c_phase_$Q_DK_SO", align 8
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.lower_bound$", i32 0)
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]", align 1
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.spacing$", i32 1)
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.spacing$[]", align 1
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.lower_bound$", i32 1)
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365", align 1
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.spacing$", i32 2)
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372", align 1
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.lower_bound$", i32 2)
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375", align 1
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]378" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.spacing$", i32 3)
  %"c_phase_$Q_DK_SO.dim_info$.spacing$[]378_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.spacing$[]378", align 1
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]381" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$Q_DK_SO.dim_info$.lower_bound$", i32 3)
  %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]381_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]381", align 1
  %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.K$.dim_info$.lower_bound$", i32 0)
  %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]", align 1
  %"c_phase_$BECP0.K$.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.K$.dim_info$.spacing$", i32 1)
  %"c_phase_$BECP0.K$.dim_info$.spacing$[]_fetch" = load i64, ptr %"c_phase_$BECP0.K$.dim_info$.spacing$[]", align 1
  %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]1205" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP0.K$.dim_info$.lower_bound$", i32 1)
  %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]1205_fetch" = load i64, ptr %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]1205", align 1
  %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$", i32 0)
  %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]", align 1
  %"c_phase_$BECP_BP.K$.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.K$.dim_info$.spacing$", i32 1)
  %"c_phase_$BECP_BP.K$.dim_info$.spacing$[]_fetch" = load i64, ptr %"c_phase_$BECP_BP.K$.dim_info$.spacing$[]", align 1
  %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]1249" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$", i32 1)
  %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]1249_fetch" = load i64, ptr %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]1249", align 1
  %0 = add nsw i64 %int_sext72, 1
  %1 = add nuw nsw i32 %mod_mp_nkb__fetch1440, 1
  %2 = add nuw nsw i32 %mod_mp_nbnd__fetch158, 1
  %3 = add nsw i32 %mod_mp_nppstr__fetch, 1
  %wide.trip.count1840 = sext i32 %2 to i64
  %wide.trip.count1832 = sext i32 %1 to i64
  br label %bb17

bb17:                                             ; preds = %bb17.preheader, %bb22
  br i1 %rel99, label %bb22, label %bb21.preheader

bb21.preheader:                                   ; preds = %bb17
  br label %bb21

bb21:                                             ; preds = %bb21.preheader, %bb529_endif
  %"c_phase_$KPAR.0" = phi i32 [ %add1483, %bb529_endif ], [ 1, %bb21.preheader ]
  %rel1477.not = icmp eq i32 %"c_phase_$KPAR.0", 1
  br i1 %rel1477.not, label %bb529_endif, label %bb26_then

bb54:                                             ; preds = %bb54.preheader, %bb54
  %"var$26.01828" = phi i64 [ %add152, %bb54 ], [ %"[]_fetch", %bb54.preheader ]
  %"var$25.01827" = phi i64 [ %add156, %bb54 ], [ 1, %bb54.preheader ]
  %"_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"[]_fetch", i64 4, ptr elementtype(i32) %"_fetch[]", i64 %"var$26.01828")
  %"_fetch[][]_fetch" = load i32, ptr %"_fetch[][]", align 1
  %"c_phase_$IGK0[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %"c_phase_$IGK0", i64 %"var$25.01827")
  store i32 %"_fetch[][]_fetch", ptr %"c_phase_$IGK0[]", align 1
  %add152 = add nsw i64 %"var$26.01828", 1
  %add156 = add nuw nsw i64 %"var$25.01827", 1
  %exitcond = icmp eq i64 %add156, %0
  br i1 %exitcond, label %bb60.loopexit, label %bb54

bb60.loopexit:                                    ; preds = %bb54
  br label %bb60

bb60:                                             ; preds = %bb60.loopexit, %bb26_then
  br i1 %rel160, label %bb529_endif, label %bb65.preheader.preheader

bb65.preheader.preheader:                         ; preds = %bb60
  br label %bb65.preheader

bb65.preheader:                                   ; preds = %bb65.preheader.preheader, %bb66
  %indvars.iv1838 = phi i64 [ %indvars.iv.next1839, %bb66 ], [ 1, %bb65.preheader.preheader ]
  %"c_phase_$L_CAL.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"c_phase_$L_CAL.dim_info$.lower_bound$[]_fetch", i64 4, ptr elementtype(i32) %"c_phase_$L_CAL.addr_a0$_fetch1449", i64 %indvars.iv1838)
  %"c_phase_$L_CAL.addr_a0$_fetch[]_fetch" = load i32, ptr %"c_phase_$L_CAL.addr_a0$_fetch[]", align 1
  %4 = and i32 %"c_phase_$L_CAL.addr_a0$_fetch[]_fetch", 1
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]249_fetch", i64 %"c_phase_$BECP0.NC$.dim_info$.spacing$[]246_fetch", ptr elementtype(%complex_128bit) null, i64 %indvars.iv1838)
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]243_fetch", i64 %"c_phase_$BECP0.NC$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$BECP0.NC$.addr_a0$_fetch[]", i64 1)
  %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]243_fetch", i64 %"c_phase_$BECP0.NC$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$BECP0.NC$.addr_a0$_fetch[]", i64 2)
  %"c_phase_$BECP0.K$.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]1205_fetch", i64 %"c_phase_$BECP0.K$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) null, i64 %indvars.iv1838)
  br label %bb65

bb65:                                             ; preds = %bb65.preheader, %bb527_endif
  %indvars.iv1834 = phi i64 [ 1, %bb65.preheader ], [ %indvars.iv.next1835, %bb527_endif ]
  %"c_phase_$L_CAL.addr_a0$_fetch[]190" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"c_phase_$L_CAL.dim_info$.lower_bound$[]_fetch", i64 4, ptr elementtype(i32) %"c_phase_$L_CAL.addr_a0$_fetch1449", i64 %indvars.iv1834)
  %"c_phase_$L_CAL.addr_a0$_fetch[]190_fetch" = load i32, ptr %"c_phase_$L_CAL.addr_a0$_fetch[]190", align 1
  %5 = and i32 %4, %"c_phase_$L_CAL.addr_a0$_fetch[]190_fetch"
  %.not = icmp eq i32 %5, 0
  %brmerge = or i1 %.not, %rel1446.not
  br i1 %brmerge, label %bb527_endif, label %bb84_then

bb85:                                             ; preds = %bb85.preheader, %bb107
  %indvars.iv1830 = phi i64 [ 1, %bb85.preheader ], [ %indvars.iv.next1831, %bb107 ]
  %"c_phase_$PREF.sroa.0.0" = phi double [ 0.000000e+00, %bb85.preheader ], [ %"c_phase_$PREF.sroa.0.3", %bb107 ]
  %"c_phase_$PREF.sroa.6.0" = phi double [ 0.000000e+00, %bb85.preheader ], [ %"c_phase_$PREF.sroa.6.3", %bb107 ]
  %"c_phase_$NKBTONH[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %"c_phase_$NKBTONH", i64 %indvars.iv1830)
  %"c_phase_$NKBTONH[]_fetch" = load i32, ptr %"c_phase_$NKBTONH[]", align 1
  %"c_phase_$NKBTONA[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %"c_phase_$NKBTONA", i64 %indvars.iv1830)
  %"c_phase_$NKBTONA[]_fetch" = load i32, ptr %"c_phase_$NKBTONA[]", align 1
  %int_sext216 = sext i32 %"c_phase_$NKBTONA[]_fetch" to i64
  %"_fetch[]224" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"[]214_fetch", i64 4, ptr elementtype(i32) %_fetch225, i64 %int_sext216)
  %"_fetch[]224_fetch" = load i32, ptr %"_fetch[]224", align 1
  %int_sext228 = sext i32 %"_fetch[]224_fetch" to i64
  %"mod_mp_nh_[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @mod_mp_nh_, i64 %int_sext228)
  %"mod_mp_nh_[]_fetch" = load i32, ptr %"mod_mp_nh_[]", align 1
  %6 = trunc i64 %indvars.iv1830 to i32
  %sub230 = sub nsw i32 %6, %"c_phase_$NKBTONH[]_fetch"
  %rel232 = icmp slt i32 %"mod_mp_nh_[]_fetch", 1
  br i1 %rel232, label %bb107, label %bb106.preheader

bb106.preheader:                                  ; preds = %bb85
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP0.NC$.addr_a0$_fetch[][]", i64 %indvars.iv1830)
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.elt1794" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]", i64 0, i32 1
  %int_sext360 = sext i32 %"c_phase_$NKBTONH[]_fetch" to i64
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 3, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]381_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]378_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch", i64 %int_sext228)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[]", i64 1)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[]", i64 2)
  %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP0.NC$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][]", i64 %indvars.iv1830)
  %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.elt1809" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]", i64 0, i32 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[]", i64 3)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]375_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]372_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[]", i64 4)
  %"c_phase_$STRUC[]" = call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 1, i64 16, ptr elementtype(%complex_128bit) nonnull %"c_phase_$STRUC", i64 %int_sext216)
  %"c_phase_$STRUC[]_fetch.elt1818" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$STRUC[]", i64 0, i32 1
  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP0.K$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP0.K$.addr_a0$_fetch[]", i64 %indvars.iv1830)
  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.elt1777" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP0.K$.addr_a0$_fetch[][]", i64 0, i32 1
  %"c_phase_$Q_DK[]" = call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 1, i64 %mul1294, ptr elementtype(%complex_128bit) nonnull %"c_phase_$Q_DK", i64 %int_sext228)
  %7 = add nuw nsw i32 %"mod_mp_nh_[]_fetch", 1
  %wide.trip.count = sext i32 %7 to i64
  br label %bb106

bb106:                                            ; preds = %bb106.preheader, %bb512_endif
  %indvars.iv = phi i64 [ 1, %bb106.preheader ], [ %indvars.iv.next, %bb512_endif ]
  %"c_phase_$PREF.sroa.0.1" = phi double [ %"c_phase_$PREF.sroa.0.0", %bb106.preheader ], [ %"c_phase_$PREF.sroa.0.2", %bb512_endif ]
  %"c_phase_$PREF.sroa.6.1" = phi double [ %"c_phase_$PREF.sroa.6.0", %bb106.preheader ], [ %"c_phase_$PREF.sroa.6.2", %bb512_endif ]
  br i1 %rel1379.not, label %bb469_else, label %bb111_then

bb112_then:                                       ; preds = %bb111_then
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack" = load double, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]", align 1
  %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795" = load double, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.elt1794", align 1
  %8 = trunc i64 %indvars.iv to i32
  %add290 = add nsw i32 %sub230, %8
  %int_sext292 = sext i32 %add290 to i64
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][]", i64 %int_sext292)
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack" = load double, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]", align 1
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.elt1797" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]", i64 0, i32 1
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798" = load double, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.elt1797", align 1
  %mul342 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %9 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795"
  %sub344 = fadd fast double %9, %mul342
  %mul348 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %10 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795"
  %add350 = fsub fast double %mul348, %10
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[][]", i64 %indvars.iv)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][]", i64 %int_sext360)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]", align 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.elt1800" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]", i64 0, i32 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.elt1800", align 1
  %mul432 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack", %sub344
  %mul430.neg = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801", %add350
  %mul436 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack", %add350
  %mul438 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch[][][][]_fetch.unpack1801", %sub344
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][]", i64 %int_sext292)
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack" = load double, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]", align 1
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.elt1803" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]", i64 0, i32 1
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804" = load double, ptr %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.elt1803", align 1
  %mul569 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %11 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795"
  %sub571 = fadd fast double %11, %mul569
  %mul575 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %12 = fmul fast double %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack", %"c_phase_$BECP0.NC$.addr_a0$_fetch[][][]_fetch.unpack1795"
  %add577 = fsub fast double %mul575, %12
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][]", i64 %indvars.iv)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][]", i64 %int_sext360)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]", align 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.elt1806" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]", i64 0, i32 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.elt1806", align 1
  %mul663 = fmul fast double %sub571, %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack"
  %mul661.neg = fmul fast double %add577, %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807"
  %mul667 = fmul fast double %add577, %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack"
  %mul669 = fmul fast double %sub571, %"c_phase_$Q_DK_SO.addr_a0$_fetch583[][][][]_fetch.unpack1807"
  %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack" = load double, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]", align 1
  %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810" = load double, ptr %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.elt1809", align 1
  %mul809 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %13 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"
  %sub811 = fadd fast double %13, %mul809
  %mul815 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack1798"
  %14 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][][]_fetch.unpack"
  %add817 = fsub fast double %mul815, %14
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][]", i64 %indvars.iv)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][]", i64 %int_sext360)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]", align 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.elt1812" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]", i64 0, i32 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.elt1812", align 1
  %mul903 = fmul fast double %sub811, %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack"
  %mul901.neg = fmul fast double %add817, %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813"
  %mul907 = fmul fast double %add817, %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack"
  %mul909 = fmul fast double %sub811, %"c_phase_$Q_DK_SO.addr_a0$_fetch823[][][][]_fetch.unpack1813"
  %mul1049 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"
  %15 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"
  %sub1051 = fadd fast double %15, %mul1049
  %mul1055 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack1804"
  %16 = fmul fast double %"c_phase_$BECP0.NC$.addr_a0$_fetch685[][][]_fetch.unpack1810", %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][][]_fetch.unpack"
  %add1057 = fsub fast double %mul1055, %16
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]365_fetch", i64 %"c_phase_$Q_DK_SO.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][]", i64 %indvars.iv)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$Q_DK_SO.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][]", i64 %int_sext360)
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]", align 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.elt1815" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]", i64 0, i32 1
  %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816" = load double, ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.elt1815", align 1
  %mul1141 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816", %add1057
  %mul1143 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack", %sub1051
  %mul1147 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack", %add1057
  %mul1149 = fmul fast double %"c_phase_$Q_DK_SO.addr_a0$_fetch1063[][][][]_fetch.unpack1816", %sub1051
  %reass.add = fadd fast double %mul661.neg, %mul430.neg
  %reass.add1825 = fadd fast double %reass.add, %mul901.neg
  %sub434 = fadd fast double %mul663, %mul432
  %add677 = fadd fast double %sub434, %mul903
  %17 = fadd fast double %mul1143, %add677
  %18 = fadd fast double %mul1141, %reass.add1825
  %add1157 = fsub fast double %17, %18
  %add1151 = fadd fast double %mul438, %mul436
  %add911 = fadd fast double %add1151, %mul667
  %add671 = fadd fast double %add911, %mul669
  %add440 = fadd fast double %add671, %mul907
  %add679 = fadd fast double %add440, %mul909
  %add919 = fadd fast double %add679, %mul1147
  %add1159 = fadd fast double %add919, %mul1149
  %"c_phase_$STRUC[]_fetch.unpack" = load double, ptr %"c_phase_$STRUC[]", align 1
  %"c_phase_$STRUC[]_fetch.unpack1819" = load double, ptr %"c_phase_$STRUC[]_fetch.elt1818", align 1
  %mul1173 = fmul fast double %add1157, %"c_phase_$STRUC[]_fetch.unpack"
  %mul1177 = fmul fast double %add1159, %"c_phase_$STRUC[]_fetch.unpack"
  %mul1179 = fmul fast double %add1157, %"c_phase_$STRUC[]_fetch.unpack1819"
  %19 = fmul fast double %add1159, %"c_phase_$STRUC[]_fetch.unpack1819"
  %sub1175 = fsub fast double %"c_phase_$PREF.sroa.0.1", %19
  %add1187 = fadd fast double %sub1175, %mul1173
  %add1181 = fadd fast double %mul1177, %"c_phase_$PREF.sroa.6.1"
  %add1189 = fadd fast double %add1181, %mul1179
  br label %bb512_endif

bb111_then:                                       ; preds = %bb106
  br i1 %rel1374.not, label %bb512_endif, label %bb112_then

bb469_else:                                       ; preds = %bb106
  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack" = load double, ptr %"c_phase_$BECP0.K$.addr_a0$_fetch[][]", align 1
  %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778" = load double, ptr %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.elt1777", align 1
  %20 = trunc i64 %indvars.iv to i32
  %add1242 = add nsw i32 %sub230, %20
  %int_sext1244 = sext i32 %add1242 to i64
  %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]_fetch", i64 16, ptr elementtype(%complex_128bit) %"c_phase_$BECP_BP.K$.addr_a0$_fetch[]", i64 %int_sext1244)
  %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack" = load double, ptr %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]", align 1
  %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.elt1780" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]", i64 0, i32 1
  %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781" = load double, ptr %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.elt1780", align 1
  %mul1276 = fmul fast double %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack", %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack"
  %21 = fmul fast double %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781", %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778"
  %sub1278 = fadd fast double %21, %mul1276
  %mul1282 = fmul fast double %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack1781", %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack"
  %22 = fmul fast double %"c_phase_$BECP_BP.K$.addr_a0$_fetch[][]_fetch.unpack", %"c_phase_$BECP0.K$.addr_a0$_fetch[][]_fetch.unpack1778"
  %add1284 = fsub fast double %mul1282, %22
  %"c_phase_$Q_DK[][]" = call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 1, i64 %mul1291, ptr elementtype(%complex_128bit) nonnull %"c_phase_$Q_DK[]", i64 %indvars.iv)
  %"c_phase_$Q_DK[][][]" = call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 1, i64 16, ptr elementtype(%complex_128bit) nonnull %"c_phase_$Q_DK[][]", i64 %int_sext360)
  %"c_phase_$Q_DK[][][]_fetch.unpack" = load double, ptr %"c_phase_$Q_DK[][][]", align 1
  %"c_phase_$Q_DK[][][]_fetch.elt1783" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$Q_DK[][][]", i64 0, i32 1
  %"c_phase_$Q_DK[][][]_fetch.unpack1784" = load double, ptr %"c_phase_$Q_DK[][][]_fetch.elt1783", align 1
  %mul1323 = fmul fast double %add1284, %"c_phase_$Q_DK[][][]_fetch.unpack1784"
  %mul1325 = fmul fast double %sub1278, %"c_phase_$Q_DK[][][]_fetch.unpack"
  %sub1327 = fsub fast double %mul1325, %mul1323
  %mul1329 = fmul fast double %add1284, %"c_phase_$Q_DK[][][]_fetch.unpack"
  %mul1331 = fmul fast double %sub1278, %"c_phase_$Q_DK[][][]_fetch.unpack1784"
  %add1333 = fadd fast double %mul1331, %mul1329
  %"c_phase_$STRUC[]1344_fetch.unpack" = load double, ptr %"c_phase_$STRUC[]", align 1
  %"c_phase_$STRUC[]1344_fetch.unpack1787" = load double, ptr %"c_phase_$STRUC[]_fetch.elt1818", align 1
  %mul1348 = fmul fast double %sub1327, %"c_phase_$STRUC[]1344_fetch.unpack"
  %mul1352 = fmul fast double %add1333, %"c_phase_$STRUC[]1344_fetch.unpack"
  %mul1354 = fmul fast double %sub1327, %"c_phase_$STRUC[]1344_fetch.unpack1787"
  %sub1350 = fadd fast double %mul1348, %"c_phase_$PREF.sroa.0.1"
  %23 = fmul fast double %add1333, %"c_phase_$STRUC[]1344_fetch.unpack1787"
  %add1364 = fsub fast double %sub1350, %23
  %add1356 = fadd fast double %mul1352, %"c_phase_$PREF.sroa.6.1"
  %add1368 = fadd fast double %add1356, %mul1354
  br label %bb512_endif

bb512_endif:                                      ; preds = %bb111_then, %bb112_then, %bb469_else
  %"c_phase_$PREF.sroa.0.2" = phi double [ %add1187, %bb112_then ], [ %"c_phase_$PREF.sroa.0.1", %bb111_then ], [ %add1364, %bb469_else ]
  %"c_phase_$PREF.sroa.6.2" = phi double [ %add1189, %bb112_then ], [ %"c_phase_$PREF.sroa.6.1", %bb111_then ], [ %add1368, %bb469_else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond1829 = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond1829, label %bb107.loopexit, label %bb106

bb107.loopexit:                                   ; preds = %bb512_endif
  %"c_phase_$PREF.sroa.0.2.lcssa" = phi double [ %"c_phase_$PREF.sroa.0.2", %bb512_endif ]
  %"c_phase_$PREF.sroa.6.2.lcssa" = phi double [ %"c_phase_$PREF.sroa.6.2", %bb512_endif ]
  br label %bb107

bb107:                                            ; preds = %bb107.loopexit, %bb85
  %"c_phase_$PREF.sroa.0.3" = phi double [ %"c_phase_$PREF.sroa.0.0", %bb85 ], [ %"c_phase_$PREF.sroa.0.2.lcssa", %bb107.loopexit ]
  %"c_phase_$PREF.sroa.6.3" = phi double [ %"c_phase_$PREF.sroa.6.0", %bb85 ], [ %"c_phase_$PREF.sroa.6.2.lcssa", %bb107.loopexit ]
  %indvars.iv.next1831 = add nuw nsw i64 %indvars.iv1830, 1
  %exitcond1833 = icmp eq i64 %indvars.iv.next1831, %wide.trip.count1832
  br i1 %exitcond1833, label %bb514.loopexit, label %bb85

bb514.loopexit:                                   ; preds = %bb107
  %"c_phase_$PREF.sroa.0.3.lcssa" = phi double [ %"c_phase_$PREF.sroa.0.3", %bb107 ]
  %"c_phase_$PREF.sroa.6.3.lcssa" = phi double [ %"c_phase_$PREF.sroa.6.3", %bb107 ]
  br label %bb514

bb514:                                            ; preds = %bb514.loopexit, %bb84_then
  %"c_phase_$PREF.sroa.0.4" = phi double [ 0.000000e+00, %bb84_then ], [ %"c_phase_$PREF.sroa.0.3.lcssa", %bb514.loopexit ]
  %"c_phase_$PREF.sroa.6.4" = phi double [ 0.000000e+00, %bb84_then ], [ %"c_phase_$PREF.sroa.6.3.lcssa", %bb514.loopexit ]
  %"c_phase_$MAT[]" = call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 1, i64 %mul1402, ptr elementtype(%complex_128bit) nonnull %"c_phase_$MAT", i64 %indvars.iv1834)
  %"c_phase_$MAT[][]" = call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 1, i64 16, ptr elementtype(%complex_128bit) nonnull %"c_phase_$MAT[]", i64 %indvars.iv1838)
  %"c_phase_$MAT[][]_fetch.unpack" = load double, ptr %"c_phase_$MAT[][]", align 1
  %"c_phase_$MAT[][]_fetch.elt1789" = getelementptr inbounds %complex_128bit, ptr %"c_phase_$MAT[][]", i64 0, i32 1
  %"c_phase_$MAT[][]_fetch.unpack1790" = load double, ptr %"c_phase_$MAT[][]_fetch.elt1789", align 1
  %add1414 = fadd fast double %"c_phase_$MAT[][]_fetch.unpack", %"c_phase_$PREF.sroa.0.4"
  %add1416 = fadd fast double %"c_phase_$MAT[][]_fetch.unpack1790", %"c_phase_$PREF.sroa.6.4"
  store double %add1414, ptr %"c_phase_$MAT[][]", align 1
  store double %add1416, ptr %"c_phase_$MAT[][]_fetch.elt1789", align 1
  br label %bb527_endif

bb84_then:                                        ; preds = %bb65
  br i1 %rel1442, label %bb514, label %bb85.preheader

bb85.preheader:                                   ; preds = %bb84_then
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 2, i64 %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]303_fetch", i64 %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]300_fetch", ptr elementtype(%complex_128bit) null, i64 %indvars.iv1834)
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]297_fetch", i64 %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[]", i64 1)
  %"c_phase_$BECP_BP.NC$.addr_a0$_fetch507[][]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP_BP.NC$.dim_info$.lower_bound$[]297_fetch", i64 %"c_phase_$BECP_BP.NC$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) %"c_phase_$BECP_BP.NC$.addr_a0$_fetch[]", i64 2)
  %"c_phase_$BECP_BP.K$.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64(i8 1, i64 %"c_phase_$BECP_BP.K$.dim_info$.lower_bound$[]1249_fetch", i64 %"c_phase_$BECP_BP.K$.dim_info$.spacing$[]_fetch", ptr elementtype(%complex_128bit) null, i64 %indvars.iv1834)
  br label %bb85

bb527_endif:                                      ; preds = %bb65, %bb514
  %indvars.iv.next1835 = add nuw nsw i64 %indvars.iv1834, 1
  %exitcond1837 = icmp eq i64 %indvars.iv.next1835, %wide.trip.count1840
  br i1 %exitcond1837, label %bb66, label %bb65

bb66:                                             ; preds = %bb527_endif
  %indvars.iv.next1839 = add nuw nsw i64 %indvars.iv1838, 1
  %exitcond1841 = icmp eq i64 %indvars.iv.next1839, %wide.trip.count1840
  br i1 %exitcond1841, label %bb529_endif.loopexit, label %bb65.preheader

bb26_then:                                        ; preds = %bb21
  br i1 %rel148.not1826, label %bb60, label %bb54.preheader

bb54.preheader:                                   ; preds = %bb26_then
  br label %bb54

bb529_endif.loopexit:                             ; preds = %bb66
  br label %bb529_endif

bb529_endif:                                      ; preds = %bb529_endif.loopexit, %bb21, %bb60
  %add1483 = add nuw nsw i32 %"c_phase_$KPAR.0", 1
  %exitcond1842 = icmp eq i32 %add1483, %3
  br i1 %exitcond1842, label %bb22.loopexit, label %bb21

bb22.loopexit:                                    ; preds = %bb529_endif
  br label %bb22

bb22:                                             ; preds = %bb22.loopexit, %bb17
  br label %bb17

bb6:                                              ; preds = %alloca_0
  %_fetch1510 = load i64, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 3), align 8
  %or1512 = or i64 %_fetch1510, 1024
  store i64 %or1512, ptr getelementptr inbounds (%"var$36", ptr @"var$37", i64 0, i32 0, i32 3), align 8
  %func_result1514 = call i32 @for_dealloc_all_nocheck(ptr @"var$37", ptr nonnull %"c_phase_$BECP_BP", i32 262144) #4
  %_fetch1518 = load i64, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 3), align 8
  %or1520 = or i64 %_fetch1518, 1024
  store i64 %or1520, ptr getelementptr inbounds (%"var$45", ptr @"var$46", i64 0, i32 0, i32 3), align 8
  %func_result1524 = call i32 @for_dealloc_all_nocheck(ptr @"var$46", ptr nonnull %"c_phase_$BECP0", i32 262144) #4
  %"c_phase_$L_CAL.flags$_fetch" = load i64, ptr %"c_phase_$L_CAL.flags$", align 8
  %and1526 = and i64 %"c_phase_$L_CAL.flags$_fetch", 1
  %rel1528 = icmp eq i64 %and1526, 0
  br i1 %rel1528, label %bb544, label %bb543

bb543:                                            ; preds = %bb6
  %"c_phase_$L_CAL.addr_a0$_fetch15301821" = load ptr, ptr %"c_phase_$L_CAL", align 8
  %"c_phase_$L_CAL.flags$_fetch.tr" = trunc i64 %"c_phase_$L_CAL.flags$_fetch" to i32
  %24 = shl i32 %"c_phase_$L_CAL.flags$_fetch.tr", 1
  %int_zext1536 = and i32 %24, 4
  %25 = lshr i64 %"c_phase_$L_CAL.flags$_fetch", 3
  %26 = trunc i64 %25 to i32
  %int_zext1558 = and i32 %26, 256
  %27 = lshr i64 %"c_phase_$L_CAL.flags$_fetch", 15
  %28 = trunc i64 %27 to i32
  %int_zext1582 = and i32 %28, 31457280
  %or1548 = or i32 %int_zext1558, %int_zext1536
  %or1572 = or i32 %or1548, %int_zext1582
  %or1588 = or i32 %or1572, 262146
  %func_result1592 = call i32 @for_dealloc_allocatable(ptr %"c_phase_$L_CAL.addr_a0$_fetch15301821", i32 %or1588) #4
  store ptr null, ptr %"c_phase_$L_CAL", align 8
  %and1600 = and i64 %"c_phase_$L_CAL.flags$_fetch", -2050
  store i64 %and1600, ptr %"c_phase_$L_CAL.flags$", align 8
  br label %bb544

bb544:                                            ; preds = %bb543, %bb6
  %"c_phase_$Q_DK_SO.flags$_fetch" = load i64, ptr %"c_phase_$Q_DK_SO.flags$", align 8
  %and1602 = and i64 %"c_phase_$Q_DK_SO.flags$_fetch", 1
  %rel1604 = icmp eq i64 %and1602, 0
  br i1 %rel1604, label %bb552, label %bb547

bb547:                                            ; preds = %bb544
  %"c_phase_$Q_DK_SO.addr_a0$_fetch16061823" = load ptr, ptr %"c_phase_$Q_DK_SO", align 8
  %"c_phase_$Q_DK_SO.flags$_fetch.tr" = trunc i64 %"c_phase_$Q_DK_SO.flags$_fetch" to i32
  %29 = shl i32 %"c_phase_$Q_DK_SO.flags$_fetch.tr", 1
  %int_zext1616 = and i32 %29, 4
  %30 = lshr i64 %"c_phase_$Q_DK_SO.flags$_fetch", 3
  %31 = trunc i64 %30 to i32
  %int_zext1638 = and i32 %31, 256
  %32 = lshr i64 %"c_phase_$Q_DK_SO.flags$_fetch", 15
  %33 = trunc i64 %32 to i32
  %int_zext1662 = and i32 %33, 31457280
  %or1628 = or i32 %int_zext1638, %int_zext1616
  %or1652 = or i32 %or1628, %int_zext1662
  %or1668 = or i32 %or1652, 262146
  %func_result1672 = call i32 @for_dealloc_allocatable(ptr %"c_phase_$Q_DK_SO.addr_a0$_fetch16061823", i32 %or1668) #4
  store ptr null, ptr %"c_phase_$Q_DK_SO", align 8
  %and1680 = and i64 %"c_phase_$Q_DK_SO.flags$_fetch", -2050
  store i64 %and1680, ptr %"c_phase_$Q_DK_SO.flags$", align 8
  br label %bb552

bb552:                                            ; preds = %bb547, %bb544
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64(i8, i64, i64, ptr, i64) #1

declare i32 @for_dealloc_all_nocheck(ptr, ptr, i32) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_dealloc_allocatable(ptr, i32) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { argmemonly nounwind willreturn writeonly }
attributes #4 = { nounwind }

!omp_offload.info = !{}
