#
# Projekt: ICP
# Súbor: Makefile - pre zdrojové súbory
# Autor:   Pavol Dubovec, xdubov02, Juraj Lazúr, xlazur00
# Datum:   15.5.2020
# 
# Použitie:
#	- preklad:		make
#	- preklad a spustenie:	run
#	- vyèistenie:		clean

make: run_qmake
run: run_qmake run_prog

#qmake_compile makefile
run_qmake: icp.pro
	qmake -makefile -o Qt_makefile icp.pro
	make -f Qt_makefile

run_prog: icp
	./icp -map ../examples/suradnice.json -stops ../examples/zastavka.json -lines ../examples/linky.json -busses ../examples/autobusy.json

#clean
clean:
	rm -f *.o
	rm icp
	rm Qt_makefile
	rm moc_mainwindow.cpp
	rm moc_mapscene.cpp
	rm moc_predefs.h
	rm ui_mainwindow.h