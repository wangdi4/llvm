; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xf87f -inline-for-xmain=false -inline-threshold=-1000 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-NEW
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xf8fe < %s -S | opt -passes='cgscc(inline)' -inline-report=0xf8fe -inline-for-xmain=false -inline-threshold=-1000 -S | opt -passes='inlinereportemitter' -inline-report=0xf8fe -inline-threshold=-1000 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

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

; ModuleID = 'main.cpp'
source_filename = "main.cpp"
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
  store i32 0, i32* %retval, align 4
  call void @_ZN8two_intsC2Eii(%class.two_ints* %value_one, i32 1, i32 2)
  call void @_ZN8two_intsC2Eii(%class.two_ints* %value_two, i32 3, i32 4)
  %array.begin = getelementptr inbounds [100 x %class.two_ints], [100 x %class.two_ints]* %a, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.two_ints, %class.two_ints* %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi %class.two_ints* [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN8two_intsC2Ev(%class.two_ints* %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.two_ints, %class.two_ints* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.two_ints* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  call void @_ZN8two_intsC2Ev(%class.two_ints* %result)
  %arraydecay = getelementptr inbounds [100 x %class.two_ints], [100 x %class.two_ints]* %a, i32 0, i32 0
  %0 = bitcast %class.two_ints* %agg.tmp to i8*
  %1 = bitcast %class.two_ints* %value_one to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %0, i8* %1, i64 8, i32 4, i1 false)
  %2 = bitcast %class.two_ints* %agg.tmp1 to i8*
  %3 = bitcast %class.two_ints* %value_two to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %2, i8* %3, i64 8, i32 4, i1 false)
  %4 = bitcast %class.two_ints* %agg.tmp to i64*
  %5 = load i64, i64* %4, align 4
  %6 = bitcast %class.two_ints* %agg.tmp1 to i64*
  %7 = load i64, i64* %6, align 4
  call void @_Z4initP8two_intsiS_S_(%class.two_ints* %arraydecay, i32 100, i64 %5, i64 %7)
  %arraydecay2 = getelementptr inbounds [100 x %class.two_ints], [100 x %class.two_ints]* %a, i32 0, i32 0
  %call = call i64 @_Z5mysumP8two_intsi(%class.two_ints* %arraydecay2, i32 100)
  %8 = bitcast %class.two_ints* %ref.tmp to i64*
  store i64 %call, i64* %8, align 4
  %9 = bitcast %class.two_ints* %result to i8*
  %10 = bitcast %class.two_ints* %ref.tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %10, i64 8, i32 4, i1 false)
  %call3 = call i32 @_ZN8two_ints7int_oneEv(%class.two_ints* %result)
  ret i32 %call3
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN8two_intsC2Eii(%class.two_ints* %this, i32 %int_one, i32 %int_two) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %class.two_ints*, align 8
  %int_one.addr = alloca i32, align 4
  %int_two.addr = alloca i32, align 4
  store %class.two_ints* %this, %class.two_ints** %this.addr, align 8
  store i32 %int_one, i32* %int_one.addr, align 4
  store i32 %int_two, i32* %int_two.addr, align 4
  %this1 = load %class.two_ints*, %class.two_ints** %this.addr, align 8
  %_int_one = getelementptr inbounds %class.two_ints, %class.two_ints* %this1, i32 0, i32 0
  %0 = load i32, i32* %int_one.addr, align 4
  store i32 %0, i32* %_int_one, align 4
  %_int_two = getelementptr inbounds %class.two_ints, %class.two_ints* %this1, i32 0, i32 1
  %1 = load i32, i32* %int_two.addr, align 4
  store i32 %1, i32* %_int_two, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN8two_intsC2Ev(%class.two_ints* %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %class.two_ints*, align 8
  store %class.two_ints* %this, %class.two_ints** %this.addr, align 8
  %this1 = load %class.two_ints*, %class.two_ints** %this.addr, align 8
  %_int_one = getelementptr inbounds %class.two_ints, %class.two_ints* %this1, i32 0, i32 0
  store i32 0, i32* %_int_one, align 4
  %_int_two = getelementptr inbounds %class.two_ints, %class.two_ints* %this1, i32 0, i32 1
  store i32 0, i32* %_int_two, align 4
  ret void
}

declare void @_Z4initP8two_intsiS_S_(%class.two_ints*, i32, i64, i64) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #3

declare i64 @_Z5mysumP8two_intsi(%class.two_ints*, i32) #2

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZN8two_ints7int_oneEv(%class.two_ints* %this) #1 comdat align 2 {
entry:
  %this.addr = alloca %class.two_ints*, align 8
  %i = alloca i32, align 4
  store %class.two_ints* %this, %class.two_ints** %this.addr, align 8
  %this1 = load %class.two_ints*, %class.two_ints** %this.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %1 = load i32, i32* %i, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %_int_one = getelementptr inbounds %class.two_ints, %class.two_ints* %this1, i32 0, i32 0
  %2 = load i32, i32* %_int_one, align 4
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %2, %3
  ret i32 %add
}

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20974)"}
