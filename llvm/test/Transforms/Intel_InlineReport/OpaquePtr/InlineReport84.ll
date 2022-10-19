; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

target triple = "x86_64-unknown-linux-gnu"

; Check that foo is inlined when -dtrans-inline-heuristics -intel-libirc-allowed is set, despite
; the existence of dynamic alloca %mes

; CHECK-MD: COMPILE FUNC: main
; CHECK-MD: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK: define dso_local i32 @main()
; CHECK-NOT: call i32 @foo
; CHECK-NOT: define internal i32 @foo({{.*}})
; CHECK-CL: COMPILE FUNC: main
; CHECK-CL: INLINE: foo{{.*}}Callee has single callsite and local linkage

declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture)

declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture)

define dso_local i32 @main() local_unnamed_addr {
entry:
  %a = alloca [1000 x i32], align 16
  call void @llvm.lifetime.start.p0i8(i64 4000, ptr nonnull %a)
  %arraydecay = getelementptr inbounds [1000 x i32], ptr %a, i64 0, i64 0
  %call = call i32 @foo(ptr nonnull %arraydecay, i32 1000)
  call void @llvm.lifetime.end.p0i8(i64 4000, ptr nonnull %a)
  ret i32 0
}

define internal i32 @foo(ptr %a, i32 %N) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %N, 50
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %N.addr.0 = phi i32 [ 20, %if.then ], [ %N, %entry ]
  %mes = alloca [4096 x i8], align 16
  %t73 = getelementptr inbounds [4096 x i8], ptr %mes, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 4096, ptr nonnull %t73)
  br label %for.cond

for.cond:                                         ; preds = %for.body, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc, %for.body ]
  %cmp1 = icmp slt i32 %i.0, %N.addr.0
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %i.0 to i64
  %ptridx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  store i32 15, ptr %ptridx, align 4
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4096, ptr nonnull %t73)
  ret i32 0
}
; end INTEL_FEATURE_SW_ADVANCED
