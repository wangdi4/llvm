; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xf87f -inline-for-xmain=false -inline-threshold=-1000 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-NEW
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8fe < %s -S | opt -passes='cgscc(inline)' -inline-report=0xf8fe -inline-for-xmain=false -inline-threshold=-1000 -S | opt -passes='inlinereportemitter' -inline-report=0xf8fe -inline-threshold=-1000 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

; CHECK-OLD: Begin
; CHECK-OLD-NOT: single callsite and local linkage
; CHECK: call i32 @_ZN8two_ints7int_oneEv
; CHECK-NEW: Begin
; CHECK-NEW-NOT: single callsite and local linkage

; Will check that the call to _ZN8two_ints7int_oneEv is not inlined when
; -inline-for-xmain=false is thrown, indicating that he community inlining
; heuristics will be used and not those of xmain.  Also, checks that the
; inlining report does not put out the xmain specific inlining reason
; message "single callsite and local linkage".

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.two_ints = type { i32, i32 }

$_ZN8two_intsC2Eii = comdat any

$_ZN8two_intsC2Ev = comdat any

$_ZN8two_ints7int_oneEv = comdat any

@z = global i32 0, align 4

; Function Attrs: norecurse uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %value_one = alloca %class.two_ints, align 4
  %value_two = alloca %class.two_ints, align 4
  %a = alloca [100 x %class.two_ints], align 16
  %result = alloca %class.two_ints, align 4
  %agg.tmp = alloca %class.two_ints, align 4
  %agg.tmp1 = alloca %class.two_ints, align 4
  %ref.tmp = alloca %class.two_ints, align 4
  store i32 0, ptr %retval, align 4
  call void @_ZN8two_intsC2Eii(ptr %value_one, i32 1, i32 2)
  call void @_ZN8two_intsC2Eii(ptr %value_two, i32 3, i32 4)
  %array.begin = getelementptr inbounds [100 x %class.two_ints], ptr %a, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.two_ints, ptr %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi ptr [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN8two_intsC2Ev(ptr %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.two_ints, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  call void @_ZN8two_intsC2Ev(ptr %result)
  %arraydecay = getelementptr inbounds [100 x %class.two_ints], ptr %a, i32 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg.tmp, ptr align 4 %value_one, i64 8, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg.tmp1, ptr align 4 %value_two, i64 8, i1 false)
  %i5 = load i64, ptr %agg.tmp, align 4
  %i7 = load i64, ptr %agg.tmp1, align 4
  call void @_Z4initP8two_intsiS_S_(ptr %arraydecay, i32 100, i64 %i5, i64 %i7)
  %arraydecay2 = getelementptr inbounds [100 x %class.two_ints], ptr %a, i32 0, i32 0
  %call = call i64 @_Z5mysumP8two_intsi(ptr %arraydecay2, i32 100)
  store i64 %call, ptr %ref.tmp, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %result, ptr align 4 %ref.tmp, i64 8, i1 false)
  %call3 = call i32 @_ZN8two_ints7int_oneEv(ptr %result)
  ret i32 %call3
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN8two_intsC2Eii(ptr %this, i32 %int_one, i32 %int_two) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %int_one.addr = alloca i32, align 4
  %int_two.addr = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store i32 %int_one, ptr %int_one.addr, align 4
  store i32 %int_two, ptr %int_two.addr, align 4
  %this1 = load ptr, ptr %this.addr, align 8
  %_int_one = getelementptr inbounds %class.two_ints, ptr %this1, i32 0, i32 0
  %i = load i32, ptr %int_one.addr, align 4
  store i32 %i, ptr %_int_one, align 4
  %_int_two = getelementptr inbounds %class.two_ints, ptr %this1, i32 0, i32 1
  %i1 = load i32, ptr %int_two.addr, align 4
  store i32 %i1, ptr %_int_two, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN8two_intsC2Ev(ptr %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_int_one = getelementptr inbounds %class.two_ints, ptr %this1, i32 0, i32 0
  store i32 0, ptr %_int_one, align 4
  %_int_two = getelementptr inbounds %class.two_ints, ptr %this1, i32 0, i32 1
  store i32 0, ptr %_int_two, align 4
  ret void
}

declare void @_Z4initP8two_intsiS_S_(ptr, i32, i64, i64) #2

declare i64 @_Z5mysumP8two_intsi(ptr, i32) #2

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZN8two_ints7int_oneEv(ptr %this) #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %i1, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i2 = load i32, ptr %i, align 4
  %inc = add nsw i32 %i2, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %_int_one = getelementptr inbounds %class.two_ints, ptr %this1, i32 0, i32 0
  %i3 = load i32, ptr %_int_one, align 4
  %i4 = load i32, ptr %i, align 4
  %add = add nsw i32 %i3, %i4
  ret i32 %add
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nofree nounwind willreturn }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20974)"}
