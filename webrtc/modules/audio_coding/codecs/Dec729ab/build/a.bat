make clean
make
make test
cd ..\test
del _test_a.syn
decoder TEST_A.BIT _test_a.syn
fc /b _test_a.syn TEST_A.SYN
cd ..\build