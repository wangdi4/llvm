; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, asserts
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CLASSIC %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-META %s

; Check that core_state_transition gets inlined into core_bench_state, by
; the early test loop fusion inlining heuristic.

; CHECK-CLASSIC: define{{.*}}@core_bench_state
; CHECK-CLASSIC-NOT: call{{.*}}core_state_transition
; CHECK-CLASSIC: define{{.*}}@core_state_transition

; CHECK-CLASSIC: COMPILE FUNC: core_state_transition
; CHECK: COMPILE FUNC: core_bench_state
; CHECK: INLINE: core_state_transition{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK: INLINE: core_state_transition{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-META: COMPILE FUNC: core_state_transition

; CHECK-META: define{{.*}}@core_bench_state
; CHECK-META-NOT: call{{.*}}core_state_transition
; CHECK-META: define{{.*}}@core_state_transition

declare dso_local zeroext i16 @crcu32(i32, i16 zeroext)

define dso_local zeroext i16 @core_bench_state(i32 %blksize, ptr %memblock, i16 signext %seed1, i16 signext %seed2, i16 signext %step, i16 zeroext %crc) local_unnamed_addr #0 {
entry:
  %final_counts = alloca [8 x i32], align 16
  %track_counts = alloca [8 x i32], align 16
  %p = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %final_counts)
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %track_counts)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %p)
  store ptr %memblock, ptr %p, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %i.0, 8
  br i1 %cmp, label %for.body, label %while.cond

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [8 x i32], ptr %track_counts, i64 0, i64 %idxprom
  store i32 0, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [8 x i32], ptr %final_counts, i64 0, i64 %idxprom
  store i32 0, ptr %arrayidx2, align 4
  %inc = add i32 %i.0, 1
  br label %for.cond

while.cond:                                       ; preds = %while.body, %for.cond
  %i3 = load ptr, ptr %p, align 8
  %i4 = load i8, ptr %i3, align 1
  %cmp3.not = icmp eq i8 %i4, 0
  br i1 %cmp3.not, label %while.cond8, label %while.body

while.body:                                       ; preds = %while.cond
  %arraydecay = getelementptr inbounds [8 x i32], ptr %track_counts, i64 0, i64 0
  %call = call i32 @core_state_transition(ptr nonnull %p, ptr nonnull %arraydecay)
  %idxprom5 = zext i32 %call to i64
  %arrayidx6 = getelementptr inbounds [8 x i32], ptr %final_counts, i64 0, i64 %idxprom5
  %i5 = load i32, ptr %arrayidx6, align 4
  %inc7 = add i32 %i5, 1
  store i32 %inc7, ptr %arrayidx6, align 4
  br label %while.cond

while.cond8:                                      ; preds = %if.end, %while.cond
  %storemerge = phi ptr [ %add.ptr21, %if.end ], [ %memblock, %while.cond ]
  store ptr %storemerge, ptr %p, align 8
  %idx.ext = zext i32 %blksize to i64
  %add.ptr = getelementptr inbounds i8, ptr %memblock, i64 %idx.ext
  %cmp9 = icmp ult ptr %storemerge, %add.ptr
  br i1 %cmp9, label %while.body11, label %while.end22

while.body11:                                     ; preds = %while.cond8
  %i6 = load i8, ptr %storemerge, align 1
  %cmp13.not = icmp eq i8 %i6, 44
  br i1 %cmp13.not, label %if.end, label %if.then

if.then:                                          ; preds = %while.body11
  %conv16 = trunc i16 %seed1 to i8
  %xor = xor i8 %i6, %conv16
  store i8 %xor, ptr %storemerge, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %while.body11
  %i7 = load ptr, ptr %p, align 8
  %idx.ext20 = sext i16 %step to i64
  %add.ptr21 = getelementptr inbounds i8, ptr %i7, i64 %idx.ext20
  br label %while.cond8

while.end22:                                      ; preds = %while.cond8
  store ptr %memblock, ptr %p, align 8
  br label %while.cond23

while.cond23:                                     ; preds = %while.body27, %while.end22
  %i8 = load ptr, ptr %p, align 8
  %i9 = load i8, ptr %i8, align 1
  %cmp25.not = icmp eq i8 %i9, 0
  br i1 %cmp25.not, label %while.cond35, label %while.body27

while.body27:                                     ; preds = %while.cond23
  %arraydecay29 = getelementptr inbounds [8 x i32], ptr %track_counts, i64 0, i64 0
  %call30 = call i32 @core_state_transition(ptr nonnull %p, ptr nonnull %arraydecay29)
  %idxprom31 = zext i32 %call30 to i64
  %arrayidx32 = getelementptr inbounds [8 x i32], ptr %final_counts, i64 0, i64 %idxprom31
  %i10 = load i32, ptr %arrayidx32, align 4
  %inc33 = add i32 %i10, 1
  store i32 %inc33, ptr %arrayidx32, align 4
  br label %while.cond23

while.cond35:                                     ; preds = %if.end50, %while.cond23
  %storemerge88 = phi ptr [ %add.ptr53, %if.end50 ], [ %memblock, %while.cond23 ]
  store ptr %storemerge88, ptr %p, align 8
  %cmp38 = icmp ult ptr %storemerge88, %add.ptr
  br i1 %cmp38, label %while.body40, label %for.cond55

while.body40:                                     ; preds = %while.cond35
  %i11 = load i8, ptr %storemerge88, align 1
  %cmp42.not = icmp eq i8 %i11, 44
  br i1 %cmp42.not, label %if.end50, label %if.then44

if.then44:                                        ; preds = %while.body40
  %conv46 = trunc i16 %seed2 to i8
  %xor48 = xor i8 %i11, %conv46
  store i8 %xor48, ptr %storemerge88, align 1
  br label %if.end50

if.end50:                                         ; preds = %if.then44, %while.body40
  %i12 = load ptr, ptr %p, align 8
  %idx.ext52 = sext i16 %step to i64
  %add.ptr53 = getelementptr inbounds i8, ptr %i12, i64 %idx.ext52
  br label %while.cond35

for.cond55:                                       ; preds = %for.body58, %while.cond35
  %i.1 = phi i32 [ %inc66, %for.body58 ], [ 0, %while.cond35 ]
  %crc.addr.0 = phi i16 [ %call64, %for.body58 ], [ %crc, %while.cond35 ]
  %cmp56 = icmp ult i32 %i.1, 8
  br i1 %cmp56, label %for.body58, label %for.end67

for.body58:                                       ; preds = %for.cond55
  %idxprom59 = zext i32 %i.1 to i64
  %arrayidx60 = getelementptr inbounds [8 x i32], ptr %final_counts, i64 0, i64 %idxprom59
  %i13 = load i32, ptr %arrayidx60, align 4
  %call61 = call zeroext i16 @crcu32(i32 %i13, i16 zeroext %crc.addr.0)
  %arrayidx63 = getelementptr inbounds [8 x i32], ptr %track_counts, i64 0, i64 %idxprom59
  %i14 = load i32, ptr %arrayidx63, align 4
  %call64 = call zeroext i16 @crcu32(i32 %i14, i16 zeroext %call61)
  %inc66 = add i32 %i.1, 1
  br label %for.cond55

for.end67:                                        ; preds = %for.cond55
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %p)
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %track_counts)
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %final_counts)
  ret i16 %crc.addr.0
}

define dso_local i32 @core_state_transition(ptr nocapture %instr, ptr nocapture %transition_count) local_unnamed_addr #0 {
for.cond.thread:
  %i = load ptr, ptr %instr, align 8
  %i1 = load i8, ptr %i, align 1
  switch i8 %i1, label %sw.bb [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond.outer:                                   ; preds = %if.else
  %i2 = load i32, ptr %transition_count, align 4
  %inc24 = add i32 %i2, 1
  store i32 %inc24, ptr %transition_count, align 4
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %sw.bb102, %sw.bb92, %if.else88, %if.then72, %if.then53, %if.else31, %for.cond.outer
  %str.0158.ph341 = phi ptr [ %i, %for.cond.outer ], [ %incdec.ptr107237, %sw.bb92 ], [ %incdec.ptr107.us281, %if.else31 ], [ %str.0194, %if.then53 ], [ %str.0175, %if.then72 ], [ %incdec.ptr107224, %if.else88 ], [ %incdec.ptr107209, %sw.bb102 ]
  br label %for.cond

for.cond.us279:                                   ; preds = %if.else, %if.else
  %i3 = load i32, ptr %transition_count, align 4
  %inc24346 = add i32 %i3, 1
  store i32 %inc24346, ptr %transition_count, align 4
  %incdec.ptr107.us281 = getelementptr inbounds i8, ptr %i, i64 1
  %i4 = load i8, ptr %incdec.ptr107.us281, align 1
  switch i8 %i4, label %sw.bb25 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond.us287:                                   ; preds = %sw.bb
  %i5 = load i32, ptr %transition_count, align 4
  %inc24351 = add i32 %i5, 1
  store i32 %inc24351, ptr %transition_count, align 4
  %incdec.ptr107.us289 = getelementptr inbounds i8, ptr %i, i64 1
  %i6 = load i8, ptr %incdec.ptr107.us289, align 1
  switch i8 %i6, label %sw.bb43 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond.us295:                                   ; preds = %if.else
  %i7 = load i32, ptr %transition_count, align 4
  %inc24356 = add i32 %i7, 1
  store i32 %inc24356, ptr %transition_count, align 4
  %incdec.ptr107.us297 = getelementptr inbounds i8, ptr %i, i64 1
  %i8 = load i8, ptr %incdec.ptr107.us297, align 1
  switch i8 %i8, label %sw.bb58 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond:                                         ; preds = %for.body, %for.cond.preheader
  %str.0158 = phi ptr [ %str.0158.ph341, %for.cond.preheader ], [ %incdec.ptr107, %for.body ]
  %incdec.ptr107 = getelementptr inbounds i8, ptr %str.0158, i64 1
  br i1 false, label %for.body, label %if.then110

for.body:                                         ; preds = %for.cond
  br i1 undef, label %if.then, label %for.cond

if.then:                                          ; preds = %for.cond.thread201, %for.cond.thread184, %for.cond.thread166, %for.cond.thread229, %for.cond.thread216, %for.body, %for.cond.us295, %for.cond.us287, %for.cond.us279, %for.cond.thread
  %state.0161 = phi i32 [ 0, %for.cond.thread ], [ 4, %for.cond.thread184 ], [ 5, %for.cond.thread166 ], [ 6, %for.cond.thread229 ], [ 3, %for.cond.thread216 ], [ 2, %for.cond.us279 ], [ 4, %for.cond.us287 ], [ 5, %for.cond.us295 ], [ 1, %for.body ], [ 7, %for.cond.thread201 ]
  %str.0160 = phi ptr [ %i, %for.cond.thread ], [ %incdec.ptr107192, %for.cond.thread184 ], [ %incdec.ptr107173, %for.cond.thread166 ], [ %incdec.ptr107237, %for.cond.thread229 ], [ %incdec.ptr107224, %for.cond.thread216 ], [ %incdec.ptr107.us281, %for.cond.us279 ], [ %incdec.ptr107.us289, %for.cond.us287 ], [ %incdec.ptr107.us297, %for.cond.us295 ], [ %incdec.ptr107, %for.body ], [ %incdec.ptr107209, %for.cond.thread201 ]
  %idxprom = zext i32 %state.0161 to i64
  %arrayidx = getelementptr inbounds i32, ptr %transition_count, i64 %idxprom
  %i9 = load i32, ptr %arrayidx, align 4
  %inc = add i32 %i9, 1
  store i32 %inc, ptr %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i8, ptr %str.0160, i64 1
  br label %if.end113

sw.bb:                                            ; preds = %for.cond.thread
  %i10 = getelementptr inbounds i32, ptr %transition_count, i64 2
  %arrayidx54 = getelementptr inbounds i32, ptr %transition_count, i64 4
  %arrayidx67 = getelementptr inbounds i32, ptr %transition_count, i64 5
  %arrayidx86 = getelementptr inbounds i32, ptr %transition_count, i64 3
  %i11 = getelementptr inbounds i32, ptr %transition_count, i64 6
  %c.off.i = add i8 %i1, -48
  %i12 = icmp ugt i8 %c.off.i, 9
  br i1 %i12, label %if.else, label %for.cond.us287

if.else:                                          ; preds = %sw.bb
  switch i8 %i1, label %for.cond.outer [
    i8 43, label %for.cond.us279
    i8 45, label %for.cond.us279
    i8 46, label %for.cond.us295
  ]

sw.bb25:                                          ; preds = %for.cond.us279
  %c.off.i252 = add i8 %i4, -48
  %i13 = icmp ugt i8 %c.off.i252, 9
  %i14 = load i32, ptr %i10, align 4
  %inc30 = add i32 %i14, 1
  store i32 %inc30, ptr %i10, align 4
  br i1 %i13, label %if.else31, label %for.cond.thread184

if.else31:                                        ; preds = %sw.bb25
  %cmp33 = icmp eq i8 %i4, 46
  br i1 %cmp33, label %for.cond.thread166, label %for.cond.preheader

sw.bb43:                                          ; preds = %for.cond.thread184, %for.cond.us287
  %i15 = phi i8 [ %i31, %for.cond.thread184 ], [ %i6, %for.cond.us287 ]
  %str.0194 = phi ptr [ %incdec.ptr107192, %for.cond.thread184 ], [ %incdec.ptr107.us289, %for.cond.us287 ]
  %cmp45 = icmp eq i8 %i15, 46
  br i1 %cmp45, label %if.then47, label %if.else50

if.then47:                                        ; preds = %sw.bb43
  %i16 = load i32, ptr %arrayidx54, align 4
  %inc49 = add i32 %i16, 1
  store i32 %inc49, ptr %arrayidx54, align 4
  br label %for.cond.thread166

if.else50:                                        ; preds = %sw.bb43
  %c.off.i250 = add i8 %i15, -48
  %i17 = icmp ugt i8 %c.off.i250, 9
  br i1 %i17, label %if.then53, label %for.cond.thread184

if.then53:                                        ; preds = %if.else50
  %i18 = load i32, ptr %arrayidx54, align 4
  %inc55 = add i32 %i18, 1
  store i32 %inc55, ptr %arrayidx54, align 4
  br label %for.cond.preheader

sw.bb58:                                          ; preds = %for.cond.thread166, %for.cond.us295
  %i19 = phi i8 [ %i30, %for.cond.thread166 ], [ %i8, %for.cond.us295 ]
  %str.0175 = phi ptr [ %incdec.ptr107173, %for.cond.thread166 ], [ %incdec.ptr107.us297, %for.cond.us295 ]
  switch i8 %i19, label %if.else69 [
    i8 69, label %for.cond.thread216
    i8 101, label %for.cond.thread216
  ]

for.cond.thread216:                               ; preds = %sw.bb58, %sw.bb58
  %i20 = load i32, ptr %arrayidx67, align 4
  %inc68 = add i32 %i20, 1
  store i32 %inc68, ptr %arrayidx67, align 4
  %incdec.ptr107224 = getelementptr inbounds i8, ptr %str.0175, i64 1
  %i21 = load i8, ptr %incdec.ptr107224, align 1
  switch i8 %i21, label %if.else88 [
    i8 0, label %if.end113
    i8 44, label %if.then
    i8 43, label %for.cond.thread229
    i8 45, label %for.cond.thread229
  ]

if.else69:                                        ; preds = %sw.bb58
  %c.off.i248 = add i8 %i19, -48
  %i22 = icmp ugt i8 %c.off.i248, 9
  br i1 %i22, label %if.then72, label %for.cond.thread166

if.then72:                                        ; preds = %if.else69
  %i23 = load i32, ptr %arrayidx67, align 4
  %inc74 = add i32 %i23, 1
  store i32 %inc74, ptr %arrayidx67, align 4
  br label %for.cond.preheader

for.cond.thread229:                               ; preds = %for.cond.thread216, %for.cond.thread216
  %i24 = load i32, ptr %arrayidx86, align 4
  %inc87 = add i32 %i24, 1
  store i32 %inc87, ptr %arrayidx86, align 4
  %incdec.ptr107237 = getelementptr inbounds i8, ptr %str.0175, i64 2
  %i25 = load i8, ptr %incdec.ptr107237, align 1
  switch i8 %i25, label %sw.bb92 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

if.else88:                                        ; preds = %for.cond.thread216
  %i26 = load i32, ptr %arrayidx86, align 4
  %inc90 = add i32 %i26, 1
  store i32 %inc90, ptr %arrayidx86, align 4
  br label %for.cond.preheader

sw.bb92:                                          ; preds = %for.cond.thread229
  %c.off.i246 = add i8 %i25, -48
  %i27 = icmp ugt i8 %c.off.i246, 9
  %i28 = load i32, ptr %i11, align 4
  %inc100 = add i32 %i28, 1
  store i32 %inc100, ptr %i11, align 4
  br i1 %i27, label %for.cond.preheader, label %for.cond.thread201

sw.bb102:                                         ; preds = %for.cond.thread201
  %c.off.i244 = add i8 %i32, -48
  %i29 = icmp ugt i8 %c.off.i244, 9
  br i1 %i29, label %for.cond.preheader, label %for.cond.thread201

for.cond.thread166:                               ; preds = %if.else69, %if.then47, %if.else31
  %str.0174 = phi ptr [ %str.0194, %if.then47 ], [ %str.0175, %if.else69 ], [ %incdec.ptr107.us281, %if.else31 ]
  %incdec.ptr107173 = getelementptr inbounds i8, ptr %str.0174, i64 1
  %i30 = load i8, ptr %incdec.ptr107173, align 1
  switch i8 %i30, label %sw.bb58 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond.thread184:                               ; preds = %if.else50, %sw.bb25
  %str.0193 = phi ptr [ %str.0194, %if.else50 ], [ %incdec.ptr107.us281, %sw.bb25 ]
  %incdec.ptr107192 = getelementptr inbounds i8, ptr %str.0193, i64 1
  %i31 = load i8, ptr %incdec.ptr107192, align 1
  switch i8 %i31, label %sw.bb43 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

for.cond.thread201:                               ; preds = %sw.bb102, %sw.bb92
  %str.0210 = phi ptr [ %incdec.ptr107209, %sw.bb102 ], [ %incdec.ptr107237, %sw.bb92 ]
  %incdec.ptr107209 = getelementptr inbounds i8, ptr %str.0210, i64 1
  %i32 = load i8, ptr %incdec.ptr107209, align 1
  switch i8 %i32, label %sw.bb102 [
    i8 0, label %if.end113
    i8 44, label %if.then
  ]

if.then110:                                       ; preds = %for.cond
  %arrayidx111 = getelementptr inbounds i32, ptr %transition_count, i64 1
  %i33 = load i32, ptr %arrayidx111, align 4
  %inc112 = add i32 %i33, 1
  store i32 %inc112, ptr %arrayidx111, align 4
  br label %if.end113

if.end113:                                        ; preds = %if.then110, %for.cond.thread201, %for.cond.thread184, %for.cond.thread166, %for.cond.thread229, %for.cond.thread216, %if.then, %for.cond.us295, %for.cond.us287, %for.cond.us279, %for.cond.thread
  %str.1243 = phi ptr [ %incdec.ptr107, %if.then110 ], [ %i, %for.cond.thread ], [ %incdec.ptr, %if.then ], [ %incdec.ptr107237, %for.cond.thread229 ], [ %incdec.ptr107224, %for.cond.thread216 ], [ %incdec.ptr107.us281, %for.cond.us279 ], [ %incdec.ptr107.us289, %for.cond.us287 ], [ %incdec.ptr107.us297, %for.cond.us295 ], [ %incdec.ptr107173, %for.cond.thread166 ], [ %incdec.ptr107192, %for.cond.thread184 ], [ %incdec.ptr107209, %for.cond.thread201 ]
  %state.0162242 = phi i32 [ 1, %if.then110 ], [ 0, %for.cond.thread ], [ %state.0161, %if.then ], [ 6, %for.cond.thread229 ], [ 3, %for.cond.thread216 ], [ 2, %for.cond.us279 ], [ 4, %for.cond.us287 ], [ 5, %for.cond.us295 ], [ 5, %for.cond.thread166 ], [ 4, %for.cond.thread184 ], [ 7, %for.cond.thread201 ]
  store ptr %str.1243, ptr %instr, align 8
  ret i32 %state.0162242
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { "pre_loopopt" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
