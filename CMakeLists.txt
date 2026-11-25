cmake_minimum_required(VERSION 3.16)
project(PerfectPlayoffPredictions LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/FantasySportsApplication)

# Core logic shared by the app and the tests.
add_library(ppp_core STATIC
  ${SRC}/Team/Team.cpp
  ${SRC}/BST/TeamTree.cpp
  ${SRC}/IO/DataIO.cpp
  ${SRC}/Season/Standings.cpp
  ${SRC}/Season/Seeding.cpp
  ${SRC}/IO/Csv.cpp
  ${SRC}/Net/HttpClient.cpp
  ${SRC}/Nflverse/Franchises.cpp
  ${SRC}/Nflverse/NflverseData.cpp
  ${SRC}/Nflverse/DataSource.cpp
)
target_include_directories(ppp_core PUBLIC ${SRC})

if(MSVC)
  target_compile_options(ppp_core PUBLIC /W4 /permissive-)
else()
  target_compile_options(ppp_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

add_executable(ppp
  ${SRC}/main.cpp
  ${SRC}/App.cpp
  ${SRC}/UI/Console.cpp
)
target_link_libraries(ppp PRIVATE ppp_core)

include(CTest)
if(BUILD_TESTING)
  foreach(test_name SeedingTests HashTableTests TeamTreeTests DataIOTests TeamTests CsvTests NflverseTests)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE ppp_core)
    add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
endif()
