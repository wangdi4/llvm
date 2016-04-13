; RUN: opt < %s -avr-generate -avr-stress-test=true -analyze | FileCheck %s

; Check that the Abstract Layer generated incoming LLVM basic blocks in lexical order.

;CHECK: Printing analysis 'AVR Generate' for function 'foo':
;CHECK: foo(i32* %a, i32* %b, i32* %c, i32* %d, i32 %N)

;CHECK: entry:
;CHECK-NEXT: %a.addr = alloca 1
;CHECK:      %cmp1 = icmp slt i32 %0, 32
;CHECK-NEXT: br i1 %cmp1, label %for.body.lr.ph, label %for.end

;CHECK-NEXT: if( %cmp1 = icmp slt i32 %0, 32 )
;CHECK:      for.body.lr.ph:
;CHECK-NEXT: br label %for.body

;CHECK: LOOP

;CHECK:      for.body:
;CHECK-NEXT: %1 = load %i
;CHECK:      %cmp7 = icmp eq i32 %rem, 0
;CHECK-NEXT: br i1 %cmp7, label %if.then, label %if.else

;CHECK-NEXT: if( %cmp7 = icmp eq i32 %rem, 0 )
;CHECK:      if.then:
;CHECK-NEXT: %12 = load %i
;CHECK:      br label %if.end

;CHECK:      else
;CHECK:      if.else:
;CHECK-NEXT: %20 = load %i
;CHECK:      br label %if.end

;CHECK:      if.end:
;CHECK-NEXT: br label %for.inc
;CHECK-NEXT: for.inc:
;CHECK-NEXT: %28 = load %i
;CHECK:      %cmp = icmp slt i32 %29, 32
;CHECK-NEXT: br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

;CHECK:      for.cond.for.end_crit_edge:
;CHECK-NEXT: br label %for.end

;CHECK:      for.end:
;CHECK-NEXT: ret void

; ModuleID = 'testfunction.bc'
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
  %0 = load i32, i32* %i, align 4
  %cmp1 = icmp slt i32 %0, 32
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %2 = load i32*, i32** %b.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %2, i64 %idxprom
  %3 = load i32, i32* %arrayidx, align 4
  %4 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %4 to i64
  %5 = load i32*, i32** %c.addr, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %5, i64 %idxprom1
  %6 = load i32, i32* %arrayidx2, align 4
  %mul = mul nsw i32 %3, %6
  %7 = load i32, i32* %i, align 4
  %idxprom3 = sext i32 %7 to i64
  %8 = load i32*, i32** %a.addr, align 8
  %arrayidx4 = getelementptr inbounds i32, i32* %8, i64 %idxprom3
  store i32 %mul, i32* %arrayidx4, align 4
  %9 = load i32, i32* %i, align 4
  %idxprom5 = sext i32 %9 to i64
  %10 = load i32*, i32** %b.addr, align 8
  %arrayidx6 = getelementptr inbounds i32, i32* %10, i64 %idxprom5
  %11 = load i32, i32* %arrayidx6, align 4
  %rem = srem i32 %11, 2
  %cmp7 = icmp eq i32 %rem, 0
  br i1 %cmp7, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %12 = load i32, i32* %i, align 4
  %idxprom8 = sext i32 %12 to i64
  %13 = load i32*, i32** %a.addr, align 8
  %arrayidx9 = getelementptr inbounds i32, i32* %13, i64 %idxprom8
  %14 = load i32, i32* %arrayidx9, align 4
  %15 = load i32, i32* %i, align 4
  %idxprom10 = sext i32 %15 to i64
  %16 = load i32*, i32** %c.addr, align 8
  %arrayidx11 = getelementptr inbounds i32, i32* %16, i64 %idxprom10
  %17 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %14, %17
  %18 = load i32, i32* %i, align 4
  %idxprom13 = sext i32 %18 to i64
  %19 = load i32*, i32** %d.addr, align 8
  %arrayidx14 = getelementptr inbounds i32, i32* %19, i64 %idxprom13
  store i32 %mul12, i32* %arrayidx14, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %20 = load i32, i32* %i, align 4
  %idxprom15 = sext i32 %20 to i64
  %21 = load i32*, i32** %a.addr, align 8
  %arrayidx16 = getelementptr inbounds i32, i32* %21, i64 %idxprom15
  %22 = load i32, i32* %arrayidx16, align 4
  %23 = load i32, i32* %i, align 4
  %idxprom17 = sext i32 %23 to i64
  %24 = load i32*, i32** %c.addr, align 8
  %arrayidx18 = getelementptr inbounds i32, i32* %24, i64 %idxprom17
  %25 = load i32, i32* %arrayidx18, align 4
  %sub = sub nsw i32 %22, %25
  %26 = load i32, i32* %i, align 4
  %idxprom19 = sext i32 %26 to i64
  %27 = load i32*, i32** %d.addr, align 8
  %arrayidx20 = getelementptr inbounds i32, i32* %27, i64 %idxprom19
  store i32 %sub, i32* %arrayidx20, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %28 = load i32, i32* %i, align 4
  %inc = add nsw i32 %28, 1
  store i32 %inc, i32* %i, align 4
  %29 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %29, 32
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (branches/vpo 5251)"}


;Test case compilation
;clang -O0 -c kernel1.c -S -emit-llvm
;opt < kernel1.ll -loop-rotate -loop-simplify  -o=testfunction.bc
;clang -c testfunction.bc -S -emit-llvm

;void foo(int *a, int  *b , int *c,  int *d, int N)
;{
;    for (int i = 0; i < 32; ++i) {
;        a[i] = b[i] * c[i];
;        if ((b[i]%2==0))
;            d[i] = a[i] * c[i];
;        else
;            d[i] = a[i] - c[i]; 
;    }
;}