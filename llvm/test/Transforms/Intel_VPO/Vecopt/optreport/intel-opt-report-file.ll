; Test to check that OptReportEmitter output is emitted to the proper output
; stream/file when the -intel-opt-report-file option is used.

; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=high -intel-ir-optreport-emitter -intel-opt-report-file=stdout < %s -disable-output | FileCheck %s --strict-whitespace -check-prefixes=LLVM
; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=high -intel-ir-optreport-emitter -intel-opt-report-file=stderr < %s -disable-output 2>&1 >%tout | FileCheck %s --strict-whitespace -check-prefixes=LLVM
; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=high -intel-ir-optreport-emitter -intel-opt-report-file=%t < %s -disable-output
; RUN: FileCheck %s --strict-whitespace -check-prefixes=LLVM < %t

; Test that output is written to stderr if the file given is invalid. "%S",
; which expands to the directory containing this test, is used here as an
; invalid output file path.
; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=high -intel-ir-optreport-emitter -intel-opt-report-file=%S < %s -disable-output 2>%terr | FileCheck %s --strict-whitespace -check-prefixes=LLVM
; RUN: FileCheck %s -check-prefixes=FILE-ERROR < %terr
; FILE-ERROR: warning #13022: could not open file '{{.*}}' for optimization report output, reverting to stdout

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_nonvls_mem(i64* %ptr, i64 *%ptr2) #0 {
; LLVM-LABEL:  Global optimization report for : test_nonvls_mem
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM:           remark #15447: --- begin vector loop memory reference summary ---
; LLVM-NEXT:      remark #15450: unmasked unaligned unit stride loads: 1
; LLVM-NEXT:      remark #15451: unmasked unaligned unit stride stores: 1
; LLVM-NEXT:      remark #15456: masked unaligned unit stride loads: 1
; LLVM-NEXT:      remark #15457: masked unaligned unit stride stores: 1
; LLVM-NEXT:      remark #15458: masked indexed (or gather) loads: 1
; LLVM-NEXT:      remark #15459: masked indexed (or scatter) stores: 1
; LLVM-NEXT:      remark #15462: unmasked indexed (or gather) loads: 1
; LLVM-NEXT:      remark #15463: unmasked indexed (or scatter) stores: 1
; LLVM-NEXT:      remark #15567: Gathers are generated due to non-unit stride index of the corresponding loads.
; LLVM-NEXT:      remark #15568: Scatters are generated due to non-unit stride index of the corresponding stores.
; LLVM:           remark #15474: --- end vector loop memory reference summary ---
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %non.linear = mul nsw nuw i64 %iv, %iv
  %neg = sub nsw nuw i64 0, %iv
  %linear.gep = getelementptr i64, i64 *%ptr, i64 %iv
  %reverse.linear.gep = getelementptr i64, i64 *%ptr, i64 %neg
  %nonlinear.gep = getelementptr i64, i64 *%ptr2, i64 %non.linear

  %unmasked.unit.load = load i64, i64 *%linear.gep
  store i64 42, i64 *%linear.gep ; unmasked.unit.store
  %cond = icmp eq i64 %unmasked.unit.load, 76

  %unmasked.gather = load i64, i64 *%nonlinear.gep
  store i64 42, i64 *%nonlinear.gep ; unmasked.scatter

  br i1 %cond, label %if, label %latch

if:
  %masked.reverse.unit.load = load i64, i64 *%reverse.linear.gep
  store i64 42, i64 *%reverse.linear.gep ; masked.reverse.unit.store

  %masked.gather = load i64, i64 *%nonlinear.gep
  store i64 77, i64 *%nonlinear.gep ; masked.scatter
  br label %latch

latch:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind readnone "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
