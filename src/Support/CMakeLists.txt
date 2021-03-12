add_library(Support STATIC
        "${MAB_INCLUDE_DIR}/kv/Support/Defer.h"
        "${MAB_INCLUDE_DIR}/kv/Support/Intrinsics.h"
        "${MAB_INCLUDE_DIR}/kv/Support/Memory.h"
        Memory.cpp
        MemoryGlobal.cpp)
