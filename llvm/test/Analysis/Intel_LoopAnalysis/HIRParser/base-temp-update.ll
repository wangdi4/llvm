; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check that parsing is update to handle base temp update from add1165.lcssa1750.lcssa -> add1165.lcssa1750 successfully.
; CHECK: %add1165.lcssa1750 = 0.000000e+00;
; CHECK: %add1165.lcssa1750 = undef;
; CHECK: {al:8}(@ety2_.zz)[0] = %add1165.lcssa1750;


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-f4f7b34.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ety2_.zz = external global double, align 8

; Function Attrs: nounwind uwtable
define void @ety2_() #0 {
entry:
  br i1 undef, label %for.end20, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.inc18, %for.body.preheader
  br i1 undef, label %for.end, label %for.body7.preheader

for.body7.preheader:                              ; preds = %for.body
  br label %for.body7

for.body7:                                        ; preds = %for.body7, %for.body7.preheader
  br i1 undef, label %for.body7, label %for.cond5.for.end_crit_edge

for.cond5.for.end_crit_edge:                      ; preds = %for.body7
  br label %for.end

for.end:                                          ; preds = %for.cond5.for.end_crit_edge, %for.body
  br i1 undef, label %if.end, label %land.lhs.true

land.lhs.true:                                    ; preds = %for.end
  br i1 undef, label %if.end, label %for.inc18

if.end:                                           ; preds = %land.lhs.true, %for.end
  br label %for.inc18

for.inc18:                                        ; preds = %if.end, %land.lhs.true
  br i1 undef, label %for.body, label %for.cond.for.end20_crit_edge

for.cond.for.end20_crit_edge:                     ; preds = %for.inc18
  br label %for.end20

for.end20:                                        ; preds = %for.cond.for.end20_crit_edge, %entry
  br i1 undef, label %L340, label %if.end23.preheader

if.end23.preheader:                               ; preds = %for.end20
  br label %if.end23

if.end23:                                         ; preds = %L60.backedge, %if.end23.preheader
  br label %L70.outer

L70.outer.loopexit:                               ; preds = %for.inc505
  br label %L70.outer

L70.outer:                                        ; preds = %L70.outer.loopexit, %if.end23
  br label %L70

L70:                                              ; preds = %for.end281, %L70.outer
  br i1 undef, label %L70.L100_crit_edge, label %for.body28.preheader

for.body28.preheader:                             ; preds = %L70
  br label %for.body28

L70.L100_crit_edge:                               ; preds = %L70
  br label %L100

for.body28:                                       ; preds = %for.inc72, %for.body28.preheader
  br i1 undef, label %for.body28.L100_crit_edge, label %if.end33

if.end33:                                         ; preds = %for.body28
  br i1 undef, label %if.then56, label %if.end57

if.then56:                                        ; preds = %if.end33
  br label %if.end57

if.end57:                                         ; preds = %if.then56, %if.end33
  br i1 undef, label %for.inc72, label %if.end57.L100_crit_edge

for.inc72:                                        ; preds = %if.end57
  br i1 undef, label %for.body28, label %for.cond26.L100_crit_edge

if.end57.L100_crit_edge:                          ; preds = %if.end57
  br label %L100

for.cond26.L100_crit_edge:                        ; preds = %for.inc72
  br label %L100

for.body28.L100_crit_edge:                        ; preds = %for.body28
  br label %L100

L100:                                             ; preds = %for.body28.L100_crit_edge, %for.cond26.L100_crit_edge, %if.end57.L100_crit_edge, %L70.L100_crit_edge
  br i1 undef, label %L270, label %if.end80

if.end80:                                         ; preds = %L100
  br i1 undef, label %L280, label %if.end93

if.end93:                                         ; preds = %if.end80
  switch i64 undef, label %L130 [
    i64 30, label %L1000
    i64 20, label %if.end101
    i64 10, label %if.end101
  ]

if.end101:                                        ; preds = %if.end93, %if.end93
  br i1 undef, label %for.end112, label %for.body105.preheader

for.body105.preheader:                            ; preds = %if.end101
  br label %for.body105

for.body105:                                      ; preds = %for.body105, %for.body105.preheader
  br i1 undef, label %for.body105, label %for.cond103.for.end112_crit_edge

for.cond103.for.end112_crit_edge:                 ; preds = %for.body105
  br label %for.end112

for.end112:                                       ; preds = %for.cond103.for.end112_crit_edge, %if.end101
  br label %L130

L130:                                             ; preds = %for.end112, %if.end93
  br i1 undef, label %L130.L150_crit_edge, label %for.body138.preheader

for.body138.preheader:                            ; preds = %L130
  br label %for.body138

L130.L150_crit_edge:                              ; preds = %L130
  br label %L150

for.body138:                                      ; preds = %for.inc261, %for.body138.preheader
  br i1 undef, label %for.body138.L150_crit_edge, label %if.end195

if.end195:                                        ; preds = %for.body138
  br i1 undef, label %for.inc261, label %if.end195.L150_crit_edge

for.inc261:                                       ; preds = %if.end195
  br i1 undef, label %for.body138, label %for.cond136.L150_crit_edge

if.end195.L150_crit_edge:                         ; preds = %if.end195
  br label %L150

for.cond136.L150_crit_edge:                       ; preds = %for.inc261
  br label %L150

for.body138.L150_crit_edge:                       ; preds = %for.body138
  br label %L150

L150:                                             ; preds = %for.body138.L150_crit_edge, %for.cond136.L150_crit_edge, %if.end195.L150_crit_edge, %L130.L150_crit_edge
  br i1 undef, label %for.end281, label %for.body267.preheader

for.body267.preheader:                            ; preds = %L150
  br label %for.body267

for.body267:                                      ; preds = %for.inc279, %for.body267.preheader
  br i1 undef, label %for.inc279, label %if.end274

if.end274:                                        ; preds = %for.body267
  br label %for.inc279

for.inc279:                                       ; preds = %if.end274, %for.body267
  br i1 undef, label %for.body267, label %for.cond265.for.end281_crit_edge

for.cond265.for.end281_crit_edge:                 ; preds = %for.inc279
  br label %for.end281

for.end281:                                       ; preds = %for.cond265.for.end281_crit_edge, %L150
  br i1 undef, label %L70, label %for.body284.preheader

for.body284.preheader:                            ; preds = %for.end281
  br label %for.body284

for.body284:                                      ; preds = %for.inc505.for.body284_crit_edge, %for.body284.preheader
  br i1 undef, label %L170, label %if.end290

if.end290:                                        ; preds = %for.body284
  br i1 undef, label %if.then300, label %if.end306

if.then300:                                       ; preds = %if.end290
  br label %if.end306

if.end306:                                        ; preds = %if.then300, %if.end290
  br i1 undef, label %for.inc505, label %if.end333

if.end333:                                        ; preds = %if.end306
  br label %L170

L170:                                             ; preds = %if.end333, %for.body284
  br i1 undef, label %L180, label %if.end346

if.end346:                                        ; preds = %L170
  br label %L190

L180:                                             ; preds = %L170
  br i1 undef, label %L190, label %if.then355

if.then355:                                       ; preds = %L180
  br label %L190

L190:                                             ; preds = %if.then355, %L180, %if.end346
  br i1 undef, label %for.end413, label %for.body375.lr.ph

for.body375.lr.ph:                                ; preds = %L190
  br label %for.body375

for.body375:                                      ; preds = %L200, %for.body375.lr.ph
  br i1 undef, label %L200, label %if.end387

if.end387:                                        ; preds = %for.body375
  br label %L200

L200:                                             ; preds = %if.end387, %for.body375
  br i1 undef, label %for.body375, label %for.cond372.for.end413_crit_edge

for.cond372.for.end413_crit_edge:                 ; preds = %L200
  br label %for.end413

for.end413:                                       ; preds = %for.cond372.for.end413_crit_edge, %L190
  br i1 undef, label %for.end462, label %for.body424.lr.ph

for.body424.lr.ph:                                ; preds = %for.end413
  br label %for.body424

for.body424:                                      ; preds = %L220, %for.body424.lr.ph
  br i1 undef, label %L220, label %if.end437

if.end437:                                        ; preds = %for.body424
  br label %L220

L220:                                             ; preds = %if.end437, %for.body424
  br i1 undef, label %for.body424, label %for.cond421.for.end462_crit_edge

for.cond421.for.end462_crit_edge:                 ; preds = %L220
  br label %for.end462

for.end462:                                       ; preds = %for.cond421.for.end462_crit_edge, %for.end413
  br i1 undef, label %for.inc505, label %for.body466.lr.ph

for.body466.lr.ph:                                ; preds = %for.end462
  br label %for.body466

for.body466:                                      ; preds = %L240, %for.body466.lr.ph
  br i1 undef, label %L240, label %if.end479

if.end479:                                        ; preds = %for.body466
  br label %L240

L240:                                             ; preds = %if.end479, %for.body466
  br i1 undef, label %for.body466, label %for.cond463.for.inc505.loopexit_crit_edge

for.cond463.for.inc505.loopexit_crit_edge:        ; preds = %L240
  br label %for.inc505

for.inc505:                                       ; preds = %for.cond463.for.inc505.loopexit_crit_edge, %for.end462, %if.end306
  br i1 undef, label %for.inc505.for.body284_crit_edge, label %L70.outer.loopexit

for.inc505.for.body284_crit_edge:                 ; preds = %for.inc505
  br label %for.body284

L270:                                             ; preds = %L100
  br label %L60.backedge

L60.backedge:                                     ; preds = %for.cond641.L330.loopexit_crit_edge, %L320, %L270
  br i1 undef, label %L340.loopexit, label %if.end23

L280:                                             ; preds = %if.end80
  br i1 undef, label %L320, label %if.end543

if.end543:                                        ; preds = %L280
  br i1 undef, label %if.then552, label %if.end556

if.then552:                                       ; preds = %if.end543
  br label %if.end556

if.end556:                                        ; preds = %if.then552, %if.end543
  br i1 undef, label %for.end612, label %for.body588.preheader

for.body588.preheader:                            ; preds = %if.end556
  br label %for.body588

for.body588:                                      ; preds = %for.body588, %for.body588.preheader
  br i1 undef, label %for.body588, label %for.cond585.for.end612_crit_edge

for.cond585.for.end612_crit_edge:                 ; preds = %for.body588
  br label %for.end612

for.end612:                                       ; preds = %for.cond585.for.end612_crit_edge, %if.end556
  br i1 undef, label %for.body644.lr.ph, label %for.body616.lr.ph

for.body616.lr.ph:                                ; preds = %for.end612
  br label %for.body616

for.body616:                                      ; preds = %for.body616, %for.body616.lr.ph
  br i1 undef, label %for.body616, label %for.cond613.for.end640_crit_edge

for.cond613.for.end640_crit_edge:                 ; preds = %for.body616
  br label %for.body644.lr.ph

for.body644.lr.ph:                                ; preds = %for.cond613.for.end640_crit_edge, %for.end612
  br label %for.body644

for.body644:                                      ; preds = %for.body644, %for.body644.lr.ph
  br i1 undef, label %for.body644, label %for.cond641.L330.loopexit_crit_edge

L320:                                             ; preds = %L280
  br label %L60.backedge

for.cond641.L330.loopexit_crit_edge:              ; preds = %for.body644
  br label %L60.backedge

L340.loopexit:                                    ; preds = %L60.backedge
  br label %L340

L340:                                             ; preds = %L340.loopexit, %for.end20
  br i1 undef, label %L1001, label %if.end679

if.end679:                                        ; preds = %L340
  br i1 undef, label %for.cond1111.preheader.thread, label %for.body683.lr.ph

for.cond1111.preheader.thread:                    ; preds = %if.end679
  br label %for.end1137

for.body683.lr.ph:                                ; preds = %if.end679
  br label %for.body683

for.cond1111.preheader:                           ; preds = %for.inc1108
  br i1 undef, label %for.end1137, label %for.body1114.preheader

for.body1114.preheader:                           ; preds = %for.cond1111.preheader
  br label %for.body1114

for.body683:                                      ; preds = %for.inc1108, %for.body683.lr.ph
  br i1 undef, label %L710, label %if.else

if.else:                                          ; preds = %for.body683
  br i1 undef, label %L600, label %for.inc1108

L600:                                             ; preds = %if.else
  br i1 undef, label %for.inc1108, label %for.cond703.preheader

for.cond703.preheader:                            ; preds = %L600
  br i1 undef, label %for.inc1108, label %for.body706.preheader

for.body706.preheader:                            ; preds = %for.cond703.preheader
  br label %for.body706

for.body706:                                      ; preds = %for.inc812, %for.body706.preheader
  br i1 undef, label %for.cond719.preheader, label %L620

for.cond719.preheader:                            ; preds = %for.body706
  br label %for.body722

for.body722:                                      ; preds = %for.body722, %for.cond719.preheader
  br i1 undef, label %for.body722, label %for.cond719.L620.loopexit_crit_edge

for.cond719.L620.loopexit_crit_edge:              ; preds = %for.body722
  br label %L620

L620:                                             ; preds = %for.cond719.L620.loopexit_crit_edge, %for.body706
  br i1 undef, label %if.end738, label %L630

if.end738:                                        ; preds = %L620
  br label %for.inc812

L630:                                             ; preds = %L620
  br i1 undef, label %L640, label %if.end743

if.end743:                                        ; preds = %L630
  br label %for.inc812

L640:                                             ; preds = %L630
  br i1 undef, label %if.end795, label %L650

if.end795:                                        ; preds = %L640
  br label %for.inc812

L650:                                             ; preds = %L640
  br label %for.inc812

for.inc812:                                       ; preds = %L650, %if.end795, %if.end743, %if.end738
  br i1 undef, label %for.body706, label %for.cond703.for.inc1108.loopexit1531_crit_edge

L710:                                             ; preds = %for.body683
  br i1 undef, label %if.end838, label %L720

if.end838:                                        ; preds = %L710
  br label %L730

L720:                                             ; preds = %L710
  br label %L730

L730:                                             ; preds = %L720, %if.end838
  br i1 undef, label %for.inc1108, label %for.cond884.preheader

for.cond884.preheader:                            ; preds = %L730
  br i1 undef, label %for.body887.preheader, label %for.inc1108

for.body887.preheader:                            ; preds = %for.cond884.preheader
  br label %for.body887

for.body887:                                      ; preds = %for.inc1105, %for.body887.preheader
  br i1 undef, label %for.body899.preheader, label %for.end918

for.body899.preheader:                            ; preds = %for.body887
  br label %for.body899

for.body899:                                      ; preds = %for.body899, %for.body899.preheader
  br i1 undef, label %for.body899, label %for.cond896.for.end918_crit_edge

for.cond896.for.end918_crit_edge:                 ; preds = %for.body899
  br label %for.end918

for.end918:                                       ; preds = %for.cond896.for.end918_crit_edge, %for.body887
  br i1 undef, label %if.end923, label %L770

if.end923:                                        ; preds = %for.end918
  br label %for.inc1105

L770:                                             ; preds = %for.end918
  br i1 undef, label %L780, label %if.end928

if.end928:                                        ; preds = %L770
  br label %for.inc1105

L780:                                             ; preds = %L770
  br i1 undef, label %if.then966, label %L780.if.end1008_crit_edge

L780.if.end1008_crit_edge:                        ; preds = %L780
  br label %if.end1008

if.then966:                                       ; preds = %L780
  br label %if.end1008

if.end1008:                                       ; preds = %if.then966, %L780.if.end1008_crit_edge
  br i1 undef, label %if.end1051, label %L785

if.end1051:                                       ; preds = %if.end1008
  br label %for.inc1105

L785:                                             ; preds = %if.end1008
  br label %for.inc1105

for.inc1105:                                      ; preds = %L785, %if.end1051, %if.end928, %if.end923
  br i1 undef, label %for.body887, label %for.cond884.for.inc1108.loopexit_crit_edge

for.cond884.for.inc1108.loopexit_crit_edge:       ; preds = %for.inc1105
  br label %for.inc1108

for.cond703.for.inc1108.loopexit1531_crit_edge:   ; preds = %for.inc812
  br label %for.inc1108

for.inc1108:                                      ; preds = %for.cond703.for.inc1108.loopexit1531_crit_edge, %for.cond884.for.inc1108.loopexit_crit_edge, %for.cond884.preheader, %L730, %for.cond703.preheader, %L600, %if.else
  br i1 undef, label %for.body683, label %for.cond1111.preheader

for.body1114:                                     ; preds = %for.inc1135, %for.body1114.preheader
  br i1 undef, label %if.end1121, label %for.inc1135

if.end1121:                                       ; preds = %for.body1114
  br i1 undef, label %for.inc1135, label %for.body1125.preheader

for.body1125.preheader:                           ; preds = %if.end1121
  br label %for.body1125

for.body1125:                                     ; preds = %for.body1125, %for.body1125.preheader
  br i1 undef, label %for.body1125, label %for.cond1122.for.inc1135.loopexit_crit_edge

for.cond1122.for.inc1135.loopexit_crit_edge:      ; preds = %for.body1125
  br label %for.inc1135

for.inc1135:                                      ; preds = %for.cond1122.for.inc1135.loopexit_crit_edge, %if.end1121, %for.body1114
  br i1 undef, label %for.body1114, label %for.cond1111.for.end1137_crit_edge

for.cond1111.for.end1137_crit_edge:               ; preds = %for.inc1135
  br label %for.end1137

for.end1137:                                      ; preds = %for.cond1111.for.end1137_crit_edge, %for.cond1111.preheader, %for.cond1111.preheader.thread
  br i1 undef, label %L1001, label %for.body1141.preheader

for.body1141.preheader:                           ; preds = %for.end1137
  br label %for.body1141

for.body1141:                                     ; preds = %for.inc1175, %for.body1141.preheader
  %inc11761552 = phi i64 [ undef, %for.inc1175 ], [ undef, %for.body1141.preheader ]
  br i1 undef, label %for.inc1175, label %for.body1153.lr.ph

for.body1153.lr.ph:                               ; preds = %for.body1141
  br label %for.body1153

for.body1153:                                     ; preds = %for.end1168, %for.body1153.lr.ph
  %inc11731545 = phi i64 [ undef, %for.body1153.lr.ph ], [ undef, %for.end1168 ]
  br i1 undef, label %for.end1168, label %for.body1157.preheader

for.body1157.preheader:                           ; preds = %for.body1153
  br label %for.body1157

for.body1157:                                     ; preds = %for.body1157, %for.body1157.preheader
  %inc11671543 = phi i64 [ undef, %for.body1157 ], [ undef, %for.body1157.preheader ]
  %cmp1155 = icmp slt i64 %inc11671543, undef
  br i1 %cmp1155, label %for.body1157, label %for.cond1154.for.end1168_crit_edge

for.cond1154.for.end1168_crit_edge:               ; preds = %for.body1157
  br label %for.end1168

for.end1168:                                      ; preds = %for.cond1154.for.end1168_crit_edge, %for.body1153
  %add1165.lcssa1750 = phi double [ undef, %for.cond1154.for.end1168_crit_edge ], [ 0.000000e+00, %for.body1153 ]
  %cmp1151 = icmp slt i64 %inc11731545, undef
  br i1 %cmp1151, label %for.body1153, label %for.cond1150.for.inc1175_crit_edge

for.cond1150.for.inc1175_crit_edge:               ; preds = %for.end1168
  %add1165.lcssa1750.lcssa = phi double [ %add1165.lcssa1750, %for.end1168 ]
  store double %add1165.lcssa1750.lcssa, double* @ety2_.zz, align 8
  br label %for.inc1175

for.inc1175:                                      ; preds = %for.cond1150.for.inc1175_crit_edge, %for.body1141
  %cmp1139 = icmp sgt i64 undef, %inc11761552
  br i1 %cmp1139, label %for.body1141, label %for.cond1138.L1001.loopexit_crit_edge

L1000:                                            ; preds = %if.end93
  br label %L1001

for.cond1138.L1001.loopexit_crit_edge:            ; preds = %for.inc1175
  br label %L1001

L1001:                                            ; preds = %for.cond1138.L1001.loopexit_crit_edge, %L1000, %for.end1137, %L340
  ret void
}

