add_executable(lwqq-cli
    cli.c
    )

include_directories(
	${EV_INCLUDE_DIRS}
	../lib 
	)

target_link_libraries(lwqq-cli
    lwqq
    pthread
    )

INSTALL(TARGETS lwqq-cli DESTINATION bin)
