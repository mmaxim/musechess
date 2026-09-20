add_library(chess_search INTERFACE)

target_include_directories(chess_search INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_search INTERFACE chess_movegen chess_eval)
