// ReSharper disable once CppMissingIncludeGuard

// NO_DISCARD: Throw a warning if the return type is not used
#ifndef NO_DISCARD
#if __cplusplus >= 201703L
#define NO_DISCARD [[nodiscard]]
#else // ELSE: __cplusplus >= 201703L
#define NO_DISCARD // If no compliance is found, we ignore this keyword
#endif // END: __cplusplus >= 201703L
#endif // END: ifndef NO_DISCARD
