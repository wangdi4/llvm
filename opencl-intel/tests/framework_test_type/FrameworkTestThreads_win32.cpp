#define NOMINMAX
#include "FrameworkTestThreads.h"
#include <Windows.h>
#include <process.h>

bool Thread::Run(ThreadEntryPoint func) {
  m_handle = (void *)_beginthreadex(
      NULL, 0, (unsigned int(__stdcall *)(void *))func, this, 0, NULL);
  return m_handle != 0;
}
void Thread::WaitForCompletion() {
  WaitForSingleObject(m_handle, INFINITE);
  CloseHandle((HANDLE *)m_handle);
}
