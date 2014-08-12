#pragma once

// Patch for std::make_unique in c++11 (should be fixed in c++14)
#if __cplusplus == 201103L
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif
