set -e
trap 'kill -HUP 0' 0


for N in {100,500,1000,5000,10000}000; do
    echo XXXXXXXXXX N=$N;
    for M2 in 1 2 4 8 16 32 64 128; do
	echo M2=$M2;
	g++ -O3 -DM2=$M2 -DN=$N test_LUT.cpp table_lut.cpp -std=c++11
	./a.out
    done
done
