; INTEL_FEATURE_CPU_RYL
; INTEL_CUSTOMIZATION
; REQUIRES: intel_feature_cpu_ryl
;
; This test checks several simple invalid cases for RAP region identification in
; machine block placement using the absence of the statistic indicating the number
; of regions identified.
;
; RUN: llc --stats < %s 2>&1 | FileCheck %s
 
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [7 x i8] c"%f %d\0A\00", align 1
@.str1 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1

; Invalid case: non-GPR destination register
; NB: volatile on res prevents creating a crit_edge. While the result would be
; a valid if-then-else RAP region, this avoids any changes to the tests if the
; default value for -enable-rap-ite changes.
;
; Generated with -xroyal -O2 -emit-llvm with manually added !prof from:
; void test_rap_float(long x, long y, float a, long threshold, int volatile *res) {
;  float z = a;
;  if (x * y + x > threshold) {
;    res[0] = 1;
;    res[2] = res[1] + x;
;    z = a * (float) res[2] + a;
;  }
;  res[1] = res[1] + res[2] + (int) (z + a);
; }
define void @test_rap_float(i64 %x, i64 %y, float %a, i64 %threshold, ptr %res) #0 !prof !2 {
entry:
  %x31 = add i64 %y, 1
  %add = mul i64 %x31, %x
  %cmp = icmp sgt i64 %add, %threshold
  br i1 %cmp, label %if.then, label %if.end, !prof !0, !unpredictable !1

if.then:
  store volatile i32 1, ptr %res, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %res, i64 1
  %0 = load volatile i32, ptr %arrayidx1, align 4
  %1 = trunc i64 %x to i32
  %conv3 = add i32 %0, %1
  %arrayidx4 = getelementptr inbounds i32, ptr %res, i64 2
  store volatile i32 %conv3, ptr %arrayidx4, align 4
  %2 = load volatile i32, ptr %arrayidx4, align 4
  %conv6 = sitofp i32 %2 to float
  %mul7 = fmul fast float %conv6, %a
  %add8 = fadd fast float %mul7, %a
  br label %if.end

if.end:
  %z.0 = phi float [ %add8, %if.then ], [ %a, %entry ]
  %arrayidx9 = getelementptr inbounds i32, ptr %res, i64 1
  %3 = load volatile i32, ptr %arrayidx9, align 4
  %arrayidx10 = getelementptr inbounds i32, ptr %res, i64 2
  %4 = load volatile i32, ptr %arrayidx10, align 4
  %add11 = add nsw i32 %4, %3
  %add12 = fadd fast float %z.0, %a
  %conv13 = fptosi float %add12 to i32
  %add14 = add nsw i32 %add11, %conv13
  store volatile i32 %add14, ptr %arrayidx9, align 4
  %conv16 = fpext float %z.0 to double
  %5 = load volatile i32, ptr %arrayidx9, align 4
  %call = tail call i32 (ptr, ...) @printf(ptr @.str, double %conv16, i32 %5)
  ret void
}

; Invalid case: calls or other stack manipulation in then/else blocks
; Generated with -xroyal -O2 -emit-llvm with manually added !prof from:
; void test_rap_call(long x, long y, long threshold, int volatile *res) {
;  if (x * y + x > threshold) {
;    res[0] = 1;
;    res[2] = res[1] + x;
;    printf(" %d\n", res[1]);
;  }
;  res[0] = res[0] + y;
; }
define void @test_rap_call(i64 %x, i64 %y, i64 %threshold, ptr %res) #0 !prof !2 {
entry:
  %x19 = add i64 %y, 1
  %add = mul i64 %x19, %x
  %cmp = icmp sgt i64 %add, %threshold
  br i1 %cmp, label %if.then, label %if.end, !prof !0, !unpredictable !1

if.then:
  store volatile i32 1, ptr %res, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %res, i64 1
  %0 = load volatile i32, ptr %arrayidx1, align 4
  %1 = trunc i64 %x to i32
  %conv3 = add i32 %0, %1
  %arrayidx4 = getelementptr inbounds i32, ptr %res, i64 2
  store volatile i32 %conv3, ptr %arrayidx4, align 4
  %2 = load volatile i32, ptr %arrayidx1, align 4
  %call = tail call i32 (ptr, ...) @printf(ptr @.str1, i32 noundef %2)
  br label %if.end

if.end:
  %3 = load volatile i32, ptr %res, align 4
  %4 = trunc i64 %y to i32
  %conv9 = add i32 %3, %4
  store volatile i32 %conv9, ptr %res, align 4
  ret void
}

declare i32 @printf(ptr, ...)

attributes #0 = { "target-cpu"="royal" }

!0 = !{!"branch_weights", i32 49, i32 50}
!1 = !{i32 1}
!2 = !{!"function_entry_count", i32 10}

; CHECK-NOT: block-placement       - Number of regions with target-preferred layout

; end INTEL_CUSTOMIZATION
; end INTEL_FEATURE_CPU_RYL
