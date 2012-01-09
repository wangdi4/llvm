#pragma once

#include "CL/cl.h"

#define KERNELS			"BuildInFunc_test_type.cl"
class Arrangement
{
private:
	cl_kernel			m_kernel;
	cl_program			m_program;
	cl_mem				m_param;
	cl_context			m_context;
	cl_command_queue    m_queue;
	size_t				m_items;

public:
						Arrangement();
						~Arrangement();

	cl_int				Init(const char* test, const char* func, const char* tail);
	cl_int				InvokeKernel();
	cl_int				SetParameter(void* data, size_t size);
	cl_int				GetResults(void* data, size_t size);

	size_t				GetItems();
};
