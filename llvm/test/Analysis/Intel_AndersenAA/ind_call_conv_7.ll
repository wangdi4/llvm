; Check that the indirectcallconv pass does NOT replace an indirect call with a
; direct call when the return type of the target function does not match the
; return type of the indirect call.

; RUN: opt < %s -S -disable-verify -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' 2>&1 | FileCheck %s

%"class.std::_Lockit" = type { i32 }
%class.ColumnEditorDlg = type { i64, ptr }
%class.NppParameters = type { ptr }

@"?_pSelf@NppParameters@@0PEAV1@EA" = internal global %class.NppParameters { ptr @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z" }

define internal i64 @"?run_dlgProc@ColumnEditorDlg@@MEAA_JI_K_J@Z"(ptr %in0) {
  %fptr.addr = getelementptr inbounds %class.NppParameters, ptr @"?_pSelf@NppParameters@@0PEAV1@EA", i64 0, i32 0
  %fptr.as.winproc = load ptr, ptr %fptr.addr
  %isnull = icmp eq ptr %fptr.as.winproc, null
  br i1 %isnull, label %done, label %make_icall

make_icall:
  ; Change the function pointer to another type, which has a different return type
  %fptr.as.ETDTProc = bitcast ptr %fptr.as.winproc to ptr
  %lockit.addr = getelementptr inbounds %class.ColumnEditorDlg, ptr %in0, i64 0, i32 1
  %lockit = load ptr, ptr %lockit.addr
  %icall_result = call i32 %fptr.as.ETDTProc(ptr %lockit, i32 6)
  br label %done

done:
  ret i64 0
}

define internal i64 @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z"(ptr %0, i32 %1) {
  ret i64 0
}

; This function is needed to make the Andersen possible target list to be
; marked as 'incomplete' to cause the indirect call conversion to have to
; insert a new call instruction, instead of just replacing the call target.
define void @test(ptr %fptr) {
  %fptr.addr = getelementptr inbounds %class.NppParameters, ptr @"?_pSelf@NppParameters@@0PEAV1@EA", i64 0, i32 0
  store ptr %fptr, ptr %fptr.addr
  ret void
}

define i32 @main() {
  %column.editor = alloca %class.ColumnEditorDlg
  %ret = call i64 @"?run_dlgProc@ColumnEditorDlg@@MEAA_JI_K_J@Z"(ptr  %column.editor)
  ret i32 0
}

; CHECK-NOT: %icall_result.indconv = call i32 @"?OFNHookProc@FileDialog@@KA_KPEAUHWND__@@I_K_J@Z"
; CHECK: %icall_result = call i32 %fptr.as.ETDTProc