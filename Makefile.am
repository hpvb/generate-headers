AUTOMAKE_OPTIONS = subdir-objects

generate_headers = src/generate-headers
bin_PROGRAMS = $(generate_headers)
src_generate_headers_SOURCES = src/generate-headers.c

man_MANS = man/generate-headers.1

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL)   \
                  $(top_srcdir)/tap-driver.sh

testdir = tests
TESTS_ENVIRONMENT = program=$(generate_headers)      \
                    testdir=$(top_srcdir)/$(testdir)
TESTS = $(testdir)/invalid-input.test                \
        $(testdir)/unistd_32.test                    \
        $(testdir)/unistd_32_unordered.test          \
        $(testdir)/allow-dash.test                   \
        $(testdir)/directory-input.test              \
        $(testdir)/directory-output.test             \
        $(testdir)/append.test                       \
        $(testdir)/table-name.test
