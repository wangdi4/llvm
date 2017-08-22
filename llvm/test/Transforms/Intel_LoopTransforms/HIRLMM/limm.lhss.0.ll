; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-post-vec-complete-unroll -hir-complete-unroll-loopnest-trip-threshold=50 -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
; (This test is based on Interchange/matmul-coremark.ll)
;
;[LLIMM Analysis]
;MemRefCollection, entries: 25
;  (@px)[0][25 * i1] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 1] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 2] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 3] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 4] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 5] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 6] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 7] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 8] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 9] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 10] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 11] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 12] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 13] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 14] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 15] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 16] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 17] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 18] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 19] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 20] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 21] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 22] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 23] {  R  W  } 1W : 1R  legal 
;  (@px)[0][25 * i1 + 24] {  R  W  } 1W : 1R  legal 
;
; LIMM's Opportunity:
; - LILH:   (0)
; - LISS:   (0)
; - LILHSS:(25)
;
;
; ORIGINAL LOOP:
;
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 40>
; CHECK:           |   + DO i2 = 0, %loop + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
; CHECK:           |   |   + DO i3 = 0, 24, 1   <DO_LOOP>
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 1]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 1]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 1] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 2]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 2]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 2] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 3]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 3]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 3] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 4]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 4]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 4] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 5]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 5]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 5] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 6]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 6]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 6] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 7]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 7]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 7] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 8]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 8]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 8] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 9]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 9]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 9] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 10]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 10]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 10] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 11]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 11]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 11] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 12]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 12]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 12] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 13]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 13]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 13] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 14]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 14]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 14] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 15]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 15]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 15] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 16]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 16]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 16] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 17]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 17]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 17] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 18]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 18]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 18] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 19]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 19]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 19] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 20]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 20]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 20] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 21]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 21]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 21] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 22]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 22]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 22] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 23]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 23]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 23] = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 24]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = (@px)[0][25 * i1 + 24]  +  %mul20;
; CHECK:           |   |   |   (@px)[0][25 * i1 + 24] = %add21;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:   END REGION
;
; *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
; CHECK: matmul
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 40>
; CHECK:           |   + DO i2 = 0, %loop + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
; CHECK:           |   |      %limm = (@px)[0][25 * i1];
; CHECK:           |   |      %limm4 = (@px)[0][25 * i1 + 1];
; CHECK:           |   |      %limm6 = (@px)[0][25 * i1 + 2];
; CHECK:           |   |      %limm8 = (@px)[0][25 * i1 + 3];
; CHECK:           |   |      %limm10 = (@px)[0][25 * i1 + 4];
; CHECK:           |   |      %limm12 = (@px)[0][25 * i1 + 5];
; CHECK:           |   |      %limm14 = (@px)[0][25 * i1 + 6];
; CHECK:           |   |      %limm16 = (@px)[0][25 * i1 + 7];
; CHECK:           |   |      %limm18 = (@px)[0][25 * i1 + 8];
; CHECK:           |   |      %limm20 = (@px)[0][25 * i1 + 9];
; CHECK:           |   |      %limm22 = (@px)[0][25 * i1 + 10];
; CHECK:           |   |      %limm24 = (@px)[0][25 * i1 + 11];
; CHECK:           |   |      %limm26 = (@px)[0][25 * i1 + 12];
; CHECK:           |   |      %limm28 = (@px)[0][25 * i1 + 13];
; CHECK:           |   |      %limm30 = (@px)[0][25 * i1 + 14];
; CHECK:           |   |      %limm32 = (@px)[0][25 * i1 + 15];
; CHECK:           |   |      %limm34 = (@px)[0][25 * i1 + 16];
; CHECK:           |   |      %limm36 = (@px)[0][25 * i1 + 17];
; CHECK:           |   |      %limm38 = (@px)[0][25 * i1 + 18];
; CHECK:           |   |      %limm40 = (@px)[0][25 * i1 + 19];
; CHECK:           |   |      %limm42 = (@px)[0][25 * i1 + 20];
; CHECK:           |   |      %limm44 = (@px)[0][25 * i1 + 21];
; CHECK:           |   |      %limm46 = (@px)[0][25 * i1 + 22];
; CHECK:           |   |      %limm48 = (@px)[0][25 * i1 + 23];
; CHECK:           |   |      %limm50 = (@px)[0][25 * i1 + 24];
; CHECK:           |   |   + DO i3 = 0, 24, 1   <DO_LOOP>
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm  +  %mul20;
; CHECK:           |   |   |   %limm = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 1]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm4  +  %mul20;
; CHECK:           |   |   |   %limm4 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 2]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm6  +  %mul20;
; CHECK:           |   |   |   %limm6 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 3]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm8  +  %mul20;
; CHECK:           |   |   |   %limm8 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 4]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm10  +  %mul20;
; CHECK:           |   |   |   %limm10 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 5]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm12  +  %mul20;
; CHECK:           |   |   |   %limm12 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 6]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm14  +  %mul20;
; CHECK:           |   |   |   %limm14 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 7]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm16  +  %mul20;
; CHECK:           |   |   |   %limm16 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 8]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm18  +  %mul20;
; CHECK:           |   |   |   %limm18 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 9]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm20  +  %mul20;
; CHECK:           |   |   |   %limm20 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 10]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm22  +  %mul20;
; CHECK:           |   |   |   %limm22 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 11]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm24  +  %mul20;
; CHECK:           |   |   |   %limm24 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 12]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm26  +  %mul20;
; CHECK:           |   |   |   %limm26 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 13]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm28  +  %mul20;
; CHECK:           |   |   |   %limm28 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 14]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm30  +  %mul20;
; CHECK:           |   |   |   %limm30 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 15]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm32  +  %mul20;
; CHECK:           |   |   |   %limm32 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 16]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm34  +  %mul20;
; CHECK:           |   |   |   %limm34 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 17]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm36  +  %mul20;
; CHECK:           |   |   |   %limm36 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 18]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm38  +  %mul20;
; CHECK:           |   |   |   %limm38 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 19]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm40  +  %mul20;
; CHECK:           |   |   |   %limm40 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 20]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm42  +  %mul20;
; CHECK:           |   |   |   %limm42 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 21]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm44  +  %mul20;
; CHECK:           |   |   |   %limm44 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 22]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm46  +  %mul20;
; CHECK:           |   |   |   %limm46 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 23]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm48  +  %mul20;
; CHECK:           |   |   |   %limm48 = %add21;
; CHECK:           |   |   |   %mul20 = (@vy)[0][i1 + sext.i32.i64(%n) * i3 + 24]  *  (@cx)[0][25 * i1 + i2 + i3 + 1];
; CHECK:           |   |   |   %add21 = %limm50  +  %mul20;
; CHECK:           |   |   |   %limm50 = %add21;
; CHECK:           |   |   + END LOOP
; CHECK:           |   |      (@px)[0][25 * i1 + 24] = %limm50;
; CHECK:           |   |      (@px)[0][25 * i1 + 23] = %limm48;
; CHECK:           |   |      (@px)[0][25 * i1 + 22] = %limm46;
; CHECK:           |   |      (@px)[0][25 * i1 + 21] = %limm44;
; CHECK:           |   |      (@px)[0][25 * i1 + 20] = %limm42;
; CHECK:           |   |      (@px)[0][25 * i1 + 19] = %limm40;
; CHECK:           |   |      (@px)[0][25 * i1 + 18] = %limm38;
; CHECK:           |   |      (@px)[0][25 * i1 + 17] = %limm36;
; CHECK:           |   |      (@px)[0][25 * i1 + 16] = %limm34;
; CHECK:           |   |      (@px)[0][25 * i1 + 15] = %limm32;
; CHECK:           |   |      (@px)[0][25 * i1 + 14] = %limm30;
; CHECK:           |   |      (@px)[0][25 * i1 + 13] = %limm28;
; CHECK:           |   |      (@px)[0][25 * i1 + 12] = %limm26;
; CHECK:           |   |      (@px)[0][25 * i1 + 11] = %limm24;
; CHECK:           |   |      (@px)[0][25 * i1 + 10] = %limm22;
; CHECK:           |   |      (@px)[0][25 * i1 + 9] = %limm20;
; CHECK:           |   |      (@px)[0][25 * i1 + 8] = %limm18;
; CHECK:           |   |      (@px)[0][25 * i1 + 7] = %limm16;
; CHECK:           |   |      (@px)[0][25 * i1 + 6] = %limm14;
; CHECK:           |   |      (@px)[0][25 * i1 + 5] = %limm12;
; CHECK:           |   |      (@px)[0][25 * i1 + 4] = %limm10;
; CHECK:           |   |      (@px)[0][25 * i1 + 3] = %limm8;
; CHECK:           |   |      (@px)[0][25 * i1 + 2] = %limm6;
; CHECK:           |   |      (@px)[0][25 * i1 + 1] = %limm4;
; CHECK:           |   |      (@px)[0][25 * i1] = %limm;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:  END REGION
;
;
; ModuleID = 'matmul-coremark.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@px = common global [1000 x float] zeroinitializer, align 16
@vy = common global [1000 x float] zeroinitializer, align 16
@cx = common global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @matmul(i32 %loop, i32 %n) #0 {
entry:
  %cmp59 = icmp slt i32 %loop, 1
  br i1 %cmp59, label %for.end34, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp854 = icmp sgt i32 %n, 0
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc32, %for.cond1.preheader.lr.ph
  %l.060 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %inc33, %for.inc32 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc29, %for.cond1.preheader
  %indvars.iv68 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next69, %for.inc29 ]
  %1 = mul nsw i64 %indvars.iv68, %0
  %2 = trunc i64 %indvars.iv68 to i32
  %add16 = add nuw i32 %2, %l.060
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc26, %for.cond4.preheader
  %indvars.iv64 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next65, %for.inc26 ]
  br i1 %cmp854, label %for.body9.lr.ph, label %for.inc26

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %3 = add nsw i64 %indvars.iv64, %1
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %4 = mul nuw nsw i64 %indvars.iv, 25
  %5 = add nuw nsw i64 %4, %indvars.iv64
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @px, i64 0, i64 %5
  %6 = load float, float* %arrayidx, align 4, !tbaa !1
  %7 = add nsw i64 %3, %indvars.iv
  %arrayidx14 = getelementptr inbounds [1000 x float], [1000 x float]* @vy, i64 0, i64 %7
  %8 = load float, float* %arrayidx14, align 4, !tbaa !1
  %9 = trunc i64 %4 to i32
  %add17 = add i32 %add16, %9
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds [1000 x float], [1000 x float]* @cx, i64 0, i64 %idxprom18
  %10 = load float, float* %arrayidx19, align 4, !tbaa !1
  %mul20 = fmul float %8, %10
  %add21 = fadd float %6, %mul20
  store float %add21, float* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc26, label %for.body9

for.inc26:                                        ; preds = %for.body9, %for.cond7.preheader
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next65, 25
  br i1 %exitcond67, label %for.inc29, label %for.cond7.preheader

for.inc29:                                        ; preds = %for.inc26
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond71 = icmp eq i64 %indvars.iv.next69, 25
  br i1 %exitcond71, label %for.inc32, label %for.cond4.preheader

for.inc32:                                        ; preds = %for.inc29
  %inc33 = add nuw nsw i32 %l.060, 1
  %exitcond72 = icmp eq i32 %l.060, %loop
  br i1 %exitcond72, label %for.end34, label %for.cond1.preheader

for.end34:                                        ; preds = %for.inc32, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 8655)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
