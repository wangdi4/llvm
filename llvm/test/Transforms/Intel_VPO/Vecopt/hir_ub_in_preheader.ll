;
; Test for that there is no crash in vectorizer when a complicated upper bound requires
; an instruction emission in the loop preheader.
;
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
;
; HIR input
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
;       + DO i1 = 0, umin((-1 + (-1 * %beg) + umax((1 + %beg), (sext.i32.i64(%size) + %beg))), (-1 + (-1 * %beg) + %4)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967292>
;       |   %6 = (%beg)[i1];
;       |   %all.069 = %all.069  |  %6; <Safe Reduction>
;       + END LOOP
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ] 
; END REGION
;
; BEGIN REGION { }
;       %entry.region1 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
;       + DO i1 = 0, (-1 * %1 + %3 + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>
;       |   %8 = (%1)[i1];
;       |   %all.165 = %8  |  %all.165; <Safe Reduction>
;       + END LOOP
;       @llvm.directive.region.exit(%entry.region1); [ DIR.VPO.END.AUTO.VEC() ] 
; END REGION
;
; BEGIN REGION { }
;       %entry.region2 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
;       + DO i1 = 0, %beg + sext.i32.i64(%size) + -1 * %9 + -1, 1   <DO_LOOP>
;       |   %10 = (%9)[i1];
;       |   %all.261 = %all.261  |  %10; <Safe Reduction>
;       + END LOOP
;       @llvm.directive.region.exit(%entry.region2); [ DIR.VPO.END.AUTO.VEC() ] 
; END REGION
;
;CHECK:  auto-vectorized
;CHECK:  auto-vectorized
;CHECK:  auto-vectorized
;
; Function Attrs: nounwind uwtable mustprogress
define hidden zeroext i1 @_ZN11__sanitizer11mem_is_zeroEPKcm(i8* %beg, i32 %size) local_unnamed_addr  {
entry:
  br label %if.end

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds i8, i8* %beg, i32 %size
  %0 = ptrtoint i8* %beg to i32
  %sub.i = add i32 %0, 3
  %and.i = and i32 %sub.i, -4
  %1 = inttoptr i32 %and.i to i32*
  %2 = ptrtoint i8* %add.ptr to i32
  %and.i59 = and i32 %2, -4
  %3 = inttoptr i32 %and.i59 to i32*
  %4 = inttoptr i32 %and.i to i8*
  %cmp567 = icmp ugt i8* %4, %beg
  %cmp668 = icmp ne i32 %size, 0
  %5 = and i1 %cmp567, %cmp668
  br i1 %5, label %for.body.preheader, label %for.cond8.preheader

for.body.preheader:                               ; preds = %if.end
  br label %for.body

for.cond8.preheader.loopexit:                     ; preds = %for.body
  %or.lcssa = phi i32 [ %or, %for.body ]
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.cond8.preheader.loopexit, %if.end
  %all.0.lcssa = phi i32 [ 0, %if.end ], [ %or.lcssa, %for.cond8.preheader.loopexit ]
  %cmp963 = icmp ult i32* %1, %3
  br i1 %cmp963, label %for.body10.preheader, label %for.end14

for.body10.preheader:                             ; preds = %for.cond8.preheader
  br label %for.body10

for.body:                                         ; preds = %for.body, %for.body.preheader
  %mem.070 = phi i8* [ %incdec.ptr, %for.body ], [ %beg, %for.body.preheader ]
  %all.069 = phi i32 [ %or, %for.body ], [ 0, %for.body.preheader ]
  %6 = load i8, i8* %mem.070, align 1
  %conv7 = sext i8 %6 to i32
  %or = or i32 %all.069, %conv7
  %incdec.ptr = getelementptr inbounds i8, i8* %mem.070, i32 1
  %cmp5 = icmp ult i8* %incdec.ptr, %4
  %cmp6 = icmp ult i8* %incdec.ptr, %add.ptr
  %7 = and i1 %cmp5, %cmp6
  br i1 %7, label %for.body, label %for.cond8.preheader.loopexit

for.body10:                                       ; preds = %for.body10, %for.body10.preheader
  %all.165 = phi i32 [ %or11, %for.body10 ], [ %all.0.lcssa, %for.body10.preheader ]
  %aligned_beg.064 = phi i32* [ %incdec.ptr13, %for.body10 ], [ %1, %for.body10.preheader ]
  %8 = load i32, i32* %aligned_beg.064, align 4
  %or11 = or i32 %8, %all.165
  %incdec.ptr13 = getelementptr inbounds i32, i32* %aligned_beg.064, i32 1
  %cmp9 = icmp ult i32* %incdec.ptr13, %3
  br i1 %cmp9, label %for.body10, label %for.end14.loopexit

for.end14.loopexit:                               ; preds = %for.body10
  %or11.lcssa = phi i32 [ %or11, %for.body10 ]
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %for.cond8.preheader
  %all.1.lcssa = phi i32 [ %all.0.lcssa, %for.cond8.preheader ], [ %or11.lcssa, %for.end14.loopexit ]
  %9 = inttoptr i32 %and.i59 to i8*
  %cmp15.not = icmp uge i8* %9, %beg
  %cmp1960 = icmp ugt i8* %add.ptr, %9
  %or.cond = and i1 %cmp15.not, %cmp1960
  br i1 %or.cond, label %for.body21.preheader, label %if.end27

for.body21.preheader:                             ; preds = %for.end14
  br label %for.body21

for.body21:                                       ; preds = %for.body21, %for.body21.preheader
  %mem17.062 = phi i8* [ %incdec.ptr25, %for.body21 ], [ %9, %for.body21.preheader ]
  %all.261 = phi i32 [ %or23, %for.body21 ], [ %all.1.lcssa, %for.body21.preheader ]
  %10 = load i8, i8* %mem17.062, align 1
  %conv22 = sext i8 %10 to i32
  %or23 = or i32 %all.261, %conv22
  %incdec.ptr25 = getelementptr inbounds i8, i8* %mem17.062, i32 1
  %exitcond.not = icmp eq i8* %incdec.ptr25, %add.ptr
  br i1 %exitcond.not, label %if.end27.loopexit, label %for.body21

if.end27.loopexit:                                ; preds = %for.body21
  %or23.lcssa = phi i32 [ %or23, %for.body21 ]
  br label %if.end27

if.end27:                                         ; preds = %if.end27.loopexit, %for.end14
  %all.3 = phi i32 [ %all.1.lcssa, %for.end14 ], [ %or23.lcssa, %if.end27.loopexit ]
  %cmp28 = icmp eq i32 %all.3, 0
  ret i1 %cmp28
}

