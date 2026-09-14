#include "mtl_engine.hpp"
#include "autorelease_pool.h"

int main() {
    AutoreleasePoolGuard pool;
        MTLEngine engine;
        engine.init();
        engine.run();
        engine.cleanup();
    
    return 0;
}