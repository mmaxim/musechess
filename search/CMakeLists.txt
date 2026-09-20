add_library(chess_search STATIC
  search.cc
)

target_include_directories(chess_search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_search PUBLIC chess_movegen chess_eval)
