add_executable(Sandbox Src/Sandbox.cpp)
target_link_libraries(Sandbox PRIVATE BurraqEngine)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /W3)
    set_property(TARGET Sandbox PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

find_program(GLSLANG_VALIDATOR glslangValidator
    HINTS $ENV{VULKAN_SDK}/Bin
)
if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found. Ensure Vulkan SDK is installed.")
endif()

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders")

file(GLOB SHADER_SOURCES
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
)

set(SHADER_SPV_OUTPUTS "")
foreach(SHADER_SRC ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_SRC} NAME)
    set(SHADER_SPV "${SHADER_SOURCE_DIR}/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT "${SHADER_SPV}"
        COMMAND ${GLSLANG_VALIDATOR} -V "${SHADER_SRC}" -o "${SHADER_SPV}"
        DEPENDS "${SHADER_SRC}"
        COMMENT "Compiling shader: ${SHADER_NAME}"
        VERBATIM
    )
    list(APPEND SHADER_SPV_OUTPUTS "${SHADER_SPV}")
endforeach()

add_custom_target(CompileShaders DEPENDS ${SHADER_SPV_OUTPUTS})
add_dependencies(Sandbox CompileShaders)

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
        "$<TARGET_FILE_DIR:Sandbox>/Resources"
    COMMENT "Copying Resources to output directory"
)

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/Engine/ThirdParty/GLFW/lib/glfw3.dll"
        "$<TARGET_FILE_DIR:Sandbox>"
    COMMENT "Copying glfw3.dll to output directory"
)
