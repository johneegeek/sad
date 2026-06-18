include_guard()

message(STATUS "Collecting build stats...")

if (VERSION_INCLUDE_BUILDID)
    string(TIMESTAMP BUILD_TIMESTAMP +%y%j%H)
endif()

string(TIMESTAMP BUILD_DATE "%b %d, %Y")

if (PROJECT_USES_GIT)
    execute_process(COMMAND git log -1 --pretty=format:%h
                    OUTPUT_VARIABLE GIT_SHA
    )
endif()