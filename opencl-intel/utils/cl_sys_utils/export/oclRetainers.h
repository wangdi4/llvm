#pragma once

#include "CL/cl.h"
#include "oclInternalFunctions.h"

// Provides smart wrapper for retaining and releasing OCL objects.
// Retains the given object on construction, and 
// releases it on destruction.
class OclRetainer
{
public:
    virtual void retain() = 0;
    virtual void release() = 0;
    virtual bool hasRetained() const
    {
        return m_retained;
    };
    virtual ~OclRetainer() { };
protected:
    bool m_retained;
};

class OclKernelRetainer : public OclRetainer
{
public:
    OclKernelRetainer(cl_kernel kernel);
    ~OclKernelRetainer();
    virtual void retain();
    virtual void release();

private:
    const cl_kernel m_kernel;
};

class OclMemObjRetainer : public OclRetainer
{
public:
    OclMemObjRetainer(cl_mem memobj);
    ~OclMemObjRetainer();
    virtual void retain();
    virtual void release();

private:
    const cl_mem m_memobj;
};

OclRetainer* getRetainer(cl_mem memobj);
OclRetainer* getRetainer(cl_kernel kernel);