; It checks that the call to myprintf has been inlined and removed by
; dead static function elimination.  It then checks that the function
; main still exists, and that the call to @llvm.va_arg_pack has been
; removed and that the function main returns 0.

; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe801 < %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-CL,CHECK
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe880 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe880 -S | opt -passes='inlinereportemitter' -inline-report=0xe880 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD,CHECK

; CHECK-MD: -> INLINE: {{.*}}myprintf{{.*}}
; CHECK-MD:DEAD STATIC FUNC: {{.*}}myprintf{{.*}}
; CHECK: define i32 @main()
; CHECK-NOT: call i32 @llvm.va_arg_pack
; CHECK: ret i32 0
; CHECK-CL:DEAD STATIC FUNC: {{.*}}myprintf{{.*}}
; CHECK-CL: -> INLINE: {{.*}}myprintf{{.*}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@stderr = external global ptr, align 8
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@_ZL4xcox = internal global i32 234, align 4
@.str.1 = private unnamed_addr constant [11 x i8] c"myprintf: \00", align 1

; Function Attrs: norecurse uwtable
define i32 @main() #0 {
entry:
  %i = load ptr, ptr @stderr, align 8
  %i1 = load i32, ptr @_ZL4xcox, align 4
  %call = call i32 (ptr, ptr, ...) @_ZL8myprintfP8_IO_FILEPKcz(ptr %i, ptr @.str, i32 %i1)
  ret i32 0
}

; Function Attrs: inlinehint uwtable
define internal i32 @_ZL8myprintfP8_IO_FILEPKcz(ptr %f, ptr %format, ...) #1 {
entry:
  %retval = alloca i32, align 4
  %f.addr = alloca ptr, align 8
  %format.addr = alloca ptr, align 8
  %r = alloca i32, align 4
  %s = alloca i32, align 4
  store ptr %f, ptr %f.addr, align 8
  store ptr %format, ptr %format.addr, align 8
  %i = load ptr, ptr %f.addr, align 8
  %call = call i32 (ptr, ptr, ...) @fprintf(ptr %i, ptr @.str.1)
  store i32 %call, ptr %r, align 4
  %i1 = load i32, ptr %r, align 4
  %cmp = icmp slt i32 %i1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i2 = load i32, ptr %r, align 4
  store i32 %i2, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %i3 = load ptr, ptr %f.addr, align 8
  %i4 = load ptr, ptr %format.addr, align 8
  %i5 = call i32 @llvm.va_arg_pack()
  %call1 = call i32 (ptr, ptr, ...) @fprintf(ptr %i3, ptr %i4, i32 %i5)
  store i32 %call1, ptr %s, align 4
  %i6 = load i32, ptr %s, align 4
  %cmp2 = icmp slt i32 %i6, 0
  br i1 %cmp2, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  %i7 = load i32, ptr %s, align 4
  store i32 %i7, ptr %retval, align 4
  br label %return

if.end4:                                          ; preds = %if.end
  %i8 = load i32, ptr %r, align 4
  %i9 = load i32, ptr %s, align 4
  %add = add nsw i32 %i8, %i9
  store i32 %add, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end4, %if.then3, %if.then
  %i10 = load i32, ptr %retval, align 4
  ret i32 %i10
}

declare i32 @fprintf(ptr, ptr, ...) #2

; Function Attrs: nounwind
declare i32 @llvm.va_arg_pack() #3

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20447) (llvm/branches/ltoprof 20619)"}
