#!/bin/bash
echo '\n___________________2)_VZAIMODEYSTVIYE_RODSTVENNIH_PROCESSOV___________________\n'

echo '\n__________p2.1.1__________\n'
gcc p2.1.1.c -o p2.1.1
gcc p2.1.1.ch.c -o p2.1.1.ch
./p2.1.1
sleep 1

echo '\n__________p2.1.2__________\n'
gcc p2.1.2.c -o p2.1.2
./p2.1.2
sleep 3

echo '\n__________p2.1.3__________\n'
gcc p2.1.3.c -o p2.1.3
./p2.1.3
sleep 1

echo '\n__________p2.2__________\n'
sh p2.2.sh
rm p2.1.1
rm p2.1.1.ch
rm p2.1.2
rm p2.1.3
sleep 1

