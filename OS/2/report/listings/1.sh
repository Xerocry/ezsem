#!/bin/bash

# Если пользователь указал имя результирующего файла, то используем его

filename=$1
if [ -z $filenmame ]; then
	# Если имя файла не указано, то используем имя по умолчанию
	filename="1.out"
fi

# Исполняем команду из предыдущего пункта, перенаправляя вывод в файл

ls / -l -R | awk '{if ($0~/^\//) path=substr($0, 0, length($0)-1); else { if($0~/^1/) $(NF-2)=path"/"$(NF-2); else {$NF=path"/"$NF} print $0} }' | grep -v ^/ | sort -k1.1,1.1 | uniq -w1 > $filename
