; RUN: opt -opaque-pointers -passes='cgscc(inline)' < %s -S 2>&1 | FileCheck %s

; Check that @I241872, which before inlining, has two calls to itself, is
; inlined once and is marked with the "no-more-recursive-inlining" attribute
; to inhibit further inlining.

; CHECK: define dso_local void @I74060
; CHECK: call void @I241872
; CHECK: define dso_local void @I241872{{.*}}#0
; CHECK: call void @I241872
; CHECK: call void @I241872
; CHECK: call void @I241872
; CHECK: attributes #0 = { "no-more-recursive-inlining" }

%struct.sm5 = type { i16, i16 }

define dso_local void @I74060(ptr %ptr, i32 %I00740, i32 %I24098, i32 %I05828, ptr %I01493) local_unnamed_addr {
entry:
  call void @I241872(ptr %ptr, i32 %I00740, i32 0, i32 %I24098, i32 %I05828, ptr %I01493)
  ret void
}

define dso_local void @I241872(ptr %ptr, i32 %I00740, i32 %I05316, i32 %I24098, i32 %I05828, ptr %I01493) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %I05316, 16
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i32 %I00740, 100
  br i1 %cmp1, label %land.lhs.true, label %if.end5

land.lhs.true:                                    ; preds = %if.end
  %i = load i8, ptr %ptr, align 1
  %cmp2 = icmp eq i8 %i, 16
  %spec.select = select i1 %cmp2, i32 101, i32 100
  br label %if.end20

if.end5:                                          ; preds = %if.end
  %I00740.addr.0 = phi i32 [ %I00740, %if.end ]
  %cmp6 = icmp eq i32 %I00740.addr.0, 102
  br i1 %cmp6, label %land.lhs.true8, label %if.end13

land.lhs.true8:                                   ; preds = %if.end5
  %i1 = getelementptr inbounds i8, ptr %ptr, i64 1
  %i2 = load i8, ptr %i1, align 1
  %cmp10 = icmp eq i8 %i2, 1
  %spec.select144 = select i1 %cmp10, i32 103, i32 102
  br label %if.end20

if.end13:                                         ; preds = %if.end5
  %I00740.addr.1 = phi i32 [ %I00740.addr.0, %if.end5 ]
  %cmp14 = icmp eq i32 %I00740.addr.1, 104
  br i1 %cmp14, label %land.lhs.true16, label %if.end20

land.lhs.true16:                                  ; preds = %if.end13
  %i3 = load i32, ptr %ptr, align 4
  %cmp17 = icmp eq i32 %i3, 12
  %spec.select145 = select i1 %cmp17, i32 105, i32 104
  br label %if.end20

if.end20:                                         ; preds = %land.lhs.true16, %if.end13, %land.lhs.true8, %land.lhs.true
  %I00740.addr.2 = phi i32 [ %I00740.addr.1, %if.end13 ], [ %spec.select, %land.lhs.true ], [ %spec.select144, %land.lhs.true8 ], [ %spec.select145, %land.lhs.true16 ]
  %cmp21 = icmp eq i32 %I00740, 97
  %cmp23 = icmp eq i32 %I00740.addr.2, 97
  %or.cond = or i1 %cmp21, %cmp23
  br i1 %or.cond, label %for.cond, label %if.end34

for.cond:                                         ; preds = %for.body, %if.end20
  %I05536.0 = phi i32 [ %inc, %for.body ], [ 0, %if.end20 ]
  %i4 = load i16, ptr %ptr, align 2
  %conv26 = zext i16 %i4 to i32
  %sub = add nsw i32 %conv26, -2
  %cmp27 = icmp sgt i32 %sub, 0
  %spec.select146 = select i1 %cmp27, i32 %sub, i32 0
  %cmp32 = icmp slt i32 %I05536.0, %spec.select146
  br i1 %cmp32, label %for.body, label %if.end34

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i8, ptr %ptr, i64 4
  %i5 = zext i32 %I05536.0 to i64
  %add.ptr = getelementptr inbounds i8, ptr %arrayidx, i64 %i5
  %add = add nsw i32 %I05316, 1
  call void @I241872(ptr nonnull %add.ptr, i32 96, i32 %add, i32 %I24098, i32 %I05828, ptr %I01493)
  %inc = add nuw nsw i32 %I05536.0, 1
  br label %for.cond

if.end34:                                         ; preds = %for.cond, %if.end20
  %cmp35 = icmp eq i32 %I00740, 99
  %cmp38 = icmp eq i32 %I00740.addr.2, 99
  %or.cond105 = or i1 %cmp35, %cmp38
  br i1 %or.cond105, label %for.cond42, label %cleanup

for.cond42:                                       ; preds = %cond.end94, %if.end34
  %I0553641.0 = phi i32 [ %inc101, %cond.end94 ], [ 0, %if.end34 ]
  %i6 = getelementptr inbounds i8, ptr %ptr, i64 1
  %i7 = load i8, ptr %i6, align 1
  %conv43 = sext i8 %i7 to i32
  %cmp44 = icmp slt i32 %I0553641.0, %conv43
  br i1 %cmp44, label %for.body46, label %cleanup

for.body46:                                       ; preds = %for.cond42
  %i8 = load i8, ptr %ptr, align 1
  %i9 = and i8 %i8, 64
  %tobool = icmp eq i8 %i9, 0
  br i1 %tobool, label %cond.end94, label %land.lhs.true58

land.lhs.true58:                                  ; preds = %for.body46
  %i10 = getelementptr inbounds i8, ptr %ptr, i64 3
  %i11 = load i8, ptr %i10, align 1
  %i12 = and i8 %i11, 35
  %cmp61 = icmp eq i8 %i12, 32
  br i1 %cmp61, label %cond.end94, label %lor.lhs.false63

lor.lhs.false63:                                  ; preds = %land.lhs.true58
  %i13 = load i8, ptr %ptr, align 1
  %i14 = and i8 %i13, 64
  %tobool67 = icmp eq i8 %i14, 0
  br i1 %tobool67, label %cond.false77, label %land.lhs.true70

land.lhs.true70:                                  ; preds = %lor.lhs.false63
  %i15 = getelementptr inbounds i8, ptr %ptr, i64 3
  %i16 = load i8, ptr %i15, align 1
  %i17 = and i8 %i16, 11
  %cmp74 = icmp eq i8 %i17, 8
  br i1 %cmp74, label %cond.end94, label %cond.false77

cond.false77:                                     ; preds = %land.lhs.true70, %lor.lhs.false63
  %i18 = getelementptr inbounds i8, ptr %ptr, i64 1
  %i19 = load i8, ptr %i18, align 1
  %conv78 = zext i8 %i19 to i64
  %mul = shl nuw nsw i64 %conv78, 1
  %add79 = add nuw nsw i64 %mul, 4
  %i20 = getelementptr inbounds i8, ptr %ptr, i64 3
  %i21 = load i8, ptr %i20, align 1
  %i22 = and i8 %i21, 16
  %tobool83 = icmp eq i8 %i22, 0
  %cond84 = select i1 %tobool83, i64 1, i64 8
  %i23 = getelementptr inbounds i8, ptr %ptr, i64 2
  %i24 = load i8, ptr %i23, align 1
  %conv85 = zext i8 %i24 to i64
  %mul86 = mul nuw nsw i64 %cond84, %conv85
  %add87 = add nuw nsw i64 %add79, %mul86
  %add90 = add nuw nsw i64 %add87, 3
  %and91 = and i64 %add90, 16380
  br label %cond.end94

cond.end94:                                       ; preds = %cond.false77, %land.lhs.true70, %land.lhs.true58, %for.body46
  %cond95 = phi i64 [ 2, %for.body46 ], [ %and91, %cond.false77 ], [ 4, %land.lhs.true70 ], [ 4, %land.lhs.true58 ]
  %add.ptr96 = getelementptr inbounds i8, ptr %ptr, i64 %cond95
  %i26 = zext i32 %I0553641.0 to i64
  %add.ptr98 = getelementptr inbounds %struct.sm5, ptr %add.ptr96, i64 %i26
  %add99 = add nsw i32 %I05316, 1
  call void @I241872(ptr %add.ptr98, i32 98, i32 %add99, i32 %I24098, i32 %I05828, ptr %I01493)
  %inc101 = add nuw nsw i32 %I0553641.0, 1
  br label %for.cond42

cleanup:                                          ; preds = %for.cond42, %if.end34, %entry
  ret void
}
