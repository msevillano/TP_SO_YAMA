#!/bin/sh

#  mkdir workspace
#  cd workspace
#  git clone https://github.com/sisoputnfrba/tp-2017-2c-Lanzallamas.git
#  cd workspace/tp-2017-2c-Lanzallamas/scripts
#  ./ZScript.sh
#  exit    <-   tenemos que levantar de nuevo la consola para q cargue las variables de entorno.
#  cd workspace/tp-2017-2c-Lanzallamas/DataNode
#  fallocate -l 50M data.bin     <-  cambiar 50 por lo q nos pidan
#  vim dataNode.conf
#  para la pc con 2 nodos{
#                   cd Debug
#                   fallocate -l 50M data.bin     <-  cambiar 50 por lo q nos pidan
#                   vim dataNode.conf
#                   cd ..
#                 }
#  cd ../Worker
#  vim worker.conf
#  para la pc con 2 nodos{
#                   cd Debug
#                   fallocate -l 50M data.bin     <-  cambiar 50 por lo q nos pidan
#                   vim worker.conf
#                   cd ..
#                 }
#  Con esto quedarian seteados los nodos de todas las pcs. 
#  Queda editar la config de cada proceso en particular
#

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

echo "Compilando DataNode"

cd DataNode/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Generando data.bin"

fallocate -l 50M data.bin

cd ..
fallocate -l 50M data.bin

cd ..

echo "Listo DataNode"

sudo apt-get install libreadline6 libreadline6-dev

echo "Compilando FileSystem"

cd FileSystem/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Listo FileSystem"
cd ..
cd ..

echo "Compilando Master"

cd Master/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Listo Master"
cd ..
cd ..

echo "Compilando Worker"

cd Worker/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

mkdir reducciones
mkdir temp
mkdir scripts

echo "Listo Worker"
cd ..
cd ..

echo "Compilando YAMA"

cd YAMA/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug

make clean
make

echo "Listo YAMA"
cd
sed -i '$ a export LD_LIBRARY_PATH=$LIBRARY_PATH:/home/utnso/workspace/tp-2017-2c-Lanzallamas/Z-Commons/Debug' .bashrc
sed -i '$ a export LC_ALL=C' .bashrc


#./Debug/DataNode

. /home/utnso/.bashrc

exit 0
