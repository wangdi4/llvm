; RUN: SATest --vectorizer-type=vpo -BUILD --config=%s.cfg | FileCheck %s

; CHECK: Test program was successfully built.
