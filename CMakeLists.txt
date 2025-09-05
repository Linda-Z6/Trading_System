# CMakeLists.txt
cmake_minimum_required(VERSION 3.24)

project(databento_historical_example)
include(FetchContent)

FetchContent_Declare(
  databento
  GIT_REPOSITORY https://github.com/databento/databento-cpp
  GIT_TAG HEAD
)
FetchContent_MakeAvailable(databento)

# add_executable(example main.cpp)
# target_link_libraries(example PRIVATE databento::databento)

add_executable(example
    src/main.cpp
    src/feed_handler.cpp
)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(example PRIVATE databento::databento)


