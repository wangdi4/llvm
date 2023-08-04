; RUN: SATest -BUILD --vectorizer-type=vpo --config=%s.cfg 2>&1 | FileCheck %s --check-prefix=LOG
; LOG: error: kernel "test": Unimplemented function(s): _Z14work_group_alli with vectorization width 128
; LOG-SAME: _Z13sub_group_alli with vectorization width 128
; LOG: CompilerException Checking vectorization factor failed
