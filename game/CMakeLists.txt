add_library(chess_game STATIC
  game.cc
)

target_include_directories(chess_game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_game PUBLIC chess_movegen chess_search chess_eval)
