# ICP

[ICP - Seminář C++](https://www.fit.vut.cz/study/course/230965/.cs)

Tento repozitár obsahuje kompletné riešenie projektu z predmetu ICP. Aplikácia simuluje mapu a pohyb vozidiel v rámci systému hromadnej dopravy z používateľom dodaných dát. V rámci riešenia je priložená sada vzorových súborov spolu s popisom požadovaného formátu.

Obrázky a ikony vozidiel prevzaté z internetovej stánky [imhd.sk](https://imhd.sk/za/mhd).

[ICP - The C++ Programming Language](https://www.fit.vut.cz/study/course/230965/.en)

This repository contains the complete project solution from the ICP course. The application simulates the map and the movement of vehicles within the public transport system from user-supplied data. A set of sample files is included with the solution, along with a description of the required format.

Images and vehicle icons taken from the website [imhd.sk](https://imhd.sk/za/mhd).

## Preklad a spustenie

 - preklad: `make`
 - preklad a spustenie: `make run`
 - vytvorenie dokumentácie: `make doxygen`
 - kompresia projektu pre odovzdanie: `make pack`
 - vyčistenie spustiteľných súborov: `make clean`

Príklad spustenia preloženého programu:

    ./icp -map ../examples/suradnice.json -stops ../examples/zastavka.json -lines ../examples/linky.json -busses ../examples/autobusy.json

## Make and run program

 - translation: `make`
 - translation and execution: `make run`
 - documentation creation: `make doxygen`
 - project compression for commit: `make pack`
 - clean up executables: `make clean`

Example of running a compiled program:

    ./icp -map ../examples/coordinates.json -stops ../examples/stops.json -lines ../examples/lines.json -busses ../examples/autobuses.json
