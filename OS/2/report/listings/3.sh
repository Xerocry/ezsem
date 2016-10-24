#!/bin/bash

# Если пользователь не указал имя файла, то выходим с ошибкой

filename=$1;
if [ -z $filename ]; then
	exit 1
fi

# Рекурсивно ищем все символьные ссылки на файл в домашнем каталоге пользователя, отсеивая поток ошибок и добавляя полный путь файла

ls $HOME -l -R 2>/dev/null | awk '{if ($0~/^\//) path=substr($0, 0, length($0)); else { if($0~/^l/) $(NF-2)=path"/"$(NF-2); else {$NF=path"/"$NF} print $0} }' | grep '\-> '$1
