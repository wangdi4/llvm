#include <iostream>
#include <CL/cl.h>
#include <exception>
#include <vector>
#include <cerrno>
#include <string>
#include <algorithm>

#include "common.h"
#include "cmds.h"
#include "images.h"


cl_device_id get_device_by_type(cl_platform_id platform, cl_device_type type){
	cl_int err;
	cl_uint num_of_devices;
	// get total number of available devices of specified type
	err = clGetDeviceIDs(platform, type, 0, 0, &num_of_devices);
	if (err != CL_DEVICE_NOT_FOUND)
		checkOpenCLErrors(err);

	// if there is no device of specified type in the system use CPU
	if (num_of_devices == 0) return get_device_by_type(platform, CL_DEVICE_TYPE_CPU);

	// use vector for automatic memory management
	vector<cl_device_id> devices(num_of_devices);

	err = clGetDeviceIDs(platform, type, num_of_devices, &devices[0], NULL);
	checkOpenCLErrors(err);
	return devices[0];
}

// main execution routine - perform simple math on float vectors
int main(int argc, const char** argv)
{
	int err = 0;
	cl_uint num_of_platforms = 0;
	cl_uint num_of_devices = 3;
	cl_uint num_of_subdevices = 2;
	cl_platform_id platform_id;
	const size_t size = 1024, size_array = size*sizeof(float);
	cl_float *p_buf_in_cpu, *p_buf_out_cpu, *p_buf_in_mic, *p_buf_out_mic;
	cl_image_format imageFormat;
	cl_image_desc image_desc;
	cl_uint width, height;
	imageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
	imageFormat.image_channel_order = CL_RGBA;
	//uchar4 * pixels_;
	cl_uchar4 *in_image, *out_image, *map_image;
	in_image = (cl_uchar4*)read_bmp_image("image.bmp", &width, &height);
	checkHostErrors(!in_image, "Failed to read image from file.");

	//out_image=(cl_uchar4*)malloc(width*height*sizeof(cl_uchar4));
	//checkHostErrors(!out_image, "Failed to allocate output image buffer");

	map_image = (cl_uchar4*)malloc(width*height*sizeof(cl_uchar4));
	checkHostErrors(!map_image, "Failed to allocate output image buffer");

	// initializa the Image data to NULL 
	memset(map_image, 0, width * height * sizeof(cl_uchar4));

	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width = width;
	image_desc.image_height = height;
	image_desc.image_depth = 0;
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = NULL;

	// get total number of available platforms:
	err = clGetPlatformIDs(0, 0, &num_of_platforms);
	checkOpenCLErrors(err);

	// use vector for automatic memory management
	vector<cl_platform_id> platforms(num_of_platforms);
	// get IDs for all platforms:
	err = clGetPlatformIDs(num_of_platforms, &platforms[0], 0);
	checkOpenCLErrors(err);

	for (cl_uint i = 0; i < num_of_platforms; ++i)
	{
		size_t platform_name_length = 0;
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &platform_name_length);
		vector<char> platform_name(platform_name_length);
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_length, &platform_name[0], 0);
		checkOpenCLErrors(err);
		/* We are focussing on Intel platform only*/
		if (string(&platform_name[0]).find("Intel") != string::npos){
			platform_id = platforms[i];
			break;
		}
	}

	vector<cl_context> contexts(num_of_devices);
	vector<cl_device_id> devices(num_of_devices);
	vector<cl_device_id> subdevices(num_of_subdevices);
	devices[CPU] = get_device_by_type(platform_id, CL_DEVICE_TYPE_CPU);
	devices[GPU] = get_device_by_type(platform_id, CL_DEVICE_TYPE_GPU);
	devices[MIC] = get_device_by_type(platform_id, CL_DEVICE_TYPE_ACCELERATOR);
	cl_device_partition_property prop[] = { CL_DEVICE_PARTITION_BY_COUNTS, 1, 2, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0 };

	vector <cl_command_queue> queues(num_of_devices);
	vector <cl_device_id> unique_devices; // CPU+GPU+MIC+2*SUBDEVICE
	err = clCreateSubDevices(devices[CPU], prop, 2, &subdevices[0], NULL);
	checkOpenCLErrors(err);
	cl_uint used_subdevices = 0;
	for (cl_uint i = 0; i< devices.size(); i++){
		/* If GPU or MIC are not available use CPU subdevices instead */
		if (std::find(unique_devices.begin(), unique_devices.end(), devices[i]) != unique_devices.end())
		{
			devices[i] = subdevices[used_subdevices++];
		}
		unique_devices.push_back(devices[i]);
	}
	for (cl_uint i = used_subdevices; i< subdevices.size(); i++)unique_devices.push_back(subdevices[i]);


	checkOpenCLErrors(err);
	for (cl_uint i = 0; i < num_of_devices; ++i)
	{
		contexts[i] = clCreateContext(0, 1, &devices[i], NULL, NULL, &err);
		if (i == CPU){
			queues[i] = clCreateCommandQueue(contexts[i], devices[i], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
			checkOpenCLErrors(err);
		}
		else{
			queues[i] = clCreateCommandQueue(contexts[i], devices[i], 0, &err);
			checkOpenCLErrors(err);
		}
	}

	// Create sampler
	cl_sampler kernelSampler = clCreateSampler(contexts[CPU], true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &err);
	checkOpenCLErrors(err);

	err = clReleaseSampler(kernelSampler);
	checkOpenCLErrors(err);

	vector<cl_kernel> kernels(num_of_devices);
	vector<cl_program> programs(num_of_devices);
	kernels[CPU] = create_kernel("queues.cl", "QueueKernel", contexts[CPU], devices[CPU], &programs[CPU]);
	kernels[GPU] = create_kernel("images.cl", "OverLap", contexts[GPU], devices[GPU], &programs[GPU]);
	kernels[MIC] = create_kernel("queues.cl", "QueueKernel", contexts[MIC], devices[MIC], &programs[MIC]);

	/*Allocate/initialize all necessary buffers/images */
	cl_mem input_buffer_cpu = create_float_buffer(contexts[CPU], devices[CPU], size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &p_buf_in_cpu);
	cl_mem output_buffer_cpu = create_float_buffer(contexts[CPU], devices[CPU], size, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, &p_buf_out_cpu);
	cl_mem input_buffer_mic = create_float_buffer(contexts[MIC], devices[MIC], size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &p_buf_in_mic);
	cl_mem output_buffer_mic = create_float_buffer(contexts[MIC], devices[MIC], size, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, &p_buf_out_mic);
	cl_mem cpuImage = clCreateImage(contexts[CPU], CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &image_desc, in_image, &err);
	checkOpenCLErrors(err);
	cl_mem inImage = clCreateImage(contexts[GPU], CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &image_desc, in_image, &err);
	checkOpenCLErrors(err);
	cl_mem fillImage = clCreateImage(contexts[GPU], CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &image_desc, map_image, &err);
	checkOpenCLErrors(err);
	cl_mem outImage = clCreateImage(contexts[GPU], CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, &imageFormat, &image_desc, NULL, &err);
	checkOpenCLErrors(err);
	cl_buffer_region reg = { 0, 64 };

	cl_mem subBuffer = clCreateSubBuffer(input_buffer_cpu, CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &reg, &err);
	checkOpenCLErrors(err);

	err = clSetKernelArg(kernels[CPU], 0, sizeof(cl_mem), (void *)&input_buffer_cpu);
	checkOpenCLErrors(err);
	err = clSetKernelArg(kernels[CPU], 1, sizeof(cl_mem), (void *)&output_buffer_cpu);
	checkOpenCLErrors(err);
	err = clSetKernelArg(kernels[MIC], 0, sizeof(cl_mem), (void *)&input_buffer_mic);
	checkOpenCLErrors(err);
	err = clSetKernelArg(kernels[MIC], 1, sizeof(cl_mem), (void *)&output_buffer_mic);
	checkOpenCLErrors(err);

	err = clSetKernelArg(kernels[GPU], 0, sizeof(cl_mem), &inImage);
	checkOpenCLErrors(err);
	err = clSetKernelArg(kernels[GPU], 1, sizeof(cl_mem), &fillImage);
	checkOpenCLErrors(err);
	err = clSetKernelArg(kernels[GPU], 2, sizeof(cl_mem), &outImage);
	checkOpenCLErrors(err);

	vector<vector<cl_event>> end_event;
	vector<vector<cl_event>> start_event;
	end_event.resize(num_of_devices);
	start_event.resize(num_of_devices);
	for (cl_uint i = 0; i < num_of_devices; ++i) {
		end_event[i].resize(3);
		start_event[i].resize(3);
		for (cl_uint j = 0; j<start_event.size(); j++) {
			end_event[i][j] = NULL;
			start_event[i][j] = clCreateUserEvent(contexts[i], &err);
			checkOpenCLErrors(err);
		}
	}

	/*Enqueue 4 buffer kernels for buffers and 1 for image operations*/
	err = clEnqueueNDRangeKernel(queues[CPU], kernels[CPU], 1, NULL, (const size_t*)&size, NULL, 1, &(start_event[CPU][0]), &(end_event[CPU][0]));
	checkOpenCLErrors(err);
	err = clEnqueueNDRangeKernel(queues[CPU], kernels[CPU], 1, NULL, (const size_t*)&size, NULL, 1, &(start_event[CPU][1]), &(end_event[CPU][1]));
	checkOpenCLErrors(err);
	err = clEnqueueNDRangeKernel(queues[MIC], kernels[MIC], 1, NULL, (const size_t*)&size, NULL, 1, &(start_event[MIC][0]), &(end_event[MIC][0]));
	checkOpenCLErrors(err);
	err = clEnqueueNDRangeKernel(queues[MIC], kernels[MIC], 1, NULL, (const size_t*)&size, NULL, 1, &(start_event[MIC][1]), &(end_event[MIC][1]));
	checkOpenCLErrors(err);

	int color[4] = { 0, 0, 80, 255 };
	size_t origin[3] = { 300, 300, 0 };
	size_t region[3] = { 100, 100, 1 };
	err = clEnqueueFillImage(queues[CPU], cpuImage, color, origin, region, 1, &(start_event[CPU][2]), &(end_event[CPU][2]));
	checkOpenCLErrors(err);
	err = clEnqueueFillImage(queues[GPU], inImage, color, origin, region, 0, NULL, NULL);
	checkOpenCLErrors(err);
	color[0] = 80; color[1] = 0; color[2] = 0; color[3] = 0;
	origin[0] = 50;	origin[1] = 50;
	err = clEnqueueFillImage(queues[GPU], fillImage, color, origin, region, 1, &(start_event[GPU][0]), &(end_event[GPU][0]));
	checkOpenCLErrors(err);

	size_t globalThreads[] = { width, height };
	size_t localThreads[] = { 256, 1 };
	err = clEnqueueNDRangeKernel(queues[GPU], kernels[GPU], 2, NULL, globalThreads, localThreads, 1, &start_event[GPU][1], &(end_event[GPU][1]));

	origin[0] = 0; origin[1] = 0; origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;
	size_t  rowPitch;
	size_t  slicePitch;
	out_image = (cl_uchar4*)clEnqueueMapImage(queues[GPU], outImage, CL_FALSE, CL_MAP_READ | CL_MAP_WRITE, origin, region, &rowPitch, &slicePitch, 1, &start_event[GPU][2], &(end_event[GPU][2]), &err);
	checkOpenCLErrors(err);
	delay();

	/* Enqueue reading buffer for first kernel when available */
	err = clEnqueueReadBuffer(queues[CPU], output_buffer_cpu, CL_FALSE, 0, size_array, p_buf_out_cpu, 1, &end_event[CPU][0], NULL);
	checkOpenCLErrors(err);
	delay();

	/* Let image operation for CPU to be executed
	* it is expected that it will be executed imediatelly (OOO queue)
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[CPU][2], CL_COMPLETE));
	delay();

	/* Let second kernel for CPU to be executed
	* it is expected that it will be executed imediatelly (OOO queue)
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[CPU][1], CL_COMPLETE));
	delay();
	delay();
	/* Let first kernel for CPU to be executed
	*  It is expected that Read buffer will be also executed here
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[CPU][0], CL_COMPLETE));
	delay();
	delay();
	/* Let second kernel for MIC to be executed
	* it is expected that it will wait untill first kernel finishes (In order queue)
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[MIC][1], CL_COMPLETE));
	delay();

	/* Let first kernel for MIC to be executed
	*  Both kernels for this queue are executed (In order queue)
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[MIC][0], CL_COMPLETE));
	delay();
	delay();
	delay();
	/* Let image to be mapped back to host
	* it should be executed after kernel is executed (In order queue)*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[GPU][2], CL_COMPLETE));
	delay();

	/* Let second image fill to be executed
	* it is expected that it will wait untill first kernel finishes (In order queue)
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[GPU][1], CL_COMPLETE));
	delay();

	/* Kernel for image to be executed
	* it should be executed emmidiatelly
	*/
	checkOpenCLErrors(clSetUserEventStatus(start_event[GPU][0], CL_COMPLETE));
	delay();
	delay();
	/* wait untill image events complete
	*/
	err = clWaitForEvents(3, &end_event[GPU][0]);
	checkOpenCLErrors(err);
	delay();

	write_bmp_image("image_res.bmp", out_image);

	/* The we will wait until buffer is read
	*/
	err = clEnqueueReadBuffer(queues[MIC], output_buffer_mic, CL_TRUE, 0, size_array, p_buf_out_mic, 2, &end_event[MIC][0], NULL);
	checkOpenCLErrors(err);
	delay();

	/* wait untill all events complete
	*/
	err = clWaitForEvents(0, NULL);
	for (cl_uint i = 0; i < num_of_devices; i++) {
		for (cl_uint j = 0; j<end_event[i].size(); j++) {
			if (end_event[i][j] != NULL){
				err = clReleaseEvent(end_event[i][j]);
				checkOpenCLErrors(err);
			}
			err = clReleaseEvent(start_event[i][j]);
			checkOpenCLErrors(err);
		}
	}
	for (int i = 0; i <kernels.size(); i++)
		checkOpenCLErrors(clReleaseKernel(kernels[i]))

	for (int i = 0; i <programs.size(); i++)
		checkOpenCLErrors(clReleaseProgram(programs[i]))

	for (cl_uint i = 0; i < num_of_devices; ++i)
	{

		err = clFinish(queues[i]);
		checkOpenCLErrors(err);
		err = clReleaseCommandQueue(queues[i]);
		checkOpenCLErrors(err);
	}
	err = clReleaseMemObject(input_buffer_cpu);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(input_buffer_mic);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(output_buffer_cpu);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(output_buffer_mic);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(cpuImage);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(inImage);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(fillImage);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(outImage);
	checkOpenCLErrors(err);
	err = clReleaseMemObject(subBuffer);
	checkOpenCLErrors(err);

	for (cl_uint i = 0; i < num_of_devices; ++i)
	{
		err = clReleaseContext(contexts[i]);
		checkOpenCLErrors(err);
	}
	for (cl_uint i = 0; i < unique_devices.size(); i++)
	{
		err = clReleaseDevice(unique_devices[i]);
		checkOpenCLErrors(err);
	}
	return err;
}
