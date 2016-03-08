
; REQUIRES: asserts
; RUN: opt < %s -analyze -hir-region-identification -debug 2>&1 | FileCheck %s

; Verify that we detect and skip an infinite loop.
; CHECK: Unreachable/Infinite loops not supported

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noreturn nounwind
define void @main() #0 {
entry:
  br i1 undef, label %for.body.12.lr.ph, label %if.else.57

for.body.12.lr.ph:                                ; preds = %entry
  br label %for.body.12

for.body.12:                                      ; preds = %for.inc.51, %for.body.12.lr.ph
  %storemerge.28157 = phi i32 [ 1, %for.body.12.lr.ph ], [ %inc52, %for.inc.51 ]
  br i1 undef, label %for.inc.51, label %if.else

if.else:                                          ; preds = %for.body.12
  br i1 undef, label %land.lhs.true.25, label %if.else.36

land.lhs.true.25:                                 ; preds = %if.else
  br i1 undef, label %if.then.28, label %if.else.36

if.then.28:                                       ; preds = %land.lhs.true.25
  br label %for.inc.51

if.else.36:                                       ; preds = %land.lhs.true.25, %if.else
  br i1 undef, label %for.inc.51, label %if.then.40

if.then.40:                                       ; preds = %if.else.36
  br i1 undef, label %if.then.46, label %if.end

if.then.46:                                       ; preds = %if.then.40
  br label %if.end

if.end:                                           ; preds = %if.then.46, %if.then.40
  br label %for.inc.51

for.inc.51:                                       ; preds = %if.end, %if.else.36, %if.then.28, %for.body.12
  %i.0 = phi i32 [ undef, %if.then.28 ], [ %storemerge.28157, %if.else.36 ], [ %storemerge.28157, %if.end ], [ %storemerge.28157, %for.body.12 ]
  %inc52 = add nsw i32 %i.0, 1
  %cmp1 = icmp slt i32 %inc52, 15
  br i1 %cmp1, label %for.body.12, label %if.end.54

if.end.54:                                        ; preds = %for.inc.51
  br i1 undef, label %if.else.57, label %if.then.56

if.then.56:                                       ; preds = %if.end.54
  br label %if.end.58

if.else.57:                                       ; preds = %if.end.54, %entry
  br label %if.end.58

if.end.58:                                        ; preds = %if.else.57, %if.then.56
  br i1 undef, label %for.body.66.preheader, label %if.end.107

for.body.66.preheader:                            ; preds = %if.end.58
  br label %for.body.66

for.cond.99.preheader:                            ; preds = %for.inc.96
  br label %if.end.107

for.body.66:                                      ; preds = %for.inc.96, %for.body.66.preheader
  br i1 undef, label %for.inc.96, label %if.then.70

if.then.70:                                       ; preds = %for.body.66
  br i1 undef, label %land.lhs.true.81, label %for.inc.96

land.lhs.true.81:                                 ; preds = %if.then.70
  br i1 undef, label %if.then.85, label %for.inc.96

if.then.85:                                       ; preds = %land.lhs.true.81
  br i1 undef, label %if.then.91, label %for.inc.96

if.then.91:                                       ; preds = %if.then.85
  br label %for.inc.96

for.inc.96:                                       ; preds = %if.then.91, %if.then.85, %land.lhs.true.81, %if.then.70, %for.body.66
  br i1 undef, label %for.cond.99.preheader, label %for.body.66

if.end.107:                                       ; preds = %for.cond.99.preheader, %if.end.58
  br i1 undef, label %if.end.137, label %while.body.preheader

while.body.preheader:                             ; preds = %if.end.107
  br label %while.body

while.body:                                       ; preds = %if.end.132, %while.body.preheader
  br i1 undef, label %if.end.126, label %if.then.113

if.then.113:                                      ; preds = %while.body
  br i1 undef, label %if.end.117, label %if.then.116

if.then.116:                                      ; preds = %if.then.113
  br label %if.end.117

if.end.117:                                       ; preds = %if.then.116, %if.then.113
  br i1 undef, label %if.end.122, label %if.then.120

if.then.120:                                      ; preds = %if.end.117
  br label %if.end.122

if.end.122:                                       ; preds = %if.then.120, %if.end.117
  br i1 undef, label %if.end.137.loopexit, label %if.end.126

if.end.126:                                       ; preds = %if.end.122, %while.body
  br i1 undef, label %if.then.130, label %if.end.132

if.then.130:                                      ; preds = %if.end.126
  br label %if.end.132

if.end.132:                                       ; preds = %if.then.130, %if.end.126
  br i1 undef, label %if.end.137.loopexit, label %while.body

if.end.137.loopexit:                              ; preds = %if.end.132, %if.end.122
  br label %if.end.137

if.end.137:                                       ; preds = %if.end.137.loopexit, %if.end.107
  br i1 undef, label %if.end.141, label %if.then.139

if.then.139:                                      ; preds = %if.end.137
  br label %if.end.141

if.end.141:                                       ; preds = %if.then.139, %if.end.137
  br i1 undef, label %if.end.146, label %if.then.144

if.then.144:                                      ; preds = %if.end.141
  br label %if.end.146

if.end.146:                                       ; preds = %if.then.144, %if.end.141
  br i1 undef, label %if.end.151, label %if.then.148

if.then.148:                                      ; preds = %if.end.146
  br label %if.end.151

if.end.151:                                       ; preds = %if.then.148, %if.end.146
  br label %do.body

do.body:                                          ; preds = %do.body.backedge, %if.end.151
  br i1 undef, label %if.end.155, label %if.then.154

if.then.154:                                      ; preds = %do.body
  br label %if.end.155

if.end.155:                                       ; preds = %if.then.154, %do.body
  br label %do.body.157

do.body.157:                                      ; preds = %do.body.157.backedge, %if.end.155
  br i1 undef, label %if.then.177, label %land.lhs.true.164

land.lhs.true.164:                                ; preds = %do.body.157
  br i1 undef, label %if.end.171, label %if.then.177

if.end.171:                                       ; preds = %land.lhs.true.164
  br i1 undef, label %if.then.177, label %lor.lhs.false.174

lor.lhs.false.174:                                ; preds = %if.end.171
  switch i32 undef, label %if.end.213 [
    i32 2, label %if.then.177
    i32 1, label %if.then.268.loopexit
  ]

if.then.177:                                      ; preds = %lor.lhs.false.174, %if.end.171, %land.lhs.true.164, %do.body.157
  br i1 undef, label %if.then.181, label %if.end.189

if.then.181:                                      ; preds = %if.then.177
  br i1 undef, label %if.else.185, label %if.then.183

if.then.183:                                      ; preds = %if.then.181
  br label %if.end.187

if.else.185:                                      ; preds = %if.then.181
  br label %if.end.187

if.end.187:                                       ; preds = %if.else.185, %if.then.183
  br label %if.end.189

if.end.189:                                       ; preds = %if.end.187, %if.then.177
  br i1 undef, label %if.end.199, label %if.then.192

if.then.192:                                      ; preds = %if.end.189
  br i1 undef, label %if.else.196, label %if.then.194

if.then.194:                                      ; preds = %if.then.192
  br label %if.end.199

if.else.196:                                      ; preds = %if.then.192
  br label %if.end.199

if.end.199:                                       ; preds = %if.else.196, %if.then.194, %if.end.189
  br i1 undef, label %land.lhs.true.202, label %if.end.213

land.lhs.true.202:                                ; preds = %if.end.199
  br i1 undef, label %if.then.205, label %if.end.213

if.then.205:                                      ; preds = %land.lhs.true.202
  br label %if.end.213

if.end.213:                                       ; preds = %if.then.205, %land.lhs.true.202, %if.end.199, %lor.lhs.false.174
  switch i32 undef, label %do.cond [
    i32 0, label %if.then.218
    i32 3, label %do.body.157.backedge
  ]

if.then.218:                                      ; preds = %if.end.213
  br i1 undef, label %if.end.256, label %if.then.225

if.then.225:                                      ; preds = %if.then.218
  br i1 undef, label %if.then.228, label %if.end.230

if.then.228:                                      ; preds = %if.then.225
  br label %if.end.230

if.end.230:                                       ; preds = %if.then.228, %if.then.225
  switch i8 undef, label %if.then.249 [
    i8 32, label %if.end.253
    i8 0, label %if.end.253
  ]

if.then.249:                                      ; preds = %if.end.230
  br label %if.end.253

if.end.253:                                       ; preds = %if.then.249, %if.end.230, %if.end.230
  br label %if.end.256

if.end.256:                                       ; preds = %if.end.253, %if.then.218
  br label %do.cond

do.cond:                                          ; preds = %if.end.256, %if.end.213
  br i1 undef, label %do.body.157.backedge, label %do.end

do.body.157.backedge:                             ; preds = %do.cond, %if.end.213
  br label %do.body.157

do.end:                                           ; preds = %do.cond
  br i1 undef, label %if.then.268, label %if.end.269

if.then.268.loopexit:                             ; preds = %lor.lhs.false.174
  br label %if.then.268

if.then.268:                                      ; preds = %if.then.268.loopexit, %do.end
  br label %if.end.269

if.end.269:                                       ; preds = %if.then.268, %do.end
  br i1 undef, label %if.then.272, label %if.else.314

if.then.272:                                      ; preds = %if.end.269
  br i1 undef, label %if.then.283, label %if.end.292

if.then.283:                                      ; preds = %if.then.272
  br i1 undef, label %if.end.292, label %if.then.289

if.then.289:                                      ; preds = %if.then.283
  br label %if.end.292

if.end.292:                                       ; preds = %if.then.289, %if.then.283, %if.then.272
  br i1 undef, label %if.then.297, label %if.end.302

if.then.297:                                      ; preds = %if.end.292
  br label %if.end.302

if.end.302:                                       ; preds = %if.then.297, %if.end.292
  br i1 undef, label %if.end.307, label %if.then.305

if.then.305:                                      ; preds = %if.end.302
  br label %if.end.307

if.end.307:                                       ; preds = %if.then.305, %if.end.302
  br i1 undef, label %if.then.310, label %if.end.313

if.then.310:                                      ; preds = %if.end.307
  br label %if.end.313

if.end.313:                                       ; preds = %if.then.310, %if.end.307
  br label %do.cond.316

if.else.314:                                      ; preds = %if.end.269
  br label %do.cond.316

do.cond.316:                                      ; preds = %if.else.314, %if.end.313
  br i1 undef, label %do.end.318, label %do.body.backedge

do.body.backedge:                                 ; preds = %if.then.680, %for.cond.669.preheader, %do.cond.316
  br label %do.body

do.end.318:                                       ; preds = %do.cond.316
  br i1 undef, label %if.then.321, label %if.end.359

if.then.321:                                      ; preds = %do.end.318
  br i1 undef, label %if.then.352, label %if.end.359.thread51

if.then.352:                                      ; preds = %if.then.321
  br i1 undef, label %if.end.359.thread136, label %if.end.359.thread

if.end.359.thread:                                ; preds = %if.then.352
  br label %if.then.362

if.end.359.thread136:                             ; preds = %if.then.352
  br label %if.then.362

if.end.359.thread51:                              ; preds = %if.then.321
  br label %if.else.363

if.end.359:                                       ; preds = %do.end.318
  br i1 undef, label %if.then.362, label %if.else.363

if.then.362:                                      ; preds = %if.end.359, %if.end.359.thread136, %if.end.359.thread
  br label %if.end.366

if.else.363:                                      ; preds = %if.end.359, %if.end.359.thread51
  br label %if.end.366

if.end.366:                                       ; preds = %if.else.363, %if.then.362
  br i1 undef, label %if.then.370, label %if.end.371

if.then.370:                                      ; preds = %if.end.366
  br label %if.end.371

if.end.371:                                       ; preds = %if.then.370, %if.end.366
  br i1 undef, label %if.then.373, label %if.else.392

if.then.373:                                      ; preds = %if.end.371
  br i1 undef, label %if.then.376, label %if.end.635

if.then.376:                                      ; preds = %if.then.373
  br i1 undef, label %if.end.635, label %if.then.383

if.then.383:                                      ; preds = %if.then.376
  br i1 undef, label %if.else.387, label %if.then.385

if.then.385:                                      ; preds = %if.then.383
  br label %if.end.635

if.else.387:                                      ; preds = %if.then.383
  br label %if.end.635

if.else.392:                                      ; preds = %if.end.371
  br i1 undef, label %land.lhs.true.395, label %if.else.403

land.lhs.true.395:                                ; preds = %if.else.392
  br i1 undef, label %if.then.398, label %if.end.419

if.then.398:                                      ; preds = %land.lhs.true.395
  br label %if.end.419

if.else.403:                                      ; preds = %if.else.392
  br i1 undef, label %if.then.411, label %if.end.419

if.then.411:                                      ; preds = %if.else.403
  br label %if.end.419

if.end.419:                                       ; preds = %if.then.411, %if.else.403, %if.then.398, %land.lhs.true.395
  br i1 undef, label %if.else.484, label %if.then.421

if.then.421:                                      ; preds = %if.end.419
  br i1 undef, label %land.lhs.true.423, label %if.then.471

land.lhs.true.423:                                ; preds = %if.then.421
  br i1 undef, label %if.then.425, label %if.else.479

if.then.425:                                      ; preds = %land.lhs.true.423
  br i1 undef, label %if.end.433, label %if.then.430

if.then.430:                                      ; preds = %if.then.425
  br label %if.end.433

if.end.433:                                       ; preds = %if.then.430, %if.then.425
  br i1 undef, label %if.end.541, label %if.then.440

if.then.440:                                      ; preds = %if.end.433
  switch i8 undef, label %if.then.461 [
    i8 32, label %if.end.465
    i8 0, label %if.end.465
  ]

if.then.461:                                      ; preds = %if.then.440
  br label %if.end.465

if.end.465:                                       ; preds = %if.then.461, %if.then.440, %if.then.440
  br label %if.end.541

if.then.471:                                      ; preds = %if.then.421
  br i1 undef, label %if.end.476, label %if.then.473

if.then.473:                                      ; preds = %if.then.471
  br label %if.end.476

if.end.476:                                       ; preds = %if.then.473, %if.then.471
  br label %if.end.541

if.else.479:                                      ; preds = %land.lhs.true.423
  br label %if.end.541

if.else.484:                                      ; preds = %if.end.419
  br i1 undef, label %if.then.488, label %if.else.532

if.then.488:                                      ; preds = %if.else.484
  br i1 undef, label %if.end.496, label %if.then.493

if.then.493:                                      ; preds = %if.then.488
  br label %if.end.496

if.end.496:                                       ; preds = %if.then.493, %if.then.488
  br i1 undef, label %if.end.541, label %if.then.503

if.then.503:                                      ; preds = %if.end.496
  switch i8 undef, label %if.then.524 [
    i8 32, label %if.end.528
    i8 0, label %if.end.528
  ]

if.then.524:                                      ; preds = %if.then.503
  br label %if.end.528

if.end.528:                                       ; preds = %if.then.524, %if.then.503, %if.then.503
  br label %if.end.541

if.else.532:                                      ; preds = %if.else.484
  br i1 undef, label %if.end.537, label %if.then.534

if.then.534:                                      ; preds = %if.else.532
  br label %if.end.537

if.end.537:                                       ; preds = %if.then.534, %if.else.532
  br label %if.end.541

if.end.541:                                       ; preds = %if.end.537, %if.end.528, %if.end.496, %if.else.479, %if.end.476, %if.end.465, %if.end.433
  br i1 undef, label %if.then.544, label %if.end.555

if.then.544:                                      ; preds = %if.end.541
  br i1 undef, label %if.end.555, label %if.then.547

if.then.547:                                      ; preds = %if.then.544
  br i1 undef, label %if.else.551, label %if.then.549

if.then.549:                                      ; preds = %if.then.547
  br label %if.end.555

if.else.551:                                      ; preds = %if.then.547
  br label %if.end.555

if.end.555:                                       ; preds = %if.else.551, %if.then.549, %if.then.544, %if.end.541
  br i1 undef, label %if.then.571, label %if.end.576

if.then.571:                                      ; preds = %if.end.555
  br label %if.end.576

if.end.576:                                       ; preds = %if.then.571, %if.end.555
  br i1 undef, label %if.then.581, label %if.end.586

if.then.581:                                      ; preds = %if.end.576
  br label %if.end.586

if.end.586:                                       ; preds = %if.then.581, %if.end.576
  br i1 undef, label %if.then.591, label %if.end.592

if.then.591:                                      ; preds = %if.end.586
  br label %if.end.592

if.end.592:                                       ; preds = %if.then.591, %if.end.586
  br i1 undef, label %land.lhs.true.596, label %if.else.633

land.lhs.true.596:                                ; preds = %if.end.592
  br i1 undef, label %if.else.633, label %if.then.600

if.then.600:                                      ; preds = %land.lhs.true.596
  br i1 undef, label %for.end.612, label %for.inc.610.preheader

for.inc.610.preheader:                            ; preds = %if.then.600
  br label %for.inc.610

for.inc.610:                                      ; preds = %for.inc.610, %for.inc.610.preheader
  br i1 undef, label %for.inc.610, label %for.end.612.loopexit

for.end.612.loopexit:                             ; preds = %for.inc.610
  br label %for.end.612

for.end.612:                                      ; preds = %for.end.612.loopexit, %if.then.600
  br label %if.end.635

if.else.633:                                      ; preds = %land.lhs.true.596, %if.end.592
  br label %if.end.635

if.end.635:                                       ; preds = %if.else.633, %for.end.612, %if.else.387, %if.then.385, %if.then.376, %if.then.373
  br i1 undef, label %if.else.639, label %if.then.637

if.then.637:                                      ; preds = %if.end.635
  br label %if.end.641

if.else.639:                                      ; preds = %if.end.635
  br label %if.end.641

if.end.641:                                       ; preds = %if.else.639, %if.then.637
  br i1 undef, label %if.end.646, label %if.then.644

if.then.644:                                      ; preds = %if.end.641
  br label %if.end.646

if.end.646:                                       ; preds = %if.then.644, %if.end.641
  br i1 undef, label %for.inc.666.preheader, label %if.then.652

for.inc.666.preheader:                            ; preds = %if.then.652, %if.end.646
  br label %for.inc.666

if.then.652:                                      ; preds = %if.end.646
  br label %for.inc.666.preheader

for.cond.669.preheader:                           ; preds = %for.inc.666
  br i1 undef, label %if.then.680, label %do.body.backedge

for.inc.666:                                      ; preds = %for.inc.666, %for.inc.666.preheader
  br i1 undef, label %for.cond.669.preheader, label %for.inc.666

if.then.680:                                      ; preds = %for.cond.669.preheader
  br label %do.body.backedge
}
