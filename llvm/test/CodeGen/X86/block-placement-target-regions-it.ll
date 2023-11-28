; INTEL_FEATURE_CPU_RYL
; INTEL_CUSTOMIZATION
; REQUIRES: intel_feature_cpu_ryl
; This test checks for basic functionality of the machine block placement feature to allow
; target-selected region layouts. This is currently only excercised under the -enable-rap option or
; by default with -xroyal. 
; With -xroyal, we have a target heuristic which forces wedge and hammock shapes to be 
; laid out contiguously without padding under certain conditions:
;  - the branch is unpredictable (either via metadata or using static probabilities)
;  - the then/else blocks are sufficiently small (conditional branch and converge point are within 64b)
;  - the instructions in the then/else blocks are non-microcoded, have only GPR/RFLAGS/EFLAGS def operands,
;    amongst other conditions (see X86InstrInfo.cpp:isLegalForRAP()).
; The preferred layout is any which has the then/else blocks in-line (no backwards jumps) and
; without padding.
; 
; The unpredictability test uses profile-derived !unpredictable metadata available, and 
; defaults to static probabilities (near .5) unless -require-profile-for-rap is used.
; 
; Since block placement is free to use the preferred layout in other cases, these tests use
; the block-placement statistic 'NumTargetPreferredRegions' which counts the number of cases
; where TII identified a preferred ordering during block placement.
;
; RUN: llc --stats < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Valid RAP region (wedge/if-then)
; Generated with -xroyal -O2 -emit-llvm with manually added !prof/!unpredictable md from:
; void test_rap_it(long x, long y, long threshold, int volatile *res) {
;   if (x * y + x > threshold) {
;     res[0] = 1;
;     res[2] = res[1] + x;
;   }
;   res[0] = res[0] + y;
; }
define void @test_rap_it(i64 %x, i64 %y, i64 %threshold, ptr %res) #0 !prof !2 {
entry:
  %x17 = add i64 %y, 1
  %add = mul i64 %x17, %x
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
  br label %if.end

if.end:
  %2 = load volatile i32, ptr %res, align 4
  %3 = trunc i64 %y to i32
  %conv8 = add i32 %2, %3
  store volatile i32 %conv8, ptr %res, align 4
  ret void
}

attributes #0 = { "target-cpu"="royal" }

!0 = !{!"branch_weights", i32 49, i32 50}
!1 = !{}
!2 = !{!"function_entry_count", i32 10}

; CHECK:   1 block-placement       - Number of regions with target-preferred layout

; end INTEL_CUSTOMIZATION
; end INTEL_FEATURE_CPU_RYL
