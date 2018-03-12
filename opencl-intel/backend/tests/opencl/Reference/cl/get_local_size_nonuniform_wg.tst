; RUN: SATest -REF -config=%s.cfg -neat=1
; RUN: FileCheck %s <%s.ref
; CHECK: <Vector id="0">2 1 2 0 </Vector>
; CHECK: <Vector id="1">2 1 2 1 </Vector>
; CHECK: <Vector id="2">1 1 2 2 </Vector>
; CHECK: <Vector id="3">2 1 2 3 </Vector>
; CHECK: <Vector id="4">2 1 2 4 </Vector>
; CHECK: <Vector id="5">1 1 2 5 </Vector>
; CHECK: <Vector id="6">2 1 1 6 </Vector>
; CHECK: <Vector id="7">2 1 1 7 </Vector>
; CHECK: <Vector id="8">1 1 1 8 </Vector>
