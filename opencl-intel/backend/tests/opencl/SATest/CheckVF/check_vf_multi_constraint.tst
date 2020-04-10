; RUN: SATest -BUILD --vectorizer-type=vpo -tsize=4 --config=%s.cfg |& FileCheck %s --check-prefix=LOG
; RUN: SATest -BUILD --vectorizer-type=vpo --config=%s.cfg |& FileCheck %s --check-prefix=LOG_1
; RUN: SATest -BUILD --vectorizer-type=vpo -tsize=4 --config=%s.cfg |& FileCheck %s --check-prefix=LOG_2

; LOG: Error in kernel <test> Only allow specifying one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size
; LOG: CompilerException Checking vectorization factor failed
; LOG_1: Error in kernel <test_1> Only allow specifying one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size
; LOG_1: CompilerException Checking vectorization factor failed
; LOG_2: Error in kernel <test_2> Only allow specifying one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size
; LOG_2: CompilerException Checking vectorization factor failed
