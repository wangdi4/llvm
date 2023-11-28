; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; extern int *a, *b;
; extern bool c1, c2, c3;
; void foo() {
;   #pragma omp target enter data map(to:a[:1]) if (c1)
;   #pragma omp target update to(b[:1]) if (c2)
;   #pragma omp target exit data map(from:a[:1]) if (c3)
; }

; The test IR is a simplified version of the above test.

; Check that the baseptrs allocations for all three constructs happen in the
; original, foo, and no other outlined function is created.
; CHECK-LABEL: define dso_local void @foo()
; CHECK-COUNT-3: %.offload_baseptrs{{.*}} = alloca [1 x ptr], align 8
; CHECK-NOT: define{{.*}}foo.{{.*}}

; Check for the branches and RTL calls emitted for the constructs.
; CHECK: %[[CHECK1:.+]] = icmp ne i1 %c1.tobool, false
; CHECK: br i1 %[[CHECK1]], label %[[IF_THEN1:.+]], label %[[IF_ELSE1:.+]]
; CHECK: [[IF_THEN1]]:
; CHECK:   %{{.+}} = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs{{.*}}, i32 0, i32 0
; CHECK:   call void @__tgt_target_data_begin_mapper
; CHECK:   br label %[[IF_END1:.+]]
; CHECK: [[IF_ELSE1]]:
; CHECK:   br label %[[IF_END1]]

; CHECK: %[[CHECK2:.+]] = icmp ne i1 %c2.tobool, false
; CHECK: br i1 %[[CHECK2]], label %[[IF_THEN2:.+]], label %[[IF_ELSE2:.+]]
; CHECK: [[IF_THEN2]]:
; CHECK:   %{{.+}} = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs{{.*}}, i32 0, i32 0
; CHECK:   call void @__tgt_target_data_update_mapper
; CHECK:   br label %[[IF_END2:.+]]
; CHECK: [[IF_ELSE2]]:
; CHECK:   br label %[[IF_END2]]

; CHECK: %[[CHECK3:.+]] = icmp ne i1 %c3.tobool, false
; CHECK: br i1 %[[CHECK3]], label %[[IF_THEN3:.+]], label %[[IF_ELSE3:.+]]
; CHECK: [[IF_THEN3]]:
; CHECK:   %{{.+}} = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs{{.*}}, i32 0, i32 0
; CHECK:   call void @__tgt_target_data_end_mapper
; CHECK:   br label %[[IF_END3:.+]]
; CHECK: [[IF_ELSE3]]:
; CHECK:   br label %[[IF_END3]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@a = external dso_local global ptr, align 8
@b = external dso_local global ptr, align 8
@c1 = external dso_local global i8, align 1
@c2 = external dso_local global i8, align 1
@c3 = external dso_local global i8, align 1


define dso_local void @foo() {
entry:
  %0 = load ptr, ptr @a, align 8
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 0
  %c1.load = load i8, ptr @c1, align 1
  %c1.tobool = trunc i8 %c1.load to i1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.IF"(i1 %c1.tobool),
    "QUAL.OMP.MAP.TO"(ptr @a, ptr %arrayidx, i64 4, i64 17, ptr null, ptr null) ] ; MAP type: 17 = 0x11 = PTR_AND_OBJ (0x10) | TO (0x1)

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]

  %2 = load ptr, ptr @b, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %2, i64 0
  %c2.load = load i8, ptr @c2, align 1
  %c2.tobool = trunc i8 %c2.load to i1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.IF"(i1 %c2.tobool),
    "QUAL.OMP.MAP.TO"(ptr @b, ptr %arrayidx1, i64 4, i64 17, ptr null, ptr null) ] ; MAP type: 17 = 0x11 = PTR_AND_OBJ (0x10) | TO (0x1)

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET.UPDATE"() ]

  %4 = load ptr, ptr @a, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %4, i64 0
  %c3.load = load i8, ptr @c3, align 1
  %c3.tobool = trunc i8 %c3.load to i1
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.IF"(i1 %c3.tobool),
    "QUAL.OMP.MAP.FROM"(ptr @a, ptr %arrayidx2, i64 4, i64 18, ptr null, ptr null) ] ; MAP type: 18 = 0x12 = PTR_AND_OBJ (0x10) | FROM (0x2)

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
