; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-opt-var-predicate,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; The test runs HIR Var OptPredicate pass on conditions at lines 33 and 37.
; We check that compiler doen't crush if there is a detached loop
; in the analysis invalidation list.

; CHECK: BEGIN REGION

;*** IR Dump After HIR OptPredicate (hir-opt-predicate) ***
;Function: main
;
;<0>          BEGIN REGION { modified }
;<99>               + DO i1 = 0, 3, 1   <DO_LOOP>
;<4>                |   %d.060 = i1;
;<100>              |
;<100>              |   + DO i2 = 0, -1 * i1 + 3, 1   <DO_LOOP>  <MAX_TC_EST = 4>
;<7>                |   |   @llvm.lifetime.start.p0(1,  &((%f)[0]));
;<8>                |   |   @llvm.lifetime.start.p0(4,  &((i8*)(%i)[0]));
;<10>               |   |   if (i1 + i2 != 0)
;<10>               |   |   {
;<108>              |   |      %2 = (@b)[0];
;<109>              |   |      %3 = (%f)[0];
;<110>              |   |      %tobool9.not = %3 == 0;
;<111>              |   |      %or.cond53 = (%2 == 0) ? -1 : %tobool9.not;
;<107>              |   |      if (%or.cond53 == 0)
;<107>              |   |      {
;<101>              |   |         + DO i3 = 0, 63, 1   <DO_LOOP>
;<17>               |   |         |   %2 = (@b)[0];
;<19>               |   |         |   %3 = (%f)[0];
;<20>               |   |         |   %tobool9.not = %3 == 0;
;<21>               |   |         |   %or.cond53 = (%2 == 0) ? -1 : %tobool9.not;
;<22>               |   |         |   if (%or.cond53 == 0) <no_unswitch>
;<22>               |   |         |   {
;<26>               |   |         |      %div54 = 8  /  %3;
;<28>               |   |         |      if (%div54 == 4)
;<28>               |   |         |      {
;<33>               |   |         |         if (i3 != 0)
;<33>               |   |         |         {
;<37>               |   |         |            if (i1 == 0)
;<37>               |   |         |            {
;<41>               |   |         |               %call21 = @_Z4copyj(undef);
;<42>               |   |         |               %xor = %call21  ^  9;
;<43>               |   |         |               %8 = (@c)[0][0];
;<45>               |   |         |               (@c)[0][0] = (%xor * %8);
;<37>               |   |         |            }
;<37>               |   |         |            else
;<37>               |   |         |            {
;<48>               |   |         |               %rem55 = %3  %  2;
;<53>               |   |         |               %tobool17 = (@a)[0] != 0;
;<54>               |   |         |               %or.cond35 = ((sext.i8.i32(%rem55) * %d.060) != 0) ? %tobool17 : 0;
;<55>               |   |         |               %5 = %2;
;<56>               |   |         |               if (%or.cond35 != 0)
;<56>               |   |         |               {
;<60>               |   |         |                  %call = @_Z6printbj(i1 + i2);
;<61>               |   |         |                  %5 = (@b)[0];
;<56>               |   |         |               }
;<67>               |   |         |               %7 = (@c)[0][-1 * i1 + i3];
;<69>               |   |         |               (@b)[0] = (%7 * %5);
;<37>               |   |         |            }
;<33>               |   |         |         }
;<72>               |   |         |         %call25 = @_Z4swapRcRj(&((%f)[0]),  &((%i)[0]));
;<28>               |   |         |      }
;<22>               |   |         |   }
;<101>              |   |         + END LOOP
;<107>              |   |      }
;<10>               |   |   }
;<84>               |   |   @llvm.lifetime.end.p0(4,  &((i8*)(%i)[0]));
;<85>               |   |   @llvm.lifetime.end.p0(1,  &((%f)[0]));
;<88>               |   |   %d.060 = i1 + i2 + 1;
;<100>              |   + END LOOP
;<99>               + END LOOP
;<0>          END REGION



; Module Before HIR
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global [1 x i32] zeroinitializer, align 4

; Function Attrs: norecurse uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr {
entry:
  %f = alloca i8, align 1
  %i = alloca i32, align 4
  %0 = bitcast ptr %i to ptr
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc31, %entry
  %indvars.iv67 = phi i64 [ 0, %entry ], [ %indvars.iv.next68, %for.inc31 ]
  %tobool12.not = icmp eq i64 %indvars.iv67, 0
  %1 = trunc i64 %indvars.iv67 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.end
  %d.060 = phi i32 [ %inc29, %for.end ], [ %1, %for.body3.preheader ]
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %f)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %0)
  %tobool = icmp eq i32 %d.060, 0
  br i1 %tobool, label %for.end, label %for.body6.preheader

for.body6.preheader:                              ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body6.preheader ]
  %2 = load i32, ptr @b, align 4
  %tobool7 = icmp eq i32 %2, 0
  %3 = load i8, ptr %f, align 1
  %tobool9.not = icmp eq i8 %3, 0
  %or.cond53 = select i1 %tobool7, i1 true, i1 %tobool9.not
  br i1 %or.cond53, label %for.inc, label %cond.end

cond.end:                                         ; preds = %for.body6
  %div54 = sdiv i8 8, %3
  %cond34 = icmp eq i8 %div54, 4
  br i1 %cond34, label %sw.bb, label %for.inc

sw.bb:                                            ; preds = %cond.end
  %tobool10.not = icmp eq i64 %indvars.iv, 0
  br i1 %tobool10.not, label %if.end24, label %if.then11

if.then11:                                        ; preds = %sw.bb
  br i1 %tobool12.not, label %if.else, label %if.then13

if.then13:                                        ; preds = %if.then11
  %rem55 = srem i8 %3, 2
  %rem.sext = sext i8 %rem55 to i32
  %mul = mul nsw i32 %d.060, %rem.sext
  %tobool15 = icmp ne i32 %mul, 0
  %4 = load i32, ptr @a, align 4
  %tobool17 = icmp ne i32 %4, 0
  %or.cond35 = select i1 %tobool15, i1 %tobool17, i1 false
  br i1 %or.cond35, label %if.then18, label %if.end19

if.then18:                                        ; preds = %if.then13
  %call = call i32 @_Z6printbj(i32 %d.060)
  %.pre = load i32, ptr @b, align 4, !tbaa !2
  br label %if.end19

if.end19:                                         ; preds = %if.then18, %if.then13
  %5 = phi i32 [ %.pre, %if.then18 ], [ %2, %if.then13 ]
  %6 = sub nsw i64 %indvars.iv, %indvars.iv67
  %arrayidx = getelementptr inbounds [1 x i32], ptr @c, i64 0, i64 %6, !intel-tbaa !6
  %7 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %mul20 = mul nsw i32 %5, %7
  store i32 %mul20, ptr @b, align 4, !tbaa !2
  br label %if.end24

if.else:                                          ; preds = %if.then11
  %call21 = call i32 @_Z4copyj(i32 undef)
  %xor = xor i32 %call21, 9
  %8 = load i32, ptr getelementptr inbounds ([1 x i32], ptr @c, i64 0, i64 0), align 4, !tbaa !6
  %mul22 = mul nsw i32 %8, %xor
  store i32 %mul22, ptr getelementptr inbounds ([1 x i32], ptr @c, i64 0, i64 0), align 4, !tbaa !6
  br label %if.end24

if.end24:                                         ; preds = %if.end19, %if.else, %sw.bb
  %call25 = call i32 @_Z4swapRcRj(ptr nonnull align 1 dereferenceable(1) %f, ptr nonnull align 4 dereferenceable(4) %i)
  br label %for.inc

for.inc:                                          ; preds = %for.body6, %if.end24, %cond.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body6, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %0)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %f)
  %inc29 = add nuw nsw i32 %d.060, 1
  %exitcond66.not = icmp eq i32 %inc29, 4
  br i1 %exitcond66.not, label %for.inc31, label %for.body3, !llvm.loop !10

for.inc31:                                        ; preds = %for.end
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond69.not = icmp eq i64 %indvars.iv.next68, 4
  br i1 %exitcond69.not, label %for.end33, label %for.body3.preheader, !llvm.loop !11

for.end33:                                        ; preds = %for.inc31
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare dso_local i32 @_Z6printbj(i32) local_unnamed_addr

declare dso_local i32 @_Z4copyj(i32) local_unnamed_addr

declare dso_local i32 @_Z4swapRcRj(ptr nonnull align 1 dereferenceable(1), ptr nonnull align 4 dereferenceable(4)) local_unnamed_addr

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1_i", !3, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = distinct !{!11, !9}
