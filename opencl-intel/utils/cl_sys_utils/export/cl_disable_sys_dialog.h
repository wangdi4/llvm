// INTEL CONFIDENTIAL
//
// Copyright 2021 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef UTILS_SYS_DISABLE_SYS_DIALOG
#define UTILS_SYS_DISABLE_SYS_DIALOG

#ifdef _WIN32
#include <Windows.h>
#include <crtdbg.h>

namespace Intel {
namespace OpenCL {
namespace Utils {

#ifdef _MSC_VER
static int AvoidMessageBoxHook(int ReportType, char *Message, int *Return) {
  // Set *Return to the retry code for the return value of _CrtDbgReport:
  // http://msdn.microsoft.com/en-us/library/8hyw4sy7(v=vs.71).aspx
  // This may also trigger just-in-time debugging via DebugBreak().
  if (Return)
    *Return = 1;
  // Don't call _CrtDbgReport.
  return 1;
}
#endif

/// Disable system dialogs on crash on windows.
/// TODO use llvm sys::DisableSystemDialogsOnCrash when all dlls are linked with
/// llvm libraries.
static void DisableSystemDialogsOnCrash() {
#ifdef _MSC_VER
  // Helpful text message is printed when a program is abnormally terminated
  _set_abort_behavior(0, _WRITE_ABORT_MSG);
  // Disable Dr. Watson.
  _set_abort_behavior(0, _CALL_REPORTFAULT);
  _CrtSetReportHook(AvoidMessageBoxHook);
#endif

  // Disable standard error dialog box.
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
               SEM_NOOPENFILEERRORBOX);
  _set_error_mode(_OUT_TO_STDERR);
}

} // namespace Utils
} // namespace OpenCL
} // namespace Intel

#endif

#endif // UTILS_SYS_DISABLE_SYS_DIALOG
