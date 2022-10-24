; RUN: llvm-as %p/work-group-builtins-64.ll -o %t.lib.bc
; RUN: opt -S -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin %s | FileCheck %s
; RUN: opt -S -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin %s | FileCheck %s
; RUN: opt -S -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

declare signext i8 @_Z20work_group_broadcastcm(i8, i64)
declare <64 x i16> @_Z20work_group_broadcastDv64_tm(<64 x i16>, i64)

define void @test() {
; CHECK: call signext i8 @_Z20work_group_broadcastcmmPc(i8 42, i64 0, i64 {{.*}}, i8* {{.*}})
; CHECK: call <64 x i16> @_Z20work_group_broadcastDv64_tmmPS_(<64 x i16> undef, i64 0, i64 {{.*}}, <64 x i16>* {{.*}})
  %1 = call signext i8 @_Z20work_group_broadcastcm(i8 42, i64 0)
  %2 = call <64 x i16> @_Z20work_group_broadcastDv64_tm(<64 x i16> undef, i64 0)
  ret void
}

;; Instructions inserted by GroupBuiltin should not have debug info
; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-13: WARNING
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
