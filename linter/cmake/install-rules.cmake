install(
    TARGETS NUTC-linter_exe
    RUNTIME COMPONENT NUTC-linter_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
