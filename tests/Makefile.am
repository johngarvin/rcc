
EXTRA_DIST = \
	sep_def.r			\
	sep_use.r			\
	test00_constants.r		\
	test01_fundef_in_loop.r		\
	test02_defun.r			\
	test03_arrayindexing.r		\
	test04_for.r			\
	test05_vectorop.r		\
	test06_myrgamma.r		\
	test07_if_nestedarrayindex.r	\
	test08_closure.r		\
	test09_memory.r			\
	test10_defun_after.r		\
	test11_env.r			\
	test12_multireturn.r		\
	test13_return.r			\
	test14_samename.r		\
	test15_source.r test_foo_15.r	\
	test16_usemethod.r		\
	test17_rfunc.r			\
	test18_ellipsis.r		\
	test19_scope.r			\
	test20_if.r			\
	test21_if_else.r		\
	test22_expensive_fn.r		\
	test23_scope.r			\
	test24_assign_array_ss.r	\
	test25_recursion.r		\
	test26_return_fn.r		\
	test27_fundef_scope.r		\
	test28_fundef_scope.r		\
	test29_fib.r			\
	test30_scoping.r		\
	test31_array.r			\
	test32_fun_redef.r		\
	test33_partial_array.r		\
	test34_redef.r			\
	test35_change_env.r		\
	test36_change_formals.r		\
	test37_change_body.r

.r.c:
	../src/rcc $<

TEST_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib

.c.so: ../lib/.libs/librcc.so
	$(CC) $(CPPFLAGS) $(TEST_CFLAGS) $(LDFLAGS) -fpic -shared ../lib/.libs/librcc.so  $< -o $@

check: ../src/rcc ../lib/.libs/librcc.so
	./run-test
