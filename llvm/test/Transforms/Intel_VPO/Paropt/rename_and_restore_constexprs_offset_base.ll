; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -early-cse -instcombine -vpo-restore-operands -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=RESTR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t2.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -passes="function(early-cse,instcombine,vpo-restore-operands)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=RESTR
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t2.ll | FileCheck %s -check-prefix=TFORM

; This is a version of rename_and_restore_constexprs_base_offset.ll, with the order of
; the clauses reversed.

; A source program equivalent of the IR (not OpenMP spec compliant) would look
; something like this:
;
; char global[20];
;
; void bar(int in1, int in2);
; void wibble() {
;   #pragma omp parallel private(((int*) &global[4])[0:1]) private(((char*)&global)[0:4])
;   bar(*((int*) &global[4]), *((int*)&global));
; }

; INTEL_CUSTOMIZATION
; A Fortran example that would result in similar IR is:
;
; integer x, y
; common /global/ x, y
;
; print*, loc(x), loc(y)   ! p1 p2
;
; !$omp parallel private(y) private(x) num_threads(1)
;   print*, loc(x), loc(y) ! p3 p4 (should be different from p1, p2)
; !$omp end parallel
; end

; end INTEL_CUSTOMIZATION
; Since the first clause is handled before the second one, we are able to handle this case
; where we couldn't handle the case in rename_and_restore_constexprs_base_offset.ll.

; This is still not reliable, because there is no guarantee that clauses will be handled
; in the order they are specified. And the clauses can be different
; (e.g. private and firstprivate), and Paropt handles all clauses of the same type
; together.

; Check that prepare pass does correct renaming/replacement of the
; clause operands and their uses inside the region.
; PREPR:      store ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), ptr [[ADDR1:.+]], align 8
; PREPR:      store ptr @global, ptr [[ADDR2:.+]], align 8
; PREPR:      "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), i32 0, i32 1)
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1)
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), ptr [[ADDR1]])
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr [[ADDR2]])
; PREPR:      [[OPND1_RENAMED:%.+]] = load volatile ptr, ptr [[ADDR1]], align 8
; PREPR:      [[OPND2_RENAMED:%.+]] = load volatile ptr, ptr [[ADDR2]], align 8
; PREPR:      %tmp2 = load i32, ptr [[OPND1_RENAMED]], align 4
; PREPR:      %tmp3 = load i32, ptr [[OPND2_RENAMED]], align 4
; PREPR:      call void @bar(i32 %tmp2, i32 %tmp3)

; Check that after restoring operands, the restored clause operands
; are used inside the region.

; TODO: vpo-restore-operands would need to be updated to not propagate back these const-exprs into the region
; so that rename_and_restore_constexprs_base_offset.ll can work after the FE change.

; RESTR:      "DIR.OMP.PARALLEL"()
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), i32 0, i32 1)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1)
; RESTR:       %tmp2 = load i32, ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), align 4
; RESTR:       %tmp3 = load i32, ptr @global, align 4
; RESTR:       call void @bar(i32 %tmp2, i32 %tmp3)

; Check that after outlining, the private copies of the two operands are
; are used in the outlined function, not the original "@global".
; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid)
; TFORM:      [[OPND2_PRIV:%.+]] = alloca [4 x i8], align 4
; TFORM:      [[OPND1_PRIV:%.+]] = alloca i32, align 4
; TFORM:      %tmp2 = load i32, ptr [[OPND1_PRIV]], align 4
; TFORM:      %tmp3 = load i32, ptr [[OPND2_PRIV]], align 4
; TFORM:      call void @bar(i32 %tmp2, i32 %tmp3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(i32, i32)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1) ]

  %tmp2 = load i32, ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), align 4
  %tmp3 = load i32, ptr @global, align 4

  call void @bar(i32 %tmp2, i32 %tmp3)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
