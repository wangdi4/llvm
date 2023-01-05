; RUN: opt -passes="require<verify>" -S < %s 2>&1 | FileCheck --check-prefix=CHECK1 %s
; RUN: sed -e s/.T2:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK2 %s
; RUN: sed -e s/.T3:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK3 %s
; RUN: sed -e s/.T4:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK4 %s
; RUN: sed -e s/.T5:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK5 %s

; Common declaration used for all runs.
declare i1 @llvm.intel.honor.fcmp.f64(double, double, metadata)

; Test that the verifier accepts legal code
; CHECK1: @f1
define i1 @f1(double %a, double %b) {
entry:
  ; The pairing of fast-math flags and predicates is arbitrary, just testing
  ; them all.
  %t1 = call i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"oeq")
  %t2 = call ninf i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"ogt")
  %t3 = call nsz i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"oge")
  %t4 = call arcp i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"olt")
  %t5 = call contract i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"ole")
  %t6 = call afn i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"one")
  %t7 = call reassoc i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"ord")
  %t8 = call ninf nsz arcp i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"uno")
  %t9 = call afn reassoc i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"ueq")
  %t10 = call ninf nsz arcp afn reassoc i1 @llvm.intel.honor.fcmp.f64(double %a,
                                          double %b,
                                          metadata !"ugt")
  %t11 = call ninf nsz arcp afn reassoc i1 @llvm.intel.honor.fcmp.f64(double %a,
                                          double %b,
                                          metadata !"uge")
  %t12 = call ninf nsz arcp contract afn reassoc i1 @llvm.intel.honor.fcmp.f64(
                                          double %a,
                                          double %b,
                                          metadata !"ult")
  %t13 = call i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"ule")
  %t14 = call i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
                                          metadata !"une")
  ret i1 %t1
}

; Test an illegal value for the predicate argument.
; CHECK2: invalid predicate for intel.honor.fcmp intrinsic
;T2: define i1 @f2(double %a, double %b) {
;T2: entry:
;T2:  %cmp = call i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
;T2:                                                metadata !"neq")
;T2:  ret i1 %cmp
;T2: }

; Test with the 'fast' flag.
; CHECK3: nnan is not permitted on intel.honor.fcmp intrinsic
;T3: define i1 @f3(double %a, double %b) {
;T3: entry:
;T3:  %cmp = call fast i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
;T3:                                                metadata !"one")
;T3:  ret i1 %cmp
;T3: }

; Test with the 'nnan' flag.
; CHECK4: nnan is not permitted on intel.honor.fcmp intrinsic
;T4: define i1 @f4(double %a, double %b) {
;T4: entry:
;T4:  %cmp = call nnan i1 @llvm.intel.honor.fcmp.f64(double %a, double %b,
;T4:                                                metadata !"one")
;T4:  ret i1 %cmp
;T4: }

; Test with all fast-math flags except 'contract'.
; CHECK5: nnan is not permitted on intel.honor.fcmp intrinsic
;T5: define i1 @f5(double %a, double %b) {
;T5: entry:
;T5:  %cmp = call ninf nnan nsz arcp afn i1 @llvm.intel.honor.fcmp.f64(double %a,
;T5:                                                double %b,
;T5:                                                metadata !"one")
;T5:  ret i1 %cmp
;T5: }

