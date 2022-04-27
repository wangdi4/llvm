; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Deprecated the llvm.intel.directive* representation.
; TODO: Update this test to use llvm.directive.region.entry/exit instead.
; XFAIL: *


target triple = "x86_64-unknown-linux-gnu"

@b = common global [100 x [100 x i32]] zeroinitializer, align 16
@.str = private unnamed_addr constant [7 x i8] c"%d %d\0A\00", align 1
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr constant { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }

; Function Attrs: noinline nounwind uwtable
define void @foo(i32 %m1, i32 %n1, i32 %m2) #0 {
entry:
  %tid.addr = alloca i32, align 4
  %tid.val = tail call i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
  %m1.addr = alloca i32, align 4
  %n1.addr = alloca i32, align 4
  %m2.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 %m1, i32* %m1.addr, align 4
  store i32 %n1, i32* %n1.addr, align 4
  store i32 %m2, i32* %m2.addr, align 4
  store i32 %tid.val, i32* %tid.addr, align 4
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.PARALLEL.1
  store i32 0, i32* %i, align 4
  %0 = load i32, i32* %i, align 4
  %cmp15 = icmp ult i32 %0, 100
  br i1 %cmp15, label %for.body.lr.ph, label %for.end13

for.body.lr.ph:                                   ; preds = %DIR.QUAL.LIST.END.2
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc11
  call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.5

DIR.QUAL.LIST.END.5:                              ; preds = %for.body
  store i32 0, i32* %j, align 4
  %1 = load i32, i32* %j, align 4
  %cmp214 = icmp ult i32 %1, 100
  br i1 %cmp214, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %DIR.QUAL.LIST.END.5
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %2 = load i32, i32* %i, align 4
  %idxprom = zext i32 %2 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %idxprom
  %3 = load i32, i32* %j, align 4
  %sub = sub i32 %3, 1
  %idxprom4 = zext i32 %sub to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom4
  %4 = load i32, i32* %arrayidx5, align 4
  %5 = load i32, i32* %i, align 4
  %add = add i32 %4, %5
  %6 = load i32, i32* %j, align 4
  %add6 = add i32 %add, %6
  %7 = load i32, i32* %i, align 4
  %idxprom7 = zext i32 %7 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %idxprom7
  %8 = load i32, i32* %j, align 4
  %idxprom9 = zext i32 %8 to i64
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx8, i64 0, i64 %idxprom9
  store i32 %add6, i32* %arrayidx10, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %9 = load i32, i32* %j, align 4
  %inc = add i32 %9, 1
  store i32 %inc, i32* %j, align 4
  %10 = load i32, i32* %j, align 4
  %cmp2 = icmp ult i32 %10, 100
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %DIR.QUAL.LIST.END.5
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.7

DIR.QUAL.LIST.END.7:                              ; preds = %for.end
  br label %for.inc11

for.inc11:                                        ; preds = %DIR.QUAL.LIST.END.7
  %11 = load i32, i32* %i, align 4
  %inc12 = add i32 %11, 1
  store i32 %inc12, i32* %i, align 4
  %12 = load i32, i32* %i, align 4
  %cmp = icmp ult i32 %12, 100
  br i1 %cmp, label %for.body, label %for.cond.for.end13_crit_edge

for.cond.for.end13_crit_edge:                     ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.cond.for.end13_crit_edge, %DIR.QUAL.LIST.END.2
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.8

DIR.QUAL.LIST.END.8:                              ; preds = %for.end13
  %13 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 0, i64 0), align 16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %13)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

declare i32 @printf(i8*, ...) #2

declare i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }*)

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20871)"}

; CHECK:  bitcast (void (i32*, i32*, i32*, i32*)*
