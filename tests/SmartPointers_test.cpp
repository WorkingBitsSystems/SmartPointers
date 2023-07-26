#include <iostream>
#include <exception>
#include <string>
#include <cassert>
#include <vector>
#include <memory>

#include <chrono>
#include <ctime>
#include <cmath>

#include "../include/SmartPointers.h"

using namespace wbs;

// #define TEST_COMPILER_ERRORS 1

class TestPtr
{
public:
   TestPtr( int x, int y )
   : a(x), b(y)
   {
       ++total;
       if ( output ) std::cout << "Add new TestPtr object, new total = " << total << std:: endl;
   }

   ~TestPtr()
   {
       if ( output ) std::cout << "Destroyed TestPtr with a = " << a << ", b = " << b << std::endl;
       --total;
       if ( output ) std::cout << "Total objects = " << total << std::endl;
   }

   int a;
   int b;

   static int total;
   static bool output;
};
int TestPtr::total = 0;
bool TestPtr::output = true;

class TestAlloc
{
public:
    TestAlloc()
    {
        bigint = 0;
        bigint2 = -1;
    }

    void* operator new( size_t sz )
    {
        std::cout << "New allocating " << sz << " bytes for TestAlloc" << std::endl;
        return ::operator new( sz );
    }

    void operator delete( void* ptr, size_t size )
    {
        std::cout << "Deleting " << size << " bytes for TestAlloc" << std::endl;
        ::operator delete( ptr, size );
    }

private:
    std::uint64_t bigint;
    std::uint64_t bigint2;
};

void testfunc( UP< TestPtr > p )
{
    // Transfer the UP using the assignment operator with move semantics.
    UP< TestPtr > x;
    x = std::move( p );
    // Transfer should clear input ptr
    assert( p.IsNull() );
    assert( !x.IsNull() );

   TP< TestPtr > y = x;

#ifdef TEST_COMPILER_ERRORS
    UP<TestPtr> z = y;   // Compile error - can't make UP from TP

    UP< TestPtr > a = x; // Compile error - can't do a regular assignment.

    SP<TestPtr> b = y;   // Compile error  - can't make an SP from a TP
#endif
}

void testfunc2( TP<TestPtr> p )
{
   TP<TestPtr> x = p;

   TP<TestPtr> y = x;

}

int main( int argc, char** argv )
{
    // Use an inner block to control smart pointer lifetimes.
    {
        //*********************** UP Tests *********************************
        // Constructors
        // Uninitialized.
        UP< TestPtr > up0;

        // Init UP with raw pointer.
        UP< TestPtr > up1( new TestPtr( 1, 3 ) );
        // Should not be null, and see the values.
        assert( !up1.IsNull() );
        assert( 1 == up1->a );
        assert( 3 == up1->b );
        assert( 1 == TestPtr::total );

        // Init UP with unique pointer.
        std::unique_ptr< TestPtr > cppup = std::make_unique< TestPtr >( 3, 4 );
        UP< TestPtr > up2( cppup );
        // unique_ptr should now be null
        assert( !cppup );
        // UP should not be null, and able to see values.
        assert( !up2.IsNull() );
        assert( 3 == up2->a );
        assert( 4 == up2->b );
        assert( 2 == TestPtr::total );

        // Create with move semantics
        UP< TestPtr > up3( std::move( up2 ) );
        // up2 should now be null, with up3 pointing to the object.
        // And test with the operator*()
        assert( up2.IsNull() );
        assert( !up3.IsNull() );
        assert( 3 == (*up3).a );
        assert( 4 == (*up3).b );
        assert( 2 == TestPtr::total );

        // Test move semantics assignment AND destructor
        {
            UP< TestPtr > up4;
            up4 = std::move( up3 );
            // up2 should now be null, with up3 pointing to the object.
            assert( up3.IsNull() );
            assert( !up4.IsNull() );
            assert( 3 == up4->a );
            assert( 4 == up4->b );
        }
        assert( 1 == TestPtr::total );

        // Verify the UnsafeAccess()
        TestPtr* usp = up1.UnsafeAccess();
        assert( !up1.IsNull() );
        assert( nullptr != usp );
        assert( 1 == up1->a );
        assert( 3 == up1->b );
        assert( 1 == usp->a );
        assert( 3 == usp->b );
        assert( 1 == TestPtr::total );
        usp = nullptr;

        // Verify comparisons
        assert( up1 == up1 );
        assert( !( up1 != up1 ) );
        assert( up1 <= up1 );
        assert( up1 >= up1 );
        UP< TestPtr > up7( new TestPtr( 22,33 ) );
        assert( up1 != up7 );
        assert( up7 != up1 );
        assert( !( up1 == up7 ) );
        if ( up1.UnsafeAccess() < up7.UnsafeAccess() )
        {
            assert( up1 < up7 );
            assert( up7 > up1 );
            assert( up1 <= up7 );
            assert( up7 >= up1 );
        }
        else
        {
            assert( up7 < up1 );
            assert( up1 > up7 );
            assert( up7 <= up1 );
            assert( up1 >= up7 );
        }
        up7.Delete();

        // Verify export to std::unique_ptr
        UP< TestPtr > up5( new TestPtr( 17, 18 ) );
        assert( 2 == TestPtr::total );
        assert( !up5.IsNull() );
        assert( 17 == up5->a );
        assert( 18 == up5->b );
        std::unique_ptr< TestPtr > sup = up5.StdUniquePtr();
        assert( 2 == TestPtr::total );
        assert( up5.IsNull() );
        assert( 17 == sup->a );
        assert( 18 == sup->b );
        sup.reset();
        assert( 1 == TestPtr::total );


        // Verify Delete().
        UP< TestPtr > up6( new TestPtr( 11, 12 ) );
        assert( 2 == TestPtr::total );
        assert( !up6.IsNull() );
        assert( 11 == up6->a );
        assert( 12 == up6->b );
        up6.Delete();
        assert( 1 == TestPtr::total );
        assert( up6.IsNull() );


        //*********************** TP Tests *********************************

        // Make a TP from a raw pointer.
        TestPtr* raw = new TestPtr( 13, 14 );
        TP< TestPtr > tp0( raw );
        assert( 2 == TestPtr::total );
        assert( !tp0.IsNull() );
        assert( 13 == tp0->a );
        assert( 14 == tp0->b );
        assert( raw == tp0.RawAccess() );

        // Test Release() call that clears the tp pointer without deleting the
        // pointed-to object.
        tp0.Release();
        assert( 2 == TestPtr::total );
        assert( tp0.IsNull() );
        // Get rid of the unmanaged object.
        delete raw;
        raw = nullptr;
        assert( 1 == TestPtr::total );

        // Make a TP from the UP.
        TP< TestPtr > tp1( up1 );
        assert( 1 == TestPtr::total );
        assert( !tp1.IsNull() );
        assert( 1 == tp1->a );
        assert( 3 == tp1->b );

        // Verify the UP and TP are pointing to the same object.
        assert( 1 == up1->a );
        assert( 1 == tp1->a );

        // Test dereferencing by operator*() on UP.
        ++(*up1).a;
        assert( 2 == up1->a );
        assert( 2 == tp1->a );

        // Test dereferencing by operator*() on TP.
        ++(*tp1).b;
        assert( 4 == tp1->b );
        assert( 4 == up1->b );

        // Make a TP from a unique_ptr
        cppup = std::make_unique< TestPtr >( 16, 17 );
        TP< TestPtr > tp2( cppup );
        // unique_ptr should not be null
        assert( cppup );
        // TP should not be null, and able to see values.
        assert( !tp2.IsNull() );
        assert( 16 == tp2->a );
        assert( 17 == tp2->b );
        assert( 2 == TestPtr::total );
        // Check the Copy constructor and assignment operator.
        TP< TestPtr > tp3( tp2 );
        TP< TestPtr > tp4;
        tp4 = tp3;
        assert( !tp2.IsNull() );
        assert( 16 == tp2->a );
        assert( 17 == tp2->b );
        assert( !tp3.IsNull() );
        assert( 16 == tp3->a );
        assert( 17 == tp3->b );
        assert( !tp4.IsNull() );
        assert( 16 == tp4->a );
        assert( 17 == tp4->b );
        assert( 2 == TestPtr::total );
        // Get rid of the unique_ptr object.
        cppup.reset();
        assert( !cppup );
        assert( 1 == TestPtr::total );
        // Note: TPs do not know unique_ptr went out of scope...
        assert( !tp2.IsNull() );
        // ...so clear them.
        tp2.Release();
        assert( tp2.IsNull() );
        tp3.Release();
        assert( tp3.IsNull() );
        tp4.Release();
        assert( tp4.IsNull() );
        assert( 1 == TestPtr::total );

        // Function call with a TP made from the UP
        testfunc2( up1 );
        // Original UP should be intact.
        assert( !up1.IsNull() );

        // Function call with a UP
        testfunc( std::move( up1 ) );
        // Move should have cleared this pointer.
        assert( up1.IsNull() );
        tp1.Release();
        assert( tp1.IsNull() );
        assert( 0 == TestPtr::total );

        // Verify comparisons
        UP< TestPtr > up10( new TestPtr( 44, 55 ) );
        UP< TestPtr > up11( new TestPtr( 66, 77 ) );
        TP< TestPtr > tp5 = up10;
        TP< TestPtr > tp6 = up11;
        assert( tp5 == tp5 );
        assert( !( tp5 != tp5 ) );
        assert( tp5 <= tp5 );
        assert( tp5 >= tp5 );
        assert( tp5 != tp6 );
        assert( tp6 != tp5 );
        assert( !( tp5 == tp6 ) );
        if ( tp5.RawAccess() < tp6.RawAccess() )
        {
            assert( tp5 < tp6 );
            assert( tp6 > tp5 );
            assert( tp5 <= tp6 );
            assert( tp6 >= tp5 );
            assert( tp5 < up11 );
            assert( tp6 > up10 );
            assert( up10 < tp6 );
            assert( up11 > tp5 );
            assert( tp5 <= up11 );
            assert( tp6 >= up10 );
            assert( up10 <= tp6 );
            assert( up11 >= tp5 );
        }
        else
        {
            assert( tp6 < tp5 );
            assert( tp5 > tp6 );
            assert( tp6 <= tp5 );
            assert( tp5 >= tp6 );
            assert( tp5 > up11 );
            assert( tp6 < up10 );
            assert( up10 > tp6 );
            assert( up11 < tp5 );
            assert( tp5 >= up11 );
            assert( tp6 <= up10 );
            assert( up10 >= tp6 );
            assert( up11 <= tp5 );
        }
        up10.Delete();
        up11.Delete();


        //*********************** SP Tests *********************************
        // Verify initialized with null pointer.  (Default constructor)
        SP<TestPtr> s0;
        assert( s0.IsNull() );
        assert( 0 == TestPtr::total );

        // Verify raw pointer constructor
        SP<TestPtr> s1( new TestPtr(5, 6) );
        assert( !s1.IsNull() );
        assert( 1 == TestPtr::total );

        // Verify copy constructor.
        SP<TestPtr> s2( s1 );
        assert( !s1.IsNull() );
        assert( !s2.IsNull() );
        assert( s1.UnsafeAccess() == s2.UnsafeAccess() );
        assert( 1 == TestPtr::total );

        // Verify move semantics copy constructor.
        SP<TestPtr> s3( std::move( s2 ) );
        assert( s2.IsNull() );
        assert( !s3.IsNull() );
        assert( s1.UnsafeAccess() == s3.UnsafeAccess() );
        assert( 1 == TestPtr::total );

        // Verify assignment operator - also, check UnsafeAccess().
        SP<TestPtr> s4;
        s4 = s3;
        assert( !s4.IsNull() );
        assert( !s3.IsNull() );
        assert( s4.UnsafeAccess() == s3.UnsafeAccess() );
        assert( 1 == TestPtr::total );

        // Verify move semantics assignment operator.
        SP<TestPtr> s5 = std::move( s4 );
        assert( s4.IsNull() );
        assert( !s5.IsNull() );
        assert( s1.UnsafeAccess() == s5.UnsafeAccess() );
        assert( 1 == TestPtr::total );

        // Verify arrow and * operators.
        s3->a++;
        assert( 6 == s5->a );
        (*s1).b++;
        assert( 7 == (*s5).b );
        assert( 7 == s5->b );

        // Verify assignment from raw pointer.
        SP<TestPtr> s6;
        s6 = new TestPtr( 22, 24 );
        assert( !s6.IsNull() );
        assert( 2 == TestPtr::total );

        // Verify assignment from UP - only from move semantics.
        UP<TestPtr> up4s( new TestPtr( 26, 28 ) );
        assert( 3 == TestPtr::total );
        SP<TestPtr> s7;
#ifdef TEST_COMPILER_ERRORS
        s7 = up4s; // Compile error - can't assign to an SP from a UP without
                   // move semantics (makes clear the UP will end up being null).
#endif
        s7 = std::move( up4s );
        assert( 3 == TestPtr::total );
        assert( 26 == s7->a );
        assert( 28 == (*s7).b );
        assert( up4s.IsNull() );

        // Verify comparisons
        assert( s1 == s3 );
        assert( s1 != s7 );
        assert( s3 <= s1 );
        assert( s3 >= s1 );
        if ( s1.UnsafeAccess() < s7.UnsafeAccess() )
        {
            assert( s1 < s7 );
            assert( s7 > s1 );
            assert( s1 <= s7 );
            assert( s7 >= s1 );
        }
        else
        {
            assert( s7 < s1 );
            assert( s1 > s7 );
            assert( s7 <= s1 );
            assert( s1 >= s7 );
        }

        // Verify Delete
        s7.Delete();
        assert( s7.IsNull() );
        assert( 2 == TestPtr::total );

        // Shares object with s3 and s1
        s5.Delete();
        assert( s5.IsNull() );
        assert( 2 == TestPtr::total );
        s3.Delete();
        assert( s3.IsNull() );
        assert( 2 == TestPtr::total );
        s1.Delete();
        assert( s1.IsNull() );
        assert( 1 == TestPtr::total );
#ifdef TEST_COMPILER_ERRORS
        s5 = tp1; // Compile error - can't make a shared pointer from a non-owning pointer.
#endif

        //*********************** WP Tests *********************************
        WP< TestPtr > w0;
        assert( w0.IsNull() );

        // Construction from SP.
        SP<TestPtr> sw1( new TestPtr( 51, 54 ) );
        WP< TestPtr > w1( sw1 );
        assert( !w1.IsNull() );
        assert( !sw1.IsNull() );
        assert( 2 == TestPtr::total );
        sw1.Delete();
        assert( w1.IsNull() );
        assert( sw1.IsNull() );
        assert( 1 == TestPtr::total );

        // Assignment from SP
        SP<TestPtr> sw2( new TestPtr( 52, 55 ) );
        WP< TestPtr > w2;
        w2 = sw2;
        assert( !w2.IsNull() );
        assert( !sw2.IsNull() );
        assert( 2 == TestPtr::total );
        sw2.Delete();
        assert( w2.IsNull() );
        assert( sw2.IsNull() );
        assert( 1 == TestPtr::total );

        // Construction from another WP
        SP<TestPtr> sw3( new TestPtr( 53, 56 ) );
        WP< TestPtr > w3( sw3 );
        WP< TestPtr > w4( w3 );
        assert( !w3.IsNull() );
        assert( !w4.IsNull() );
        assert( !sw3.IsNull() );
        assert( 2 == TestPtr::total );
        sw3.Delete();
        assert( w3.IsNull() );
        assert( w4.IsNull() );
        assert( sw3.IsNull() );
        assert( 1 == TestPtr::total );

        // Construction with move semantics from another WP
        SP<TestPtr> sw4( new TestPtr( 54, 57 ) );
        WP< TestPtr > w5( sw4 );
        WP< TestPtr > w6( std::move(  w5 ) );
        assert( w5.IsNull() );
        assert( !w6.IsNull() );
        assert( !sw4.IsNull() );
        assert( 2 == TestPtr::total );
        sw4.Delete();
        assert( w5.IsNull() );
        assert( w6.IsNull() );
        assert( sw4.IsNull() );
        assert( 1 == TestPtr::total );

        // Assignment from another WP
        SP<TestPtr> sw5( new TestPtr( 55, 58 ) );
        WP< TestPtr > w7( sw5 );
        WP< TestPtr > w8;
        w8 = w7;
        assert( !w7.IsNull() );
        assert( !w8.IsNull() );
        assert( !sw5.IsNull() );
        assert( 2 == TestPtr::total );
        sw5.Delete();
        assert( w7.IsNull() );
        assert( w8.IsNull() );
        assert( sw5.IsNull() );
        assert( 1 == TestPtr::total );

        // Assignment with move semantics from another WP
        SP<TestPtr> sw6( new TestPtr( 56, 59 ) );
        WP< TestPtr > w9( sw6 );
        WP< TestPtr > w10;
        w10 = std::move( w9 );
        assert( w9.IsNull() );
        assert( !w10.IsNull() );
        assert( !sw6.IsNull() );
        assert( 2 == TestPtr::total );
        sw6.Delete();
        assert( w9.IsNull() );
        assert( w10.IsNull() );
        assert( sw6.IsNull() );
        assert( 1 == TestPtr::total );

        // Get SP from the WP
        SP<TestPtr> sw7( new TestPtr( 57, 60 ) );
        WP< TestPtr > w11( sw7 );
        SP< TestPtr > sptst = w11.GetSP();
        assert( !sw7.IsNull() );
        assert( !w11.IsNull() );
        assert( !sptst.IsNull() );
        assert( 2 == TestPtr::total );
        sw7.Delete();
        assert( sw7.IsNull() );
        assert( !w11.IsNull() );
        assert( !sptst.IsNull() );
        assert( 2 == TestPtr::total );
        w11.Drop();
        assert( sw7.IsNull() );
        assert( w11.IsNull() );
        assert( !sptst.IsNull() );
        assert( 2 == TestPtr::total );
        sptst.Delete();
        assert( sw7.IsNull() );
        assert( w11.IsNull() );
        assert( sptst.IsNull() );
        assert( 1 == TestPtr::total );


    } // End of inner block, causing all pointers to go out of scope.


    // Should be 0 TestPtr objects now.
    assert( 0 == TestPtr::total );

    //*********************** Performance Tests ************************

    // Disable outputs from ctor/dtor
    TestPtr::output = false;

    // UP vs std::unique_ptr
    constexpr int owning_iters = 10000000;

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;

    // unique_ptrs
    std::vector< std::unique_ptr< TestPtr > > uniquePtrs;
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
       uniquePtrs.push_back( std::make_unique< TestPtr >( i, i + 1 ) );
    }
    end = std::chrono::system_clock::now();

    double time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make unique_ptrs = " << time << std::endl;


    start = std::chrono::system_clock::now();
    uniquePtrs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete unique_ptrs = " << time << std::endl;

    // UPs
    std::vector< UP< TestPtr > > UPs;
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
       UPs.push_back( UP<TestPtr>( new TestPtr( i, i + 1 ) ) );
    }
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make UPs = " << time << std::endl;


    start = std::chrono::system_clock::now();
    UPs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete UPs = " << time << std::endl;

    // shared_ptrs - everything is different
    std::vector< std::shared_ptr< TestPtr > > sharedPtrs;
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
        sharedPtrs.push_back( std::make_shared<TestPtr>( i, i + 1 ) );
    }
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make shared_ptrs all different = " << time << std::endl;


    start = std::chrono::system_clock::now();
    sharedPtrs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete shared_ptrs all different = " << time << std::endl;

    // SPs - everything is different
    std::vector< SP< TestPtr > > SPs;
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
       SPs.push_back( SP<TestPtr>( new  TestPtr( i, i + 1 ) ) );
    }
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make SPs all different = " << time << std::endl;


    start = std::chrono::system_clock::now();
    SPs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete SPs all different = " << time << std::endl;


    // shared_ptrs - everything is the same
    std::vector< std::shared_ptr< TestPtr > > sameSharedPtrs;
    std::shared_ptr< TestPtr > mainShared = std::make_shared<TestPtr>( 3, 13 );
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
        sameSharedPtrs.push_back( mainShared );
    }
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make shared_ptrs all the same = " << time << std::endl;


    start = std::chrono::system_clock::now();
    sameSharedPtrs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete shared_ptrs all the same = " << time << std::endl;

    // SPs - everything is the same
    std::vector< SP< TestPtr > > sameSPs;
    SP< TestPtr > mainSP = new TestPtr( 3, 13 );
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < owning_iters; ++i )
    {
        sameSPs.push_back( mainSP );
    }
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to make SPs all the same = " << time << std::endl;


    start = std::chrono::system_clock::now();
    sameSPs.clear();
    end = std::chrono::system_clock::now();

    time = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    std::cout << "Time to delete SPs all the same = " << time << std::endl;


    //********************** Size tests ***************************
    {
        std::cout << "Allocation using unique_ptr" << std::endl;
        std::unique_ptr< TestAlloc > uniqueTA = std::make_unique< TestAlloc >();
        std::cout << "Allocation using shared_ptr" << std::endl;
        std::shared_ptr< TestAlloc > sharedTA = std::make_shared< TestAlloc >();

        std::cout << "Allocation using UP" << std::endl;
        UP< TestAlloc > upTA( new TestAlloc() );
        std::cout << "Allocation using SP" << std::endl;
        SP< TestAlloc > spTA( new TestAlloc() );

        std::cout << "Deleting TestAlloc instances" << std::endl;
    }

    std::cout << "End delete" << std::endl << std::flush;

    return 0;
}
