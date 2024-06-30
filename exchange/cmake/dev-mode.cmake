include(cmake/folders.cmake)

include(CTest)
add_subdirectory(test)
add_subdirectory(benchmark)

add_custom_target(
    run-exe
    COMMAND EXCHANGE_exe 
    VERBATIM
)
add_dependencies(run-exe EXCHANGE_exe)

option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if(BUILD_MCSS_DOCS)
  include(cmake/docs.cmake)
endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)

add_folders(Project)
