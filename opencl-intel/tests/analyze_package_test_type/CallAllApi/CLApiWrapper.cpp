/*****************************************************************************
 * Copyright (c) 2013-2014 Intel Corporation
 * All rights reserved.
 *
 * WARRANTY DISCLAIMER
 *
 * THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
 * MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Intel Corporation is the author of the Materials, and requests that all
 * problem reports or change requests be submitted to it directly
 *****************************************************************************/

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <windows.h>
#include "CL\cl.h"
#include "CL\cl_ext.h"
#include "utils.h"
#include <assert.h>
#include <string>
#include "CLApiWrapper.h"


ocl_wrap_data::ocl_wrap_data()
{
    apiCallsMap = new map<string, list<int>, mapComparer>;
    kernelLaunchList = new list<string>;
    memCommandsMap = new map<int, list<string> >;

    commandQueuesOrder = new list<cl_command_queue>;
    contextsOrder = new list<cl_context>;

    // generate two dir up from the current dir (for test with debug files output)
    apiCallsOut.open("ApiCalls_output.txt");
    kernelLaunchOut.open("kernelLaunch_output.txt");
    memCommandsOut.open("memoryCommands_output.txt");

}

ocl_wrap_data::~ocl_wrap_data()
{
    delete apiCallsMap;
    apiCallsMap = NULL;

    delete memCommandsMap;
    memCommandsMap = NULL;

    delete kernelLaunchList;
    kernelLaunchList = NULL;

    delete commandQueuesOrder;
    commandQueuesOrder = NULL;

    delete contextsOrder;
    contextsOrder = NULL;

    apiCallsOut.close();
    kernelLaunchOut.close();
    memCommandsOut.close();
}

int getCommandqueueId(cl_command_queue command_queue, ocl_wrap_data* wrap_data)
{
    int queueId = 1;
    list<cl_command_queue>::iterator it ;
    for(it = wrap_data->commandQueuesOrder->begin(); it != wrap_data->commandQueuesOrder->end(); it++)
    {
        if(*it == command_queue)
        {
            break;
        }
        queueId++;
    }

    return queueId;
}

int getContextId(cl_context context, ocl_wrap_data* wrap_data)
{
    int contextId = 1;
    list<cl_context>::iterator it ;
    for(it = wrap_data->contextsOrder->begin(); it != wrap_data->contextsOrder->end(); it++)
    {
        if(*it == context)
        {
            break;
        }
        contextId++;
    }

    return contextId;
}


void printDimSizeToSS(stringstream* ss, const size_t* size, size_t work_dim, ocl_wrap_data* wrap_data)
{
    if(size)
    {
        switch (work_dim)
        {
        case 1: *ss << "(" <<size[0] << ")"; break;

        case 2:    *ss << "(" <<size[0] << "," << size[1] << ")"; break;

        case 3:    *ss << "(" <<size[0] << "," << size[1] << "," << size[2] << ")"; break;

        default:
            LogError("WTF!! work_dim is zerp or greater than 3\n");
            break;
        }
    }
    *ss << ", ";
}

// this function updates the map according to the function called, this function called from all API wrappers.
void updateApiMap(string funcName, int err_code, ocl_wrap_data* wrap_data)
{

    // check if it first time we call this function
    map< string, list<int>, mapComparer >::iterator it =  wrap_data->apiCallsMap->find(funcName);
    if( it == wrap_data->apiCallsMap->end() )
    {
        list<int>* errList = new list<int>;
        errList->push_back(err_code);
        wrap_data->apiCallsMap->insert(pair<string, list<int> >(funcName, *errList ) );
        delete errList;
    }
    else
    {
        (*wrap_data->apiCallsMap)[funcName].push_back(err_code);
    }
}

// this function updates the data structure that saves the kernelLaunch output, this function called from clEnqueueNDRangeKernel & clEnqueueTask
void updateKernelLaunch(string funcName, cl_command_queue command_queue,cl_kernel kernel, cl_uint work_dim,  const size_t*  global_work_offset, const size_t* global_work_size, const size_t* local_work_size, cl_int ret_val, ocl_wrap_data* wrap_data)
{
    cl_int err;
    char kernelName[64];
    stringstream ss;
    string outLine;

    size_t kernelNameSize;

    err = GetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME, 0, NULL, &kernelNameSize, wrap_data);
    if( err != CL_SUCCESS)
    {
        LogError("Erro: cannot get kernel name size!\n");
        return;
    }

    err = GetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME, kernelNameSize, kernelName, NULL, wrap_data);
    if( err != CL_SUCCESS)
    {
        LogError("Erro: cannot get kernel name!\n");
        return;
    }
    ss << kernelName << ", ";


    printDimSizeToSS(&ss, global_work_size, work_dim, wrap_data);

    printDimSizeToSS(&ss, local_work_size, work_dim, wrap_data);



    if(global_work_offset)
    {
        printDimSizeToSS(&ss, global_work_offset, work_dim, wrap_data);
    }
    else
    {
        if( funcName != "clEnqueueTask")
        {
            size_t zeroSize[3] = {0, 0, 0};
            printDimSizeToSS(&ss, zeroSize, work_dim, wrap_data);
        }
        else //if "clEnqueueTask" so no need for printing offset
        {
            ss << ", ";
        }
    }

    int queueId = getCommandqueueId(command_queue, wrap_data);

    ss << "[" << queueId << "]" << ", ";

    if(funcName == "clEnqueueNDRangeKernel")
    {
        ss << "CL_COMMAND_NDRANGE_KERNEL" <<", ";
    }
    else if (funcName == "clEnqueueTask")
    {
        ss << "CL_COMMAND_TASK" <<", ";
    }
    else
    {
        LogError("shouldn't reach here!");
    }

    ss << ret_val << endl;

    outLine = ss.str();
    //if we want to save the order we should do push_back!, i change it to pass validation vs. Analyze sys output file
    wrap_data->kernelLaunchList->push_front(outLine);
}

// this function updates the data structure that saves Memory Commands output, this function called from all the memory commands API's
void updateMemCommands(string commandName, cl_command_queue command_queue, cl_mem memObj, const size_t* size, bool isRectSize, cl_int ret_val, ocl_wrap_data* wrap_data)
{
    bool isBufferCommand = true;
    stringstream ss;
    string outLine;
    string channelOrder, channelDataType;

    cl_int err = CL_SUCCESS;

    ss << commandName;
    ss << ", " << ret_val;
    if(!isRectSize)
    {
        ss << ", " << *size;
    }
    else
    {
        size_t totalSize = size[0] * size[1] * size[2];
        ss << ", " << totalSize;
    }

    cl_mem_object_type type;
    err = GetMemObjectInfo(memObj, CL_MEM_TYPE, sizeof(cl_mem_object_type), &type, NULL, wrap_data);
    if(err != CL_SUCCESS)
    {
        LogError("Erro: cannot get memory object type!\n");
        return;
    }
    if(type != CL_MEM_OBJECT_BUFFER)
    {
        isBufferCommand = false;
    }

    // it shouldn't be not bufferCommand and not rectSize (not region) at the same time.
    assert( !(!isRectSize && !isBufferCommand) );

    string memObjType;
    memObjType = memTypeToString(type);
    ss << ", " << memObjType;

    cl_mem_flags flags;
    err = GetMemObjectInfo(memObj, CL_MEM_FLAGS, sizeof(cl_mem_flags), &flags, NULL, wrap_data);
    if(err != CL_SUCCESS)
    {
        LogError("Erro: cannot get memory object flags!\n");
        return;
    }
    string memFlags;
    memFlags = memFlagsToString(flags);
    ss << "," << memFlags;

    cl_context context;
    err = GetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetCommandQueueInfo returned %d\n");
        return;
    }

    int contextId = getContextId(context, wrap_data);
    ss << ", " << "[" <<contextId << "]";

    int queueId = getCommandqueueId(command_queue, wrap_data);
    ss << ", " << "[" << queueId << "]";

    cl_uint mapCount;
    err = GetMemObjectInfo(memObj, CL_MEM_MAP_COUNT, sizeof(cl_uint), &mapCount, NULL, wrap_data);
    if(err != CL_SUCCESS)
    {
        LogError("Erro: GetMemObjectInfo returned %d\n", err);
        return;
    }

    // if it is a buffer command so there is no other command details to show
    if(isBufferCommand)
    {
        goto End;
    }

    cl_image_format imgFormat;
    err = GetImageInfo(memObj, CL_IMAGE_FORMAT, sizeof(cl_image_format), &imgFormat, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }

    channelOrder = channelOrderToString(imgFormat.image_channel_order);
    channelDataType = channelTypeToString(imgFormat.image_channel_data_type);
    ss << ", Image Format = {";
    ss << channelOrder;
    ss << ", " << channelDataType;
    ss << "}";

    size_t elementSize;
    err = GetImageInfo(memObj, CL_IMAGE_ELEMENT_SIZE, sizeof(cl_image_format), &elementSize, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Element Size = " <<elementSize;

    size_t rowPitch;
    err = GetImageInfo(memObj, CL_IMAGE_ROW_PITCH, sizeof(cl_image_format), &rowPitch, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Row Pitch = " <<rowPitch;

    size_t slicePitch;
    err = GetImageInfo(memObj, CL_IMAGE_SLICE_PITCH, sizeof(cl_image_format), &slicePitch, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Slice Pitch = " << slicePitch;

    size_t imgWidth;
    err = GetImageInfo(memObj, CL_IMAGE_WIDTH, sizeof(cl_image_format), &imgWidth, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Width = " << imgWidth;

    size_t imgHeight;
    err = GetImageInfo(memObj, CL_IMAGE_HEIGHT, sizeof(cl_image_format), &imgHeight, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Height = " << imgHeight;

    size_t imgDepth;
    err = GetImageInfo(memObj, CL_IMAGE_HEIGHT, sizeof(cl_image_format), &imgDepth, NULL, wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %d\n");
        return;
    }
    ss << ", " << "Image Depth = " << imgDepth;

End:
    ss << ", Map Count = " << mapCount << ",";    //this comma to pass validation vs analyze system output file
    ss << endl;

    outLine = ss.str();
    // check if it first time we call this function
    map< int, list<string>, mapComparer >::iterator it =  wrap_data->memCommandsMap->find(queueId);
    if( it == wrap_data->memCommandsMap->end() )
    {
        list<string>* outList = new list<string>;
        outList->push_back(outLine);
        wrap_data->memCommandsMap->insert(pair<int, list<string> >(queueId, *outList ) );
        delete outList;
    }
    else
    {
        (*wrap_data->memCommandsMap)[queueId].push_back(outLine);
    }
}

// ***************** OpenCL 2.0 Wrappers *****************************************

// CL_CALLBACK added!
cl_int BuildProgram (    cl_program program,
     cl_uint num_devices,
     const cl_device_id *device_list,
     const char *options,
     void (CL_CALLBACK *pfn_notify)(cl_program, void *user_data),
     void *user_data,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);

    updateApiMap("clBuildProgram", ret_val, wrap_data);

    return ret_val;
}


cl_int CompileProgram (    cl_program program,
     cl_uint num_devices,
     const cl_device_id *device_list,
     const char *options,
     cl_uint num_input_headers,
     const cl_program *input_headers,
     const char **header_include_names,
     void (CL_CALLBACK *pfn_notify)( cl_program program, void *user_data),
     void *user_data,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clCompileProgram(program, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);

    updateApiMap("clCompileProgram", ret_val, wrap_data);

    return ret_val;
}


cl_mem CreateBuffer (    cl_context context,
     cl_mem_flags flags,
     size_t size,
     void *host_ptr,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_mem ret_val;
    cl_int err_code;

    ret_val = clCreateBuffer(context, flags, size, host_ptr, &err_code);

    updateApiMap("clCreateBuffer", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}

#ifdef ENABLE_OPENCL2_0
cl_command_queue CreateCommandQueueWithProperties(    cl_context context,
     cl_device_id device,
     const cl_queue_properties *properties,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_command_queue ret_val;
    cl_int err_code;

    ret_val = clCreateCommandQueueWithProperties(context, device, properties, &err_code);

    updateApiMap("clCreateCommandQueueWithProperties", err_code, wrap_data);

    if(err_code == CL_SUCCESS)
    {
        // if we created a new command queue we want to save pointer of it later
        wrap_data->commandQueuesOrder->push_back(&ret_val);
    }

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}
#endif // ENABLE_OPENCL2_0


// CL_CALLBACK added!
cl_context CreateContext(    cl_context_properties *properties,
     cl_uint num_devices,
     const cl_device_id *devices,
     void (CL_CALLBACK *pfn_notify) (
    const char *errinfo,
    const void *private_info,
    size_t cb,
    void *user_data
    ),
     void *user_data,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_context ret_val;
    cl_int err_code;

    ret_val = clCreateContext(properties, num_devices, devices, pfn_notify, user_data, &err_code);

    updateApiMap("clCreateContext", err_code, wrap_data);
    if(err_code == CL_SUCCESS)
    {
        wrap_data->contextsOrder->push_back(ret_val);
    }

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}

// CL_CALLBACK added!
cl_context CreateContextFromType (    cl_context_properties   *properties,
     cl_device_type  device_type,
     void  (CL_CALLBACK *pfn_notify) (const char *errinfo,
     const void  *private_info,
     size_t  cb,
     void  *user_data),
     void  *user_data,
     cl_int  *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_context ret_val;
    cl_int err_code;

    ret_val = clCreateContextFromType(properties, device_type, pfn_notify, user_data, &err_code);

    updateApiMap("clCreateContextFromType", err_code, wrap_data);
    if(err_code == CL_SUCCESS)
    {
        wrap_data->contextsOrder->push_back(ret_val);
    }

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_mem CreateImage (    cl_context context,
     cl_mem_flags flags,
     const cl_image_format *image_format,
     const cl_image_desc *image_desc,
     void *host_ptr,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_mem ret_val;
    cl_int err_code;

    ret_val = clCreateImage(context, flags, image_format, image_desc, host_ptr, &err_code);

    updateApiMap("clCreateImage", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_kernel CreateKernel (    cl_program  program,
     const char *kernel_name,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_kernel ret_val;
    cl_int err_code;

    ret_val = clCreateKernel(program, kernel_name, &err_code);

    updateApiMap("clCreateKernel", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_int CreateKernelsInProgram (    cl_program  program,
     cl_uint num_kernels,
     cl_kernel *kernels,
     cl_uint *num_kernels_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clCreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);

    updateApiMap("clCreateKernelsInProgram", ret_val, wrap_data);

    return ret_val;
}


cl_program CreateProgramWithBinary (    cl_context context,
     cl_uint num_devices,
     const cl_device_id *device_list,
     const size_t *lengths,
     const unsigned char **binaries,
     cl_int *binary_status,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_program ret_val;
    cl_int err_code;

    ret_val = clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, &err_code);

    updateApiMap("clCreateProgramWithBinary", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_program CreateProgramWithBuiltInKernels (    cl_context context,
     cl_uint num_devices,
     const cl_device_id *device_list,
     const char *kernel_names,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_program ret_val;
    cl_int err_code;

    ret_val = clCreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, &err_code);

    updateApiMap("clCreateProgramWithBuiltInKernels", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_program CreateProgramWithSource (    cl_context context,
     cl_uint count,
     const char **strings,
     const size_t *lengths,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_program ret_val;
    cl_int err_code;

    ret_val = clCreateProgramWithSource(context, count, strings, lengths, &err_code);

    updateApiMap("clCreateProgramWithSource", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}

#ifdef ENABLE_OPENCL2_0
cl_sampler CreateSamplerWithProperties (    cl_context context,
     const cl_sampler_properties *sampler_properties,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_sampler ret_val;
    cl_int err_code;

    ret_val = clCreateSamplerWithProperties(context, sampler_properties, errcode_ret);

    updateApiMap("clCreateSamplerWithProperties", &err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;

}
#endif // ENABLE_OPENCL2_0


cl_mem CreateSubBuffer (    cl_mem buffer,
     cl_mem_flags flags,
     cl_buffer_create_type buffer_create_type,
     const void *buffer_create_info,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_mem ret_val;
    cl_int err_code;

    ret_val = clCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, &err_code);

    updateApiMap("clCreateSubBuffer", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}



cl_int CreateSubDevices (    cl_device_id  in_device ,
     const cl_device_partition_property  *properties ,
     cl_uint  num_devices ,
     cl_device_id  *out_devices ,
     cl_uint  *num_devices_ret,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;

    ret_val = clCreateSubDevices(in_device, properties, num_devices, out_devices, num_devices_ret);

    updateApiMap("clCreateSubDevices", ret_val, wrap_data);

    return ret_val;
}


cl_event CreateUserEvent (    cl_context context,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_event ret_val;
    cl_int err_code;

    ret_val = clCreateUserEvent(context, &err_code);

    updateApiMap("clCreateUserEvent", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_int EnqueueBarrierWithWaitList (    cl_command_queue  command_queue ,
     cl_uint  num_events_in_wait_list ,
     const cl_event  *event_wait_list ,
     cl_event  *event ,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueBarrierWithWaitList", ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueCopyBuffer (    cl_command_queue command_queue,
     cl_mem src_buffer,
     cl_mem dst_buffer,
     size_t src_offset,
     size_t dst_offset,
     size_t cb,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueCopyBuffer", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_COPY_BUFFER", command_queue, src_buffer, &cb, false, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueCopyBufferRect (    cl_command_queue command_queue,
     cl_mem src_buffer,
     cl_mem dst_buffer,
     const size_t src_origin[3],
     const size_t dst_origin[3],
     const size_t region[3],
     size_t src_row_pitch,
     size_t src_slice_pitch,
     size_t dst_row_pitch,
     size_t dst_slice_pitch,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueCopyBufferRect", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_COPY_BUFFER_RECT", command_queue, src_buffer, region, true, ret_val, wrap_data);

    return ret_val;
}

cl_int EnqueueCopyBufferToImage (    cl_command_queue command_queue,
     cl_mem src_buffer,
     cl_mem  dst_image,
     size_t src_offset,
     const size_t dst_origin[3],
     const size_t region[3],
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueCopyBufferToImage", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_COPY_BUFFER_TO_IMAGE", command_queue, src_buffer, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueCopyImage (    cl_command_queue command_queue,
     cl_mem src_image,
     cl_mem dst_image,
     const size_t src_origin[3],
     const size_t dst_origin[3],
     const size_t region[3],
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueCopyImage", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_COPY_IMAGE", command_queue, src_image, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueCopyImageToBuffer (    cl_command_queue command_queue,
     cl_mem src_image,
     cl_mem  dst_buffer,
     const size_t src_origin[3],
     const size_t region[3],
     size_t dst_offset,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueCopyImageToBuffer", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_COPY_IMAGE_TO_BUFFER", command_queue, src_image, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueFillBuffer (    cl_command_queue  command_queue ,
     cl_mem  buffer ,
     const void  *pattern ,
     size_t  pattern_size ,
     size_t  offset ,
     size_t  size ,
     cl_uint  num_events_in_wait_list ,
     const cl_event  *event_wait_list ,
     cl_event  *event,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;
    size_t totalSize = size * pattern_size;

    ret_val = clEnqueueFillBuffer(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueFillBuffer", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_FILL_BUFFER", command_queue, buffer, &totalSize, false, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueFillImage (    cl_command_queue command_queue,
     cl_mem image,
     const void *fill_color,
     const size_t *origin,
     const size_t *region,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;


    ret_val = clEnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueFillImage", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_FILL_IMAGE", command_queue, image, region, true, ret_val, wrap_data);

    return ret_val;
}


void * EnqueueMapBuffer (    cl_command_queue command_queue,
     cl_mem buffer,
     cl_bool blocking_map,
     cl_map_flags map_flags,
     size_t offset,
     size_t cb,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    void* ret_val;
    cl_int err_code;

    ret_val = clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, &err_code);

    updateApiMap("clEnqueueMapBuffer", err_code, wrap_data);
    updateMemCommands("CL_COMMAND_MAP_BUFFER", command_queue, buffer, &cb, false, err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}

void* EnqueueMapImage (    cl_command_queue command_queue,
     cl_mem image,
     cl_bool blocking_map,
     cl_map_flags map_flags,
     const size_t origin[3],
     const size_t region[3],
     size_t *image_row_pitch,
     size_t *image_slice_pitch,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    void* ret_val;
    cl_int err_code;

    ret_val = clEnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, &err_code);

    updateApiMap("clEnqueueMapImage", err_code, wrap_data);
    updateMemCommands("CL_COMMAND_MAP_IMAGE", command_queue, image, region, true, err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}



cl_int EnqueueMarkerWithWaitList (    cl_command_queue  command_queue ,
     cl_uint  num_events_in_wait_list ,
     const cl_event  *event_wait_list ,
     cl_event  *event,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;

    ret_val = clEnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueMarkerWithWaitList", ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueMigrateMemObjects (    cl_command_queue  command_queue ,
     cl_uint  num_mem_objects ,
     const cl_mem  *mem_objects ,
     cl_mem_migration_flags  flags ,
     cl_uint  num_events_in_wait_list ,
     const cl_event  *event_wait_list ,
     cl_event  *event,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;
    size_t stamSize[3] = {0, 0, 0};

    ret_val = clEnqueueMigrateMemObjects(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueMigrateMemObjects", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_MIGRATE_MEM_OBJECTS", command_queue, *mem_objects, stamSize, true, CL_SUCCESS, wrap_data);

    return ret_val;
}

// CL_CALLBACK added!
cl_int EnqueueNativeKernel (    cl_command_queue command_queue,
     void (CL_CALLBACK *user_func)(void *),
     void *args,
     size_t cb_args,
     cl_uint num_mem_objects,
     const cl_mem *mem_list,
     const void **args_mem_loc,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueNativeKernel", ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueNDRangeKernel (    cl_command_queue command_queue,
     cl_kernel kernel,
     cl_uint work_dim,
     const size_t *global_work_offset,
     const size_t *global_work_size,
     const size_t *local_work_size,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueNDRangeKernel", ret_val, wrap_data);

    updateKernelLaunch("clEnqueueNDRangeKernel", command_queue, kernel,work_dim, global_work_offset, global_work_size, local_work_size, ret_val, wrap_data);

    return ret_val;
}

cl_int EnqueueReadBuffer (    cl_command_queue command_queue,
     cl_mem buffer,
     cl_bool blocking_read,
     size_t offset,
     size_t cb,
     void *ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueReadBuffer", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_READ_BUFFER", command_queue, buffer, &cb, false, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueReadBufferRect (    cl_command_queue command_queue,
     cl_mem buffer,
     cl_bool blocking_read,
     const size_t buffer_origin[3],
     const size_t host_origin[3],
     const size_t region[3],
     size_t buffer_row_pitch,
     size_t buffer_slice_pitch,
     size_t host_row_pitch,
     size_t host_slice_pitch,
     void *ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueReadBufferRect", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_READ_BUFFER_RECT", command_queue, buffer, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueReadImage (    cl_command_queue command_queue,
     cl_mem image,
     cl_bool blocking_read,
     const size_t origin[3],
     const size_t region[3],
     size_t row_pitch,
     size_t slice_pitch,
     void *ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueReadImage", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_READ_IMAGE", command_queue, image, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueUnmapMemObject (    cl_command_queue command_queue,
     cl_mem memobj,
     void *mapped_ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;
    size_t stamSize[3] = {0, 0, 0};

    ret_val = clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueUnmapMemObject", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_UNMAP_MEM_OBJECT", command_queue, memobj, stamSize, true, ret_val, wrap_data);

    return ret_val;
}



cl_int EnqueueWriteBuffer (    cl_command_queue command_queue,
     cl_mem buffer,
     cl_bool blocking_write,
     size_t offset,
     size_t cb,
     const void *ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueWriteBuffer", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_WRITE_BUFFER", command_queue, buffer, &cb, false, ret_val, wrap_data);

    return ret_val;
}

cl_int EnqueueWriteBufferRect (    cl_command_queue command_queue,
     cl_mem buffer,
     cl_bool blocking_write,
     const size_t buffer_origin[3],
     const size_t host_origin[3],
     const size_t region[3],
     size_t buffer_row_pitch,
     size_t buffer_slice_pitch,
     size_t host_row_pitch,
     size_t host_slice_pitch,
     void *ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueWriteBufferRect(command_queue, buffer, blocking_write, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueWriteBufferRect", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_WRITE_BUFFER_RECT", command_queue, buffer, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int EnqueueWriteImage (    cl_command_queue command_queue,
     cl_mem image,
     cl_bool blocking_write,
     const size_t origin[3],
     const size_t region[3],
     size_t input_row_pitch,
     size_t input_slice_pitch,
     const void * ptr,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueWriteImage", ret_val, wrap_data);
    updateMemCommands("CL_COMMAND_WRITE_IMAGE", command_queue, image, region, true, ret_val, wrap_data);

    return ret_val;
}


cl_int Finish (    cl_command_queue command_queue,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clFinish(command_queue);

    updateApiMap("clFinish", ret_val, wrap_data);

    return ret_val;
}


cl_int Flush (    cl_command_queue command_queue,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clFlush(command_queue);

    updateApiMap("clFlush", ret_val, wrap_data);

    return ret_val;
}



cl_int GetCommandQueueInfo(    cl_command_queue command_queue,
     cl_command_queue_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetCommandQueueInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetContextInfo (    cl_context context,
     cl_context_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t* param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetContextInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetDeviceIDs(    cl_platform_id platform,
     cl_device_type device_type,
     cl_uint num_entries,
     cl_device_id *devices,
     cl_uint *num_devices,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);

    updateApiMap("clGetDeviceIDs", ret_val, wrap_data);

    return ret_val;
}



cl_int GetDeviceInfo(    cl_device_id device,
     cl_device_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetDeviceInfo", ret_val, wrap_data);

    return ret_val;
}



cl_int GetEventInfo (    cl_event event,
     cl_event_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetEventInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetEventProfilingInfo (    cl_event event,
     cl_profiling_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetEventProfilingInfo", ret_val, wrap_data);

    return ret_val;
}

cl_int GetImageInfo (    cl_mem image,
     cl_image_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetImageInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetKernelArgInfo (    cl_kernel  kernel ,
     cl_uint  arg_indx ,
     cl_kernel_arg_info  param_name ,
     size_t  param_value_size ,
     void  *param_value ,
     size_t  *param_value_size_ret,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;

    ret_val = clGetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetKernelArgInfo", ret_val, wrap_data);

    return ret_val;
}

cl_int GetKernelInfo (    cl_kernel kernel,
     cl_kernel_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetKernelInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetKernelWorkGroupInfo (    cl_kernel kernel,
     cl_device_id device,
     cl_kernel_work_group_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetKernelWorkGroupInfo", ret_val, wrap_data);

    return ret_val;
}



cl_int GetMemObjectInfo (    cl_mem memobj,
     cl_mem_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetMemObjectInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetPlatformIDs(    cl_uint num_entries,
     cl_platform_id *platforms,
     cl_uint *num_platforms,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetPlatformIDs(num_entries, platforms, num_platforms);

    updateApiMap("clGetPlatformIDs", ret_val, wrap_data);

    return ret_val;
}



cl_int GetPlatformInfo(    cl_platform_id platform,
     cl_platform_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetPlatformInfo", ret_val, wrap_data);

    return ret_val;
}


cl_int GetProgramBuildInfo (    cl_program  program,
     cl_device_id  device,
     cl_program_build_info  param_name,
     size_t  param_value_size,
     void  *param_value,
     size_t  *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetProgramBuildInfo", ret_val, wrap_data);

    return ret_val;
}



cl_int GetProgramInfo (    cl_program program,
     cl_program_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetProgramInfo", ret_val, wrap_data);

    return ret_val;
}

cl_int GetSamplerInfo (    cl_sampler sampler,
     cl_sampler_info param_name,
     size_t param_value_size,
     void *param_value,
     size_t *param_value_size_ret,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);

    updateApiMap("clGetSamplerInfo", ret_val, wrap_data);

    return ret_val;
}



cl_int GetSupportedImageFormats ( cl_context context,
     cl_mem_flags flags,
     cl_mem_object_type image_type,
     cl_uint num_entries,
     cl_image_format *image_formats,
     cl_uint *num_image_formats,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);

    updateApiMap("clGetSupportedImageFormats", ret_val, wrap_data);

    return ret_val;
}


cl_program LinkProgram ( cl_context context,
     cl_uint num_devices,
     const cl_device_id *device_list,
     const char *options,
     cl_uint num_input_programs,
     const cl_program *input_programs,
     void (CL_CALLBACK *pfn_notify) (cl_program program, void *user_data),
     void *user_data,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_program ret_val;
    cl_int err_code;

    ret_val = clLinkProgram(context, num_devices, device_list, options, num_input_programs, input_programs, pfn_notify, user_data, &err_code);

    updateApiMap("clLinkProgram", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_int ReleaseCommandQueue(    cl_command_queue command_queue,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseCommandQueue(command_queue);

    updateApiMap("clReleaseCommandQueue", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseContext (    cl_context context,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseContext(context);

    updateApiMap("clReleaseContext", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseDevice ( cl_device_id device,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseDevice(device);

    updateApiMap("clReleaseDevice", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseEvent ( cl_event event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseEvent(event);

    updateApiMap("clReleaseEvent", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseKernel ( cl_kernel kernel,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseKernel(kernel);

    updateApiMap("clReleaseKernel", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseMemObject (    cl_mem memobj,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseMemObject(memobj);

    updateApiMap("clReleaseMemObject", ret_val, wrap_data);

    return ret_val;
}



cl_int ReleaseProgram (    cl_program program,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseProgram(program);

    updateApiMap("clReleaseProgram", ret_val, wrap_data);

    return ret_val;
}


cl_int ReleaseSampler (    cl_sampler sampler,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clReleaseSampler(sampler);

    updateApiMap("clReleaseSampler", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainCommandQueue(    cl_command_queue command_queue,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainCommandQueue(command_queue);

    updateApiMap("clRetainCommandQueue", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainContext (    cl_context context,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainContext(context);

    updateApiMap("clRetainContext", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainDevice (    cl_device_id  device,
    ocl_wrap_data* wrap_data )
{
    cl_int ret_val;

    ret_val = clRetainDevice(device);

    updateApiMap("clRetainDevice", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainEvent (    cl_event event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainEvent(event);

    updateApiMap("clRetainEvent", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainKernel (    cl_kernel kernel,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainKernel(kernel);

    updateApiMap("clRetainKernel", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainMemObject (    cl_mem memobj,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainMemObject(memobj);

    updateApiMap("clRetainMemObject", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainProgram (    cl_program program,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainProgram(program);

    updateApiMap("clRetainProgram", ret_val, wrap_data);

    return ret_val;
}


cl_int RetainSampler(    cl_sampler sampler,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clRetainSampler(sampler);

    updateApiMap("clRetainSampler", ret_val, wrap_data);

    return ret_val;
}


cl_int SetEventCallback (    cl_event event,
     cl_int  command_exec_callback_type ,
     void (CL_CALLBACK  *pfn_event_notify) (cl_event event, cl_int event_command_exec_status, void *user_data),
     void *user_data,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clSetEventCallback(event, command_exec_callback_type, pfn_event_notify, user_data);

    updateApiMap("clSetEventCallback", ret_val, wrap_data);

    return ret_val;
}


cl_int SetKernelArg (    cl_kernel kernel,
     cl_uint arg_index,
     size_t arg_size,
     const void *arg_value,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clSetKernelArg(kernel, arg_index, arg_size, arg_value);

    updateApiMap("clSetKernelArg", ret_val, wrap_data);

    return ret_val;
}


// CL_CALLBACK added!
cl_int SetMemObjectDestructorCallback (    cl_mem memobj,
     void (CL_CALLBACK  *pfn_notify) (cl_mem memobj,
     void *user_data),
     void *user_data,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clSetMemObjectDestructorCallback(memobj, pfn_notify, user_data);

    updateApiMap("clSetMemObjectDestructorCallback", ret_val, wrap_data);

    return ret_val;
}



cl_int SetUserEventStatus (    cl_event event,
     cl_int execution_status,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clSetUserEventStatus(event,execution_status);

    updateApiMap("clSetUserEventStatus", ret_val, wrap_data);

    return ret_val;
}


cl_int UnloadPlatformCompiler (    cl_platform_id platform,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clUnloadPlatformCompiler(platform);

    updateApiMap("clUnloadPlatformCompiler", ret_val, wrap_data);

    return ret_val;
}


cl_int WaitForEvents (    cl_uint num_events,
     const cl_event *event_list,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clWaitForEvents(num_events, event_list);

    updateApiMap("clWaitForEvents", ret_val, wrap_data);

    return ret_val;
}


#ifndef NOT_DEFINED

cl_command_queue CreateCommandQueue(    cl_context context,
     cl_device_id device,
     cl_command_queue_properties properties,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_command_queue ret_val;
    cl_int err_code;

    ret_val = clCreateCommandQueue(context, device, properties, &err_code);

    updateApiMap("clCreateCommandQueue", err_code, wrap_data);
    if(err_code == CL_SUCCESS)
    {
        // if we created a new command queue we want to save pointer of it later
        wrap_data->commandQueuesOrder->push_back(ret_val);
    }

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}


cl_sampler CreateSampler (    cl_context context,
     cl_bool normalized_coords,
     cl_addressing_mode addressing_mode,
     cl_filter_mode filter_mode,
     cl_int *errcode_ret,
    ocl_wrap_data* wrap_data)
{
    cl_sampler ret_val;
    cl_int err_code;

    ret_val = clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, &err_code);

    updateApiMap("clCreateSampler", err_code, wrap_data);

    // if the user ask for the return code value so we return it.
    if(errcode_ret != NULL)
    {
        *errcode_ret = err_code;
    }

    return ret_val;
}



cl_int EnqueueTask (    cl_command_queue command_queue,
     cl_kernel kernel,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
    ocl_wrap_data* wrap_data)
{
    cl_int ret_val;

    ret_val = clEnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);

    updateApiMap("clEnqueueTask", ret_val, wrap_data);
    updateKernelLaunch("clEnqueueTask", command_queue, kernel, 0, NULL, NULL, NULL, ret_val, wrap_data);

    return ret_val;
}
#endif
