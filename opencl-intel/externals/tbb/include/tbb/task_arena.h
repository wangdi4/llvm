/*
    Copyright (c) 2005-2020 Intel Corporation

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

#ifndef __TBB_task_arena_H
#define __TBB_task_arena_H

#include "detail/_task.h"
#include "detail/_exception.h"
#include "detail/_aligned_space.h"
#include "detail/_small_object_pool.h"

#if __TBB_NUMA_SUPPORT
#include "info.h"
#endif /*__TBB_NUMA_SUPPORT*/

namespace tbb {
namespace detail {
namespace d1 {

class arena;
class task_arena;

template<typename F, typename R>
class task_arena_function : public delegate_base {
    F &my_func;
    aligned_space<R> my_return_storage;
    // The function should be called only once.
    bool operator()() const override {
        new (my_return_storage.begin()) R(my_func());
        return true;
    }
public:
    task_arena_function(F& f) : my_func(f) {}
    // The function can be called only after operator() and only once.
    R consume_result() const {
        return std::move(*(my_return_storage.begin()));
    }
    ~task_arena_function() override {
        my_return_storage.begin()->~R();
    }
};

template<typename F>
class task_arena_function<F,void> : public delegate_base {
    F &my_func;
    bool operator()() const override {
        my_func();
        return true;
    }
public:
    task_arena_function(F& f) : my_func(f) {}
    void consume_result() const {}

    friend class task_arena_base;
};


class task_arena_base {
    friend class task_scheduler_observer;
    friend int current_thread_index();
    friend int max_concurrency();
    friend void submit(task&, task_arena&, task_group_context&, bool);
#if __TBB_NUMA_SUPPORT
public:
    // TODO: consider version approach to resolve backward compatibility potential issues.
    struct constraints {
        constraints(numa_node_id id = automatic, int maximal_concurrency = automatic)
            : numa_id(id)
            , max_concurrency(maximal_concurrency)
        {}
        numa_node_id numa_id;
        int max_concurrency;
    };
#endif /*__TBB_NUMA_SUPPORT*/
protected:
    //! NULL if not currently initialized.
    arena* my_arena;

    //! Concurrency level for deferred initialization
    int my_max_concurrency;

    //! Reserved master slots
    unsigned my_master_slots;

    //! Special settings
    intptr_t my_version_and_traits;

    std::atomic<do_once_state> my_initialization_state;

#if __TBB_NUMA_SUPPORT
    //! The NUMA node index to which the arena will be attached
    numa_node_id my_numa_id;

    // Do not access my_numa_id without the following runtime check.
    // Despite my_numa_id is accesible, it does not exist in task_arena_base on user side
    // if TBB_PREVIEW_NUMA_SUPPORT macro is not defined by the user. To be sure that
    // my_numa_id exists in task_arena_base layout we check the traits.
    // TODO: Consider increasing interface version for task_arena_base instead of this runtime check.
    numa_node_id numa_id() {
        return (my_version_and_traits & numa_support_flag) == numa_support_flag ? my_numa_id : automatic;
    }
#endif

    enum {
        default_flags = 0
#if __TBB_NUMA_SUPPORT
        , numa_support_flag = 1
#endif
    };

    task_arena_base(int max_concurrency, unsigned reserved_for_masters)
        : my_arena(nullptr)
        , my_max_concurrency(max_concurrency)
        , my_master_slots(reserved_for_masters)
#if __TBB_NUMA_SUPPORT
        , my_version_and_traits(default_flags | numa_support_flag)
#else
        , my_version_and_traits(default_flags)
#endif
        , my_initialization_state(do_once_state::uninitialized)
#if __TBB_NUMA_SUPPORT
        , my_numa_id(automatic)
#endif
        {}

#if __TBB_NUMA_SUPPORT
    task_arena_base(const constraints& constraints_, unsigned reserved_for_masters)
        : my_arena(nullptr)
        , my_max_concurrency(constraints_.max_concurrency)
        , my_master_slots(reserved_for_masters)
        , my_version_and_traits(default_flags | numa_support_flag)
        , my_initialization_state(do_once_state::uninitialized)
        , my_numa_id(constraints_.numa_id )
        {}
#endif /*__TBB_NUMA_SUPPORT*/

    void __TBB_EXPORTED_METHOD internal_initialize();
    void __TBB_EXPORTED_METHOD internal_terminate();
    bool __TBB_EXPORTED_METHOD internal_attach();
    void __TBB_EXPORTED_METHOD internal_enqueue( task& ) const;
    void __TBB_EXPORTED_METHOD internal_execute( delegate_base& ) const;
    void __TBB_EXPORTED_METHOD internal_wait() const;
    static int __TBB_EXPORTED_FUNC internal_max_concurrency( const task_arena * );
public:
    //! Typedef for number of threads that is automatic.
    static const int automatic = -1;
    static const int not_initialized = -2;
};

void __TBB_EXPORTED_FUNC isolate_within_arena(delegate_base& d, isolation_tag isolation = 0);

template<typename R, typename F>
R isolate_impl(F& f) {
    task_arena_function<F, R> func(f);
    isolate_within_arena(func);
    return func.consume_result();
}

/** 1-to-1 proxy representation class of scheduler's arena
 * Constructors set up settings only, real construction is deferred till the first method invocation
 * Destructor only removes one of the references to the inner arena representation.
 * Final destruction happens when all the references (and the work) are gone.
 */
class task_arena : public task_arena_base {

    template <typename F>
    class enqueue_task : public task {
        small_object_allocator m_allocator;
        const F m_func;

        void finalize(const execute_data& ed) {
            m_allocator.delete_object(this, ed);
        }
        task* execute(const execute_data& ed) override {
            m_func();
            finalize(ed);
            return nullptr;
        }
        task* cancel(const execute_data& ed) override {
            finalize(ed);
            return nullptr;
        }
    public:
        enqueue_task(const F& f, small_object_allocator& alloc) : m_allocator(alloc), m_func(f) {}
        enqueue_task(F&& f, small_object_allocator& alloc) : m_allocator(alloc), m_func(std::move(f)) {}
    };

    void mark_initialized() {
        __TBB_ASSERT( my_arena, "task_arena initialization is incomplete" );
        my_initialization_state.store(do_once_state::initialized, std::memory_order_release);
    }

    template<typename F>
    void enqueue_impl(F&& f) {
        initialize();
        small_object_allocator alloc{};
        internal_enqueue(*alloc.new_object<enqueue_task<typename std::decay<F>::type>>(std::forward<F>(f), alloc));
    }

    template<typename R, typename F>
    R execute_impl(F& f) {
        initialize();
        task_arena_function<F, R> func(f);
        internal_execute(func);
        return func.consume_result();
    }
public:
    //! Creates task_arena with certain concurrency limits
    /** Sets up settings only, real construction is deferred till the first method invocation
     *  @arg max_concurrency specifies total number of slots in arena where threads work
     *  @arg reserved_for_masters specifies number of slots to be used by master threads only.
     *       Value of 1 is default and reflects behavior of implicit arenas.
     **/
    task_arena(int max_concurrency_ = automatic, unsigned reserved_for_masters = 1)
        : task_arena_base(max_concurrency_, reserved_for_masters)
    {}

#if __TBB_NUMA_SUPPORT
    //! Creates task arena pinned to certain NUMA node
    task_arena(const constraints& constraints_, unsigned reserved_for_masters = 1)
        : task_arena_base(constraints_, reserved_for_masters)
    {}

    //! Copies settings from another task_arena
    task_arena(const task_arena &s) // copy settings but not the reference or instance
        : task_arena_base(constraints(s.my_numa_id, s.my_max_concurrency), s.my_master_slots)
    {}
#else
    //! Copies settings from another task_arena
    task_arena(const task_arena& a) // copy settings but not the reference or instance
        : task_arena_base(a.my_max_concurrency, a.my_master_slots)
    {}
#endif /*__TBB_NUMA_SUPPORT*/

    //! Tag class used to indicate the "attaching" constructor
    struct attach {};

    //! Creates an instance of task_arena attached to the current arena of the thread
    explicit task_arena( attach )
        : task_arena_base(automatic, 1) // use default settings if attach fails
    {
        if (internal_attach()) {
            mark_initialized();
        }
    }

    //! Forces allocation of the resources for the task_arena as specified in constructor arguments
    void initialize() {
        atomic_do_once([this]{ this->internal_initialize();}, my_initialization_state);
    }

    //! Overrides concurrency level and forces initialization of internal representation
    void initialize(int max_concurrency_, unsigned reserved_for_masters = 1) {
        __TBB_ASSERT(!my_arena, "Impossible to modify settings of an already initialized task_arena");
        if( !is_active() ) {
            my_max_concurrency = max_concurrency_;
            my_master_slots = reserved_for_masters;
            internal_initialize();
            mark_initialized();
        }
    }

#if __TBB_NUMA_SUPPORT
    void initialize(constraints constraints_, unsigned reserved_for_masters = 1) {
        __TBB_ASSERT(!my_arena, "Impossible to modify settings of an already initialized task_arena");
        if( !is_active() ) {
            my_numa_id = constraints_.numa_id;
            my_max_concurrency = constraints_.max_concurrency;
            my_master_slots = reserved_for_masters;
            internal_initialize();
            mark_initialized();
        }
    }
#endif /*__TBB_NUMA_SUPPORT*/

    //! Attaches this instance to the current arena of the thread
    void initialize(attach) {
        // TODO: decide if this call must be thread-safe
        __TBB_ASSERT(!my_arena, "Impossible to modify settings of an already initialized task_arena");
        if( !is_active() ) {
            if ( !internal_attach() ) {
                internal_initialize();
            }
            mark_initialized();
        }
    }

    //! Removes the reference to the internal arena representation.
    //! Not thread safe wrt concurrent invocations of other methods.
    void terminate() {
        if( is_active() ) {
            internal_terminate();
            my_initialization_state.store(do_once_state::uninitialized, std::memory_order_relaxed);
        }
    }

    //! Removes the reference to the internal arena representation, and destroys the external object.
    //! Not thread safe wrt concurrent invocations of other methods.
    ~task_arena() {
        terminate();
    }

    //! Returns true if the arena is active (initialized); false otherwise.
    //! The name was chosen to match a task_scheduler_init method with the same semantics.
    bool is_active() const { 
        return my_initialization_state.load(std::memory_order_acquire) == do_once_state::initialized;
    }

    //! Enqueues a task into the arena to process a functor, and immediately returns.
    //! Does not require the calling thread to join the arena

    template<typename F>
    void enqueue(F&& f) {
        enqueue_impl(std::forward<F>(f));
    }

    //! Joins the arena and executes a mutable functor, then returns
    //! If not possible to join, wraps the functor into a task, enqueues it and waits for task completion
    //! Can decrement the arena demand for workers, causing a worker to leave and free a slot to the calling thread
    //! Since C++11, the method returns the value returned by functor (prior to C++11 it returns void).
    template<typename F>
    auto execute(F&& f) -> decltype(f()) {
        return execute_impl<decltype(f())>(f);
    }

#if __TBB_EXTRA_DEBUG
    //! Returns my_master_slots
    int debug_reserved_slots() const {
        // Handle special cases inside the library
        return my_master_slots;
    }

    //! Returns my_max_concurrency
    int debug_max_concurrency() const {
        // Handle special cases inside the library
        return my_max_concurrency;
    }

    //! Wait for all work in the arena to be completed
    //! Even submitted by other application threads
    //! Joins arena if/when possible (in the same way as execute())
    void debug_wait_until_empty() {
        initialize();
        internal_wait();
    }
#endif //__TBB_EXTRA_DEBUG

    //! Returns the maximal number of threads that can work inside the arena
    int max_concurrency() const {
        // Handle special cases inside the library
        return (my_max_concurrency > 1) ? my_max_concurrency : internal_max_concurrency(this);
    }
};

//! Executes a mutable functor in isolation within the current task arena.
//! Since C++11, the method returns the value returned by functor (prior to C++11 it returns void).
template<typename F>
inline auto isolate(F&& f) -> decltype(f()) {
    return isolate_impl<decltype(f())>(f);
}

//! Returns the index, aka slot number, of the calling thread in its current arena
inline int current_thread_index() {
    int idx = current_thread_index_impl(nullptr);
    return idx == -1 ? task_arena_base::not_initialized : idx;
}

//! Returns the maximal number of threads that can work inside the arena
inline int max_concurrency() {
    return task_arena_base::internal_max_concurrency(nullptr);
}

void __TBB_EXPORTED_FUNC submit_impl(task&, arena&, task_group_context&, std::uintptr_t);

inline void submit(task& t, task_arena& arena, task_group_context& ctx, bool as_critical) {
    __TBB_ASSERT(arena.is_active(), nullptr);
    __TBB_ASSERT(arena.my_arena, nullptr);
    submit_impl(t, *arena.my_arena, ctx, as_critical ? 1 : 0);
}

} // namespace d1
} // namespace detail

inline namespace v1 {
using detail::d1::task_arena;

namespace this_task_arena {
using detail::d1::current_thread_index;
using detail::d1::max_concurrency;
using detail::d1::isolate;
} // namespace this_task_arena

} // inline namespace v1

} // namespace tbb
#endif /* __TBB_task_arena_H */
