================================================================================================

Autori: Juraj Lazúr, Pavol Dubovec
Loginy: xlazur00, xdubov02

================================================================================================

Súbor: suradnice.json
Popis: Súbor obsahujúci dáta o krivkách, prípadne mnohouholníkoch vo formáte JSON,
       potrebných na vykreslenie.
Formát obsahu súboru je nasledovný:
Ulice:
{
    "názov_ulice" : [
        {
             "x1" : Súradnica x prvého bodu úsečky názov_ulice,
             "y1" : Súradnica y prvého bodu úsečky názov_ulice,
             "x2" : Súradnica x druhého bodu úsečky názov_ulice,
             "y2" : Súradnica y druhého bodu úsečky názov_ulice,
             "typ" : Typ krivky: 0 - cesta autobusu, 1 - cesta autobus/trolejbus,
                                 2 - lano lanovky, 3 - železničná trať
        }(,)Blok je možné opakovať viacnásobne. Predlžuje danú komunikáciu.
    ](,)Blok je možné opakovať viacnásobne. Pridáva inú ulicu.
}
Polygóny pozadia:
"názov_pozadia" : [
     {
          "x1" : Súradnica x prvého bodu polygónu názov_pozadia,
          "y1" : Súradnica y prvého bodu polygónu názov_pozadia,
          "x2" : Súradnica x druhého bodu polygónu názov_pozadia,
          "y2" : Súradnica y druhého bodu polygónu názov_pozadia,
             ...
             ...
             ...
          "typ" : 4 - vodná plocha, 5 - zeleň, 6 - zastavaná plocha, 7 - priemyselná oblasť
     }
](,)Blok je možné opakovať viacnásobne. Pridáva iný blok pozadia.

------------------------------------------------------------------------------------------------

Súbor: zastávka.json
Popis: Súbor obsahujúci dáta vo formáte JSON o zastávkach na vykreslenie.
Formát obsahu súboru je nasledovný:
"názov_zastávky" : [
    {
         "typ" : 0 - zastávka autobusu, 1 - zastávka trolejbusu/autobusu,
                 2 - zastávka trolejbusu, 3 - zastávka lanovky, 4 - zastávka vlaku. ,
         "ulica1" : "názov_ulice 1 medzi ktorými má byť zastávka",
         "ulica2" : "názov_ulice 2 medzi ktorými má byť zastávka"
    }
 ](,)Blok je možné opakovať viacnásobne. Pridáva ďalšiu zastávku.
ulica2 - nepovinná oblasť, v prípade nevyplnenia tejto časti sa zastávka nastaví na koniec úsečky.

------------------------------------------------------------------------------------------------

Súbor: linky.json
Popis: Súbor obsahujúci dáta vo formáte JSON o linkách dopravy.
Formát obsahu súboru je nasledovný:
"názov_linky" : [
    {
         "typ" : 0 - autobusová linka, 1 - trolejbusová linka, 2 - linka lanovky, 3 - spoj vlaku
         "farba" : "Farba signatúry linky vo formáte RGB",
         "0" : "názov_zastávky 1, medzi ktorými má linka jazdiť.",
         "1" : "názov_zastávky 2, medzi ktorými má linka jazdiť.",
         ...
         ...
         ...
    }
 ](,)Blok je možné opakovať viacnásobne. Pridáva ďalšiu linku.

------------------------------------------------------------------------------------------------

Súbor: autobusy.json
Popis: Súbor obsahujúci dáta vo formáte JSON o autobusoch jazdiacich na zadaných linkách.
Formát obsahu súboru je nasledovný:
"číslo_autobusu" : [
  {
    "linka" : "názov_linky",
    "typ" : "názov obrázku daného dopravného prostriedku na načítanie vo formáte PNG.",
    "0" : "čas_odchodu_prvého_obehu_linky-smer_linky",
    "1" : "čas_odchodu_druhého_obehu_linky-smer_linky",
    ...
    ...
  }
](,)Blok je možné opakovať viacnásobne. Pridáva ďalšie vozidlo.
Smer linky - 0 - po smere zápisu v súbore linky.json
           - 1 - proti smeru zápisu v súbore linky.json
Čas odchodu x-tého obehu linky - formát hh:mm

================================================================================================
