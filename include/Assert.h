#ifndef ASSERT_H
#define ASSERT_H

#include <cstdio>
#include <cstdlib>

// Diagnostic assertion macro: prints details and aborts on failure.
// In Release builds, writes fatal crash information to CRASH.log before aborting.


#ifdef DEBUG
    #define ASSERT(expr, msg) \
        do { \
            if (!(expr)) { \
                fprintf(stderr, "[ASSERT FAILED]\n"); \
                fprintf(stderr, "Expr: %s\n", #expr); \
                fprintf(stderr, "Msg:  %s\n", msg); \
                fprintf(stderr, "File: %s:%d\n", __FILE__, __LINE__); \
                std::abort(); \
            } \
        } while (0)
#else
    // In Release builds, log fatal assertions to CRASH.log before aborting.
    #define ASSERT(expr, msg) \
        do { \
            if (!(expr)) { \
                FILE* f = fopen("CRASH.log", "a"); \
                if (f) { \
                    fprintf(f, "[FATAL ERROR]\n"); \
                    fprintf(f, "Expr: %s\n", #expr); \
                    fprintf(f, "Msg:  %s\n", msg); \
                    fprintf(f, "File: %s:%d\n\n", __FILE__, __LINE__); \
                    fclose(f); \
                } \
                std::abort(); \
            } \
        } while (0)
#endif

#endif // ASSERT_H
