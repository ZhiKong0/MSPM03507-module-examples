add_library(tmx_expansion_board OBJECT
    src/tmx_expansion_board.c
)

target_include_directories(tmx_expansion_board PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

option(TMX_EXPANSION_BUILD_TESTS "Build TMX expansion board host mock tests" OFF)

if(TMX_EXPANSION_BUILD_TESTS)
    add_executable(tmx_expansion_board_mock_tests
        tests/test_tmx_expansion_board_mock.c
        $<TARGET_OBJECTS:tmx_expansion_board>
    )

    target_include_directories(tmx_expansion_board_mock_tests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
    )
endif()
