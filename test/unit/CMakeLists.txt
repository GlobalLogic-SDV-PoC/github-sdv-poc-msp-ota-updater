set(test_sources main.tst.cpp)
set(bin_name ${PROJECT_NAME}_unit_tests)

add_executable(${bin_name} ${test_sources})
target_link_libraries(${bin_name} PRIVATE ${PROJECT_NAME} gmock gtest)

if(UPDATER_ENABLE_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_xml(
        NAME
        ${PROJECT_NAME}_coverage
        EXECUTABLE
        ${bin_name}
        EXCLUDE
        ${PROJECT_SOURCE_DIR}/ipc
        ${PROJECT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR})
endif()
