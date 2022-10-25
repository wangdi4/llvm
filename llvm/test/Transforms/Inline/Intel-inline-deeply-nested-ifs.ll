; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; INTEL CUSTOMIZATION:

; RUN: opt -inline -inline-threshold=20 -inlining-for-deep-ifs=true -dtrans-inline-heuristics -intel-libirc-allowed -inlining-min-if-depth=4 -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='cgscc(inline)' -inline-threshold=20 -inlining-for-deep-ifs=true -dtrans-inline-heuristics -intel-libirc-allowed -inlining-min-if-depth=4 -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s

; Test checks 'deeply nested IFs' inlining heuristic.


; CHECK: COMPILE FUNC: bar
; CHECK-NEXT: INLINE: foo{{.*}}<<Callee was inlined due to deeply nested ifs>>
; CHECK-NEXT: INLINE: foo{{.*}}<<Callee was inlined due to deeply nested ifs>>


; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %n, i32 %i, i32 %j, i32 %k) #0 {
entry:
  %retval = alloca i32, align 4
  %n.addr = alloca i32, align 4
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  %k.addr = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %j, i32* %j.addr, align 4
  store i32 %k, i32* %k.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = load i32, i32* %i.addr, align 4
  %cmp = icmp sgt i32 %0, %1
  br i1 %cmp, label %land.lhs.true, label %if.else14

land.lhs.true:                                    ; preds = %entry
  %2 = load i32, i32* %n.addr, align 4
  %3 = load i32, i32* %j.addr, align 4
  %cmp1 = icmp slt i32 %2, %3
  br i1 %cmp1, label %land.lhs.true2, label %if.else14

land.lhs.true2:                                   ; preds = %land.lhs.true
  %4 = load i32, i32* %n.addr, align 4
  %5 = load i32, i32* %k.addr, align 4
  %cmp3 = icmp ne i32 %4, %5
  br i1 %cmp3, label %if.then, label %if.else14

if.then:                                          ; preds = %land.lhs.true2
  %6 = load i32, i32* %j.addr, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %j.addr, align 4
  %7 = load i32, i32* %n.addr, align 4
  %8 = load i32, i32* %j.addr, align 4
  %cmp4 = icmp slt i32 %7, %8
  br i1 %cmp4, label %land.lhs.true5, label %if.else13

land.lhs.true5:                                   ; preds = %if.then
  %9 = load i32, i32* %n.addr, align 4
  %10 = load i32, i32* %i.addr, align 4
  %cmp6 = icmp slt i32 %9, %10
  br i1 %cmp6, label %land.lhs.true7, label %if.else13

land.lhs.true7:                                   ; preds = %land.lhs.true5
  %11 = load i32, i32* %n.addr, align 4
  %12 = load i32, i32* %k.addr, align 4
  %cmp8 = icmp eq i32 %11, %12
  br i1 %cmp8, label %if.then9, label %if.else13

if.then9:                                         ; preds = %land.lhs.true7
  %13 = load i32, i32* %k.addr, align 4
  %inc10 = add nsw i32 %13, 1
  store i32 %inc10, i32* %k.addr, align 4
  %14 = load i32, i32* %n.addr, align 4
  %15 = load i32, i32* %k.addr, align 4
  %cmp11 = icmp sgt i32 %14, %15
  br i1 %cmp11, label %if.then12, label %if.else

if.then12:                                        ; preds = %if.then9
  %16 = load i32, i32* %n.addr, align 4
  store i32 %16, i32* %retval, align 4
  br label %return

if.else:                                          ; preds = %if.then9
  %17 = load i32, i32* %k.addr, align 4
  store i32 %17, i32* %retval, align 4
  br label %return

if.else13:                                        ; preds = %land.lhs.true7, %land.lhs.true5, %if.then
  %18 = load i32, i32* %j.addr, align 4
  store i32 %18, i32* %retval, align 4
  br label %return

if.else14:                                        ; preds = %land.lhs.true2, %land.lhs.true, %entry
  %19 = load i32, i32* %i.addr, align 4
  store i32 %19, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.else14, %if.else13, %if.else, %if.then12
  %20 = load i32, i32* %retval, align 4
  ret i32 %20
}

; Function Attrs: nounwind uwtable
define i32 @bar(i32 %i, i32 %a, i32 %b, i32 %c) #0 {
entry:
  %i.addr = alloca i32, align 4
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  store i32 %c, i32* %c.addr, align 4
  store i32 0, i32* %sum, align 4
  %0 = load i32, i32* %i.addr, align 4
  %1 = load i32, i32* %a.addr, align 4
  %cmp = icmp sgt i32 %0, %1
  br i1 %cmp, label %land.lhs.true, label %lor.lhs.false

land.lhs.true:                                    ; preds = %entry
  %2 = load i32, i32* %i.addr, align 4
  %3 = load i32, i32* %b.addr, align 4
  %cmp1 = icmp slt i32 %2, %3
  br i1 %cmp1, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %land.lhs.true, %entry
  %4 = load i32, i32* %i.addr, align 4
  %5 = load i32, i32* %c.addr, align 4
  %cmp2 = icmp slt i32 %4, %5
  br i1 %cmp2, label %if.then, label %if.else

if.then:                                          ; preds = %lor.lhs.false, %land.lhs.true
  %6 = load i32, i32* %i.addr, align 4
  %7 = load i32, i32* %a.addr, align 4
  %8 = load i32, i32* %b.addr, align 4
  %9 = load i32, i32* %c.addr, align 4
  %call = call i32 @foo(i32 %6, i32 %7, i32 %8, i32 %9)
  %10 = load i32, i32* %sum, align 4
  %add = add nsw i32 %10, %call
  store i32 %add, i32* %sum, align 4
  br label %if.end11

if.else:                                          ; preds = %lor.lhs.false
  %11 = load i32, i32* %i.addr, align 4
  %12 = load i32, i32* %b.addr, align 4
  %cmp3 = icmp sgt i32 %11, %12
  br i1 %cmp3, label %land.lhs.true4, label %lor.lhs.false6

land.lhs.true4:                                   ; preds = %if.else
  %13 = load i32, i32* %i.addr, align 4
  %14 = load i32, i32* %a.addr, align 4
  %cmp5 = icmp slt i32 %13, %14
  br i1 %cmp5, label %if.then8, label %lor.lhs.false6

lor.lhs.false6:                                   ; preds = %land.lhs.true4, %if.else
  %15 = load i32, i32* %i.addr, align 4
  %16 = load i32, i32* %c.addr, align 4
  %cmp7 = icmp eq i32 %15, %16
  br i1 %cmp7, label %if.then8, label %if.end

if.then8:                                         ; preds = %lor.lhs.false6, %land.lhs.true4
  %17 = load i32, i32* %i.addr, align 4
  %18 = load i32, i32* %b.addr, align 4
  %19 = load i32, i32* %c.addr, align 4
  %20 = load i32, i32* %a.addr, align 4
  %call9 = call i32 @foo(i32 %17, i32 %18, i32 %19, i32 %20)
  %21 = load i32, i32* %sum, align 4
  %add10 = add nsw i32 %21, %call9
  store i32 %add10, i32* %sum, align 4
  br label %if.end

if.end:                                           ; preds = %if.then8, %lor.lhs.false6
  br label %if.end11

if.end11:                                         ; preds = %if.end, %if.then
  %22 = load i32, i32* %i.addr, align 4
  switch i32 %22, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb13
  ]

sw.bb:                                            ; preds = %if.end11
  %23 = load i32, i32* %c.addr, align 4
  %24 = load i32, i32* %sum, align 4
  %add12 = add nsw i32 %24, %23
  store i32 %add12, i32* %sum, align 4
  br label %sw.epilog

sw.bb13:                                          ; preds = %if.end11
  %25 = load i32, i32* %a.addr, align 4
  %26 = load i32, i32* %sum, align 4
  %add14 = add nsw i32 %26, %25
  store i32 %add14, i32* %sum, align 4
  br label %sw.epilog

sw.default:                                       ; preds = %if.end11
  %27 = load i32, i32* %b.addr, align 4
  %28 = load i32, i32* %sum, align 4
  %add15 = add nsw i32 %28, %27
  store i32 %add15, i32* %sum, align 4
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb13, %sw.bb
  %29 = load i32, i32* %sum, align 4
  %30 = load i32, i32* %c.addr, align 4
  %31 = load i32, i32* %b.addr, align 4
  %32 = load i32, i32* %a.addr, align 4
  %call16 = call i32 @bar(i32 %29, i32 %30, i32 %31, i32 %32)
  ret i32 %call16
}

attributes #0 = { nounwind uwtable }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
; end INTEL_FEATURE_SW_ADVANCED
