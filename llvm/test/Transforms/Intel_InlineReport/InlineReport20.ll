; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck  --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; This tests the "huge function" heuristic modification to the single callsite
; inlining heuristic.  The funciton myhelper2 should not inline.

target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_match = type { i32, i32 }
%struct.lzma_range_encoder = type { i64, i64, i32, i8, i64, i64, [58 x i32], [58 x i16*] }
%struct.lzma_length_encoder = type { i16, i16, [16 x [8 x i16]], [16 x [8 x i16]], [256 x i16], [16 x [272 x i32]], i32, [16 x i32] }
%struct.lzma_optimal = type { i32, i8, i8, i32, i32, i32, i32, i32, [4 x i32] }
%struct.lzma_coder_s = type { %struct.lzma_range_encoder, i32, [4 x i32], [274 x %struct.lzma_match], i32, i32, i8, i8, i8, i32, i32, i32, [16 x [768 x i16]], [12 x [16 x i16]], [12 x i16], [12 x i16], [12 x i16], [12 x i16], [12 x [16 x i16]], [4 x [64 x i16]], [114 x i16], [16 x i16], %struct.lzma_length_encoder, %struct.lzma_length_encoder, [4 x [64 x i32]], [4 x [128 x i32]], i32, i32, [16 x i32], i32, i32, i32, [4096 x %struct.lzma_optimal] }

@lzma_rc_prices = external dso_local constant [128 x i8], align 16
@lzma_fastpos = external dso_local constant [8192 x i8], align 16

define internal fastcc i32 @myhelper2(i8* %pcoder, i32* %reps, i8* %buf, i32 %len_end, i32 %position, i32 %cur, i32 %nice_len, i32 %buf_avail_full) unnamed_addr #5 {
entry:
  %0 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %matches_count1 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 4
  %1 = load i32, i32* %matches_count1, align 4
  %longest_match_length = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 5
  %2 = load i32, i32* %longest_match_length, align 8
  %opts = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 32
  %idxprom = zext i32 %cur to i64
  %arrayidx = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom
  %pos_prev2 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 6
  %3 = load i32, i32* %pos_prev2, align 4
  %prev_1_is_literal = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 1
  %4 = load i8, i8* %prev_1_is_literal, align 4
  %tobool = icmp eq i8 %4, 0
  br i1 %tobool, label %if.else39, label %if.then

if.then:                                          ; preds = %entry
  %dec = add i32 %3, -1
  %prev_2 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 2
  %5 = load i8, i8* %prev_2, align 1
  %tobool9 = icmp eq i8 %5, 0
  br i1 %tobool9, label %if.else25, label %if.then10

if.then10:                                        ; preds = %if.then
  %pos_prev_2 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 3
  %6 = load i32, i32* %pos_prev_2, align 4
  %idxprom15 = zext i32 %6 to i64
  %arrayidx16 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom15
  %state17 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx16, i64 0, i32 0
  %7 = load i32, i32* %state17, align 4
  %back_prev_2 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 4
  %8 = load i32, i32* %back_prev_2, align 4
  %cmp = icmp ult i32 %8, 4
  %cmp22 = icmp ult i32 %7, 7
  %cond = select i1 %cmp22, i32 8, i32 11
  %cond24 = select i1 %cmp22, i32 7, i32 10
  %9 = select i1 %cmp, i32 %cond, i32 %cond24
  br label %cond.false

if.else25:                                        ; preds = %if.then
  %idxprom27 = zext i32 %dec to i64
  %arrayidx28 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom27
  %state29 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx28, i64 0, i32 0
  %10 = load i32, i32* %state29, align 4
  %cmp31 = icmp ult i32 %10, 4
  br i1 %cmp31, label %if.end44, label %cond.false

cond.false:                                       ; preds = %if.then10, %if.else25
  %state.01402 = phi i32 [ %10, %if.else25 ], [ %9, %if.then10 ]
  %cmp32 = icmp ult i32 %state.01402, 10
  %.v1399 = select i1 %cmp32, i32 -3, i32 -6
  %11 = add i32 %state.01402, %.v1399
  br label %if.end44

if.else39:                                        ; preds = %entry
  %idxprom41 = zext i32 %3 to i64
  %arrayidx42 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom41
  %state43 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx42, i64 0, i32 0
  %12 = load i32, i32* %state43, align 4
  br label %if.end44

if.end44:                                         ; preds = %if.else25, %cond.false, %if.else39
  %state.1 = phi i32 [ %12, %if.else39 ], [ 0, %if.else25 ], [ %11, %cond.false ]
  %pos_prev.0 = phi i32 [ %3, %if.else39 ], [ %dec, %cond.false ], [ %dec, %if.else25 ]
  %sub45 = add i32 %cur, -1
  %cmp46 = icmp eq i32 %pos_prev.0, %sub45
  br i1 %cmp46, label %if.then47, label %if.else69

if.then47:                                        ; preds = %if.end44
  %back_prev = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 7
  %13 = load i32, i32* %back_prev, align 4
  %cmp51 = icmp eq i32 %13, 0
  br i1 %cmp51, label %if.then52, label %if.else55

if.then52:                                        ; preds = %if.then47
  %cmp53 = icmp ult i32 %state.1, 7
  %cond54 = select i1 %cmp53, i32 9, i32 11
  br label %if.end157

if.else55:                                        ; preds = %if.then47
  %cmp56 = icmp ult i32 %state.1, 4
  br i1 %cmp56, label %if.end157, label %cond.false58

cond.false58:                                     ; preds = %if.else55
  %cmp59 = icmp ult i32 %state.1, 10
  %.v1398 = select i1 %cmp59, i32 -3, i32 -6
  %14 = add i32 %state.1, %.v1398
  br label %if.end157

if.else69:                                        ; preds = %if.end44
  %15 = load i8, i8* %prev_1_is_literal, align 4
  %tobool74 = icmp eq i8 %15, 0
  br i1 %tobool74, label %if.else91, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.else69
  %prev_278 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 2
  %16 = load i8, i8* %prev_278, align 1
  %tobool79 = icmp eq i8 %16, 0
  br i1 %tobool79, label %if.else91, label %if.then80

if.then80:                                        ; preds = %land.lhs.true
  %pos_prev_284 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 3
  %17 = load i32, i32* %pos_prev_284, align 4
  %back_prev_288 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 4
  %18 = load i32, i32* %back_prev_288, align 4
  %cmp89 = icmp ult i32 %state.1, 7
  %cond90 = select i1 %cmp89, i32 8, i32 11
  %cmp105 = icmp ult i32 %18, 4
  br i1 %cmp105, label %if.then106, label %if.else137

if.else91:                                        ; preds = %land.lhs.true, %if.else69
  %back_prev95 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 7
  %19 = load i32, i32* %back_prev95, align 4
  %cmp96 = icmp ult i32 %19, 4
  %cmp98 = icmp ult i32 %state.1, 7
  br i1 %cmp96, label %if.then97, label %if.else100

if.then97:                                        ; preds = %if.else91
  %cond99 = select i1 %cmp98, i32 8, i32 11
  br label %if.then106

if.else100:                                       ; preds = %if.else91
  %cond102 = select i1 %cmp98, i32 7, i32 10
  br label %if.else137

if.then106:                                       ; preds = %if.then97, %if.then80
  %pos_prev.11410 = phi i32 [ %pos_prev.0, %if.then97 ], [ %17, %if.then80 ]
  %state.21409 = phi i32 [ %cond99, %if.then97 ], [ %cond90, %if.then80 ]
  %pos.01407 = phi i32 [ %19, %if.then97 ], [ %18, %if.then80 ]
  %idxprom108 = zext i32 %pos_prev.11410 to i64
  %arrayidx109 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom108
  %backs = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx109, i64 0, i32 8
  %idxprom110 = zext i32 %pos.01407 to i64
  %arrayidx111 = getelementptr inbounds [4 x i32], [4 x i32]* %backs, i64 0, i64 %idxprom110
  %20 = load i32, i32* %arrayidx111, align 4
  store i32 %20, i32* %reps, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %if.then106
  %i.0 = phi i32 [ 1, %if.then106 ], [ %inc, %for.body ]
  %cmp113 = icmp ugt i32 %i.0, %pos.01407
  br i1 %cmp113, label %for.cond123, label %for.body

for.body:                                         ; preds = %for.cond
  %sub118 = add i32 %i.0, -1
  %idxprom119 = zext i32 %sub118 to i64
  %arrayidx120 = getelementptr inbounds [4 x i32], [4 x i32]* %backs, i64 0, i64 %idxprom119
  %21 = load i32, i32* %arrayidx120, align 4
  %idxprom121 = zext i32 %i.0 to i64
  %arrayidx122 = getelementptr inbounds i32, i32* %reps, i64 %idxprom121
  store i32 %21, i32* %arrayidx122, align 4
  %inc = add i32 %i.0, 1
  br label %for.cond

for.cond123:                                      ; preds = %for.cond, %for.body125
  %i.1 = phi i32 [ %i.0, %for.cond ], [ %inc135, %for.body125 ]
  %cmp124 = icmp ult i32 %i.1, 4
  br i1 %cmp124, label %for.body125, label %if.end157

for.body125:                                      ; preds = %for.cond123
  %idxprom130 = zext i32 %i.1 to i64
  %arrayidx131 = getelementptr inbounds [4 x i32], [4 x i32]* %backs, i64 0, i64 %idxprom130
  %22 = load i32, i32* %arrayidx131, align 4
  %arrayidx133 = getelementptr inbounds i32, i32* %reps, i64 %idxprom130
  store i32 %22, i32* %arrayidx133, align 4
  %inc135 = add i32 %i.1, 1
  br label %for.cond123

if.else137:                                       ; preds = %if.else100, %if.then80
  %pos_prev.11418 = phi i32 [ %pos_prev.0, %if.else100 ], [ %17, %if.then80 ]
  %state.21417 = phi i32 [ %cond102, %if.else100 ], [ %cond90, %if.then80 ]
  %pos.01416 = phi i32 [ %19, %if.else100 ], [ %18, %if.then80 ]
  %sub138 = add i32 %pos.01416, -4
  store i32 %sub138, i32* %reps, align 4
  br label %for.cond141

for.cond141:                                      ; preds = %for.body143, %if.else137
  %i140.0 = phi i32 [ 1, %if.else137 ], [ %inc154, %for.body143 ]
  %cmp142 = icmp ult i32 %i140.0, 4
  br i1 %cmp142, label %for.body143, label %if.end157

for.body143:                                      ; preds = %for.cond141
  %idxprom145 = zext i32 %pos_prev.11418 to i64
  %arrayidx146 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom145
  %backs147 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx146, i64 0, i32 8
  %sub148 = add i32 %i140.0, -1
  %idxprom149 = zext i32 %sub148 to i64
  %arrayidx150 = getelementptr inbounds [4 x i32], [4 x i32]* %backs147, i64 0, i64 %idxprom149
  %23 = load i32, i32* %arrayidx150, align 4
  %idxprom151 = zext i32 %i140.0 to i64
  %arrayidx152 = getelementptr inbounds i32, i32* %reps, i64 %idxprom151
  store i32 %23, i32* %arrayidx152, align 4
  %inc154 = add i32 %i140.0, 1
  br label %for.cond141

if.end157:                                        ; preds = %for.cond123, %for.cond141, %if.else55, %cond.false58, %if.then52
  %state.3 = phi i32 [ %cond54, %if.then52 ], [ 0, %if.else55 ], [ %14, %cond.false58 ], [ %state.21409, %for.cond123 ], [ %state.21417, %for.cond141 ]
  %state161 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 0
  store i32 %state.3, i32* %state161, align 4
  br label %for.cond163

for.cond163:                                      ; preds = %for.body166, %if.end157
  %i162.0 = phi i32 [ 0, %if.end157 ], [ %inc176, %for.body166 ]
  %cmp164 = icmp ult i32 %i162.0, 4
  br i1 %cmp164, label %for.body166, label %for.cond.cleanup165

for.cond.cleanup165:                              ; preds = %for.cond163
  %price = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 5
  %24 = load i32, i32* %price, align 4
  %25 = load i8, i8* %buf, align 1
  %26 = load i32, i32* %reps, align 4
  %idx.ext = zext i32 %26 to i64
  %idx.neg = sub nsw i64 0, %idx.ext
  %add.ptr = getelementptr inbounds i8, i8* %buf, i64 %idx.neg
  %add.ptr182 = getelementptr inbounds i8, i8* %add.ptr, i64 -1
  %27 = load i8, i8* %add.ptr182, align 1
  %pos_mask = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 9
  %28 = load i32, i32* %pos_mask, align 8
  %and = and i32 %28, %position
  %is_match = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 13
  %idxprom183 = zext i32 %state.3 to i64
  %arrayidx184 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 %idxprom183
  %idxprom185 = zext i32 %and to i64
  %arrayidx186 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx184, i64 0, i64 %idxprom185
  %29 = load i16, i16* %arrayidx186, align 2
  %30 = lshr i16 %29, 4
  %idxprom.i = zext i16 %30 to i64
  %arrayidx.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i
  %31 = load i8, i8* %arrayidx.i, align 1
  %conv1.i = zext i8 %31 to i32
  %add = add i32 %24, %conv1.i
  %arrayidx187 = getelementptr inbounds i8, i8* %buf, i64 -1
  %32 = load i8, i8* %arrayidx187, align 1
  %conv = zext i8 %32 to i32
  %cmp188 = icmp ult i32 %state.3, 7
  %lnot = xor i1 %cmp188, true
  %conv190 = zext i8 %27 to i32
  %conv191 = zext i8 %25 to i32
  %33 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %literal.i1601 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %33, i64 0, i32 12
  %literal_pos_mask.i1602 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %33, i64 0, i32 11
  %34 = load i32, i32* %literal_pos_mask.i1602, align 8
  %and.i1603 = and i32 %34, %position
  %literal_context_bits.i1604 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %33, i64 0, i32 10
  %35 = load i32, i32* %literal_context_bits.i1604, align 4
  %shl.mask.i1605 = and i32 %35, 31
  %shl.i1606 = shl i32 %and.i1603, %shl.mask.i1605
  %sub.i1607 = sub i32 8, %35
  %shl.mask2.i1608 = and i32 %sub.i1607, 31
  %shr.i1609 = lshr i32 %conv, %shl.mask2.i1608
  %add.i1610 = add i32 %shr.i1609, %shl.i1606
  %idxprom.i1611 = zext i32 %add.i1610 to i64
  %arrayidx.i1612 = getelementptr inbounds [16 x [768 x i16]], [16 x [768 x i16]]* %literal.i1601, i64 0, i64 %idxprom.i1611
  %arraydecay.i1613 = getelementptr inbounds [768 x i16], [768 x i16]* %arrayidx.i1612, i64 0, i64 0
  %add.i.i1614 = add i32 %conv191, 256
  br i1 %lnot, label %do.body.i1650, label %do.body.i.i

do.body.i.i:                                      ; preds = %do.body.i.i, %for.cond.cleanup165
  %price.0.i.i = phi i32 [ %add1.i.i, %do.body.i.i ], [ 0, %for.cond.cleanup165 ]
  %symbol.addr.0.i.i = phi i32 [ %shr.i.i1616, %do.body.i.i ], [ %add.i.i1614, %for.cond.cleanup165 ]
  %and.i.i1615 = and i32 %symbol.addr.0.i.i, 1
  %shr.i.i1616 = lshr i32 %symbol.addr.0.i.i, 1
  %idxprom.i.i1617 = zext i32 %shr.i.i1616 to i64
  %arrayidx.i.i1618 = getelementptr inbounds i16, i16* %arraydecay.i1613, i64 %idxprom.i.i1617
  %36 = load i16, i16* %arrayidx.i.i1618, align 2
  %conv.i.i.i = zext i16 %36 to i32
  %sub.i.i.i = sub nsw i32 0, %and.i.i1615
  %and.i.i.i = and i32 %sub.i.i.i, 2032
  %xor.i.i.i = xor i32 %and.i.i.i, %conv.i.i.i
  %shr.i.i.i = lshr i32 %xor.i.i.i, 4
  %idxprom.i.i.i1619 = zext i32 %shr.i.i.i to i64
  %arrayidx.i.i.i1620 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i.i1619
  %37 = load i8, i8* %arrayidx.i.i.i1620, align 1
  %conv1.i.i.i1621 = zext i8 %37 to i32
  %add1.i.i = add i32 %price.0.i.i, %conv1.i.i.i1621
  %cmp.i.i1622 = icmp eq i32 %shr.i.i1616, 1
  br i1 %cmp.i.i1622, label %get_literal_price.exit1651, label %do.body.i.i

do.body.i1650:                                    ; preds = %do.body.i1650, %for.cond.cleanup165
  %offset.0.i1623 = phi i32 [ %and16.i1648, %do.body.i1650 ], [ 256, %for.cond.cleanup165 ]
  %price.0.i1624 = phi i32 [ %add14.i1644, %do.body.i1650 ], [ 0, %for.cond.cleanup165 ]
  %symbol.addr.0.i1625 = phi i32 [ %shl15.i1645, %do.body.i1650 ], [ %add.i.i1614, %for.cond.cleanup165 ]
  %match_byte.addr.0.i1626 = phi i32 [ %shl4.i1627, %do.body.i1650 ], [ %conv190, %for.cond.cleanup165 ]
  %shl4.i1627 = shl i32 %match_byte.addr.0.i1626, 1
  %and5.i1628 = and i32 %shl4.i1627, %offset.0.i1623
  %shr7.i1629 = lshr i32 %symbol.addr.0.i1625, 8
  %add6.i1630 = add i32 %shr7.i1629, %offset.0.i1623
  %add8.i1631 = add i32 %add6.i1630, %and5.i1628
  %shr9.i1632 = lshr i32 %symbol.addr.0.i1625, 7
  %and10.i1633 = and i32 %shr9.i1632, 1
  %idxprom11.i1634 = zext i32 %add8.i1631 to i64
  %arrayidx12.i1635 = getelementptr inbounds i16, i16* %arraydecay.i1613, i64 %idxprom11.i1634
  %38 = load i16, i16* %arrayidx12.i1635, align 2
  %conv.i.i1636 = zext i16 %38 to i32
  %sub.i.i1637 = sub nsw i32 0, %and10.i1633
  %and.i39.i1638 = and i32 %sub.i.i1637, 2032
  %xor.i.i1639 = xor i32 %and.i39.i1638, %conv.i.i1636
  %shr.i40.i1640 = lshr i32 %xor.i.i1639, 4
  %idxprom.i41.i1641 = zext i32 %shr.i40.i1640 to i64
  %arrayidx.i42.i1642 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i41.i1641
  %39 = load i8, i8* %arrayidx.i42.i1642, align 1
  %conv1.i.i1643 = zext i8 %39 to i32
  %add14.i1644 = add i32 %price.0.i1624, %conv1.i.i1643
  %shl15.i1645 = shl i32 %symbol.addr.0.i1625, 1
  %xor.i1646 = xor i32 %shl15.i1645, -1
  %neg.i1647 = xor i32 %shl4.i1627, %xor.i1646
  %and16.i1648 = and i32 %neg.i1647, %offset.0.i1623
  %cmp.i1649 = icmp ult i32 %shl15.i1645, 65536
  br i1 %cmp.i1649, label %do.body.i1650, label %get_literal_price.exit1651

get_literal_price.exit1651:                       ; preds = %do.body.i.i, %do.body.i1650
  %price.1.i = phi i32 [ %add14.i1644, %do.body.i1650 ], [ %add1.i.i, %do.body.i.i ]
  %add193 = add i32 %add, %price.1.i
  %add195 = add i32 %cur, 1
  %idxprom196 = zext i32 %add195 to i64
  %arrayidx197 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom196
  %price198 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 5
  %40 = load i32, i32* %price198, align 4
  %cmp199 = icmp ult i32 %add193, %40
  br i1 %cmp199, label %if.then201, label %if.end216

for.body166:                                      ; preds = %for.cond163
  %idxprom167 = zext i32 %i162.0 to i64
  %arrayidx168 = getelementptr inbounds i32, i32* %reps, i64 %idxprom167
  %41 = load i32, i32* %arrayidx168, align 4
  %backs172 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx, i64 0, i32 8
  %arrayidx174 = getelementptr inbounds [4 x i32], [4 x i32]* %backs172, i64 0, i64 %idxprom167
  store i32 %41, i32* %arrayidx174, align 4
  %inc176 = add i32 %i162.0, 1
  br label %for.cond163

if.then201:                                       ; preds = %get_literal_price.exit1651
  store i32 %add193, i32* %price198, align 4
  %pos_prev211 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 6
  store i32 %cur, i32* %pos_prev211, align 4
  %back_prev.i1599 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 7
  store i32 -1, i32* %back_prev.i1599, align 4
  %prev_1_is_literal.i1600 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 1
  store i8 0, i8* %prev_1_is_literal.i1600, align 4
  br label %if.end216

if.end216:                                        ; preds = %if.then201, %get_literal_price.exit1651
  %next_is_literal.0 = phi i8 [ 1, %if.then201 ], [ 0, %get_literal_price.exit1651 ]
  %42 = load i16, i16* %arrayidx186, align 2
  %43 = lshr i16 %42, 4
  %44 = xor i16 %43, 127
  %idxprom.i1596 = zext i16 %44 to i64
  %arrayidx.i1597 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1596
  %45 = load i8, i8* %arrayidx.i1597, align 1
  %conv1.i1598 = zext i8 %45 to i32
  %add223 = add i32 %24, %conv1.i1598
  %is_rep = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 14
  %arrayidx225 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep, i64 0, i64 %idxprom183
  %46 = load i16, i16* %arrayidx225, align 2
  %47 = lshr i16 %46, 4
  %48 = xor i16 %47, 127
  %idxprom.i1593 = zext i16 %48 to i64
  %arrayidx.i1594 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1593
  %49 = load i8, i8* %arrayidx.i1594, align 1
  %conv1.i1595 = zext i8 %49 to i32
  %add227 = add i32 %add223, %conv1.i1595
  %cmp230 = icmp eq i8 %27, %25
  br i1 %cmp230, label %land.lhs.true232, label %if.end274

land.lhs.true232:                                 ; preds = %if.end216
  %pos_prev237 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 6
  %50 = load i32, i32* %pos_prev237, align 4
  %cmp238 = icmp ult i32 %50, %cur
  br i1 %cmp238, label %land.lhs.true240, label %if.then248

land.lhs.true240:                                 ; preds = %land.lhs.true232
  %back_prev245 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 7
  %51 = load i32, i32* %back_prev245, align 4
  %cmp246 = icmp eq i32 %51, 0
  br i1 %cmp246, label %if.end274, label %if.then248

if.then248:                                       ; preds = %land.lhs.true240, %land.lhs.true232
  %52 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %is_rep0.i1582 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %52, i64 0, i32 15
  %idxprom.i1583 = zext i32 %state.3 to i64
  %arrayidx.i1584 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep0.i1582, i64 0, i64 %idxprom.i1583
  %53 = load i16, i16* %arrayidx.i1584, align 2
  %54 = lshr i16 %53, 4
  %idxprom.i.i1585 = zext i16 %54 to i64
  %arrayidx.i.i1586 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i1585
  %55 = load i8, i8* %arrayidx.i.i1586, align 1
  %conv1.i.i1587 = zext i8 %55 to i32
  %is_rep0_long.i1588 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %52, i64 0, i32 18
  %arrayidx2.i1589 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_rep0_long.i1588, i64 0, i64 %idxprom.i1583
  %idxprom3.i1590 = zext i32 %and to i64
  %arrayidx4.i1591 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx2.i1589, i64 0, i64 %idxprom3.i1590
  %56 = load i16, i16* %arrayidx4.i1591, align 2
  %57 = lshr i16 %56, 4
  %idxprom.i9.i = zext i16 %57 to i64
  %arrayidx.i10.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i9.i
  %58 = load i8, i8* %arrayidx.i10.i, align 1
  %conv1.i11.i = zext i8 %58 to i32
  %add.i1592 = add nuw nsw i32 %conv1.i11.i, %conv1.i.i1587
  %add250 = add i32 %add227, %add.i1592
  %59 = load i32, i32* %price198, align 4
  %cmp256 = icmp ugt i32 %add250, %59
  br i1 %cmp256, label %if.end274, label %if.then258

if.then258:                                       ; preds = %if.then248
  store i32 %add250, i32* %price198, align 4
  store i32 %cur, i32* %pos_prev237, align 4
  %back_prev.i = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 7
  store i32 0, i32* %back_prev.i, align 4
  %prev_1_is_literal.i = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx197, i64 0, i32 1
  store i8 0, i8* %prev_1_is_literal.i, align 4
  br label %if.end274

if.end274:                                        ; preds = %if.then258, %if.then248, %land.lhs.true240, %if.end216
  %next_is_literal.2 = phi i8 [ %next_is_literal.0, %land.lhs.true240 ], [ %next_is_literal.0, %if.end216 ], [ 1, %if.then258 ], [ %next_is_literal.0, %if.then248 ]
  %cmp275 = icmp ult i32 %buf_avail_full, 2
  br i1 %cmp275, label %cleanup913, label %if.end278

if.end278:                                        ; preds = %if.end274
  %cmp279 = icmp ult i32 %buf_avail_full, %nice_len
  %60 = select i1 %cmp279, i32 %buf_avail_full, i32 %nice_len
  %61 = and i8 %next_is_literal.2, 1
  %tobool285 = icmp ne i8 %61, 0
  %cmp289 = icmp eq i8 %27, %25
  %or.cond = or i1 %tobool285, %cmp289
  br i1 %or.cond, label %if.end393, label %if.then291

if.then291:                                       ; preds = %if.end278
  %62 = load i32, i32* %reps, align 4
  %idx.ext293 = zext i32 %62 to i64
  %idx.neg294 = sub nsw i64 0, %idx.ext293
  %add.ptr295 = getelementptr inbounds i8, i8* %buf, i64 %idx.neg294
  %add.ptr296 = getelementptr inbounds i8, i8* %add.ptr295, i64 -1
  %add297 = add i32 %nice_len, 1
  %cmp298 = icmp ugt i32 %add297, %buf_avail_full
  %63 = select i1 %cmp298, i32 %buf_avail_full, i32 %add297
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.then291
  %len_test.0 = phi i32 [ 1, %if.then291 ], [ %inc315, %while.body ]
  %cmp305 = icmp ult i32 %len_test.0, %63
  br i1 %cmp305, label %land.rhs, label %while.end

land.rhs:                                         ; preds = %while.cond
  %idxprom307 = zext i32 %len_test.0 to i64
  %arrayidx308 = getelementptr inbounds i8, i8* %buf, i64 %idxprom307
  %64 = load i8, i8* %arrayidx308, align 1
  %arrayidx311 = getelementptr inbounds i8, i8* %add.ptr296, i64 %idxprom307
  %65 = load i8, i8* %arrayidx311, align 1
  %cmp313 = icmp eq i8 %64, %65
  br i1 %cmp313, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs
  %inc315 = add i32 %len_test.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond, %land.rhs
  %dec316 = add i32 %len_test.0, -1
  %cmp317 = icmp ugt i32 %dec316, 1
  br i1 %cmp317, label %if.then319, label %if.end393

if.then319:                                       ; preds = %while.end
  %cmp320 = icmp ult i32 %state.3, 4
  br i1 %cmp320, label %cond.end332, label %cond.false323

cond.false323:                                    ; preds = %if.then319
  %cmp324 = icmp ult i32 %state.3, 10
  %.v = select i1 %cmp324, i32 -3, i32 -6
  %66 = add i32 %state.3, %.v
  br label %cond.end332

cond.end332:                                      ; preds = %cond.false323, %if.then319
  %cond333 = phi i32 [ 0, %if.then319 ], [ %66, %cond.false323 ]
  %add334 = add i32 %position, 1
  %67 = load i32, i32* %pos_mask, align 8
  %and336 = and i32 %add334, %67
  %idxprom338 = zext i32 %cond333 to i64
  %arrayidx339 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 %idxprom338
  %idxprom340 = zext i32 %and336 to i64
  %arrayidx341 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx339, i64 0, i64 %idxprom340
  %68 = load i16, i16* %arrayidx341, align 2
  %69 = lshr i16 %68, 4
  %70 = xor i16 %69, 127
  %idxprom.i1579 = zext i16 %70 to i64
  %arrayidx.i1580 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1579
  %71 = load i8, i8* %arrayidx.i1580, align 1
  %conv1.i1581 = zext i8 %71 to i32
  %add343 = add i32 %add193, %conv1.i1581
  %arrayidx346 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep, i64 0, i64 %idxprom338
  %72 = load i16, i16* %arrayidx346, align 2
  %73 = lshr i16 %72, 4
  %74 = xor i16 %73, 127
  %idxprom.i1576 = zext i16 %74 to i64
  %arrayidx.i1577 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1576
  %75 = load i8, i8* %arrayidx.i1577, align 1
  %conv1.i1578 = zext i8 %75 to i32
  %add348 = add i32 %add343, %conv1.i1578
  %add350 = add i32 %len_test.0, %cur
  br label %while.cond351

while.cond351:                                    ; preds = %while.body354, %cond.end332
  %len_end.addr.0 = phi i32 [ %len_end, %cond.end332 ], [ %inc356, %while.body354 ]
  %cmp352 = icmp ult i32 %len_end.addr.0, %add350
  br i1 %cmp352, label %while.body354, label %while.end360

while.body354:                                    ; preds = %while.cond351
  %inc356 = add i32 %len_end.addr.0, 1
  %idxprom357 = zext i32 %inc356 to i64
  %arrayidx358 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom357
  %price359 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx358, i64 0, i32 5
  store i32 1073741824, i32* %price359, align 4
  br label %while.cond351

while.end360:                                     ; preds = %while.cond351
  %76 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %rep_len_encoder.i1556 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %76, i64 0, i32 23
  %prices.i.i1557 = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %rep_len_encoder.i1556, i64 0, i32 5
  %idxprom.i.i1558 = zext i32 %and336 to i64
  %arrayidx.i.i1559 = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i.i1557, i64 0, i64 %idxprom.i.i1558
  %sub.i.i1560 = add i32 %dec316, -2
  %idxprom1.i.i1561 = zext i32 %sub.i.i1560 to i64
  %arrayidx2.i.i1562 = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i.i1559, i64 0, i64 %idxprom1.i.i1561
  %77 = load i32, i32* %arrayidx2.i.i1562, align 4
  %is_rep0.i.i1563 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %76, i64 0, i32 15
  %idxprom.i1.i = zext i32 %cond333 to i64
  %arrayidx.i2.i1564 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep0.i.i1563, i64 0, i64 %idxprom.i1.i
  %78 = load i16, i16* %arrayidx.i2.i1564, align 2
  %79 = lshr i16 %78, 4
  %idxprom.i.i.i1565 = zext i16 %79 to i64
  %arrayidx.i.i.i1566 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i.i1565
  %80 = load i8, i8* %arrayidx.i.i.i1566, align 1
  %conv1.i.i.i1567 = zext i8 %80 to i32
  %is_rep0_long.i.i1568 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %76, i64 0, i32 18
  %arrayidx2.i3.i1569 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_rep0_long.i.i1568, i64 0, i64 %idxprom.i1.i
  %arrayidx4.i.i1570 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx2.i3.i1569, i64 0, i64 %idxprom.i.i1558
  %81 = load i16, i16* %arrayidx4.i.i1570, align 2
  %82 = lshr i16 %81, 4
  %83 = xor i16 %82, 127
  %idxprom.i53.i.i1571 = zext i16 %83 to i64
  %arrayidx.i54.i.i1572 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i53.i.i1571
  %84 = load i8, i8* %arrayidx.i54.i.i1572, align 1
  %conv1.i55.i.i1573 = zext i8 %84 to i32
  %add.i.i1574 = add i32 %77, %conv1.i.i.i1567
  %add.i1575 = add i32 %add.i.i1574, %conv1.i55.i.i1573
  %add362 = add i32 %add348, %add.i1575
  %idxprom364 = zext i32 %add350 to i64
  %arrayidx365 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom364
  %price366 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx365, i64 0, i32 5
  %85 = load i32, i32* %price366, align 4
  %cmp367 = icmp ult i32 %add362, %85
  br i1 %cmp367, label %if.then369, label %if.end393

if.then369:                                       ; preds = %while.end360
  store i32 %add362, i32* %price366, align 4
  %pos_prev378 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx365, i64 0, i32 6
  store i32 %add195, i32* %pos_prev378, align 4
  %back_prev382 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx365, i64 0, i32 7
  store i32 0, i32* %back_prev382, align 4
  %prev_1_is_literal386 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx365, i64 0, i32 1
  store i8 1, i8* %prev_1_is_literal386, align 4
  %prev_2390 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx365, i64 0, i32 2
  store i8 0, i8* %prev_2390, align 1
  br label %if.end393

if.end393:                                        ; preds = %while.end, %if.then369, %while.end360, %if.end278
  %len_end.addr.2 = phi i32 [ %len_end, %if.end278 ], [ %len_end, %while.end ], [ %len_end.addr.0, %if.then369 ], [ %len_end.addr.0, %while.end360 ]
  br label %for.cond394

for.cond394:                                      ; preds = %cleanup, %if.end393
  %rep_index.0 = phi i32 [ 0, %if.end393 ], [ %inc643, %cleanup ]
  %start_len.0 = phi i32 [ 2, %if.end393 ], [ %start_len.2, %cleanup ]
  %len_end.addr.3 = phi i32 [ %len_end.addr.2, %if.end393 ], [ %len_end.addr.7, %cleanup ]
  %cmp395 = icmp ult i32 %rep_index.0, 4
  br i1 %cmp395, label %for.body398, label %for.cond.cleanup397

for.cond.cleanup397:                              ; preds = %for.cond394
  %cmp646 = icmp ugt i32 %2, %60
  br i1 %cmp646, label %while.cond649, label %if.end662

for.body398:                                      ; preds = %for.cond394
  %idxprom400 = zext i32 %rep_index.0 to i64
  %arrayidx401 = getelementptr inbounds i32, i32* %reps, i64 %idxprom400
  %86 = load i32, i32* %arrayidx401, align 4
  %idx.ext402 = zext i32 %86 to i64
  %idx.neg403 = sub nsw i64 0, %idx.ext402
  %add.ptr404 = getelementptr inbounds i8, i8* %buf, i64 %idx.neg403
  %add.ptr405 = getelementptr inbounds i8, i8* %add.ptr404, i64 -1
  %87 = load i8, i8* %buf, align 1
  %88 = load i8, i8* %add.ptr405, align 1
  %cmp410 = icmp eq i8 %87, %88
  br i1 %cmp410, label %lor.lhs.false, label %cleanup

lor.lhs.false:                                    ; preds = %for.body398
  %arrayidx412 = getelementptr inbounds i8, i8* %buf, i64 1
  %89 = load i8, i8* %arrayidx412, align 1
  %arrayidx414 = getelementptr inbounds i8, i8* %add.ptr405, i64 1
  %90 = load i8, i8* %arrayidx414, align 1
  %cmp416 = icmp eq i8 %89, %90
  br i1 %cmp416, label %for.cond421, label %cleanup

for.cond421:                                      ; preds = %lor.lhs.false, %for.inc435
  %len_test420.0 = phi i32 [ %inc436, %for.inc435 ], [ 2, %lor.lhs.false ]
  %cmp422 = icmp ult i32 %len_test420.0, %60
  br i1 %cmp422, label %land.rhs424, label %for.end437

land.rhs424:                                      ; preds = %for.cond421
  %idxprom425 = zext i32 %len_test420.0 to i64
  %arrayidx426 = getelementptr inbounds i8, i8* %buf, i64 %idxprom425
  %91 = load i8, i8* %arrayidx426, align 1
  %arrayidx429 = getelementptr inbounds i8, i8* %add.ptr405, i64 %idxprom425
  %92 = load i8, i8* %arrayidx429, align 1
  %cmp431 = icmp eq i8 %91, %92
  br i1 %cmp431, label %for.inc435, label %for.end437

for.inc435:                                       ; preds = %land.rhs424
  %inc436 = add i32 %len_test420.0, 1
  br label %for.cond421

for.end437:                                       ; preds = %for.cond421, %land.rhs424
  br label %while.cond438

while.cond438:                                    ; preds = %while.body442, %for.end437
  %len_end.addr.4 = phi i32 [ %len_end.addr.3, %for.end437 ], [ %inc444, %while.body442 ]
  %add439 = add i32 %len_test420.0, %cur
  %cmp440 = icmp ult i32 %len_end.addr.4, %add439
  br i1 %cmp440, label %while.body442, label %while.end448

while.body442:                                    ; preds = %while.cond438
  %inc444 = add i32 %len_end.addr.4, 1
  %idxprom445 = zext i32 %inc444 to i64
  %arrayidx446 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom445
  %price447 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx446, i64 0, i32 5
  store i32 1073741824, i32* %price447, align 4
  br label %while.cond438

while.end448:                                     ; preds = %while.cond438
  %93 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %cmp.i1539 = icmp eq i32 %rep_index.0, 0
  %is_rep0.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %93, i64 0, i32 15
  %idxprom.i1540 = zext i32 %state.3 to i64
  %arrayidx.i1541 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep0.i, i64 0, i64 %idxprom.i1540
  %94 = load i16, i16* %arrayidx.i1541, align 2
  %95 = lshr i16 %94, 4
  br i1 %cmp.i1539, label %if.then.i1547, label %if.else.i1548

if.then.i1547:                                    ; preds = %while.end448
  %idxprom.i.i1542 = zext i16 %95 to i64
  %arrayidx.i.i1543 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i1542
  %96 = load i8, i8* %arrayidx.i.i1543, align 1
  %conv1.i.i1544 = zext i8 %96 to i32
  %is_rep0_long.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %93, i64 0, i32 18
  %arrayidx2.i1545 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_rep0_long.i, i64 0, i64 %idxprom.i1540
  %idxprom3.i = zext i32 %and to i64
  %arrayidx4.i = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx2.i1545, i64 0, i64 %idxprom3.i
  %97 = load i16, i16* %arrayidx4.i, align 2
  %98 = lshr i16 %97, 4
  %99 = xor i16 %98, 127
  %idxprom.i53.i = zext i16 %99 to i64
  %arrayidx.i54.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i53.i
  %100 = load i8, i8* %arrayidx.i54.i, align 1
  %conv1.i55.i = zext i8 %100 to i32
  %add.i1546 = add nuw nsw i32 %conv1.i55.i, %conv1.i.i1544
  br label %get_pure_rep_price.exit

if.else.i1548:                                    ; preds = %while.end448
  %101 = xor i16 %95, 127
  %idxprom.i50.i = zext i16 %101 to i64
  %arrayidx.i51.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i50.i
  %102 = load i8, i8* %arrayidx.i51.i, align 1
  %conv1.i52.i = zext i8 %102 to i32
  %cmp10.i = icmp eq i32 %rep_index.0, 1
  %is_rep1.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %93, i64 0, i32 16
  %arrayidx13.i = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep1.i, i64 0, i64 %idxprom.i1540
  %103 = load i16, i16* %arrayidx13.i, align 2
  %104 = lshr i16 %103, 4
  br i1 %cmp10.i, label %if.then11.i, label %if.else16.i

if.then11.i:                                      ; preds = %if.else.i1548
  %idxprom.i47.i = zext i16 %104 to i64
  %arrayidx.i48.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i47.i
  %105 = load i8, i8* %arrayidx.i48.i, align 1
  %conv1.i49.i = zext i8 %105 to i32
  %add15.i = add nuw nsw i32 %conv1.i49.i, %conv1.i52.i
  br label %get_pure_rep_price.exit

if.else16.i:                                      ; preds = %if.else.i1548
  %106 = xor i16 %104, 127
  %idxprom.i44.i = zext i16 %106 to i64
  %arrayidx.i45.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i44.i
  %107 = load i8, i8* %arrayidx.i45.i, align 1
  %conv1.i46.i = zext i8 %107 to i32
  %add21.i = add nuw nsw i32 %conv1.i46.i, %conv1.i52.i
  %is_rep2.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %93, i64 0, i32 17
  %arrayidx23.i = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep2.i, i64 0, i64 %idxprom.i1540
  %108 = load i16, i16* %arrayidx23.i, align 2
  %conv.i.i1549 = zext i16 %108 to i32
  %sub.i.i1550 = sub i32 2, %rep_index.0
  %and.i.i = and i32 %sub.i.i1550, 2032
  %xor.i.i1551 = xor i32 %and.i.i, %conv.i.i1549
  %shr.i.i1552 = lshr i32 %xor.i.i1551, 4
  %idxprom.i41.i1553 = zext i32 %shr.i.i1552 to i64
  %arrayidx.i42.i1554 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i41.i1553
  %109 = load i8, i8* %arrayidx.i42.i1554, align 1
  %conv1.i43.i = zext i8 %109 to i32
  %add25.i = add nuw nsw i32 %add21.i, %conv1.i43.i
  br label %get_pure_rep_price.exit

get_pure_rep_price.exit:                          ; preds = %if.then.i1547, %if.then11.i, %if.else16.i
  %price.0.i1555 = phi i32 [ %add.i1546, %if.then.i1547 ], [ %add15.i, %if.then11.i ], [ %add25.i, %if.else16.i ]
  %add451 = add i32 %add227, %price.0.i1555
  br label %do.body

do.body:                                          ; preds = %if.end483, %get_pure_rep_price.exit
  %len_test420.1 = phi i32 [ %len_test420.0, %get_pure_rep_price.exit ], [ %dec484, %if.end483 ]
  %rep_len_encoder = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 23
  %prices.i1533 = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %rep_len_encoder, i64 0, i32 5
  %idxprom.i1534 = zext i32 %and to i64
  %arrayidx.i1535 = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i1533, i64 0, i64 %idxprom.i1534
  %sub.i1536 = add i32 %len_test420.1, -2
  %idxprom1.i1537 = zext i32 %sub.i1536 to i64
  %arrayidx2.i1538 = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i1535, i64 0, i64 %idxprom1.i1537
  %110 = load i32, i32* %arrayidx2.i1538, align 4
  %add454 = add i32 %add451, %110
  %add456 = add i32 %len_test420.1, %cur
  %idxprom457 = zext i32 %add456 to i64
  %arrayidx458 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom457
  %price459 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx458, i64 0, i32 5
  %111 = load i32, i32* %price459, align 4
  %cmp460 = icmp ult i32 %add454, %111
  br i1 %cmp460, label %if.then462, label %if.end483

if.then462:                                       ; preds = %do.body
  store i32 %add454, i32* %price459, align 4
  %pos_prev472 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx458, i64 0, i32 6
  store i32 %cur, i32* %pos_prev472, align 4
  %back_prev477 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx458, i64 0, i32 7
  store i32 %rep_index.0, i32* %back_prev477, align 4
  %prev_1_is_literal482 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx458, i64 0, i32 1
  store i8 0, i8* %prev_1_is_literal482, align 4
  br label %if.end483

if.end483:                                        ; preds = %if.then462, %do.body
  %dec484 = add i32 %len_test420.1, -1
  %cmp485 = icmp ugt i32 %dec484, 1
  br i1 %cmp485, label %do.body, label %do.end

do.end:                                           ; preds = %if.end483
  %cmp487 = icmp eq i32 %rep_index.0, 0
  %add490 = add i32 %len_test420.0, 1
  %spec.select = select i1 %cmp487, i32 %add490, i32 %start_len.0
  %add492 = add i32 %len_test420.0, 1
  %add494 = add i32 %add492, %nice_len
  %cmp495 = icmp ugt i32 %add494, %buf_avail_full
  %112 = select i1 %cmp495, i32 %buf_avail_full, i32 %add494
  br label %for.cond502

for.cond502:                                      ; preds = %for.inc516, %do.end
  %len_test_2.0 = phi i32 [ %add492, %do.end ], [ %inc517, %for.inc516 ]
  %cmp503 = icmp ult i32 %len_test_2.0, %112
  br i1 %cmp503, label %land.rhs505, label %for.end518

land.rhs505:                                      ; preds = %for.cond502
  %idxprom506 = zext i32 %len_test_2.0 to i64
  %arrayidx507 = getelementptr inbounds i8, i8* %buf, i64 %idxprom506
  %113 = load i8, i8* %arrayidx507, align 1
  %arrayidx510 = getelementptr inbounds i8, i8* %add.ptr405, i64 %idxprom506
  %114 = load i8, i8* %arrayidx510, align 1
  %cmp512 = icmp eq i8 %113, %114
  br i1 %cmp512, label %for.inc516, label %for.end518

for.inc516:                                       ; preds = %land.rhs505
  %inc517 = add i32 %len_test_2.0, 1
  br label %for.cond502

for.end518:                                       ; preds = %for.cond502, %land.rhs505
  %sub520 = sub i32 %len_test_2.0, %add492
  %cmp521 = icmp ugt i32 %sub520, 1
  br i1 %cmp521, label %if.then523, label %cleanup

if.then523:                                       ; preds = %for.end518
  %cond527 = select i1 %cmp188, i64 8, i64 11
  %add529 = add i32 %len_test420.0, %position
  %115 = load i32, i32* %pos_mask, align 8
  %and531 = and i32 %add529, %115
  %prices.i = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %rep_len_encoder, i64 0, i32 5
  %idxprom.i1530 = zext i32 %and to i64
  %arrayidx.i1531 = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i, i64 0, i64 %idxprom.i1530
  %sub.i1532 = add i32 %len_test420.0, -2
  %idxprom1.i = zext i32 %sub.i1532 to i64
  %arrayidx2.i = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i1531, i64 0, i64 %idxprom1.i
  %116 = load i32, i32* %arrayidx2.i, align 4
  %add534 = add i32 %add451, %116
  %arrayidx537 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 %cond527
  %idxprom538 = zext i32 %and531 to i64
  %arrayidx539 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx537, i64 0, i64 %idxprom538
  %117 = load i16, i16* %arrayidx539, align 2
  %118 = lshr i16 %117, 4
  %idxprom.i1527 = zext i16 %118 to i64
  %arrayidx.i1528 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1527
  %119 = load i8, i8* %arrayidx.i1528, align 1
  %conv1.i1529 = zext i8 %119 to i32
  %add541 = add i32 %add534, %conv1.i1529
  %sub543 = add i32 %len_test420.0, -1
  %idxprom544 = zext i32 %sub543 to i64
  %arrayidx545 = getelementptr inbounds i8, i8* %buf, i64 %idxprom544
  %120 = load i8, i8* %arrayidx545, align 1
  %conv546 = zext i8 %120 to i32
  %idxprom547 = zext i32 %len_test420.0 to i64
  %arrayidx548 = getelementptr inbounds i8, i8* %add.ptr405, i64 %idxprom547
  %121 = load i8, i8* %arrayidx548, align 1
  %conv549 = zext i8 %121 to i32
  %arrayidx551 = getelementptr inbounds i8, i8* %buf, i64 %idxprom547
  %122 = load i8, i8* %arrayidx551, align 1
  %conv552 = zext i8 %122 to i32
  %123 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %literal.i1484 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %123, i64 0, i32 12
  %literal_pos_mask.i1485 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %123, i64 0, i32 11
  %124 = load i32, i32* %literal_pos_mask.i1485, align 8
  %and.i1486 = and i32 %124, %add529
  %literal_context_bits.i1487 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %123, i64 0, i32 10
  %125 = load i32, i32* %literal_context_bits.i1487, align 4
  %shl.mask.i1488 = and i32 %125, 31
  %shl.i1489 = shl i32 %and.i1486, %shl.mask.i1488
  %sub.i1490 = sub i32 8, %125
  %shl.mask2.i1491 = and i32 %sub.i1490, 31
  %shr.i1492 = lshr i32 %conv546, %shl.mask2.i1491
  %add.i1493 = add i32 %shr.i1492, %shl.i1489
  %idxprom.i1494 = zext i32 %add.i1493 to i64
  %arrayidx.i1495 = getelementptr inbounds [16 x [768 x i16]], [16 x [768 x i16]]* %literal.i1484, i64 0, i64 %idxprom.i1494
  %arraydecay.i1496 = getelementptr inbounds [768 x i16], [768 x i16]* %arrayidx.i1495, i64 0, i64 0
  %add.i.i1497 = add i32 %conv552, 256
  br label %do.body.i1525

do.body.i1525:                                    ; preds = %do.body.i1525, %if.then523
  %offset.0.i1498 = phi i32 [ %and16.i1523, %do.body.i1525 ], [ 256, %if.then523 ]
  %price.0.i1499 = phi i32 [ %add14.i1519, %do.body.i1525 ], [ 0, %if.then523 ]
  %symbol.addr.0.i1500 = phi i32 [ %shl15.i1520, %do.body.i1525 ], [ %add.i.i1497, %if.then523 ]
  %match_byte.addr.0.i1501 = phi i32 [ %shl4.i1502, %do.body.i1525 ], [ %conv549, %if.then523 ]
  %shl4.i1502 = shl i32 %match_byte.addr.0.i1501, 1
  %and5.i1503 = and i32 %shl4.i1502, %offset.0.i1498
  %shr7.i1504 = lshr i32 %symbol.addr.0.i1500, 8
  %add6.i1505 = add i32 %shr7.i1504, %offset.0.i1498
  %add8.i1506 = add i32 %add6.i1505, %and5.i1503
  %shr9.i1507 = lshr i32 %symbol.addr.0.i1500, 7
  %and10.i1508 = and i32 %shr9.i1507, 1
  %idxprom11.i1509 = zext i32 %add8.i1506 to i64
  %arrayidx12.i1510 = getelementptr inbounds i16, i16* %arraydecay.i1496, i64 %idxprom11.i1509
  %126 = load i16, i16* %arrayidx12.i1510, align 2
  %conv.i.i1511 = zext i16 %126 to i32
  %sub.i.i1512 = sub nsw i32 0, %and10.i1508
  %and.i39.i1513 = and i32 %sub.i.i1512, 2032
  %xor.i.i1514 = xor i32 %and.i39.i1513, %conv.i.i1511
  %shr.i40.i1515 = lshr i32 %xor.i.i1514, 4
  %idxprom.i41.i1516 = zext i32 %shr.i40.i1515 to i64
  %arrayidx.i42.i1517 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i41.i1516
  %127 = load i8, i8* %arrayidx.i42.i1517, align 1
  %conv1.i.i1518 = zext i8 %127 to i32
  %add14.i1519 = add i32 %price.0.i1499, %conv1.i.i1518
  %shl15.i1520 = shl i32 %symbol.addr.0.i1500, 1
  %xor.i1521 = xor i32 %shl15.i1520, -1
  %neg.i1522 = xor i32 %shl4.i1502, %xor.i1521
  %and16.i1523 = and i32 %neg.i1522, %offset.0.i1498
  %cmp.i1524 = icmp ult i32 %shl15.i1520, 65536
  br i1 %cmp.i1524, label %do.body.i1525, label %get_literal_price.exit1526

get_literal_price.exit1526:                       ; preds = %do.body.i1525
  %add554 = add i32 %add541, %add14.i1519
  %add570 = add i32 %add529, 1
  %128 = load i32, i32* %pos_mask, align 8
  %and572 = and i32 %add570, %128
  %arrayidx576 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 5
  %idxprom577 = zext i32 %and572 to i64
  %arrayidx578 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx576, i64 0, i64 %idxprom577
  %129 = load i16, i16* %arrayidx578, align 2
  %130 = lshr i16 %129, 4
  %131 = xor i16 %130, 127
  %idxprom.i1481 = zext i16 %131 to i64
  %arrayidx.i1482 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1481
  %132 = load i8, i8* %arrayidx.i1482, align 1
  %conv1.i1483 = zext i8 %132 to i32
  %add580 = add i32 %add554, %conv1.i1483
  %arrayidx583 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep, i64 0, i64 5
  %133 = load i16, i16* %arrayidx583, align 2
  %134 = lshr i16 %133, 4
  %135 = xor i16 %134, 127
  %idxprom.i1478 = zext i16 %135 to i64
  %arrayidx.i1479 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1478
  %136 = load i8, i8* %arrayidx.i1479, align 1
  %conv1.i1480 = zext i8 %136 to i32
  %add585 = add i32 %add580, %conv1.i1480
  %add588 = add i32 %add439, 1
  %add589 = add i32 %add588, %sub520
  br label %while.cond590

while.cond590:                                    ; preds = %while.body593, %get_literal_price.exit1526
  %len_end.addr.5 = phi i32 [ %len_end.addr.4, %get_literal_price.exit1526 ], [ %inc595, %while.body593 ]
  %cmp591 = icmp ult i32 %len_end.addr.5, %add589
  br i1 %cmp591, label %while.body593, label %while.end599

while.body593:                                    ; preds = %while.cond590
  %inc595 = add i32 %len_end.addr.5, 1
  %idxprom596 = zext i32 %inc595 to i64
  %arrayidx597 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom596
  %price598 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx597, i64 0, i32 5
  store i32 1073741824, i32* %price598, align 4
  br label %while.cond590

while.end599:                                     ; preds = %while.cond590
  %137 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %rep_len_encoder.i1458 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %137, i64 0, i32 23
  %prices.i.i1459 = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %rep_len_encoder.i1458, i64 0, i32 5
  %idxprom.i.i1460 = zext i32 %and572 to i64
  %arrayidx.i.i1461 = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i.i1459, i64 0, i64 %idxprom.i.i1460
  %sub.i.i1462 = add i32 %sub520, -2
  %idxprom1.i.i1463 = zext i32 %sub.i.i1462 to i64
  %arrayidx2.i.i1464 = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i.i1461, i64 0, i64 %idxprom1.i.i1463
  %138 = load i32, i32* %arrayidx2.i.i1464, align 4
  %is_rep0.i.i1465 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %137, i64 0, i32 15
  %arrayidx.i2.i1466 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep0.i.i1465, i64 0, i64 5
  %139 = load i16, i16* %arrayidx.i2.i1466, align 2
  %140 = lshr i16 %139, 4
  %idxprom.i.i.i1467 = zext i16 %140 to i64
  %arrayidx.i.i.i1468 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i.i1467
  %141 = load i8, i8* %arrayidx.i.i.i1468, align 1
  %conv1.i.i.i1469 = zext i8 %141 to i32
  %is_rep0_long.i.i1470 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %137, i64 0, i32 18
  %arrayidx2.i3.i1471 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_rep0_long.i.i1470, i64 0, i64 5
  %arrayidx4.i.i1472 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx2.i3.i1471, i64 0, i64 %idxprom.i.i1460
  %142 = load i16, i16* %arrayidx4.i.i1472, align 2
  %143 = lshr i16 %142, 4
  %144 = xor i16 %143, 127
  %idxprom.i53.i.i1473 = zext i16 %144 to i64
  %arrayidx.i54.i.i1474 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i53.i.i1473
  %145 = load i8, i8* %arrayidx.i54.i.i1474, align 1
  %conv1.i55.i.i1475 = zext i8 %145 to i32
  %add.i.i1476 = add i32 %138, %conv1.i.i.i1469
  %add.i1477 = add i32 %add.i.i1476, %conv1.i55.i.i1475
  %add602 = add i32 %add585, %add.i1477
  %idxprom604 = zext i32 %add589 to i64
  %arrayidx605 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom604
  %price606 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 5
  %146 = load i32, i32* %price606, align 4
  %cmp607 = icmp ult i32 %add602, %146
  br i1 %cmp607, label %if.then609, label %cleanup

if.then609:                                       ; preds = %while.end599
  store i32 %add602, i32* %price606, align 4
  %pos_prev619 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 6
  store i32 %add588, i32* %pos_prev619, align 4
  %back_prev623 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 7
  store i32 0, i32* %back_prev623, align 4
  %prev_1_is_literal627 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 1
  store i8 1, i8* %prev_1_is_literal627, align 4
  %prev_2631 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 2
  store i8 1, i8* %prev_2631, align 1
  %pos_prev_2635 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 3
  store i32 %cur, i32* %pos_prev_2635, align 4
  %back_prev_2639 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx605, i64 0, i32 4
  store i32 %rep_index.0, i32* %back_prev_2639, align 4
  br label %cleanup

cleanup:                                          ; preds = %for.end518, %if.then609, %while.end599, %for.body398, %lor.lhs.false
  %start_len.2 = phi i32 [ %start_len.0, %lor.lhs.false ], [ %start_len.0, %for.body398 ], [ %spec.select, %while.end599 ], [ %spec.select, %if.then609 ], [ %spec.select, %for.end518 ]
  %len_end.addr.7 = phi i32 [ %len_end.addr.3, %lor.lhs.false ], [ %len_end.addr.3, %for.body398 ], [ %len_end.addr.4, %for.end518 ], [ %len_end.addr.5, %if.then609 ], [ %len_end.addr.5, %while.end599 ]
  %inc643 = add i32 %rep_index.0, 1
  br label %for.cond394

while.cond649:                                    ; preds = %while.cond649, %for.cond.cleanup397
  %matches_count.0 = phi i32 [ 0, %for.cond.cleanup397 ], [ %inc655, %while.cond649 ]
  %matches = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 3
  %idxprom650 = zext i32 %matches_count.0 to i64
  %arrayidx651 = getelementptr inbounds [274 x %struct.lzma_match], [274 x %struct.lzma_match]* %matches, i64 0, i64 %idxprom650
  %len = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %arrayidx651, i64 0, i32 0
  %147 = load i32, i32* %len, align 4
  %cmp652 = icmp ugt i32 %60, %147
  %inc655 = add i32 %matches_count.0, 1
  br i1 %cmp652, label %while.cond649, label %while.end656

while.end656:                                     ; preds = %while.cond649
  store i32 %60, i32* %len, align 4
  br label %if.end662

if.end662:                                        ; preds = %while.end656, %for.cond.cleanup397
  %new_len.0 = phi i32 [ %60, %while.end656 ], [ %2, %for.cond.cleanup397 ]
  %matches_count.1 = phi i32 [ %inc655, %while.end656 ], [ %1, %for.cond.cleanup397 ]
  %cmp663 = icmp ult i32 %new_len.0, %start_len.0
  br i1 %cmp663, label %cleanup913, label %if.then665

if.then665:                                       ; preds = %if.end662
  %148 = load i16, i16* %arrayidx225, align 2
  %149 = lshr i16 %148, 4
  %idxprom.i1455 = zext i16 %149 to i64
  %arrayidx.i1456 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1455
  %150 = load i8, i8* %arrayidx.i1456, align 1
  %conv1.i1457 = zext i8 %150 to i32
  %add670 = add i32 %add223, %conv1.i1457
  br label %while.cond671

while.cond671:                                    ; preds = %while.body675, %if.then665
  %len_end.addr.8 = phi i32 [ %len_end.addr.3, %if.then665 ], [ %inc677, %while.body675 ]
  %add672 = add i32 %new_len.0, %cur
  %cmp673 = icmp ult i32 %len_end.addr.8, %add672
  br i1 %cmp673, label %while.body675, label %while.cond683

while.body675:                                    ; preds = %while.cond671
  %inc677 = add i32 %len_end.addr.8, 1
  %idxprom678 = zext i32 %inc677 to i64
  %arrayidx679 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom678
  %price680 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx679, i64 0, i32 5
  store i32 1073741824, i32* %price680, align 4
  br label %while.cond671

while.cond683:                                    ; preds = %while.cond671, %while.body690
  %i682.0 = phi i32 [ %inc691, %while.body690 ], [ 0, %while.cond671 ]
  %matches684 = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %0, i64 0, i32 3
  %idxprom685 = zext i32 %i682.0 to i64
  %arrayidx686 = getelementptr inbounds [274 x %struct.lzma_match], [274 x %struct.lzma_match]* %matches684, i64 0, i64 %idxprom685
  %len687 = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %arrayidx686, i64 0, i32 0
  %151 = load i32, i32* %len687, align 4
  %cmp688 = icmp ugt i32 %start_len.0, %151
  br i1 %cmp688, label %while.body690, label %for.cond694

while.body690:                                    ; preds = %while.cond683
  %inc691 = add i32 %i682.0, 1
  br label %while.cond683

for.cond694:                                      ; preds = %while.cond683, %for.inc906
  %i682.1 = phi i32 [ %i682.31425, %for.inc906 ], [ %i682.0, %while.cond683 ]
  %len_test693.0 = phi i32 [ %inc907, %for.inc906 ], [ %start_len.0, %while.cond683 ]
  %len_end.addr.9 = phi i32 [ %len_end.addr.131426, %for.inc906 ], [ %len_end.addr.8, %while.cond683 ]
  %idxprom696 = zext i32 %i682.1 to i64
  %arrayidx697 = getelementptr inbounds [274 x %struct.lzma_match], [274 x %struct.lzma_match]* %matches684, i64 0, i64 %idxprom696
  %dist = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %arrayidx697, i64 0, i32 1
  %152 = load i32, i32* %dist, align 4
  %153 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %cmp.i1441 = icmp ult i32 %len_test693.0, 6
  %sub.i1442 = add i32 %len_test693.0, -2
  %154 = select i1 %cmp.i1441, i32 %sub.i1442, i32 3
  %cmp1.i = icmp ult i32 %152, 128
  br i1 %cmp1.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %for.cond694
  %distances_prices.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %153, i64 0, i32 25
  %idxprom.i1443 = zext i32 %154 to i64
  %arrayidx.i1444 = getelementptr inbounds [4 x [128 x i32]], [4 x [128 x i32]]* %distances_prices.i, i64 0, i64 %idxprom.i1443
  %idxprom2.i = zext i32 %152 to i64
  %arrayidx3.i = getelementptr inbounds [128 x i32], [128 x i32]* %arrayidx.i1444, i64 0, i64 %idxprom2.i
  %155 = load i32, i32* %arrayidx3.i, align 4
  br label %get_pos_len_price.exit

if.else.i:                                        ; preds = %for.cond694
  %cmp.i.i = icmp ult i32 %152, 524288
  br i1 %cmp.i.i, label %if.then.i.i, label %if.end.i.i

if.then.i.i:                                      ; preds = %if.else.i
  %shr.i.i = lshr i32 %152, 6
  %idxprom.i.i1445 = zext i32 %shr.i.i to i64
  %arrayidx.i.i1446 = getelementptr inbounds [8192 x i8], [8192 x i8]* @lzma_fastpos, i64 0, i64 %idxprom.i.i1445
  %156 = load i8, i8* %arrayidx.i.i1446, align 1
  %conv.i.i1447 = zext i8 %156 to i64
  %add.i.i1448 = add nuw nsw i64 %conv.i.i1447, 12
  br label %get_pos_slot_2.exit.i

if.end.i.i:                                       ; preds = %if.else.i
  %cmp1.i.i = icmp sgt i32 %152, -1
  br i1 %cmp1.i.i, label %if.then3.i.i, label %if.end9.i.i

if.then3.i.i:                                     ; preds = %if.end.i.i
  %shr4.i.i = lshr i32 %152, 18
  %idxprom5.i.i = zext i32 %shr4.i.i to i64
  %arrayidx6.i.i = getelementptr inbounds [8192 x i8], [8192 x i8]* @lzma_fastpos, i64 0, i64 %idxprom5.i.i
  %157 = load i8, i8* %arrayidx6.i.i, align 1
  %conv7.i.i = zext i8 %157 to i64
  %add8.i.i = add nuw nsw i64 %conv7.i.i, 36
  br label %get_pos_slot_2.exit.i

if.end9.i.i:                                      ; preds = %if.end.i.i
  %shr10.i.i = lshr i32 %152, 30
  %idxprom11.i.i = zext i32 %shr10.i.i to i64
  %arrayidx12.i.i = getelementptr inbounds [8192 x i8], [8192 x i8]* @lzma_fastpos, i64 0, i64 %idxprom11.i.i
  %158 = load i8, i8* %arrayidx12.i.i, align 1
  %conv13.i.i = zext i8 %158 to i64
  %add14.i.i = add nuw nsw i64 %conv13.i.i, 60
  br label %get_pos_slot_2.exit.i

get_pos_slot_2.exit.i:                            ; preds = %if.end9.i.i, %if.then3.i.i, %if.then.i.i
  %retval.0.i.i = phi i64 [ %add.i.i1448, %if.then.i.i ], [ %add8.i.i, %if.then3.i.i ], [ %add14.i.i, %if.end9.i.i ]
  %pos_slot_prices.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %153, i64 0, i32 24
  %idxprom4.i = zext i32 %154 to i64
  %arrayidx5.i = getelementptr inbounds [4 x [64 x i32]], [4 x [64 x i32]]* %pos_slot_prices.i, i64 0, i64 %idxprom4.i
  %idxprom6.i = and i64 %retval.0.i.i, 4294967295
  %arrayidx7.i = getelementptr inbounds [64 x i32], [64 x i32]* %arrayidx5.i, i64 0, i64 %idxprom6.i
  %159 = load i32, i32* %arrayidx7.i, align 4
  %align_prices.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %153, i64 0, i32 28
  %and.i1449 = and i32 %152, 15
  %idxprom8.i = zext i32 %and.i1449 to i64
  %arrayidx9.i = getelementptr inbounds [16 x i32], [16 x i32]* %align_prices.i, i64 0, i64 %idxprom8.i
  %160 = load i32, i32* %arrayidx9.i, align 4
  %add.i1450 = add i32 %160, %159
  br label %get_pos_len_price.exit

get_pos_len_price.exit:                           ; preds = %if.then.i, %get_pos_slot_2.exit.i
  %price.0.i1451 = phi i32 [ %155, %if.then.i ], [ %add.i1450, %get_pos_slot_2.exit.i ]
  %match_len_encoder.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %153, i64 0, i32 22
  %prices.i.i1452 = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %match_len_encoder.i, i64 0, i32 5
  %idxprom.i26.i = zext i32 %and to i64
  %arrayidx.i27.i = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i.i1452, i64 0, i64 %idxprom.i26.i
  %idxprom1.i.i1453 = zext i32 %sub.i1442 to i64
  %arrayidx2.i.i1454 = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i27.i, i64 0, i64 %idxprom1.i.i1453
  %161 = load i32, i32* %arrayidx2.i.i1454, align 4
  %add11.i = add i32 %161, %price.0.i1451
  %add700 = add i32 %add670, %add11.i
  %add702 = add i32 %len_test693.0, %cur
  %idxprom703 = zext i32 %add702 to i64
  %arrayidx704 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom703
  %price705 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx704, i64 0, i32 5
  %162 = load i32, i32* %price705, align 4
  %cmp706 = icmp ult i32 %add700, %162
  br i1 %cmp706, label %if.then708, label %if.end730

if.then708:                                       ; preds = %get_pos_len_price.exit
  store i32 %add700, i32* %price705, align 4
  %pos_prev718 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx704, i64 0, i32 6
  store i32 %cur, i32* %pos_prev718, align 4
  %add719 = add i32 %152, 4
  %back_prev724 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx704, i64 0, i32 7
  store i32 %add719, i32* %back_prev724, align 4
  %prev_1_is_literal729 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx704, i64 0, i32 1
  store i8 0, i8* %prev_1_is_literal729, align 4
  br label %if.end730

if.end730:                                        ; preds = %if.then708, %get_pos_len_price.exit
  %len734 = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %arrayidx697, i64 0, i32 0
  %163 = load i32, i32* %len734, align 4
  %cmp735 = icmp eq i32 %len_test693.0, %163
  br i1 %cmp735, label %if.then737, label %for.inc906

if.then737:                                       ; preds = %if.end730
  %idx.ext739 = zext i32 %152 to i64
  %idx.neg740 = sub nsw i64 0, %idx.ext739
  %add.ptr741 = getelementptr inbounds i8, i8* %buf, i64 %idx.neg740
  %add.ptr742 = getelementptr inbounds i8, i8* %add.ptr741, i64 -1
  %add744 = add i32 %len_test693.0, 1
  %add746 = add i32 %add744, %nice_len
  %cmp747 = icmp ugt i32 %add746, %buf_avail_full
  %buf_avail_full.add746 = select i1 %cmp747, i32 %buf_avail_full, i32 %add746
  br label %for.cond754

for.cond754:                                      ; preds = %for.inc768, %if.then737
  %len_test_2743.0 = phi i32 [ %add744, %if.then737 ], [ %inc769, %for.inc768 ]
  %cmp755 = icmp ult i32 %len_test_2743.0, %buf_avail_full.add746
  br i1 %cmp755, label %land.rhs757, label %for.end770

land.rhs757:                                      ; preds = %for.cond754
  %idxprom758 = zext i32 %len_test_2743.0 to i64
  %arrayidx759 = getelementptr inbounds i8, i8* %buf, i64 %idxprom758
  %164 = load i8, i8* %arrayidx759, align 1
  %arrayidx762 = getelementptr inbounds i8, i8* %add.ptr742, i64 %idxprom758
  %165 = load i8, i8* %arrayidx762, align 1
  %cmp764 = icmp eq i8 %164, %165
  br i1 %cmp764, label %for.inc768, label %for.end770

for.inc768:                                       ; preds = %land.rhs757
  %inc769 = add i32 %len_test_2743.0, 1
  br label %for.cond754

for.end770:                                       ; preds = %for.cond754, %land.rhs757
  %sub772 = sub i32 %len_test_2743.0, %add744
  %cmp773 = icmp ugt i32 %sub772, 1
  br i1 %cmp773, label %if.then775, label %if.end890

if.then775:                                       ; preds = %for.end770
  %cond779 = select i1 %cmp188, i64 7, i64 10
  %add781 = add i32 %len_test693.0, %position
  %166 = load i32, i32* %pos_mask, align 8
  %and783 = and i32 %add781, %166
  %arrayidx787 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 %cond779
  %idxprom788 = zext i32 %and783 to i64
  %arrayidx789 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx787, i64 0, i64 %idxprom788
  %167 = load i16, i16* %arrayidx789, align 2
  %168 = lshr i16 %167, 4
  %idxprom.i1438 = zext i16 %168 to i64
  %arrayidx.i1439 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1438
  %169 = load i8, i8* %arrayidx.i1439, align 1
  %conv1.i1440 = zext i8 %169 to i32
  %add791 = add i32 %add700, %conv1.i1440
  %sub793 = add i32 %len_test693.0, -1
  %idxprom794 = zext i32 %sub793 to i64
  %arrayidx795 = getelementptr inbounds i8, i8* %buf, i64 %idxprom794
  %170 = load i8, i8* %arrayidx795, align 1
  %conv796 = zext i8 %170 to i32
  %idxprom797 = zext i32 %len_test693.0 to i64
  %arrayidx798 = getelementptr inbounds i8, i8* %add.ptr742, i64 %idxprom797
  %171 = load i8, i8* %arrayidx798, align 1
  %conv799 = zext i8 %171 to i32
  %arrayidx801 = getelementptr inbounds i8, i8* %buf, i64 %idxprom797
  %172 = load i8, i8* %arrayidx801, align 1
  %conv802 = zext i8 %172 to i32
  %173 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %literal.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %173, i64 0, i32 12
  %literal_pos_mask.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %173, i64 0, i32 11
  %174 = load i32, i32* %literal_pos_mask.i, align 8
  %and.i = and i32 %174, %add781
  %literal_context_bits.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %173, i64 0, i32 10
  %175 = load i32, i32* %literal_context_bits.i, align 4
  %shl.mask.i = and i32 %175, 31
  %shl.i = shl i32 %and.i, %shl.mask.i
  %sub.i = sub i32 8, %175
  %shl.mask2.i = and i32 %sub.i, 31
  %shr.i = lshr i32 %conv796, %shl.mask2.i
  %add.i1433 = add i32 %shr.i, %shl.i
  %idxprom.i1434 = zext i32 %add.i1433 to i64
  %arrayidx.i1435 = getelementptr inbounds [16 x [768 x i16]], [16 x [768 x i16]]* %literal.i, i64 0, i64 %idxprom.i1434
  %arraydecay.i = getelementptr inbounds [768 x i16], [768 x i16]* %arrayidx.i1435, i64 0, i64 0
  %add.i.i1436 = add i32 %conv802, 256
  br label %do.body.i

do.body.i:                                        ; preds = %do.body.i, %if.then775
  %offset.0.i = phi i32 [ %and16.i, %do.body.i ], [ 256, %if.then775 ]
  %price.0.i = phi i32 [ %add14.i, %do.body.i ], [ 0, %if.then775 ]
  %symbol.addr.0.i = phi i32 [ %shl15.i, %do.body.i ], [ %add.i.i1436, %if.then775 ]
  %match_byte.addr.0.i = phi i32 [ %shl4.i, %do.body.i ], [ %conv799, %if.then775 ]
  %shl4.i = shl i32 %match_byte.addr.0.i, 1
  %and5.i = and i32 %shl4.i, %offset.0.i
  %shr7.i = lshr i32 %symbol.addr.0.i, 8
  %add6.i = add i32 %shr7.i, %offset.0.i
  %add8.i = add i32 %add6.i, %and5.i
  %shr9.i = lshr i32 %symbol.addr.0.i, 7
  %and10.i = and i32 %shr9.i, 1
  %idxprom11.i = zext i32 %add8.i to i64
  %arrayidx12.i = getelementptr inbounds i16, i16* %arraydecay.i, i64 %idxprom11.i
  %176 = load i16, i16* %arrayidx12.i, align 2
  %conv.i.i = zext i16 %176 to i32
  %sub.i.i1437 = sub nsw i32 0, %and10.i
  %and.i39.i = and i32 %sub.i.i1437, 2032
  %xor.i.i = xor i32 %and.i39.i, %conv.i.i
  %shr.i40.i = lshr i32 %xor.i.i, 4
  %idxprom.i41.i = zext i32 %shr.i40.i to i64
  %arrayidx.i42.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i41.i
  %177 = load i8, i8* %arrayidx.i42.i, align 1
  %conv1.i.i = zext i8 %177 to i32
  %add14.i = add i32 %price.0.i, %conv1.i.i
  %shl15.i = shl i32 %symbol.addr.0.i, 1
  %xor.i = xor i32 %shl15.i, -1
  %neg.i = xor i32 %shl4.i, %xor.i
  %and16.i = and i32 %neg.i, %offset.0.i
  %cmp.i = icmp ult i32 %shl15.i, 65536
  br i1 %cmp.i, label %do.body.i, label %get_literal_price.exit

get_literal_price.exit:                           ; preds = %do.body.i
  %add804 = add i32 %add791, %add14.i
  %add819 = add i32 %and783, 1
  %178 = load i32, i32* %pos_mask, align 8
  %and821 = and i32 %add819, %178
  %arrayidx825 = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_match, i64 0, i64 4
  %idxprom826 = zext i32 %and821 to i64
  %arrayidx827 = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx825, i64 0, i64 %idxprom826
  %179 = load i16, i16* %arrayidx827, align 2
  %180 = lshr i16 %179, 4
  %181 = xor i16 %180, 127
  %idxprom.i1430 = zext i16 %181 to i64
  %arrayidx.i1431 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1430
  %182 = load i8, i8* %arrayidx.i1431, align 1
  %conv1.i1432 = zext i8 %182 to i32
  %add829 = add i32 %add804, %conv1.i1432
  %arrayidx832 = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep, i64 0, i64 4
  %183 = load i16, i16* %arrayidx832, align 2
  %184 = lshr i16 %183, 4
  %185 = xor i16 %184, 127
  %idxprom.i1427 = zext i16 %185 to i64
  %arrayidx.i1428 = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i1427
  %186 = load i8, i8* %arrayidx.i1428, align 1
  %conv1.i1429 = zext i8 %186 to i32
  %add834 = add i32 %add829, %conv1.i1429
  %add837 = add i32 %add702, 1
  %add838 = add i32 %add837, %sub772
  br label %while.cond839

while.cond839:                                    ; preds = %while.body842, %get_literal_price.exit
  %len_end.addr.10 = phi i32 [ %len_end.addr.9, %get_literal_price.exit ], [ %inc844, %while.body842 ]
  %cmp840 = icmp ult i32 %len_end.addr.10, %add838
  br i1 %cmp840, label %while.body842, label %while.end848

while.body842:                                    ; preds = %while.cond839
  %inc844 = add i32 %len_end.addr.10, 1
  %idxprom845 = zext i32 %inc844 to i64
  %arrayidx846 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom845
  %price847 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx846, i64 0, i32 5
  store i32 1073741824, i32* %price847, align 4
  br label %while.cond839

while.end848:                                     ; preds = %while.cond839
  %187 = bitcast i8* %pcoder to %struct.lzma_coder_s*
  %rep_len_encoder.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %187, i64 0, i32 23
  %prices.i.i = getelementptr inbounds %struct.lzma_length_encoder, %struct.lzma_length_encoder* %rep_len_encoder.i, i64 0, i32 5
  %idxprom.i.i = zext i32 %and821 to i64
  %arrayidx.i.i = getelementptr inbounds [16 x [272 x i32]], [16 x [272 x i32]]* %prices.i.i, i64 0, i64 %idxprom.i.i
  %sub.i.i = add i32 %sub772, -2
  %idxprom1.i.i = zext i32 %sub.i.i to i64
  %arrayidx2.i.i = getelementptr inbounds [272 x i32], [272 x i32]* %arrayidx.i.i, i64 0, i64 %idxprom1.i.i
  %188 = load i32, i32* %arrayidx2.i.i, align 4
  %is_rep0.i.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %187, i64 0, i32 15
  %arrayidx.i2.i = getelementptr inbounds [12 x i16], [12 x i16]* %is_rep0.i.i, i64 0, i64 4
  %189 = load i16, i16* %arrayidx.i2.i, align 2
  %190 = lshr i16 %189, 4
  %idxprom.i.i.i = zext i16 %190 to i64
  %arrayidx.i.i.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i.i.i
  %191 = load i8, i8* %arrayidx.i.i.i, align 1
  %conv1.i.i.i = zext i8 %191 to i32
  %is_rep0_long.i.i = getelementptr inbounds %struct.lzma_coder_s, %struct.lzma_coder_s* %187, i64 0, i32 18
  %arrayidx2.i3.i = getelementptr inbounds [12 x [16 x i16]], [12 x [16 x i16]]* %is_rep0_long.i.i, i64 0, i64 4
  %arrayidx4.i.i = getelementptr inbounds [16 x i16], [16 x i16]* %arrayidx2.i3.i, i64 0, i64 %idxprom.i.i
  %192 = load i16, i16* %arrayidx4.i.i, align 2
  %193 = lshr i16 %192, 4
  %194 = xor i16 %193, 127
  %idxprom.i53.i.i = zext i16 %194 to i64
  %arrayidx.i54.i.i = getelementptr inbounds [128 x i8], [128 x i8]* @lzma_rc_prices, i64 0, i64 %idxprom.i53.i.i
  %195 = load i8, i8* %arrayidx.i54.i.i, align 1
  %conv1.i55.i.i = zext i8 %195 to i32
  %add.i.i = add i32 %188, %conv1.i.i.i
  %add.i = add i32 %add.i.i, %conv1.i55.i.i
  %add850 = add i32 %add834, %add.i
  %idxprom852 = zext i32 %add838 to i64
  %arrayidx853 = getelementptr inbounds [4096 x %struct.lzma_optimal], [4096 x %struct.lzma_optimal]* %opts, i64 0, i64 %idxprom852
  %price854 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 5
  %196 = load i32, i32* %price854, align 4
  %cmp855 = icmp ult i32 %add850, %196
  br i1 %cmp855, label %if.then857, label %if.end890

if.then857:                                       ; preds = %while.end848
  store i32 %add850, i32* %price854, align 4
  %pos_prev867 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 6
  store i32 %add837, i32* %pos_prev867, align 4
  %back_prev871 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 7
  store i32 0, i32* %back_prev871, align 4
  %prev_1_is_literal875 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 1
  store i8 1, i8* %prev_1_is_literal875, align 4
  %prev_2879 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 2
  store i8 1, i8* %prev_2879, align 1
  %pos_prev_2883 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 3
  store i32 %cur, i32* %pos_prev_2883, align 4
  %add884 = add i32 %152, 4
  %back_prev_2888 = getelementptr inbounds %struct.lzma_optimal, %struct.lzma_optimal* %arrayidx853, i64 0, i32 4
  store i32 %add884, i32* %back_prev_2888, align 4
  br label %if.end890

if.end890:                                        ; preds = %while.end848, %if.then857, %for.end770
  %len_end.addr.11 = phi i32 [ %len_end.addr.9, %for.end770 ], [ %len_end.addr.10, %if.then857 ], [ %len_end.addr.10, %while.end848 ]
  %inc891 = add i32 %i682.1, 1
  %cmp892 = icmp eq i32 %inc891, %matches_count.1
  br i1 %cmp892, label %cleanup913, label %for.inc906

for.inc906:                                       ; preds = %if.end890, %if.end730
  %len_end.addr.131426 = phi i32 [ %len_end.addr.9, %if.end730 ], [ %len_end.addr.11, %if.end890 ]
  %i682.31425 = phi i32 [ %i682.1, %if.end730 ], [ %inc891, %if.end890 ]
  %inc907 = add i32 %len_test693.0, 1
  br label %for.cond694

cleanup913:                                       ; preds = %if.end662, %if.end890, %if.end274
  %retval.0 = phi i32 [ %len_end, %if.end274 ], [ %len_end.addr.3, %if.end662 ], [ %len_end.addr.11, %if.end890 ]
  ret i32 %retval.0
}

@buffer = common dso_local global i8* null, align 8
@mypointer = common dso_local global i32* null, align 8

define dso_local i32 @main() {
  %t0 = load i8*, i8** @buffer, align 8
  %t1 = load i32*, i32** @mypointer, align 8
  %t2 = call i32 @myhelper2(i8* %t0, i32* %t1, i8* %t0, i32 1, i32 2, i32 3, i32 4, i32 5)
  ret i32 %t2;
}

; CHECK-OLD: COMPILE FUNC: myhelper2
; CHECK-OLD: COMPILE FUNC: main
; CHECK-OLD: myhelper2{{.*}}Inlining is not profitable
; CHECK-OLD: {{.*}}call i32 @myhelper2

; CHECK-NEW: {{.*}}call i32 @myhelper2
; CHECK-NEW: COMPILE FUNC: myhelper2
; CHECK-NEW: COMPILE FUNC: main
; CHECK-NEW: myhelper2{{.*}}Inlining is not profitable

; end INTEL_FEATURE_SW_ADVANCED
