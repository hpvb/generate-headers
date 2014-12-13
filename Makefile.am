AUTOMAKE_OPTIONS = subdir-objects

generate_headers = src/generate-headers
bin_PROGRAMS = $(generate_headers)
src_generate_headers_SOURCES = src/generate-headers.c

man_MANS = man/generate-headers.1

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/tap-driver.sh

TESTS_ENVIRONMENT = program=$(generate_headers)
TESTS = tests/invalid-input.test
