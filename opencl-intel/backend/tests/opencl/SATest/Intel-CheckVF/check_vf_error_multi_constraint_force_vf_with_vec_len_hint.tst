; RUN: SATest -BUILD --vectorizer-type=vpo -tsize=4 --config=%s.cfg 2>&1 | FileCheck %s --check-prefix=LOG

; LOG: error: kernel "test": Conflicting CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size are specified
; LOG: CompilerException Checking vectorization factor failed
