all: release

release:
	make -f lib.mak clean
	export DEBUG=0 && make -f lib.mak

debug:
	make -f lib.mak clean
	export DEBUG=2 && make -f lib.mak

profile:
	make -f lib.mak clean
	export DEBUG=1 && make -f lib.mak
	make -f test.mak clean
	export DEBUG=1 && make -f test.mak
	
test:
	make -f test.mak clean
	export DEBUG=2 && make -f test.mak

cvsrar:
	make -f rel.mak clean
	make -f rel.mak

clean: 
	make -f lib.mak clean 	
	make -f rel.mak clean 	
	make -f test.mak clean	