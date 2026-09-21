cmake_minimum_required(VERSION 3.20)
project(chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(movegen)
add_subdirectory(eval)
add_subdirectory(search)
add_subdirectory(game)
add_subdirectory(ui)

# --- Executables moved to top level to land in build root ---
add_executable(chess-demo movegen/main.cc)
target_link_libraries(chess-demo PRIVATE chess_movegen)

add_executable(chess_cli ui/cli/main.cc)
target_include_directories(chess_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_cli PRIVATE chess_game chess_movegen)

add_executable(uci ui/uci/main.cc)
target_link_libraries(uci PRIVATE chess_uci)

# --- Tests (GoogleTest, fetched at configure time) ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests
  movegen/board_test.cc
  movegen/move_test.cc
  movegen/movegen_test.cc
  search/search_test.cc
  game/game_test.cc
  ui/uci/uci_test.cc
  eval/eval_test.cc)
target_link_libraries(tests PRIVATE chess_movegen chess_eval chess_search chess_game chess_uci GTest::gtest GTest::gtest_main)

enable_testing()
add_test(NAME tests COMMAND tests)
