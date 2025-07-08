# Add library cpp files

if (NOT DEFINED TST_DIR)
    set(TST_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/tst-library")
endif()
if (NOT DEFINED TST_PORT)
    set(TST_PORT "${CMAKE_CURRENT_LIST_DIR}/port/tst-library")
endif()

add_library(tst STATIC)
target_sources(tst PUBLIC
    ${TST_DIR}/src/c/tst_library.c
    ${TST_DIR}/src/c/tst_variables.c
)

# Add include directory
target_include_directories(tst PUBLIC 
    ${TST_DIR}/src/c
    ${TST_PORT}/
)

# Add the standard library to the build
target_link_libraries(tst PUBLIC pico_stdlib)