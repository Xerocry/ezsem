#!/bin/bash

filename=$1
dir=$HOME/$filename.txt
command=s

# prompt="$(whoami)@$(hostname): $(/bin/pwd | sed "s|$HOME|~|") $"

> $dir

echo "Log to $dir enabled, old content removed"
echo "Execute "q" or press ^C to log off" 

while [ "$command" != q ]
do
	printf "$(whoami)@$(hostname): $(/bin/pwd | sed "s|$HOME|~|") $ "
	read command
	ifcd="$(echo "$command" | cut -f 1 -d " ")"
	if [ "$ifcd" = "cd" ];
	then
		path=$(echo "$command" | sed "s/cd //")
		echo "$(whoami)@$(hostname): $(/bin/pwd | sed "s|$HOME|~|") $ cd $path" >> $dir
		cd $path 
	else
		echo "$(whoami)@$(hostname): $(/bin/pwd | sed "s|$HOME|~|") $ $command" >> $dir
		result=$($command)
		echo "$result" >> $dir
		echo "$result"
	fi
done

echo "Log closed"
