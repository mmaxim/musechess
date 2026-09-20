add_library(chess_eval STATIC
  eval.cc
)

target_include_directories(chess_eval PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_eval PRIVATE chess_movegen)
