; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,hir-lmm,hir-unroll-and-jam,hir-vec-dir-insert,hir-vplan-vec,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -hir-loop-blocking-blocksize=128 -disable-output < %s 2>&1 | FileCheck %s
;
; Scalar Replacement Support for vector types: limit to dist-0 within a group in a vectorized group
;
; [Source code]
; program main
; parameter (n=4096)
; double precision ,  dimension(n,n) :: a,b,c
; integer i,j,k
; do j=1,n
;   do k = 1,n
;     do i = 1,n
;       c(i,j) = c(i,j) + a(i,k) * b(k,j)
;     enddo
;   enddo
;  enddo
; end

;
;*** IR Dump Before HIR Scalar Replacement of Array  (hir-scalarrepl-array) ***
;
;<0>          BEGIN REGION { modified }
;<43>               + DO i1 = 0, 31, 1   <DO_LOOP>
;<40>               |   + DO i2 = 0, 31, 1   <DO_LOOP>
;<44>               |   |   + DO i3 = 0, 31, 1   <DO_LOOP>
;<52>               |   |   |   + DO i4 = 0, 31, 1   <DO_LOOP> <nounroll and jam>
;<68>               |   |   |   |   + DO i5 = 0, 31, 1   <DO_LOOP> <nounroll and jam>
;<70>               |   |   |   |   |   %temp5 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5];
;<71>               |   |   |   |   |   %temp6 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5];
;<72>               |   |   |   |   |   %temp7 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5];
;<73>               |   |   |   |   |   %temp8 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5];
;<74>               |   |   |   |   |   %temp9 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 1];
;<75>               |   |   |   |   |   %temp10 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 1];
;<76>               |   |   |   |   |   %temp11 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 1];
;<77>               |   |   |   |   |   %temp12 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 1];
;<78>               |   |   |   |   |   %temp13 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 2];
;<79>               |   |   |   |   |   %temp14 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 2];
;<80>               |   |   |   |   |   %temp15 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 2];
;<81>               |   |   |   |   |   %temp16 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 2];
;<56>               |   |   |   |   |   %temp = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 3];
;<57>               |   |   |   |   |   %temp3 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 3];
;<58>               |   |   |   |   |   %temp4 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 3];
;<9>                |   |   |   |   |   %"main_$B[][]_fetch.9" = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 3];
;<120>              |   |   |   |   |
;<120>              |   |   |   |   |   + DO i6 = 0, 127, 4   <DO_LOOP> <auto-vectorized> <novectorize>
;<121>              |   |   |   |   |   |   %.vec = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5][128 * i3 + i6];
;<122>              |   |   |   |   |   |   %.vec17 = %temp5  *  %.vec;
;<123>              |   |   |   |   |   |   %.vec18 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6];
;<124>              |   |   |   |   |   |   %.vec19 = %.vec17  +  %.vec18;
;<125>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6] = %.vec19;
;<126>              |   |   |   |   |   |   %.vec20 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5][128 * i3 + i6];
;<127>              |   |   |   |   |   |   %.vec21 = %temp6  *  %.vec20;
;<128>              |   |   |   |   |   |   %.vec22 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6];
;<129>              |   |   |   |   |   |   %.vec23 = %.vec21  +  %.vec22;
;<130>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6] = %.vec23;
;<131>              |   |   |   |   |   |   %.vec24 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5][128 * i3 + i6];
;<132>              |   |   |   |   |   |   %.vec25 = %temp7  *  %.vec24;
;<133>              |   |   |   |   |   |   %.vec26 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6];
;<134>              |   |   |   |   |   |   %.vec27 = %.vec25  +  %.vec26;
;<135>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6] = %.vec27;
;<136>              |   |   |   |   |   |   %.vec28 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5][128 * i3 + i6];
;<137>              |   |   |   |   |   |   %.vec29 = %temp8  *  %.vec28;
;<138>              |   |   |   |   |   |   %.vec30 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6];
;<139>              |   |   |   |   |   |   %.vec31 = %.vec29  +  %.vec30;
;<140>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6] = %.vec31;
;<141>              |   |   |   |   |   |   %.vec32 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 1][128 * i3 + i6];
;<142>              |   |   |   |   |   |   %.vec33 = %temp9  *  %.vec32;
;<143>              |   |   |   |   |   |   %.vec34 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6];
;<144>              |   |   |   |   |   |   %.vec35 = %.vec33  +  %.vec34;
;<145>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6] = %.vec35;
;<146>              |   |   |   |   |   |   %.vec36 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 1][128 * i3 + i6];
;<147>              |   |   |   |   |   |   %.vec37 = %temp10  *  %.vec36;
;<148>              |   |   |   |   |   |   %.vec38 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6];
;<149>              |   |   |   |   |   |   %.vec39 = %.vec37  +  %.vec38;
;<150>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6] = %.vec39;
;<151>              |   |   |   |   |   |   %.vec40 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 1][128 * i3 + i6];
;<152>              |   |   |   |   |   |   %.vec41 = %temp11  *  %.vec40;
;<153>              |   |   |   |   |   |   %.vec42 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6];
;<154>              |   |   |   |   |   |   %.vec43 = %.vec41  +  %.vec42;
;<155>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6] = %.vec43;
;<156>              |   |   |   |   |   |   %.vec44 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 1][128 * i3 + i6];
;<157>              |   |   |   |   |   |   %.vec45 = %temp12  *  %.vec44;
;<158>              |   |   |   |   |   |   %.vec46 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6];
;<159>              |   |   |   |   |   |   %.vec47 = %.vec45  +  %.vec46;
;<160>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6] = %.vec47;
;<161>              |   |   |   |   |   |   %.vec48 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 2][128 * i3 + i6];
;<162>              |   |   |   |   |   |   %.vec49 = %temp13  *  %.vec48;
;<163>              |   |   |   |   |   |   %.vec50 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6];
;<164>              |   |   |   |   |   |   %.vec51 = %.vec49  +  %.vec50;
;<165>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6] = %.vec51;
;<166>              |   |   |   |   |   |   %.vec52 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 2][128 * i3 + i6];
;<167>              |   |   |   |   |   |   %.vec53 = %temp14  *  %.vec52;
;<168>              |   |   |   |   |   |   %.vec54 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6];
;<169>              |   |   |   |   |   |   %.vec55 = %.vec53  +  %.vec54;
;<170>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6] = %.vec55;
;<171>              |   |   |   |   |   |   %.vec56 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 2][128 * i3 + i6];
;<172>              |   |   |   |   |   |   %.vec57 = %temp15  *  %.vec56;
;<173>              |   |   |   |   |   |   %.vec58 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6];
;<174>              |   |   |   |   |   |   %.vec59 = %.vec57  +  %.vec58;
;<175>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6] = %.vec59;
;<176>              |   |   |   |   |   |   %.vec60 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 2][128 * i3 + i6];
;<177>              |   |   |   |   |   |   %.vec61 = %temp16  *  %.vec60;
;<178>              |   |   |   |   |   |   %.vec62 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6];
;<179>              |   |   |   |   |   |   %.vec63 = %.vec61  +  %.vec62;
;<180>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6] = %.vec63;
;<181>              |   |   |   |   |   |   %.vec64 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 3][128 * i3 + i6];
;<182>              |   |   |   |   |   |   %.vec65 = %temp  *  %.vec64;
;<183>              |   |   |   |   |   |   %.vec66 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6];
;<184>              |   |   |   |   |   |   %.vec67 = %.vec65  +  %.vec66;
;<185>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4][128 * i3 + i6] = %.vec67;
;<186>              |   |   |   |   |   |   %.vec68 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 3][128 * i3 + i6];
;<187>              |   |   |   |   |   |   %.vec69 = %temp3  *  %.vec68;
;<188>              |   |   |   |   |   |   %.vec70 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6];
;<189>              |   |   |   |   |   |   %.vec71 = %.vec69  +  %.vec70;
;<190>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 1][128 * i3 + i6] = %.vec71;
;<191>              |   |   |   |   |   |   %.vec72 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 3][128 * i3 + i6];
;<192>              |   |   |   |   |   |   %.vec73 = %temp4  *  %.vec72;
;<193>              |   |   |   |   |   |   %.vec74 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6];
;<194>              |   |   |   |   |   |   %.vec75 = %.vec73  +  %.vec74;
;<195>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 2][128 * i3 + i6] = %.vec75;
;<196>              |   |   |   |   |   |   %.vec76 = (<4 x double>*)(@"main_$A")[128 * i2 + 4 * i5 + 3][128 * i3 + i6];
;<197>              |   |   |   |   |   |   %.vec77 = %"main_$B[][]_fetch.9"  *  %.vec76;
;<198>              |   |   |   |   |   |   %.vec78 = (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6];
;<199>              |   |   |   |   |   |   %.vec79 = %.vec77  +  %.vec78;
;<200>              |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[128 * i1 + 4 * i4 + 3][128 * i3 + i6] = %.vec79;
;<120>              |   |   |   |   |   + END LOOP
;<68>               |   |   |   |   + END LOOP
;<52>               |   |   |   + END LOOP
;<44>               |   |   + END LOOP
;<40>               |   + END LOOP
;<43>               + END LOOP
;<0>          END REGION


;*** IR Dump After HIR Scalar Replacement of Array  (hir-scalarrepl-array) ***


;<0>          BEGIN REGION { modified }
;<43>               + DO i1 = 0, 31, 1   <DO_LOOP>
;<40>               |   + DO i2 = 0, 31, 1   <DO_LOOP>
;<44>               |   |   + DO i3 = 0, 31, 1   <DO_LOOP>
;<52>               |   |   |   + DO i4 = 0, 31, 1   <DO_LOOP> <nounroll and jam>
;<68>               |   |   |   |   + DO i5 = 0, 31, 1   <DO_LOOP> <nounroll and jam>
;<70>               |   |   |   |   |   %temp5 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5];
;<71>               |   |   |   |   |   %temp6 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5];
;<72>               |   |   |   |   |   %temp7 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5];
;<73>               |   |   |   |   |   %temp8 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5];
;<74>               |   |   |   |   |   %temp9 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 1];
;<75>               |   |   |   |   |   %temp10 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 1];
;<76>               |   |   |   |   |   %temp11 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 1];
;<77>               |   |   |   |   |   %temp12 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 1];
;<78>               |   |   |   |   |   %temp13 = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 2];
;<79>               |   |   |   |   |   %temp14 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 2];
;<80>               |   |   |   |   |   %temp15 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 2];
;<81>               |   |   |   |   |   %temp16 = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 2];
;<56>               |   |   |   |   |   %temp = (@"main_$B")[128 * i1 + 4 * i4][128 * i2 + 4 * i5 + 3];
;<57>               |   |   |   |   |   %temp3 = (@"main_$B")[128 * i1 + 4 * i4 + 1][128 * i2 + 4 * i5 + 3];
;<58>               |   |   |   |   |   %temp4 = (@"main_$B")[128 * i1 + 4 * i4 + 2][128 * i2 + 4 * i5 + 3];
;<9>                |   |   |   |   |   %"main_$B[][]_fetch.9" = (@"main_$B")[128 * i1 + 4 * i4 + 3][128 * i2 + 4 * i5 + 3];
;<120>              |   |   |   |   |
;<120>              |   |   |   |   |   + DO i6 = 0, 127, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM0:[0-9]+]] = (<4 x double>*)(@"main_$A")[0][128 * i2 + 4 * i5][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec = %scalarepl.vec[[NUM0]];
; CHECK:            |   |   |   |   |   |   %.vec17 = %temp5  *  %.vec;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec = (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec18 = %scalarepl.vec;
; CHECK:            |   |   |   |   |   |   %.vec19 = %.vec17  +  %.vec18;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec = %.vec19;
; CHECK:            |   |   |   |   |   |   %.vec20 = %scalarepl.vec[[NUM0]];
; CHECK:            |   |   |   |   |   |   %.vec21 = %temp6  *  %.vec20;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM1:[0-9]+]] = (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 1][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec22 = %scalarepl.vec[[NUM1]];
; CHECK:            |   |   |   |   |   |   %.vec23 = %.vec21  +  %.vec22;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM1]] = %.vec23;
; CHECK:            |   |   |   |   |   |   %.vec24 = %scalarepl.vec[[NUM0]];
; CHECK:            |   |   |   |   |   |   %.vec25 = %temp7  *  %.vec24;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM2:[0-9]+]] = (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 2][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec26 = %scalarepl.vec[[NUM2]];
; CHECK:            |   |   |   |   |   |   %.vec27 = %.vec25  +  %.vec26;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM2]] = %.vec27;
; CHECK:            |   |   |   |   |   |   %.vec28 = %scalarepl.vec[[NUM0]];
; CHECK:            |   |   |   |   |   |   %.vec29 = %temp8  *  %.vec28;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM3:[0-9]+]] = (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 3][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec30 = %scalarepl.vec[[NUM3]];
; CHECK:            |   |   |   |   |   |   %.vec31 = %.vec29  +  %.vec30;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM3]] = %.vec31;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM4:[0-9]+]] = (<4 x double>*)(@"main_$A")[0][128 * i2 + 4 * i5 + 1][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec32 = %scalarepl.vec[[NUM4]];
; CHECK:            |   |   |   |   |   |   %.vec33 = %temp9  *  %.vec32;
; CHECK:            |   |   |   |   |   |   %.vec34 = %scalarepl.vec;
; CHECK:            |   |   |   |   |   |   %.vec35 = %.vec33  +  %.vec34;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec = %.vec35;
; CHECK:            |   |   |   |   |   |   %.vec36 = %scalarepl.vec[[NUM4]];
; CHECK:            |   |   |   |   |   |   %.vec37 = %temp10  *  %.vec36;
; CHECK:            |   |   |   |   |   |   %.vec38 = %scalarepl.vec[[NUM1]];
; CHECK:            |   |   |   |   |   |   %.vec39 = %.vec37  +  %.vec38;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM1]] = %.vec39;
; CHECK:            |   |   |   |   |   |   %.vec40 = %scalarepl.vec[[NUM4]];
; CHECK:            |   |   |   |   |   |   %.vec41 = %temp11  *  %.vec40;
; CHECK:            |   |   |   |   |   |   %.vec42 = %scalarepl.vec[[NUM2]];
; CHECK:            |   |   |   |   |   |   %.vec43 = %.vec41  +  %.vec42;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM2]] = %.vec43;
; CHECK:            |   |   |   |   |   |   %.vec44 = %scalarepl.vec[[NUM4]];
; CHECK:            |   |   |   |   |   |   %.vec45 = %temp12  *  %.vec44;
; CHECK:            |   |   |   |   |   |   %.vec46 = %scalarepl.vec[[NUM3]];
; CHECK:            |   |   |   |   |   |   %.vec47 = %.vec45  +  %.vec46;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM3]] = %.vec47;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM5:[0-9]+]] = (<4 x double>*)(@"main_$A")[0][128 * i2 + 4 * i5 + 2][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec48 = %scalarepl.vec[[NUM5]];
; CHECK:            |   |   |   |   |   |   %.vec49 = %temp13  *  %.vec48;
; CHECK:            |   |   |   |   |   |   %.vec50 = %scalarepl.vec;
; CHECK:            |   |   |   |   |   |   %.vec51 = %.vec49  +  %.vec50;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec = %.vec51;
; CHECK:            |   |   |   |   |   |   %.vec52 = %scalarepl.vec[[NUM5]];
; CHECK:            |   |   |   |   |   |   %.vec53 = %temp14  *  %.vec52;
; CHECK:            |   |   |   |   |   |   %.vec54 = %scalarepl.vec[[NUM1]];
; CHECK:            |   |   |   |   |   |   %.vec55 = %.vec53  +  %.vec54;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM1]] = %.vec55;
; CHECK:            |   |   |   |   |   |   %.vec56 = %scalarepl.vec[[NUM5]];
; CHECK:            |   |   |   |   |   |   %.vec57 = %temp15  *  %.vec56;
; CHECK:            |   |   |   |   |   |   %.vec58 = %scalarepl.vec[[NUM2]];
; CHECK:            |   |   |   |   |   |   %.vec59 = %.vec57  +  %.vec58;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM2]] = %.vec59;
; CHECK:            |   |   |   |   |   |   %.vec60 = %scalarepl.vec[[NUM5]];
; CHECK:            |   |   |   |   |   |   %.vec61 = %temp16  *  %.vec60;
; CHECK:            |   |   |   |   |   |   %.vec62 = %scalarepl.vec[[NUM3]];
; CHECK:            |   |   |   |   |   |   %.vec63 = %.vec61  +  %.vec62;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM3]] = %.vec63;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM6:[0-9]+]] = (<4 x double>*)(@"main_$A")[0][128 * i2 + 4 * i5 + 3][128 * i3 + i6];
; CHECK:            |   |   |   |   |   |   %.vec64 = %scalarepl.vec[[NUM6]];
; CHECK:            |   |   |   |   |   |   %.vec65 = %temp  *  %.vec64;
; CHECK:            |   |   |   |   |   |   %.vec66 = %scalarepl.vec;
; CHECK:            |   |   |   |   |   |   %.vec67 = %.vec65  +  %.vec66;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec = %.vec67;
; CHECK:            |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4][128 * i3 + i6] = %scalarepl.vec;
; CHECK:            |   |   |   |   |   |   %.vec68 = %scalarepl.vec[[NUM6]];
; CHECK:            |   |   |   |   |   |   %.vec69 = %temp3  *  %.vec68;
; CHECK:            |   |   |   |   |   |   %.vec70 = %scalarepl.vec[[NUM1]];
; CHECK:            |   |   |   |   |   |   %.vec71 = %.vec69  +  %.vec70;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM1]] = %.vec71;
; CHECK:            |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 1][128 * i3 + i6] = %scalarepl.vec[[NUM1]];
; CHECK:            |   |   |   |   |   |   %.vec72 = %scalarepl.vec[[NUM6]];
; CHECK:            |   |   |   |   |   |   %.vec73 = %temp4  *  %.vec72;
; CHECK:            |   |   |   |   |   |   %.vec74 = %scalarepl.vec[[NUM2]];
; CHECK:            |   |   |   |   |   |   %.vec75 = %.vec73  +  %.vec74;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM2]] = %.vec75;
; CHECK:            |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 2][128 * i3 + i6] = %scalarepl.vec[[NUM2]];
; CHECK:            |   |   |   |   |   |   %.vec76 = %scalarepl.vec[[NUM6]];
; CHECK:            |   |   |   |   |   |   %.vec77 = %"main_$B[][]_fetch.9"  *  %.vec76;
; CHECK:            |   |   |   |   |   |   %.vec78 = %scalarepl.vec[[NUM3]];
; CHECK:            |   |   |   |   |   |   %.vec79 = %.vec77  +  %.vec78;
; CHECK:            |   |   |   |   |   |   %scalarepl.vec[[NUM3]] = %.vec79;
; CHECK:            |   |   |   |   |   |   (<4 x double>*)(@"main_$C")[0][128 * i1 + 4 * i4 + 3][128 * i3 + i6] = %scalarepl.vec[[NUM3]];
;<120>              |   |   |   |   |   + END LOOP
;<68>               |   |   |   |   + END LOOP
;<52>               |   |   |   + END LOOP
;<44>               |   |   + END LOOP
;<40>               |   + END LOOP
;<43>               + END LOOP
;<0>          END REGION



; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
;Module Before HIR
; ModuleID = 'matmul_simple.f'
source_filename = "matmul_simple.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"main_$C" = internal unnamed_addr global [4096 x [4096 x double]] zeroinitializer, align 16
@"main_$B" = internal unnamed_addr constant [4096 x [4096 x double]] zeroinitializer, align 16
@"main_$A" = internal unnamed_addr constant [4096 x [4096 x double]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %func_result = tail call i32 @for_set_reentrancy(ptr nonnull @0) #3
  br label %bb1

bb1:                                              ; preds = %bb8, %alloca_0
  %indvars.iv24 = phi i64 [ %indvars.iv.next25, %bb8 ], [ 1, %alloca_0 ]
  %"main_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32768, ptr elementtype(double) @"main_$C", i64 %indvars.iv24)
  %"main_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32768, ptr elementtype(double) @"main_$B", i64 %indvars.iv24)
  br label %bb5

bb5:                                              ; preds = %bb12, %bb1
  %indvars.iv21 = phi i64 [ %indvars.iv.next22, %bb12 ], [ 1, %bb1 ]
  %"main_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32768, ptr elementtype(double) @"main_$A", i64 %indvars.iv21)
  %"main_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"main_$B[]", i64 %indvars.iv21)
  %"main_$B[][]_fetch.9" = load double, ptr %"main_$B[][]", align 1
  br label %bb9

bb9:                                              ; preds = %bb9, %bb5
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb9 ], [ 1, %bb5 ]
  %"main_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"main_$C[]", i64 %indvars.iv)
  %"main_$C[][]_fetch.3" = load double, ptr %"main_$C[][]", align 1
  %"main_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"main_$A[]", i64 %indvars.iv)
  %"main_$A[][]_fetch.6" = load double, ptr %"main_$A[][]", align 1
  %mul.4 = fmul fast double %"main_$B[][]_fetch.9", %"main_$A[][]_fetch.6"
  %add.1 = fadd fast double %mul.4, %"main_$C[][]_fetch.3"
  store double %add.1, ptr %"main_$C[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4097
  br i1 %exitcond.not, label %bb12, label %bb9

bb12:                                             ; preds = %bb9
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23.not = icmp eq i64 %indvars.iv.next22, 4097
  br i1 %exitcond23.not, label %bb8, label %bb5

bb8:                                              ; preds = %bb12
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26.not = icmp eq i64 %indvars.iv.next25, 4097
  br i1 %exitcond26.not, label %bb4, label %bb1

bb4:                                              ; preds = %bb8
  ret void
}

; Function Attrs: nofree
declare i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nounwind }

!omp_offload.info = !{}
