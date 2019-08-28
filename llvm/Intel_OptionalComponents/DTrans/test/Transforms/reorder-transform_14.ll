; This test verifies that reorder-field transformation (DFR)'s supplementary legal test on dependent types will
; skip any one that is NOT a Inclusive Struct Type (IST).
;

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields -dtrans-reorderfield-enable-applicable-test=0 -dtrans-reorderfield-enable-legal-test=0 | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields -dtrans-reorderfield-enable-applicable-test=0 -dtrans-reorderfield-enable-legal-test=0 | FileCheck %s

; check __DFR_struct.S means DTrans Field-Reorder pass triggered on this struct type
; CHECK: %__DFR_struct.S = type { %struct.S0, i32, i64, i16, i16, i16, i64, i64, i64, i8*, i32*, i16*, i32, i32, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i32, i32 }

; [Note]
; -T is NOT a Inclusive Struct Type of S
; -T is a dependent type of S
; Thus, T will NOT be tested for checkDependentTypeSafety!


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { %struct.S0, i32, i64, i32, i16, i32, i16, i32, i32, i64, i64, i64, i16, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i8*, i32*, i16* }
%struct.S0 = type { i32 }
%struct.T = type { %struct.S*, i32 }

@.str = private unnamed_addr constant [10 x i8] c"sum: %lu\0A\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"T.i: %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @work(%struct.S* %s, i32 %UB1, i32 %UB2) #0 {
entry:
  %s.addr = alloca %struct.S*, align 8
  %UB1.addr = alloca i32, align 4
  %UB2.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i64, align 8
  store %struct.S* %s, %struct.S** %s.addr, align 8
  store i32 %UB1, i32* %UB1.addr, align 4
  store i32 %UB2, i32* %UB2.addr, align 4
  %0 = load i32, i32* %UB1.addr, align 4
  %1 = load %struct.S*, %struct.S** %s.addr, align 8
  %f1 = getelementptr inbounds %struct.S, %struct.S* %1, i32 0, i32 1
  store i32 %0, i32* %f1, align 4
  %2 = load i32, i32* %UB2.addr, align 4
  %conv = trunc i32 %2 to i16
  %3 = load %struct.S*, %struct.S** %s.addr, align 8
  %f4 = getelementptr inbounds %struct.S, %struct.S* %3, i32 0, i32 4
  store i16 %conv, i16* %f4, align 4
  %4 = load i32, i32* %UB2.addr, align 4
  %5 = load %struct.S*, %struct.S** %s.addr, align 8
  %f7 = getelementptr inbounds %struct.S, %struct.S* %5, i32 0, i32 7
  store i32 %4, i32* %f7, align 8
  %6 = load i32, i32* %UB1.addr, align 4
  %conv1 = sext i32 %6 to i64
  %7 = load %struct.S*, %struct.S** %s.addr, align 8
  %f11 = getelementptr inbounds %struct.S, %struct.S* %7, i32 0, i32 11
  store i64 %conv1, i64* %f11, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %8 = load i32, i32* %i, align 4
  %9 = load i32, i32* %UB1.addr, align 4
  %cmp = icmp slt i32 %8, %9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %10 = load %struct.S*, %struct.S** %s.addr, align 8
  %f3 = getelementptr inbounds %struct.S, %struct.S* %10, i32 0, i32 3
  %11 = load i32, i32* %f3, align 8
  %12 = load i32, i32* %i, align 4
  %add = add i32 %11, %12
  %13 = load %struct.S*, %struct.S** %s.addr, align 8
  %f13 = getelementptr inbounds %struct.S, %struct.S* %13, i32 0, i32 1
  %14 = load i32, i32* %f13, align 4
  %add4 = add i32 %14, %add
  store i32 %add4, i32* %f13, align 4
  %15 = load %struct.S*, %struct.S** %s.addr, align 8
  %f75 = getelementptr inbounds %struct.S, %struct.S* %15, i32 0, i32 7
  %16 = load i32, i32* %f75, align 8
  %17 = load i32, i32* %i, align 4
  %add6 = add i32 %16, %17
  %18 = load %struct.S*, %struct.S** %s.addr, align 8
  %f37 = getelementptr inbounds %struct.S, %struct.S* %18, i32 0, i32 3
  %19 = load i32, i32* %f37, align 8
  %add8 = add i32 %19, %add6
  store i32 %add8, i32* %f37, align 8
  %20 = load %struct.S*, %struct.S** %s.addr, align 8
  %f119 = getelementptr inbounds %struct.S, %struct.S* %20, i32 0, i32 11
  %21 = load i64, i64* %f119, align 8
  %22 = load i32, i32* %i, align 4
  %conv10 = sext i32 %22 to i64
  %add11 = add i64 %21, %conv10
  %23 = load %struct.S*, %struct.S** %s.addr, align 8
  %f712 = getelementptr inbounds %struct.S, %struct.S* %23, i32 0, i32 7
  %24 = load i32, i32* %f712, align 8
  %conv13 = zext i32 %24 to i64
  %add14 = add i64 %conv13, %add11
  %conv15 = trunc i64 %add14 to i32
  store i32 %conv15, i32* %f712, align 8
  %25 = load %struct.S*, %struct.S** %s.addr, align 8
  %f316 = getelementptr inbounds %struct.S, %struct.S* %25, i32 0, i32 3
  %26 = load i32, i32* %f316, align 8
  %27 = load i32, i32* %i, align 4
  %add17 = add i32 %26, %27
  %conv18 = zext i32 %add17 to i64
  %28 = load %struct.S*, %struct.S** %s.addr, align 8
  %f1119 = getelementptr inbounds %struct.S, %struct.S* %28, i32 0, i32 11
  %29 = load i64, i64* %f1119, align 8
  %add20 = add i64 %29, %conv18
  store i64 %add20, i64* %f1119, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %30 = load i32, i32* %i, align 4
  %add21 = add nsw i32 %30, 2
  store i32 %add21, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %31 = load %struct.S*, %struct.S** %s.addr, align 8
  %f122 = getelementptr inbounds %struct.S, %struct.S* %31, i32 0, i32 1
  %32 = load i32, i32* %f122, align 4
  %33 = load %struct.S*, %struct.S** %s.addr, align 8
  %f323 = getelementptr inbounds %struct.S, %struct.S* %33, i32 0, i32 3
  %34 = load i32, i32* %f323, align 8
  %add24 = add i32 %32, %34
  %35 = load %struct.S*, %struct.S** %s.addr, align 8
  %f425 = getelementptr inbounds %struct.S, %struct.S* %35, i32 0, i32 4
  %36 = load i16, i16* %f425, align 4
  %conv26 = zext i16 %36 to i32
  %add27 = add i32 %add24, %conv26
  %37 = load %struct.S*, %struct.S** %s.addr, align 8
  %f728 = getelementptr inbounds %struct.S, %struct.S* %37, i32 0, i32 7
  %38 = load i32, i32* %f728, align 8
  %add29 = add i32 %add27, %38
  %conv30 = zext i32 %add29 to i64
  %39 = load %struct.S*, %struct.S** %s.addr, align 8
  %f1131 = getelementptr inbounds %struct.S, %struct.S* %39, i32 0, i32 11
  %40 = load i64, i64* %f1131, align 8
  %add32 = add i64 %conv30, %40
  store i64 %add32, i64* %sum, align 8
  %41 = load i64, i64* %sum, align 8
  ret i64 %41
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  %s = alloca %struct.S*, align 8
  %sum = alloca i64, align 8
  %t = alloca %struct.T, align 8
  store i32 0, i32* %retval, align 4
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  %call = call noalias i8* @malloc(i64 112) #3
  %0 = bitcast i8* %call to %struct.S*
  store %struct.S* %0, %struct.S** %s, align 8
  %1 = load %struct.S*, %struct.S** %s, align 8
  %2 = load i32, i32* %argc.addr, align 4
  %3 = load i32, i32* %argc.addr, align 4
  %add = add nsw i32 %3, 100
  %call1 = call i64 @work(%struct.S* %1, i32 %2, i32 %add)
  store i64 %call1, i64* %sum, align 8
  %4 = load i64, i64* %sum, align 8
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i64 %4)
  %5 = load %struct.S*, %struct.S** %s, align 8
  %6 = bitcast %struct.S* %5 to i8*
  call void @free(i8* %6) #3
  %7 = load i32, i32* %argc.addr, align 4
  %i = getelementptr inbounds %struct.T, %struct.T* %t, i32 0, i32 1
  store i32 %7, i32* %i, align 8
  %i3 = getelementptr inbounds %struct.T, %struct.T* %t, i32 0, i32 1
  %8 = load i32, i32* %i3, align 8
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i64 0, i64 0), i32 %8)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }


;
; *** Source Code ***
;
; cmdline: icx test.c -O0 -S -emit-llvm
;
;#include <stdint.h>
;#include <stdio.h>
;#include <stdlib.h>
;
;struct S0 {
;  int d;
;};
;
;// note: a large struct with 20 fields of integer types, struct types, and pointer types.
;struct S {
;  struct S0 f0;// 1 struct type: model inheritance
;  uint32_t f1;
;  uint64_t f2;
;  uint32_t f3;
;  uint16_t f4;
;  uint32_t f5;
;  uint16_t f6;
;  uint32_t f7;
;  uint32_t f8;
;  uint64_t f9;
;  uint64_t f10;
;  uint64_t f11;
;  uint16_t f12;
;  struct S0 f13; // 4 struct types
;  struct S0 f14;
;  struct S0 f15;
;  struct S0 f16;
;  char *f17; // 3 pointer types
;  int *f18;
;  short *f19;
;};
;
;// type T contains a pointer field to struct S *
;struct T {
;  struct S *base;
;  int i;
;};
;
;
;uint64_t __attribute__((noinline)) work(struct S *s, int UB1, int UB2) {
;  // ONE-time field assign
;  s->f1 = UB1;
;  s->f4 = UB2;
;  s->f7 = UB2;
;  s->f11 = UB1;
;
;  // single-loop assignment with an unknown trip count
;  for (int i = 0; i < UB1; i += 2) {
;    s->f1 += s->f3 + i;
;    s->f3 += s->f7 + i;
;    s->f7 += s->f11 + i;
;    s->f11 += s->f3 + i;
;  }
;
;  // do a sum, so all fields are used
;  uint64_t sum = s->f1 + s->f3 + s->f4 + s->f7 + s->f11;
;
;  return sum;
;}
;
;int main(int argc, char *argv[]) {
;  struct S *s = malloc(sizeof(struct S));
;  uint64_t sum = work(s, argc, argc + 100);
;  printf("sum: %lu\n", sum);
;  free(s);
;
;  struct T t;
;  t.i = argc;
;  printf("T.i: %d\n", t.i);
;
;}
;
;
;