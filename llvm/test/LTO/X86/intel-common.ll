; RUN: llvm-as < %s > %t1.o
; RUN: llvm-lto2 run -use-new-pm=true -print-after-all -o %t2 %t1.o -save-temps -r %t1.o,MAIN__,px -r %t1.o,for_set_reentrancy,x -r %t1.o,__BLNK__,px 2>&1 | FileCheck %s

; Check that the definition of the blank common is the right size

; CHECK: @__BLNK__ = common unnamed_addr global [6643930048 x i8] zeroinitializer, align 32

; Check that the reference to the blank common has the right size

; CHECK: IR Dump After Safe Stack instrumentation pass
; CHECK: getelementptr inbounds ([6643930048 x i8], ptr @__BLNK__, i64 0, i64 6169425232)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__BLNK__ = common unnamed_addr global [6643930048 x i8] zeroinitializer, align 32
@anon.ccf94a87bdf228abecabe0e91f1f56a9.0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 8
  %"_unnamed_main$$_$J" = alloca i32, align 8
  %"_unnamed_main$$_$I" = alloca i32, align 8
  %func_result = call i32 @for_set_reentrancy(ptr @anon.ccf94a87bdf228abecabe0e91f1f56a9.0)
  store i32 1, ptr %"_unnamed_main$$_$I", align 1
  br label %bb3

bb3:                                              ; preds = %bb10, %alloca_0
  store i32 1, ptr %"_unnamed_main$$_$J", align 1
  br label %bb7

bb7:                                              ; preds = %bb7, %bb3
  %"_unnamed_main$$_$I_fetch" = load i32, ptr %"_unnamed_main$$_$I", align 1
  %"_unnamed_main$$_$J_fetch" = load i32, ptr %"_unnamed_main$$_$J", align 1
  %add = add nsw i32 %"_unnamed_main$$_$I_fetch", %"_unnamed_main$$_$J_fetch"
  %"(double)add$" = sitofp i32 %add to double
  %"_unnamed_main$$_$I_fetch5" = load i32, ptr %"_unnamed_main$$_$I", align 1
  %int_sext = sext i32 %"_unnamed_main$$_$I_fetch5" to i64
  %"_unnamed_main$$_$J_fetch6" = load i32, ptr %"_unnamed_main$$_$J", align 1
  %int_sext7 = sext i32 %"_unnamed_main$$_$J_fetch6" to i64
  %0 = sub nsw i64 %int_sext7, 1
  %1 = mul nsw i64 7702, %0
  %2 = getelementptr inbounds double, ptr bitcast (ptr getelementptr inbounds ([6643930048 x i8], ptr @__BLNK__, i32 0, i64 6169363616) to ptr), i64 %1
  %3 = sub nsw i64 %int_sext, 1
  %4 = getelementptr inbounds double, ptr %2, i64 %3
  store double %"(double)add$", ptr %4, align 1
  %"_unnamed_main$$_$J_fetch9" = load i32, ptr %"_unnamed_main$$_$J", align 1
  %add11 = add nsw i32 %"_unnamed_main$$_$J_fetch9", 1
  store i32 %add11, ptr %"_unnamed_main$$_$J", align 1
  %"_unnamed_main$$_$J_fetch13" = load i32, ptr %"_unnamed_main$$_$J", align 1
  %rel = icmp sle i32 %"_unnamed_main$$_$J_fetch13", 7702
  br i1 %rel, label %bb7, label %bb10

bb10:                                             ; preds = %bb7
  %"_unnamed_main$$_$I_fetch15" = load i32, ptr %"_unnamed_main$$_$I", align 1
  %add17 = add nsw i32 %"_unnamed_main$$_$I_fetch15", 1
  store i32 %add17, ptr %"_unnamed_main$$_$I", align 1
  %"_unnamed_main$$_$I_fetch19" = load i32, ptr %"_unnamed_main$$_$I", align 1
  %rel21 = icmp sle i32 %"_unnamed_main$$_$I_fetch19", 7702
  br i1 %rel21, label %bb3, label %bb6

bb6:                                              ; preds = %bb10
  ret void
}

declare i32 @for_set_reentrancy(ptr %0)

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 %0, i64 %1, i64 %2, ptr %3, i64 %4) #1

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}

