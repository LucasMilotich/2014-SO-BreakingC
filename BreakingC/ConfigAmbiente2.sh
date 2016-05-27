#Pasos para ejecutar el script:
echo "Correr esto antes de ejecutar el script
Paso 1:
export PATH="$ PATH:/home/utnso/Escritorio"
Paso 2:
Luego correr asi:  . ./ConfigAmbiente.sh "

#cd /home/utnso
#git clone https://github.com/sisoputnfrba/tp-2014-1c-breaking-c.git

#Libraries
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/utnso/tp-2014-1c-breaking-c/BreakingC/Libraries/Debug

#UMV
export UMVCFG=/home/utnso/tp-2014-1c-breaking-c/BreakingC/UMV/Config.txt

#Programa
export ANSISOP_CONFIG=/home/utnso/tp-2014-1c-breaking-c/BreakingC/Programa/Config.txt

#CPU
export CONFIG=/home/utnso/tp-2014-1c-breaking-c/BreakingC/CPU/Config.txt

#KERNEL
export CONFIG_KERNEL=/home/utnso/tp-2014-1c-breaking-c/BreakingC/Kernel/Config_Kernel.txt

#cd /home/utnso/;ls workspace || mkdir workspace
#cd workspace
#git clone https://github.com/sisoputnfrba/ansisop-parser.git

#echo "DESCARGA DEL PARSER COMPLETA"
#echo "PROCEDEMOS A INSTALARLA"
#cd ansisop-parser/parser
#sudo make all
#sudo make install

#cd /home/utnso/workspace
#git clone https://github.com/sisoputnfrba/so-commons-library.git

echo "DESCARGA DE LAS COMMONS COMPLETA"
echo "PROCEDEMOS A INSTALARLAS"
#cd so-commons-library
#sudo make all
#sudo make install

