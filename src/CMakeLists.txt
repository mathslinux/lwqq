add_executable(lwqq-cli
    cli.c
    )

include_directories(../lib)

target_link_libraries(lwqq-cli
    lwqq
    pthread
    )
