; RUN: SATest --vectorizer-type=vpo -BUILD --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test program was successfully built.
