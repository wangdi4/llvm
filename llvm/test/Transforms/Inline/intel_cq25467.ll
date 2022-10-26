; RUN: opt < %s -S -passes='function-attrs,cgscc(inline)' -inline-report=0xe807 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function-attrs,cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-NEW

; Check that the calls to stmxcsr and ldmcscr are still in the IR after
; inlining. (The new inline report is printed before the IR.)

; CHECK-NEW: INLINE: getp
; CHECK-NEW: INLINE: setp
; CHECK-NEW-NOT: DELETE: setp
; CHECK-NEW: INLINE: getp

; Check that the calls to @getp and @setp are inlined, and that @setp is not
; marked as deleted. @setp should not be deleted because it calls
; @llvm.x86.sse.ldmxcsr which has the side effect of setting the MXCSR
; control/status register.

; CHECK: call void @llvm.x86.sse.stmxcsr
; CHECK: call void @llvm.x86.sse.ldmxcsr
; CHECK: call void @llvm.x86.sse.stmxcsr

; Check that the calls to stmxcsr and ldmcscr are still in the IR after
; inlining. (The old inline report is printed before the IR.)

; CHECK-OLD: INLINE: getp
; CHECK-OLD: INLINE: setp
; CHECK-OLD-NOT: DELETE: setp
; CHECK-OLD: INLINE: getp

@str = private unnamed_addr constant [7 x i8] c"failed\00", align 1
@str.2 = private unnamed_addr constant [7 x i8] c"passed\00", align 1

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.x86.sse.stmxcsr(i8*)
declare void @llvm.x86.sse.ldmxcsr(i8*)
declare noundef i32 @puts(i8* nocapture noundef readonly)

define internal fastcc void @getp(i32* %p) unnamed_addr {
entry:
  %tmp = alloca i32, align 4
  %0 = bitcast i32* %tmp to i8*
  call void @llvm.x86.sse.stmxcsr(i8* nonnull %0)
  %stmxcsr = load i32, i32* %tmp, align 4
  store i32 %stmxcsr, i32* %p, align 4
  ret void
}

define internal fastcc void @setp(i32* %p) unnamed_addr {
entry:
  %tmp = alloca i32, align 4
  %0 = load i32, i32* %p, align 4
  store i32 %0, i32* %tmp, align 4
  %1 = bitcast i32* %tmp to i8*
  call void @llvm.x86.sse.ldmxcsr(i8* nonnull %1)
  ret void
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  %q = alloca i32, align 4
  %r = alloca i32, align 4
  %0 = bitcast i32* %q to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  %1 = bitcast i32* %r to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  call fastcc void @getp(i32* nonnull %q)
  %2 = load i32, i32* %q, align 4
  %add = add i32 %2, 1
  store i32 %add, i32* %r, align 4
  call fastcc void @setp(i32* nonnull %r)
  call fastcc void @getp(i32* nonnull %q)
  %3 = load i32, i32* %q, align 4
  %4 = load i32, i32* %r, align 4
  %cmp = icmp eq i32 %3, %4
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %puts3 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([7 x i8], [7 x i8]* @str.2, i64 0, i64 0))
  br label %cleanup

if.end:                                           ; preds = %entry
  %puts = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([7 x i8], [7 x i8]* @str, i64 0, i64 0))
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 0, %if.then ], [ 1, %if.end ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret i32 %retval.0
}
