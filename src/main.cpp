
#include "mtl_engine.hpp"
#include "autorelease_pool.h"

int main() {
    AutoreleasePoolGuard AutoreleasePoolGuard;
        MTLEngine engine;
        engine.init();
        engine.run();
        engine.cleanup();
    
    return 0;
}