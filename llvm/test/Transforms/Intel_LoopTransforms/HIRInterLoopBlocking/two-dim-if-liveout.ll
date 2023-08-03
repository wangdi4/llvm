; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -hir-inter-loop-blocking-force-test -hir-create-function-level-region -hir-details < %s 2>&1 | FileCheck %s

; Verify that  ByStripLoops have right liveOut info.

; Before transformation
; <0>          BEGIN REGION { }
; <2>                if (%"sub1_$K_fetch" < %"sub1_$NTIMES_fetch")
; <2>                {
; <69>                  + DO i1 = 0, 2, 1   <DO_LOOP> <nounroll>
; <70>                  |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <16>                  |   |   %add13 = (%"sub1_$B")[i1][i2]  +  1.000000e+00;
; <18>                  |   |   (%"sub1_$A")[i1][i2] = %add13;
; <70>                  |   + END LOOP
; <69>                  + END LOOP
; <69>
; <33>                  %add68.lcssa118 = 0.000000e+00;
; <71>
; <71>                  + DO i1 = 0, 1, 1   <DO_LOOP>
; <72>                  |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <44>                  |   |   %"sub1_$A[][]_fetch" = (%"sub1_$A")[i1][i2];
; <45>                  |   |   %add40 = %"sub1_$A[][]_fetch"  +  2.000000e+00;
; <47>                  |   |   (%"sub1_$B")[i1][i2] = %add40;
; <48>                  |   |   %mul67 = %"sub1_$A[][]_fetch"  *  %add40;
; <49>                  |   |   %add68.lcssa118 = %add68.lcssa118  +  %mul67;
; <72>                  |   + END LOOP
; <71>                  + END LOOP
; <71>
; <64>                  (%"sub1_$T")[0] = %add68.lcssa118;
; <65>                  @newfunc_(&((%"sub1_$B")[0]),  &((%"sub1_$T")[0]));
; <2>                }
; <68>               ret ;
; <0>          END REGION

; after transformation
; CHECK:         BEGIN REGION { modified }

; CHECK:                  + Innermost: No
; CHECK:                  + LiveOut symbases: [[SYMBASE:[1-9]+]]
; CHECK:                  + DO i64 i1 = 0, 2, 2   <DO_LOOP>
; CHECK:                  |
; CHECK:                  |
; CHECK:                  |   + LiveOut symbases: [[SYMBASE]]
; CHECK:                  |   + DO i64 i2 = 0, 2, 2   <DO_LOOP>
; CHECK:                  |   |
; CHECK:                  |   |
; CHECK:                  |   |   + Blocking levels and factors:(1,0)
; CHECK:                  |   |   + DO i64 i3 = 0,
; CHECK:                  |   |   + Blocking levels and factors:(1,0)
; CHECK:                  |   |   |   + DO i64 i4 = 0,
; CHECK:                  |   |   |   |
; CHECK:                  |   |   |   + END LOOP
; CHECK:                  |   |   + END LOOP
; CHECK:                  |   |
; CHECK:                  |   |   [[OUT:%[a-z0-9.]+]] = 0.000000e+00;
; CHECK:                  |   |   <LVAL-REG> NON-LINEAR double [[OUT]] {sb:[[SYMBASE]]}
; CHECK:                  |   |
; CHECK:                  |   |   + LiveOut symbases: [[SYMBASE]]
; CHECK:                  |   |   + Blocking levels and factors:(1,0)
; CHECK:                  |   |   + DO i64 i3 = 0
; CHECK:                  |   |   |   + LiveOut symbases: [[SYMBASE]]
; CHECK:                  |   |   |   + Blocking levels and factors:(1,0)
; CHECK:                  |   |   |   + DO i64 i4 = 0
; CHECK:                  |   |   |   |   [[OUT]] = [[OUT]]  +
; CHECK:                  |   |   |   |   <LVAL-REG> NON-LINEAR double [[OUT]] {sb:[[SYMBASE]]}
; CHECK:                  |   |   |   |   <RVAL-REG> NON-LINEAR double [[OUT]] {sb:[[SYMBASE]]}
; CHECK:                  |   |   |   |
; CHECK:                  |   |   |   + END LOOP
; CHECK:                  |   |   + END LOOP
; CHECK:                  |   + END LOOP
; CHECK:                  + END LOOP
;
; CHECK:                  = [[OUT]];
; CHECK:                  <RVAL-REG> LINEAR double [[OUT]] {sb:[[SYMBASE]]}
;
; CHECK:               }
; CHECK:               ret ;
; CHECK:         END REGION

; after transformation
; <0>          BEGIN REGION { modified }
; <2>                if (%"sub1_$K_fetch" < %"sub1_$NTIMES_fetch")
; <2>                <RVAL-REG> LINEAR i32 %"sub1_$K_fetch" {sb:4}
; <2>                <RVAL-REG> LINEAR i32 %"sub1_$NTIMES_fetch" {sb:5}
; <2>
; <2>                {
; <73>                  + Ztt: No
; <73>                  + NumExits: 1
; <73>                  + Innermost: No
; <73>                  + HasSignedIV: Yes
; <73>                  + LiveIn symbases: 4, 5, 11, 12, 16, 34
; <73>                  + LiveOut symbases: 3, 35
; <73>                  + Loop metadata: No
; <73>                  + DO i64 i1 = 0, 2, 2   <DO_LOOP>
; <74>                  |   %tile_e_min = (i1 + 1 <= 2) ? i1 + 1 : 2;
; <74>                  |   <LVAL-REG> NON-LINEAR i64 %tile_e_min {sb:38}
; <74>                  |   <RVAL-REG> LINEAR i64 i1 + 1 {sb:2}
; <74>                  |   <RVAL-REG> LINEAR i64 i1 + 1 {sb:2}
; <74>                  |
; <75>                  |
; <75>                  |   + Ztt: No
; <75>                  |   + NumExits: 1
; <75>                  |   + Innermost: No
; <75>                  |   + HasSignedIV: Yes
; <75>                  |   + LiveIn symbases: 4, 5, 11, 12, 16, 34, 38
; <75>                  |   + LiveOut symbases: 3, 35
; <75>                  |   + Loop metadata: No
; <75>                  |   + DO i64 i2 = 0, 2, 2   <DO_LOOP>
; <76>                  |   |   %tile_e_min6 = (i2 + 1 <= 2) ? i2 + 1 : 2;
; <76>                  |   |   <LVAL-REG> NON-LINEAR i64 %tile_e_min6 {sb:39}
; <76>                  |   |   <RVAL-REG> LINEAR i64 i2 + 1 {sb:2}
; <76>                  |   |   <RVAL-REG> LINEAR i64 i2 + 1 {sb:2}
; <76>                  |   |
; <80>                  |   |   %lb_max7 = (0 <= i1) ? i1 : 0;
; <80>                  |   |   <LVAL-REG> NON-LINEAR i64 %lb_max7 {sb:42}
; <80>                  |   |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; <80>                  |   |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; <80>                  |   |
; <81>                  |   |   %ub_min8 = (2 <= %tile_e_min) ? 2 : %tile_e_min;
; <81>                  |   |   <LVAL-REG> NON-LINEAR i64 %ub_min8 {sb:43}
; <81>                  |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min {sb:38}
; <81>                  |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min {sb:38}
; <81>                  |   |
; <69>                  |   |
; <69>                  |   |   + Ztt: if (%lb_max7 < %ub_min8 + 1)
; <69>                  |   |   + NumExits: 1
; <69>                  |   |   + Innermost: No
; <69>                  |   |   + HasSignedIV: Yes
; <69>                  |   |   + LiveIn symbases: 11, 12, 16, 38, 39, 42, 43
; <69>                  |   |   + LiveOut symbases:
; <69>                  |   |   + Loop metadata: !llvm.loop !2
; <69>                  |   |   + Blocking levels and factors:(1,0)
; <69>                  |   |   + Blocking privates:
; <69>                  |   |   + DO i64 i3 = 0, -1 * %lb_max7 + %ub_min8, 1   <DO_LOOP> <nounroll>
; <69>                  |   |   | <RVAL-REG> LINEAR i64 -1 * %lb_max7 + %ub_min8{def@2} {sb:2}
; <69>                  |   |   |    <BLOB> LINEAR i64 %lb_max7{def@2} {sb:42}
; <69>                  |   |   |    <BLOB> LINEAR i64 %ub_min8{def@2} {sb:43}
; <69>                  |   |   | <ZTT-REG> LINEAR i64 %lb_max7{def@2} {sb:42}
; <69>                  |   |   | <ZTT-REG> LINEAR i64 %ub_min8 + 1{def@2} {sb:2}
; <69>                  |   |   |    <BLOB> LINEAR i64 %ub_min8{def@2} {sb:43}
; <69>                  |   |   |
; <77>                  |   |   |   %lb_max = (0 <= i2) ? i2 : 0;
; <77>                  |   |   |   <LVAL-REG> NON-LINEAR i64 %lb_max {sb:40}
; <77>                  |   |   |   <RVAL-REG> LINEAR i64 i2 {sb:2}
; <77>                  |   |   |   <RVAL-REG> LINEAR i64 i2 {sb:2}
; <77>                  |   |   |
; <78>                  |   |   |   %ub_min = (2 <= %tile_e_min6) ? 2 : %tile_e_min6;
; <78>                  |   |   |   <LVAL-REG> NON-LINEAR i64 %ub_min {sb:41}
; <78>                  |   |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min6 {sb:39}
; <78>                  |   |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min6 {sb:39}
; <78>                  |   |   |
; <70>                  |   |   |
; <70>                  |   |   |   + Ztt: if (%lb_max < %ub_min + 1)
; <70>                  |   |   |   + NumExits: 1
; <70>                  |   |   |   + Innermost: Yes
; <70>                  |   |   |   + HasSignedIV: Yes
; <70>                  |   |   |   + LiveIn symbases: 11, 12, 16, 39, 40, 41, 42, 43
; <70>                  |   |   |   + LiveOut symbases:
; <70>                  |   |   |   + Loop metadata: No
; <70>                  |   |   |   + Blocking levels and factors:(1,0)
; <70>                  |   |   |   + Blocking privates:
; <70>                  |   |   |   + DO i64 i4 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; <70>                  |   |   |   | <RVAL-REG> LINEAR i64 -1 * %lb_max + %ub_min{def@3} {sb:2}
; <70>                  |   |   |   |    <BLOB> LINEAR i64 %lb_max{def@3} {sb:40}
; <70>                  |   |   |   |    <BLOB> LINEAR i64 %ub_min{def@3} {sb:41}
; <70>                  |   |   |   | <ZTT-REG> LINEAR i64 %lb_max{def@3} {sb:40}
; <70>                  |   |   |   | <ZTT-REG> LINEAR i64 %ub_min + 1{def@3} {sb:2}
; <70>                  |   |   |   |    <BLOB> LINEAR i64 %ub_min{def@3} {sb:41}
; <70>                  |   |   |   |
; <16>                  |   |   |   |   %add13 = (%"sub1_$B")[i3 + %lb_max7][i4 + %lb_max]  +  1.000000e+00; <ninf,nsz,arcp>
; <16>                  |   |   |   |   <LVAL-REG> NON-LINEAR double %add13 {sb:14}
; <16>                  |   |   |   |   <RVAL-REG> {al:1}(LINEAR double* %"sub1_$B"{def@2})[LINEAR i64 i3 + %lb_max7{def@2}][LINEAR i64 i4 + %lb_max{def@3}] inbounds  {sb:35}
; <16>                  |   |   |   |      <BLOB> LINEAR i32 %"sub1_$N_fetch"{def@2} {sb:12}
; <16>                  |   |   |   |      <BLOB> LINEAR double* %"sub1_$B"{def@2} {sb:11}
; <16>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max7{def@2} {sb:42}
; <16>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max{def@3} {sb:40}
; <16>                  |   |   |   |
; <18>                  |   |   |   |   (%"sub1_$A")[i3 + %lb_max7][i4 + %lb_max] = %add13;
; <18>                  |   |   |   |   <LVAL-REG> {al:1}(LINEAR double* %"sub1_$A"{def@2})[LINEAR i64 i3 + %lb_max7{def@2}][LINEAR i64 i4 + %lb_max{def@3}] inbounds  {sb:36}
; <18>                  |   |   |   |      <BLOB> LINEAR i32 %"sub1_$N_fetch"{def@2} {sb:12}
; <18>                  |   |   |   |      <BLOB> LINEAR double* %"sub1_$A"{def@2} {sb:16}
; <18>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max7{def@2} {sb:42}
; <18>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max{def@3} {sb:40}
; <18>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %add13 {sb:14}
; <18>                  |   |   |   |
; <70>                  |   |   |   + END LOOP
; <69>                  |   |   + END LOOP
; <69>                  |   |
; <33>                  |   |   %add68.lcssa118 = 0.000000e+00;
; <33>                  |   |   <LVAL-REG> NON-LINEAR double %add68.lcssa118 {sb:3}
; <33>                  |   |
; <86>                  |   |   %lb_max11 = (0 <= i1) ? i1 : 0;
; <86>                  |   |   <LVAL-REG> NON-LINEAR i64 %lb_max11 {sb:46}
; <86>                  |   |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; <86>                  |   |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; <86>                  |   |
; <87>                  |   |   %ub_min12 = (1 <= %tile_e_min) ? 1 : %tile_e_min;
; <87>                  |   |   <LVAL-REG> NON-LINEAR i64 %ub_min12 {sb:47}
; <87>                  |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min {sb:38}
; <87>                  |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min {sb:38}
; <87>                  |   |
; <71>                  |   |
; <71>                  |   |   + Ztt: if (%lb_max11 < %ub_min12 + 1)
; <71>                  |   |   + NumExits: 1
; <71>                  |   |   + Innermost: No
; <71>                  |   |   + HasSignedIV: Yes
; <71>                  |   |   + LiveIn symbases: 3, 11, 12, 16, 38, 39, 46, 47
; <71>                  |   |   + LiveOut symbases: 3
; <71>                  |   |   + Loop metadata: No
; <71>                  |   |   + Blocking levels and factors:(1,0)
; <71>                  |   |   + Blocking privates:
; <71>                  |   |   + DO i64 i3 = 0, -1 * %lb_max11 + %ub_min12, 1   <DO_LOOP>
; <71>                  |   |   | <RVAL-REG> LINEAR i64 -1 * %lb_max11 + %ub_min12{def@2} {sb:2}
; <71>                  |   |   |    <BLOB> LINEAR i64 %lb_max11{def@2} {sb:46}
; <71>                  |   |   |    <BLOB> LINEAR i64 %ub_min12{def@2} {sb:47}
; <71>                  |   |   | <ZTT-REG> LINEAR i64 %lb_max11{def@2} {sb:46}
; <71>                  |   |   | <ZTT-REG> LINEAR i64 %ub_min12 + 1{def@2} {sb:2}
; <71>                  |   |   |    <BLOB> LINEAR i64 %ub_min12{def@2} {sb:47}
; <71>                  |   |   |
; <83>                  |   |   |   %lb_max9 = (0 <= i2) ? i2 : 0;
; <83>                  |   |   |   <LVAL-REG> NON-LINEAR i64 %lb_max9 {sb:44}
; <83>                  |   |   |   <RVAL-REG> LINEAR i64 i2 {sb:2}
; <83>                  |   |   |   <RVAL-REG> LINEAR i64 i2 {sb:2}
; <83>                  |   |   |
; <84>                  |   |   |   %ub_min10 = (2 <= %tile_e_min6) ? 2 : %tile_e_min6;
; <84>                  |   |   |   <LVAL-REG> NON-LINEAR i64 %ub_min10 {sb:45}
; <84>                  |   |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min6 {sb:39}
; <84>                  |   |   |   <RVAL-REG> NON-LINEAR i64 %tile_e_min6 {sb:39}
; <84>                  |   |   |
; <72>                  |   |   |
; <72>                  |   |   |   + Ztt: if (%lb_max9 < %ub_min10 + 1)
; <72>                  |   |   |   + NumExits: 1
; <72>                  |   |   |   + Innermost: Yes
; <72>                  |   |   |   + HasSignedIV: Yes
; <72>                  |   |   |   + LiveIn symbases: 3, 11, 12, 16, 39, 44, 45, 46, 47
; <72>                  |   |   |   + LiveOut symbases: 3
; <72>                  |   |   |   + Loop metadata: No
; <72>                  |   |   |   + Blocking levels and factors:(1,0)
; <72>                  |   |   |   + Blocking privates:
; <72>                  |   |   |   + DO i64 i4 = 0, -1 * %lb_max9 + %ub_min10, 1   <DO_LOOP>
; <72>                  |   |   |   | <RVAL-REG> LINEAR i64 -1 * %lb_max9 + %ub_min10{def@3} {sb:2}
; <72>                  |   |   |   |    <BLOB> LINEAR i64 %lb_max9{def@3} {sb:44}
; <72>                  |   |   |   |    <BLOB> LINEAR i64 %ub_min10{def@3} {sb:45}
; <72>                  |   |   |   | <ZTT-REG> LINEAR i64 %lb_max9{def@3} {sb:44}
; <72>                  |   |   |   | <ZTT-REG> LINEAR i64 %ub_min10 + 1{def@3} {sb:2}
; <72>                  |   |   |   |    <BLOB> LINEAR i64 %ub_min10{def@3} {sb:45}
; <72>                  |   |   |   |
; <44>                  |   |   |   |   %"sub1_$A[][]_fetch" = (%"sub1_$A")[i3 + %lb_max11][i4 + %lb_max9];
; <44>                  |   |   |   |   <LVAL-REG> NON-LINEAR double %"sub1_$A[][]_fetch" {sb:27}
; <44>                  |   |   |   |   <RVAL-REG> {al:1}(LINEAR double* %"sub1_$A"{def@2})[LINEAR i64 i3 + %lb_max11{def@2}][LINEAR i64 i4 + %lb_max9{def@3}] inbounds  {sb:36}
; <44>                  |   |   |   |      <BLOB> LINEAR i32 %"sub1_$N_fetch"{def@2} {sb:12}
; <44>                  |   |   |   |      <BLOB> LINEAR double* %"sub1_$A"{def@2} {sb:16}
; <44>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max11{def@2} {sb:46}
; <44>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max9{def@3} {sb:44}
; <44>                  |   |   |   |
; <45>                  |   |   |   |   %add40 = %"sub1_$A[][]_fetch"  +  2.000000e+00; <ninf,nsz,arcp>
; <45>                  |   |   |   |   <LVAL-REG> NON-LINEAR double %add40 {sb:28}
; <45>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %"sub1_$A[][]_fetch" {sb:27}
; <45>                  |   |   |   |
; <47>                  |   |   |   |   (%"sub1_$B")[i3 + %lb_max11][i4 + %lb_max9] = %add40;
; <47>                  |   |   |   |   <LVAL-REG> {al:1}(LINEAR double* %"sub1_$B"{def@2})[LINEAR i64 i3 + %lb_max11{def@2}][LINEAR i64 i4 + %lb_max9{def@3}] inbounds  {sb:35}
; <47>                  |   |   |   |      <BLOB> LINEAR i32 %"sub1_$N_fetch"{def@2} {sb:12}
; <47>                  |   |   |   |      <BLOB> LINEAR double* %"sub1_$B"{def@2} {sb:11}
; <47>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max11{def@2} {sb:46}
; <47>                  |   |   |   |      <BLOB> LINEAR i64 %lb_max9{def@3} {sb:44}
; <47>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %add40 {sb:28}
; <47>                  |   |   |   |
; <48>                  |   |   |   |   %mul67 = %"sub1_$A[][]_fetch"  *  %add40; <ninf,nsz,arcp>
; <48>                  |   |   |   |   <LVAL-REG> NON-LINEAR double %mul67 {sb:30}
; <48>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %"sub1_$A[][]_fetch" {sb:27}
; <48>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %add40 {sb:28}
; <48>                  |   |   |   |
; <49>                  |   |   |   |   %add68.lcssa118 = %add68.lcssa118  +  %mul67; <ninf,nsz,arcp>
; <49>                  |   |   |   |   <LVAL-REG> NON-LINEAR double %add68.lcssa118 {sb:3}
; <49>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %add68.lcssa118 {sb:3}
; <49>                  |   |   |   |   <RVAL-REG> NON-LINEAR double %mul67 {sb:30}
; <49>                  |   |   |   |
; <72>                  |   |   |   + END LOOP
; <71>                  |   |   + END LOOP
; <75>                  |   + END LOOP
; <73>                  + END LOOP
; <73>
; <64>                  (%"sub1_$T")[0] = %add68.lcssa118;
; <64>                  <LVAL-REG> {al:8}(LINEAR double* %"sub1_$T")[i64 0] inbounds  {sb:37}
; <64>                     <BLOB> LINEAR double* %"sub1_$T" {sb:34}
; <64>                  <RVAL-REG> LINEAR double %add68.lcssa118 {sb:3}
; <64>
; <65>                  @newfunc_(&((%"sub1_$B")[0]),  &((%"sub1_$T")[0]));
; <65>                  <RVAL-REG> &((LINEAR double* %"sub1_$B")[i64 0]) inbounds  {sb:35}
; <65>                     <BLOB> LINEAR double* %"sub1_$B" {sb:11}
; <65>                  <RVAL-REG> &((LINEAR double* %"sub1_$T")[i64 0]) inbounds  {sb:37}
; <65>                     <BLOB> LINEAR double* %"sub1_$T" {sb:34}
; <65>                  <FAKE-LVAL-REG> (LINEAR double* %"sub1_$B")[i64 undef] inbounds  {undefined} {sb:35}
; <65>                     <BLOB> LINEAR double* %"sub1_$B" {sb:11}
; <65>                  <FAKE-LVAL-REG> (LINEAR double* %"sub1_$T")[i64 undef] inbounds  {undefined} {sb:37}
; <65>                     <BLOB> LINEAR double* %"sub1_$T" {sb:34}
; <65>
; <2>                }
; <68>               ret ;
; <0>          END REGION

;Module Before HIR
; ModuleID = 'two-dim-if-liveout.f90'
source_filename = "two-dim-if-liveout.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @sub1_(ptr noalias nocapture dereferenceable(8) %"sub1_$A", ptr noalias dereferenceable(8) %"sub1_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$K", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$T" = alloca double, align 8
  %"sub1_$N_fetch" = load i32, ptr %"sub1_$N", align 1
  %int_sext = sext i32 %"sub1_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %"sub1_$K_fetch" = load i32, ptr %"sub1_$K", align 1
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %rel85 = icmp slt i32 %"sub1_$K_fetch", %"sub1_$NTIMES_fetch"
  br i1 %rel85, label %bb5.preheader, label %bb34_endif

bb5.preheader:                                    ; preds = %alloca_0
  br label %bb5

bb5:                                              ; preds = %bb5.preheader, %bb13
  %indvars.iv122 = phi i64 [ %indvars.iv.next123, %bb13 ], [ 1, %bb5.preheader ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %indvars.iv122)
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %indvars.iv122)
  br label %bb10

bb10:                                             ; preds = %bb10, %bb5
  %indvars.iv119 = phi i64 [ %indvars.iv.next120, %bb10 ], [ 1, %bb5 ]
  %"sub1_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]", i64 %indvars.iv119)
  %"sub1_$B[][]_fetch" = load double, ptr %"sub1_$B[][]", align 1
  %add13 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch", 1.000000e+00
  %"sub1_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]", i64 %indvars.iv119)
  store double %add13, ptr %"sub1_$A[][]", align 1
  %indvars.iv.next120 = add nuw nsw i64 %indvars.iv119, 1
  %exitcond121.not = icmp eq i64 %indvars.iv.next120, 4
  br i1 %exitcond121.not, label %bb13, label %bb10

bb13:                                             ; preds = %bb10
  %indvars.iv.next123 = add nuw nsw i64 %indvars.iv122, 1
  %exitcond124.not = icmp eq i64 %indvars.iv.next123, 4
  br i1 %exitcond124.not, label %bb19.preheader, label %bb5, !llvm.loop !0

bb19.preheader:                                   ; preds = %bb13
  br label %bb19

bb19:                                             ; preds = %bb19.preheader, %bb26
  %add68.lcssa118 = phi double [ %add68.lcssa, %bb26 ], [ 0.000000e+00, %bb19.preheader ]
  %rel84 = phi i1 [ false, %bb26 ], [ true, %bb19.preheader ]
  %"sub1_$I.1" = phi i64 [ 2, %bb26 ], [ 1, %bb19.preheader ]
  %"sub1_$A[]38" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$B[]47" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  br label %bb23

bb23:                                             ; preds = %bb23, %bb19
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb23 ], [ 1, %bb19 ]
  %add68116 = phi double [ %add68, %bb23 ], [ %add68.lcssa118, %bb19 ]
  %"sub1_$A[][]39" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]38", i64 %indvars.iv)
  %"sub1_$A[][]_fetch" = load double, ptr %"sub1_$A[][]39", align 1
  %add40 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch", 2.000000e+00
  %"sub1_$B[][]48" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]47", i64 %indvars.iv)
  store double %add40, ptr %"sub1_$B[][]48", align 1
  %mul67 = fmul reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch", %add40
  %add68 = fadd reassoc ninf nsz arcp contract afn double %add68116, %mul67
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %bb26, label %bb23

bb26:                                             ; preds = %bb23
  %add68.lcssa = phi double [ %add68, %bb23 ]
  br i1 %rel84, label %bb19, label %bb22

bb22:                                             ; preds = %bb26
  %add68.lcssa.lcssa = phi double [ %add68.lcssa, %bb26 ]
  store double %add68.lcssa.lcssa, ptr %"sub1_$T", align 8
  call void (...) @newfunc_(ptr nonnull %"sub1_$B", ptr nonnull %"sub1_$T") #2
  br label %bb34_endif

bb34_endif:                                       ; preds = %alloca_0, %bb22
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

declare void @newfunc_(...) local_unnamed_addr

attributes #0 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
