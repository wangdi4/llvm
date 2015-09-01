; RUN: opt < %s -avr-generate  -avr-stress-test=true -analyze | FileCheck %s

; Check sequence AVRs generated for given test.
;CHECK: AVR_FUNCTION
;CHECK-NEXT: AVR_LABEL:    entry
;CHECK-NEXT: AVR_ASSIGN:   %a.addr = alloca i32*, align 8
;CHECK-NEXT: AVR_ASSIGN:   %b.addr = alloca i32*, align 8
;CHECK-NEXT: AVR_ASSIGN:   %c.addr = alloca i32*, align 8
;CHECK-NEXT: AVR_ASSIGN:   %d.addr = alloca i32*, align 8
;CHECK-NEXT: AVR_ASSIGN:   %N.addr = alloca i32, align 4
;CHECK-NEXT: AVR_ASSIGN:   %i = alloca i32, align 4
;CHECK-NEXT: AVR_ASSIGN:   store i32* %a, i32** %a.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   store i32* %b, i32** %b.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   store i32* %c, i32** %c.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   store i32* %d, i32** %d.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   store i32 %N, i32* %N.addr, align 4
;CHECK-NEXT: AVR_ASSIGN:   store i32 0, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    for.cond
;CHECK-NEXT: AVR_ASSIGN:   %0 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %1 = load i32, i32* %N.addr, align 4
;CHECK-NEXT: AVR_IF:       %cmp = icmp slt i32 %0, %1
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp, label %for.body, label %for.end
;CHECK-NEXT: AVR_LABEL:    for.body
;CHECK-NEXT: AVR_ASSIGN:   %2 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom = sext i32 %2 to i64
;CHECK-NEXT: AVR_ASSIGN:   %3 = load i32*, i32** %a.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx = getelementptr inbounds i32, i32* %3, i64 %idxprom
;CHECK-NEXT: AVR_ASSIGN:   %4 = load i32, i32* %arrayidx, align 4
;CHECK-NEXT: AVR_ASSIGN:   %rem = srem i32 %4, 2
;CHECK-NEXT: AVR_IF:       %cmp1 = icmp eq i32 %rem, 0
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp1, label %land.lhs.true, label %if.else
;CHECK-NEXT: AVR_LABEL:    land.lhs.true
;CHECK-NEXT: AVR_ASSIGN:   %5 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom2 = sext i32 %5 to i64
;CHECK-NEXT: AVR_ASSIGN:   %6 = load i32*, i32** %b.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx3 = getelementptr inbounds i32, i32* %6, i64 %idxprom2
;CHECK-NEXT: AVR_ASSIGN:   %7 = load i32, i32* %arrayidx3, align 4
;CHECK-NEXT: AVR_ASSIGN:   %rem4 = srem i32 %7, 2
;CHECK-NEXT: AVR_IF:       %cmp5 = icmp eq i32 %rem4, 0
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp5, label %if.then, label %if.else
;CHECK-NEXT: AVR_LABEL:    if.then
;CHECK-NEXT: AVR_ASSIGN:   %8 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom6 = sext i32 %8 to i64
;CHECK-NEXT: AVR_ASSIGN:   %9 = load i32*, i32** %a.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx7 = getelementptr inbounds i32, i32* %9, i64 %idxprom6
;CHECK-NEXT: AVR_ASSIGN:   %10 = load i32, i32* %arrayidx7, align 4
;CHECK-NEXT: AVR_ASSIGN:   %11 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom8 = sext i32 %11 to i64
;CHECK-NEXT: AVR_ASSIGN:   %12 = load i32*, i32** %b.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx9 = getelementptr inbounds i32, i32* %12, i64 %idxprom8
;CHECK-NEXT: AVR_ASSIGN:   %13 = load i32, i32* %arrayidx9, align 4
;CHECK-NEXT: AVR_ASSIGN:   %14 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom10 = sext i32 %14 to i64
;CHECK-NEXT: AVR_ASSIGN:   %15 = load i32*, i32** %c.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx11 = getelementptr inbounds i32, i32* %15, i64 %idxprom10
;CHECK-NEXT: AVR_ASSIGN:   %16 = load i32, i32* %arrayidx11, align 4
;CHECK-NEXT: AVR_ASSIGN:   %mul = mul nsw i32 %13, %16
;CHECK-NEXT: AVR_ASSIGN:   %add = add nsw i32 %10, %mul
;CHECK-NEXT: AVR_ASSIGN:   %17 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom12 = sext i32 %17 to i64
;CHECK-NEXT: AVR_ASSIGN:   %18 = load i32*, i32** %d.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx13 = getelementptr inbounds i32, i32* %18, i64 %idxprom12
;CHECK-NEXT: AVR_ASSIGN:   store i32 %add, i32* %arrayidx13, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %if.end
;CHECK-NEXT: AVR_LABEL:    if.else
;CHECK-NEXT: AVR_ASSIGN:   %19 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom14 = sext i32 %19 to i64
;CHECK-NEXT: AVR_ASSIGN:   %20 = load i32*, i32** %a.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx15 = getelementptr inbounds i32, i32* %20, i64 %idxprom14
;CHECK-NEXT: AVR_ASSIGN:   %21 = load i32, i32* %arrayidx15, align 4
;CHECK-NEXT: AVR_ASSIGN:   %22 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom16 = sext i32 %22 to i64
;CHECK-NEXT: AVR_ASSIGN:   %23 = load i32*, i32** %c.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx17 = getelementptr inbounds i32, i32* %23, i64 %idxprom16
;CHECK-NEXT: AVR_ASSIGN:   %24 = load i32, i32* %arrayidx17, align 4
;CHECK-NEXT: AVR_ASSIGN:   %sub = sub nsw i32 %21, %24
;CHECK-NEXT: AVR_ASSIGN:   %25 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom18 = sext i32 %25 to i64
;CHECK-NEXT: AVR_ASSIGN:   %26 = load i32*, i32** %d.addr, align 8
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx19 = getelementptr inbounds i32, i32* %26, i64 %idxprom18
;CHECK-NEXT: AVR_ASSIGN:   store i32 %sub, i32* %arrayidx19, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %if.end
;CHECK-NEXT: AVR_LABEL:    if.end
;CHECK-NEXT: AVR_FBRANCH:  br label %for.inc
;CHECK-NEXT: AVR_LABEL:    for.inc
;CHECK-NEXT: AVR_ASSIGN:   %27 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %inc = add nsw i32 %27, 1
;CHECK-NEXT: AVR_ASSIGN:   store i32 %inc, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LABEL:    for.end
;CHECK-NEXT: AVR_RETURN:   ret void

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %a, i32* %b, i32* %c, i32* %d, i32 %N) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %b.addr = alloca i32*, align 8
  %c.addr = alloca i32*, align 8
  %d.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %a, i32** %a.addr, align 8
  store i32* %b, i32** %b.addr, align 8
  store i32* %c, i32** %c.addr, align 8
  store i32* %d, i32** %d.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %N.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %3 = load i32*, i32** %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %3, i64 %idxprom
  %4 = load i32, i32* %arrayidx, align 4
  %rem = srem i32 %4, 2
  %cmp1 = icmp eq i32 %rem, 0
  br i1 %cmp1, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %5 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %5 to i64
  %6 = load i32*, i32** %b.addr, align 8
  %arrayidx3 = getelementptr inbounds i32, i32* %6, i64 %idxprom2
  %7 = load i32, i32* %arrayidx3, align 4
  %rem4 = srem i32 %7, 2
  %cmp5 = icmp eq i32 %rem4, 0
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %8 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %8 to i64
  %9 = load i32*, i32** %a.addr, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %9, i64 %idxprom6
  %10 = load i32, i32* %arrayidx7, align 4
  %11 = load i32, i32* %i, align 4
  %idxprom8 = sext i32 %11 to i64
  %12 = load i32*, i32** %b.addr, align 8
  %arrayidx9 = getelementptr inbounds i32, i32* %12, i64 %idxprom8
  %13 = load i32, i32* %arrayidx9, align 4
  %14 = load i32, i32* %i, align 4
  %idxprom10 = sext i32 %14 to i64
  %15 = load i32*, i32** %c.addr, align 8
  %arrayidx11 = getelementptr inbounds i32, i32* %15, i64 %idxprom10
  %16 = load i32, i32* %arrayidx11, align 4
  %mul = mul nsw i32 %13, %16
  %add = add nsw i32 %10, %mul
  %17 = load i32, i32* %i, align 4
  %idxprom12 = sext i32 %17 to i64
  %18 = load i32*, i32** %d.addr, align 8
  %arrayidx13 = getelementptr inbounds i32, i32* %18, i64 %idxprom12
  store i32 %add, i32* %arrayidx13, align 4
  br label %if.end

if.else:                                          ; preds = %land.lhs.true, %for.body
  %19 = load i32, i32* %i, align 4
  %idxprom14 = sext i32 %19 to i64
  %20 = load i32*, i32** %a.addr, align 8
  %arrayidx15 = getelementptr inbounds i32, i32* %20, i64 %idxprom14
  %21 = load i32, i32* %arrayidx15, align 4
  %22 = load i32, i32* %i, align 4
  %idxprom16 = sext i32 %22 to i64
  %23 = load i32*, i32** %c.addr, align 8
  %arrayidx17 = getelementptr inbounds i32, i32* %23, i64 %idxprom16
  %24 = load i32, i32* %arrayidx17, align 4
  %sub = sub nsw i32 %21, %24
  %25 = load i32, i32* %i, align 4
  %idxprom18 = sext i32 %25 to i64
  %26 = load i32*, i32** %d.addr, align 8
  %arrayidx19 = getelementptr inbounds i32, i32* %26, i64 %idxprom18
  store i32 %sub, i32* %arrayidx19, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %27 = load i32, i32* %i, align 4
  %inc = add nsw i32 %27, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (branches/vpo 1169)"}
