#
# Projekt: ICP
# Súbor:   Makefile - hlavný a centrálny
# Autor:   Pavol Dubovec, xdubov02, Juraj Lazúr, xlazur00
# Datum:   15.5.2020
# 
# Použitie:
#	- preklad:			make
#	- preklad a spustenie:		run
#	- vytvorenie dokumentácie:	doxygen
#	- zabalenie projektu:		pack
#	- vyèistenie:			clean

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