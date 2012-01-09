#include <pthread.h>
#include "FrameworkTestThreads.h"

bool Thread::Run(ThreadEntryPoint func)
{
    int err = pthread_create(&m_pthread, NULL, func, this);
	return err == 0;
}
void Thread::WaitForCompletion()
{
	pthread_join(m_pthread, NULL);
}
