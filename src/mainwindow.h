/**
 * @mainpage Projekt ICP
 * @link
 * mainwindow.h
 * @endlink
 *
 * @file mainwindow.h
 * @brief Projekt ICP - Simulacia liniek verejnej dopravy
 * @author Pavol Dubovec, xdubov02
 * @author Juraj Lazur, xlazur00
 * @version 1.0
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Použité knižnice
#include <QMainWindow>
#include <QGraphicsLineItem>
#include "mapscene.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QByteArray>
#include <QMap>
#include <QTimer>
#include <QComboBox>
#include <QPolygonF>
#include <QLineF>
#include <QColor>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPixmap>
#include <QStyle>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QScrollBar>
#include <QDateTimeEdit>

// Konštanty
/// Základná úroveň dopravy na ulici
const double TRAFFIC = 0.6;
/// Rozmedzie dĺžky pobytu vozidla na zastávke
const int STOP_DURATION[2] = {15, 35};
/// Rozmedzie dĺžky pobytu vlaku v stanici
const int TRAIN_STOP_DURATION[2] = {75, 105};
/// Grafická veľkosť zastávky
const int BUS_STOP_SIZE = 6;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

public slots:

    void run_aplication();
    /**
     * @defgroup map_functions Funkcie pracujuce s mapou
     * Funkcie pracujuce s mapovym podkladom, s nahladom na mapu.
     * @{
     */

    /**
     * Funkcia pre priblizenie mapy. Vykona transformaciu matice grafickeho pohladu.
     *
     * @pre
     * Vygenerovany mapovy podklad.
     *
     * @post
     * Priblizeny pohlad na mapu.
     */
    void zoom_in();

    /**
     * Funkcia pre oddialenie mapy. Vykona transformaciu matice grafickeho pohladu.
     *
     * @pre
     * Vygenerovany mapovy podklad.
     *
     * @post
     * Oddialeny pohlad na mapu.
     */
    void zoom_out();
    /**
     * @}
     */

    /**
     * @defgroup time_functions Funkcie pracujuce s vnutornym casom
     * Funkcie pracujuce s vnutornym casom aplikacie. Zabezpecuju jeho zrychlenie, spomalenie, zastavenie.
     * @{
     */

    /**
     * Funkcia pre zrychlenie casu.
     *
     * @post
     * Pohyb casu je zrychleny dvojnasobne.
     */
    void speed_up();

    /**
     * Funkcia pre spomalenie casu.
     *
     * @post
     * Pohyb casu je spomaleny dvojnasobne.
     */
    void speed_down();

    /**
     * Funkcia pre zastavenie casu.
     *
     * @post
     * Pohyb casu je zastaveny, v pripade opatovneho stlacenia je cas opat spusteny.
     */
    void pause();

    /**
     * Funkcia pre aktualizaciu mapy a casovych udajov. Funkcia aktualizuje zobrazovany cas v aplikacii.
     * Zaroven zavola funkcie pre aktualizaciu polohy vozidiel, pripadne aktualizaciu vybranych zobrazenych informacii.
     * Aktualizacia tychto zobrazovanych informacii je pre svoju vypoctovu narocnost vykonavana len do casovej rychlosti 4.
     * Pri vyssich casovych rychlostiach tieto informacie nie su aktualizovane.
     *
     * @post
     * Pohyb casu je zrychleny dvojnasobne.
     */
    void time_change();
    /**
     * @}
     */

    /**
     * @defgroup show_info Funkcie zobrazujuce informacie zvolene pouzivatelom
     * Funkcie zabezpecuju vypis vybranych informacii o vozidle, zastavke, linke alebo ulici zvolenej pouzivatelom.
     * @{
     */

    /**
     * Funkcia pre zobrazenie linky
     *
     * @post
     * Na mape je zvyraznena trasa konkretnej linky.
     * Zaroven su vypisane informacie o aktualnom pocte vozidiel na linke a vypis zastavok na trase linky.
     *
     * @param index Aktualny index objektu ComboBox
     */
    void line_show(int index);
    /**
     * @}
     */

    /**
     * @defgroup traffic_functions Funkcie pracujuce s urovnou dopravy na uliciach
     * Funkcie zvysuju, alebo znizuju uroven dopravy na zvolenej ulici. Definovanych je 5 stupnov urovne dopravy.
     * @{
     */

    /**
     * Funkcia zvysujuca uroven dopravy na zvolenej ulici.
     *
     * @post
     * Doprava sa zacina spomalovat, vznikaju meskania spojov.
     */
    void traffic_up();

    /**
     * Funkcia znizujuca uroven dopravy na zvolenej ulici.
     *
     * @post
     * Doprava sa zrychluje, na urovni dopravy 1 mozu vozidla meskania mierne znizovat.
     */
    void traffic_down();
    /**
     * @}
     */

private:
    void InitScene();
    Ui::MainWindow *ui;
    double zoom = 1.0;
    QTimer *timer;
};

/**
 * @defgroup map_info Funkcie a struktury uchovavajuce mapovy podklad
 * Funkcie a struktury, ktore nacitavaju suradnice zo suboru a vykresluju mapovy podklad.
 * Zaroven poskytuju informacie o polohe ulic pre dalsie funkcie.
 * @{
 */

/**
 * @brief Format struktury pre ulozenie jednej casti ulice.
 */
struct street_parts
{
    /** Ukazovatel na priamku v grafickej scene */
    QGraphicsLineItem* on_map;
    /** Uroven dopravy na priamke */
    double traffic;
    /** Typ priamky vzhladom na typ dopravy */
    int type;
};
/**
 * @}
 */

/**
 * @defgroup bus_stop_info Funkcie a struktury uchovavajuce informacie o zastavkach
 * Funkcie a struktury, ktore nacitavaju udaje o zastavkach zo suboru.
 * Zastavky su zaroven generovane do mapoveho podkladu a informacie o nich su poskytovane inym funkciam.
 * @{
 */

/**
 * @brief Format struktury pre ulozenie zastavky.
 */
struct bus_stop
{
    /** Nazov zastavky */
    QString name;
    /** Ukazovatel na objekt v grafickej scene */
    QGraphicsEllipseItem* on_map;
    /** Typ zastavky vzhladom na typ dopravy */
    int type;
};
/**
 * @}
 */

/**
 * @defgroup line_info Funkcie a struktury uchovavajuce informacie o linkach
 * Funkcie a struktury, ktore nacitavaju udaje o linkach zo suboru.
 * Zaroven vytvaraju trasy liniek zlozene zo zastavok a ulic.
 * Tieto informacie potom poskytujú funkciám pracujúcim s vozidlami.
 * @{
 */

/**
 * @brief Format struktury pre ulozenie jednej casti trasy.
 */
struct line_part
{
    /** Ukazovatel na objekt v grafickej scene */
    void* on_map;
    /** Cas, za ktory je mozne ulicu prejst, pri zastavkach je to doba pobytu na zastavke */
    double duration;
    /** Skutocny cas vzhladom na uroven dopravy, za ktory je mozne ulicu prejst */
    double realduration;
    /** Informacia, ci sa jedna o zastavku alebo ulicu */
    bool stop;
};

/**
 * @brief Informacie o linke. Farba linky a typ linky vzhladom na typ dopravy.
 */
struct line_info
{
    /** Typ linky vzhladom na typ dopravy */
    int type;
    /** Farba linky */
    QColor color;
};
/**
 * @}
 */

/**
 * @defgroup bus_info Funkcie a struktury uchovavajuce informacie o vozidlach
 * Funkcie a struktury, ktore nacitavaju udaje o vozidlach zo suboru.
 * Generuju vozidla do grafickeho priestoru. Poskytuju informacie pri aktualizacii polohy vozidiel.
 * @{
 */

/**
 * @brief Informacie o jednom spoji, vykonavanom vozidlom.
 */
struct bus_tour
{
    /** Smer spoja */
    int type;
    /** Cas odchodu spoja z pociatocnej zastavky */
    int dep_time;
};

/**
 * @brief Informacie o vozidle.
 */
struct bus_parts
{
    /** Ukazovatel na vozidlo v grafickej scene */
    QGraphicsPolygonItem *on_map;
    /** Aktualna linka vozidla */
    QString ac_line;
    /** Typ vozidla */
    QString type;
    /** Meskanie vozidla */
    double del;
    /** Cislo vykonavaneho spoja */
    int tour_num;
    /** Flag, ci sa vozidlo zobrazuje na mape, a teda ci je v sluzbe */
    bool visible;
    /** Zoznam spojov vykonavanych vozidlom */
    QList<bus_tour> dep_times;
};
/**
 * @}
 */

/*! \addtogroup show_info
 *  @{
 */

/**
 * @brief Pomocna struktura pri zobrazovani odchodovych tabul na zastavkach.
 */
struct bus_stop_info
{
    double time;
    QString ac_line;
    QString dest;
    double del;
};
/**
 * @}
 */

// Pomocna funkcia pre triedenie pri zobrazovani zastavkovej tabule
struct QPairFirstComparer
{
    template<typename T1, typename T2>
    bool operator()(const QPair<T1,T2> & a, const QPair<T1,T2> & b) const
    {
        return a.first < b.first;
    }
};

/*! \addtogroup map_functions
 *  @{
 */

/**
 * Funkcia spustajuca moznost pohybu po mape.
 *
 */
void move_start();

/**
 * Funkcia ukoncujuca moznost pohybu po mape.
 *
 */
void move_end();
/**
 * @}
 */

/*! \addtogroup show_info
 *  @{
 */

/**
 * Riadiaca funkcia pre zobrazovanie informacii
 * Funkcia na zaklade parametrov spusta prislusne funkcie pre zobrazenie konkretnych informacii zvolenych pouzivatelom.
 *
 * @param street_ts Ukazovatel na ulicu v grafickej scene
 * @param bus_stop_ts Ukazovatel na zastavku v grafickej scene
 * @param bus_ts Ukazovatel na vozidlo v grafickej scene
 * @param line_ts Zvolena linka
 */
void show_info(QGraphicsLineItem* street_ts, QGraphicsEllipseItem* bus_stop_ts, QGraphicsPolygonItem* bus_ts, QString line_ts);

/**
 * Funkcia ukoncujuca zobrazenie informacii
 * Funkcia odstrani vsetky zobrazene informacie a nastavi aplikaciu do stavu po spusteni.
 *
 */
void show_clean();

/**
 * Funkcia zobrazujuca informacie o zvolenej ulici
 *
 * @pre Vygenerovany mapovy podklad
 *
 * @post Zobrazene informacie o ulici, jej nazov, uroven dopravy
 * @post Je mozne menit uroven dopravy na ulici
 *
 * @param road Ukazovatel na ulicu
 */
void show_street(QGraphicsLineItem *road);

/**
 * Funkcia zobrazujuca informacie o zvolenej zastavke, stanici
 *
 * @pre Vygenerovany mapovy podklad
 *
 * @post Zobrazene informacie o zastavke, jej nazov, linky ktore na nej zastavuju
 * @post Zobrazenie odchodovej tabule s najblizsimi odchodmi spojov
 *
 * @param bus_stop Ukazovatel na zastavku
 */
void show_busstop(QGraphicsEllipseItem* bus_stop);

/**
 * Funkcia zobrazujuca informacie o zvolenej linke
 *
 * @pre Vygenerovany mapovy podklad
 *
 * @post Zobrazene informacie o linke, pocet vozidiel, trasa
 * @post Na mape je zvyraznena cela trasa linky
 *
 * @param number Zobrazovana linka
 */
void show_line(QString number);

/**
 * Funkcia zobrazujuca informacie o zvolenom vozidle
 *
 * @pre Vygenerovany mapovy podklad
 * @pre Nacitane linky
 *
 * @post Zobrazene informacie o vozidle, typ, meskanie, trasa aktualneho spoja spolu s polohou vozidla
 *
 * @param bus Ukazovatel na vozidlo
 */
void show_bus(QGraphicsPolygonItem* bus);

/**
 * Pomocna funkcia pre zobrazovanie informacii o spoji
 * Funckcia vráti názov zastávky, cas odchodu podla cestovneho poriadku a ci sa vozidlo na trase nachadza pred alebo za nou.
 *
 * @param times Suhrnna dlzka spoja
 * @param i Cislo useku trasy
 * @param result Vysledny string urceny pre zobrazenie
 * @param on_stop Flag, ci sa vozidlo nachádza pred, alebo za zastávkou
 * @param delay Meskanie vozidla
 */
void show_timetable(double *times, int i, QString *result, bool *on_stop, double delay);

/**
 * Funkcia zobrazujuca napovedu
 *
 * @post Zobrazene napovedy
 */
void show_hint();
/**
 * @}
 */

/*! \addtogroup map_info
 *  @{
 */

/**
 * Funkcia vykreslujuca ulice
 * Funkcia nacita subor, vypocita polohu ulice, vykresli ju a ulozi o nej informacie pre dalsie pouzitie.
 *
 * @pre Subor so suradnicami ulic
 *
 * @post Vygenerovana mapa s ulicami
 *
 * @param scena Ukazovatel na graficku scenu
 */
void generate_map(QGraphicsScene *scena);

/**
 * Funkcia vykreslujuca pozadie mapoveho podkladu
 * Funkcia nacita subor a vykresli jednotlive objekty podla ich suradnic a typu
 *
 * @pre Subor so suradnicami objektov pozadia
 *
 * @post Vygenerovana mapa s pozadim
 *
 * @param scena Ukazovatel na graficku scenu
 */
void generate_background(QGraphicsScene *scena);
/**
 * @}
 */

/*! \addtogroup bus_stop_info
 *  @{
 */

/**
 * Funkcia vykreslujuca zastavky
 * Funkcia nacita subor, vypocita polohu zastavky, vykresli ju a ulozi o nej informacie pre dalsie pouzitie.
 *
 * @pre Subor s polohou zastavok
 * @pre Vygenerovana mapa s ulicami
 *
 * @post Vygenerovana mapa so zastavkami
 *
 * @param scena Ukazovatel na graficku scenu
 */
void generate_stops(QGraphicsScene *scena);
/**
 * @}
 */

/*! \addtogroup bus_info
 *  @{
 */

/**
 * Funkcia vykreslujuca vozidla
 * Funkcia nacita subor, vypocita polohu vozdila, vykresli ho a ulozi informacie o vozidle pre dalsie pouzitie.
 *
 * @pre Subor s udajmi o vozidlach
 * @pre Vygenerovana mapa s ulicami, zastavkami
 * @pre Nacitane linky
 *
 * @post Vygenerovana mapa s autobusmi
 *
 * @param scena Ukazovatel na graficku scenu
 */
void generate_busses(QGraphicsScene *scena);
/**
 * @}
 */

// Pomocne funkcie
// Nacitanie argumentov
void get_args(QStringList *input);
// Funkcia detekujuca zastavky
int line_or_rec(void* pointer);

/*! \addtogroup line_info
 *  @{
 */

/**
 * Funkcia nacitavajuca linky
 * Funkcia nacita subor, vypocita trasy liniek na mape, vypocita jazdne casy a ulozi tieto informacie.
 * Trasy liniek su vypocitavanie na zaklade zastavok, ktore linka obsahuje.
 *
 * @pre Subor s udajmi o linkach
 * @pre Vygenerovana mapa s ulicami, zastavkami
 *
 * @post Nacitane linky
 */
void generate_lines();

/**
 * Funkcia pridavajuca spojenie medzi dvoma zastavkami
 * Funkcia pridava ulice na trasu linky. V pripade, ze cesta nie je jedna ulica, volá pomocné funkcie.
 *
 * @param ac_point Ukazovatel na suradnice 1 zastavky
 * @param next_point Ukazovatel na suradnice 2 zastavky
 * @param route Ukazovatel na linku, ktorej trasa je vytvarana
 */
bool add_connection(int *ac_point, int *next_point, QList<line_part> *route);

/**
 * Pomocna funkcia hladajuca spojenie dvoch ulic medzi dvoma zastavkami
 * Funkcia najde spojenie medzi 2 bodmi trasy.
 *
 * @param start Ukazovatel na suradnice 1 zastavky
 * @param finish Ukazovatel na suradnice 2 zastavky
 * @param route Ukazovatel na linku, ktorej trasa je vytvarana
 */
void findcrossing(int *start, int *finish, QList<line_part> *route);

/**
 * Pomocna funkcia hladajuca bod spojenia
 * Funkcia prechaza ulice od daneho zaciatku a vrati suradnice krizenia tychto ulic s inymi ulicami
 *
 * @param pos_finish Ukazovatel na suradnice moznych bodov spojenia
 * @param start Ukazovatel na suradnice pociatocneho bodu
 */
void cross(int *pos_finish, int *start);
/**
 * @}
 */

/*! \addtogroup bus_info
 *  @{
 */

/**
 * Pomocna funkcia vypocitavajuca polohu vozidla
 * Funkcia na zaklade parametrov vypoci polohu vozidla a jeho aktualne meskanie na zaklade jeho polohy
 *
 * @param number Linka vozidla
 * @param time Cas od zaciatku spoja
 * @param track_or Poradove cislo spoja vozidla
 * @param bus_pos Ukazovatel na vozidlo v grafickej scene
 * @param end Flag, ci sa vozidlo nachadza na konecnej
 * @param delay Meskanie vozidla pred vypoctom
 */
double set_bus_on_route(QString number, double time, int track_or, QGraphicsPolygonItem *bus_pos, bool *end, double delay);

/**
 * Pomocna funkcia umiestnujuca vozidlo na mapu
 * Funkcia na zaklade parametrov vypoci polohu vozidla a umiestni ho na mapu
 *
 * @param time Cas od zaciatku spoja
 * @param bus_pos Ukazovatel na vozidlo v grafickej scene
 * @param object Ukazovatel na cast trasy, kam sa autobus ma vykreslit
 * @param prev_obj Ukazovatel na predchadzajucu cast trasy
 * @param duration Cas, za ktory vozidlo prejde dany usek
 */
void put_on_route(double time, QGraphicsPolygonItem *bus_pos, void *object, void *prev_obj, double duration);

/**
 * Funkcia riadiaca vypocet stavu vozidiel
 * Funkcia prechadza vsetky vozidla a na zaklade ich spojov a aktualneho casu ich bud vykresli na trase, na konecnej zastavke alebo ich nevykresli, pretoze nie su v sluzbe.
 * Kazde vozidlo ma v zdrojovom subore definovany svoj denny rozpis spojov. Na zaklade tohoto rozpisu a aktualneho casu funkcia rozhodne, co sa s vozidlom ma stat.
 *
 * @pre Subor s udajmi o linkach
 * @pre Vygenerovana mapa s ulicami, zastavkami
 *
 * @post Nacitane linky
 */
void compute_bus_situation();
/**
 * @}
 */

#endif // MAINWINDOW_H
