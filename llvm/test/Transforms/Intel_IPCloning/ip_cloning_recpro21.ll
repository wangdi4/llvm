; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -opaque-pointers -enable-npm-dtrans -passes='lto-pre-link<O3>' -S 2>&1 | FileCheck %s

; Check that tail call elimination is not performed on foo

; CHECK: define internal fastcc void @foo
; CHECK: tail call fastcc void @foo
; CHECK: tail call fastcc void @foo

%struct.kdnode = type { i32, ptr, ptr }

@.str = private unnamed_addr constant [40 x i8] c"Error allocate kdnode array in nbtree!\0A\00", align 1
@stderr = external dso_local global ptr, align 8

declare dso_local i32 @get_blocksize() local_unnamed_addr

declare dso_local noalias ptr @malloc(i64) local_unnamed_addr

declare dso_local ptr @ivector(i32, i32) local_unnamed_addr

declare i64 @fwrite(ptr nocapture, i64, i64, ptr nocapture)

declare dso_local void @exit(i32) local_unnamed_addr

declare dso_local void @free_ivector(ptr, i32, i32) local_unnamed_addr

declare dso_local void @free(ptr nocapture) local_unnamed_addr

declare fastcc void @buildkdtree(ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, ptr, ptr nocapture, ptr readonly, i32, i32) unnamed_addr

declare fastcc void @heapsort_pairs(ptr nocapture, i32) unnamed_addr

define dso_local i32 @nblist(ptr nocapture %lpears, ptr nocapture %upears, ptr nocapture %pearlist, ptr readonly %x, i32 %context_PxQ, i32 %derivs, double %cutoff, i32 %natom, i32 %dim, ptr readonly %frozen) local_unnamed_addr {
entry:
  %locnt = alloca i32, align 4
  %upcnt = alloca i32, align 4
  %kdptr = alloca ptr, align 8
  %i = bitcast ptr %locnt to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i)
  %i1 = bitcast ptr %upcnt to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i1)
  %i2 = bitcast ptr %kdptr to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i2)
  %mul = fmul fast double %cutoff, %cutoff
  %call = tail call i32 @get_blocksize()
  %conv = sext i32 %natom to i64
  %mul1 = mul nsw i64 %conv, 24
  %call2 = tail call noalias ptr @malloc(i64 %mul1)
  %i3 = bitcast ptr %call2 to ptr
  %cmp = icmp eq ptr %call2, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i4 = load ptr, ptr @stderr, align 8
  %i5 = tail call i64 @fwrite(ptr @.str, i64 39, i64 1, ptr %i4)
  tail call void @exit(i32 1)
  unreachable

if.end:                                           ; preds = %entry
  %call5 = tail call ptr @ivector(i32 0, i32 %natom)
  %call6 = tail call ptr @ivector(i32 0, i32 %natom)
  %call7 = tail call ptr @ivector(i32 0, i32 %natom)
  %call8 = tail call ptr @ivector(i32 0, i32 %natom)
  %cmp9 = icmp eq i32 %dim, 4
  br i1 %cmp9, label %if.then11, label %if.end13

if.then11:                                        ; preds = %if.end
  %call12 = tail call ptr @ivector(i32 0, i32 %natom)
  br label %if.end13

if.end13:                                         ; preds = %if.then11, %if.end
  %wn.0 = phi ptr [ %call12, %if.then11 ], [ null, %if.end ]
  %call14 = tail call ptr @ivector(i32 0, i32 %natom)
  br label %for.cond

for.cond:                                         ; preds = %if.end26, %if.end13
  %i.0 = phi i32 [ 0, %if.end13 ], [ %inc, %if.end26 ]
  %cmp15 = icmp slt i32 %i.0, %natom
  br i1 %cmp15, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i6 = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32, ptr %call7, i64 %i6
  store i32 %i.0, ptr %arrayidx, align 4
  %arrayidx18 = getelementptr inbounds i32, ptr %call6, i64 %i6
  store i32 %i.0, ptr %arrayidx18, align 4
  %arrayidx20 = getelementptr inbounds i32, ptr %call5, i64 %i6
  store i32 %i.0, ptr %arrayidx20, align 4
  br i1 %cmp9, label %if.then23, label %if.end26

if.then23:                                        ; preds = %for.body
  %arrayidx25 = getelementptr inbounds i32, ptr %wn.0, i64 %i6
  store i32 %i.0, ptr %arrayidx25, align 4
  br label %if.end26

if.end26:                                         ; preds = %if.then23, %for.body
  %arrayidx28 = getelementptr inbounds i32, ptr %call14, i64 %i6
  store i32 %i.0, ptr %arrayidx28, align 4
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %div.i = sdiv i32 %natom, 2
  %cmp27.i = icmp sgt i32 %div.i, 0
  br i1 %cmp27.i, label %for.body.i, label %while.cond.preheader.i

while.cond.preheader.i:                           ; preds = %downheap_index.exit312, %for.end
  %cmp125.i = icmp sgt i32 %natom, 1
  br i1 %cmp125.i, label %while.body.i, label %heapsort_index.exit

for.body.i:                                       ; preds = %downheap_index.exit312, %for.end
  %k.028.i = phi i32 [ %sub.i, %downheap_index.exit312 ], [ %div.i, %for.end ]
  %sub.i = add nsw i32 %k.028.i, -1
  %idxprom.i = sext i32 %sub.i to i64
  %arrayidx.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom.i
  %i7 = load i32, ptr %arrayidx.i, align 4
  %cmp68.i270 = icmp slt i32 %div.i, %k.028.i
  br i1 %cmp68.i270, label %downheap_index.exit312, label %while.body.lr.ph.i274

while.body.lr.ph.i274:                            ; preds = %for.body.i
  %mul15.i271 = mul nsw i32 %i7, %dim
  %idxprom17.i272 = sext i32 %mul15.i271 to i64
  %arrayidx18.i273 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i272
  %i8 = load double, ptr %arrayidx18.i273, align 8
  br label %while.body.i278

while.body.i278:                                  ; preds = %if.end28.i309, %while.body.lr.ph.i274
  %k.addr.069.i275 = phi i32 [ %k.028.i, %while.body.lr.ph.i274 ], [ %j.0.i293, %if.end28.i309 ]
  %add.i276 = shl nsw i32 %k.addr.069.i275, 1
  %cmp1.i277 = icmp slt i32 %add.i276, %natom
  br i1 %cmp1.i277, label %land.lhs.true.i292, label %if.end.i301

land.lhs.true.i292:                               ; preds = %while.body.i278
  %sub2.i279 = add nsw i32 %add.i276, -1
  %idxprom3.i280 = sext i32 %sub2.i279 to i64
  %arrayidx4.i281 = getelementptr inbounds i32, ptr %call5, i64 %idxprom3.i280
  %i9 = load i32, ptr %arrayidx4.i281, align 4
  %mul.i282 = mul nsw i32 %i9, %dim
  %idxprom6.i283 = sext i32 %mul.i282 to i64
  %arrayidx7.i284 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i283
  %i10 = load double, ptr %arrayidx7.i284, align 8
  %idxprom8.i285 = sext i32 %add.i276 to i64
  %arrayidx9.i286 = getelementptr inbounds i32, ptr %call5, i64 %idxprom8.i285
  %i11 = load i32, ptr %arrayidx9.i286, align 4
  %mul10.i287 = mul nsw i32 %i11, %dim
  %idxprom12.i288 = sext i32 %mul10.i287 to i64
  %arrayidx13.i289 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i288
  %i12 = load double, ptr %arrayidx13.i289, align 8
  %cmp14.i290 = fcmp fast olt double %i10, %i12
  %inc.i291 = zext i1 %cmp14.i290 to i32
  %i13 = or i32 %add.i276, %inc.i291
  br label %if.end.i301

if.end.i301:                                      ; preds = %land.lhs.true.i292, %while.body.i278
  %j.0.i293 = phi i32 [ %i13, %land.lhs.true.i292 ], [ %add.i276, %while.body.i278 ]
  %sub19.i294 = add nsw i32 %j.0.i293, -1
  %idxprom20.i295 = sext i32 %sub19.i294 to i64
  %arrayidx21.i296 = getelementptr inbounds i32, ptr %call5, i64 %idxprom20.i295
  %i14 = load i32, ptr %arrayidx21.i296, align 4
  %mul22.i297 = mul nsw i32 %i14, %dim
  %idxprom24.i298 = sext i32 %mul22.i297 to i64
  %arrayidx25.i299 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i298
  %i15 = load double, ptr %arrayidx25.i299, align 8
  %cmp26.i300 = fcmp fast ult double %i8, %i15
  %sub32.i305 = add nsw i32 %k.addr.069.i275, -1
  %idxprom33.i306 = sext i32 %sub32.i305 to i64
  br i1 %cmp26.i300, label %if.end28.i309, label %downheap_index.exit312

if.end28.i309:                                    ; preds = %if.end.i301
  %arrayidx34.i307 = getelementptr inbounds i32, ptr %call5, i64 %idxprom33.i306
  store i32 %i14, ptr %arrayidx34.i307, align 4
  %cmp.i308 = icmp sgt i32 %j.0.i293, %div.i
  br i1 %cmp.i308, label %downheap_index.exit312, label %while.body.i278

downheap_index.exit312:                           ; preds = %if.end28.i309, %if.end.i301, %for.body.i
  %idxprom36.pre-phi.i310 = phi i64 [ %idxprom.i, %for.body.i ], [ %idxprom20.i295, %if.end28.i309 ], [ %idxprom33.i306, %if.end.i301 ]
  %arrayidx37.i311 = getelementptr inbounds i32, ptr %call5, i64 %idxprom36.pre-phi.i310
  store i32 %i7, ptr %arrayidx37.i311, align 4
  %cmp.i = icmp sgt i32 %sub.i, 0
  br i1 %cmp.i, label %for.body.i, label %while.cond.preheader.i

while.body.i:                                     ; preds = %downheap_index.exit, %while.cond.preheader.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %downheap_index.exit ], [ %conv, %while.cond.preheader.i ]
  %i16 = load i32, ptr %call5, align 4
  %indvars.iv.next.i = add nsw i64 %indvars.iv.i, -1
  %arrayidx2.i = getelementptr inbounds i32, ptr %call5, i64 %indvars.iv.next.i
  %i17 = load i32, ptr %arrayidx2.i, align 4
  store i32 %i17, ptr %call5, align 4
  store i32 %i16, ptr %arrayidx2.i, align 4
  %i18 = trunc i64 %indvars.iv.next.i to i32
  %i19 = load i32, ptr %call5, align 4
  %div.i265 = sdiv i32 %i18, 2
  %cmp68.i = icmp slt i32 %div.i265, 1
  br i1 %cmp68.i, label %downheap_index.exit, label %while.body.lr.ph.i

while.body.lr.ph.i:                               ; preds = %while.body.i
  %mul15.i = mul nsw i32 %i19, %dim
  %idxprom17.i = sext i32 %mul15.i to i64
  %arrayidx18.i = getelementptr inbounds double, ptr %x, i64 %idxprom17.i
  %i20 = load double, ptr %arrayidx18.i, align 8
  br label %while.body.i267

while.body.i267:                                  ; preds = %if.end28.i, %while.body.lr.ph.i
  %k.addr.069.i = phi i32 [ 1, %while.body.lr.ph.i ], [ %j.0.i, %if.end28.i ]
  %add.i = shl nsw i32 %k.addr.069.i, 1
  %cmp1.i266 = icmp slt i32 %add.i, %i18
  br i1 %cmp1.i266, label %land.lhs.true.i, label %if.end.i

land.lhs.true.i:                                  ; preds = %while.body.i267
  %sub2.i = add nsw i32 %add.i, -1
  %idxprom3.i = sext i32 %sub2.i to i64
  %arrayidx4.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom3.i
  %i21 = load i32, ptr %arrayidx4.i, align 4
  %mul.i = mul nsw i32 %i21, %dim
  %idxprom6.i = sext i32 %mul.i to i64
  %arrayidx7.i = getelementptr inbounds double, ptr %x, i64 %idxprom6.i
  %i22 = load double, ptr %arrayidx7.i, align 8
  %idxprom8.i = sext i32 %add.i to i64
  %arrayidx9.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom8.i
  %i23 = load i32, ptr %arrayidx9.i, align 4
  %mul10.i = mul nsw i32 %i23, %dim
  %idxprom12.i = sext i32 %mul10.i to i64
  %arrayidx13.i = getelementptr inbounds double, ptr %x, i64 %idxprom12.i
  %i24 = load double, ptr %arrayidx13.i, align 8
  %cmp14.i = fcmp fast olt double %i22, %i24
  %inc.i = zext i1 %cmp14.i to i32
  %i25 = or i32 %add.i, %inc.i
  br label %if.end.i

if.end.i:                                         ; preds = %land.lhs.true.i, %while.body.i267
  %j.0.i = phi i32 [ %i25, %land.lhs.true.i ], [ %add.i, %while.body.i267 ]
  %sub19.i = add nsw i32 %j.0.i, -1
  %idxprom20.i = sext i32 %sub19.i to i64
  %arrayidx21.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom20.i
  %i26 = load i32, ptr %arrayidx21.i, align 4
  %mul22.i = mul nsw i32 %i26, %dim
  %idxprom24.i = sext i32 %mul22.i to i64
  %arrayidx25.i = getelementptr inbounds double, ptr %x, i64 %idxprom24.i
  %i27 = load double, ptr %arrayidx25.i, align 8
  %cmp26.i = fcmp fast ult double %i20, %i27
  %sub32.i = add nsw i32 %k.addr.069.i, -1
  %idxprom33.i = sext i32 %sub32.i to i64
  br i1 %cmp26.i, label %if.end28.i, label %downheap_index.exit

if.end28.i:                                       ; preds = %if.end.i
  %arrayidx34.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom33.i
  store i32 %i26, ptr %arrayidx34.i, align 4
  %cmp.i268 = icmp sgt i32 %j.0.i, %div.i265
  br i1 %cmp.i268, label %downheap_index.exit, label %while.body.i267

downheap_index.exit:                              ; preds = %if.end28.i, %if.end.i, %while.body.i
  %idxprom36.pre-phi.i = phi i64 [ 0, %while.body.i ], [ %idxprom20.i, %if.end28.i ], [ %idxprom33.i, %if.end.i ]
  %arrayidx37.i = getelementptr inbounds i32, ptr %call5, i64 %idxprom36.pre-phi.i
  store i32 %i19, ptr %arrayidx37.i, align 4
  %cmp1.i = icmp sgt i64 %indvars.iv.next.i, 1
  br i1 %cmp1.i, label %while.body.i, label %heapsort_index.exit

heapsort_index.exit:                              ; preds = %downheap_index.exit, %while.cond.preheader.i
  br i1 %cmp27.i, label %for.body.i321, label %while.cond.preheader.i316

while.cond.preheader.i316:                        ; preds = %downheap_index.exit422, %heapsort_index.exit
  br i1 %cmp125.i, label %while.body.i326, label %heapsort_index.exit327

for.body.i321:                                    ; preds = %downheap_index.exit422, %heapsort_index.exit
  %k.028.i318 = phi i32 [ %sub.i372, %downheap_index.exit422 ], [ %div.i, %heapsort_index.exit ]
  %sub.i372 = add nsw i32 %k.028.i318, -1
  %idxprom.i373 = sext i32 %sub.i372 to i64
  %arrayidx.i374 = getelementptr inbounds i32, ptr %call6, i64 %idxprom.i373
  %i28 = load i32, ptr %arrayidx.i374, align 4
  %cmp68.i376 = icmp slt i32 %div.i, %k.028.i318
  br i1 %cmp68.i376, label %downheap_index.exit422, label %while.body.lr.ph.i381

while.body.lr.ph.i381:                            ; preds = %for.body.i321
  %mul15.i377 = mul nsw i32 %i28, %dim
  %add16.i378 = add nsw i32 %mul15.i377, 1
  %idxprom17.i379 = sext i32 %add16.i378 to i64
  %arrayidx18.i380 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i379
  %i29 = load double, ptr %arrayidx18.i380, align 8
  br label %while.body.i385

while.body.i385:                                  ; preds = %if.end28.i419, %while.body.lr.ph.i381
  %k.addr.069.i382 = phi i32 [ %k.028.i318, %while.body.lr.ph.i381 ], [ %j.0.i402, %if.end28.i419 ]
  %add.i383 = shl nsw i32 %k.addr.069.i382, 1
  %cmp1.i384 = icmp slt i32 %add.i383, %natom
  br i1 %cmp1.i384, label %land.lhs.true.i401, label %if.end.i411

land.lhs.true.i401:                               ; preds = %while.body.i385
  %sub2.i386 = add nsw i32 %add.i383, -1
  %idxprom3.i387 = sext i32 %sub2.i386 to i64
  %arrayidx4.i388 = getelementptr inbounds i32, ptr %call6, i64 %idxprom3.i387
  %i30 = load i32, ptr %arrayidx4.i388, align 4
  %mul.i389 = mul nsw i32 %i30, %dim
  %add5.i390 = add nsw i32 %mul.i389, 1
  %idxprom6.i391 = sext i32 %add5.i390 to i64
  %arrayidx7.i392 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i391
  %i31 = load double, ptr %arrayidx7.i392, align 8
  %idxprom8.i393 = sext i32 %add.i383 to i64
  %arrayidx9.i394 = getelementptr inbounds i32, ptr %call6, i64 %idxprom8.i393
  %i32 = load i32, ptr %arrayidx9.i394, align 4
  %mul10.i395 = mul nsw i32 %i32, %dim
  %add11.i396 = add nsw i32 %mul10.i395, 1
  %idxprom12.i397 = sext i32 %add11.i396 to i64
  %arrayidx13.i398 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i397
  %i33 = load double, ptr %arrayidx13.i398, align 8
  %cmp14.i399 = fcmp fast olt double %i31, %i33
  %inc.i400 = zext i1 %cmp14.i399 to i32
  %i34 = or i32 %add.i383, %inc.i400
  br label %if.end.i411

if.end.i411:                                      ; preds = %land.lhs.true.i401, %while.body.i385
  %j.0.i402 = phi i32 [ %i34, %land.lhs.true.i401 ], [ %add.i383, %while.body.i385 ]
  %sub19.i403 = add nsw i32 %j.0.i402, -1
  %idxprom20.i404 = sext i32 %sub19.i403 to i64
  %arrayidx21.i405 = getelementptr inbounds i32, ptr %call6, i64 %idxprom20.i404
  %i35 = load i32, ptr %arrayidx21.i405, align 4
  %mul22.i406 = mul nsw i32 %i35, %dim
  %add23.i407 = add nsw i32 %mul22.i406, 1
  %idxprom24.i408 = sext i32 %add23.i407 to i64
  %arrayidx25.i409 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i408
  %i36 = load double, ptr %arrayidx25.i409, align 8
  %cmp26.i410 = fcmp fast ult double %i29, %i36
  %sub32.i415 = add nsw i32 %k.addr.069.i382, -1
  %idxprom33.i416 = sext i32 %sub32.i415 to i64
  br i1 %cmp26.i410, label %if.end28.i419, label %downheap_index.exit422

if.end28.i419:                                    ; preds = %if.end.i411
  %arrayidx34.i417 = getelementptr inbounds i32, ptr %call6, i64 %idxprom33.i416
  store i32 %i35, ptr %arrayidx34.i417, align 4
  %cmp.i418 = icmp sgt i32 %j.0.i402, %div.i
  br i1 %cmp.i418, label %downheap_index.exit422, label %while.body.i385

downheap_index.exit422:                           ; preds = %if.end28.i419, %if.end.i411, %for.body.i321
  %idxprom36.pre-phi.i420 = phi i64 [ %idxprom.i373, %for.body.i321 ], [ %idxprom20.i404, %if.end28.i419 ], [ %idxprom33.i416, %if.end.i411 ]
  %arrayidx37.i421 = getelementptr inbounds i32, ptr %call6, i64 %idxprom36.pre-phi.i420
  store i32 %i28, ptr %arrayidx37.i421, align 4
  %cmp.i320 = icmp sgt i32 %sub.i372, 0
  br i1 %cmp.i320, label %for.body.i321, label %while.cond.preheader.i316

while.body.i326:                                  ; preds = %downheap_index.exit371, %while.cond.preheader.i316
  %indvars.iv.i322 = phi i64 [ %indvars.iv.next.i323, %downheap_index.exit371 ], [ %conv, %while.cond.preheader.i316 ]
  %i37 = load i32, ptr %call6, align 4
  %indvars.iv.next.i323 = add nsw i64 %indvars.iv.i322, -1
  %arrayidx2.i324 = getelementptr inbounds i32, ptr %call6, i64 %indvars.iv.next.i323
  %i38 = load i32, ptr %arrayidx2.i324, align 4
  store i32 %i38, ptr %call6, align 4
  store i32 %i37, ptr %arrayidx2.i324, align 4
  %i39 = trunc i64 %indvars.iv.next.i323 to i32
  %i40 = load i32, ptr %call6, align 4
  %div.i328 = sdiv i32 %i39, 2
  %cmp68.i329 = icmp slt i32 %div.i328, 1
  br i1 %cmp68.i329, label %downheap_index.exit371, label %while.body.lr.ph.i333

while.body.lr.ph.i333:                            ; preds = %while.body.i326
  %mul15.i330 = mul nsw i32 %i40, %dim
  %add16.i = add nsw i32 %mul15.i330, 1
  %idxprom17.i331 = sext i32 %add16.i to i64
  %arrayidx18.i332 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i331
  %i41 = load double, ptr %arrayidx18.i332, align 8
  br label %while.body.i337

while.body.i337:                                  ; preds = %if.end28.i368, %while.body.lr.ph.i333
  %k.addr.069.i334 = phi i32 [ 1, %while.body.lr.ph.i333 ], [ %j.0.i352, %if.end28.i368 ]
  %add.i335 = shl nsw i32 %k.addr.069.i334, 1
  %cmp1.i336 = icmp slt i32 %add.i335, %i39
  br i1 %cmp1.i336, label %land.lhs.true.i351, label %if.end.i360

land.lhs.true.i351:                               ; preds = %while.body.i337
  %sub2.i338 = add nsw i32 %add.i335, -1
  %idxprom3.i339 = sext i32 %sub2.i338 to i64
  %arrayidx4.i340 = getelementptr inbounds i32, ptr %call6, i64 %idxprom3.i339
  %i42 = load i32, ptr %arrayidx4.i340, align 4
  %mul.i341 = mul nsw i32 %i42, %dim
  %add5.i = add nsw i32 %mul.i341, 1
  %idxprom6.i342 = sext i32 %add5.i to i64
  %arrayidx7.i343 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i342
  %i43 = load double, ptr %arrayidx7.i343, align 8
  %idxprom8.i344 = sext i32 %add.i335 to i64
  %arrayidx9.i345 = getelementptr inbounds i32, ptr %call6, i64 %idxprom8.i344
  %i44 = load i32, ptr %arrayidx9.i345, align 4
  %mul10.i346 = mul nsw i32 %i44, %dim
  %add11.i = add nsw i32 %mul10.i346, 1
  %idxprom12.i347 = sext i32 %add11.i to i64
  %arrayidx13.i348 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i347
  %i45 = load double, ptr %arrayidx13.i348, align 8
  %cmp14.i349 = fcmp fast olt double %i43, %i45
  %inc.i350 = zext i1 %cmp14.i349 to i32
  %i46 = or i32 %add.i335, %inc.i350
  br label %if.end.i360

if.end.i360:                                      ; preds = %land.lhs.true.i351, %while.body.i337
  %j.0.i352 = phi i32 [ %i46, %land.lhs.true.i351 ], [ %add.i335, %while.body.i337 ]
  %sub19.i353 = add nsw i32 %j.0.i352, -1
  %idxprom20.i354 = sext i32 %sub19.i353 to i64
  %arrayidx21.i355 = getelementptr inbounds i32, ptr %call6, i64 %idxprom20.i354
  %i47 = load i32, ptr %arrayidx21.i355, align 4
  %mul22.i356 = mul nsw i32 %i47, %dim
  %add23.i = add nsw i32 %mul22.i356, 1
  %idxprom24.i357 = sext i32 %add23.i to i64
  %arrayidx25.i358 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i357
  %i48 = load double, ptr %arrayidx25.i358, align 8
  %cmp26.i359 = fcmp fast ult double %i41, %i48
  %sub32.i364 = add nsw i32 %k.addr.069.i334, -1
  %idxprom33.i365 = sext i32 %sub32.i364 to i64
  br i1 %cmp26.i359, label %if.end28.i368, label %downheap_index.exit371

if.end28.i368:                                    ; preds = %if.end.i360
  %arrayidx34.i366 = getelementptr inbounds i32, ptr %call6, i64 %idxprom33.i365
  store i32 %i47, ptr %arrayidx34.i366, align 4
  %cmp.i367 = icmp sgt i32 %j.0.i352, %div.i328
  br i1 %cmp.i367, label %downheap_index.exit371, label %while.body.i337

downheap_index.exit371:                           ; preds = %if.end28.i368, %if.end.i360, %while.body.i326
  %idxprom36.pre-phi.i369 = phi i64 [ 0, %while.body.i326 ], [ %idxprom20.i354, %if.end28.i368 ], [ %idxprom33.i365, %if.end.i360 ]
  %arrayidx37.i370 = getelementptr inbounds i32, ptr %call6, i64 %idxprom36.pre-phi.i369
  store i32 %i40, ptr %arrayidx37.i370, align 4
  %cmp1.i325 = icmp sgt i64 %indvars.iv.next.i323, 1
  br i1 %cmp1.i325, label %while.body.i326, label %heapsort_index.exit327

heapsort_index.exit327:                           ; preds = %downheap_index.exit371, %while.cond.preheader.i316
  br i1 %cmp27.i, label %for.body.i431, label %while.cond.preheader.i426

while.cond.preheader.i426:                        ; preds = %downheap_index.exit536, %heapsort_index.exit327
  br i1 %cmp125.i, label %while.body.i436, label %heapsort_index.exit437

for.body.i431:                                    ; preds = %downheap_index.exit536, %heapsort_index.exit327
  %k.028.i428 = phi i32 [ %sub.i486, %downheap_index.exit536 ], [ %div.i, %heapsort_index.exit327 ]
  %sub.i486 = add nsw i32 %k.028.i428, -1
  %idxprom.i487 = sext i32 %sub.i486 to i64
  %arrayidx.i488 = getelementptr inbounds i32, ptr %call7, i64 %idxprom.i487
  %i49 = load i32, ptr %arrayidx.i488, align 4
  %cmp68.i490 = icmp slt i32 %div.i, %k.028.i428
  br i1 %cmp68.i490, label %downheap_index.exit536, label %while.body.lr.ph.i495

while.body.lr.ph.i495:                            ; preds = %for.body.i431
  %mul15.i491 = mul nsw i32 %i49, %dim
  %add16.i492 = add nsw i32 %mul15.i491, 2
  %idxprom17.i493 = sext i32 %add16.i492 to i64
  %arrayidx18.i494 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i493
  %i50 = load double, ptr %arrayidx18.i494, align 8
  br label %while.body.i499

while.body.i499:                                  ; preds = %if.end28.i533, %while.body.lr.ph.i495
  %k.addr.069.i496 = phi i32 [ %k.028.i428, %while.body.lr.ph.i495 ], [ %j.0.i516, %if.end28.i533 ]
  %add.i497 = shl nsw i32 %k.addr.069.i496, 1
  %cmp1.i498 = icmp slt i32 %add.i497, %natom
  br i1 %cmp1.i498, label %land.lhs.true.i515, label %if.end.i525

land.lhs.true.i515:                               ; preds = %while.body.i499
  %sub2.i500 = add nsw i32 %add.i497, -1
  %idxprom3.i501 = sext i32 %sub2.i500 to i64
  %arrayidx4.i502 = getelementptr inbounds i32, ptr %call7, i64 %idxprom3.i501
  %i51 = load i32, ptr %arrayidx4.i502, align 4
  %mul.i503 = mul nsw i32 %i51, %dim
  %add5.i504 = add nsw i32 %mul.i503, 2
  %idxprom6.i505 = sext i32 %add5.i504 to i64
  %arrayidx7.i506 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i505
  %i52 = load double, ptr %arrayidx7.i506, align 8
  %idxprom8.i507 = sext i32 %add.i497 to i64
  %arrayidx9.i508 = getelementptr inbounds i32, ptr %call7, i64 %idxprom8.i507
  %i53 = load i32, ptr %arrayidx9.i508, align 4
  %mul10.i509 = mul nsw i32 %i53, %dim
  %add11.i510 = add nsw i32 %mul10.i509, 2
  %idxprom12.i511 = sext i32 %add11.i510 to i64
  %arrayidx13.i512 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i511
  %i54 = load double, ptr %arrayidx13.i512, align 8
  %cmp14.i513 = fcmp fast olt double %i52, %i54
  %inc.i514 = zext i1 %cmp14.i513 to i32
  %i55 = or i32 %add.i497, %inc.i514
  br label %if.end.i525

if.end.i525:                                      ; preds = %land.lhs.true.i515, %while.body.i499
  %j.0.i516 = phi i32 [ %i55, %land.lhs.true.i515 ], [ %add.i497, %while.body.i499 ]
  %sub19.i517 = add nsw i32 %j.0.i516, -1
  %idxprom20.i518 = sext i32 %sub19.i517 to i64
  %arrayidx21.i519 = getelementptr inbounds i32, ptr %call7, i64 %idxprom20.i518
  %i56 = load i32, ptr %arrayidx21.i519, align 4
  %mul22.i520 = mul nsw i32 %i56, %dim
  %add23.i521 = add nsw i32 %mul22.i520, 2
  %idxprom24.i522 = sext i32 %add23.i521 to i64
  %arrayidx25.i523 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i522
  %i57 = load double, ptr %arrayidx25.i523, align 8
  %cmp26.i524 = fcmp fast ult double %i50, %i57
  %sub32.i529 = add nsw i32 %k.addr.069.i496, -1
  %idxprom33.i530 = sext i32 %sub32.i529 to i64
  br i1 %cmp26.i524, label %if.end28.i533, label %downheap_index.exit536

if.end28.i533:                                    ; preds = %if.end.i525
  %arrayidx34.i531 = getelementptr inbounds i32, ptr %call7, i64 %idxprom33.i530
  store i32 %i56, ptr %arrayidx34.i531, align 4
  %cmp.i532 = icmp sgt i32 %j.0.i516, %div.i
  br i1 %cmp.i532, label %downheap_index.exit536, label %while.body.i499

downheap_index.exit536:                           ; preds = %if.end28.i533, %if.end.i525, %for.body.i431
  %idxprom36.pre-phi.i534 = phi i64 [ %idxprom.i487, %for.body.i431 ], [ %idxprom20.i518, %if.end28.i533 ], [ %idxprom33.i530, %if.end.i525 ]
  %arrayidx37.i535 = getelementptr inbounds i32, ptr %call7, i64 %idxprom36.pre-phi.i534
  store i32 %i49, ptr %arrayidx37.i535, align 4
  %cmp.i430 = icmp sgt i32 %sub.i486, 0
  br i1 %cmp.i430, label %for.body.i431, label %while.cond.preheader.i426

while.body.i436:                                  ; preds = %downheap_index.exit485, %while.cond.preheader.i426
  %indvars.iv.i432 = phi i64 [ %indvars.iv.next.i433, %downheap_index.exit485 ], [ %conv, %while.cond.preheader.i426 ]
  %i58 = load i32, ptr %call7, align 4
  %indvars.iv.next.i433 = add nsw i64 %indvars.iv.i432, -1
  %arrayidx2.i434 = getelementptr inbounds i32, ptr %call7, i64 %indvars.iv.next.i433
  %i59 = load i32, ptr %arrayidx2.i434, align 4
  store i32 %i59, ptr %call7, align 4
  store i32 %i58, ptr %arrayidx2.i434, align 4
  %i60 = trunc i64 %indvars.iv.next.i433 to i32
  %i61 = load i32, ptr %call7, align 4
  %div.i438 = sdiv i32 %i60, 2
  %cmp68.i439 = icmp slt i32 %div.i438, 1
  br i1 %cmp68.i439, label %downheap_index.exit485, label %while.body.lr.ph.i444

while.body.lr.ph.i444:                            ; preds = %while.body.i436
  %mul15.i440 = mul nsw i32 %i61, %dim
  %add16.i441 = add nsw i32 %mul15.i440, 2
  %idxprom17.i442 = sext i32 %add16.i441 to i64
  %arrayidx18.i443 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i442
  %i62 = load double, ptr %arrayidx18.i443, align 8
  br label %while.body.i448

while.body.i448:                                  ; preds = %if.end28.i482, %while.body.lr.ph.i444
  %k.addr.069.i445 = phi i32 [ 1, %while.body.lr.ph.i444 ], [ %j.0.i465, %if.end28.i482 ]
  %add.i446 = shl nsw i32 %k.addr.069.i445, 1
  %cmp1.i447 = icmp slt i32 %add.i446, %i60
  br i1 %cmp1.i447, label %land.lhs.true.i464, label %if.end.i474

land.lhs.true.i464:                               ; preds = %while.body.i448
  %sub2.i449 = add nsw i32 %add.i446, -1
  %idxprom3.i450 = sext i32 %sub2.i449 to i64
  %arrayidx4.i451 = getelementptr inbounds i32, ptr %call7, i64 %idxprom3.i450
  %i63 = load i32, ptr %arrayidx4.i451, align 4
  %mul.i452 = mul nsw i32 %i63, %dim
  %add5.i453 = add nsw i32 %mul.i452, 2
  %idxprom6.i454 = sext i32 %add5.i453 to i64
  %arrayidx7.i455 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i454
  %i64 = load double, ptr %arrayidx7.i455, align 8
  %idxprom8.i456 = sext i32 %add.i446 to i64
  %arrayidx9.i457 = getelementptr inbounds i32, ptr %call7, i64 %idxprom8.i456
  %i65 = load i32, ptr %arrayidx9.i457, align 4
  %mul10.i458 = mul nsw i32 %i65, %dim
  %add11.i459 = add nsw i32 %mul10.i458, 2
  %idxprom12.i460 = sext i32 %add11.i459 to i64
  %arrayidx13.i461 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i460
  %i66 = load double, ptr %arrayidx13.i461, align 8
  %cmp14.i462 = fcmp fast olt double %i64, %i66
  %inc.i463 = zext i1 %cmp14.i462 to i32
  %i67 = or i32 %add.i446, %inc.i463
  br label %if.end.i474

if.end.i474:                                      ; preds = %land.lhs.true.i464, %while.body.i448
  %j.0.i465 = phi i32 [ %i67, %land.lhs.true.i464 ], [ %add.i446, %while.body.i448 ]
  %sub19.i466 = add nsw i32 %j.0.i465, -1
  %idxprom20.i467 = sext i32 %sub19.i466 to i64
  %arrayidx21.i468 = getelementptr inbounds i32, ptr %call7, i64 %idxprom20.i467
  %i68 = load i32, ptr %arrayidx21.i468, align 4
  %mul22.i469 = mul nsw i32 %i68, %dim
  %add23.i470 = add nsw i32 %mul22.i469, 2
  %idxprom24.i471 = sext i32 %add23.i470 to i64
  %arrayidx25.i472 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i471
  %i69 = load double, ptr %arrayidx25.i472, align 8
  %cmp26.i473 = fcmp fast ult double %i62, %i69
  %sub32.i478 = add nsw i32 %k.addr.069.i445, -1
  %idxprom33.i479 = sext i32 %sub32.i478 to i64
  br i1 %cmp26.i473, label %if.end28.i482, label %downheap_index.exit485

if.end28.i482:                                    ; preds = %if.end.i474
  %arrayidx34.i480 = getelementptr inbounds i32, ptr %call7, i64 %idxprom33.i479
  store i32 %i68, ptr %arrayidx34.i480, align 4
  %cmp.i481 = icmp sgt i32 %j.0.i465, %div.i438
  br i1 %cmp.i481, label %downheap_index.exit485, label %while.body.i448

downheap_index.exit485:                           ; preds = %if.end28.i482, %if.end.i474, %while.body.i436
  %idxprom36.pre-phi.i483 = phi i64 [ 0, %while.body.i436 ], [ %idxprom20.i467, %if.end28.i482 ], [ %idxprom33.i479, %if.end.i474 ]
  %arrayidx37.i484 = getelementptr inbounds i32, ptr %call7, i64 %idxprom36.pre-phi.i483
  store i32 %i61, ptr %arrayidx37.i484, align 4
  %cmp1.i435 = icmp sgt i64 %indvars.iv.next.i433, 1
  br i1 %cmp1.i435, label %while.body.i436, label %heapsort_index.exit437

heapsort_index.exit437:                           ; preds = %downheap_index.exit485, %while.cond.preheader.i426
  br i1 %cmp9, label %if.then31, label %for.end.split

for.end.split:                                    ; preds = %heapsort_index.exit437
  %incdec.ptr255 = getelementptr inbounds i8, ptr %call2, i64 24
  %i70 = bitcast ptr %kdptr to ptr
  store ptr %incdec.ptr255, ptr %i70, align 8
  %lo256 = getelementptr inbounds i8, ptr %call2, i64 8
  %i71 = bitcast ptr %lo256 to ptr
  store ptr null, ptr %i71, align 8
  %hi257 = getelementptr inbounds i8, ptr %call2, i64 16
  %i72 = bitcast ptr %hi257 to ptr
  store ptr null, ptr %i72, align 8
  %sub258 = add nsw i32 %natom, -1
  call fastcc void @buildkdtree(ptr %call14, ptr %call5, ptr %call6, ptr %call7, ptr %wn.0, ptr %call8, i32 0, i32 %sub258, ptr nonnull %kdptr, ptr %i3, ptr %x, i32 0, i32 %dim)
  br label %if.end32

if.then31:                                        ; preds = %heapsort_index.exit437
  br i1 %cmp27.i, label %for.body.i545, label %while.cond.preheader.i540

while.cond.preheader.i540:                        ; preds = %downheap_index.exit650, %if.then31
  br i1 %cmp125.i, label %while.body.i550, label %heapsort_index.exit551

for.body.i545:                                    ; preds = %downheap_index.exit650, %if.then31
  %k.028.i542 = phi i32 [ %sub.i600, %downheap_index.exit650 ], [ %div.i, %if.then31 ]
  %sub.i600 = add nsw i32 %k.028.i542, -1
  %idxprom.i601 = sext i32 %sub.i600 to i64
  %arrayidx.i602 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom.i601
  %i73 = load i32, ptr %arrayidx.i602, align 4
  %cmp68.i604 = icmp slt i32 %div.i, %k.028.i542
  br i1 %cmp68.i604, label %downheap_index.exit650, label %while.body.lr.ph.i609

while.body.lr.ph.i609:                            ; preds = %for.body.i545
  %mul15.i605 = shl nsw i32 %i73, 2
  %add16.i606 = or i32 %mul15.i605, 3
  %idxprom17.i607 = sext i32 %add16.i606 to i64
  %arrayidx18.i608 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i607
  %i74 = load double, ptr %arrayidx18.i608, align 8
  br label %while.body.i613

while.body.i613:                                  ; preds = %if.end28.i647, %while.body.lr.ph.i609
  %k.addr.069.i610 = phi i32 [ %k.028.i542, %while.body.lr.ph.i609 ], [ %j.0.i630, %if.end28.i647 ]
  %add.i611 = shl nsw i32 %k.addr.069.i610, 1
  %cmp1.i612 = icmp slt i32 %add.i611, %natom
  br i1 %cmp1.i612, label %land.lhs.true.i629, label %if.end.i639

land.lhs.true.i629:                               ; preds = %while.body.i613
  %sub2.i614 = add nsw i32 %add.i611, -1
  %idxprom3.i615 = sext i32 %sub2.i614 to i64
  %arrayidx4.i616 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom3.i615
  %i75 = load i32, ptr %arrayidx4.i616, align 4
  %mul.i617 = shl nsw i32 %i75, 2
  %add5.i618 = or i32 %mul.i617, 3
  %idxprom6.i619 = sext i32 %add5.i618 to i64
  %arrayidx7.i620 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i619
  %i76 = load double, ptr %arrayidx7.i620, align 8
  %idxprom8.i621 = sext i32 %add.i611 to i64
  %arrayidx9.i622 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom8.i621
  %i77 = load i32, ptr %arrayidx9.i622, align 4
  %mul10.i623 = shl nsw i32 %i77, 2
  %add11.i624 = or i32 %mul10.i623, 3
  %idxprom12.i625 = sext i32 %add11.i624 to i64
  %arrayidx13.i626 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i625
  %i78 = load double, ptr %arrayidx13.i626, align 8
  %cmp14.i627 = fcmp fast olt double %i76, %i78
  %inc.i628 = zext i1 %cmp14.i627 to i32
  %i79 = or i32 %add.i611, %inc.i628
  br label %if.end.i639

if.end.i639:                                      ; preds = %land.lhs.true.i629, %while.body.i613
  %j.0.i630 = phi i32 [ %i79, %land.lhs.true.i629 ], [ %add.i611, %while.body.i613 ]
  %sub19.i631 = add nsw i32 %j.0.i630, -1
  %idxprom20.i632 = sext i32 %sub19.i631 to i64
  %arrayidx21.i633 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom20.i632
  %i80 = load i32, ptr %arrayidx21.i633, align 4
  %mul22.i634 = shl nsw i32 %i80, 2
  %add23.i635 = or i32 %mul22.i634, 3
  %idxprom24.i636 = sext i32 %add23.i635 to i64
  %arrayidx25.i637 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i636
  %i81 = load double, ptr %arrayidx25.i637, align 8
  %cmp26.i638 = fcmp fast ult double %i74, %i81
  %sub32.i643 = add nsw i32 %k.addr.069.i610, -1
  %idxprom33.i644 = sext i32 %sub32.i643 to i64
  br i1 %cmp26.i638, label %if.end28.i647, label %downheap_index.exit650

if.end28.i647:                                    ; preds = %if.end.i639
  %arrayidx34.i645 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom33.i644
  store i32 %i80, ptr %arrayidx34.i645, align 4
  %cmp.i646 = icmp sgt i32 %j.0.i630, %div.i
  br i1 %cmp.i646, label %downheap_index.exit650, label %while.body.i613

downheap_index.exit650:                           ; preds = %if.end28.i647, %if.end.i639, %for.body.i545
  %idxprom36.pre-phi.i648 = phi i64 [ %idxprom.i601, %for.body.i545 ], [ %idxprom20.i632, %if.end28.i647 ], [ %idxprom33.i644, %if.end.i639 ]
  %arrayidx37.i649 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom36.pre-phi.i648
  store i32 %i73, ptr %arrayidx37.i649, align 4
  %cmp.i544 = icmp sgt i32 %sub.i600, 0
  br i1 %cmp.i544, label %for.body.i545, label %while.cond.preheader.i540

while.body.i550:                                  ; preds = %downheap_index.exit599, %while.cond.preheader.i540
  %indvars.iv.i546 = phi i64 [ %indvars.iv.next.i547, %downheap_index.exit599 ], [ %conv, %while.cond.preheader.i540 ]
  %i82 = load i32, ptr %wn.0, align 4
  %indvars.iv.next.i547 = add nsw i64 %indvars.iv.i546, -1
  %arrayidx2.i548 = getelementptr inbounds i32, ptr %wn.0, i64 %indvars.iv.next.i547
  %i83 = load i32, ptr %arrayidx2.i548, align 4
  store i32 %i83, ptr %wn.0, align 4
  store i32 %i82, ptr %arrayidx2.i548, align 4
  %i84 = trunc i64 %indvars.iv.next.i547 to i32
  %i85 = load i32, ptr %wn.0, align 4
  %div.i552 = sdiv i32 %i84, 2
  %cmp68.i553 = icmp slt i32 %div.i552, 1
  br i1 %cmp68.i553, label %downheap_index.exit599, label %while.body.lr.ph.i558

while.body.lr.ph.i558:                            ; preds = %while.body.i550
  %mul15.i554 = shl nsw i32 %i85, 2
  %add16.i555 = or i32 %mul15.i554, 3
  %idxprom17.i556 = sext i32 %add16.i555 to i64
  %arrayidx18.i557 = getelementptr inbounds double, ptr %x, i64 %idxprom17.i556
  %i86 = load double, ptr %arrayidx18.i557, align 8
  br label %while.body.i562

while.body.i562:                                  ; preds = %if.end28.i596, %while.body.lr.ph.i558
  %k.addr.069.i559 = phi i32 [ 1, %while.body.lr.ph.i558 ], [ %j.0.i579, %if.end28.i596 ]
  %add.i560 = shl nsw i32 %k.addr.069.i559, 1
  %cmp1.i561 = icmp slt i32 %add.i560, %i84
  br i1 %cmp1.i561, label %land.lhs.true.i578, label %if.end.i588

land.lhs.true.i578:                               ; preds = %while.body.i562
  %sub2.i563 = add nsw i32 %add.i560, -1
  %idxprom3.i564 = sext i32 %sub2.i563 to i64
  %arrayidx4.i565 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom3.i564
  %i87 = load i32, ptr %arrayidx4.i565, align 4
  %mul.i566 = shl nsw i32 %i87, 2
  %add5.i567 = or i32 %mul.i566, 3
  %idxprom6.i568 = sext i32 %add5.i567 to i64
  %arrayidx7.i569 = getelementptr inbounds double, ptr %x, i64 %idxprom6.i568
  %i88 = load double, ptr %arrayidx7.i569, align 8
  %idxprom8.i570 = sext i32 %add.i560 to i64
  %arrayidx9.i571 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom8.i570
  %i89 = load i32, ptr %arrayidx9.i571, align 4
  %mul10.i572 = shl nsw i32 %i89, 2
  %add11.i573 = or i32 %mul10.i572, 3
  %idxprom12.i574 = sext i32 %add11.i573 to i64
  %arrayidx13.i575 = getelementptr inbounds double, ptr %x, i64 %idxprom12.i574
  %i90 = load double, ptr %arrayidx13.i575, align 8
  %cmp14.i576 = fcmp fast olt double %i88, %i90
  %inc.i577 = zext i1 %cmp14.i576 to i32
  %i91 = or i32 %add.i560, %inc.i577
  br label %if.end.i588

if.end.i588:                                      ; preds = %land.lhs.true.i578, %while.body.i562
  %j.0.i579 = phi i32 [ %i91, %land.lhs.true.i578 ], [ %add.i560, %while.body.i562 ]
  %sub19.i580 = add nsw i32 %j.0.i579, -1
  %idxprom20.i581 = sext i32 %sub19.i580 to i64
  %arrayidx21.i582 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom20.i581
  %i92 = load i32, ptr %arrayidx21.i582, align 4
  %mul22.i583 = shl nsw i32 %i92, 2
  %add23.i584 = or i32 %mul22.i583, 3
  %idxprom24.i585 = sext i32 %add23.i584 to i64
  %arrayidx25.i586 = getelementptr inbounds double, ptr %x, i64 %idxprom24.i585
  %i93 = load double, ptr %arrayidx25.i586, align 8
  %cmp26.i587 = fcmp fast ult double %i86, %i93
  %sub32.i592 = add nsw i32 %k.addr.069.i559, -1
  %idxprom33.i593 = sext i32 %sub32.i592 to i64
  br i1 %cmp26.i587, label %if.end28.i596, label %downheap_index.exit599

if.end28.i596:                                    ; preds = %if.end.i588
  %arrayidx34.i594 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom33.i593
  store i32 %i92, ptr %arrayidx34.i594, align 4
  %cmp.i595 = icmp sgt i32 %j.0.i579, %div.i552
  br i1 %cmp.i595, label %downheap_index.exit599, label %while.body.i562

downheap_index.exit599:                           ; preds = %if.end28.i596, %if.end.i588, %while.body.i550
  %idxprom36.pre-phi.i597 = phi i64 [ 0, %while.body.i550 ], [ %idxprom20.i581, %if.end28.i596 ], [ %idxprom33.i593, %if.end.i588 ]
  %arrayidx37.i598 = getelementptr inbounds i32, ptr %wn.0, i64 %idxprom36.pre-phi.i597
  store i32 %i85, ptr %arrayidx37.i598, align 4
  %cmp1.i549 = icmp sgt i64 %indvars.iv.next.i547, 1
  br i1 %cmp1.i549, label %while.body.i550, label %heapsort_index.exit551

heapsort_index.exit551:                           ; preds = %downheap_index.exit599, %while.cond.preheader.i540
  %incdec.ptr259 = getelementptr inbounds i8, ptr %call2, i64 24
  %i94 = bitcast ptr %kdptr to ptr
  store ptr %incdec.ptr259, ptr %i94, align 8
  %lo260 = getelementptr inbounds i8, ptr %call2, i64 8
  %i95 = bitcast ptr %lo260 to ptr
  store ptr null, ptr %i95, align 8
  %hi261 = getelementptr inbounds i8, ptr %call2, i64 16
  %i96 = bitcast ptr %hi261 to ptr
  store ptr null, ptr %i96, align 8
  %sub262 = add nsw i32 %natom, -1
  call fastcc void @buildkdtree(ptr %call14, ptr %call5, ptr %call6, ptr %call7, ptr %wn.0, ptr %call8, i32 0, i32 %sub262, ptr nonnull %kdptr, ptr %i3, ptr %x, i32 0, i32 4)
  br label %if.end32

if.end32:                                         ; preds = %heapsort_index.exit551, %for.end.split
  %call33 = call ptr @ivector(i32 0, i32 %natom)
  %call34 = call ptr @ivector(i32 0, i32 %natom)
  br label %for.cond35

for.cond35:                                       ; preds = %for.end117, %if.end32
  %totpair.0 = phi i32 [ 0, %if.end32 ], [ %add119, %for.end117 ]
  %i.1 = phi i32 [ 0, %if.end32 ], [ %inc121, %for.end117 ]
  %cmp36 = icmp slt i32 %i.1, %natom
  br i1 %cmp36, label %for.body38, label %for.end122

for.body38:                                       ; preds = %for.cond35
  store i32 0, ptr %upcnt, align 4
  store i32 0, ptr %locnt, align 4
  call fastcc void @foo(ptr %i3, ptr %x, i32 0, i32 %i.1, ptr nonnull %locnt, ptr nonnull %upcnt, ptr %call33, ptr %call34, double %cutoff, double %mul, i32 %dim, ptr %frozen)
  %i97 = load i32, ptr %locnt, align 4
  call fastcc void @heapsort_pairs(ptr %call33, i32 %i97)
  %i98 = load i32, ptr %upcnt, align 4
  call fastcc void @heapsort_pairs(ptr %call34, i32 %i98)
  %i99 = zext i32 %i.1 to i64
  %arrayidx40 = getelementptr inbounds ptr, ptr %pearlist, i64 %i99
  %i100 = load ptr, ptr %arrayidx40, align 8
  %cmp41 = icmp eq ptr %i100, null
  %i101 = load i32, ptr %locnt, align 4
  %i102 = load i32, ptr %upcnt, align 4
  %add = add nsw i32 %i101, %i102
  br i1 %cmp41, label %land.lhs.true, label %land.lhs.true54

land.lhs.true:                                    ; preds = %for.body38
  %cmp43 = icmp sgt i32 %add, 0
  br i1 %cmp43, label %if.then45, label %if.end86

if.then45:                                        ; preds = %land.lhs.true
  %call47 = call ptr @ivector(i32 0, i32 %add)
  store ptr %call47, ptr %arrayidx40, align 8
  br label %if.end86

land.lhs.true54:                                  ; preds = %for.body38
  %arrayidx57 = getelementptr inbounds i32, ptr %lpears, i64 %i99
  %i103 = load i32, ptr %arrayidx57, align 4
  %arrayidx59 = getelementptr inbounds i32, ptr %upears, i64 %i99
  %i104 = load i32, ptr %arrayidx59, align 4
  %add60 = add nsw i32 %i103, %i104
  %cmp61 = icmp sgt i32 %add, %add60
  br i1 %cmp61, label %if.then73, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %land.lhs.true54
  %mul64 = shl nsw i32 %add, 2
  %mul70 = mul nsw i32 %add60, 3
  %cmp71 = icmp slt i32 %mul64, %mul70
  br i1 %cmp71, label %if.then73, label %if.end86

if.then73:                                        ; preds = %lor.lhs.false, %land.lhs.true54
  call void @free_ivector(ptr nonnull %i100, i32 0, i32 %add60)
  %i105 = load i32, ptr %locnt, align 4
  %i106 = load i32, ptr %upcnt, align 4
  %add81 = add nsw i32 %i105, %i106
  %call82 = call ptr @ivector(i32 0, i32 %add81)
  store ptr %call82, ptr %arrayidx40, align 8
  br label %if.end86

if.end86:                                         ; preds = %if.then73, %lor.lhs.false, %if.then45, %land.lhs.true
  %i107 = load i32, ptr %locnt, align 4
  %arrayidx88 = getelementptr inbounds i32, ptr %lpears, i64 %i99
  store i32 %i107, ptr %arrayidx88, align 4
  %i108 = load i32, ptr %upcnt, align 4
  %arrayidx90 = getelementptr inbounds i32, ptr %upears, i64 %i99
  store i32 %i108, ptr %arrayidx90, align 4
  br label %for.cond91

for.cond91:                                       ; preds = %for.body94, %if.end86
  %j.0 = phi i32 [ 0, %if.end86 ], [ %inc102, %for.body94 ]
  %i109 = load i32, ptr %locnt, align 4
  %cmp92 = icmp slt i32 %j.0, %i109
  br i1 %cmp92, label %for.body94, label %for.cond104

for.body94:                                       ; preds = %for.cond91
  %i110 = zext i32 %j.0 to i64
  %arrayidx96 = getelementptr inbounds i32, ptr %call33, i64 %i110
  %i111 = load i32, ptr %arrayidx96, align 4
  %i112 = load ptr, ptr %arrayidx40, align 8
  %arrayidx100 = getelementptr inbounds i32, ptr %i112, i64 %i110
  store i32 %i111, ptr %arrayidx100, align 4
  %inc102 = add nuw nsw i32 %j.0, 1
  br label %for.cond91

for.cond104:                                      ; preds = %for.body107, %for.cond91
  %j.1 = phi i32 [ %inc116, %for.body107 ], [ 0, %for.cond91 ]
  %i113 = load i32, ptr %upcnt, align 4
  %cmp105 = icmp slt i32 %j.1, %i113
  %i114 = load i32, ptr %locnt, align 4
  br i1 %cmp105, label %for.body107, label %for.end117

for.body107:                                      ; preds = %for.cond104
  %i115 = zext i32 %j.1 to i64
  %arrayidx109 = getelementptr inbounds i32, ptr %call34, i64 %i115
  %i116 = load i32, ptr %arrayidx109, align 4
  %i117 = load ptr, ptr %arrayidx40, align 8
  %add112 = add nsw i32 %i114, %j.1
  %idxprom113 = sext i32 %add112 to i64
  %arrayidx114 = getelementptr inbounds i32, ptr %i117, i64 %idxprom113
  store i32 %i116, ptr %arrayidx114, align 4
  %inc116 = add nuw nsw i32 %j.1, 1
  br label %for.cond104

for.end117:                                       ; preds = %for.cond104
  %add118 = add nsw i32 %i114, %i113
  %add119 = add nsw i32 %totpair.0, %add118
  %inc121 = add nuw nsw i32 %i.1, 1
  br label %for.cond35

for.end122:                                       ; preds = %for.cond35
  call void @free_ivector(ptr %call33, i32 0, i32 %natom)
  call void @free_ivector(ptr %call34, i32 0, i32 %natom)
  call void @free(ptr %call2)
  call void @free_ivector(ptr %call5, i32 0, i32 %natom)
  call void @free_ivector(ptr %call6, i32 0, i32 %natom)
  call void @free_ivector(ptr %call7, i32 0, i32 %natom)
  call void @free_ivector(ptr %call8, i32 0, i32 %natom)
  br i1 %cmp9, label %if.then125, label %if.end126

if.then125:                                       ; preds = %for.end122
  call void @free_ivector(ptr %wn.0, i32 0, i32 %natom)
  br label %if.end126

if.end126:                                        ; preds = %if.then125, %for.end122
  call void @free_ivector(ptr %call14, i32 0, i32 %natom)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i2)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i1)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i)
  ret i32 %totpair.0
}

define internal fastcc void @foo(ptr nocapture readonly %that, ptr readonly %x, i32 %p, i32 %q, ptr %loindexp, ptr %upindexp, ptr %lopairlist, ptr %uppairlist, double %cut, double %cut2, i32 %dim, ptr readonly %frozen) unnamed_addr {
entry:
  %add = add nsw i32 %dim, 1
  %rem = srem i32 %p, %add
  %cmp = icmp eq i32 %rem, 0
  %hi = getelementptr inbounds %struct.kdnode, ptr %that, i64 0, i32 2
  %i = load ptr, ptr %hi, align 8
  %cmp1 = icmp eq ptr %i, null
  br i1 %cmp, label %land.lhs.true, label %land.lhs.true3

land.lhs.true:                                    ; preds = %entry
  br i1 %cmp1, label %if.end, label %if.then

land.lhs.true3:                                   ; preds = %entry
  br i1 %cmp1, label %if.end, label %land.lhs.true6

land.lhs.true6:                                   ; preds = %land.lhs.true3
  %mul = mul nsw i32 %dim, %q
  %add7 = add nsw i32 %mul, %rem
  %sub = add nsw i32 %add7, -1
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds double, ptr %x, i64 %idxprom
  %i1 = load double, ptr %arrayidx, align 8
  %add8 = fadd fast double %i1, %cut
  %n = getelementptr inbounds %struct.kdnode, ptr %that, i64 0, i32 0
  %i2 = load i32, ptr %n, align 8
  %mul9 = mul nsw i32 %i2, %dim
  %add10 = add nsw i32 %mul9, %rem
  %sub11 = add nsw i32 %add10, -1
  %idxprom12 = sext i32 %sub11 to i64
  %arrayidx13 = getelementptr inbounds double, ptr %x, i64 %idxprom12
  %i3 = load double, ptr %arrayidx13, align 8
  %cmp14 = fcmp fast ult double %add8, %i3
  br i1 %cmp14, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true6, %land.lhs.true
  %add16 = add nsw i32 %rem, 1
  tail call fastcc void @foo(ptr nonnull %i, ptr %x, i32 %add16, i32 %q, ptr %loindexp, ptr %upindexp, ptr %lopairlist, ptr %uppairlist, double %cut, double %cut2, i32 %dim, ptr %frozen)
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true6, %land.lhs.true3, %land.lhs.true
  %n17 = getelementptr inbounds %struct.kdnode, ptr %that, i64 0, i32 0
  %i4 = load i32, ptr %n17, align 8
  %cmp18 = icmp eq i32 %i4, %q
  br i1 %cmp18, label %if.end92, label %land.lhs.true19

land.lhs.true19:                                  ; preds = %if.end
  %idxprom20 = sext i32 %q to i64
  %arrayidx21 = getelementptr inbounds i32, ptr %frozen, i64 %idxprom20
  %i5 = load i32, ptr %arrayidx21, align 4
  %tobool = icmp eq i32 %i5, 0
  br i1 %tobool, label %if.then27, label %lor.lhs.false22

lor.lhs.false22:                                  ; preds = %land.lhs.true19
  %idxprom24 = sext i32 %i4 to i64
  %arrayidx25 = getelementptr inbounds i32, ptr %frozen, i64 %idxprom24
  %i6 = load i32, ptr %arrayidx25, align 4
  %tobool26 = icmp eq i32 %i6, 0
  br i1 %tobool26, label %if.then27, label %if.end92

if.then27:                                        ; preds = %lor.lhs.false22, %land.lhs.true19
  %mul28 = mul nsw i32 %dim, %q
  %idxprom30 = sext i32 %mul28 to i64
  %arrayidx31 = getelementptr inbounds double, ptr %x, i64 %idxprom30
  %i7 = load double, ptr %arrayidx31, align 8
  %mul33 = mul nsw i32 %i4, %dim
  %idxprom35 = sext i32 %mul33 to i64
  %arrayidx36 = getelementptr inbounds double, ptr %x, i64 %idxprom35
  %i8 = load double, ptr %arrayidx36, align 8
  %sub37 = fsub fast double %i7, %i8
  %add39 = add nsw i32 %mul28, 1
  %idxprom40 = sext i32 %add39 to i64
  %arrayidx41 = getelementptr inbounds double, ptr %x, i64 %idxprom40
  %i9 = load double, ptr %arrayidx41, align 8
  %add44 = add nsw i32 %mul33, 1
  %idxprom45 = sext i32 %add44 to i64
  %arrayidx46 = getelementptr inbounds double, ptr %x, i64 %idxprom45
  %i10 = load double, ptr %arrayidx46, align 8
  %sub47 = fsub fast double %i9, %i10
  %add49 = add nsw i32 %mul28, 2
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx51 = getelementptr inbounds double, ptr %x, i64 %idxprom50
  %i11 = load double, ptr %arrayidx51, align 8
  %add54 = add nsw i32 %mul33, 2
  %idxprom55 = sext i32 %add54 to i64
  %arrayidx56 = getelementptr inbounds double, ptr %x, i64 %idxprom55
  %i12 = load double, ptr %arrayidx56, align 8
  %sub57 = fsub fast double %i11, %i12
  %mul58 = fmul fast double %sub37, %sub37
  %mul59 = fmul fast double %sub47, %sub47
  %add60 = fadd fast double %mul58, %mul59
  %mul61 = fmul fast double %sub57, %sub57
  %add62 = fadd fast double %add60, %mul61
  %cmp63 = icmp eq i32 %dim, 4
  br i1 %cmp63, label %if.then64, label %if.end77

if.then64:                                        ; preds = %if.then27
  %add66 = add nsw i32 %mul28, 3
  %idxprom67 = sext i32 %add66 to i64
  %arrayidx68 = getelementptr inbounds double, ptr %x, i64 %idxprom67
  %i13 = load double, ptr %arrayidx68, align 8
  %add71 = add nsw i32 %mul33, 3
  %idxprom72 = sext i32 %add71 to i64
  %arrayidx73 = getelementptr inbounds double, ptr %x, i64 %idxprom72
  %i14 = load double, ptr %arrayidx73, align 8
  %sub74 = fsub fast double %i13, %i14
  %mul75 = fmul fast double %sub74, %sub74
  %add76 = fadd fast double %add62, %mul75
  br label %if.end77

if.end77:                                         ; preds = %if.then64, %if.then27
  %r2.0 = phi double [ %add76, %if.then64 ], [ %add62, %if.then27 ]
  %cmp78 = fcmp fast olt double %r2.0, %cut2
  br i1 %cmp78, label %if.then79, label %if.end92

if.then79:                                        ; preds = %if.end77
  %cmp81 = icmp slt i32 %i4, %q
  br i1 %cmp81, label %if.then82, label %if.else

if.then82:                                        ; preds = %if.then79
  %i15 = load i32, ptr %loindexp, align 4
  %idxprom84 = sext i32 %i15 to i64
  %arrayidx85 = getelementptr inbounds i32, ptr %lopairlist, i64 %idxprom84
  store i32 %i4, ptr %arrayidx85, align 4
  %i16 = load i32, ptr %loindexp, align 4
  %inc = add nsw i32 %i16, 1
  store i32 %inc, ptr %loindexp, align 4
  br label %if.end92

if.else:                                          ; preds = %if.then79
  %i17 = load i32, ptr %upindexp, align 4
  %idxprom87 = sext i32 %i17 to i64
  %arrayidx88 = getelementptr inbounds i32, ptr %uppairlist, i64 %idxprom87
  store i32 %i4, ptr %arrayidx88, align 4
  %i18 = load i32, ptr %upindexp, align 4
  %inc89 = add nsw i32 %i18, 1
  store i32 %inc89, ptr %upindexp, align 4
  br label %if.end92

if.end92:                                         ; preds = %if.else, %if.then82, %if.end77, %lor.lhs.false22, %if.end
  %lo = getelementptr inbounds %struct.kdnode, ptr %that, i64 0, i32 1
  %i19 = load ptr, ptr %lo, align 8
  %cmp95 = icmp eq ptr %i19, null
  br i1 %cmp, label %land.lhs.true94, label %land.lhs.true98

land.lhs.true94:                                  ; preds = %if.end92
  br i1 %cmp95, label %if.end118, label %if.then115

land.lhs.true98:                                  ; preds = %if.end92
  br i1 %cmp95, label %if.end118, label %land.lhs.true101

land.lhs.true101:                                 ; preds = %land.lhs.true98
  %mul102 = mul nsw i32 %dim, %q
  %add103 = add nsw i32 %mul102, %rem
  %sub104 = add nsw i32 %add103, -1
  %idxprom105 = sext i32 %sub104 to i64
  %arrayidx106 = getelementptr inbounds double, ptr %x, i64 %idxprom105
  %i20 = load double, ptr %arrayidx106, align 8
  %sub107 = fsub fast double %i20, %cut
  %i21 = load i32, ptr %n17, align 8
  %mul109 = mul nsw i32 %i21, %dim
  %add110 = add nsw i32 %mul109, %rem
  %sub111 = add nsw i32 %add110, -1
  %idxprom112 = sext i32 %sub111 to i64
  %arrayidx113 = getelementptr inbounds double, ptr %x, i64 %idxprom112
  %i22 = load double, ptr %arrayidx113, align 8
  %cmp114 = fcmp fast olt double %sub107, %i22
  br i1 %cmp114, label %if.then115, label %if.end118

if.then115:                                       ; preds = %land.lhs.true101, %land.lhs.true94
  %add117 = add nsw i32 %rem, 1
  tail call fastcc void @foo(ptr nonnull %i19, ptr %x, i32 %add117, i32 %q, ptr %loindexp, ptr %upindexp, ptr %lopairlist, ptr %uppairlist, double %cut, double %cut2, i32 %dim, ptr %frozen)
  ret void

if.end118:                                        ; preds = %land.lhs.true101, %land.lhs.true98, %land.lhs.true94
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
