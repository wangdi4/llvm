// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/*
*
* File mic_tracer_counters.h
*
*/
public:
	typedef unsigned int unsigned_int;
	typedef unsigned long long unsigned_long_long;
	typedef char* char_pointer;

	TRACE_COMMAND_STRING(char_pointer, command_type, 1, initial);

	TRACE_COMMAND_SIMPLE(unsigned_long_long, command_id, 1, command_type);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_program_time_start, 1, command_id);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_program_time_end, 1, build_program_time_start);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_serialize_time_start, 1, build_program_time_end);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_serialize_time_end, 1, build_serialize_time_start);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_deserialize_time_start, 1, build_serialize_time_end);

	TRACE_COMMAND_TIMER(unsigned_long_long, build_deserialize_time_end, 1, build_deserialize_time_start);

	TRACE_COMMAND_TIMER(unsigned_long_long, command_host_time_start, 1, build_deserialize_time_end);

	TRACE_COMMAND_TIMER(unsigned_long_long, command_host_time_end, 1, command_host_time_start);

	TRACE_COMMAND_TIMER(unsigned_long_long, coi_execute_command_time_start, 1, command_host_time_end);

	TRACE_COMMAND_TIMER(unsigned_long_long, coi_execute_command_time_end, 1, coi_execute_command_time_start);

	TRACE_COMMAND_COUNTER(unsigned_int, num_of_buffer_operations, 1, coi_execute_command_time_end);

	TRACE_COMMAND_COUNTER(unsigned_long_long, buffer_operation_overall_size, 1, num_of_buffer_operations);

	TRACE_COMMAND_TIMER(unsigned_long_long, cmd_run_in_device_time_start, 1, buffer_operation_overall_size);

	TRACE_COMMAND_TIMER(unsigned_long_long, cmd_run_in_device_time_end, 1, cmd_run_in_device_time_start);

	TRACE_COMMAND_TIMER(unsigned_long_long, tbb_exe_in_device_time_start, 1, cmd_run_in_device_time_end);

	TRACE_COMMAND_TIMER(unsigned_long_long, tbb_exe_in_device_time_end, 1, tbb_exe_in_device_time_start);

	TRACE_COMMAND_STRING(char_pointer, kernel_name, 1, tbb_exe_in_device_time_end);

	TRACE_COMMAND_COUNTER(unsigned_int, num_of_buffer_sent_to_device, 1, kernel_name);

	TRACE_COMMAND_COUNTER(unsigned_long_long, buffers_size_sent_to_device, 1, num_of_buffer_sent_to_device);

	TRACE_COMMAND_SIMPLE(unsigned_long_long, global_work_size, 3, buffers_size_sent_to_device);

	TRACE_COMMAND_SIMPLE(unsigned_long_long, work_group_size, 3, global_work_size);

	TRACE_COMMAND_COUNTER(unsigned_int, thread_num_of_invocations, 120, work_group_size);

	TRACE_COMMAND_TIMER(unsigned_long_long, thread_overall_time, 120, thread_num_of_invocations);

	TRACE_COMMAND_COUNTER(unsigned_long_long, thread_num_wg_exe, 120, thread_overall_time);



	TRACE_COMMAND_LAST( thread_num_wg_exe );

private:

	void initializeAll()
	{
		init_command_type();
		init_command_id();
		init_build_program_time_start();
		init_build_program_time_end();
		init_build_serialize_time_start();
		init_build_serialize_time_end();
		init_build_deserialize_time_start();
		init_build_deserialize_time_end();
		init_command_host_time_start();
		init_command_host_time_end();
		init_coi_execute_command_time_start();
		init_coi_execute_command_time_end();
		init_num_of_buffer_operations();
		init_buffer_operation_overall_size();
		init_cmd_run_in_device_time_start();
		init_cmd_run_in_device_time_end();
		init_tbb_exe_in_device_time_start();
		init_tbb_exe_in_device_time_end();
		init_kernel_name();
		init_num_of_buffer_sent_to_device();
		init_buffers_size_sent_to_device();
		init_global_work_size();
		init_work_group_size();
		init_thread_num_of_invocations();
		init_thread_overall_time();
		init_thread_num_wg_exe();
	}

