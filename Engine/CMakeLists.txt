file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/BRQ/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/BRQ/*.h"
)
list(APPEND ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Src/Engine.h")
list(APPEND ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SPIR-V-Reflect/spirv_reflect.cpp"
)

add_library(BurraqEngine STATIC ${ENGINE_SOURCES})

target_include_directories(BurraqEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/BRQ"
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLM/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SPIR-V-Reflect/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/STBI/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/meshoptimizer/include"
)

target_precompile_headers(BurraqEngine PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Src/BRQ/BRQ.h>"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SPIR-V-Reflect/spirv_reflect.cpp"
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

target_link_libraries(BurraqEngine PUBLIC Vulkan::Vulkan)

add_library(glfw SHARED IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/lib/glfw3dll.lib"
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/lib/glfw3.dll"
)
target_link_libraries(BurraqEngine PUBLIC glfw)

add_library(meshoptimizer STATIC IMPORTED)
set_target_properties(meshoptimizer PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/meshoptimizer/lib/meshoptimizer.lib"
)
target_link_libraries(BurraqEngine PRIVATE meshoptimizer)

target_compile_definitions(BurraqEngine PUBLIC
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:BRQ_PLATFORM_WINDOWS>
    $<$<CONFIG:Debug>:BRQ_DEBUG>
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:BRQ_RELEASE>
    $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
    target_compile_options(BurraqEngine PRIVATE
        /W3
        /wd26812 /wd26495 /wd26451
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/GF>
        $<$<CONFIG:Release>:/Gy>
        $<$<CONFIG:Release>:/fp:fast>
    )
    set_property(TARGET BurraqEngine PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()
