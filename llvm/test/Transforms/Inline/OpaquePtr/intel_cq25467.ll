; RUN: opt -opaque-pointers < %s -S -passes='function-attrs,cgscc(inline)' -inline-report=0xe807 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function-attrs,cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-NEW

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

; Function Attrs: argmemonly nounwind writeonly
declare void @llvm.x86.sse.stmxcsr(ptr) #0

; Function Attrs: nounwind
declare void @llvm.x86.sse.ldmxcsr(ptr) #1

declare noundef i32 @puts(ptr nocapture noundef readonly)

define internal fastcc void @getp(ptr %p) unnamed_addr {
entry:
  %tmp = alloca i32, align 4
  call void @llvm.x86.sse.stmxcsr(ptr nonnull %tmp)
  %stmxcsr = load i32, ptr %tmp, align 4
  store i32 %stmxcsr, ptr %p, align 4
  ret void
}

define internal fastcc void @setp(ptr %p) unnamed_addr {
entry:
  %tmp = alloca i32, align 4
  %i = load i32, ptr %p, align 4
  store i32 %i, ptr %tmp, align 4
  call void @llvm.x86.sse.ldmxcsr(ptr nonnull %tmp)
  ret void
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  %q = alloca i32, align 4
  %r = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %q)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %r)
  call fastcc void @getp(ptr nonnull %q)
  %i2 = load i32, ptr %q, align 4
  %add = add i32 %i2, 1
  store i32 %add, ptr %r, align 4
  call fastcc void @setp(ptr nonnull %r)
  call fastcc void @getp(ptr nonnull %q)
  %i3 = load i32, ptr %q, align 4
  %i4 = load i32, ptr %r, align 4
  %cmp = icmp eq i32 %i3, %i4
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %puts3 = call i32 @puts(ptr nonnull dereferenceable(1) @str.2)
  br label %cleanup

if.end:                                           ; preds = %entry
  %puts = call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 0, %if.then ], [ 1, %if.end ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %r)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %q)
  ret i32 %retval.0
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { argmemonly nounwind writeonly }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
