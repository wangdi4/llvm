; INTEL_FEATURE_SW_ADVANCED
; Inline report
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes='cgscc(inline),module(ip-cloning)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck  --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline),module(ip-cloning)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; Test that @mynoclone is inlined even though it is a potential candidate
; for cloning for specialization, as cloning for specialization will not
; occur after inlining.

@myglobalarray = common dso_local global [500 x i32] zeroinitializer, align 16

declare dso_local i32 @rand()

define dso_local i32 @main() {
entry:
  %call = call i32 @rand() #3
  %cmp = icmp sgt i32 %call, 7
  br i1 %cmp, label %if.then, label %if.end
if.then:                                          ; preds = %entry
  %call1 = call i32 @rand() #3
  br label %if.end
if.end:                                           ; preds = %if.then, %entry
  %myarg.0 = phi i32 [ %call1, %if.then ], [ %call, %entry ]
  %call2 = call i32 @mynoclone(i32 %myarg.0)
  ret i32 %call2
}

define internal i32 @mynoclone(i32 %bound) {
entry:
  br label %for.cond
for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %bound
  br i1 %cmp, label %for.body, label %for.end
for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [500 x i32], ptr @myglobalarray, i64 0, i64 %idxprom
  store i32 23, ptr %arrayidx, align 4
  %inc = add nsw i32 %i.0, 1
  br label %for.cond
for.end:                                          ; preds = %for.cond
  ret i32 23
}

; CHECK-OLD: INLINE: mynoclone{{.*}}Callee has single callsite and local linkage
; CHECK-OLD-NOT: i32 call i32 @mynoclone

; CHECK-NEW-NOT: i32 call i32 @mynoclone
; CHECK-NEW: INLINE: mynoclone{{.*}}Callee has single callsite and local linkage

; end INTEL_FEATURE_SW_ADVANCED
