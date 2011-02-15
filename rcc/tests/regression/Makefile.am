.SECONDARY:   # don't delete intermediate .c files

MYTESTFILES = $(wildcard *.r)

EXTRA_DIST = $(MYTESTFILES)

MOSTLYCLEANFILES = $(subst .r,,$(MYTESTFILES)) $(subst .r,.c,$(MYTESTFILES)) compiled.log interpreted.log

check_SCRIPTS = runtests

interpreted:
	../scripts/run-interpreted $(MYTESTFILES) | tee interpreted.log

compiled:
	../scripts/run-compiled -q $(MYTESTFILES) | tee compiled.log

diffs:
	diff --context compiled.log interpreted.log | tee diff.log

runtests: compiled interpreted diffs
