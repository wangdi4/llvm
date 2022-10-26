; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; INTEL CUSTOMIZATION:

; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-threshold=20 -inlining-for-deep-ifs=true -dtrans-inline-heuristics -intel-libirc-allowed -inlining-min-if-depth=4 -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s

; Test checks 'deeply nested IFs' inlining heuristic.


; CHECK: COMPILE FUNC: bar
; CHECK-NEXT: INLINE: foo{{.*}}<<Callee was inlined due to deeply nested ifs>>
; CHECK-NEXT: INLINE: foo{{.*}}<<Callee was inlined due to deeply nested ifs>>

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
  store i32 %n, ptr %n.addr, align 4
  store i32 %i, ptr %i.addr, align 4
  store i32 %j, ptr %j.addr, align 4
  store i32 %k, ptr %k.addr, align 4
  %i1 = load i32, ptr %n.addr, align 4
  %i2 = load i32, ptr %i.addr, align 4
  %cmp = icmp sgt i32 %i1, %i2
  br i1 %cmp, label %land.lhs.true, label %if.else14

land.lhs.true:                                    ; preds = %entry
  %i3 = load i32, ptr %n.addr, align 4
  %i4 = load i32, ptr %j.addr, align 4
  %cmp1 = icmp slt i32 %i3, %i4
  br i1 %cmp1, label %land.lhs.true2, label %if.else14

land.lhs.true2:                                   ; preds = %land.lhs.true
  %i5 = load i32, ptr %n.addr, align 4
  %i6 = load i32, ptr %k.addr, align 4
  %cmp3 = icmp ne i32 %i5, %i6
  br i1 %cmp3, label %if.then, label %if.else14

if.then:                                          ; preds = %land.lhs.true2
  %i7 = load i32, ptr %j.addr, align 4
  %inc = add nsw i32 %i7, 1
  store i32 %inc, ptr %j.addr, align 4
  %i8 = load i32, ptr %n.addr, align 4
  %i9 = load i32, ptr %j.addr, align 4
  %cmp4 = icmp slt i32 %i8, %i9
  br i1 %cmp4, label %land.lhs.true5, label %if.else13

land.lhs.true5:                                   ; preds = %if.then
  %i10 = load i32, ptr %n.addr, align 4
  %i11 = load i32, ptr %i.addr, align 4
  %cmp6 = icmp slt i32 %i10, %i11
  br i1 %cmp6, label %land.lhs.true7, label %if.else13

land.lhs.true7:                                   ; preds = %land.lhs.true5
  %i12 = load i32, ptr %n.addr, align 4
  %i13 = load i32, ptr %k.addr, align 4
  %cmp8 = icmp eq i32 %i12, %i13
  br i1 %cmp8, label %if.then9, label %if.else13

if.then9:                                         ; preds = %land.lhs.true7
  %i14 = load i32, ptr %k.addr, align 4
  %inc10 = add nsw i32 %i14, 1
  store i32 %inc10, ptr %k.addr, align 4
  %i15 = load i32, ptr %n.addr, align 4
  %i16 = load i32, ptr %k.addr, align 4
  %cmp11 = icmp sgt i32 %i15, %i16
  br i1 %cmp11, label %if.then12, label %if.else

if.then12:                                        ; preds = %if.then9
  %i17 = load i32, ptr %n.addr, align 4
  store i32 %i17, ptr %retval, align 4
  br label %return

if.else:                                          ; preds = %if.then9
  %i18 = load i32, ptr %k.addr, align 4
  store i32 %i18, ptr %retval, align 4
  br label %return

if.else13:                                        ; preds = %land.lhs.true7, %land.lhs.true5, %if.then
  %i19 = load i32, ptr %j.addr, align 4
  store i32 %i19, ptr %retval, align 4
  br label %return

if.else14:                                        ; preds = %land.lhs.true2, %land.lhs.true, %entry
  %i20 = load i32, ptr %i.addr, align 4
  store i32 %i20, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.else14, %if.else13, %if.else, %if.then12
  %i21 = load i32, ptr %retval, align 4
  ret i32 %i21
}

; Function Attrs: nounwind uwtable
define i32 @bar(i32 %i, i32 %a, i32 %b, i32 %c) #0 {
entry:
  %i.addr = alloca i32, align 4
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 %i, ptr %i.addr, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  store i32 %c, ptr %c.addr, align 4
  store i32 0, ptr %sum, align 4
  %i1 = load i32, ptr %i.addr, align 4
  %i2 = load i32, ptr %a.addr, align 4
  %cmp = icmp sgt i32 %i1, %i2
  br i1 %cmp, label %land.lhs.true, label %lor.lhs.false

land.lhs.true:                                    ; preds = %entry
  %i3 = load i32, ptr %i.addr, align 4
  %i4 = load i32, ptr %b.addr, align 4
  %cmp1 = icmp slt i32 %i3, %i4
  br i1 %cmp1, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %land.lhs.true, %entry
  %i5 = load i32, ptr %i.addr, align 4
  %i6 = load i32, ptr %c.addr, align 4
  %cmp2 = icmp slt i32 %i5, %i6
  br i1 %cmp2, label %if.then, label %if.else

if.then:                                          ; preds = %lor.lhs.false, %land.lhs.true
  %i7 = load i32, ptr %i.addr, align 4
  %i8 = load i32, ptr %a.addr, align 4
  %i9 = load i32, ptr %b.addr, align 4
  %i10 = load i32, ptr %c.addr, align 4
  %call = call i32 @foo(i32 %i7, i32 %i8, i32 %i9, i32 %i10)
  %i11 = load i32, ptr %sum, align 4
  %add = add nsw i32 %i11, %call
  store i32 %add, ptr %sum, align 4
  br label %if.end11

if.else:                                          ; preds = %lor.lhs.false
  %i12 = load i32, ptr %i.addr, align 4
  %i13 = load i32, ptr %b.addr, align 4
  %cmp3 = icmp sgt i32 %i12, %i13
  br i1 %cmp3, label %land.lhs.true4, label %lor.lhs.false6

land.lhs.true4:                                   ; preds = %if.else
  %i14 = load i32, ptr %i.addr, align 4
  %i15 = load i32, ptr %a.addr, align 4
  %cmp5 = icmp slt i32 %i14, %i15
  br i1 %cmp5, label %if.then8, label %lor.lhs.false6

lor.lhs.false6:                                   ; preds = %land.lhs.true4, %if.else
  %i16 = load i32, ptr %i.addr, align 4
  %i17 = load i32, ptr %c.addr, align 4
  %cmp7 = icmp eq i32 %i16, %i17
  br i1 %cmp7, label %if.then8, label %if.end

if.then8:                                         ; preds = %lor.lhs.false6, %land.lhs.true4
  %i18 = load i32, ptr %i.addr, align 4
  %i19 = load i32, ptr %b.addr, align 4
  %i20 = load i32, ptr %c.addr, align 4
  %i21 = load i32, ptr %a.addr, align 4
  %call9 = call i32 @foo(i32 %i18, i32 %i19, i32 %i20, i32 %i21)
  %i22 = load i32, ptr %sum, align 4
  %add10 = add nsw i32 %i22, %call9
  store i32 %add10, ptr %sum, align 4
  br label %if.end

if.end:                                           ; preds = %if.then8, %lor.lhs.false6
  br label %if.end11

if.end11:                                         ; preds = %if.end, %if.then
  %i23 = load i32, ptr %i.addr, align 4
  switch i32 %i23, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb13
  ]

sw.bb:                                            ; preds = %if.end11
  %i24 = load i32, ptr %c.addr, align 4
  %i25 = load i32, ptr %sum, align 4
  %add12 = add nsw i32 %i25, %i24
  store i32 %add12, ptr %sum, align 4
  br label %sw.epilog

sw.bb13:                                          ; preds = %if.end11
  %i26 = load i32, ptr %a.addr, align 4
  %i27 = load i32, ptr %sum, align 4
  %add14 = add nsw i32 %i27, %i26
  store i32 %add14, ptr %sum, align 4
  br label %sw.epilog

sw.default:                                       ; preds = %if.end11
  %i28 = load i32, ptr %b.addr, align 4
  %i29 = load i32, ptr %sum, align 4
  %add15 = add nsw i32 %i29, %i28
  store i32 %add15, ptr %sum, align 4
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb13, %sw.bb
  %i30 = load i32, ptr %sum, align 4
  %i31 = load i32, ptr %c.addr, align 4
  %i32 = load i32, ptr %b.addr, align 4
  %i33 = load i32, ptr %a.addr, align 4
  %call16 = call i32 @bar(i32 %i30, i32 %i31, i32 %i32, i32 %i33)
  ret i32 %call16
}

attributes #0 = { nounwind uwtable }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
; end INTEL_FEATURE_SW_ADVANCED
