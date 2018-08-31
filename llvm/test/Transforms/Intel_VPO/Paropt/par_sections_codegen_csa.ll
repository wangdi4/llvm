; RUN: opt < %s -domtree -loops -lcssa-verification -vpo-wrncollection -vpo-wrninfo -loops -vpo-paropt -S | FileCheck %s
; REQUIRES: csa-registered-target
;
; Check paropt lowering of "omp parallel sections" for CSA target.

target triple = "csa"

; CHECK-LABEL: @foo
define void @foo() {
entry:
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.SECTIONS"() ]
  br label %DIR.OMP.PARALLEL.SECTIONS.1

DIR.OMP.PARALLEL.SECTIONS.1:
; CHECK: [[SECTION1:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.SECTION.2

DIR.OMP.SECTION.2:
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION1]])
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.3

DIR.OMP.END.SECTION.3:
; CHECK: [[SECTION2:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.SECTION.4

DIR.OMP.SECTION.4:
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION2]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.5

DIR.OMP.END.SECTION.5:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.SECTIONS"() ]
  br label %DIR.OMP.END.PARALLEL.SECTIONS.1

DIR.OMP.END.PARALLEL.SECTIONS.1:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
