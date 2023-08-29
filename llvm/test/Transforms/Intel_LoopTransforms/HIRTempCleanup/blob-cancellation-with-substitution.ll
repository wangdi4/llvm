; RUN: opt  < %s -passes="hir-ssa-deconstruction,print<hir>,hir-temp-cleanup,print<hir>" 2>&1 | FileCheck %s

; Verify that the use ref in the HLIf is made consistent when these two blobs get
; cancelled after substitution-
; 1. -1 * ptrtoint.ptr.i64(%pLineSrc.191713)
; 2. ptrtoint.ptr.i64(%pLineSrc.191713.out)

; This test was compfailing because verifier was asserting on inconsistent def level.

; The ref is using both values because SSA deconstruction did not replace use of
; %pLineSrc.191713 in %sub.ptr.rhs.cast827 with the liveout copy as it was able
; to deduce that it doesn't cause live range violation.

; Print Before
; CHECK: Function

; CHECK: + DO i1 = 0, -1 * %spec.select + smax((1 + %spec.select), %i3) + -1, 1   <DO_LOOP>
; CHECK: |   %pLineSrc.191713.out = &((%pLineSrc.191713)[0]);
; CHECK: |   %i259 = (%pLineSrc.191713.out)[%idx];
; CHECK: |   %src1.19 = 0;
; CHECK: |   if (%idx + -1 * ptrtoint.ptr.i64(%pLineSrc.191713) + ptrtoint.ptr.i64(%pLineSrc.191713.out) + 1 < %i258)
; CHECK: |   {
; CHECK: |      %i261 = (%pLineSrc.191713.out)[%idx + 1];
; CHECK: |      %src1.19 = %i261;
; CHECK: |   }
; CHECK: |   %i263 = (%m_nStride)[0];
; CHECK: |   %pLineSrc.191713 = &((%pLineSrc.191713)[%i263]);
; CHECK: |   %i258 = %i263;
; CHECK: + END LOOP

; Print After
; CHECK: Function

; CHECK: + DO i1 = 0, -1 * %spec.select + smax((1 + %spec.select), %i3) + -1, 1   <DO_LOOP>
; CHECK: |   %src1.19 = 0;
; CHECK: |   if (%idx + 1 < %i258)
; CHECK: |   {
; CHECK: |      %i261 = (%pLineSrc.191713)[%idx + 1];
; CHECK: |      %src1.19 = %i261;
; CHECK: |   }
; CHECK: |   %i263 = (%m_nStride)[0];
; CHECK: |   %pLineSrc.191713 = &((%pLineSrc.191713)[%i263]);
; CHECK: |   %i258 = %i263;
; CHECK: + END LOOP

define void @foo(i32 %i3, i32 %i9, ptr %add.ptr, i32 %spec.select, i64 %idx, ptr %m_nStride) {
entry:
  br label %for.body802

for.body802:                                      ; preds = %if.end836, %entry
  %i258 = phi i32 [ %i263, %if.end836 ], [ %i9, %entry ]
  %pLineSrc.191713 = phi ptr [ %add.ptr850, %if.end836 ], [ %add.ptr, %entry ]
  %yy.191712 = phi i32 [ %inc852, %if.end836 ], [ %spec.select, %entry ]
  %add.ptr806 = getelementptr inbounds i8, ptr %pLineSrc.191713, i64 %idx
  %i259 = load i8, ptr %add.ptr806, align 1
  %conv808 = zext i8 %i259 to i32
  %srcPtr.91701 = getelementptr inbounds i8, ptr %add.ptr806, i64 1
  %shl8371703 = shl nuw nsw i32 %conv808, 8
  %sub.ptr.lhs.cast826 = ptrtoint ptr %srcPtr.91701 to i64
  %sub.ptr.rhs.cast827 = ptrtoint ptr %pLineSrc.191713 to i64
  %sub.ptr.sub828 = sub i64 %sub.ptr.lhs.cast826, %sub.ptr.rhs.cast827
  %conv830 = sext i32 %i258 to i64
  %cmp831 = icmp slt i64 %sub.ptr.sub828, %conv830
  br i1 %cmp831, label %if.then832, label %if.end836

if.then832:                                       ; preds = %for.body802
  %i261 = load i8, ptr %srcPtr.91701, align 1
  %conv834 = zext i8 %i261 to i32
  br label %if.end836

if.end836:                                        ; preds = %if.then832, %for.body802
  %src1.19 = phi i32 [ %conv834, %if.then832 ], [ 0, %for.body802 ]
  %i263 = load i32, ptr %m_nStride, align 8
  %idx.ext849 = sext i32 %i263 to i64
  %add.ptr850 = getelementptr inbounds i8, ptr %pLineSrc.191713, i64 %idx.ext849
  %inc852 = add nsw i32 %yy.191712, 1
  %cmp801 = icmp slt i32 %inc852, %i3
  br i1 %cmp801, label %for.body802, label %cleanup.loopexit1846

cleanup.loopexit1846:
  %src1.lcssa = phi i32 [ %src1.19, %if.end836 ]
  ret void
}
