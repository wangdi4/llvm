// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "cl_synch_objects.h"
#include <set>
#include <list>

// macro that each class T that wish to user SharedPtrBase<T> must call in its public section
#define PREPARE_SHARED_PTR(T)    \
    friend class Intel::OpenCL::Utils::SharedPtr<T >;  \
    friend class Intel::OpenCL::Utils::SharedPtrBase<const T >; \
    private:   /* this is to prevent taking SharedPtrs of non-heap objects */ \
    Intel::OpenCL::Utils::SharedPtr<T > operator&() { return Intel::OpenCL::Utils::SharedPtr<T >(this); } \
    Intel::OpenCL::Utils::ConstSharedPtr<T > operator&() const { return Intel::OpenCL::Utils::ConstSharedPtr<T >(this); } \
    public:  \
    const char* GetTypeName() const { return #T ; } \
    public:


namespace Intel { namespace OpenCL { namespace Utils {

/**
 * Initialize the shared pointers mechanism (needed for debug mode only)
 */
void InitSharedPtrs();

/**
 * Finalize the shared pointers mechanism (needed for debug mode only)
 */
void FiniSharedPts();

#ifdef _DEBUG
/**
 * Dump current state
 */
void DumpSharedPts( const char* map_title = nullptr, bool if_non_empty = false );
#endif


template<typename T> class SharedPtrBase;
class LifetimeReferenceCountedObjectContainer;

/**
 * This class represents an object that is reference counted and therefore can be used by SharedPtrBase
 */
class ReferenceCountedObject
{
public:

    /**
     * Destructor
     * It is virtual so that the object can be deleted from a pointer to any of its super-classes
     */
    virtual ~ReferenceCountedObject() { }

    /**
     * Increment the reference counter
     * @return the new counter's value
     */
    long IncRefCnt() const { return ++m_refCnt; }

    /**
     * Decrement the reference counter
     * @return the new counter's value
     */
    long DecRefCnt() const
    {
        long new_val;
        
        // immediatey after decreasing counter object may disappear in another thread.
        // do maximum before decreasing. 
        if (true == m_bCheckZombie)
        {
            // zombie support required
            new_val = DriveEnterZombieState();
        }
        else
        {
            // no zombie support required 
            new_val = --m_refCnt;
        }
        return new_val;
    }

    /**
     * @return the counter's value
     */
    long GetRefCnt() const { return m_refCnt; }

    /**
     * Do any clean up and delete this
     * @param bIsTerminate whether the application is terminating
     */
    virtual void Cleanup(bool bIsTerminate = false) { delete this; }

    /**
     * @return the type name of this object
     */
    virtual const char* GetTypeName() const = 0;

    /**
     * @return an address which identifies this ReferenceCountedObject uniquely
     */
    const ReferenceCountedObject* GetThis() const { return this; }

protected:

    /**
     * Constructor
     */
    ReferenceCountedObject() : 
         m_zombieLevelCnt(0),
#ifdef _DEBUG 
         m_bEnterZombieStateCalled(false),
#endif
         m_bCheckZombie(false), m_state(NORMAL) {}

    /**
     * Check if object is in Zombie State - non-zero RefCnt but not visible
     */
    bool isZombie() const { return (NORMAL != m_state); }

    /**
     * Object enters Zombie state when m_zombieLevelCnt > 0 and m_refCnt drops to m_zombieLevelCnt
     * Zombie state is locking - once entered object cannot exit it
     * Object may perform actions once entering Zombie state
     */
    enum EnterZombieStateLevel
    {
        TOP_LEVEL_CALL = 0,
        RECURSIVE_CALL
    };

    virtual void EnterZombieState( EnterZombieStateLevel call_level ) 
    {
#ifdef _DEBUG
        m_bEnterZombieStateCalled = true;
        assert( (TOP_LEVEL_CALL != call_level) && "RefCounted object was added to the LifetimeObjectContainer or IncZombieCnt() used without overriding EnterZombieState()" );
#endif
    }

    /**
     * This method should be used only when registering object in containters that should contain them until object death
     * In this case refCounts of this objects in a such containers should not be included in life-cycle BUT as refCnt 
     * reach zombie count value EnterZombieState should be fired.
     * This method is intended for LifetimeReferenceCountedObjectContainer and GenericMemObject that maintains its own children
     * in such a container.
     */
    void IncZombieCnt() const;

    /**
     * operator new
     * Prevent users of ReferenceCountedObject frrom allocating objects by themselves. The semantics is not changed.
     */
    void* operator new(size_t size) { return ::operator new(size); }

    /**
     * operator delete
     * Prevent users of ReferenceCountedObject from deleting objects by themselves. The semantics is not changed.
     */
    void operator delete(void* p) { ::operator delete(p); }

private:
    enum State
    {
        NORMAL = 0,
        ZOMBIE
    };

    long DriveEnterZombieState() const;

    mutable AtomicCounter            m_refCnt;
    mutable OclNonReentrantSpinMutex m_zombieLock;     // all zombie-related checks to be done inside lock except m_bCheckZombie
    mutable long                     m_zombieLevelCnt; // when object is considered zombie
#ifdef _DEBUG
    bool                             m_bEnterZombieStateCalled; // for debug - test for zombie protocol implementation
#endif
    mutable volatile bool            m_bCheckZombie;   // object should support zombies 
    mutable          State           m_state;          // zombie state

    friend class LifetimeReferenceCountedObjectContainer;
};

/**
 * Represents an abstract data type that simulates a pointer while providing additional features
 */
template<typename T>
class SmartPtr
{

public:

    /**
     * Destructor
     */
    virtual ~SmartPtr() { }

    /**
     * Cast operator to size_t
     * @return the address of the pointed to object as a size_t type
     */
    operator size_t() const { return (size_t)m_ptr; }

    /**
     * Indirection operator - this method should not be called if GetPtr() returns NULL
     * @return a reference to the pointed to object
     */
    T& operator*() const
    {
        return *m_ptr;
    }

    /**
     * Object derefernce operator - this method should not be called if GetPtr() returns NULL.
     * Note: the intension of this class is to prevent the user from getting a hold of T* directly.
     *  However, using this operator one can do this:
     *
     *      SharedPtrBase<T> p;
     *      T* pT = p.operator->();
     *
     *  But using this operator this way is an abuse. We provide this operator as a syntactic
     *  sugar instead of "(*p).foo();".
     *
     * @return a pointer to the pointed to object
     */
    T* operator->() const
    {
        return m_ptr;
    }

    /**
     * Note: use this method with caution - you get an unmanaged pointer!
     * @return the pointed to object
     */
    T* GetPtr() const { return m_ptr; }

protected:

    /**
     * Constructor
     * @param ptr a pointer to the object
     */
    SmartPtr(T* ptr = nullptr) : m_ptr(ptr) { }

    /**
     * the pointer to the object
     */
    T* m_ptr;

};

// forward declaration
template<typename T> class ConstSharedPtr;
template<typename T> class ConstWeakPtr;

/**
 * This template class represents a shared pointer to an object and offers regular pointer
 * semantics by overriding the dereferencing operators. This class deliberately does not directly
 * expose the object's pointer to the user, so that no accidental uncontrolled deletion of it might
 * occur. It handles reference counting of the object.
 * Note that it is the pointed object's responsibility to ensure synchronization of operations on
 * the reference count. SharedPtrBase class itself cannot avoid a race between the destruction of one
 * SharedPtrBase and the deletion of its pointed to object and the incrementing of the object's
 * reference count by another SharedPtrBase in the case where these two SharedPtrBase objects are the only
 * shared pointers to the object (see documentation of the constructors, assignment operator and
 * destructor for more details).
 *
 * @param T the type of object pointed to by the SharedPtrBase object (do not use SharedPtrBase<const T>
 *          directly; @see ConstSharedPtr). T must be a sub-class of ReferenceCountedObject.
 */
template <typename T>
class SharedPtrBase : public SmartPtr<T>
{
public:

    /**
     * Constructor
     * @param ptr a pointer to object of type T to which this SharedPtrBase will point or NULL. ptr's
     *            reference counter is incremented in this method if it is not NULL.
     */
    SharedPtrBase(T* ptr = nullptr) : SmartPtr<T>(ptr)
    {
        if (nullptr != this->m_ptr)
        {
            IncRefCnt();
        }
    }   

    /**
     * Copy constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtrBase whose object pointer will be copied to this SharedPtrBase. The
     *              reference counter of the object is incremented in this method if it is not
     *              NULL.
     */
    SharedPtrBase(const SharedPtrBase& other) : SmartPtr<T>(nullptr)
    {
        *this = other;
    }

    template<typename S>
    SharedPtrBase(const SharedPtrBase<S>& other) : SmartPtr<T>(nullptr)
    {       
        *this = other;
    }

#ifdef _HAS_CPP0X		
    /**
     * Move constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtrBase whose object pointer will be moved to this SharedPtrBase.
     */
    SharedPtrBase(SharedPtrBase&& other) : SmartPtr<T>(other.GetPtr())
    {
        other.NullifyWithoutDecRefCnt();
    }

    template<typename S>
    SharedPtrBase(SharedPtrBase<S>&& other) : SmartPtr<T>(other.GetPtr())
    {
        other.NullifyWithoutDecRefCnt();
    }
#endif        

    /**
     * l-value Assignment operators
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other another SharedPtrBase whose object pointer will be copied to this SharedfPtr. The
     *              reference counter of the currently pointed to object is decremented and it is
     *              deleted if it becomes 0 (if it is not NULL). The reference counter of the newly
     *              pointed to object is incremented in this method if it is not NULL.
     * @return a reference to this
     */
    SharedPtrBase& operator=(const SharedPtrBase& other)
    {
        return operator=<T>(other);
    }

    template<typename S>
    SharedPtrBase& operator=(const SharedPtrBase<S>& other)
    {
        if (other == *this)
        {
            return *this;
        }

        // get hold of the original object address
        T* ptr = this->m_ptr;
        // update m_ptr to the new address
        if (nullptr != other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = nullptr;
        }
        // increment the reference counter if m_ptr isn't null
        if (nullptr != this->m_ptr)
        {
            IncRefCnt();
        }
        /* Only now can we decrement the reference counter of the old address. This is to prevent the case where DecRefCntInt would cause the object containing this SharedPtrBase to be deleted and
            then accesses to m_ptr would already be invalid, if they had been performed after the call to DecRefCntInt. */
        DecRefCntInt(ptr);
        return *this;
    }

#ifdef _HAS_CPP0X	
    /**
     * r-value Assignment operators
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other another SharedPtrBase whose object pointer will be moved to this SharedfPtr. The
     *              reference counter of the currently pointed to object is decremented and it is
     *              deleted if it becomes 0 (if it is not NULL).
     * @return a reference to this
     */

    SharedPtrBase& operator=(SharedPtrBase&& other)
    {
        // see comments in l-value assignment operator
        T* ptr = this->m_ptr;
        if (other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = nullptr;
        }
        DecRefCntInt(ptr);
        other.NullifyWithoutDecRefCnt();
        return *this;
    }

    template<typename S>
    SharedPtrBase& operator=(SharedPtrBase<S>&& other)
    {
        // see comments in l-value assignment operator
        T* ptr = this->m_ptr;        
        if (other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = nullptr;
        }
        DecRefCntInt(ptr);
        other.NullifyWithoutDecRefCnt();
        return *this;
    }
#endif

    /**
     * nullify the pointer without decrementing the reference counter of the pointered to object.
     * NOTE: this is an internally auxiliary function not to be used outside of this class. It is not declared private, because we need to call it on SharedPtrBase objects of
     * different type parameters, which are actually not the same class.
     */
    void NullifyWithoutDecRefCnt()
    {
        this->m_ptr = nullptr;
    }

    /**
     * manually increment the reference counter of the pointed to object
     */
    void IncRefCnt();

    /**
     * manually decrement the reference counter of the pointed to object
     */
    void DecRefCnt()
    {
        DecRefCntInt(this->m_ptr);
    }

    /**
     * @return the value of the pointed to object's reference count
     */
    long GetRefCnt() const;

protected:

    /**
     * Handle the case where the reference counter of an object has reached 0.
     * @param ptr a pointer to the object
     */
    virtual void HandleRefCnt0(T* ptr) = 0;

private:

    void DecRefCntInt(T* ptr);      

};

/**
 * This class represents a non-const shared pointer.
 * It deletes it upon SharedPtr's destruction if it finds that it holds the last reference to the object.
 */
template<typename T>
class SharedPtr : public SharedPtrBase<T>
{
public:

    /**
     * Constructor
     * @param ptr a pointer to object of type T to which this SharedPtr will point or NULL. ptr's
     *            reference counter is incremented in this method if it is not NULL.
     */
    SharedPtr(T* ptr = nullptr) : SharedPtrBase<T>(ptr) { }

    /**
     * Copy constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtr whose object pointer will be copied to this SharedPtr. The
     *              reference counter of the object is incremented in this method if it is not
     *              NULL.
     */
    SharedPtr(const SharedPtr& other) : SharedPtrBase<T>(other) { }

    template<typename S>
    SharedPtr(const SharedPtr<S>& other) : SharedPtrBase<T>(other) { }

#ifdef _HAS_CPP0X		
    /**
     * Move constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtr whose object pointer will be moved to this SharedPtr.
     */
    SharedPtr(SharedPtr&& other) : SharedPtrBase<T>(other)
    {
    }

    template<typename S>
    SharedPtr(SharedPtr<S>&& other) : SharedPtrBase<T>(other)
    {
    }

    SharedPtr& operator=(const SharedPtr&) = default;
#endif    

    /**
     * Destructor
     * The reference counter of the currently pointed to object is decremented and it is deleted if
     * it becomes 0 (if it is not NULL).
     */
    ~SharedPtr()
    {
        SharedPtrBase<T>::DecRefCnt();
    }

    /**
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @return a SharedPtrBase<S> holding the pointed to object of this or holding NULL if this' object
     *         cannot be dynamically cast to S*
     */
    template<typename S>
    SharedPtr<S> DynamicCast() const
    {
        S* const pS = dynamic_cast<S*>(this->m_ptr);
        return pS;
    }

    template<typename S>
    SharedPtr<S> StaticCast() const
    {
        S* const pS = static_cast<S*>(this->m_ptr);
        return pS;
    }

    /**
     * Cast operator to ConstSharedPtr<T>
     * @return a ConstSharedPtr pointing to the object this points to
     */
    operator ConstSharedPtr<T>() const;

protected:

    // overriden method
    virtual void HandleRefCnt0(T* ptr);

    friend class ConstSharedPtr<T>;
};

/**
 * This template class represents a const shared pointer to an object. It extends
 * SharedPtrBase<const T> by adding an option to construct a ConstSharedPtr<T> from a
 * SharedPtrBase<T>, as a regular const pointer can be initialized by a regular non-
 * const pointer. It is assumed that ConstSharedPtr are destroyed before the reference counter of 
 * the object they point to reaches 0.
 * 
 * @param T the type of object pointed to by the ConstSharedPtr (without the 'const' modifier)
 */
template<typename T>
class ConstSharedPtr : public SharedPtrBase<const T>
{
    
public:
    
    /**
     * Constructor
     * @param ptr a const pointer to object of type T to which this ConstSharedPtr will point.
     *            ptr's reference counter is incremented in this method.
     */
    ConstSharedPtr(const T* ptr = nullptr) : SharedPtrBase<const T>(ptr) { }

    /**
     * Constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtrBase whose object pointer will be copied to this ConstSharedfPtr. The
     *              reference counter of the object is incremented in this method if it is not
     *              NULL.
     */
    ConstSharedPtr(const ConstSharedPtr& other) : SharedPtrBase<const T>(other) { }

    template<typename S>
    ConstSharedPtr(const ConstSharedPtr<S>& other) : SharedPtrBase<const T>(other) { }

#ifdef _HAS_CPP0X		
    /**
     * Move constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other ConstSharedPtr whose object pointer will be moved to this ConstSharedPtr.
     */
    ConstSharedPtr(ConstSharedPtr&& other) : SharedPtrBase<const T>(other)
    {
    }

    template<typename S>
    ConstSharedPtr(ConstSharedPtr<S>&& other) : SharedPtrBase<const T>(other)
    {
    }

    ConstSharedPtr& operator=(const ConstSharedPtr&) = default;
#endif    

    /**
     * Destructor
     * The reference counter of the currently pointed to object is decremented and it is deleted if
     * it becomes 0 (if it is not NULL).
     */
    ~ConstSharedPtr()
    {
        SharedPtrBase<const T>::DecRefCnt();
    }

    /**
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @return a SharedPtrBase<S> holding the pointed to object of this or holding NULL if this' object
     *         cannot be dynamically cast to S*
     */
    template<typename S>
    ConstSharedPtr<S> DynamicCast() const
    {
        const S* const pS = dynamic_cast<const S*>(this->m_ptr);
        return pS;
    }

    template<typename S>
    ConstSharedPtr<S> StaticCast() const
    {
        const S* const pS = static_cast<const S*>(this->m_ptr);
        return pS;
    }

protected:

    // overriden method
    virtual void HandleRefCnt0(const T* ptr);
    
};

/**
 * operator less than of SmartPtr
 */
template<typename T>
bool operator<(const SmartPtr<T>& a, const SmartPtr<T>& b)
{
    return a.GetPtr() < b.GetPtr();
}

/**
 * This class represents a container that should contain SharedPtrs for the whole object lifetime.
 */
class LifetimeReferenceCountedObjectContainer
{
protected:
    bool isZombie( const ReferenceCountedObject* o ) const { return o->isZombie(); }
    void IncZombieCnt( ReferenceCountedObject* o ) const { o->IncZombieCnt(); }
};

template<typename T>
class LifetimeObjectContainer : public LifetimeReferenceCountedObjectContainer
{
public:

    void add( const SharedPtr<T>& ptr );
    void remove( const SharedPtr<T>& ptr );

    template<class T1>
        void getObjects( T1& containerToFill );

private:
    std::set< SharedPtr<T> >  m_set;
    OclMutex                 m_lock;
};

}}}
