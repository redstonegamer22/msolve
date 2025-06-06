AUTOMAKE_OPTIONS = color-tests
SUBDIRS = src/usolve src/fglm src/neogb src/msolve

AM_CFLAGS	= $(SIMD_FLAGS) $(CPUEXT_FLAGS) $(OPENMP_CFLAGS)
LDADD 		= src/neogb/libneogb.la src/fglm/libfglm.la src/usolve/libusolve.la

bin_PROGRAMS	= msolve
msolve_SOURCES 	= src/msolve/main.c

check_PROGRAMS		= neogb_io \
			  fglm_build_matrixn_radical_shape-31 \
			  fglm_build_matrixn_nonradical_shape-31 \
			  fglm_build_matrixn_nonradical_radicalshape-31 \
			  line_endings_support

checkdiff               = test/diff/diff_cp_d_3_n_4_p_2.sh \
			  test/diff/diff_cyclic5-16.sh \
			  test/diff/diff_cyclic5-31.sh \
			  test/diff/diff_cyclic5-qq.sh \
			  test/diff/diff_eco6-16.sh \
			  test/diff/diff_eco6-31.sh \
			  test/diff/diff_eco6-qq.sh \
			  test/diff/diff_groebner-g2.sh \
			  test/diff/diff_henrion5-qq.sh \
			  test/diff/diff_eco10-31.sh \
			  test/diff/diff_truncate_gb.sh \
			  test/diff/diff_elim-31.sh \
			  test/diff/diff_elim-qq.sh \
			  test/diff/diff_F4SAT-31.sh \
			  test/diff/diff_F4SAT-byone-31.sh \
			  test/diff/diff_kat6-31.sh \
			  test/diff/diff_kat7-qq.sh \
			  test/diff/diff_multy-16.sh \
			  test/diff/diff_multy-31.sh \
			  test/diff/diff_multy-qq.sh \
			  test/diff/diff_nonradical_shape-qq.sh \
			  test/diff/diff_nonradical_radicalshape-qq.sh \
			  test/diff/diff_nonradical_radicalshape-no-square-31.sh \
			  test/diff/diff_nonradical_radicalshape-no-square-qq.sh \
			  test/diff/diff_one-16.sh \
			  test/diff/diff_one-31.sh \
			  test/diff/diff_one-qq.sh \
			  test/diff/diff_radical_shape-31.sh \
			  test/diff/diff_radical_shape-qq.sh \
			  test/diff/diff_realroot-extraction-exact-root.sh \
			  test/diff/diff_reals_dim0.sh \
			  test/diff/diff_realroot1.sh \
			  test/diff/diff_reals_dim0-chgvar.sh \
			  test/diff/diff_reals_dim0-extract.sh \
			  test/diff/diff_reals_dim0-swapvar.sh \
			  test/diff/diff_bug_empty_tracer.sh \
			  test/diff/diff_bug_2nd_prime_bad.sh \
			  test/diff/diff_bug_68.sh \
			  test/diff/diff_mq_2_1.sh \
			  test/diff/diff_xy-qq.sh \
			  test/diff/diff_test-lifting2.sh \
			  test/diff/diff_linear-qq.sh \
			  test/diff/diff_input-overflow-16.sh \
			  test/diff/diff_nf_8.sh \
			  test/diff/diff_nf_16.sh \
			  test/diff/diff_nf_31.sh \
			  test/diff/diff_nf_lm_bug.sh \
			  test/diff/diff_f4sat-field-char.sh \
			  test/diff/diff_f4sat-zero-input.sh \
			  test/diff/diff_f4sat-is-saturated-check.sh \
			  test/diff/diff_maxbitsize-bug.sh

# dist_check_DATA         = test/input_files
neogb_io_SOURCES 	= test/neogb/io/validate_input_data.c
fglm_build_matrixn_radical_shape_31_SOURCES = test/fglm/build_matrixn_radical_shape-31.c
fglm_build_matrixn_nonradical_shape_31_SOURCES = test/fglm/build_matrixn_nonradical_shape-31.c
fglm_build_matrixn_nonradical_radicalshape_31_SOURCES = test/fglm/build_matrixn_nonradical_radicalshape-31.c
line_endings_support_SOURCES = test/msolve/line_endings_support.c

TESTS = $(check_PROGRAMS) $(checkdiff)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = msolve.pc
