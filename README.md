# SmartPointers
Replacement for the standard library smart pointers in C++.  The intent is to provide better semantics as well as following expected memory allocation operations when a class's new and delete operators are overridden. The rationale and history behind this work can be found in doc/Rationale.docx

# Summary
- `wbs::UP<Type>` is roughly equivelent to `std::unique_ptr<Type>`
  - Can be assigned from a std::unique_ptr
  - Can be assigned with a raw pointer (e.g., using new operator)
- `wbs::SP<Type>` is roughly equivelent to `std::shared_ptr<Type>`
  - Can be assigned using move semantics from a wbs::UP
  - Can be assigned using a raw pointer
  - _Cannot_  be assigned from a std::shared_ptr (incompatible internal implementation)
- `wbs::WP<Type>` is roughly equivelent to `std::weak_ptr<Type>`
  - Obtained from a wbs::SP
- `wbs::TP<Type>` is added to provide explicit non-ownership semantics
  - Can be assigned from a wbs::UP, wbs::SP, or raw pointer.
  - Intended to be used to pass an object by reference, but in no way passes ownership.
  - Should not be used for long-term access.  Inteded use is only within the scope of the called function (SP or WP are better for long-term access).
  
## Detailed Discussion
### General
These variations in these smart pointers vs. the standard library are based on two things: 
- Poor design decisions in the standard library versions, particularly with respect to the std:shared_ptr.
- Stylistic differences regarding semantics and the dogma surrounding the use of the "new" operator.

### Design Issues
The design issues include the way that std::make_shared() allocates memory itself, as opposed to following the standard of using new and delete internally, ostensibly to improve performance by allocating the overhead of a shared pointer along with the object being shared (rather than 2 separate allocations - one for the overhead, one for the object itself).  By following this model, the authors have short-circuited any code enhancement by overriding the new and delete operators for a class.  This can include caching memory allocations to improve performance of frequently allocated and de-allocated objects, use of memory pools for a class (tracking memory usage by the objects of a class, for example), or allocating objects from a particular memory location.  Further, this design decision causes the behavior of std::make_unique to differ from std::make_shared in many cases.  This inconsistancy is non-obvious, and can lead to many issues.

### Style Issues
In general, the following stylistic issues were improved upon from the standard library smart pointers.  Admittedly, some of these are purely personal opinion.
- The template names were too long for something used so often.
- The paranoia around the use of the "new" keyword is unnecessary.  All that matters is that allocated meory be managed by smart pointers.  Note that it is perfectly valid to use std::make_unique with both the UP and SP classes (SPs can be initialized from UPs and unique_ptrs).
- Allocation and deallocation of memory should be associated with the class, not the pointer type.  By managing memory allocation at a class level, significant performance improvements can be made with very little coding cost.  Managing memory allocation at every point of allocation is prohibitively expensive and error-prone.
- The lack of any standard library smart pointer that specifically declares that it does not own the memory to which it points is an oversight.  Adding a wbs::TP class enables semantic clarity that the standard library does not have.

### Specific issues surrounding std::shared_ptr
The std::shared_ptr type, in particular, has several stylistic drawbacks:
- Should be initializable from a std::unique_ptr using move semantics.
- shared_ptr and unique_ptr can (and often do) allocate memory differently, which can lead to unexpected results, performance issues, and/or software bugs.
- Since the shared_ptr combines the internal data structures and the object in a single allocation, this effectively allows weak_ptrs to hold the object's memory, causing what could be considered a memory leak until all weak_ptrs are also released.

wbs::SP fixes these issues:
- Assignment from a wbs::UP (or std::unique_ptr) using move semantics.
- Internally uses a wbs::UP to manage the allocated memory for the object.  This guarantees that SPs and UPs work using the same memory allocation mechanisms as defined by the class itself.
- Once the internal SP reference count goes to zero, the object's memory is released, regardless of the wbs::WPs that may also point to the object.  The additional internal memory that holds the SP count, WP count, and internal UP is still allocated, but that's typically a much smaller footprint being held until all WPs are released.

