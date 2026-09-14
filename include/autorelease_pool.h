#pragma once

// objc_autoreleasePoolPush/Pop are plain C functions from the ObjC runtime,
// so they can be called from a .cpp file without any Objective-C syntax.
// Requires linking libobjc (already linked on macOS whenever you link
// Cocoa/Metal frameworks).
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