
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -hir-cg -hir-general-unroll-disable-replace-by-first-iteration -S 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,hir-cg" -aa-pipeline="basic-aa" -hir-general-unroll-disable-replace-by-first-iteration -S  2>&1 < %s| FileCheck %s

; Before General Unroll
;
;        BEGIN REGION { }
;              + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20> <unroll = 2>
;              |   %conv = sitofp.i32.double(i1 + 2);
;              |   (@a)[0][i1] = %conv;
;              |   %mul = i1  *  i1;
;              |   switch(%mul + -3 * (%mul /u 3))
;              |   {
;              |   case 0:
;              |      %call = @printf(&((@.str)[0][0]),  i1);
;              |      break;
;              |   case 1:
;              |      %call2 = @printf(&((@.str.1)[0][0]),  i1);
;              |      break;
;              |   default:
;              |      %call3 = @printf(&((@.str.2)[0][0]),  i1);
;              |      break;
;              |   }
;              + END LOOP
;        END REGION

; After General Unroll
; *** IR Dump After HIR General Unroll ***
; Function: sub
;
;          BEGIN REGION { modified }
;                %tgu = (sext.i32.i64(%N))/u2;
;
;                + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10> <nounroll>
;                |   %conv = sitofp.i32.double(2 * i1 + 2);
;                |   (@a)[0][2 * i1] = %conv;
;                |   %mul = 2 * i1  *  2 * i1;
;                |   switch(%mul + -3 * (%mul /u 3))
;                |   {
;                |   case 0:
;                |      %call = @printf(&((@.str)[0][0]),  2 * i1);
;                |      break;
;                |   case 1:
;                |      %call2 = @printf(&((@.str.1)[0][0]),  2 * i1);
;                |      break;
;                |   default:
;                |      %call3 = @printf(&((@.str.2)[0][0]),  2 * i1);
;                |      break;
;                |   }
;                |   %conv = sitofp.i32.double(2 * i1 + 3);
;                |   (@a)[0][2 * i1 + 1] = %conv;
;                |   %mul = 2 * i1 + 1  *  2 * i1 + 1;
;                |   switch(%mul + -3 * (%mul /u 3))
;                |   {
;                |   case 0:
;                |      %call = @printf(&((@.str)[0][0]),  2 * i1 + 1);
;                |      break;
;                |   case 1:
;                |      %call2 = @printf(&((@.str.1)[0][0]),  2 * i1 + 1);
;                |      break;
;                |   default:
;                |      %call3 = @printf(&((@.str.2)[0][0]),  2 * i1 + 1);
;                |      break;
;                |   }
;                + END LOOP
;
;
;                + DO i1 = 2 * %tgu, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1> <nounroll> <max_trip_count = 1>
;                |   %conv = sitofp.i32.double(i1 + 2);
;                |   (@a)[0][i1] = %conv;
;                |   %mul = i1  *  i1;
;                |   switch(%mul + -3 * (%mul /u 3))
;                |   {
;                |   case 0:
;                |      %call = @printf(&((@.str)[0][0]),  i1);
;                |      break;
;                |   case 1:
;                |      %call2 = @printf(&((@.str.1)[0][0]),  i1);
;                |      break;
;                |   default:
;                |      %call3 = @printf(&((@.str.2)[0][0]),  i1);
;                |      break;
;                |   }
;                + END LOOP
;          END REGION

; Unrolled loop
;CHECK: region.0:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_LOOP_UNROLLED:[0-9]+]]
;CHECK: switch i32 %{{[0-9]+}}, label %[[SWITCH_UNROLLED_1:hir.sw.[0-9]+]].default [
;CHECK-NEXT: i32 0, label %[[SWITCH_UNROLLED_1]].case.0
;CHECK-NEXT: i32 1, label %[[SWITCH_UNROLLED_1]].case.1
;CHECK-NEXT: ], !prof ![[PROF_SWITCH_UNROLLED:[0-9]+]]
;CHECK: switch i32 %{{[0-9]+}}, label %[[SWITCH_UNROLLED_2:hir.sw.[0-9]+]].default [
;CHECK-NEXT: i32 0, label %[[SWITCH_UNROLLED_2]].case.0
;CHECK-NEXT: i32 1, label %[[SWITCH_UNROLLED_2]].case.1
;CHECK-NEXT: ], !prof ![[PROF_SWITCH_UNROLLED]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_LOOP_UNROLLED]]

; Remainder loop
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_LOOP_REMAINDER:[0-9]+]]
;CHECK: switch i32 %{{[0-9]+}}, label %[[SWITCH_REMAINDER_LOOP:hir.sw.[0-9]+]].default [
;CHECK-NEXT: i32 0, label %[[SWITCH_REMAINDER_LOOP]].case.0
;CHECK-NEXT: i32 1, label %[[SWITCH_REMAINDER_LOOP]].case.1
;CHECK-NEXT: ], !prof ![[PROF_SWITCH_REMAINDER:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_LOOP_REMAINDER]]

;CHECK-DAG: ![[PROF_LOOP_UNROLLED]] = !{!"branch_weights", i32 5, i32 1}
;CHECK-DAG: ![[PROF_SWITCH_UNROLLED]] = !{!"branch_weights", i32 0, i32 2, i32 3}
;CHECK-DAG: ![[PROF_LOOP_REMAINDER]] = !{!"branch_weights", i32 1, i32 1}
;CHECK-DAG: ![[PROF_SWITCH_REMAINDER]] = !{!"branch_weights", i32 0, i32 1, i32 1}


; ModuleID = 'switch-pragma-unroll.c'
source_filename = "switch-pragma-unroll.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [20 x double] zeroinitializer, align 16
@.str = private unnamed_addr constant [14 x i8] c"%d is case 1\0A\00", align 1
@.str.1 = private unnamed_addr constant [14 x i8] c"%d is case 2\0A\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"%d is default.\0A\00", align 1
@b = common dso_local local_unnamed_addr global [20 x double] zeroinitializer, align 16

; Function Attrs: inlinehint nounwind uwtable
define dso_local i32 @sub(i32 %N) local_unnamed_addr #0 !prof !29 {
entry:
  %cmp16 = icmp sgt i32 %N, 0
  br i1 %cmp16, label %for.body.preheader, label %for.end, !prof !30

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %1 = add i32 %0, 2
  %conv = sitofp i32 %1 to double
  %arrayidx = getelementptr inbounds [20 x double], [20 x double]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !31
  store double %conv, double* %arrayidx, align 8, !tbaa !31
  %2 = trunc i64 %indvars.iv to i32
  %mul = mul nsw i32 %2, %2
  %rem = urem i32 %mul, 3
  switch i32 %rem, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
  ], !prof !36

sw.bb:                                            ; preds = %for.body
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 %2), !intel-profx !37
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %call2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i64 0, i64 0), i32 %2), !intel-profx !38
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %call3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.2, i64 0, i64 0), i32 %2), !intel-profx !39
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !prof !40, !llvm.loop !41

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %3 = load double, double* getelementptr inbounds ([20 x double], [20 x double]* @a, i64 0, i64 3), align 8, !tbaa !31
  %conv4 = fptosi double %3 to i32
  ret i32 %conv4
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: inlinehint nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %call = tail call i32 @sub(i32 11) #2, !intel-profx !43
  ret i32 0
}

attributes #0 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 13}
!5 = !{!"MaxCount", i64 7}
!6 = !{!"MaxInternalCount", i64 7}
!7 = !{!"MaxFunctionCount", i64 4}
!8 = !{!"NumCounts", i64 5}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 0, i32 0}
!13 = !{i32 100000, i64 7, i32 1}
!14 = !{i32 200000, i64 7, i32 1}
!15 = !{i32 300000, i64 7, i32 1}
!16 = !{i32 400000, i64 7, i32 1}
!17 = !{i32 500000, i64 7, i32 1}
!18 = !{i32 600000, i64 7, i32 1}
!19 = !{i32 700000, i64 4, i32 2}
!20 = !{i32 800000, i64 4, i32 2}
!21 = !{i32 900000, i64 4, i32 2}
!22 = !{i32 950000, i64 1, i32 4}
!23 = !{i32 990000, i64 1, i32 4}
!24 = !{i32 999000, i64 1, i32 4}
!25 = !{i32 999900, i64 1, i32 4}
!26 = !{i32 999990, i64 1, i32 4}
!27 = !{i32 999999, i64 1, i32 4}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"branch_weights", i32 11, i32 1}
!31 = !{!32, !33, i64 0}
!32 = !{!"array@_ZTSA20_d", !33, i64 0}
!33 = !{!"double", !34, i64 0}
!34 = !{!"omnipotent char", !35, i64 0}
!35 = !{!"Simple C/C++ TBAA"}
!36 = !{!"branch_weights", i32 0, i32 4, i32 7}
!37 = !{!"intel_profx", i64 4}
!38 = !{!"intel_profx", i64 7}
!39 = !{!"intel_profx", i64 0}
!40 = !{!"branch_weights", i32 1, i32 11}
!41 = distinct !{!41, !42}
!42 = !{!"llvm.loop.unroll.count", i32 2}
!43 = !{!"intel_profx", i64 1}
