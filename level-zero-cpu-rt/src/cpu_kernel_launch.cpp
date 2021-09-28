// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#define _CRT_SECURE_NO_WARNINGS // for fopen on WIN32
#include "cm_printf_host.h"
#include "ze_kernel.hpp"
#include "ze_utils.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <list>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace __zert__ {

struct Machine {
public:
  int n_dss_per_machine() {
    std::string n_dss_str = util::get_envvar("L0SIM_N_DSS_PER_MACHINE");
    if (!n_dss_str.empty()) {
      return std::atoi(n_dss_str.c_str());
    } else {
      return default_n_dss_per_machine();
    }
  };

  virtual int n_eu_per_dss() = 0;

  virtual ~Machine() {}

private:
  virtual int default_n_dss_per_machine() = 0;
};

// TODO fit in the correct value
struct ELG : Machine {
private:
  // https://gfxspecs.intel.com/Predator/Home/Index/65637
  // Follow the Project 2x2 (2x4 Fused)
  virtual int default_n_dss_per_machine() override { return 4; }

public:
  virtual int n_eu_per_dss() override { return 8; }
};

struct PVC : Machine {
private:
  virtual int default_n_dss_per_machine() override { return 64; }

public:
  virtual int n_eu_per_dss() override { return 8; }
};

struct ATS : Machine {
private:
  virtual int default_n_dss_per_machine() override { return 8; }

public:
  virtual int n_eu_per_dss() override { return 16; }
};

struct SKL : Machine {
private:
  virtual int default_n_dss_per_machine() override { return 3; }

public:
  virtual int n_eu_per_dss() override { return 8; }
};

template <class T> struct thread_safe_queue_t {
private:
  using value_t = std::unique_ptr<T>;
  static value_t mk_value(T const &value) { return std::make_unique<T>(value); }
  std::list<value_t> list_;
  std::mutex mutex_;

public:
  uint32_t count() const { return uint32_t(list_.size()); }

  // push back atomically list of values
  void push_back(std::vector<T> const &values) {
    std::unique_lock<std::mutex> lk(mutex_);
    for (auto const &value : values) {
      list_.push_back(mk_value(value));
    }
  }
  // atomically push value to the back, and then pop one from the front
  // if the queue is empty, the same value will be poped as pushed, and the
  // queue will remain empty
  value_t push_back_then_pop_front(value_t value_new) {
    std::unique_lock<std::mutex> lk(mutex_);
    if (value_new) {
      list_.push_back(std::move(value_new));
    }
    value_t value_old;
    if (!list_.empty()) {
      value_old = this->mk_value(*list_.front());
      list_.pop_front();
    }
    return value_old;
  }
};

struct SemaphoreForParallelDispatchedKernelsPerDevice {
public:
  SemaphoreForParallelDispatchedKernelsPerDevice(Semaphore *semaphore) {
    semaphore_ = semaphore;
    semaphore->wait();
  }

  ~SemaphoreForParallelDispatchedKernelsPerDevice() { semaphore_->notify(); }

private:
  Semaphore *semaphore_ = nullptr;
};

ze_result_t CPUdevice::launch(ZeKernel *kernel, ze_group_count_t config) {
  // some values are used in multi-threaded setting, if not const and not
  // inside its own scope, must be made atomic

  // semaphore here is used to limit the number of dispatched kernels in
  // parallel for this device
  (void)kernel;
  if (config.groupCountX > 10)
    return ZE_RESULT_SUCCESS;
#if 0
    SemaphoreForParallelDispatchedKernelsPerDevice semaphore(
        this->semaphore_.get());

    std::atomic<long long> nready_groups =
        config.groupCountX * config.groupCountY * config.groupCountZ;
    std::array<uint32_t, 3> const group_count = {
        config.groupCountX, config.groupCountY, config.groupCountZ};

    auto factorize_group_idx = [group_count](size_t index) {
        std::array<uint32_t, 3> group_idx;
        group_idx[0] = uint32_t(index % group_count[0]);
        group_idx[1] = uint32_t((index / group_count[0]) % group_count[1]);
        group_idx[2] = uint32_t(index / (group_count[0] * group_count[1]));
        return group_idx;
    };


    unsigned int num_threads_in_group;

    genISAi_kernel_get_num_threads_in_group(kernel->genISAi_kernel_,
                                            &num_threads_in_group);

    struct worker_t
    {
        genISAi_interpreter_t const interp;
        size_t const linear_group_idx;
        uint32_t time_slice;
    };

    // use thread_safe queue
    using ready_queue_t = thread_safe_queue_t<worker_t>;
    ready_queue_t ready_queue;

    // get time slice value
    auto const time_slice_str = util::get_envvar("L0SIM_INTERP_MAX_TIME_SLICE");
    uint32_t const max_time_slice =
        time_slice_str.empty() ? 1 << 31 : std::atoi(time_slice_str.c_str());

    std::atomic<size_t> global_linear_group_idx = 0;

    // allocate atomics equal to total number of group
    // threads_done_in_group must not be mutated, but its content is atomic
    std::vector<std::atomic<uint32_t>> threads_done_in_group(nready_groups);

    // Since kernel is shared by mutiple threads, so we must atomically
    // change kernel variables
    static std::mutex kernel_mutex_;

    // release all interpreter instances in ready queue
    auto release_interpreter_instances = [&]() {
        auto worker = ready_queue.push_back_then_pop_front({});
        while (worker)
        {
            {
                std::unique_lock<std::mutex> lk(kernel_mutex_);
                genISAi_interpreter_destroy(worker->interp);
            }
            worker = ready_queue.push_back_then_pop_front({});
        }
    };

    auto dispatch_next_group = [&]() -> ze_result_t {
        // all vardefs inside this scope are thread-private
        //
        // no group left, return
        if (nready_groups.fetch_sub(1) <= 0)
        {
            return ZE_RESULT_SUCCESS;
        }

        // get next value fo linear_group_idx
        auto linear_group_idx = global_linear_group_idx.fetch_add(1);
        assert(linear_group_idx < threads_done_in_group.size());
        threads_done_in_group.at(linear_group_idx) = 0;

        // create interpreter group
        std::vector<genISAi_interpreter_t> group(num_threads_in_group);
        auto group_idx = factorize_group_idx(linear_group_idx);
        genISAi_status_t status;
        {
            // create groups atomically
            std::unique_lock<std::mutex> lk(kernel_mutex_);

            // create group of interpreters
            status = genISAi_kernel_create_interpreters_in_3d_group(
                kernel->genISAi_kernel_,
                group_count.data(),
                group_idx.data(),
                num_threads_in_group,
                group.data());
        }

        if (GENISAI_STATUS_SUCCESS != status)
        {
            ZESIMERR
                << "genISAi: failed to create 3d group of interpreters, error="
                << status;
            return ZE_RESULT_ERROR_UNKNOWN;
        }

        // create workers in the group
        std::vector<worker_t> workers;
        workers.reserve(group.size());
        for (auto i : group)
        {
            for (uint32_t pos = 0; pos < kernel->param_count_; ++pos)
            {
                auto status = genISAi_interpreter_set_kernel_argument_value(
                    i,
                    pos,
                    kernel->param_sizes_[pos],
                    kernel->arguments_[pos].data());
                if (status != GENISAI_STATUS_SUCCESS)
                {
                    ZESIMERR << "genISAi: failed to set kernel argument value, "
                                "error= "
                             << status;
                    return ZE_RESULT_ERROR_UNKNOWN;
                }
            }
            status = genISAi_interpreter_initialize_thread_data(i);
            if (status != GENISAI_STATUS_SUCCESS)
            {
                ZESIMERR << "genISAi: failed to initialize thread data, "
                            "error= "
                         << status;
                return ZE_RESULT_ERROR_UNKNOWN;
            }
            uint32_t mask = -1;
            status = genISAi_interpreter_set_execution_mask(i, mask);
            if (status != GENISAI_STATUS_SUCCESS)
            {
                ZESIMERR << "genISAi: failed to set execution mask data, "
                            "error= "
                         << status;
                return ZE_RESULT_ERROR_UNKNOWN;
            }
            workers.push_back({i, linear_group_idx, max_time_slice});
        }

        // push workers to the end of the queue
        ready_queue.push_back(workers);

        return ZE_RESULT_SUCCESS;
    };

    auto get_resident_group_count = [&](auto device_kind) {
        // nresident_group depends on SKU/arch, Kernel resources
        // logic for computation goes here..
        uint32_t n_threads_per_eu;
        genISAi_kernel_get_n_threads_per_eu(kernel->genISAi_kernel(),
                                            &n_threads_per_eu);

        std::unique_ptr<Machine> machine = nullptr;
        switch (device_kind)
        {
        case GENISAI_DEVICE_KIND_SKL:
            machine = std::make_unique<SKL>();
            break;
        case GENISAI_DEVICE_KIND_ATS:
            machine = std::make_unique<ATS>();
            break;
        case GENISAI_DEVICE_KIND_PVC:
            machine = std::make_unique<PVC>();
            break;
        case GENISAI_DEVICE_KIND_ELG:
            machine = std::make_unique<ELG>();
            break;
        }

        auto nresident_groups = machine->n_dss_per_machine() *
                                machine->n_eu_per_dss() * n_threads_per_eu /
                                num_threads_in_group;
        assert(machine->n_dss_per_machine() * machine->n_eu_per_dss() *
                   n_threads_per_eu % num_threads_in_group ==
               0);
        return nresident_groups;
    };

    cpu_device_kind_t device_kind;
    auto result = kernel->module()->device()->getPlatform(device_kind);
    if (ZE_RESULT_SUCCESS != result)
    {
        ZESIMERR << "Invalid platform";
        return ZE_RESULT_ERROR_UNKNOWN;
    }
    size_t const nresident_groups = get_resident_group_count(device_kind);

    // schedule the first nresident_groups
    for (uint32_t i = 0; i < nresident_groups; ++i)
    {
        auto result = dispatch_next_group();
        if (ZE_RESULT_SUCCESS != result)
        {
            release_interpreter_instances();
            return result;
        }
        if (nready_groups == 0)
        {
            break;
        }
    }

    auto get_num_exec_threads = [&ready_queue]() {
        auto exec_threads = util::get_envvar("L0SIM_NUM_EXEC_THREADS");
        uint32_t num_threads = 1;
        if (!exec_threads.empty())
        {
            num_threads = std::atoi(exec_threads.c_str());
            num_threads = std::min(num_threads, ready_queue.count());
            fprintf(stderr,
                    "genisai_kernel_launch: using %d threads\n",
                    num_threads);
        }
        else
        {
            fprintf(stderr,
                    "genisai_kernel_launch:`L0SIM_NUM_EXEC_THREADS` not set, "
                    "using %d threads\n",
                    num_threads);
        }
        return num_threads;
    };
    uint32_t const num_threads = get_num_exec_threads();

    // result per thread, size is not mutated, but content is assigned once
    std::vector<ze_result_t> ze_result(num_threads, ZE_RESULT_ERROR_UNKNOWN);

    auto worker_func = [&](int thread_idx) {
        size_t count = 0;
        size_t count_until = 1 << 31;
        auto worker = ready_queue.push_back_then_pop_front({});
        while (worker)
        {
            if (count++ == count_until)
            {
                break;
            }
            genISAi_status_t status;
            status = genISAi_interpreter_single_step(worker->interp);
            if (GENISAI_STATUS_SUCCESS != status)
            {
                ZESIMERR << "genISAi: failed to single step, "
                            "error= "
                         << status;
                return;
            }
            genISAi_interpreter_state_t state;
            status = genISAi_interpreter_get_state(worker->interp, &state);
            if (GENISAI_STATUS_SUCCESS != status)
            {
                ZESIMERR << "genISAi: failed to get state, "
                            "error= "
                         << status;
                return;
            }

            worker->time_slice--;

            if (GENISAI_INTERPRETER_STATE_READY == state &&
                worker->time_slice == 0)
            {
                state = GENISAI_INTERPRETER_STATE_YIELD;
            }

            switch (state)
            {
            case GENISAI_INTERPRETER_STATE_YIELD:
                worker->time_slice = max_time_slice;
                worker =
                    ready_queue.push_back_then_pop_front(std::move(worker));
                break;
            case GENISAI_INTERPRETER_STATE_DONE: {
                // Release genISAi resources, i.e. interpreter instance and
                // interpreter group instance
                // Destroy interpreter instance atomically
                {
                    std::unique_lock<std::mutex> lk(kernel_mutex_);
                    status = genISAi_interpreter_destroy(worker->interp);
                }
                if (GENISAI_STATUS_SUCCESS != status)
                {
                    ZESIMERR
                        << "genISAi: failed to destroy interpreter instance, "
                           "error= "
                        << status;
                    return;
                }

                auto ndone = threads_done_in_group.at(worker->linear_group_idx)
                                 .fetch_add(1);
                if (ndone == num_threads_in_group - 1)
                {
                    auto result = dispatch_next_group();
                    if (ZE_RESULT_SUCCESS != result)
                    {
                        ze_result[thread_idx] = result;
                        return;
                    }
                }
                worker = ready_queue.push_back_then_pop_front({});
                break;
            }
            case GENISAI_INTERPRETER_STATE_READY:
                break;
            }
        }

        ze_result[thread_idx] = ZE_RESULT_SUCCESS;
        return;
    };

    std::vector<std::unique_ptr<std::thread>> worker_threads;
    for (uint32_t i = 0; i < num_threads; ++i)
    {
        worker_threads.push_back(std::make_unique<std::thread>(worker_func, i));
    };

    for (auto &thread : worker_threads)
    {
        thread->join();
    }

    for (auto &result : ze_result)
    {
        if (ZE_RESULT_SUCCESS != result)
        {
            release_interpreter_instances();
            return result;
        }
    }

    // get print buffer
    unsigned int size = 0;
    if (GENISAI_STATUS_SUCCESS != genISAi_kernel_get_print_buffer(
                                      kernel->genISAi_kernel_, &size, nullptr))
    {
        ZESIMERR << "Cannot retrieve print buffer size";
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    std::vector<unsigned char> print_buffer(size);
    if (GENISAI_STATUS_SUCCESS !=
        genISAi_kernel_get_print_buffer(
            kernel->genISAi_kernel_, &size, print_buffer.data()))
    {
        ZESIMERR << "Cannot retrieve print buffer content";
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    // dump print buffer to dispay by default, will dump to file if
    // L0SIM_PRINT_DUMP_FILE_NAME env variable is set
    auto dump_file = util::get_envvar("L0SIM_PRINT_DUMP_FILE_NAME");
    if (dump_file.empty() || dump_file == "0")
    {
        // dump to display
        DumpAllThreadOutput(stdout, print_buffer.data(), print_buffer.size());
    }
    else
    {
        FILE *streamOutFile = fopen(dump_file.c_str(), "wb");
        if (streamOutFile == nullptr)
        {
            ZESIMERR << "failed to open file";
            return ZE_RESULT_ERROR_UNKNOWN;
        }

        DumpAllThreadOutput(
            streamOutFile, print_buffer.data(), print_buffer.size());

        // Flush and close stream
        fflush(streamOutFile);
        fclose(streamOutFile);
    }
#endif
  return ZE_RESULT_SUCCESS;
}

} // namespace __zert__
