; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; Check that multi-versioning is not happening for unsafe loads.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS1S.S = type { i8 }

declare void @bar(ptr "intel_dtrans_func_index"="1") local_unnamed_addr

; Check that multi-versioning is still happening when unsafe loads are removed
; from the closure. The third load+cmp+select should be left unchanged.
;
;  CHECK: define dso_local i32 @foo1
;  CHECK: br i1 true
;  CHECK: select i1 true
;  CHECK: call void @bar
;  CHECK: load i8, ptr %0
;  CHECK: icmp ne i8 %
;  CHECK: select i1 %

;  CHECK: br i1 false
;  CHECK: select i1 false
;  CHECK: call void @bar
;  CHECK: load i8, ptr %0
;  CHECK: icmp ne i8 %
;  CHECK: select i1 %

; Function Attrs: nounwind uwtable
define dso_local i32 @foo1(ptr noundef "intel_dtrans_func_index"="1" %Arg) #0 !intel.dtrans.func.type !8 {
entry:
  %field0 = getelementptr inbounds %struct._ZTS1S.S, ptr %Arg, i32 0, i32 0
  %t1 = load i8, ptr %field0, align 1
  %t2 = icmp ne i8 %t1, 0
  br i1 %t2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %t3 = load i8, ptr %field0, align 1
  %t4 = icmp ne i8 %t3, 0
  %t5 = select i1 %t4, i32 33, i32 22
  call void @bar(ptr nonnull %Arg)
  %t6 = load i8, ptr %field0, align 1
  %t7 = icmp ne i8 %t6, 0
  %t8 = select i1 %t7, i32 33, i32 22
  %t9 = add i32 %t5, %t8
  ret i32 %t9
}

; Check that multi-versioning is not happening when moving load to entry block
; is not safe. The second load is unsafe and thus should be removed from the
; closure. After that there will be unsufficient number of conditional uses.

; CHECK: define dso_local i32 @foo2
; CHECK-NOT: br i1 true
; CHECK-NOT: select i1 true
; CHECK-NOT: br i1 false
; CHECK-NOT: select i1 false

; Function Attrs: nounwind uwtable
define dso_local i32 @foo2(ptr noundef "intel_dtrans_func_index"="1" %Arg) #0 !intel.dtrans.func.type !8 {
entry:
  %field0 = getelementptr inbounds %struct._ZTS1S.S, ptr %Arg, i32 0, i32 0
  %t1 = load i8, ptr %field0, align 1
  %t2 = icmp ne i8 %t1, 0
  br i1 %t2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @bar(ptr nonnull %Arg)
  %t3 = load i8, ptr %field0, align 1
  %t4 = icmp ne i8 %t3, 0
  %t5 = select i1 %t4, i32 33, i32 22
  ret i32 %t5
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
