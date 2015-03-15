# Initialization

AC_INIT([DatABriCxx], [0], [], [databricxx])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror])


# Libtool init and language selection

# Workaround for newer vs. older versions of Automake:
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_CXX

CXX_PROJECT_INIT
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])


# Checks for dependencies

ROOTSYS_DEPS([glibs], [-lSpectrum -lProof -lEve -lRHTTP])

CXX_PROJECT_DEPS([])


# Output

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
	Makefile
	databricxx.pc
	databricxx-config
	src/Makefile
	doc/Makefile doc/Doxyfile
])

AC_OUTPUT
