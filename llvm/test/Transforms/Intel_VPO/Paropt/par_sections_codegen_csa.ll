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

; CHECK-LABEL: @foo.par.and.sections
define void @foo.par.and.sections() {
entry:
  br label %DIR.OMP.PARALLEL

DIR.OMP.PARALLEL:
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.SECTIONS

DIR.OMP.SECTIONS:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  br label %DIR.OMP.SECTION.1

DIR.OMP.SECTION.1:
; CHECK: [[SECTION1:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.END.SECTION.1

DIR.OMP.END.SECTION.1:
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION1]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.SECTION.2

DIR.OMP.SECTION.2:
; CHECK: [[SECTION2:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.END.SECTION.2

DIR.OMP.END.SECTION.2:
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION2]])
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTIONS

DIR.OMP.END.SECTIONS:
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTIONS"() ]
  br label %DIR.OMP.END.PARALLEL

DIR.OMP.END.PARALLEL:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %exit

exit:
  ret void
}
