add_library(line_trace_template OBJECT
    src/line_trace_template.c
)

target_include_directories(line_trace_template PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)
