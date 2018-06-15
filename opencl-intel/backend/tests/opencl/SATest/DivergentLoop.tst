;; The test checks if there are no heuristic issues during vectorization
;; of a divergent loop

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s

; CHECK: my_cc_cc_analysis
