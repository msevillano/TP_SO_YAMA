#!/bin/sh
echo "Instalando/Compilando bibliotecas"

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

echo "Compilando Worker"

cd Worker/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Ejecutando Worker"
cd ..

export LC_ALL=C
#./Debug/Worker

exit 0
