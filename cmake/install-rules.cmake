install(
    TARGETS NUTC24_exe
    RUNTIME COMPONENT NUTC24_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
