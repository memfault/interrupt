cmake_minimum_required(VERSION 3.16)

project(example3 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11) # Using C11
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(${CMAKE_CURRENT_LIST_DIR}/../lua.cmake)

add_executable(${PROJECT_NAME}
    ${LUA_SOURCES}
    # Local
    swig.c
    example3.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:CONFIG_LUA_MAXSTACK=8192> # 8KB
    $<$<COMPILE_LANGUAGE:C,CXX>:CONFIG_LUA_ROOT="./">
    $<$<COMPILE_LANGUAGE:C,CXX>:LUA_HEAP_SIZE=262144> # 256KB
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:${LUA_INCLUDES}>
    $<$<COMPILE_LANGUAGE:C,CXX>:${CMAKE_CURRENT_LIST_DIR}/>
)

add_compile_options(-Wall -Werror)
