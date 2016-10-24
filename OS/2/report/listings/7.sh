#!/bin/bash

# Создаем папку
mkdir tempfolder

# Фиксируем в переменной первоначальный размер пустой папки (4096 для текущей ОС)
defaultsize=$(ls -l -d tempfolder | cut -d ' ' -f5)
currentsize=$defaultsize
index=0

# Продолжаем цикл, пока размер папки не изменится
while [ "$defaultsize" -eq "$currentsize" ]
do
	# Создаем новые папки
	mkdir ./tempfolder/$index
	# Инкрементируем счетчик
	index=$((index+1))
	# Фиксируем в переменной размер заполняющейся папки
	currentsize=$(ls -l -d tempfolder | cut -d ' ' -f5)
done

# Удаляем папку
rm -rf tempfolder

echo "Count of new directories to change size $index"

