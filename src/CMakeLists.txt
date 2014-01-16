add_executable(lwqq-cli
    cli.c
    )

include_directories(
    ${PROJECT_BINARY_DIR}
	../lib 
	)

#link_directories(
#    ${CMAKE_CURRENT_BINARY_DIR}/../lib
#    )
if(WIN32)
	target_link_libraries(lwqq-cli
		lwqq-static
		)
else()
	target_link_libraries(lwqq-cli
		lwqq
		)
endif()

if(LWQQ_INSTALL)
INSTALL(TARGETS lwqq-cli DESTINATION bin)
endif()
