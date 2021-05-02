# boris media Makefile


.PHONY: goal ilibs \
		EXE_borisa \
		EXE_juicebox \
		all_execs


goal: ilibs all_execs


all_execs: ilibs juicebox

juicebox: juicebox/src/juicebox
juicebox/src/juicebox:
	cd juicebox/src && make


ilibs: .info


.info:
	cd shlibs ; ln -s ../../../iprog/iprog_ux/ilibs 2> ../.info

