#pragma once

#include <iostream>
#include <exception>

#ifndef NDEBUG
#   define print_if_glerror(message) \
    do { \
        if (! (glGetError() == 0)) { \
            std::cerr << "glGetError() == " << glGetError() << " in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define print_if_glerror(message) do { } while (false)
#endif
