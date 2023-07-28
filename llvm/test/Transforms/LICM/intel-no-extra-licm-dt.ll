; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -enable-intel-advanced-opts -enable-npm-dtrans -debug-pass-manager -passes='default<O3>' -o /dev/null %s 2>&1 | FileCheck %s -check-prefix=NEWPM

; https://reviews.llvm.org/D99249 has inserted an extra LICM pass just before
; loop rotate.
; The extra LICM pass is fusing a key loop in 526.blender, which causes cache
; misses.
; We check the PM output directly, as this is a pass manager related change.

; NEWPM: LoopSimplifyCFGPass
; NEWPM-NOT: LICMPass
; NEWPM: LoopRotatePass

; Function Attrs: nounwind uwtable mustprogress
define dso_local i32 @_Z3fooi(i32 %i) #0 {
entry:
  %retval = alloca i32, align 4
  %i.addr = alloca i32, align 4
  %j = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  store i32 %i, ptr %i.addr, align 4
  store i32 0, ptr %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %j, align 4
  %1 = load i32, ptr %i.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr %j, align 4
  %cmp1 = icmp sgt i32 %2, 10
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  store i32 0, ptr %retval, align 4
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %3 = load i32, ptr %j, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond
for.end:                                          ; preds = %for.cond
  store i32 1, ptr %retval, align 4
  store i32 1, ptr %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %for.end, %if.then
  %4 = load i32, ptr %retval, align 4
  ret i32 %4
}

; end INTEL_FEATURE_SW_ADVANCED
