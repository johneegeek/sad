
set (sad_SRCS
		binstr.c
		dprintf.c
		getopt.c
		makeargs.c
		sad.c
		util.cpp
	)

include_directories (
        ${sad_SOURCE_DIR}
    )	

if (WATCOM)
	#set(CMAKE_C_FLAGS_RELEASE "-bc -bm -6s -otaxl+ -dNDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-bc -bm -ot -6r -zlf -dNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -xs")
	#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
	set(CMAKE_CXX_LINK_EXECUTABLE "wlink  option quiet name '<TARGET_UNQUOTED>' <LINK_FLAGS> option caseexact file {<OBJECTS>}")
#	MESSAGE( STATUS "CMAKE_C_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE} )
#	MESSAGE( STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE} )
#	MESSAGE( STATUS "CMAKE_CXX_LINK_EXECUTABLE: " ${CMAKE_CXX_LINK_EXECUTABLE} )
endif(WATCOM)	
	
add_executable(sad ${sad_SRCS})

# Set the install target
# You need to set the Env Varable SAD_INSTALLDIR to where you want to install it
# prior to calling CMake
#    For windows it usually `set SAD_INSTALLDIR=C:\\ST32\\bin`
#    In linux it usually is `export SAD_INSTALLDIR=/usr/local/bin`
set (InstDirSet $ENV{SAD_INSTALLDIR})
if (InstDirSet)
	install(TARGETS sad DESTINATION $ENV{SAD_INSTALLDIR})
else ()
	install(TARGETS sad DESTINATION /usr/local/bin)
endif (InstDirSet)
	
	