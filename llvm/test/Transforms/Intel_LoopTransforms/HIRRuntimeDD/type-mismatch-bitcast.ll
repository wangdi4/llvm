; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -print-after=hir-runtime-dd -hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that refs in the compare of segment with bounds of different types are casted to a compatible type.

; Here are the bounds of a single segment.
; &((i32*)(%pDst1)[sext.i32.i64(%div1) * i1 + 3 * ((-1 + smax(3, %0)) /u 3) + 2])
; &((%pDst1)[sext.i32.i64(%div1) * i1])

; BEGIN REGION { }
;       + DO i1 = 0, trunc.i64.i32((%roi.coerce /u 4294967296)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;       |   if (%roi.coerce > 0)
;       |   {
;       |      %res.168 = %res.074;
;       |
;       |      + DO i2 = 0, (smax(3, %0) + -1)/u3, 1   <DO_LOOP>  <MAX_TC_EST = 715827883>
;       |      |   + DO i3 = 0, 2, 1   <DO_LOOP>
;       |      |   |   %1 = (%v)[i3];
;       |      |   |   %3 = (%pSrc1)[sext.i32.i64(%div) * i1 + 3 * i2 + i3];
;       |      |   |   if (%1 !=u 0.000000e+00)
;       |      |   |   {
;       |      |   |      %div14 = %3  /  %1;
;       |      |   |      (%pDst1)[sext.i32.i64(%div1) * i1 + 3 * i2 + i3] = %div14;
;       |      |   |   }
;       |      |   |   else
;       |      |   |   {
;       |      |   |      %4 = (%3 == 0.000000e+00) ? -4194304 : 2139095040;
;       |      |   |      (i32*)(%pDst1)[sext.i32.i64(%div1) * i1 + 3 * i2 + i3] = %4;
;       |      |   |      %res.168 = 6;
;       |      |   |   }
;       |      |   + END LOOP
;       |      + END LOOP
;       |
;       |      %res.074 = %res.168;
;       |   }
;       + END LOOP
; END REGION

; %v %pDst1 %pSrc1

; CHECK: Function
; CHECK-DAG: = &((%v)[2]) >=u &((%pDst1)[sext.i32.i64(%div1) * i1]);
; CHECK-DAG: = &((i32*)(%pDst1)[sext.i32.i64(%div1) * i1 + 3 * ((-1 + smax(3, %0)) /u 3) + 2]) >=u &((i32*)(%v)[0]);
; CHECK: %mv.and = %mv.test  &&  %mv.test5;
; CHECK-DAG: = &((i32*)(%pDst1)[sext.i32.i64(%div1) * i1 + 3 * ((-1 + smax(3, %0)) /u 3) + 2]) >=u &((i32*)(%pSrc1)[sext.i32.i64(%div) * i1]);
; CHECK-DAG: = &((%pSrc1)[sext.i32.i64(%div) * i1 + 3 * ((-1 + smax(3, %0)) /u 3) + 2]) >=u &((%pDst1)[sext.i32.i64(%div1) * i1]);
; CHECK: %mv.and8 = %mv.test6  &&  %mv.test7;
; CHECK: if (%mv.and == 0 && %mv.and8 == 0)

; ModuleID = 'func.bc'
source_filename = "cq91196.cpp"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; Function Attrs: norecurse nounwind uwtable
define i32 @func123(float* nocapture readonly %pSrc1, i32 %src1Step, float* nocapture readonly %v, float* nocapture %pDst1, i32 %dst1Step, i64 %roi.coerce) local_unnamed_addr #0 {
entry:
  %roi.sroa.2.0.extract.shift = lshr i64 %roi.coerce, 32
  %roi.sroa.2.0.extract.trunc = trunc i64 %roi.sroa.2.0.extract.shift to i32
  %cmp70 = icmp sgt i32 %roi.sroa.2.0.extract.trunc, 0
  br i1 %cmp70, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %div1 = sdiv i32 %dst1Step, 4
  %div = sdiv i32 %src1Step, 4
  %roi.sroa.0.0.extract.trunc = trunc i64 %roi.coerce to i32
  %cmp367 = icmp sgt i32 %roi.sroa.0.0.extract.trunc, 0
  %idx.ext = sext i32 %div to i64
  %idx.ext34 = sext i32 %div1 to i64
  %sext = mul i64 %roi.coerce, 12884901888
  %0 = ashr exact i64 %sext, 32
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup4
  %res.1.lcssa.lcssa = phi i32 [ %res.1.lcssa, %for.cond.cleanup4 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.cond.cleanup4, %for.body.lr.ph
  %i.075 = phi i32 [ 0, %for.body.lr.ph ], [ %inc36, %for.cond.cleanup4 ]
  %res.074 = phi i32 [ 0, %for.body.lr.ph ], [ %res.1.lcssa, %for.cond.cleanup4 ]
  %src1.073 = phi float* [ %pSrc1, %for.body.lr.ph ], [ %add.ptr, %for.cond.cleanup4 ]
  %dst1.071 = phi float* [ %pDst1, %for.body.lr.ph ], [ %add.ptr35, %for.cond.cleanup4 ]
  br i1 %cmp367, label %for.body5.lr.ph, label %for.cond.cleanup4

for.body5.lr.ph:                                  ; preds = %for.body
  br label %for.body5

for.cond.cleanup4.loopexit:                       ; preds = %for.cond.cleanup8
  %res.3.lcssa.lcssa = phi i32 [ %res.3.lcssa, %for.cond.cleanup8 ]
  br label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond.cleanup4.loopexit, %for.body
  %res.1.lcssa = phi i32 [ %res.074, %for.body ], [ %res.3.lcssa.lcssa, %for.cond.cleanup4.loopexit ]
  %add.ptr = getelementptr inbounds float, float* %src1.073, i64 %idx.ext
  %add.ptr35 = getelementptr inbounds float, float* %dst1.071, i64 %idx.ext34
  %inc36 = add nuw nsw i32 %i.075, 1
  %exitcond80 = icmp eq i32 %inc36, %roi.sroa.2.0.extract.trunc
  br i1 %exitcond80, label %for.cond.cleanup.loopexit, label %for.body

for.body5:                                        ; preds = %for.cond.cleanup8, %for.body5.lr.ph
  %indvars.iv78 = phi i64 [ 0, %for.body5.lr.ph ], [ %indvars.iv.next79, %for.cond.cleanup8 ]
  %res.168 = phi i32 [ %res.074, %for.body5.lr.ph ], [ %res.3.lcssa, %for.cond.cleanup8 ]
  br label %for.body9

for.cond.cleanup8:                                ; preds = %for.inc
  %res.3.lcssa = phi i32 [ %res.3, %for.inc ]
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 3
  %cmp3 = icmp slt i64 %indvars.iv.next79, %0
  br i1 %cmp3, label %for.body5, label %for.cond.cleanup4.loopexit

for.body9:                                        ; preds = %for.inc, %for.body5
  %indvars.iv = phi i64 [ 0, %for.body5 ], [ %indvars.iv.next, %for.inc ]
  %res.265 = phi i32 [ %res.168, %for.body5 ], [ %res.3, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %v, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !10
  %tobool = fcmp une float %1, 0.000000e+00
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv78
  %arrayidx11 = getelementptr inbounds float, float* %src1.073, i64 %2
  %3 = load float, float* %arrayidx11, align 4, !tbaa !10
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body9
  %div14 = fdiv float %3, %1
  %arrayidx17 = getelementptr inbounds float, float* %dst1.071, i64 %2
  store float %div14, float* %arrayidx17, align 4, !tbaa !10
  br label %for.inc

if.else:                                          ; preds = %for.body9
  %cmp21 = fcmp oeq float %3, 0.000000e+00
  %4 = select i1 %cmp21, i32 -4194304, i32 2139095040
  %arrayidx2964 = getelementptr inbounds float, float* %dst1.071, i64 %2
  %arrayidx29 = bitcast float* %arrayidx2964 to i32*
  store i32 %4, i32* %arrayidx29, align 4, !tbaa !14
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %res.3 = phi i32 [ %res.265, %if.then ], [ 6, %if.else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup8, label %for.body9
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6}
!llvm.module.flags = !{!7, !8}
!llvm.ident = !{!9}

!0 = !{!"/DEFAULTLIB:libircmt.lib"}
!1 = !{!"/DEFAULTLIB:libmmt.lib"}
!2 = !{!"/DEFAULTLIB:msvcrt.lib"}
!3 = !{!"/DEFAULTLIB:oldnames.lib"}
!4 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!5 = !{!"/DEFAULTLIB:libdecimal.lib"}
!6 = !{!"/DEFAULTLIB:cilkrts.lib"}
!7 = !{i32 1, !"wchar_size", i32 2}
!8 = !{i32 7, !"PIC Level", i32 2}
!9 = !{!"clang version 6.0.0 (cfe/trunk)"}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}

