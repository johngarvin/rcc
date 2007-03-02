.SECONDARY:   # don't delete intermediate .c files

MYTESTFILES = $(wildcard *.r)

EXTRA_DIST = run-test.in $(MYTESTFILES)

MOSTLYCLEANFILES = $(subst .r,,$(MYTESTFILES)) $(subst .r,.c,$(MYTESTFILES)) compiled.log interpreted.log

check_SCRIPTS = runtests

run-compiled: run-compiled.in

run-interpreted: run-interpreted.in

interpreted:
	./run-interpreted $(MYTESTFILES) | tee interpreted.log

compiled:
	./run-compiled $(MYTESTFILES) | tee compiled.log

diffs:
	diff --context compiled.log interpreted.log | tee diff.log

runtests: compiled interpreted diffs
