add_library(chess_movegen STATIC
  board.cc
  movegen.cc)
target_include_directories(chess_movegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(chess-demo main.cc)
target_link_libraries(chess-demo PRIVATE chess_movegen)
