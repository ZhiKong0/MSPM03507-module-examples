add_library(line_trace_template OBJECT
    src/line_trace_template.c
)

target_include_directories(line_trace_template PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

option(LINE_TRACE_BUILD_TESTS "Build line trace host mock tests" OFF)

if(LINE_TRACE_BUILD_TESTS)
    add_executable(line_trace_mock_tests
        tests/test_line_trace_mock.c
        $<TARGET_OBJECTS:line_trace_template>
    )

    target_include_directories(line_trace_mock_tests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
    )
endif()
