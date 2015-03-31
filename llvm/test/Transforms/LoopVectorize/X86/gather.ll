; RUN: opt < %s  -O3 -mcpu=knl -S -disable-loop-unrolling| FileCheck %s -check-prefix=AVX512

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;void foo1(double * __restrict__ in, double * __restrict__ out, unsigned size, int *__restrict__ trigger, int * __restrict__ index) {
;
;  for (unsigned i=0; i<size; i++) {
;    if (trigger[i] > 0) {
;      out[i] = in[index[i]] + (double) 0.5;
;    }
;  }
;}

; AVX512-LABEL: foo1
; AVX512: icmp sgt
; AVX512: masked.load
; AVX512: masked.gather
; AVX512: masked.store
define void @foo1(double* noalias %in, double* noalias %out, i32 %size, i32* noalias %trigger, i32* noalias %index) {
entry:
  %in.addr = alloca double*, align 8
  %out.addr = alloca double*, align 8
  %size.addr = alloca i32, align 4
  %trigger.addr = alloca i32*, align 8
  %index.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  store double* %in, double** %in.addr, align 8
  store double* %out, double** %out.addr, align 8
  store i32 %size, i32* %size.addr, align 4
  store i32* %trigger, i32** %trigger.addr, align 8
  store i32* %index, i32** %index.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %size.addr, align 4
  %cmp = icmp ult i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %idxprom = zext i32 %2 to i64
  %3 = load i32*, i32** %trigger.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %3, i64 %idxprom
  %4 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %4, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %5 = load i32, i32* %i, align 4
  %idxprom2 = zext i32 %5 to i64
  %6 = load i32*, i32** %index.addr, align 8
  %arrayidx3 = getelementptr inbounds i32, i32* %6, i64 %idxprom2
  %7 = load i32, i32* %arrayidx3, align 4
  %idxprom4 = sext i32 %7 to i64
  %8 = load double*, double** %in.addr, align 8
  %arrayidx5 = getelementptr inbounds double, double* %8, i64 %idxprom4
  %9 = load double, double* %arrayidx5, align 8
  %add = fadd double %9, 5.000000e-01
  %10 = load i32, i32* %i, align 4
  %idxprom6 = zext i32 %10 to i64
  %11 = load double*, double** %out.addr, align 8
  %arrayidx7 = getelementptr inbounds double, double* %11, i64 %idxprom6
  store double %add, double* %arrayidx7, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %12 = load i32, i32* %i, align 4
  %inc = add i32 %12, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
