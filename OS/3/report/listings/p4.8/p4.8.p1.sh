#!/bin/bash
echo '\n___________________1)_POROJDENIE_I_ZAPUSK_PROCESSOV___________________\n'

echo '\n__________p1.1__________\n'
gcc p1.1.c -o p1.1
./p1.1
rm p1.1
sleep 1

echo '\n__________p1.2__________\n'
gcc p1.2.c -o p1.2
./p1.2
rm p1.2
sleep 1

echo '\n__________p1.3__________\n'
g++ p1.3.c singlecore.c -o p1.3
./p1.3
rm p1.3
sleep 1

echo '\n__________p1.4.fifo__________\n'
g++ -std=c++11 p1.4.cpp singlecore.c -o p1.4
./p1.4 0 64 40 30 20
sleep 1

echo '\n__________p1.4.rr__________\n'
./p1.4 1 74 60 30 20 
sleep 1

echo '\n__________p1.4.other__________\n'
./p1.4 2 0 0 0 0
rm p1.4
sleep 1

echo '\n__________p1.5_p1.6_p1.7__________\n'
gcc child.c -o child
gcc parent.c -o parent
./parent
rm child
rm parent
sleep 1

echo '\n__________p1.8__________\n'
g++ -std=c++11 p1.8.cpp -o p1.8
./p1.8 0
rm p1.8
sleep 1

echo '\n__________p1.9.1__________\n'
gcc p1.9.1.c -o p1.9.1
./p1.9.1
rm p1.9.1
sleep 1

echo '\n__________p1.9.2.c1_p1.9.2.c2_p1.9.2.c3__________\n'
gcc p1.9.2.c1.c -o p1.9.2.c1
gcc p1.9.2.c2.c -o p1.9.2.c2
gcc p1.9.2.c3.c -o p1.9.2.c3
./p1.9.2.c1
./p1.9.2.c2
./p1.9.2.c3
sleep 1

echo '\n__________p1.9.2.p__________\n'
g++ -std=c++11 p1.9.2.p.cpp -o p1.9.2.p
./p1.9.2.p 0
rm p1.9.2.c1
rm p1.9.2.c2
rm p1.9.2.c3
rm p1.9.2.p
sleep 1

echo '\n__________p1.10.1.fifo__________\n'
g++ -std=c++11 p1.10.1.cpp singlecore.c -o p1.10.1
./p1.10.1 0 64 40 30 20
sleep 1

echo '\n__________p1.10.1.rr__________\n'
./p1.10.1 1 74 60 30 20 
sleep 1

echo '\n__________p1.10.1.other__________\n'
./p1.10.1 2 0 0 0 0
rm p1.10.1
sleep 1

echo '\n__________p1.10.2__________\n'
g++ -std=c++11 p1.10.2.cpp singlecore.c -o p1.10.2
./p1.10.2 0 1 0 1
rm p1.10.2
sleep 1

echo '\n__________p1.11__________\n'
g++ -std=c++11 p1.11.cpp singlecore.c -o p1.11
./p1.11
rm p1.11
sleep 1

echo '\n__________p1.11.1__________\n'
g++ -std=c++11 p1.11.1.cpp singlecore.c -o p1.11.1
./p1.11.1
rm p1.11.1
sleep 1

echo '\n__________p1.12__________\n'
g++ p1.12.c singlecore.c -o p1.12
g++ -std=c++11 p1.12.ch.cpp -o p1.12.ch
./p1.12
rm p1.12
rm p1.12.ch
sleep 1







