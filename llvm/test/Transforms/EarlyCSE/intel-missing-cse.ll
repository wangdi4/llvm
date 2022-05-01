; RUN: opt < %s -S -aa-pipeline=basic-aa,tbaa -passes='require<memoryssa>,early-cse<memssa>' | FileCheck %s

; MemSSA was not generating "optimized" form, which prevented the last
; subscript fetch.2505 from being CSEd with the earlier similar ones.

; CHECK-LABEL: jacld_
; CHECK-NOT: fetch.2505

%"QNCA_a0" = type { double*, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }

@appludata_mp_u_ = external global %"QNCA_a0"

define void @jacld_(double* %"fetch.2051[][]", double* %"fetch.2051[][][]", double* %"fetch.2051[][][][]") {
omp.pdo.body7.lr.ph:
  store i64 0, i64* undef, align 4
  %fetch.133 = load double*, double** getelementptr inbounds (%"QNCA_a0", %"QNCA_a0"* @appludata_mp_u_, i64 0, i32 0), align 8
  %"fetch.133[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) %fetch.133, i64 0)
  %"fetch.2051[][]1" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 undef, double* elementtype(double) undef, i64 0)
  %"fetch.2051[][][]2" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) %"fetch.2051[][]", i64 0)
  %"fetch.2051[][][][]3" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) %"fetch.2051[][]", i64 0)
  store double 0.000000e+00, double* %"fetch.2051[][]", align 1, !tbaa !0
  %"val$[]_fetch.2070" = load i64, i64* undef, align 1
  %"fetch.2068[][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 %"val$[]_fetch.2070", double* elementtype(double) undef, i64 0)
  %"fetch.2068[][][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) undef, i64 0)
  store double 0.000000e+00, double* %"fetch.2068[][][]", align 1
  %fetch.2221 = load double*, double** getelementptr inbounds (%"QNCA_a0", %"QNCA_a0"* @appludata_mp_u_, i64 0, i32 0), align 8, !tbaa !5
  %"val$[]_fetch.2229" = load i64, i64* null, align 1
  %"val$[]_fetch.2230" = load i64, i64* null, align 1
  %int_sext1748 = sext i32 0 to i64
  %"fetch.2221[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 0, double* elementtype(double) %fetch.2221, i64 0)
  %"fetch.2221[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) %"fetch.2221[]", i64 0)
  %fetch.2505 = load double*, double** getelementptr inbounds (%"QNCA_a0", %"QNCA_a0"* @appludata_mp_u_, i64 0, i32 0), align 8
  %"val$[]_fetch.2513" = load i64, i64* null, align 1
  %"val$[]_fetch.2514" = load i64, i64* null, align 1
  %int_sext1975 = sext i32 0 to i64
  %"fetch.2505[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 0, double* elementtype(double) %fetch.2505, i64 0)
  %"fetch.2505[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 0, double* elementtype(double) %"fetch.2505[]", i64 0)
  unreachable
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #1 = { nounwind readnone speculatable }

!0 = !{!1, !2, i64 0}
!1 = !{!"ifx$descr$6", !2, i64 0, !2, i64 8, !2, i64 16, !2, i64 24, !2, i64 32, !2, i64 40, !2, i64 48, !2, i64 56, !2, i64 64, !2, i64 72, !2, i64 80, !2, i64 88, !2, i64 96, !2, i64 104, !2, i64 112, !2, i64 120, !2, i64 128, !2, i64 136}
!2 = !{!"ifx$descr$field", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$jacld_"}
!5 = !{!6, !2, i64 0}
!6 = !{!"ifx$descr$3", !2, i64 0, !2, i64 8, !2, i64 16, !2, i64 24, !2, i64 32, !2, i64 40, !2, i64 48, !2, i64 56, !2, i64 64, !2, i64 72, !2, i64 80, !2, i64 88, !2, i64 96, !2, i64 104, !2, i64 112, !2, i64 120, !2, i64 128, !2, i64 136}
