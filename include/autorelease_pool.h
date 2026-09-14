#pragma once

extern "C" {
    void* objc_autoreleasePoolPush(void);
    void  objc_autoreleasePoolPop(void* ctx);
}

class AutoreleasePoolGuard {
public:
    AutoreleasePoolGuard()  : ctx_(objc_autoreleasePoolPush()) {}
    ~AutoreleasePoolGuard() { objc_autoreleasePoolPop(ctx_); }
    AutoreleasePoolGuard(const AutoreleasePoolGuard&) = delete;
    AutoreleasePoolGuard& operator=(const AutoreleasePoolGuard&) = delete;
private:
    void* ctx_;
};