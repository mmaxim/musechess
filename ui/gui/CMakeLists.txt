add_executable(chess_gui WIN32
  main.cc
)

target_include_directories(chess_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(chess_gui PRIVATE chess_game chess_movegen chess_search chess_eval)
set_target_properties(chess_gui PROPERTIES
  WIN32_EXECUTABLE TRUE
)
