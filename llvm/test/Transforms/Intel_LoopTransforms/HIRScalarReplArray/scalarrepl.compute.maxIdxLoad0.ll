; RUN: opt -passes="mem2reg,loop(loop-rotate),hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; A bug reported by the newly (being) developped HIR Loop Fusion Pass.
;
; [Failure Analysis]
; For the given MemRefGroup: {A[i](W), A[i](R), A[i](R), A[i](W)}
; - The MaxDepDist is 0, and the 1st Ref is a Store, so the 1st Ref domintes all Uses in the group.
;   Thus, the MaxIndexLoad doesn't exist in this group.
;
; [Fix]
; - The fix addresses the above concern and make sure logic on MaxIndexLoad detection won't trigger.
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, %ITER.0 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>
; CHECK:        |   + DO i2 = 0, 300, 1   <DO_LOOP>
; CHECK:        |   |   (@flags1)[0][i2] = i2 + 70000;
; CHECK:        |   |   %1 = (@flags1)[0][i2];
; CHECK:        |   |   (@flags2)[0][i2] = 2 * i2 + %1;
; CHECK:        |   |   %2 = (@flags2)[0][i2];
; CHECK:        |   |   (@flags3)[0][i2] = 3 * i2 + %2;
; CHECK:        |   |   %3 = (@flags3)[0][i2];
; CHECK:        |   |   (@flags4)[0][i2] = 4 * i2 + %3;
; CHECK:        |   |   %4 = (@flags1)[0][i2];
; CHECK:        |   |   %5 = (@flags2)[0][i2];
; CHECK:        |   |   %6 = (@flags3)[0][i2];
; CHECK:        |   |   %7 = (@flags4)[0][i2];
; CHECK:        |   |   %8 = (@flags2)[0][i2];
; CHECK:        |   |   (@flags1)[0][i2] = %4 + %5 + %6 + %7 + -1 * %8;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; CHECK: Function
;
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %ITER.0 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>
; CHECK:        |   + DO i2 = 0, 300, 1   <DO_LOOP>
; CHECK:        |   |   %scalarepl = i2 + 70000;
; CHECK:        |   |   %1 = %scalarepl;
; CHECK:        |   |   %scalarepl8 = 2 * i2 + %1;
; CHECK:        |   |   (@flags2)[0][i2] = %scalarepl8;
; CHECK:        |   |   %2 = %scalarepl8;
; CHECK:        |   |   %scalarepl13 = 3 * i2 + %2;
; CHECK:        |   |   (@flags3)[0][i2] = %scalarepl13;
; CHECK:        |   |   %3 = %scalarepl13;
; CHECK:        |   |   %scalarepl17 = 4 * i2 + %3;
; CHECK:        |   |   (@flags4)[0][i2] = %scalarepl17;
; CHECK:        |   |   %4 = %scalarepl;
; CHECK:        |   |   %5 = %scalarepl8;
; CHECK:        |   |   %6 = %scalarepl13;
; CHECK:        |   |   %7 = %scalarepl17;
; CHECK:        |   |   %8 = %scalarepl8;
; CHECK:        |   |   %scalarepl = %4 + %5 + %6 + %7 + -1 * %8;
; CHECK:        |   |   (@flags1)[0][i2] = %scalarepl;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
; ModuleID = 'addarray.c'
source_filename = "addarray.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@flags1 = common global [301 x i64] zeroinitializer, align 16
@flags2 = common global [301 x i64] zeroinitializer, align 16
@flags3 = common global [301 x i64] zeroinitializer, align 16
@flags4 = common global [301 x i64] zeroinitializer, align 16
@.str = private unnamed_addr constant [13 x i8] c"%d %d %d %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main(i32 %argc, ptr %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca ptr, align 8
  %dummy = alloca [80 x i8], align 16
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %l = alloca i32, align 4
  %m = alloca i32, align 4
  %ITER = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 %argc, ptr %argc.addr, align 4
  store ptr %argv, ptr %argv.addr, align 8
  %0 = load i32, ptr %argc.addr, align 4
  %cmp = icmp eq i32 %0, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 2, ptr %ITER, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %1 = load ptr, ptr %argv.addr, align 8
  %incdec.ptr = getelementptr inbounds ptr, ptr %1, i32 1
  store ptr %incdec.ptr, ptr %argv.addr, align 8
  %2 = load ptr, ptr %incdec.ptr, align 8
  %call = call i32 (ptr, ...) @atoi(ptr %2)
  store i32 %call, ptr %ITER, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  store i32 1, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc52, %if.end
  %3 = load i32, ptr %i, align 4
  %4 = load i32, ptr %ITER, align 4
  %cmp1 = icmp sle i32 %3, %4
  br i1 %cmp1, label %for.body, label %for.end54

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %j, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %5 = load i32, ptr %j, align 4
  %cmp3 = icmp sle i32 %5, 300
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %6 = load i32, ptr %j, align 4
  %add = add nsw i32 70000, %6
  %conv = sext i32 %add to i64
  %7 = load i32, ptr %j, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds [301 x i64], ptr @flags1, i64 0, i64 %idxprom
  store i64 %conv, ptr %arrayidx, align 8
  %8 = load i32, ptr %j, align 4
  store i32 %8, ptr %k, align 4
  %9 = load i32, ptr %j, align 4
  %idxprom5 = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds [301 x i64], ptr @flags1, i64 0, i64 %idxprom5
  %10 = load i64, ptr %arrayidx6, align 8
  %11 = load i32, ptr %k, align 4
  %conv7 = sext i32 %11 to i64
  %add8 = add nsw i64 %10, %conv7
  %12 = load i32, ptr %k, align 4
  %conv9 = sext i32 %12 to i64
  %add10 = add nsw i64 %add8, %conv9
  %13 = load i32, ptr %k, align 4
  %idxprom11 = sext i32 %13 to i64
  %arrayidx12 = getelementptr inbounds [301 x i64], ptr @flags2, i64 0, i64 %idxprom11
  store i64 %add10, ptr %arrayidx12, align 8
  %14 = load i32, ptr %j, align 4
  store i32 %14, ptr %l, align 4
  %15 = load i32, ptr %k, align 4
  %idxprom13 = sext i32 %15 to i64
  %arrayidx14 = getelementptr inbounds [301 x i64], ptr @flags2, i64 0, i64 %idxprom13
  %16 = load i64, ptr %arrayidx14, align 8
  %17 = load i32, ptr %l, align 4
  %conv15 = sext i32 %17 to i64
  %add16 = add nsw i64 %16, %conv15
  %18 = load i32, ptr %l, align 4
  %conv17 = sext i32 %18 to i64
  %add18 = add nsw i64 %add16, %conv17
  %19 = load i32, ptr %l, align 4
  %conv19 = sext i32 %19 to i64
  %add20 = add nsw i64 %add18, %conv19
  %20 = load i32, ptr %l, align 4
  %idxprom21 = sext i32 %20 to i64
  %arrayidx22 = getelementptr inbounds [301 x i64], ptr @flags3, i64 0, i64 %idxprom21
  store i64 %add20, ptr %arrayidx22, align 8
  %21 = load i32, ptr %j, align 4
  store i32 %21, ptr %m, align 4
  %22 = load i32, ptr %l, align 4
  %idxprom23 = sext i32 %22 to i64
  %arrayidx24 = getelementptr inbounds [301 x i64], ptr @flags3, i64 0, i64 %idxprom23
  %23 = load i64, ptr %arrayidx24, align 8
  %24 = load i32, ptr %m, align 4
  %conv25 = sext i32 %24 to i64
  %add26 = add nsw i64 %23, %conv25
  %25 = load i32, ptr %m, align 4
  %conv27 = sext i32 %25 to i64
  %add28 = add nsw i64 %add26, %conv27
  %26 = load i32, ptr %m, align 4
  %conv29 = sext i32 %26 to i64
  %add30 = add nsw i64 %add28, %conv29
  %27 = load i32, ptr %m, align 4
  %conv31 = sext i32 %27 to i64
  %add32 = add nsw i64 %add30, %conv31
  %28 = load i32, ptr %m, align 4
  %idxprom33 = sext i32 %28 to i64
  %arrayidx34 = getelementptr inbounds [301 x i64], ptr @flags4, i64 0, i64 %idxprom33
  store i64 %add32, ptr %arrayidx34, align 8
  %29 = load i32, ptr %j, align 4
  store i32 %29, ptr %k, align 4
  %30 = load i32, ptr %j, align 4
  store i32 %30, ptr %l, align 4
  %31 = load i32, ptr %j, align 4
  store i32 %31, ptr %m, align 4
  %32 = load i32, ptr %j, align 4
  %idxprom35 = sext i32 %32 to i64
  %arrayidx36 = getelementptr inbounds [301 x i64], ptr @flags1, i64 0, i64 %idxprom35
  %33 = load i64, ptr %arrayidx36, align 8
  %34 = load i32, ptr %k, align 4
  %idxprom37 = sext i32 %34 to i64
  %arrayidx38 = getelementptr inbounds [301 x i64], ptr @flags2, i64 0, i64 %idxprom37
  %35 = load i64, ptr %arrayidx38, align 8
  %add39 = add nsw i64 %33, %35
  %36 = load i32, ptr %l, align 4
  %idxprom40 = sext i32 %36 to i64
  %arrayidx41 = getelementptr inbounds [301 x i64], ptr @flags3, i64 0, i64 %idxprom40
  %37 = load i64, ptr %arrayidx41, align 8
  %add42 = add nsw i64 %add39, %37
  %38 = load i32, ptr %m, align 4
  %idxprom43 = sext i32 %38 to i64
  %arrayidx44 = getelementptr inbounds [301 x i64], ptr @flags4, i64 0, i64 %idxprom43
  %39 = load i64, ptr %arrayidx44, align 8
  %add45 = add nsw i64 %add42, %39
  %40 = load i32, ptr %k, align 4
  %41 = load i32, ptr %j, align 4
  %sub = sub nsw i32 %40, %41
  %42 = load i32, ptr %l, align 4
  %add46 = add nsw i32 %sub, %42
  %idxprom47 = sext i32 %add46 to i64
  %arrayidx48 = getelementptr inbounds [301 x i64], ptr @flags2, i64 0, i64 %idxprom47
  %43 = load i64, ptr %arrayidx48, align 8
  %sub49 = sub nsw i64 %add45, %43
  %44 = load i32, ptr %j, align 4
  %idxprom50 = sext i32 %44 to i64
  %arrayidx51 = getelementptr inbounds [301 x i64], ptr @flags1, i64 0, i64 %idxprom50
  store i64 %sub49, ptr %arrayidx51, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %45 = load i32, ptr %j, align 4
  %inc = add nsw i32 %45, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc52

for.inc52:                                        ; preds = %for.end
  %46 = load i32, ptr %i, align 4
  %inc53 = add nsw i32 %46, 1
  store i32 %inc53, ptr %i, align 4
  br label %for.cond

for.end54:                                        ; preds = %for.cond
  %47 = load i64, ptr @flags1, align 16
  %48 = load i64, ptr getelementptr inbounds ([301 x i64], ptr @flags1, i64 0, i64 1), align 8
  %49 = load i64, ptr getelementptr inbounds ([301 x i64], ptr @flags1, i64 0, i64 299), align 8
  %50 = load i64, ptr getelementptr inbounds ([301 x i64], ptr @flags1, i64 0, i64 300), align 16
  %call55 = call i32 (ptr, ...) @printf(ptr @.str, i64 %47, i64 %48, i64 %49, i64 %50)
  ret i32 0
}

declare i32 @atoi(...) #1

declare i32 @printf(ptr, ...) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
