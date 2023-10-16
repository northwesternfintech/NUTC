install(
    TARGETS NUTC-client_exe
    RUNTIME COMPONENT NUTC-client_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
