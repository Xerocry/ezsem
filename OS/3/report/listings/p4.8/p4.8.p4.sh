#!/bin/bash
echo '\n___________________4)_MNOGONITEVOE_FUNKCIONIROVANIE___________________\n'

echo '\n__________p4.1__________\n'
gcc -pthread p4.1.c -o p4.1
timeout 6s ./p4.1
rm p4.1
sleep 1

echo '\n__________p4.2__________\n'
gcc -pthread p4.2.c -o p4.2
timeout 6s ./p4.2
rm p4.2
sleep 1

echo '\n__________p4.3__________\n'
gcc -pthread p4.3.c -o p4.3
timeout 6s ./p4.3
rm p4.3
sleep 1

echo '\n__________p4.4__________\n'
gcc -pthread p4.4.c -o p4.4
timeout 6s ./p4.4
rm p4.4
sleep 1

echo '\n__________p4.6__________\n'
kill -l

echo '\n__________p4.7__________\n'
gcc -pthread p4.7.c -o p4.7
timeout 6s ./p4.7
rm p4.7
sleep 1


