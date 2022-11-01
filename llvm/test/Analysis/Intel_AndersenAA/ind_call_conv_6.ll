; Check that the indirectcallconv pass does NOT replace an indirect call with a
; direct call when the number of arguments required by the potential target
; function does not match the number of arguments given at the call site.
; Regression test for CMPLRLLVM-22598.

; RUN: opt < %s -S -disable-verify -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' 2>&1 | FileCheck %s

%"class.std::_Lockit" = type { i32 }
%class.ColumnEditorDlg = type { i64, %"class.std::_Lockit"* }
%class.NppParameters = type { i64 (%"class.std::_Lockit"*, i32, i64, i64)* }

@"?_pSelf@NppParameters@@0PEAV1@EA" = internal global %class.NppParameters { i64 (%"class.std::_Lockit"*, i32, i64, i64)* @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z" }

define internal i64 @"?run_dlgProc@ColumnEditorDlg@@MEAA_JI_K_J@Z"(%class.ColumnEditorDlg* %in0) {
  %fptr.addr = getelementptr inbounds %class.NppParameters, %class.NppParameters* @"?_pSelf@NppParameters@@0PEAV1@EA", i64 0, i32 0
  %fptr.as.winproc = load i64 (%"class.std::_Lockit"*, i32, i64, i64)*, i64 (%"class.std::_Lockit"*, i32, i64, i64)** %fptr.addr
  %isnull = icmp eq i64 (%"class.std::_Lockit"*, i32, i64, i64)* %fptr.as.winproc, null
  br i1 %isnull, label %done, label %make_icall

make_icall:
  ; Change the function pointer to another type, which has a different number of arguments
  %fptr.as.ETDTProc = bitcast i64 (%"class.std::_Lockit"*, i32, i64, i64)* %fptr.as.winproc to i64 (%"class.std::_Lockit"*, i32)*
  %lockit.addr = getelementptr inbounds %class.ColumnEditorDlg, %class.ColumnEditorDlg* %in0, i64 0, i32 1
  %lockit = load %"class.std::_Lockit"*, %"class.std::_Lockit"** %lockit.addr
  %icall_result = call i64 %fptr.as.ETDTProc(%"class.std::_Lockit"* %lockit, i32 6)
  br label %done

done:
  ret i64 0
}

define internal i64 @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z"(%"class.std::_Lockit"* %0, i32 %1, i64 %2, i64 %3) {
  ret i64 0
}

; This function is needed to make the Andersen possible target list to be
; marked as 'incomplete' to cause the indirect call conversion to have to
; insert a new call instruction, instead of just replacing the call target.
define void @test(i64 (%"class.std::_Lockit"*, i32, i64, i64)* %fptr) {
  %fptr.addr = getelementptr inbounds %class.NppParameters, %class.NppParameters* @"?_pSelf@NppParameters@@0PEAV1@EA", i64 0, i32 0
  store i64 (%"class.std::_Lockit"*, i32, i64, i64)* %fptr, i64 (%"class.std::_Lockit"*, i32, i64, i64)** %fptr.addr
  ret void
}

define i32 @main() {
  %column.editor = alloca %class.ColumnEditorDlg
  %ret = call i64 @"?run_dlgProc@ColumnEditorDlg@@MEAA_JI_K_J@Z"(%class.ColumnEditorDlg*  %column.editor)
  ret i32 0
}

; CHECK-NOT: %icall_result.indconv = call i64 @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z"
; CHECK: %icall_result = call i64 %fptr.as.ETDTProc
