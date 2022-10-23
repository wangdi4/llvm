; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -new-double-callsite-inlining-heuristics=true -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -new-double-callsite-inlining-heuristics=true -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CHECK: Callee has double callsite without local linkage
; This LIT tets checks the worthy double external callsite heuristic
; The criteria for this heuristic are:
;    (1) Single basic block in the caller
;    (2) Single use which is not a direct invocation of the caller
;    (3) No calls to functions other than the called function
;       (except intrinsics added by using -g)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = global i32 78, align 4
@y = global i32 20, align 4

; Function Attrs: nounwind uwtable
define i32 @bar(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %s = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  store i32 0, ptr %s, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %i1, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i2 = load i32, ptr %s, align 4
  %i3 = load i32, ptr %x.addr, align 4
  %add = add nsw i32 %i2, %i3
  store i32 %add, ptr %s, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i4 = load i32, ptr %i, align 4
  %inc = add nsw i32 %i4, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr %i, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc5, %for.end
  %i5 = load i32, ptr %i, align 4
  %cmp2 = icmp slt i32 %i5, 10
  br i1 %cmp2, label %for.body3, label %for.end7

for.body3:                                        ; preds = %for.cond1
  %i6 = load i32, ptr %s, align 4
  %i7 = load i32, ptr @y, align 4
  %add4 = add nsw i32 %i6, %i7
  store i32 %add4, ptr %s, align 4
  br label %for.inc5

for.inc5:                                         ; preds = %for.body3
  %i8 = load i32, ptr %i, align 4
  %inc6 = add nsw i32 %i8, 1
  store i32 %inc6, ptr %i, align 4
  br label %for.cond1

for.end7:                                         ; preds = %for.cond1
  store i32 0, ptr %i, align 4
  br label %for.cond8

for.cond8:                                        ; preds = %for.inc12, %for.end7
  %i9 = load i32, ptr %i, align 4
  %cmp9 = icmp slt i32 %i9, 10
  br i1 %cmp9, label %for.body10, label %for.end14

for.body10:                                       ; preds = %for.cond8
  %i10 = load i32, ptr %s, align 4
  %i11 = load i32, ptr %x.addr, align 4
  %add11 = add nsw i32 %i10, %i11
  store i32 %add11, ptr %s, align 4
  br label %for.inc12

for.inc12:                                        ; preds = %for.body10
  %i12 = load i32, ptr %i, align 4
  %inc13 = add nsw i32 %i12, 1
  store i32 %inc13, ptr %i, align 4
  br label %for.cond8

for.end14:                                        ; preds = %for.cond8
  store i32 0, ptr %i, align 4
  br label %for.cond15

for.cond15:                                       ; preds = %for.inc19, %for.end14
  %i13 = load i32, ptr %i, align 4
  %cmp16 = icmp slt i32 %i13, 10
  br i1 %cmp16, label %for.body17, label %for.end21

for.body17:                                       ; preds = %for.cond15
  %i14 = load i32, ptr %s, align 4
  %i15 = load i32, ptr @y, align 4
  %add18 = add nsw i32 %i14, %i15
  store i32 %add18, ptr %s, align 4
  br label %for.inc19

for.inc19:                                        ; preds = %for.body17
  %i16 = load i32, ptr %i, align 4
  %inc20 = add nsw i32 %i16, 1
  store i32 %inc20, ptr %i, align 4
  br label %for.cond15

for.end21:                                        ; preds = %for.cond15
  store i32 0, ptr %i, align 4
  br label %for.cond22

for.cond22:                                       ; preds = %for.inc26, %for.end21
  %i17 = load i32, ptr %i, align 4
  %cmp23 = icmp slt i32 %i17, 10
  br i1 %cmp23, label %for.body24, label %for.end28

for.body24:                                       ; preds = %for.cond22
  %i18 = load i32, ptr %s, align 4
  %i19 = load i32, ptr %x.addr, align 4
  %add25 = add nsw i32 %i18, %i19
  store i32 %add25, ptr %s, align 4
  br label %for.inc26

for.inc26:                                        ; preds = %for.body24
  %i20 = load i32, ptr %i, align 4
  %inc27 = add nsw i32 %i20, 1
  store i32 %inc27, ptr %i, align 4
  br label %for.cond22

for.end28:                                        ; preds = %for.cond22
  store i32 0, ptr %i, align 4
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc33, %for.end28
  %i21 = load i32, ptr %i, align 4
  %cmp30 = icmp slt i32 %i21, 10
  br i1 %cmp30, label %for.body31, label %for.end35

for.body31:                                       ; preds = %for.cond29
  %i22 = load i32, ptr %s, align 4
  %i23 = load i32, ptr %x.addr, align 4
  %add32 = add nsw i32 %i22, %i23
  store i32 %add32, ptr %s, align 4
  br label %for.inc33

for.inc33:                                        ; preds = %for.body31
  %i24 = load i32, ptr %i, align 4
  %inc34 = add nsw i32 %i24, 1
  store i32 %inc34, ptr %i, align 4
  br label %for.cond29

for.end35:                                        ; preds = %for.cond29
  store i32 0, ptr %i, align 4
  br label %for.cond36

for.cond36:                                       ; preds = %for.inc40, %for.end35
  %i25 = load i32, ptr %i, align 4
  %cmp37 = icmp slt i32 %i25, 10
  br i1 %cmp37, label %for.body38, label %for.end42

for.body38:                                       ; preds = %for.cond36
  %i26 = load i32, ptr %s, align 4
  %i27 = load i32, ptr @y, align 4
  %add39 = add nsw i32 %i26, %i27
  store i32 %add39, ptr %s, align 4
  br label %for.inc40

for.inc40:                                        ; preds = %for.body38
  %i28 = load i32, ptr %i, align 4
  %inc41 = add nsw i32 %i28, 1
  store i32 %inc41, ptr %i, align 4
  br label %for.cond36

for.end42:                                        ; preds = %for.cond36
  store i32 0, ptr %i, align 4
  br label %for.cond43

for.cond43:                                       ; preds = %for.inc47, %for.end42
  %i29 = load i32, ptr %i, align 4
  %cmp44 = icmp slt i32 %i29, 10
  br i1 %cmp44, label %for.body45, label %for.end49

for.body45:                                       ; preds = %for.cond43
  %i30 = load i32, ptr %s, align 4
  %i31 = load i32, ptr %x.addr, align 4
  %add46 = add nsw i32 %i30, %i31
  store i32 %add46, ptr %s, align 4
  br label %for.inc47

for.inc47:                                        ; preds = %for.body45
  %i32 = load i32, ptr %i, align 4
  %inc48 = add nsw i32 %i32, 1
  store i32 %inc48, ptr %i, align 4
  br label %for.cond43

for.end49:                                        ; preds = %for.cond43
  store i32 0, ptr %i, align 4
  br label %for.cond50

for.cond50:                                       ; preds = %for.inc54, %for.end49
  %i33 = load i32, ptr %i, align 4
  %cmp51 = icmp slt i32 %i33, 10
  br i1 %cmp51, label %for.body52, label %for.end56

for.body52:                                       ; preds = %for.cond50
  %i34 = load i32, ptr %s, align 4
  %i35 = load i32, ptr @y, align 4
  %add53 = add nsw i32 %i34, %i35
  store i32 %add53, ptr %s, align 4
  br label %for.inc54

for.inc54:                                        ; preds = %for.body52
  %i36 = load i32, ptr %i, align 4
  %inc55 = add nsw i32 %i36, 1
  store i32 %inc55, ptr %i, align 4
  br label %for.cond50

for.end56:                                        ; preds = %for.cond50
  store i32 0, ptr %i, align 4
  br label %for.cond57

for.cond57:                                       ; preds = %for.inc61, %for.end56
  %i37 = load i32, ptr %i, align 4
  %cmp58 = icmp slt i32 %i37, 10
  br i1 %cmp58, label %for.body59, label %for.end63

for.body59:                                       ; preds = %for.cond57
  %i38 = load i32, ptr %s, align 4
  %i39 = load i32, ptr %x.addr, align 4
  %add60 = add nsw i32 %i38, %i39
  store i32 %add60, ptr %s, align 4
  br label %for.inc61

for.inc61:                                        ; preds = %for.body59
  %i40 = load i32, ptr %i, align 4
  %inc62 = add nsw i32 %i40, 1
  store i32 %inc62, ptr %i, align 4
  br label %for.cond57

for.end63:                                        ; preds = %for.cond57
  %i41 = load i32, ptr %x.addr, align 4
  %i42 = load i32, ptr @y, align 4
  %add64 = add nsw i32 %i41, %i42
  %i43 = load i32, ptr %s, align 4
  %add65 = add nsw i32 %add64, %i43
  ret i32 %add65
}

; Function Attrs: noinline nounwind uwtable
define i32 @foo() #1 {
entry:
  %call = call i32 @bar(i32 5)
  %call1 = call i32 @bar(i32 10)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

; Function Attrs: noinline nounwind uwtable
define i32 @baz(ptr %mine) #1 {
entry:
  %mine.addr = alloca ptr, align 8
  store ptr %mine, ptr %mine.addr, align 8
  %i = load ptr, ptr %mine.addr, align 8
  %call = call i32 %i(i32 15)
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %call = call i32 @baz(ptr @foo)
  ret i32 %call
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20715)"}
; end INTEL_FEATURE_SW_ADVANCED
