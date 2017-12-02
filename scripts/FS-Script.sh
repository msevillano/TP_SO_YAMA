#!/bin/sh
echo "Instalando/Compilando bibliotecas"

sudo apt-get install libreadline6 libreadline6-dev

git clone "https://github.com/sisoputnfrba/so-commons-library"

cd so-commons-library
sudo make install
cd ..

cd ..
cd Z-Commons/Debug

make clean
make

cd ..
cd ..

echo "Compilando FileSystem"

cd FileSystem/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Ejecutando FileSystem"
cd ..

export LC_ALL=C
#./Debug/FileSystem

exit 0
