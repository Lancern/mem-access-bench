add_library(Json INTERFACE)
target_sources(Json INTERFACE
        "${MAB_INCLUDE_DIR}/kv/Json/JsonException.h"
        "${MAB_INCLUDE_DIR}/kv/Json/JsonObject.h"
        "${MAB_INCLUDE_DIR}/kv/Json/JsonSerializer.h")
target_link_libraries(Json
        INTERFACE Support)
