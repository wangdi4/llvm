namespace APIDebugger.ProbTraceMapClasses
{
    using System;
    using System.Collections.Generic;
    using System.CodeDom.Compiler;
    using Microsoft.VisualStudio.TestTools.UITest.Extension;
    using Microsoft.VisualStudio.TestTools.UITesting;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
    using Mouse = Microsoft.VisualStudio.TestTools.UITesting.Mouse;
    using MouseButtons = System.Windows.Forms.MouseButtons;
    using System.Drawing;
    using System.Windows.Input;
    using System.Text.RegularExpressions;
    
    
    public partial class ProbTraceMap
    {
        public string[] api_calls = {"clCreateContext", "clCreateKernel", "clEnqueueWriteBuffer", "clWaitForEvents", "clGetEventInfo", "clReleaseEvent", "clReleaseMemObject", "clCreateProgramWithSource",
                                 "clBuildProgram", "clCreateKernelsInProgram", "clRetainKernel", "clReleaseKernel", "clRetainProgram", "clReleaseProgram", "clCreateBuffer", "clSetKernelArg",
                                 "clEnqueueNDRangeKernel", "clEnqueueReadBuffer", "clCreateCommandQueue", "clRetainCommandQueue", "clReleaseCommandQueue", "clGetDeviceInfo", "clCreateContextFromType",
                                 "clGetProgramBuildInfo", "clGetProgramInfo", "clCreateProgramWithBinary", "clFinish", "clCreateSampler", "clGetSamplerInfo", "clSetMemObjectDestructorCallback",
                                 "clGetMemObjectInfo", "clCreateSubBuffer", "clEnqueueNativeKernel", "clEnqueueTask", "clRetainMemObject", "clGetCommandQueueInfo", "clGetPlatformIDs",
                                 "clGetPlatformInfo", "clGetDeviceIDs", "clGetContextInfo", "clRetainContext", "clReleaseContext", "clGetKernelWorkGroupInfo", "clGetKernelArgInfo", "clGetKernelInfo",
                                 "clEnqueueCopyBufferRect", "clEnqueueReadBufferRect", "clEnqueueWriteBufferRect", "clEnqueueCopyBuffer", "clGetSupportedImageFormats", "clGetImageInfo", 
                                 "clEnqueueCopyBufferToImage", "clEnqueueMapBuffer", "clEnqueueCopyImage", "clEnqueueReadImage", "clEnqueueCopyImageToBuffer", "clCreateImage", "clEnqueueWriteImage",
                                 "clEnqueueMapImage", "clEnqueueUnmapMemObject", "clCreateSubDevices", "clRetainDevice", "clReleaseDevice", "clEnqueueMigrateMemObjects", "clRetainSampler",
                                 "clReleaseSampler", "clCompileProgram", "clCreateUserEvent", "clRetainEvent", "clLinkProgram", "clSetUserEventStatus", "clEnqueueMarkerWithWaitList",
                                 "clFlush", "clEnqueueBarrierWithWaitList", "clEnqueueFillBuffer", "clSetEventCallback"};
        public string[] api_err_calls = {"clCreateContext", "clCreateKernel", "clEnqueueWriteBuffer", "clWaitForEvents", "clGetEventInfo", "clReleaseEvent", "clReleaseMemObject", "clCreateProgramWithSource",
                                 "clBuildProgram", "clCreateKernelsInProgram", "clRetainKernel", "clReleaseKernel", "clRetainProgram", "clReleaseProgram", "clCreateBuffer", "clSetKernelArg",
                                 "clEnqueueNDRangeKernel", "clEnqueueReadBuffer", "clCreateCommandQueue", "clRetainCommandQueue", "clReleaseCommandQueue", "clGetDeviceInfo", "clCreateContextFromType",
                                 "clGetProgramBuildInfo", "clGetProgramInfo", "clCreateProgramWithBinary", "clFinish", "clCreateSampler", "clGetSamplerInfo", "clSetMemObjectDestructorCallback",
                                 "clGetMemObjectInfo", "clCreateSubBuffer", "clEnqueueNativeKernel", "clEnqueueTask", "clRetainMemObject", "clGetCommandQueueInfo", "clGetPlatformIDs",
                                 "clGetPlatformInfo", "clGetDeviceIDs", "clGetContextInfo", "clRetainContext", "clReleaseContext", "clGetKernelWorkGroupInfo", "clGetKernelArgInfo", "clGetKernelInfo",
                                 "clEnqueueCopyBufferRect", "clEnqueueReadBufferRect", "clEnqueueWriteBufferRect", "clEnqueueCopyBuffer", "clGetSupportedImageFormats", "clGetImageInfo", 
                                 "clEnqueueCopyBufferToImage", "clEnqueueMapBuffer", "clEnqueueCopyImage", "clEnqueueReadImage", "clEnqueueCopyImageToBuffer", "clCreateImage", "clEnqueueWriteImage",
                                 "clEnqueueMapImage", "clEnqueueUnmapMemObject", "clCreateSubDevices", "clRetainDevice", "clReleaseDevice", "clEnqueueMigrateMemObjects", "clRetainSampler",
                                 "clReleaseSampler", "clCompileProgram", "clCreateUserEvent", "clRetainEvent", "clLinkProgram", "clSetUserEventStatus", "clEnqueueMarkerWithWaitList",
                                 "clFlush", "clEnqueueBarrierWithWaitList", "clEnqueueFillBuffer", "clSetEventCallback"};
    }
}
