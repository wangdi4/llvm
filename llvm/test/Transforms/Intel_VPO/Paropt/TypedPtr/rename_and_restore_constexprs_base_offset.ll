; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -early-cse -instcombine -vpo-restore-operands -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t2.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes="function(early-cse,instcombine,vpo-restore-operands)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t2.ll | FileCheck %s -check-prefix=TFORM

; The test IR was hand-generated to simulate uses of constexprs offsets into an
; array as clause operands. A source program equivalent of the IR
; (not OpenMP spec compliant) would look something like this:
;
; char global[20];
;
; void bar(int in1, int in2);
; void wibble() {
;   #pragma omp parallel private(((char*)&global)[0:4]) private(((int*) &global[4])[0:1])
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
; !$omp parallel private(x) private(y) num_threads(1)
;   print*, loc(x), loc(y) ! p3 p4 (should be different from p1, p2)
; !$omp end parallel
; end

; end INTEL_CUSTOMIZATION
; Check that prepare pass does correct renaming/replacement of the
; clause operands and their uses inside the region.
; PREPR:      store [4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8]** [[ADDR1:%.+]], align 8
; PREPR:      store i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32** [[ADDR2:%.+]], align 8
; PREPR:      "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.PRIVATE"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*))
; PREPR-SAME: "QUAL.OMP.PRIVATE"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*))
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8]** [[ADDR1]])
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32** [[ADDR2]])
; PREPR:      [[OPND1_RENAMED:%.+]] = load volatile [4 x i8]*, [4 x i8]** [[ADDR1]], align 8
; PREPR:      %global = bitcast [4 x i8]* [[OPND1_RENAMED]] to [20 x i8]*
; PREPR:      [[OPND2_RENAMED:%.+]] = load volatile i32*, i32** [[ADDR2]], align 8
; PREPR:      %tmp2 = load i32, i32* [[OPND2_RENAMED]], align 4
; PREPR:      [[GLOBAL_CAST:%.+]] = bitcast [20 x i8]* %global to i32*
; PREPR:      %tmp3 = load i32, i32* [[GLOBAL_CAST]], align 4
; PREPR:      call void @bar(i32 %tmp2, i32 %tmp3)

; Check that after restoring operands, the restored clause operands
; are used inside the region.
; RESTR:      "DIR.OMP.PARALLEL"()
; RESTR-SAME: "QUAL.OMP.PRIVATE"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*)),
; RESTR-SAME: "QUAL.OMP.PRIVATE"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i64 0, i64 4) to i32*))
; RESTR:       %tmp2 = load i32, i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i64 0, i64 4) to i32*), align 4
; RESTR:       [[OPND1_CAST:%.+]] = bitcast [4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*) to i32*
; RESTR:       %tmp3 = load i32, i32* [[OPND1_CAST]], align 4
; RESTR:       call void @bar(i32 %tmp2, i32 %tmp3)

; Check that after outlining, the private copies of the two operands are
; are used in the outlined function, not the original "@global".
; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(i32* %tid, i32* %bid)
; TFORM:      [[OPND2_PRIV:%.+]] = alloca i32, align 4
; TFORM:      [[OPND1_PRIV:%.+]] = alloca [4 x i8], align 1
; TFORM:      %tmp2 = load i32, i32* [[OPND2_PRIV]], align 4
; TFORM:      [[OPND1_PRIV_CAST:%.+]] = bitcast [4 x i8]* [[OPND1_PRIV]] to i32*
; TFORM:      %tmp3 = load i32, i32* [[OPND1_PRIV_CAST]], align 4
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
    "QUAL.OMP.PRIVATE"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*)),
    "QUAL.OMP.PRIVATE"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*)) ]

  %tmp2 = load i32, i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), align 4
  %tmp3 = load i32, i32* bitcast ([20 x i8]* @global to i32*), align 4

  call void @bar(i32 %tmp2, i32 %tmp3)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
