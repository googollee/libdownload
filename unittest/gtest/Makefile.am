# Automake file
AUTOMAKE_OPTIONS = foreign

# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/include

# Build rules for libraries.
check_LTLIBRARIES = lib/libgtest.la lib/libgtest_main.la

lib_libgtest_la_SOURCES = src/gtest.cc \
                          src/gtest-death-test.cc \
                          src/gtest-filepath.cc \
                          src/gtest-internal-inl.h \
                          src/gtest-port.cc \
                          src/gtest-test-part.cc \
                          src/gtest-typed-test.cc

lib_libgtest_main_la_SOURCES = src/gtest_main.cc
lib_libgtest_main_la_LIBADD = lib/libgtest.la
