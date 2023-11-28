; Verify that the substitution of use ref %t1 is in the right order. After single rval definition
; of temp %j.0205 = %j.0205  +  1 was moved after use %t1 = @s2[0][...],  %t1 = (@s2)[0][...] will
; not be considered as a temp substitution candidate node.
;
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

;*** IR Dump Before HIR Temp Cleanup (hir-temp-cleanup) ***
;Function: main
;
;<0>          BEGIN REGION { }
;<31>               + DO i1 = 0, 511, 1   <DO_LOOP>
;<2>                |   %j.0205.out = %j.0205;
;<4>                |   %cond46 = 0;
;<5>                |   if (i1 != zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s1))))) + 304)
;<5>                |   {
;<10>               |      %cond46 = 98;
;<11>               |      if (i1 >=u zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s1))))) + 176)
;<11>               |      {
;<15>               |         %j.0205 = %j.0205  +  1;
;<19>               |         %t1 = (@s2)[0][zext.i6.i64(trunc.i64.i6(%j.0205.out)) + zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s2))))) + 112];
;<20>               |         %cond46 = %t1;
;<11>               |      }
;<5>                |   }
;<24>               |   (@s1)[0][i1] = %cond46;
;<31>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Temp Cleanup (hir-temp-cleanup) ***
;Function: main
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 511, 1   <DO_LOOP>
; CHECK:           |   %cond46 = 0;
; CHECK:           |   if (i1 != zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s1))))) + 304)
; CHECK:           |   {
; CHECK:           |      %cond46 = 98;
; CHECK:           |      if (i1 >=u zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s1))))) + 176)
; CHECK:           |      {
; CHECK:           |         %t1 = (@s2)[0][zext.i6.i64((16 + (-1 * trunc.i64.i6(ptrtoint.ptr.i64(@s2))))) + zext.i6.i64(trunc.i64.i6(%j.0205)) + 112];
; CHECK:           |         %j.0205 = %j.0205  +  1;
; CHECK:           |         %cond46 = %t1;
; CHECK:           |      }
; CHECK:           |   }
; CHECK:           |   (@s1)[0][i1] = %cond46;
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'c9x_7_21_5_7_a_1.c'
source_filename = "c9x_7_21_5_7_a_1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@s1 = dso_local global [512 x i8] zeroinitializer, align 16
@s2 = dso_local global [512 x i8] zeroinitializer, align 16
@.str = private unnamed_addr constant [19 x i8] c"\09 Error in Block A\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"\09strstr return %p\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"\09s1 is - \0A %s \0A\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"\09s2 is - \0A %s \0A\00", align 1
@.str.4 = private unnamed_addr constant [30 x i8] c"\09Expected return value is %p\0A\00", align 1
@.str.5 = private unnamed_addr constant [19 x i8] c"\09 Error in Block B\00", align 1
@.str.7 = private unnamed_addr constant [12 x i8] c" test - %s\0A\00", align 1
@.str.8 = private unnamed_addr constant [5 x i8] c"FAIL\00", align 1
@.str.9 = private unnamed_addr constant [7 x i8] c"PASSED\00", align 1
@str = private unnamed_addr constant [31 x i8] c"\09Expected return value is NULL\00", align 1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                       ; preds = %for.body.preheader, %cond.end45
  %i.1206 = phi i64 [ %inc50, %cond.end45 ], [ 0, %entry ]
  %j.0205 = phi i64 [ %j.1, %cond.end45 ], [ 0, %entry ]
  %zext = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add = add i64 %zext, 304
  %cmp30 = icmp eq i64 %i.1206, %add
  br i1 %cmp30, label %cond.end45, label %cond.false33

cond.false33:                                     ; preds = %for.body
  %zext2 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add2 = add i64 %zext2, 176
  %cmp34.not = icmp ult i64 %i.1206, %add2
  br i1 %cmp34.not, label %cond.end45, label %cond.true36

cond.true36:                                      ; preds = %cond.false33
  %inc37 = add i64 %j.0205, 1
  %rem38 = and i64 %j.0205, 63
  %zext3 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s2 to i6)) to i64
  %add3 = add i64 %zext3, 112
  %add39 = add nuw nsw i64 %rem38, %add3
  %arrayidx40 = getelementptr inbounds [512 x i8], ptr @s2, i64 0, i64 %add39, !intel-tbaa !3
  %t1 = load i8, ptr %arrayidx40, align 1, !tbaa !3
  br label %cond.end45

cond.end45:                                       ; preds = %cond.true36, %cond.false33, %for.body
  %j.1 = phi i64 [ %j.0205, %for.body ], [ %inc37, %cond.true36 ], [ %j.0205, %cond.false33 ]
  %cond46 = phi i8 [ 0, %for.body ], [ %t1, %cond.true36 ], [ 98, %cond.false33 ]
  %arrayidx48 = getelementptr inbounds [512 x i8], ptr @s1, i64 0, i64 %i.1206, !intel-tbaa !3
  store i8 %cond46, ptr %arrayidx48, align 1, !tbaa !3
  %inc50 = add nuw nsw i64 %i.1206, 1
  %exitcond210.not = icmp eq i64 %inc50, 512
  br i1 %exitcond210.not, label %for.end51, label %for.body, !llvm.loop !9

for.end51:                                        ; preds = %cond.end45
  %zext4 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add4 = add i64 %zext4, 112
  %gep1 = getelementptr [512 x i8], ptr @s1, i64 0, i64 %add4
  %zext5 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s2 to i6)) to i64
  %add5 = add i64 %zext5, 112
  %gep2 = getelementptr [512 x i8], ptr @s2, i64 0, i64 %add5
  %call = tail call ptr @strstr(ptr noundef nonnull %gep1, ptr noundef nonnull %gep2) #3
  %zext6 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add6 = add i64 %zext6, 176
  %gep3 = getelementptr inbounds [512 x i8], ptr @s1, i64 0, i64 %add6
  %cmp55.not = icmp eq ptr %call, %gep3
  br i1 %cmp55.not, label %if.end66, label %if.then57

if.then57:                                        ; preds = %for.end51
  %call58 = tail call i32 @puts(ptr nonnull dereferenceable(1) @.str)
  %call59 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.1, ptr %call)
  %zext7 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add7 = add i64 %zext7, 112
  %gep4 = getelementptr [512 x i8], ptr @s1, i64 0, i64 %add7
  %call61 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.2, ptr nonnull %gep3)
  %zext8 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s2 to i6)) to i64
  %add8 = add i64 %zext8, 112
  %gep5 = getelementptr [512 x i8], ptr @s2, i64 0, i64 %add8
  %call63 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.3, ptr nonnull %gep4)
  %zext9 = zext i6 sub (i6 16, i6 ptrtoint (ptr @s1 to i6)) to i64
  %add9 = add i64 %zext9, 176
  %gep6 = getelementptr inbounds [512 x i8], ptr @s1, i64 0, i64 %add9
  %call65 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str.4, ptr nonnull %gep5)
  br label %if.end66

if.end66:                                         ; preds = %if.then57, %for.end51
  %TestPassFlag.0 = phi i32 [ -1, %if.then57 ], [ 0, %for.end51 ]
  br label %for.body70

for.body70:
  ret i32 0
}
declare dso_local ptr @strstr(ptr, ptr nocapture) local_unnamed_addr

declare dso_local noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr

declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA512_c", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
