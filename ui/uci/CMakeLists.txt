add_library(chess_uci STATIC
  uci.cc
)

target_include_directories(chess_uci PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_uci PUBLIC chess_game chess_movegen chess_search chess_eval)



