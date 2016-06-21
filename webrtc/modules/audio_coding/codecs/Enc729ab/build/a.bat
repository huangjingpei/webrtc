make clean
make
make test
cd ..\test
del _test_a.bit
coder TEST.INP _test_a.bit 1
fc /b _test_a.bit TEST_A.BIT
cd ..\build