; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -passes=inlinereportsetup -inline-report=0x186 < %s -S 2>&1 | opt -passes='cgscc(inline)' -inline-report=0x186 -S | opt -passes=inlinereportemitter -inline-report=0x186 -S 2>&1 | FileCheck --check-prefix=CHECK-MD-NEW %s

; Check that @I241872, which before inlining, has two calls to itself, is
; inlined once and that the opt report correctly indicates the created
; callsites and inlining reasons.

; NOTE: The old and new pass manager results differ because the old pass
; manager has the following code in Inliner.cpp that is not replicated for
; the new pass manager.
; 
; // Now that we have all of the call sites, move the ones to functions in the
; // current SCC to the end of the list.
; unsigned FirstCallInSCC = CallSites.size();
; for (unsigned I = 0; I < FirstCallInSCC; ++I)
;   if (Function *F = CallSites[I].first->getCalledFunction())
;     if (SCCFunctions.count(F))
;       std::swap(CallSites[I--], CallSites[--FirstCallInSCC]);

; Checks for old pass manager

; CHECK-OLD: COMPILE FUNC: I241872
; CHECK-OLD: I241872{{.*}}Callee has recursion
; CHECK-OLD: INLINE: I241872{{.*}}<<Inlining is profitable>>
; CHECK-OLD: I241872{{.*}}Callee has recursion
; CHECK-OLD: I241872{{.*}}Callee has recursion
; CHECK-OLD: COMPILE FUNC: I74060
; CHECK-OLD: I241872{{.*}}Callee has recursion

; Checks for new pass manager

; CHECK-NEW: COMPILE FUNC: I241872
; CHECK-NEW: INLINE: I241872{{.*}}<<Inlining is profitable>>
; CHECK-NEW: I241872{{.*}}Callee has recursion
; CHECK-NEW: I241872{{.*}}Callee has recursion
; CHECK-NEW: I241872{{.*}}Callee has recursion
; CHECK-NEW: COMPILE FUNC: I74060
; CHECK-NEW: I241872{{.*}}Callee has recursion

; Checks for old pass manager (metadata report)

; CHECK-MD-OLD: COMPILE FUNC: I74060
; CHECK-MD-OLD: I241872{{.*}}Callee has recursion
; CHECK-MD-OLD: COMPILE FUNC: I241872
; CHECK-MD-OLD: I241872{{.*}}Callee has recursion
; CHECK-MD-OLD: INLINE: I241872{{.*}}<<Inlining is profitable>>
; CHECK-MD-OLD: I241872{{.*}}Callee has recursion
; CHECK-MD-OLD: I241872{{.*}}Callee has recursion

; Checks for new pass manager (metadata report)

; CHECK-MD-NEW: COMPILE FUNC: I74060
; CHECK-MD-NEW: I241872{{.*}}Callee has recursion
; CHECK-MD-NEW: COMPILE FUNC: I241872
; CHECK-MD-NEW: INLINE: I241872{{.*}}<<Inlining is profitable>>
; CHECK-MD-NEW: I241872{{.*}}Callee has recursion
; CHECK-MD-NEW: I241872{{.*}}Callee has recursion
; CHECK-MD-NEW: I241872{{.*}}Callee has recursion

%struct.sm1 = type { i8, i8 }
%struct.sm2 = type { i8, i8 }
%struct.sm8 = type { i32 }
%struct.sm3 = type { i16, [2 x i8] }
%struct.sm4 = type { i8, i8 }
%struct.sm6 = type { i8, i8, i8, i8 }
%struct.sm5 = type { i16, i16 }

define dso_local void @I74060(i8* %ptr, i32 %I00740, i32 %I24098, i32 %I05828, i8* %I01493) local_unnamed_addr {
entry:
  call void @I241872(i8* %ptr, i32 %I00740, i32 0, i32 %I24098, i32 %I05828, i8* %I01493)
  ret void
}
define dso_local void @I241872(i8* %ptr, i32 %I00740, i32 %I05316, i32 %I24098, i32 %I05828, i8* %I01493) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %I05316, 16
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i32 %I00740, 100
  br i1 %cmp1, label %land.lhs.true, label %if.end5

land.lhs.true:                                    ; preds = %if.end
  %0 = load i8, i8* %ptr, align 1
  %cmp2 = icmp eq i8 %0, 16
  %spec.select = select i1 %cmp2, i32 101, i32 100
  br label %if.end20

if.end5:                                          ; preds = %if.end
  %I00740.addr.0 = phi i32 [ %I00740, %if.end ]
  %cmp6 = icmp eq i32 %I00740.addr.0, 102
  br i1 %cmp6, label %land.lhs.true8, label %if.end13

land.lhs.true8:                                   ; preds = %if.end5
  %1 = getelementptr inbounds i8, i8* %ptr, i64 1
  %2 = load i8, i8* %1, align 1
  %cmp10 = icmp eq i8 %2, 1
  %spec.select144 = select i1 %cmp10, i32 103, i32 102
  br label %if.end20

if.end13:                                         ; preds = %if.end5
  %I00740.addr.1 = phi i32 [ %I00740.addr.0, %if.end5 ]
  %cmp14 = icmp eq i32 %I00740.addr.1, 104
  br i1 %cmp14, label %land.lhs.true16, label %if.end20

land.lhs.true16:                                  ; preds = %if.end13
  %I61599 = bitcast i8* %ptr to i32*
  %3 = load i32, i32* %I61599, align 4
  %cmp17 = icmp eq i32 %3, 12
  %spec.select145 = select i1 %cmp17, i32 105, i32 104
  br label %if.end20

if.end20:                                         ; preds = %land.lhs.true16, %land.lhs.true8, %land.lhs.true, %if.end13
  %I00740.addr.2 = phi i32 [ %I00740.addr.1, %if.end13 ], [ %spec.select, %land.lhs.true ], [ %spec.select144, %land.lhs.true8 ], [ %spec.select145, %land.lhs.true16 ]
  %cmp21 = icmp eq i32 %I00740, 97
  %cmp23 = icmp eq i32 %I00740.addr.2, 97
  %or.cond = or i1 %cmp21, %cmp23
  br i1 %or.cond, label %for.cond, label %if.end34

for.cond:                                         ; preds = %if.end20, %for.body
  %I05536.0 = phi i32 [ %inc, %for.body ], [ 0, %if.end20 ]
  %I01615 = bitcast i8* %ptr to i16*
  %4 = load i16, i16* %I01615, align 2
  %conv26 = zext i16 %4 to i32
  %sub = add nsw i32 %conv26, -2
  %cmp27 = icmp sgt i32 %sub, 0
  %spec.select146 = select i1 %cmp27, i32 %sub, i32 0
  %cmp32 = icmp slt i32 %I05536.0, %spec.select146
  br i1 %cmp32, label %for.body, label %if.end34

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i8, i8* %ptr, i64 4
  %5 = zext i32 %I05536.0 to i64
  %add.ptr = getelementptr inbounds i8, i8* %arrayidx, i64 %5
  %add = add nsw i32 %I05316, 1
  call void @I241872(i8* nonnull %add.ptr, i32 96, i32 %add, i32 %I24098, i32 %I05828, i8* %I01493)
  %inc = add nuw nsw i32 %I05536.0, 1
  br label %for.cond

if.end34:                                         ; preds = %for.cond, %if.end20
  %cmp35 = icmp eq i32 %I00740, 99
  %cmp38 = icmp eq i32 %I00740.addr.2, 99
  %or.cond105 = or i1 %cmp35, %cmp38
  br i1 %or.cond105, label %for.cond42, label %cleanup

for.cond42:                                       ; preds = %if.end34, %cond.end94
  %I0553641.0 = phi i32 [ %inc101, %cond.end94 ], [ 0, %if.end34 ]
  %6 = getelementptr inbounds i8, i8* %ptr, i64 1
  %7 = load i8, i8* %6, align 1
  %conv43 = sext i8 %7 to i32
  %cmp44 = icmp slt i32 %I0553641.0, %conv43
  br i1 %cmp44, label %for.body46, label %cleanup

for.body46:                                       ; preds = %for.cond42
  %8 = load i8, i8* %ptr, align 1
  %9 = and i8 %8, 64
  %tobool = icmp eq i8 %9, 0
  br i1 %tobool, label %cond.end94, label %land.lhs.true58

land.lhs.true58:                                  ; preds = %for.body46
  %10 = getelementptr inbounds i8, i8* %ptr, i64 3
  %11 = load i8, i8* %10, align 1
  %12 = and i8 %11, 35
  %cmp61 = icmp eq i8 %12, 32
  br i1 %cmp61, label %cond.end94, label %lor.lhs.false63

lor.lhs.false63:                                  ; preds = %land.lhs.true58
  %13 = load i8, i8* %ptr, align 1
  %14 = and i8 %13, 64
  %tobool67 = icmp eq i8 %14, 0
  br i1 %tobool67, label %cond.false77, label %land.lhs.true70

land.lhs.true70:                                  ; preds = %lor.lhs.false63
  %15 = getelementptr inbounds i8, i8* %ptr, i64 3
  %16 = load i8, i8* %15, align 1
  %17 = and i8 %16, 11
  %cmp74 = icmp eq i8 %17, 8
  br i1 %cmp74, label %cond.end94, label %cond.false77

cond.false77:                                     ; preds = %lor.lhs.false63, %land.lhs.true70
  %18 = getelementptr inbounds i8, i8* %ptr, i64 1
  %19 = load i8, i8* %18, align 1
  %conv78 = zext i8 %19 to i64
  %mul = shl nuw nsw i64 %conv78, 1
  %add79 = add nuw nsw i64 %mul, 4
  %20 = getelementptr inbounds i8, i8* %ptr, i64 3
  %21 = load i8, i8* %20, align 1
  %22 = and i8 %21, 16
  %tobool83 = icmp eq i8 %22, 0
  %cond84 = select i1 %tobool83, i64 1, i64 8
  %23 = getelementptr inbounds i8, i8* %ptr, i64 2
  %24 = load i8, i8* %23, align 1
  %conv85 = zext i8 %24 to i64
  %mul86 = mul nuw nsw i64 %cond84, %conv85
  %add87 = add nuw nsw i64 %add79, %mul86
  %add90 = add nuw nsw i64 %add87, 3
  %and91 = and i64 %add90, 16380
  br label %cond.end94

cond.end94:                                       ; preds = %for.body46, %cond.false77, %land.lhs.true70, %land.lhs.true58
  %cond95 = phi i64 [ 2, %for.body46 ], [ %and91, %cond.false77 ], [ 4, %land.lhs.true70 ], [ 4, %land.lhs.true58 ]
  %add.ptr96 = getelementptr inbounds i8, i8* %ptr, i64 %cond95
  %25 = bitcast i8* %add.ptr96 to %struct.sm5*
  %26 = zext i32 %I0553641.0 to i64
  %add.ptr98 = getelementptr inbounds %struct.sm5, %struct.sm5* %25, i64 %26
  %27 = bitcast %struct.sm5* %add.ptr98 to i8*
  %add99 = add nsw i32 %I05316, 1
  call void @I241872(i8* %27, i32 98, i32 %add99, i32 %I24098, i32 %I05828, i8* %I01493)
  %inc101 = add nuw nsw i32 %I0553641.0, 1
  br label %for.cond42

cleanup:                                          ; preds = %if.end34, %for.cond42, %entry
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
