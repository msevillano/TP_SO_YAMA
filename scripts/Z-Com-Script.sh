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
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/Z-Commons/Debug

exit 0
