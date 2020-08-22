# Install script for directory: C:/Users/juhan/Downloads/pcre-master/pcre-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/PCRE")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcred.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcre.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcre.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcre.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcreposixd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcreposix.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcreposix.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcreposix.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcrecppd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcrecpp.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcrecpp.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcrecpp.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcregrep.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcregrep.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcregrep.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcregrep.exe")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcretest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcretest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcretest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcretest.exe")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcrecpp_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcrecpp_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcrecpp_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcrecpp_unittest.exe")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcre_scanner_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcre_scanner_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcre_scanner_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcre_scanner_unittest.exe")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Debug/pcre_stringpiece_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/Release/pcre_stringpiece_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/MinSizeRel/pcre_stringpiece_unittest.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/RelWithDebInfo/pcre_stringpiece_unittest.exe")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/pcre.h"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/pcreposix.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/pcrecpp.h"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/pcre_scanner.h"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/pcrecpparg.h"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/pcre_stringpiece.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1" TYPE FILE FILES
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre-config.1"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcregrep.1"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcretest.1"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man3" TYPE FILE FILES
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre16.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre32.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_assign_jit_stack.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_compile.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_compile2.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_config.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_copy_named_substring.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_copy_substring.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_dfa_exec.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_exec.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_free_study.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_free_substring.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_free_substring_list.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_fullinfo.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_get_named_substring.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_get_stringnumber.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_get_stringtable_entries.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_get_substring.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_get_substring_list.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_jit_exec.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_jit_stack_alloc.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_jit_stack_free.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_maketables.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_pattern_to_host_byte_order.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_refcount.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_study.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_utf16_to_host_byte_order.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_utf32_to_host_byte_order.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcre_version.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcreapi.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrebuild.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrecallout.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrecompat.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrecpp.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcredemo.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrejit.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrelimits.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrematching.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrepartial.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrepattern.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcreperform.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcreposix.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcreprecompile.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcresample.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcrestack.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcresyntax.3"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/pcreunicode.3"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/pcre/html" TYPE FILE FILES
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/index.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre-config.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre16.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre32.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_assign_jit_stack.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_compile.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_compile2.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_config.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_copy_named_substring.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_copy_substring.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_dfa_exec.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_exec.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_free_study.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_free_substring.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_free_substring_list.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_fullinfo.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_get_named_substring.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_get_stringnumber.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_get_stringtable_entries.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_get_substring.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_get_substring_list.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_jit_exec.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_jit_stack_alloc.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_jit_stack_free.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_maketables.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_pattern_to_host_byte_order.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_refcount.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_study.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_utf16_to_host_byte_order.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_utf32_to_host_byte_order.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcre_version.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcreapi.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrebuild.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrecallout.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrecompat.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrecpp.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcredemo.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcregrep.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrejit.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrelimits.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrematching.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrepartial.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrepattern.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcreperform.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcreposix.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcreprecompile.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcresample.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcrestack.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcresyntax.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcretest.html"
    "C:/Users/juhan/Downloads/pcre-master/pcre-master/doc/html/pcreunicode.html"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/juhan/Downloads/pcre-master/pcre-master/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
