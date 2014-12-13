default : all;

run : test;

test : all
	make -C kernel kernel.img
	/usr/bin/time -p expect e0.tcl | tee t1.out
	@cat t1.out | grep -v sgabios > test.out
	
% :
	(make -C kernel $@)
	(make -C user $@)
	(make -C fat439 $@)
