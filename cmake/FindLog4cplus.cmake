#-------------------------------------------------------------------------------------
# Locate Log4cplus library
# This module defines
#    LOG4CPLUS_FOUND, if false, do not try to link to Log4cplus
#    LOG4CPLUS_LIBRARIES
#    LOG4CPLUS_INCLUDE_DIR, where to find log4cplus.hppa
#
# Original script was picked up here 
#    https://github.com/manvelavetisian/cmake-modules/blob/master/FindLog4cplus.cmake
#
#-------------------------------------------------------------------------------------
find_path(LOG4CPLUS_INCLUDE_DIR
  NAMES
    logger.h
  PATH_PREFIXES
    log4cplus
  PATHS
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/csw/include
    /opt/include
    $ENV{LOG4CPLUS_DIR}/include
    $ENV{LOG4CPLUS_ROOT}/include
    ${LOG4CPLUS_DIR}/include
    ${LOG4CPLUS_ROOT}/include
)

if(Log4Cplus_USE_STATIC_LIBS)
    set(log4cplus_postfix "${log4cplus_postfix}S")
endif()
if(Log4Cplus_USE_UNICODE)
    set(log4cplus_postfix "${log4cplus_postfix}U")
endif()

set(LOG4CPLUS_LIB_NAMES_RELEASE "log4cplus${log4cplus_postfix}")

find_library(LOG4CPLUS_LIBRARY_RELEASE
  NAMES
    ${LOG4CPLUS_LIB_NAMES_RELEASE}
  PATHS
    /usr/local
    /usr/lib
    /sw
    /opt/local
    /opt/csw
    /opt
    $ENV{LOG4CPLUS_DIR}/lib
    $ENV{LOG4CPLUS_ROOT}/lib
    ${LOG4CPLUS_DIR}/lib
    ${LOG4CPLUS_ROOT}/lib
  NO_DEFAULT_PATH
)

if(LOG4CPLUS_LIBRARY_RELEASE)
  set(LOG4CPLUS_LIBRARIES optimized ${LOG4CPLUS_LIBRARY_RELEASE} CACHE STRING "Log4cplus Libraries")
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LOG4CPLUS_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cplus DEFAULT_MSG LOG4CPLUS_LIBRARIES LOG4CPLUS_INCLUDE_DIR)

MARK_AS_ADVANCED(LOG4CPLUS_INCLUDE_DIR LOG4CPLUS_LIBRARIES LOG4CPLUS_LIBRARY_RELEASE)
