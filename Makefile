#
# Projekt: ICP
# S�bor:   Makefile - hlavn� a centr�lny
# Autor:   Pavol Dubovec, xdubov02, Juraj Laz�r, xlazur00
# Datum:   15.5.2020
# 
# Pou�itie:
#	- preklad:			make
#	- preklad a spustenie:		run
#	- vytvorenie dokument�cie:	doxygen
#	- zabalenie projektu:		pack
#	- vy�istenie:			clean

#make
make:
	cd src && make -f Makefile

#run
run:
	cd src && make run -f Makefile

#doxygen
doxygen:
	cd src && doxygen dox_info

#pack
pack:
	zip xlazur00-xdubov02.zip -r src -r examples Makefile README.txt

#clear
clean:
	cd src && make clean -f Makefile