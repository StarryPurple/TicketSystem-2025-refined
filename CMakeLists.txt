cmake_minimum_required(VERSION 3.24)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

# enable_testing()

add_subdirectory(template)
add_subdirectory(include)
add_subdirectory(src)
# add_subdirectory(test)

add_executable(code main.cpp)
add_executable(tester test.cpp)

target_link_libraries(code PRIVATE IncludeModule SrcModule)
target_link_libraries(tester PRIVATE IncludeModule SrcModule)