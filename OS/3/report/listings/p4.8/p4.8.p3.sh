#!/bin/bash
echo '\n___________________3)_UPRAVLENIE_PROCESSAMI_POSREDSTVOM_SIGNALOV___________________\n'

echo '\n__________p3.1.k__________\n'
kill -l
sleep 1

: <<'p3.1.1'
echo '\n__________p3.1.1__________\n'
gcc p3.1.p.c -o p3.1.0
gcc p3.1.1.ch.c -o p3.1.1.ch
gcc p3.1.2.ch.c -o p3.1.2.ch
gcc p3.1.3.ch.c -o p3.1.3.ch
./p3.1.0
sleep 4
rm p3.1.0
rm p3.1.1.ch
rm p3.1.2.ch
rm p3.1.3.ch
sleep 1
p3.1.1

echo '\n__________p3.3__________\n'
gcc p3.3.c -o p3.3
cat p3.3.txt & ./p3.3 & sh p3.3.sh &
rm p3.3
sleep 1

echo '\n__________p3.4.1.1__________\n'
gcc p3.4.1.c -o p3.4.1
nice -5 ./p3.4.1 & nice -5 ./p3.4.1
sleep 1

echo '\n__________p3.4.1.2__________\n'
nice -15 ./p3.4.1 & nice -1 ./p3.4.1
rm p3.4.1
sleep 1

echo '\n__________p3.4.2__________\n'
g++ -std=c++11 p3.4.2.cpp -o p3.4.2
./p3.4.2
rm p3.4.2

echo '\n__________p3.6__________\n'
whoami
id -u nikita
id -u root
ps -Al

