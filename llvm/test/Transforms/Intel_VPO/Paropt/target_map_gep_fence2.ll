; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
;
; Below is the original C source.
;
; unsigned g_sbox[256];
; int main () {
;    unsigned * sbox = g_sbox;
;    #pragma omp target map(to: sbox[0:256])
;    {
;      int dummy=123;
;    }
;    return 0;
; }
;
; Beore the fix, the early-cse substitutes use of %arrayidx with its definition.
;
; Before early-cse:
;
; entry:
;   %sbox = alloca i32*, align 8
;   store i32* getelementptr inbounds ([256 x i32], [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox
;   %sbox.val = load i32*, i32** %sbox, align 8
;   %arrayidx = getelementptr inbounds i32, i32* %sbox.val, i64 0
;   br label %DIR.OMP.TARGET.1
;
; DIR.OMP.TARGET.1:                                 ; preds = %entry
;   %2 = call token @llvm.directive.region.entry() [
;          "DIR.OMP.TARGET"(),
;          "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
;          "QUAL.OMP.MAP.TO"(i32* %sbox.val, i32* %arrayidx, i64 1024, i64 1, i8* null, i8* null)
;          ... ]
;
; Make sure that in prepare pass, %arrayidx and %sbox are renamed, even though %arrayidx has
; no use inside the region.

; Make sure that vpo-paropt-prepare captures %local to a temporary location.
; PREPR: store ptr %arrayidx, ptr [[AADDR:%[a-zA-Z._0-9]+]]
; PREPR: store ptr %sbox.val, ptr [[SADDR:%[a-zA-Z._0-9]+]]
; PREPR: call token @llvm.directive.region.entry()
; And the Value where %local is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause.
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %arrayidx, ptr [[AADDR]])
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %sbox.val, ptr [[SADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR-NOT: store ptr %arrayidx, ptr {{AADDR:%[a-zA-Z._0-9]+}}
; RESTR-NOT: store ptr %sbox.val, ptr {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: call token @llvm.directive.region.entry()
; RESTR-SAME: "QUAL.OMP.MAP.TO"(ptr @g_sbox, ptr @g_sbox, i64 1024, i64 1, ptr null, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@g_sbox = dso_local global [256 x i32] zeroinitializer, align 16

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %sbox = alloca ptr, align 8
  %sbox.map.ptr.tmp = alloca ptr, align 8
  %dummy = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %sbox)
  store ptr @g_sbox, ptr %sbox, align 8
  %sbox.val = load ptr, ptr %sbox, align 8
  %arrayidx = load ptr, ptr %sbox, align 8
  %i1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %sbox.val, ptr %arrayidx, i64 1024, i64 1, ptr null, ptr null), ; MAP type: 1 = 0x1 = TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %dummy, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %sbox.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %sbox.val, ptr %sbox.map.ptr.tmp, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %dummy)
  store i32 123, ptr %dummy, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %dummy)
  call void @llvm.directive.region.exit(token %i1) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.lifetime.end.p0(i64 8, ptr %sbox)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66306, i32 40916905, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
