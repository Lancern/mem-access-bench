find_package(GTest REQUIRED)

function(add_mab_test name)
    set(target_name "Test${name}")
    add_executable("${target_name}" ${ARGN})
    target_link_libraries("${target_name}"
            PRIVATE GTest::GTest GTest::Main "${name}")
    add_test(NAME "${target_name}" COMMAND "${target_name}")
endfunction()

add_subdirectory(Support)
