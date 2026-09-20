add_executable(chess_cli
  main.cc
)

target_include_directories(chess_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_cli PRIVATE chess_game chess_movegen)
