; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-lmm -hir-loop-reroll -print-before=hir-loop-reroll -print-after=hir-loop-reroll < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Verify reroll does not cause a transformation but ends quietly. From atg_CMPLRLLVM-20518.c.

; *** IR Dump Before HIR Loop Reroll ***
; CHECK:       Function: main

; CHECK:       BEGIN REGION { modified }
; CHECK:               %limm196 = (%i2)[0][2][1];
; CHECK:               %limm199 = (%cw)[0][1];
; CHECK:               %limm262 = (%i2)[0][5][4];
; CHECK:               %limm265 = (%cw)[0][4];
; CHECK:             + DO i1 = 0, 90, 1   <DO_LOOP>
; CHECK:             |   %add = i1 + 1  +  1;
; CHECK:             |   %11 = (%e)[0][i1 + 2];
; CHECK:             |   %limm = ((1 + (-1 * %11)) * %11);
; CHECK:             |   (%p4.0100)[0] = (%cw)[0][i1 + 1];
; CHECK:             |   %13 = (%cw)[0][i1 + 2];
; CHECK:             |   %add28 = %13  +  %11;
; CHECK:             |   %limm194 = %11 + %13;
; CHECK:             |   %14 = %limm;
; CHECK:             |   %15 = (%o1)[0][i1];
; CHECK:             |   (%e)[0][i1] = %11 + -1 * %14 + %15;
; CHECK:             |   %y.promoted = %14;
; CHECK:             |   %limm197 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 30;
; CHECK:             |   %limm263 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 30;
; CHECK:             |   %limm328 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 29;
; CHECK:             |   %limm330 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 29;
; CHECK:             |   %limm332 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 28;
; CHECK:             |   %limm334 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 28;
; CHECK:             |   %limm336 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 27;
; CHECK:             |   %limm338 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 27;
; CHECK:             |   %limm340 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 26;
; CHECK:             |   %limm342 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 26;
; CHECK:             |   %limm344 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 25;
; CHECK:             |   %limm346 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 25;
; CHECK:             |   %limm348 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 24;
; CHECK:             |   %limm350 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 24;
; CHECK:             |   %limm352 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 23;
; CHECK:             |   %limm354 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 23;
; CHECK:             |   %limm356 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 22;
; CHECK:             |   %limm358 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 22;
; CHECK:             |   %limm360 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 21;
; CHECK:             |   %limm362 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 21;
; CHECK:             |   %limm364 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 20;
; CHECK:             |   %limm366 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 20;
; CHECK:             |   %limm368 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 19;
; CHECK:             |   %limm370 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 19;
; CHECK:             |   %limm372 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 18;
; CHECK:             |   %limm374 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 18;
; CHECK:             |   %limm376 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 17;
; CHECK:             |   %limm378 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 17;
; CHECK:             |   %limm380 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 16;
; CHECK:             |   %limm382 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 16;
; CHECK:             |   %limm384 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 15;
; CHECK:             |   %limm386 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 15;
; CHECK:             |   %limm388 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 14;
; CHECK:             |   %limm390 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 14;
; CHECK:             |   %limm392 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 13;
; CHECK:             |   %limm394 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 13;
; CHECK:             |   %limm396 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 12;
; CHECK:             |   %limm398 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 12;
; CHECK:             |   %limm400 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 11;
; CHECK:             |   %limm402 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 11;
; CHECK:             |   %limm404 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 10;
; CHECK:             |   %limm406 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 10;
; CHECK:             |   %limm408 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 9;
; CHECK:             |   %limm410 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 9;
; CHECK:             |   %limm412 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 8;
; CHECK:             |   %limm414 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 8;
; CHECK:             |   %limm416 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 7;
; CHECK:             |   %limm418 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 7;
; CHECK:             |   %limm420 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 6;
; CHECK:             |   %limm422 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 6;
; CHECK:             |   %limm424 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 5;
; CHECK:             |   %limm426 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 5;
; CHECK:             |   %limm428 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 4;
; CHECK:             |   %limm430 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 4;
; CHECK:             |   %limm432 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 3;
; CHECK:             |   %limm434 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 3;
; CHECK:             |   %limm436 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 2;
; CHECK:             |   %limm438 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 2;
; CHECK:             |   %limm440 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 1;
; CHECK:             |   %limm442 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 1;
; CHECK:             |   %limm444 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21;
; CHECK:             |   %limm446 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21;
; CHECK:             |   %limm = %y.promoted;
; CHECK:             |   %p4.0100 = &((%y)[0]);
; CHECK:             + END LOOP
; CHECK:             (%i2)[0][3][4] = %limm446;
; CHECK:             (%i2)[0][3][1] = %limm444;
; CHECK:             (%i2)[0][4][4] = %limm442;
; CHECK:             (%i2)[0][4][1] = %limm440;
; CHECK:             (%i2)[0][5][4] = %limm438;
; CHECK:             (%i2)[0][5][1] = %limm436;
; CHECK:             (%i2)[0][6][4] = %limm434;
; CHECK:             (%i2)[0][6][1] = %limm432;
; CHECK:             (%i2)[0][7][4] = %limm430;
; CHECK:             (%i2)[0][7][1] = %limm428;
; CHECK:             (%i2)[0][8][4] = %limm426;
; CHECK:             (%i2)[0][8][1] = %limm424;
; CHECK:             (%i2)[0][9][4] = %limm422;
; CHECK:             (%i2)[0][9][1] = %limm420;
; CHECK:             (%i2)[0][10][4] = %limm418;
; CHECK:             (%i2)[0][10][1] = %limm416;
; CHECK:             (%i2)[0][11][4] = %limm414;
; CHECK:             (%i2)[0][11][1] = %limm412;
; CHECK:             (%i2)[0][12][4] = %limm410;
; CHECK:             (%i2)[0][12][1] = %limm408;
; CHECK:             (%i2)[0][13][4] = %limm406;
; CHECK:             (%i2)[0][13][1] = %limm404;
; CHECK:             (%i2)[0][14][4] = %limm402;
; CHECK:             (%i2)[0][14][1] = %limm400;
; CHECK:             (%i2)[0][15][4] = %limm398;
; CHECK:             (%i2)[0][15][1] = %limm396;
; CHECK:             (%i2)[0][16][4] = %limm394;
; CHECK:             (%i2)[0][16][1] = %limm392;
; CHECK:             (%i2)[0][17][4] = %limm390;
; CHECK:             (%i2)[0][17][1] = %limm388;
; CHECK:             (%i2)[0][18][4] = %limm386;
; CHECK:             (%i2)[0][18][1] = %limm384;
; CHECK:             (%i2)[0][19][4] = %limm382;
; CHECK:             (%i2)[0][19][1] = %limm380;
; CHECK:             (%i2)[0][20][4] = %limm378;
; CHECK:             (%i2)[0][20][1] = %limm376;
; CHECK:             (%i2)[0][21][4] = %limm374;
; CHECK:             (%i2)[0][21][1] = %limm372;
; CHECK:             (%i2)[0][22][4] = %limm370;
; CHECK:             (%i2)[0][22][1] = %limm368;
; CHECK:             (%i2)[0][23][4] = %limm366;
; CHECK:             (%i2)[0][23][1] = %limm364;
; CHECK:             (%i2)[0][24][4] = %limm362;
; CHECK:             (%i2)[0][24][1] = %limm360;
; CHECK:             (%i2)[0][25][4] = %limm358;
; CHECK:             (%i2)[0][25][1] = %limm356;
; CHECK:             (%i2)[0][26][4] = %limm354;
; CHECK:             (%i2)[0][26][1] = %limm352;
; CHECK:             (%i2)[0][27][4] = %limm350;
; CHECK:             (%i2)[0][27][1] = %limm348;
; CHECK:             (%i2)[0][28][4] = %limm346;
; CHECK:             (%i2)[0][28][1] = %limm344;
; CHECK:             (%i2)[0][29][4] = %limm342;
; CHECK:             (%i2)[0][29][1] = %limm340;
; CHECK:             (%i2)[0][30][4] = %limm338;
; CHECK:             (%i2)[0][30][1] = %limm336;
; CHECK:             (%i2)[0][31][4] = %limm334;
; CHECK:             (%i2)[0][31][1] = %limm332;
; CHECK:             (%i2)[0][32][4] = %limm330;
; CHECK:             (%i2)[0][32][1] = %limm328;
; CHECK:             (%cw)[0][4] = %limm265;
; CHECK:             (%i2)[0][33][4] = %limm263;
; CHECK:             (%cw)[0][1] = %limm199;
; CHECK:             (%i2)[0][33][1] = %limm197;
; CHECK:             (%o)[0] = %limm194;
; CHECK:             (%y)[0] = %limm;
; CHECK:       END REGION

; Reroll does not cause a transformation but ends quietly.

; *** IR Dump After HIR Loop Reroll ***
; CHECK:Function: main

; CHECK:        BEGIN REGION { modified }
; CHECK:             %limm196 = (%i2)[0][2][1];
; CHECK:             %limm199 = (%cw)[0][1];
; CHECK:             %limm262 = (%i2)[0][5][4];
; CHECK:             %limm265 = (%cw)[0][4];
; CHECK:              + DO i1 = 0, 90, 1   <DO_LOOP>
; CHECK:               |   %add = i1 + 1  +  1;
; CHECK:               |   %11 = (%e)[0][i1 + 2];
; CHECK:             |   %limm = ((1 + (-1 * %11)) * %11);
; CHECK:              |   (%p4.0100)[0] = (%cw)[0][i1 + 1];
; CHECK:              |   %13 = (%cw)[0][i1 + 2];
; CHECK:              |   %add28 = %13  +  %11;
; CHECK:             |   %limm194 = %11 + %13;
; CHECK:             |   %14 = %limm;
; CHECK:              |   %15 = (%o1)[0][i1];
; CHECK:              |   (%e)[0][i1] = %11 + -1 * %14 + %15;
; CHECK:              |   %y.promoted = %14;
; CHECK:             |   %limm197 = %limm196;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 30;
; CHECK:             |   %limm263 = %limm262;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 30;
; CHECK:             |   %limm328 = %limm196;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 29;
; CHECK:             |   %limm330 = %limm262;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 29;
; CHECK:             |   %limm332 = %limm196;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 28;
; CHECK:             |   %limm334 = %limm262;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 28;
; CHECK:             |   %limm336 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 27;
; CHECK:             |   %limm338 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 27;
; CHECK:             |   %limm340 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 26;
; CHECK:             |   %limm342 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 26;
; CHECK:             |   %limm344 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 25;
; CHECK:             |   %limm346 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 25;
; CHECK:             |   %limm348 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 24;
; CHECK:             |   %limm350 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 24;
; CHECK:             |   %limm352 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 23;
; CHECK:             |   %limm354 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 23;
; CHECK:             |   %limm356 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 22;
; CHECK:             |   %limm358 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 22;
; CHECK:             |   %limm360 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 21;
; CHECK:             |   %limm362 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 21;
; CHECK:             |   %limm364 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 20;
; CHECK:             |   %limm366 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 20;
; CHECK:             |   %limm368 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 19;
; CHECK:             |   %limm370 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 19;
; CHECK:             |   %limm372 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 18;
; CHECK:             |   %limm374 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 18;
; CHECK:             |   %limm376 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 17;
; CHECK:             |   %limm378 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 17;
; CHECK:             |   %limm380 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 16;
; CHECK:             |   %limm382 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 16;
; CHECK:             |   %limm384 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 15;
; CHECK:             |   %limm386 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 15;
; CHECK:             |   %limm388 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 14;
; CHECK:             |   %limm390 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 14;
; CHECK:             |   %limm392 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 13;
; CHECK:             |   %limm394 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 13;
; CHECK:             |   %limm396 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 12;
; CHECK:             |   %limm398 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 12;
; CHECK:             |   %limm400 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 11;
; CHECK:             |   %limm402 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 11;
; CHECK:             |   %limm404 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 10;
; CHECK:             |   %limm406 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 10;
; CHECK:             |   %limm408 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 9;
; CHECK:             |   %limm410 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 9;
; CHECK:             |   %limm412 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 8;
; CHECK:             |   %limm414 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 8;
; CHECK:             |   %limm416 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 7;
; CHECK:             |   %limm418 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 7;
; CHECK:             |   %limm420 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 6;
; CHECK:             |   %limm422 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 6;
; CHECK:             |   %limm424 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 5;
; CHECK:             |   %limm426 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 5;
; CHECK:             |   %limm428 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 4;
; CHECK:             |   %limm430 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 4;
; CHECK:             |   %limm432 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 3;
; CHECK:             |   %limm434 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 3;
; CHECK:             |   %limm436 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 2;
; CHECK:             |   %limm438 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 2;
; CHECK:             |   %limm440 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21 + 1;
; CHECK:             |   %limm442 = %limm262;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21 + 1;
; CHECK:             |   %limm444 = %limm196;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm199;
; CHECK:             |   %limm199 = %21;
; CHECK:             |   %limm446 = %limm262;
; CHECK:              |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm265;
; CHECK:             |   %limm265 = %21;
; CHECK:             |   %limm = %y.promoted;
; CHECK:              |   %p4.0100 = &((%y)[0]);
; CHECK:              + END LOOP
; CHECK:             (%i2)[0][3][4] = %limm446;
; CHECK:             (%i2)[0][3][1] = %limm444;
; CHECK:             (%i2)[0][4][4] = %limm442;
; CHECK:             (%i2)[0][4][1] = %limm440;
; CHECK:             (%i2)[0][5][4] = %limm438;
; CHECK:             (%i2)[0][5][1] = %limm436;
; CHECK:             (%i2)[0][6][4] = %limm434;
; CHECK:             (%i2)[0][6][1] = %limm432;
; CHECK:             (%i2)[0][7][4] = %limm430;
; CHECK:             (%i2)[0][7][1] = %limm428;
; CHECK:             (%i2)[0][8][4] = %limm426;
; CHECK:             (%i2)[0][8][1] = %limm424;
; CHECK:             (%i2)[0][9][4] = %limm422;
; CHECK:             (%i2)[0][9][1] = %limm420;
; CHECK:             (%i2)[0][10][4] = %limm418;
; CHECK:             (%i2)[0][10][1] = %limm416;
; CHECK:             (%i2)[0][11][4] = %limm414;
; CHECK:             (%i2)[0][11][1] = %limm412;
; CHECK:             (%i2)[0][12][4] = %limm410;
; CHECK:             (%i2)[0][12][1] = %limm408;
; CHECK:             (%i2)[0][13][4] = %limm406;
; CHECK:             (%i2)[0][13][1] = %limm404;
; CHECK:             (%i2)[0][14][4] = %limm402;
; CHECK:             (%i2)[0][14][1] = %limm400;
; CHECK:             (%i2)[0][15][4] = %limm398;
; CHECK:             (%i2)[0][15][1] = %limm396;
; CHECK:             (%i2)[0][16][4] = %limm394;
; CHECK:             (%i2)[0][16][1] = %limm392;
; CHECK:             (%i2)[0][17][4] = %limm390;
; CHECK:             (%i2)[0][17][1] = %limm388;
; CHECK:             (%i2)[0][18][4] = %limm386;
; CHECK:             (%i2)[0][18][1] = %limm384;
; CHECK:             (%i2)[0][19][4] = %limm382;
; CHECK:             (%i2)[0][19][1] = %limm380;
; CHECK:             (%i2)[0][20][4] = %limm378;
; CHECK:             (%i2)[0][20][1] = %limm376;
; CHECK:             (%i2)[0][21][4] = %limm374;
; CHECK:             (%i2)[0][21][1] = %limm372;
; CHECK:             (%i2)[0][22][4] = %limm370;
; CHECK:             (%i2)[0][22][1] = %limm368;
; CHECK:             (%i2)[0][23][4] = %limm366;
; CHECK:             (%i2)[0][23][1] = %limm364;
; CHECK:             (%i2)[0][24][4] = %limm362;
; CHECK:             (%i2)[0][24][1] = %limm360;
; CHECK:             (%i2)[0][25][4] = %limm358;
; CHECK:             (%i2)[0][25][1] = %limm356;
; CHECK:             (%i2)[0][26][4] = %limm354;
; CHECK:             (%i2)[0][26][1] = %limm352;
; CHECK:             (%i2)[0][27][4] = %limm350;
; CHECK:             (%i2)[0][27][1] = %limm348;
; CHECK:             (%i2)[0][28][4] = %limm346;
; CHECK:             (%i2)[0][28][1] = %limm344;
; CHECK:             (%i2)[0][29][4] = %limm342;
; CHECK:             (%i2)[0][29][1] = %limm340;
; CHECK:             (%i2)[0][30][4] = %limm338;
; CHECK:             (%i2)[0][30][1] = %limm336;
; CHECK:             (%i2)[0][31][4] = %limm334;
; CHECK:             (%i2)[0][31][1] = %limm332;
; CHECK:             (%i2)[0][32][4] = %limm330;
; CHECK:             (%i2)[0][32][1] = %limm328;
; CHECK:             (%cw)[0][4] = %limm265;
; CHECK:             (%i2)[0][33][4] = %limm263;
; CHECK:             (%cw)[0][1] = %limm199;
; CHECK:             (%i2)[0][33][1] = %limm197;
; CHECK:             (%o)[0] = %limm194;
; CHECK:             (%y)[0] = %limm;
; CHECK:         END REGION

;Module Before HIR
; ModuleID = 'cycle.c'
source_filename = "cycle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"%u %u %u %u %u\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %id4 = alloca i32, align 4
  %y = alloca i32, align 4
  %o = alloca i32, align 4
  %j6 = alloca i32, align 4
  %jy3 = alloca i32, align 4
  %e = alloca [100 x i32], align 16
  %cw = alloca [100 x i32], align 16
  %o1 = alloca [100 x i32], align 16
  %i2 = alloca [100 x [100 x i32]], align 16
  %0 = bitcast i32* %id4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #5
  store i32 44, i32* %id4, align 4, !tbaa !2
  %1 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #5
  store i32 92, i32* %y, align 4, !tbaa !2
  %2 = bitcast i32* %o to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #5
  store i32 45, i32* %o, align 4, !tbaa !2
  %3 = bitcast i32* %j6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #5
  store i32 41, i32* %j6, align 4, !tbaa !2
  %4 = bitcast i32* %jy3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #5
  store i32 85, i32* %jy3, align 4, !tbaa !2
  %5 = bitcast [100 x i32]* %e to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %5, i8 0, i64 400, i1 false)
  %6 = bitcast [100 x i32]* %cw to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %6, i8 0, i64 400, i1 false)
  %7 = bitcast [100 x i32]* %o1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %7) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %7, i8 0, i64 400, i1 false)
  %8 = bitcast [100 x [100 x i32]]* %i2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 40000, i8* nonnull %8) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(40000) %8, i8 0, i64 40000, i1 false)
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 0
  %call = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay, i32 100, i32 68) #5
  %arraydecay1 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 0
  %call2 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay1, i32 100, i32 1) #5
  %arraydecay3 = getelementptr inbounds [100 x i32], [100 x i32]* %o1, i64 0, i64 0
  %call4 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay3, i32 100, i32 86) #5
  %9 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 0, i64 0
  %call6 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %9, i32 10000, i32 44) #5
  %call7 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32* nonnull %id4, i32* nonnull %y, i32* nonnull %o, i32* nonnull %j6, i32* nonnull %jy3)
  store i32 1, i32* %id4, align 4, !tbaa !2
  br label %for.body

for.cond.loopexit:                                ; preds = %for.inc70
  %add64.lcssa.lcssa = phi i32 [ %add64.lcssa, %for.inc70 ]
  store i32 %add64.lcssa.lcssa, i32* %y, align 4, !tbaa !2
  %cmp = icmp ult i32 %10, 91
  br i1 %cmp, label %for.body, label %for.end73

for.body:                                         ; preds = %entry, %for.cond.loopexit
  %p4.0100 = phi i32* [ %o, %entry ], [ %y, %for.cond.loopexit ]
  %10 = phi i32 [ 1, %entry ], [ %add, %for.cond.loopexit ]
  %add = add nuw nsw i32 %10, 1
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 %idxprom
  %11 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %sub = add nsw i32 %10, -1
  %idxprom8 = zext i32 %sub to i64
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 %idxprom8
  %mul = mul i32 %11, %11
  %sub19 = sub i32 %11, %mul
  store i32 %sub19, i32* %y, align 4, !tbaa !2
  %idxprom20 = zext i32 %10 to i64
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %idxprom20, !intel-tbaa !6
  %12 = load i32, i32* %arrayidx21, align 4, !tbaa !6
  store i32 %12, i32* %p4.0100, align 4, !tbaa !2
  %arrayidx27 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %idxprom, !intel-tbaa !6
  %13 = load i32, i32* %arrayidx27, align 4, !tbaa !6
  %add28 = add i32 %13, %11
  store i32 %add28, i32* %o, align 4, !tbaa !2
  %14 = load i32, i32* %y, align 4, !tbaa !2
  %arrayidx34 = getelementptr inbounds [100 x i32], [100 x i32]* %o1, i64 0, i64 %idxprom8, !intel-tbaa !6
  %15 = load i32, i32* %arrayidx34, align 4, !tbaa !6
  %sub35.neg = sub i32 %11, %14
  %sub36 = add i32 %sub35.neg, %15
  store i32 %sub36, i32* %arrayidx9, align 4, !tbaa !6
  br label %for.cond43.preheader

for.cond43.preheader:                             ; preds = %for.body, %for.inc70
  %y.promoted = phi i32 [ %14, %for.body ], [ %add64.lcssa, %for.inc70 ]
  %indvars.iv103 = phi i64 [ 32, %for.body ], [ %indvars.iv.next104, %for.inc70 ]
  %16 = add nuw nsw i64 %indvars.iv103, 1
  %17 = trunc i64 %indvars.iv103 to i32
  %18 = add i32 %17, -2
  br label %for.body45

for.body45:                                       ; preds = %for.cond43.preheader, %for.body45
  %indvars.iv = phi i64 [ 1, %for.cond43.preheader ], [ %indvars.iv.next, %for.body45 ]
  %add64101 = phi i32 [ %y.promoted, %for.cond43.preheader ], [ %add64, %for.body45 ]
  %19 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 %19, i64 %indvars.iv, !intel-tbaa !8
  %20 = load i32, i32* %arrayidx50, align 4, !tbaa !8
  %arrayidx55 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 %16, i64 %indvars.iv, !intel-tbaa !8
  store i32 %20, i32* %arrayidx55, align 4, !tbaa !8
  %mul63 = mul i32 %add64101, %add28
  %add64 = add i32 %mul63, %add64101
  %arrayidx67 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %21 = load i32, i32* %arrayidx67, align 4, !tbaa !6
  %add68 = add i32 %18, %21
  store i32 %add68, i32* %arrayidx67, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp44 = icmp ult i64 %indvars.iv, 3
  br i1 %cmp44, label %for.body45, label %for.inc70

for.inc70:                                        ; preds = %for.body45
  %add64.lcssa = phi i32 [ %add64, %for.body45 ]
  %indvars.iv.next104 = add nsw i64 %indvars.iv103, -1
  %cmp41 = icmp ugt i64 %indvars.iv.next104, 1
  br i1 %cmp41, label %for.cond43.preheader, label %for.cond.loopexit

for.end73:                                        ; preds = %for.cond.loopexit
  %add.lcssa = phi i32 [ %add, %for.cond.loopexit ]
  %add28.lcssa = phi i32 [ %add28, %for.cond.loopexit ]
  %add64.lcssa.lcssa.lcssa = phi i32 [ %add64.lcssa.lcssa, %for.cond.loopexit ]
  store i32 1, i32* %j6, align 4, !tbaa !2
  store i32 7, i32* %jy3, align 4, !tbaa !2
  store i32 %add.lcssa, i32* %id4, align 4, !tbaa !2
  %call77 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay, i32 100) #5
  %call79 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay1, i32 100) #5
  %call82 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay3, i32 100) #5
  %call85 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %9, i32 10000) #5
  %add80 = add i32 %add28.lcssa, %add64.lcssa.lcssa.lcssa
  %sub83 = add i32 %add80, -1
  %add86 = add i32 %sub83, %call77
  %add74 = add i32 %add86, %call79
  %sub75 = sub i32 %add74, %call82
  %add87 = add i32 %sub75, %call85
  %call88 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0), i32 %add87)
  call void @llvm.lifetime.end.p0i8(i64 40000, i8* nonnull %8) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %7) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #5
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

declare dso_local i32 @init(...) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare dso_local i32 @__isoc99_scanf(i8* nocapture readonly, ...) local_unnamed_addr #4

declare dso_local i32 @checkSum(...) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { argmemonly nounwind willreturn writeonly }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_j", !3, i64 0}
!8 = !{!9, !3, i64 0}
!9 = !{!"array@_ZTSA100_A100_j", !7, i64 0}
