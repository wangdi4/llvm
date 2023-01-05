; RUN: opt -passes="loop(indvars)" < %s -enable-intel-advanced-opts=true -scalar-evolution-use-expensive-range-sharpening -S | FileCheck %s

; CMPLRLLVM-34056:
; %iv and %val1 have the same value.
; "iv" is widened to 64 bits.
; "val1" may be replaced by creating a trunc instruction to convert "iv" back
; to 32 bits.
; We prefer not to do this for HIR, as it does not seem to improve performance,
; and it adds extra instructions that disturb HIR's cost modeling.

target triple = "x86_64--linux-gnu"
target datalayout = "n8:16:32:64"

declare void @foo(i64 %v)
declare void @bar(i32 %v)

define void @test1() #0 {
; CHECK-LABEL: @test1(
; CHECK:       loop:
; CHECK:         [[INDVARS_IV:%.*]] = phi i64 {{.*}}
; CHECK:         [[INDVARS_IV_NEXT:%.*]] = add nuw nsw i64 [[INDVARS_IV]], 1
; CHECK-NOT:     trunc i64 [[INDVARS_IV_NEXT]] to i32
;
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %iv = phi i32 [ %iv.next, %loop ], [ 0, %entry ]
  %val1 = phi i32 [ %val1.inc, %loop ], [ 0, %entry ]
  %val1.inc = add i32 %val1, 1
  %iv.next = add i32 %iv, 1
  call void @bar(i32 %val1.inc)
  %iv.wide = zext i32 %iv to i64
  call void @foo(i64 %iv.wide)
  %loop.cond = icmp eq i32 %iv, 1000
  br i1 %loop.cond, label %exit, label %loop

exit:                                             ; preds = %loop
  ret void
}

attributes #0 = { "target-cpu"="skylake" "target-features"="+adx,+aes,+avx,+avx2,+bmi,+bmi2,+clflushopt,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sgx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "pre_loopopt" }

