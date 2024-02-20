install(
    TARGETS EXCHANGE_exe
    RUNTIME COMPONENT EXCHANGE_runtime 
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
