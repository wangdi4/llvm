; RUN: SATest -BUILD --vectorizer-type=vpo --config=%s.cfg |& FileCheck %s --check-prefix=LOG

; LOG: error: kernel "test": Only one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size can be specified
; LOG: CompilerException Checking vectorization factor failed
