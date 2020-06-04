/*
    Copyright (c) 2020 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef __TBB__task_H
#define __TBB__task_H

#include "_config.h"
#include "_assert.h"
#include "_template_helpers.h"
#include "_small_object_pool.h"

#include <cstddef>
#include <cstdint>
#include <climits>
#include <utility>
#include <atomic>

namespace tbb {
namespace detail {
namespace d1 {

class task;
class task_arena;
class wait_object;
class task_group_context;
class thread_data;
class task_dispatcher;

//! An id as used for specifying affinity
using affinity_id = unsigned short;

//! A tag for task isolation
using isolation_tag = intptr_t;
constexpr isolation_tag no_isolation = 0;

// TODO (TBB_REVAMP_TODO): reconsider entry points
//! Task spawn/wait entry points
void __TBB_EXPORTED_FUNC spawn_impl(task& t, task_group_context& ctx);
void __TBB_EXPORTED_FUNC spawn_with_affinity_impl(task& t, task_group_context& ctx, affinity_id id);
void __TBB_EXPORTED_FUNC execute_and_wait_impl(task&t, task_group_context& t_ctx, wait_object&, task_group_context& w_ctx);
void __TBB_EXPORTED_FUNC wait_impl(wait_object&, task_group_context& ctx);

//! A tag for suspend point
// Do not place under __TBB_RESUMABLE_TASKS. It is a stub for unsupported platforms.
struct suspend_point_type;
using suspend_point = suspend_point_type*;
using suspend_callback_type = void(*)(void*, suspend_point);

#if __TBB_RESUMABLE_TASKS
//! The resumable tasks entry points
void __TBB_EXPORTED_FUNC internal_suspend(suspend_callback_type suspend_callback, void* user_callback);
void __TBB_EXPORTED_FUNC internal_resume(suspend_point tag);
suspend_point __TBB_EXPORTED_FUNC internal_current_suspend_point();

template <typename F>
static void suspend_callback(void* user_callback, suspend_point tag) {
    // Copy user function to a new stack after the context switch to avoid a race when the previous
    // suspend point is resumed while the user_callback is being called.
    F user_callback_copy = *static_cast<F*>(user_callback);
    user_callback_copy(tag);
}

template <typename F>
void suspend(F f) {
    internal_suspend(&suspend_callback<F>, &f);
}

inline void resume(suspend_point tag) {
    internal_resume(tag);
}
#endif

class wait_object {
    static constexpr std::uint64_t abandon_wait_flag = 1LLU << 33;
    static constexpr std::uint64_t overflow_mask = ~((1LLU << 32) - 1) & ~abandon_wait_flag;

    std::uint64_t m_version_and_traits{};
    std::atomic<std::uint64_t> m_ref_count{};
#if __TBB_RESUMABLE_TASKS
    suspend_point m_waiting_coroutine{};

    void abandon_wait() {
        __TBB_ASSERT((m_ref_count.load(std::memory_order_relaxed) & abandon_wait_flag) == 0, "The wait object can be abandoned only once");
        add_reference(abandon_wait_flag);
    }
#endif /* __TBB_RESUMABLE_TASKS */

    void add_reference(const std::int64_t delta) {
        std::uint64_t r = m_ref_count.fetch_add(delta) + delta;
        __TBB_ASSERT_EX((r & overflow_mask) == 0, "Overflow is detected");
#if __TBB_RESUMABLE_TASKS
        if (r == abandon_wait_flag) {
            // There is no any more references but the waiting stack is
            // suspended (abandoned) so resume it.
            __TBB_ASSERT(m_waiting_coroutine != nullptr, nullptr);
            m_ref_count.store(0, std::memory_order_relaxed);
            resume(m_waiting_coroutine);
        }
#endif /* __TBB_RESUMABLE_TASKS */
    }

    void internal_reserve_wait(std::uint32_t delta) {
        add_reference(delta);
    }

    void internal_release_wait(std::uint32_t delta) {
        add_reference(-std::int64_t(delta));
    }

    bool continue_execution() const {
        std::uint64_t r = m_ref_count.load(std::memory_order_acquire);
        __TBB_ASSERT_EX((r & overflow_mask) == 0, "Overflow is detected");
        return (r & ~abandon_wait_flag) > 0;
    }

    friend class thread_data;
    friend class task_dispatcher;
    friend class external_waiter;
    friend class task_group;
    friend class task_group_base;
    friend class task_arena_base;
    friend struct suspend_point_type;
public:
    wait_object() = default;
    // Despite the internal reference count is uin64_t we limit the user interface with uint32_t
    // to preserve a part of the internal reference count for special needs.
    wait_object(std::uint32_t ref_count) : m_ref_count{ref_count} { suppress_unused_warning(m_version_and_traits); }
    wait_object(const wait_object&) = delete;

    void reserve_wait(std::uint32_t delta = 1) {
        internal_reserve_wait(delta);
    }

    void release_wait(std::uint32_t delta = 1) {
        internal_release_wait(delta);
    }
#if __TBB_EXTRA_DEBUG
    unsigned reference_count() const {
        return unsigned(m_ref_count.load(std::memory_order_acquire));
    }
#endif
};

struct execute_data;
execute_data* __TBB_EXPORTED_FUNC current_execute_data_impl();
int __TBB_EXPORTED_FUNC current_thread_index_impl(const execute_data*);

struct execute_data {
    task_group_context* context{};
    unsigned short task_owner_thread_id{};
    unsigned short task_affinity_id{};

    int current_thread_index() const {
        int idx = current_thread_index_impl(this);
        __TBB_ASSERT(idx >= 0, nullptr);
        return idx;
    }

    bool is_stolen_task() const {
        return current_thread_index() != task_owner_thread_id;
    }

    bool is_not_my_affinity() const {
        return task_affinity_id != 0 && task_affinity_id != (current_thread_index() + 1);
    }

    unsigned short current_affinity() const {
        return (unsigned short)(current_thread_index() + 1);
    }
};

class task_traits {
    std::uint64_t m_version_and_traits{};
    friend struct task_accessor;
};

//! Alignment for a task object
static constexpr std::size_t task_alignment = 64;

//! Base class for user-defined tasks.
/** @ingroup task_scheduling */
class alignas(task_alignment) task : public task_traits {
protected:
    virtual ~task() = default;

public:
    virtual task* execute(const execute_data&) = 0;
    virtual task* cancel(const execute_data&) = 0;

    using affinity_id = unsigned short;

    static void spawn(task& t, task_group_context& ctx) { spawn_impl(t, ctx); }

    static void spawn_with_affinity(task& t, task_group_context& ctx, affinity_id id) { spawn_with_affinity_impl(t, ctx, id); }

    static void execute_and_wait(task& t, task_group_context& t_ctx, wait_object& wo, task_group_context& w_ctx) { execute_and_wait_impl(t, t_ctx, wo, w_ctx); }

    static void wait(wait_object& wo, task_group_context& ctx) { wait_impl(wo, ctx); }

    static execute_data* current_execute_data() { return current_execute_data_impl(); }

private:
    std::uint64_t m_reserved[5];

    // Reserve one pointer-sized object in derived class
    // static_assert(sizeof(task) == 64 - 8);

    friend struct task_accessor;
};

} // namespace d1
} // namespace detail
} // namespace tbb

#endif /* __TBB__task_H */
