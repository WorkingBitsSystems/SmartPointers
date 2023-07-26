/**
 * SP.h
 *
 * Abstract:
 * Definition of a template class that is a smart pointer class that maintains
 * an "owning pointer" to a memory location of the passed TYPESP.  Unlike the
 * UP<> class though, the SP<> class allows for multiple copies of itself to be
 * passed around, and uses an embedded UP<> and reference count to determine
 * when the object can be deleted.  Also, unlike the other smart pointers, the
 * data in the SP is a pointer to the managing OP and it's count of references.
 *
 * The SP class is similar to the std:shared_ptr<> class, but is not
 * interchangeable as it uses it's own internal count and UP.
 *
 * @see UP.h
 *
 * @copyright Copyright(C) Working Bits Systems, LLC 2023
 */
#ifndef _SP_H_
#define _SP_H_

#include <mutex>

#include "UP.h"


/**
 * The Working Bits Systems namespace
 */
namespace wbs
{
    // Forward declare the Weak Pointer type to make it a friend below.
    template < typename TYPEWP > class WP;

    /**
     * @class SP
     * This class manages ownership of an object of the TYPESP passed as the
     * template parameter.  It forces the semantics of an "owning" pointer such
     * that memory will not be leaked.  Copies of this pointer may be passed
     * around with multiple copies of the original SP<>, and this code will keep
     * track of the number of copies so that when the final one is destructed,
     * the memory is freed.
     */
    template< typename TYPESP > class SP
    {
    public:
        // Constructors, destructor, and Assignment operator -------------------
        /**
         * The Default Constructor.
         */
        SP()
        {
            m_internalObject = nullptr;
        }

        /**
         * A constructor from a c-style raw pointer.  This takes ownership from
         * the passed raw pointer and handles destroying the passed object.
         *
         * @param p - The c-style raw pointer whose object will be managed by
         *            this SP.
         */
        SP( TYPESP* p )
        {
            m_internalObject = new InternalObject( p );
        }

        /**
         * The Destructor.
         * This MAY delete the referenced object, IF the embedded tracking
         * count has gone to zero.
         */
        ~SP()
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecRef();
            }
        }

        /**
         * The copy constructor.
         *
         * @param other - The other SP to share ownership of the embedded object
         *                with.
         */
        SP( const SP< TYPESP >& other )
        {
            m_internalObject = other.m_internalObject;
            if ( nullptr != m_internalObject )
            {
                m_internalObject->AddRef();
            }
        }

        /**
         * The move semantics copy constructor.
         *
         * @param other - The other SP to pass ownership of the embedded object
         *                with.
         */
        SP( SP< TYPESP >&& other )
        {
            // Move the internal reference object and clear the passed
            // reference.  We can ignore reference counts as it'll be a
            // remove one followed by an add one.  Net zero.
            m_internalObject = other.m_internalObject;
            other.m_internalObject = nullptr;
        }

        /**
         * The assignment operator.
         *
         * @param other - The other SP to share ownership of the embedded object
         *                with.
         *
         * @return The reference to this pointer.
         */
        SP& operator=( const SP< TYPESP >& other )
        {
            // Skip self-assignment and assignment to the same internal object.
            if ( ( &other != this ) &&
                 ( other.m_internalObject != m_internalObject ) )
            {
                // If we were already pointing to an object, decrement the
                // reference to it.
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->DecRef();
                }

                // Assign the internal pointer and increment the reference.
                m_internalObject = other.m_internalObject;
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->AddRef();
                }
            }

            return *this;
        }

        /**
         * The move semantics assignment operator.
         *
         * @param other - The other SP to share ownership of the embedded object
         *                with.
         *
         * @return The reference to this pointer.
         */
        SP& operator=( SP< TYPESP >&& other )
        {
            // Skip self-assignment and assignment to the same internal object.
            if ( &other != this )
            {
                // If we were already pointing to an object, decrement the
                // reference to it.
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->DecRef();
                }

                // Assign the internal pointer.  We can ignore reference counts
                // as it'll be a remove one followed by an add one.  Net zero.
                m_internalObject = other.m_internalObject;

                // Clear the input SP's internal pointer.
                other.m_internalObject = nullptr;
            }

            return *this;
        }

        // Operations ----------------------------------------------------------
        /**
         * The arrow operator.
         *
         * @return The pointer to the referenced object.  If this pointer is
         *         unset (null), returns nullptr.
         */
        TYPESP* operator->()
        {
            if ( nullptr == m_internalObject )
            {
                return nullptr;
            }
            else
            {
                return m_internalObject->GetPtr();
            }
        }

        /**
         * The star operator.
         *
         * @return A reference to the pointed-to object.
         *
         * @note If the internal pointer is null, this will be the same as
         *       dereferencing a null pointer such as:
         *       char* p = nullptr;
         *       char c = *p;
         */
        TYPESP& operator*()
        {
            return *( m_internalObject->GetPtr() );
        }

        /**
         * Assignment from a c-style raw pointer.
         *
         * @param p - the c-style raw pointer to an object of type TYPESP.
         *
         * @return The reference to this pointer.
         */
        SP& operator=( TYPESP* p )
        {
            // Clear any existing reference.
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecRef();
            }

            m_internalObject = new InternalObject( p );

            return *this;
        }

        /**
         * Assignment from a UP.  MUST use move semantics to give up ownership.
         *
         * @param p - the c-style raw pointer to an object of type TYPESP.
         *
         * @return The reference to this pointer.
         */
        SP& operator=( UP< TYPESP >&& p )
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecRef();
            }

            m_internalObject = new InternalObject( p.UnsafeAccess() );

            p.m_ptr = nullptr;

            return *this;
        }

        /**
         * Comparison operators.  Lumping these all together for ease.
         *
         * @param rhs - the right-hand-side of the comparison (this is lhs)
         *
         * @return true if this pointer has the relationship to the rhs as
         *         specified.
         */
        inline bool operator==( const SP< TYPESP >& other ) const { return UnsafeAccess() == other.UnsafeAccess(); }
        inline bool operator!=( const SP< TYPESP >& other ) const { return UnsafeAccess() != other.UnsafeAccess(); }
        inline bool operator<( const SP< TYPESP >& other ) const  { return UnsafeAccess() < other.UnsafeAccess(); }
        inline bool operator<=( const SP< TYPESP >& other ) const { return UnsafeAccess() <= other.UnsafeAccess(); }
        inline bool operator>( const SP< TYPESP >& other ) const  { return UnsafeAccess() > other.UnsafeAccess(); }
        inline bool operator>=( const SP< TYPESP >& other ) const { return UnsafeAccess() >= other.UnsafeAccess(); }

        // Methods -------------------------------------------------------------
        /**
         * Unsafe access to the embedded c-style raw pointer.
         *
         * @return the internal TYPESP*, or nullptr if unassigned.
         */
        TYPESP* UnsafeAccess() const
        {
            // Clear any existing reference.
            if ( nullptr == m_internalObject )
            {
                return nullptr;
            }
            else
            {
                return m_internalObject->GetPtr();
            }
        }

        /**
         * Delete the object AS FAR AS THIS POINTER IS CONCERNED.  That is, this
         * method decrements the internal counter, and if it goes to zero, the
         * object is deleted.  Either way, the internal pointer is set to
         * nullptr.
         */
        void Delete()
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecRef();
                m_internalObject = nullptr;
            }
        }

        /**
         * Determined if this pointer is pointing to anything.
         *
         * @return true if the internal reference is null, false otherwise.
         */
        bool IsNull()
        {
            return ( nullptr == m_internalObject );
        }

    private:
        // We'll need to  make weak ptr (WP) a friend so it can access the
        // same InternalObject as SPs of the type..
        friend class wbs::WP< TYPESP >;

        // Attributes ----------------------------------------------------------
        // Inner class to manage reference counts and the object pointer.  Note
        // that this model holds a disadvantage to the std::shared_ptr as this
        // class is allocated for every object.  std::shared_ptr manages the
        // allocation and puts the extra space needed in the allocated object,
        // requiring a single memory allocation for both object and the
        // shared_ptr internal storage.  This trade-off allows for a cleaner
        // implementation, as well as the ability to use SPs with any pre-
        // existing object (see assignment from UPs).  For this implementation,
        // that could be mitigated by using an object/memory pool for this
        // class.
        class InternalObject
        {
        public:
            // Constructors, destructor, and Assignment operator ---------------
            // Set up the pointer, and start with a count of 1 shared reference
            //  (the one creating this object), and no weak references.
            InternalObject( TYPESP* p )
            : m_ptr( p ), m_refCount( 1 ), m_weakRefCount( 0 )
            {
            }

            // Increments the reference count.
            void AddRef()
            {
                // Lock access to keep reference counts from being modified
                // at the same time.
                m_lock.lock();
                ++m_refCount;
                m_lock.unlock();
            }

            // Decrement the reference count, and if the refCount is then zero,
            // delete the object we point to.  We also destroy this inner object
            // if we have no shared nor weak pointers pointing to it.
            void DecRef()
            {
                bool deleteMe = false;

                // Lock access until we get the counters figured out.
                m_lock.lock();
                --m_refCount;
                if ( 0 == m_refCount )
                {
                    // Delete the referenced object.
                    m_ptr.Delete();

                    // If we have no weak pointers, delete this internal object.
                    if ( 0 == m_weakRefCount )
                    {
                        // Have to release the lock first.  It's ok, if both
                        // counters are zero, nothing is pointing to this, so
                        // releasing the lock doesn't expose the object to
                        // issues.
                        deleteMe = true;
                    }
                }

                // Unlock before we (maybe) delete.
                m_lock.unlock();

                // Zero references of either kind means we delete ourselves.
                if ( deleteMe )
                {
                    delete this;
                }
            }

            // Increments the weak reference count.
            void AddWeakRef()
            {
                // Lock access to keep reference counts from being modified
                // at the same time.
                m_lock.lock();
                ++m_weakRefCount;
                m_lock.unlock();
            }

            // Decrement the weak reference count, and self-delete if the weak
            // and shared references are now zero.
            void DecWeakRef()
            {
                bool deleteMe = false;

                // Lock access until we get the counters figured out.
                m_lock.lock();
                --m_weakRefCount;
                if ( ( 0 == m_weakRefCount ) && ( 0 == m_refCount ) )
                {
                    // Note that the pointer should already be null with no
                    // shared pointers.
                    deleteMe = true;
                }

                m_lock.unlock();

                // Zero references means we delete ourselves.
                if ( deleteMe )
                {
                    delete this;
                }
            }

            TYPESP* GetPtr()
            {
                return m_ptr.UnsafeAccess();
            }

        private:
            ~InternalObject()
            {
                m_ptr.Delete();
            }

            // Attributes ------------------------------------------------------
            // Use an embedded UP to manage the object itself.  When this object
            // self-deletes, this UP deletes the object it points to.
            UP< TYPESP > m_ptr;

            // Keeps track of the number of shared pointers referencing this
            // pointer.  When this goes to zero the referenced object is
            // deleted.  When both this and the weakRefCount go to zero, this
            // internal object deletes itself.
            std::uint64_t m_refCount;

            // Keeps track of the number of weak pointers referencing this
            // pointer.  When both this and the shareds refCount go to zero,
            // this internal object deletes itself.
            std::uint64_t m_weakRefCount;

            // Mutex to protect access to this object's counters.
            // In particular, since we have 2 counters, we want to make sure
            // one thread doesn't chage a weak pointer count while another
            // changes a shared pointer count, and this object is left in limbo.
            std::mutex m_lock;

        };

        // The internal object that all of the shared pointers for this object
        // use to keep track of the object itself ands the count of existing
        // SPs.  It handles deleting itself if the count of holding SPs goes to
        // zero.
        InternalObject* m_internalObject;

        // A constructor for use by a WP, that initializes an SP based on the
        // InternalObject pointer.  This effectively promotes a weak pointer to
        // a new shared pointer.
        SP( InternalObject* p )
        {
            // Note that the InternalObject must be non-null, and the internal
            // pointer is also non-null to construct a useful SP.
            if ( ( nullptr != p ) &&
                 ( nullptr != p->GetPtr() ) )
            {

                m_internalObject = p;

                m_internalObject->AddRef();
            }
            else
            {
                m_internalObject = nullptr;
            }
        }
    };
}; // End of namespace wbs

#endif // #ifndef _OP_H_
