; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; This test case checks that multiversioning was applied even if the compare
; instruction was passed through a freeze instruction, and then the freeze
; instruction is used for branching.

%struct._ZTS1S.S = type { i8 }

;                                    / Branch
;  Arg - GEP - Load - Cmp NE - Freeze
;                                    \ Select

; CHECK: %3 = freeze i1 true
; CHECK: br i1 %3, label %if.then, label %if.else
; CHECK: %5 = freeze i1 false
; CHECK: br i1 %5, label %if.then.clone, label %if.else.clone

define i32 @foo1(ptr "intel_dtrans_func_index"="1" %Arg) local_unnamed_addr #0 {
entry:
  %0 = getelementptr inbounds %struct._ZTS1S.S, ptr %Arg, i64 0, i32 0
  %1 = load i8, ptr %0, align 1
  %2 = icmp ne i8 %1, 0
  %3 = freeze i1 %2
  br i1 %3, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %4 = select i1 %3, i32 33, i32 22
  ret i32 %4
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 1, !6}
!6 = !{i8 0, i32 0}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !{!9}
!9 = !{%struct._ZTS1S.S zeroinitializer, i32 1}

