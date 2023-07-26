/**
 * SP.h
 *
 * Abstract:
 * Definition of a template class that is a smart pointer class that keeps track
 * of a shared pointer (SP), but without ownership of that referenced class.
 * This class keeps track of the internal structures used by the SP in order to
 * query whether an object still exists or has been deleted.
 *
 * The WP class is similar to the std:weak_ptr<> class, but is not
 * interchangeable as it uses an SP with it's own internal count and UP.
 *
 * @see SP.h UP.h
 *
 * @copyright Copyright(C) Working Bits Systems, LLC 2023
 */
#ifndef _WP_H_
#define _WP_H_

#include "SP.h"

/**
 * The Working Bits Systems namespace
 */
namespace wbs
{
    /**
     * @class WP
     * This class is like an SP (shared pointer), except it does not have
     * ownership on the shared object.  Instead, it tracks the shared pointer's
     * internal object that points to the shared object.  If the shared pointers
     * on this object go out of scope, the weak pointer can recognize this, and
     * signal that the pointer is no longer usable.
     */
    template< typename TYPEWP > class WP
    {
    public:
        // Constructors, destructor, and Assignment operator -------------------
        /**
         * The Default Constructor.
         */
        WP()
        {
            m_internalObject = nullptr;
        }

        /**
         * Construction from an SP.
         *
         * @param p - the SP to an object of type TYPEWP.
         */
        WP( const SP< TYPEWP >& p )
        {
            m_internalObject = p.m_internalObject;

            if ( nullptr != m_internalObject )
            {
                m_internalObject->AddWeakRef();
            }
        }

        /**
         * The Destructor.
         * This MAY delete the internal object, IF the embedded tracking
         * counts have gone to zero.
         */
        ~WP()
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecWeakRef();
            }
        }

        /**
         * The copy constructor.
         *
         * @param other -The other WP to also track the object without ownership
         */
        WP( const WP< TYPEWP >& other )
        {
            m_internalObject = other.m_internalObject;
            if ( nullptr != m_internalObject )
            {
                m_internalObject->AddWeakRef();
            }
        }

        /**
         * The move semantics copy constructor.
         *
         * @param other - The other WP to pass ownership of the embedded object
         *                with.
         */
        WP( WP< TYPEWP >&& other )
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
         * @param other - The other WP to track the embedded object
         *
         * @return The reference to this pointer.
         */
        WP& operator=( const WP< TYPEWP >& other )
        {
            // Skip self-assignment and assignment to the same internal object.
            if ( ( &other != this ) &&
                 ( other.m_internalObject != m_internalObject ) )
            {
                // If we were already pointing to an object, decrement the
                // reference to it.
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->DecWeakRef();
                }

                // Assign the internal pointer and increment the reference.
                m_internalObject = other.m_internalObject;
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->AddWeakRef();
                }
            }

            return *this;
        }

        /**
         * The move semantics assignment operator.
         *
         * @param other - The other WP to we are getting the internal structure
         *                from.
         *
         * @return The reference to this pointer.
         */
        WP& operator=( WP< TYPEWP >&& other )
        {
            // Skip self-assignment and assignment to the same internal object.
            if ( &other != this )
            {
                // If we were already pointing to an object, decrement the
                // reference to it.
                if ( nullptr != m_internalObject )
                {
                    m_internalObject->DecWeakRef();
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
         * Assignment from an SP.
         *
         * @param p - the SP to an object of type TYPEWP.
         *
         * @return The reference to this pointer.
         */
        WP& operator=( const SP< TYPEWP >& p )
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecWeakRef();
            }

            m_internalObject = p.m_internalObject;

            if ( nullptr != m_internalObject )
            {
                m_internalObject->AddWeakRef();
            }

            return *this;
        }

        // Methods -------------------------------------------------------------
        /**
         * Use an SP in order to access the object.  This prevents the object
         * from going away while the weak pointer tries to access it.
         *
         * @return An SP pointing to the object this WP is referencing.  Note
         *         that this SP may be a null pointer, and should be checked
         *         before use.
         */
        SP< TYPEWP > GetSP()
        {
            return std::move( SP< TYPEWP >( m_internalObject ) );
        }

        /**
         * Drops the weak reference to the internal object.
         */
        void Drop()
        {
            if ( nullptr != m_internalObject )
            {
                m_internalObject->DecWeakRef();
                m_internalObject = nullptr;
            }
        }

        /**
         * Determines if this pointer is pointing to anything.
         *
         * @return true if the internal reference is null, false otherwise.
         *
         * @note If this is to see if the weak pointer is valid before use, it
         *       is better to get the SP, and check that.  Otherwise, timing may
         *       cause this to return true, but the SP is actually null by the
         *       time it is created.
         */
        bool IsNull()
        {
            return ( ( nullptr == m_internalObject ) ||
                     ( nullptr == m_internalObject->GetPtr() ) );
        }

    private:
        // Attributes ----------------------------------------------------------


        // The internal object that all of the shared pointers for this object
        // use to keep track of the object itself ands the count of existing
        // SPs.  It handles deleting itself if the count of holding SPs goes to
        // zero.
        typename SP< TYPEWP >::InternalObject* m_internalObject;
    };
}; // End of namespace wbs

#endif // #ifndef _OP_H_
