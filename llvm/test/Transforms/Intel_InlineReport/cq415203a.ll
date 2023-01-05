; It checks that the function bar was inlined and then removed by
; dead static function elimination.  It also checks that the main
; function is still there, and that the call to @llvm.va_arg_pack_len
; has been removed, so that the main function returns 4.

; ModuleID = 'cq415203a.cpp'
source_filename = "cq415203a.cpp"

; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe801 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CL,CHECK
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe880 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe880 -S | opt -passes='inlinereportemitter' -inline-report=0xe880 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD,CHECK

; CHECK-MD: -> INLINE: {{.*}}bar{{.*}}
; CHECK-MD: DEAD STATIC FUNC: {{.*}}bar{{.*}}
; CHECK: define i32 @main()
; CHECK-NOT: @llvm.va_arg_pack_len
; CHECK: ret i32 4
; CHECK-CL: DEAD STATIC FUNC: {{.*}}bar{{.*}}
; CHECK-CL: -> INLINE: {{.*}}bar{{.*}}
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 (i32, ...) @_ZL3bariz(i32 1, i32 2, i32 3, i32 4, i32 5)
  ret i32 %call
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define internal i32 @_ZL3bariz(i32 %x, ...) #1 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = call i32 @llvm.va_arg_pack_len()
  ret i32 %0
}

; Function Attrs: nounwind
declare i32 @llvm.va_arg_pack_len() #2

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20447) (llvm/branches/ltoprof 20619)"}
