#pragma once

#define EXAMPLE_PROJ_NO_RETURN __attribute__((noreturn))
#define EXAMPLE_PROJ_ALIGNED(x) __attribute__((aligned(x)))

#if defined(__clang__)

#define EXAMPLE_PROJ_NO_OPT __attribute__((optnone))

//
// For unix.Malloc scan-build checkers
//

#define EXAMPLE_PROJ_MALLOC_LIKE_FUNC __attribute__((ownership_returns(malloc)))
//! Note: Second Argument is the argument index for
//! the pointer being free'd. For simplicity, let's assume
//! the pointer is always in the 1st argument
#define EXAMPLE_PROJ_FREE_LIKE_FUNC __attribute__((ownership_takes(malloc, 1)))

//
// For -Wthread-safety analysis
//


//! The Thread Safety Analyzers need an identifier to track things by
//! When using C++, this capability attribute can be bound to a Class
//! For C, we'll just create a dummy variable that is not referenced
//! by actual code so it gets optimized away
typedef int __attribute__((capability("mutex")))
  _ClangThreadSafetyLockReference;
#define INTERNAL_EXAMPLE_PROJ_DECLARE_LOCK_TRACKER(name) \
  extern _ClangThreadSafetyLockReference _##name

//! Flags a function that acquires a resource. In our example
//! we'll want to apply this attribute to flash_lock() and
//! accel_lock()
#define EXAMPLE_PROJ_FUNC_ACQUIRES_LOCK(name) \
  INTERNAL_EXAMPLE_PROJ_DECLARE_LOCK_TRACKER(name); \
  __attribute__((acquire_capability(_##name)))

//! Flags a function that releases a resource. For our example,
//! the accel_unlock() and flash_unlock() functions need this
#define EXAMPLE_PROJ_FUNC_RELEASES_LOCK(name) \
  __attribute__((release_capability(_##name)))

//! Flags a function as requiring a lock be held by the time
//! it is invoked. For example, an "accel_read()" function.
#define EXAMPLE_PROJ_FUNC_REQUIRES_LOCK_HELD(name) \
  __attribute__((requires_capability(_##name)))

//! Disables thread safety checks for a function
//! This is required for the _lock/_unlock functions in our example
//! to prevent False positives.
#define EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS \
  __attribute__((no_thread_safety_analysis))



#elif defined(__GNUC__)

#define EXAMPLE_PROJ_NO_OPT __attribute__((optimize("O0")))

#define EXAMPLE_PROJ_MALLOC_LIKE_FUNC
#define EXAMPLE_PROJ_FREE_LIKE_FUNC


#define EXAMPLE_PROJ_FUNC_ACQUIRES_LOCK(...)
#define EXAMPLE_PROJ_FUNC_RELEASES_LOCK(...)
#define EXAMPLE_PROJ_FUNC_REQUIRES_LOCK_HELD(...)
#define EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS

#else
#  error "Unsupported Compiler"
#endif
