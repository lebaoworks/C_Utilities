cmake_minimum_required(VERSION 3.10)
project(IPC_TEST)

# find_package(nlohmann_json 3.2.0 REQUIRED)
add_subdirectory(nlohmann_json)
set(JSON_BuildTests OFF CACHE INTERNAL "")

set(executables
    client
    server
    # main
)

file(GLOB SOURCES
    util/*.h
    util/*.c
)

foreach(exe IN LISTS executables)
    add_executable(${exe} ${exe}.cpp ${SOURCES})
    target_include_directories(${exe}
        PRIVATE ${PROJECT_SOURCE_DIR}
    )
    target_link_libraries(${exe}
        PUBLIC nlohmann_json::nlohmann_json
    )
endforeach()