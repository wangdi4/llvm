; REQUIRES: asserts
; RUN: opt -passes "function(print<array-use>)" -o /dev/null < %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.example = type { i32, i32 }

; Check that the array use analysis is capable of recognizing non-use of
; the pointer values pointed to by the array.

; CHECK:  store [[I8_P:.*]] %call, [[I8_PP:.*]] %1, align 8
; CHECK:  -->  array at   %array = alloca [10000 x [[ST_P:.*]]], align 16 (size 10000) [0, 9999]
; CHECK:       only using empty set
; CHECK:  %2 = load [[ST_P]], [[ST_PP:.*]] %arrayidx7, align 8
; CHECK:  -->  array at   %array = alloca [10000 x [[ST_P]]], align 16 (size 10000) [0, 9999]
; CHECK:       only using [0, 999]
; CHECK:  %5 = load [[ST_P]], [[ST_PP]] %arrayidx19, align 8
; CHECK:  -->  array at   %array = alloca [10000 x [[ST_P]]], align 16 (size 10000) [0, 999]
; CHECK:       only using [0, 999]


; Function Attrs: noinline nounwind uwtable
define dso_local void @do_something() local_unnamed_addr {
entry:
  %array = alloca [10000 x %struct.example*], align 16
  %0 = bitcast [10000 x %struct.example*]* %array to i8*
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.body ]
  %call = call dereferenceable_or_null(8) i8* @malloc(i64 8)
  %arrayidx = getelementptr inbounds [10000 x %struct.example*], [10000 x %struct.example*]* %array, i64 0, i64 %indvars.iv47
  %1 = bitcast %struct.example** %arrayidx to i8**
  store i8* %call, i8** %1, align 8
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 10000
  br i1 %exitcond49, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @optbarrier()
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv43 = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next44, %for.body5 ]
  %arrayidx7 = getelementptr inbounds [10000 x %struct.example*], [10000 x %struct.example*]* %array, i64 0, i64 %indvars.iv43
  %2 = load %struct.example*, %struct.example** %arrayidx7, align 8
  %a = getelementptr inbounds %struct.example, %struct.example* %2, i64 0, i32 0
  %3 = trunc i64 %indvars.iv43 to i32
  store i32 %3, i32* %a, align 4
  %b = getelementptr inbounds %struct.example, %struct.example* %2, i64 0, i32 1
  %indvars.iv43.tr = trunc i64 %indvars.iv43 to i32
  %4 = shl i32 %indvars.iv43.tr, 1
  store i32 %4, i32* %b, align 4
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next44, 10000
  br i1 %exitcond46, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  call void @optbarrier()
  br label %for.body17

for.body17:                                       ; preds = %for.body17, %for.cond.cleanup4
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup4 ], [ %indvars.iv.next, %for.body17 ]
  %arrayidx19 = getelementptr inbounds [10000 x %struct.example*], [10000 x %struct.example*]* %array, i64 0, i64 %indvars.iv
  %5 = load %struct.example*, %struct.example** %arrayidx19, align 8
  %a20 = getelementptr inbounds %struct.example, %struct.example* %5, i64 0, i32 0
  %6 = load i32, i32* %a20, align 4
  %b23 = getelementptr inbounds %struct.example, %struct.example* %5, i64 0, i32 1
  %7 = load i32, i32* %b23, align 4
  %add = add nsw i32 %7, %6
  call void @use_value(i32 %add)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup16, label %for.body17

for.cond.cleanup16:                               ; preds = %for.body17
  ret void

}

; Function Attrs: nofree nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr

declare dso_local void @optbarrier() local_unnamed_addr

declare dso_local void @use_value(i32) local_unnamed_addr

