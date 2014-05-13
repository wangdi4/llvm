#include "cmds.h"
#include "common.h"



void commands(cl_context context, vector <cl_device_id> devices)
{
	cl_float*   p_input = NULL;
    cl_float*   p_output = NULL;
	int err = 0;


	vector<vector<cl_event>> end_event;
	vector<vector<cl_event>> start_event;
	end_event.resize(3);
	start_event.resize(3);
    for (int i = 0; i < 3; ++i) {
		end_event[i].resize(2);
		start_event[i].resize(2);
		for(int j=0;j<2;j++) {
			end_event[i][j]=NULL;
			start_event[i][j]=clCreateUserEvent(context, &err);
			checkOpenCLErrors(err);
		}
	}

    // Set kernel arguments
 /*   err = clSetKernelArg(kernel1, 0, sizeof(cl_mem), (void *) &cl_input_buffer);
    checkOpenCLErrors(err);
    err = clSetKernelArg(kernel1, 1, sizeof(cl_mem), (void *) &cl_output_buffer);
    checkOpenCLErrors(err);

	err= clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, (const size_t*)&size_buffer, NULL, 1, &(start_event[0][0]), &(end_event[0][0]));
    checkOpenCLErrors(err);
	err= clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, (const size_t*)&size_buffer, NULL, 1, &(start_event[0][1]), &(end_event[0][1]));
    checkOpenCLErrors(err);
	err= clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, (const size_t*)&size_buffer, NULL, 1, &(start_event[1][0]), &(end_event[1][0]));
    checkOpenCLErrors(err);
	err= clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, (const size_t*)&size_buffer, NULL, 1, &(start_event[1][1]), &(end_event[1][1]));
    checkOpenCLErrors(err);
	delay();
	clSetUserEventStatus(start_event[0][1], CL_COMPLETE);
	delay();
	clSetUserEventStatus(start_event[0][0], CL_COMPLETE);
	delay();
	clSetUserEventStatus(start_event[1][1], CL_COMPLETE);
	delay();
	clSetUserEventStatus(start_event[1][0], CL_COMPLETE);
	delay();
	vector<cl_event> tmp=end_event[0];
	tmp.insert(tmp.end(),end_event[1].begin(),end_event[1].end());
	err = clWaitForEvents(4, &tmp[0]);
	checkOpenCLErrors(err);
    err = clEnqueueReadBuffer(queue1, cl_output_buffer, CL_TRUE, 0, size_array , p_output, 0, NULL, NULL);
    checkOpenCLErrors(err);
	delay();

	err=clFinish(queue1);
	checkOpenCLErrors(err);
	err=clFinish(queue2);
	checkOpenCLErrors(err);
    for (int i = 0; i < 2; i++) {
		for(int j=0;j<2;j++) {
			err = clReleaseEvent(end_event[i][j]);
			checkOpenCLErrors(err);
			err = clReleaseEvent(start_event[i][j]);
			checkOpenCLErrors(err);
		}
	}
	err = clReleaseMemObject(cl_input_buffer);
    checkOpenCLErrors(err);
	err = clReleaseMemObject(cl_output_buffer);
    checkOpenCLErrors(err);
	err = clReleaseKernel(kernel1);
	checkOpenCLErrors(err);
	err = clReleaseProgram(program);
	checkOpenCLErrors(err);
	err = clReleaseCommandQueue(queue1);
	checkOpenCLErrors(err);
	err = clReleaseCommandQueue(queue2);
	checkOpenCLErrors(err);
	err = clReleaseContext(context);
	checkOpenCLErrors(err);*/
}