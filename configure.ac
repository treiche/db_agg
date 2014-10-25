AC_INIT([db_agg], [0.9.2], [arnd@brusdeilins.net])

AC_CONFIG_HEADERS([config.h:config.h.in])
AC_CHECK_HEADERS([curses.h])

AC_PROG_CXX
AC_LANG(C++)

AX_CXX_COMPILE_STDCXX_11(noext, mandatory)

function fail {
    echo "configuration failed. missing dependency: $1 '$2'"
    exit 1
}

AC_CHECK_HEADERS([pthread.h gtest/gtest.h jansson.h pcre.h zip.h curses.h log4cplus/logger.h dlfcn.h],[],[fail header $ac_header])

AC_CHECK_LIB(zip,zip_open,[],[fail library zip])
AC_CHECK_LIB(ncursesw,initscr,[],[fail library ncurses])
AC_CHECK_LIB(jansson,json_loads,[],[fail library jansson])
AC_CHECK_LIB(dl,dlopen,[],[fail library dl])
AC_CHECK_LIB(xml2,xmlCtxtReadFile,[],[fail library libxml2])
AC_CHECK_LIB(pcre,pcre_compile,[],[fail library pcre])
AC_CHECK_LIB(log4cplus,_ZN9log4cplus20PropertyConfigurator11doConfigureERKSsRNS_9HierarchyEj,[],[fail library liblog4cplus])
AC_CHECK_LIB(pq,PQconnectdb,[],[fail library libpq])
AC_CHECK_LIB(pthread,pthread_create,[],[fail library pthread])

AC_PROG_INSTALL

AC_CHECK_PROGS(XML2_CONFIG, xml2-config)
if test "x$XML2_CONFIG" = x; then :
    fail program xml2-config
fi

AC_CHECK_PROGS(PG_CONFIG, pg_config)
if test "x$PG_CONFIG" = x; then :
    fail program pg_config
fi

AC_CHECK_PROGS(CXXTESTGEN, cxxtestgen)
if test "x$CXXTESTGEN" = x; then :
    fail program cxxtestgen
fi

AC_CHECK_PROGS(ASCIIDOC, asciidoc)

AS_VAR_APPEND([INCLUDES],`xml2-config --cflags`)
AS_VAR_APPEND([INCLUDES]," -I`pg_config --includedir`")

AC_SUBST(INCLUDES)
AC_OUTPUT(Makefile src/installation.h)

