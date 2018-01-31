; RUN: llc < %s
; The only requirement here is that we don't crash.
; ModuleID = 'creduce-register_probs.cpp'
source_filename = "creduce-register_probs.cpp"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

@a = local_unnamed_addr global i32 0, align 4
@b = local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind
define void @_Z3fn1b(i1 zeroext %p1) local_unnamed_addr #0 {
entry:
  br i1 %p1, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = load i32, i32* @b, align 4, !tbaa !2
  store i32 %0, i32* @a, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 8fca8f5a48ba4efa850dafc4ba4bdbd000cb921b)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
