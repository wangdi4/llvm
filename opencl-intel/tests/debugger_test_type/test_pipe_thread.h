#ifndef TEST_PIPE_UTILS_H
#define TEST_PIPE_UTILS_H

#ifdef _WIN32
#include "cl_thread.h"
#include <cstdlib>

using namespace std;
using Intel::OpenCL::Utils::OclThread;

class NamedPipeThread: public OclThread
{
public:
    HANDLE pipe;
    char* portNumber;

    NamedPipeThread();

protected:
    virtual RETURN_TYPE_ENTRY_POINT Run();
};
#endif // _WIN32

#endif // TEST_PIPE_UTILS_H


