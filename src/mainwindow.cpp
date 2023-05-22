/**
 * @mainpage Projekt ICP
 * @link
 * mainwindow.cpp
 * @endlink
 *
 * @file mainwindow.cpp
 * @brief Projekt ICP - Simulacia liniek verejnej dopravy
 * @author Pavol Dubovec, xdubov02
 * @author Juraj Lazur, xlazur00
 * @version 1.0
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

// Štruktúry pre uchovávanie dát
// Ulice
QMap<QString, QList<street_parts>> map_streets;
QMap<QString, QList<street_parts>>::iterator map_str;
// Zastávky
QList<bus_stop> bus_stops;
// Linky
QMap<QString, QList<line_part>> bus_lines;
QMap<QString, QList<line_part>>::iterator bus_lines_str;
QMap<QString, line_info> bus_lines_info;
// Autobusy
QMap<QString, bus_parts> busses;
QMap<QString, bus_parts>::iterator busses_ptr;

// Argumenty aplikácie
bool prog_run = false;
QRect screen_size;
QStringList arguments;

// Ukazovatele na prvky UI
static int seconds = 0;
QLabel *info, *info_2, *info_3, *info_4, *info_5, *time_label, *time_lab_speed, *picture;
QGraphicsView *view;
QComboBox *combo_box;
double time_speed = 1.0;
QString cur_line;
QPushButton *traf_up, *traf_down;

// Farby
QColor color_li_stop = QColor::fromRgb(255,196,37);
QColor color_li_train = QColor::fromRgb(0,177,89);
QColor color_li_carstop = QColor::fromRgb(66,139,202);
QColor color_li_impor = QColor::fromRgb(209,17,65);

QColor color_li_road = QColor::fromRgb(38,38,38);
QColor color_li_wire = QColor::fromRgb(145,144,144);
QColor color_li_cacar = QColor::fromRgb(38,38,38);
QColor color_li_rail = QColor::fromRgb(38,38,38);

QColor color_li_water = QColor::fromRgb(170,218,255);
QColor color_li_ground = QColor::fromRgb(201,232,201);
QColor color_li_city = QColor::fromRgb(254,247,224);
QColor color_li_indus = QColor::fromRgb(232,232,232);

// Ukazovatele na práve používateľom zobrazené prvky na mape
QGraphicsLineItem* street_to_show = nullptr;
QGraphicsEllipseItem* bus_stop_to_show = nullptr;
QGraphicsPolygonItem* bus_to_show = nullptr;
QString line_to_show = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitScene();

    // Vnútorné hodiny
    timer = new QTimer(this);

    timer->setInterval(time_speed * 1000);

    time_lab_speed->setText("Rýchlosť času: " + QString::number(time_speed));

    // Naviazanie funkcií na sloty UI prvkov
    connect(ui->time_ok,&QPushButton::clicked,this,&MainWindow::run_aplication);

    connect(timer, &QTimer::timeout, this, &MainWindow::time_change);

    connect(ui->zoom_in,&QPushButton::clicked,this,&MainWindow::zoom_in);
    connect(ui->zoom_out,&QPushButton::clicked,this,&MainWindow::zoom_out);

    connect(ui->speed_up,&QPushButton::clicked,this,&MainWindow::speed_up);
    connect(ui->speed_down,&QPushButton::clicked,this,&MainWindow::speed_down);
    connect(ui->speed_stop,&QPushButton::clicked,this,&MainWindow::pause);

    connect(ui->traffic_up,&QPushButton::clicked,this,&MainWindow::traffic_up);
    connect(ui->traffic_down,&QPushButton::clicked,this,&MainWindow::traffic_down);

    connect(ui->comboBox,SIGNAL (activated(int)),this,SLOT(line_show(int)));
    connect(ui->hint,&QPushButton::clicked,this,&show_hint);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::zoom_in()
{
    auto matrix = ui->graphicsView->transform();

    if(zoom < 2.6){
        zoom = zoom + 0.2;
        // Transformácia pohľadu
        ui->graphicsView->setTransform(QTransform(zoom, matrix.m12(), matrix.m21(), zoom, matrix.dx(), matrix.dy()));
    }
}

void MainWindow::zoom_out()
{
    auto matrix = ui->graphicsView->transform();

    if(zoom > 0.5){
        zoom = zoom - 0.2;
        // Transformácia pohľadu
        ui->graphicsView->setTransform(QTransform(zoom, matrix.m12(), matrix.m21(), zoom, matrix.dx(), matrix.dy()));
    }
}

void MainWindow::speed_up()
{
    if (time_speed > 0.01){
        time_speed /= 2;
        timer->setInterval(time_speed * 1000);
        time_lab_speed->setText("Rýchlosť času: " + QString::number(1 / time_speed) + "x");
    }
}

void MainWindow::speed_down()
{
    if (time_speed < 2.0){
        time_speed *= 2;
        timer->setInterval(time_speed * 1000);
        time_lab_speed->setText("Rýchlosť času: " + QString::number(1 / time_speed) + "x");
    }
}

void MainWindow::pause(){
    if (prog_run == true){
        timer->stop();
        prog_run = false;
        ui->speed_stop->setText("Pokračovať");
    } else {
        timer->start();
        prog_run = true;
        ui->speed_stop->setText("Pauza");
    }
}

void MainWindow::time_change(){
    QString result;

    // Globálna premenná seconds slúži ako ukazovateľ času v celej aplikácií
    seconds++;

    // Reset času o polnoci
    if (seconds >= 86400){
        seconds = 0;
    }

    // Prepočet polohy vozidiel
    compute_bus_situation();

    if (time_speed > 0.4){
        show_info(street_to_show, bus_stop_to_show, bus_to_show, line_to_show);
    }

    int hou = (seconds / (60 * 60) % 24);
    int min = (seconds / 60) % 60;
    int sec = seconds % 60;

    result = QString("%1").arg(hou, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(min, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(sec, 2, 10, QLatin1Char('0'));

    time_label->setText(result);
}

void MainWindow::line_show(int index){
    cur_line = ui->comboBox->currentText();
    show_clean();
    ui->comboBox->setCurrentIndex(index);
    show_info(nullptr, nullptr, nullptr, cur_line);
}

void move_start(){
    view->setDragMode(QGraphicsView::ScrollHandDrag);
}

void move_end(){
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::traffic_up(){
    double traf = 0;
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        for (int i = 0; i < map_str.value().size(); i++){
            if (map_str.value()[i].on_map == street_to_show){
                traf = map_str.value()[i].traffic;

                if (traf > 5.3){
                    return;
                }

                info_3->setStyleSheet(QString("padding: 5 0 5 0; font-size: 15pt; color: rgb(231,239,246); background-color: hsl(" +
                                              QString::number(100 - ((map_str.value()[i].traffic + 1.2) / 0.06)) +
                                              ",100%, 60%); font-weight: 500; text-transform: uppercase"));
                info_3->setText(QString::number(qRound(traf + 1.2)));

                // Pre každú trasu liniek, kde sa ulica vyskytuje, je zmenený čas prejazdu daným úsekom
                for (int k = 0; k < map_str.value().size(); k++){
                    for (bus_lines_str = bus_lines.begin(); bus_lines_str != bus_lines.end(); ++bus_lines_str){
                        for (int j = 0; j < bus_lines_str.value().size(); j++){
                            if (bus_lines_str.value()[j].on_map == map_str.value()[k].on_map)
                                bus_lines_str.value()[j].realduration = map_str.value()[k].on_map->line().length() * (traf + 1.2);
                        }
                    }
                    map_str.value()[k].traffic = traf + 1.2;
                }
                return;
            }
        }
    }
}

void MainWindow::traffic_down(){
    double traf = 0;
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        for (int i = 0; i < map_str.value().size(); i++){
            if (map_str.value()[i].on_map == street_to_show){
                traf = map_str.value()[i].traffic;

                if (traf < 0.7){
                    return;
                }

                info_3->setStyleSheet(QString("margin: 0 0 0 0px; padding: 5 0 5 0; font-size: 15pt; color: rgb(231,239,246); background-color: hsl(" +
                                              QString::number(100 - ((map_str.value()[i].traffic - 1.2) / 0.06)) +
                                              ",100%, 60%); font-weight: 500; text-transform: uppercase"));
                info_3->setText(QString::number(qRound(traf - 1.2)));

                // Pre každú trasu liniek, kde sa ulica vyskytuje, je zmenený čas prejazdu daným úsekom
                for (int k = 0; k < map_str.value().size(); k++){
                    for (bus_lines_str = bus_lines.begin(); bus_lines_str != bus_lines.end(); ++bus_lines_str){
                        for (int j = 0; j < bus_lines_str.value().size(); j++){
                            if (bus_lines_str.value()[j].on_map == map_str.value()[k].on_map)
                                bus_lines_str.value()[j].realduration = map_str.value()[k].on_map->line().length() * (traf - 1.2);
                        }
                    }
                    map_str.value()[k].traffic = traf - 1.2;
                }
                return;
            }
        }
    }
}

void generate_background(QGraphicsScene *scena){
    QString val;
    QFile file;
    file.setFileName(arguments.at(0));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument dokument = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject Mapa = dokument.object();
    QStringList Meno = Mapa.keys();

    QPolygonF rec;

    // Je prechádzáný celý súbor, jednotlivé mnohouholníky sú vytvorené a vyfarbené podľa súradníc zo súboru
    for(int j=0; j< Meno.size(); j++)
    {
        QJsonArray Ulica = Mapa.value(QString(Meno[j])).toArray();

        if ((Ulica[0].toObject().value("typ") != 4) && (Ulica[0].toObject().value("typ") != 5) &&
            (Ulica[0].toObject().value("typ") != 6) && (Ulica[0].toObject().value("typ") != 7)){
            continue;
        }

        for(int i = 0; i<Ulica.size();i++)
        {
            for (int j = 1; j <= ((Ulica[i].toObject().size() - 1) / 2); j++){
                rec.append(QPointF(Ulica[i].toObject().value("x" + QString::number(j)).toInt(), Ulica[i].toObject().value("y" + QString::number(j)).toInt()));
            }

            // Farba je zvolená podľa typu polygónu
            switch(Ulica[i].toObject().value("typ").toInt()) {
              case 4:
                scena->addPolygon(rec,QPen({color_li_water}, 0), QBrush(color_li_water));
                break;
              case 5:
                scena->addPolygon(rec,QPen({color_li_ground}, 0), QBrush(color_li_ground));
                break;
              case 6:
                scena->addPolygon(rec,QPen({color_li_city}, 0), QBrush(color_li_city));
                break;
              case 7:
                scena->addPolygon(rec,QPen({color_li_indus}, 0), QBrush(color_li_indus));
                break;
            }
            rec.clear();
        }
    }
}

void generate_map(QGraphicsScene *scena){
    QString val;
    QFile file;
    file.setFileName(arguments.at(0));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument dokument = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject Mapa = dokument.object();
    QStringList Meno = Mapa.keys();
    QList<street_parts> lines;
    street_parts street;
    QPen pen;
    pen.setWidth(4);

    // Každá ulica je načítaná vykreslená a ukazovateľ na ňu je uložený
    // Ulice sa môžu skladať z ľubovoľného počtu priamok
    for(int j=0; j< Meno.size(); j++)
    {
        QJsonArray Ulica = Mapa.value(QString(Meno[j])).toArray();

        if ((Ulica[0].toObject().value("typ") != 0) && (Ulica[0].toObject().value("typ") != 1) &&
            (Ulica[0].toObject().value("typ") != 2) && (Ulica[0].toObject().value("typ") != 3)){
            continue;
        }

        for(int i=0; i<Ulica.size();i++)
        {
            auto line = scena->addLine(Ulica[i].toObject().value("x1").toInt(), Ulica[i].toObject().value("y1").toInt(), Ulica[i].toObject().value("x2").toInt(), Ulica[i].toObject().value("y2").toInt());
            switch (Ulica[i].toObject().value("typ").toInt()){
            case 0:
                pen.setStyle(Qt::SolidLine);
                pen.setColor(color_li_road);
                line->setPen(pen);
                break;
            case 1:
                pen.setStyle(Qt::SolidLine);
                pen.setColor(color_li_wire);
                line->setPen(pen);
                break;
            case 2:
                pen.setColor(color_li_cacar);
                pen.setStyle(Qt::DotLine);
                line->setPen(pen);
                break;
            case 3:
                pen.setColor(color_li_rail);
                pen.setStyle(Qt::DashDotLine);
                line->setPen(pen);
                break;
            }

            street.on_map = line;
            street.type = Ulica[i].toObject().value("typ").toInt();
            street.traffic = TRAFFIC;
            lines.insert(0, street);
        }

        map_streets.insert(QString(Meno[j]), lines);
        lines.clear();
    }
}

void generate_stops(QGraphicsScene *scena){
    QString val;
    QFile file;
    int street1[4] = {0, 0, 0, 0}, street2[4] = {0, 0, 0, 0};
    QGraphicsEllipseItem* bus_stop;
    file.setFileName(arguments.at(1));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument dokument = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject Mapa = dokument.object();
    QStringList Meno = Mapa.keys();

    // Zastávky sú definované v bode stretu dvoch ulíc, alebo na konci ulice
    // Vždy je nájdený bod, kam sa má zastávka vykresliť a jej poloha je uložená
    for(int j = 0; j < Meno.size(); j++)
    {
        QJsonArray Ulica = Mapa.value(QString(Meno[j])).toArray();

        // Musíme prehľadať celú ulicu, aby sme našli pozíciu, kam máme zastávku umiestniť
        for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
            if (map_str.key() == Ulica[0].toObject().value("ulica1").toString()){
                street1[0] = map_str.value()[0].on_map->line().x2();
                street1[1] = map_str.value()[0].on_map->line().y2();
                street1[2] = map_str.value()[map_str.value().size() - 1].on_map->line().x1();
                street1[3] = map_str.value()[map_str.value().size() - 1].on_map->line().y1();
            }
            if (map_str.key() == Ulica[0].toObject().value("ulica2").toString()){
                street2[0] = map_str.value()[0].on_map->line().x2();
                street2[1] = map_str.value()[0].on_map->line().y2();
                street2[2] = map_str.value()[map_str.value().size() - 1].on_map->line().x1();
                street2[3] = map_str.value()[map_str.value().size() - 1].on_map->line().y1();

            }
        }

        if (Ulica[0].toObject().value("ulica2").toString() == ""){
            bus_stop = scena->addEllipse(street1[0] - BUS_STOP_SIZE, street1[1] - BUS_STOP_SIZE, BUS_STOP_SIZE * 2, BUS_STOP_SIZE * 2);
        } else {
            if (street1[0] == street2[0] && street1[1] == street2[1]){
                bus_stop = scena->addEllipse(street1[0] - BUS_STOP_SIZE, street1[1] - BUS_STOP_SIZE, BUS_STOP_SIZE * 2, BUS_STOP_SIZE * 2);

            } else if (street1[2] == street2[2] && street1[3] == street2[3]){
                bus_stop = scena->addEllipse(street1[2] - BUS_STOP_SIZE, street1[3] - BUS_STOP_SIZE, BUS_STOP_SIZE * 2, BUS_STOP_SIZE * 2);

            } else if (street1[0] == street2[2] && street1[1] == street2[3]){
                bus_stop = scena->addEllipse(street1[0] - BUS_STOP_SIZE, street1[1] - BUS_STOP_SIZE, BUS_STOP_SIZE * 2, BUS_STOP_SIZE * 2);

            } else if (street1[2] == street2[0] && street1[3] == street2[1]){
                bus_stop = scena->addEllipse(street1[2] - BUS_STOP_SIZE, street1[3] - BUS_STOP_SIZE, BUS_STOP_SIZE * 2, BUS_STOP_SIZE * 2);
            } else {
                exit(-1);
            }
        }

        // Zastávku vykreslíme a uložíme ju
        bus_stop->setPen(QPen(Qt::black, 2));
        if (Ulica[0].toObject().value("typ").toInt() == 0 || Ulica[0].toObject().value("typ").toInt() == 1 || Ulica[0].toObject().value("typ").toInt() == 2)
            bus_stop->setBrush(color_li_stop);
        else if (Ulica[0].toObject().value("typ").toInt() == 3)
            bus_stop->setBrush(color_li_carstop);
        else
            bus_stop->setBrush(color_li_train);
        bus_stops.insert(bus_stops.size(), {QString(Meno[j]), bus_stop, Ulica[0].toObject().value("typ").toInt()});
    }
}

void generate_busses(QGraphicsScene *scena){
    QString value;
    QFile file;
    bus_parts ac_bus;
    QString time_str;
    bus_tour tour;
    QList<bus_tour> tours;
    file.setFileName(arguments.at(3));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    value = file.readAll();
    file.close();
    QJsonDocument dokument = QJsonDocument::fromJson(value.toUtf8());
    QJsonObject Buses = dokument.object();
    QStringList Meno = Buses.keys();

    // Tvar autobusu
    QPolygonF bus_tbus;
    bus_tbus.append(QPointF(0, -5));
    bus_tbus.append(QPointF(-7.5, 5));
    bus_tbus.append(QPointF(7.5, 5));

    // Tvar vlaku
    QPolygonF train;
    train.append(QPointF(0, -5));
    train.append(QPointF(-7.5, 5));
    train.append(QPointF(0, 15));
    train.append(QPointF(7.5, 5));
    train.append(QPointF(-7.5, 5));
    train.append(QPointF(7.5, 5));

    // Tvar lanovky
    QPolygonF car_car;
    car_car.append(QPointF(-5, -5));
    car_car.append(QPointF(-5, 5));
    car_car.append(QPointF(5, 5));
    car_car.append(QPointF(5, -5));

    // Postupne načítavame, vytvárame a ukladáme vozidlá
    for(int j=0; j< Meno.size(); j++)
    {
        QJsonArray bus = Buses.value(QString(Meno[j])).toArray();

        auto pointer = bus_lines_info.find(bus[0].toObject().value("linka").toString());

        // Tvar vozidla zodpovedá linke, na ktorej jazdí
        if (pointer.key() != bus[0].toObject().value("linka").toString())
            exit(-1);
        else {
            if (pointer.value().type == 0 || pointer.value().type == 1 || pointer.value().type == 2)
                ac_bus.on_map = scena->addPolygon(bus_tbus,QPen({Qt::black}, 0), QBrush(bus_lines_info.value(bus[0].toObject().value("linka").toString()).color));
            else if (pointer.value().type == 3)
                ac_bus.on_map = scena->addPolygon(car_car,QPen({Qt::black}, 0), QBrush(bus_lines_info.value(bus[0].toObject().value("linka").toString()).color));
            else
                ac_bus.on_map = scena->addPolygon(train,QPen({Qt::black}, 0), QBrush(bus_lines_info.value(bus[0].toObject().value("linka").toString()).color));
        }

        ac_bus.ac_line = bus[0].toObject().value("linka").toString();
        ac_bus.type = bus[0].toObject().value("typ").toString();
        ac_bus.del = 0;
        ac_bus.visible = false;
        ac_bus.on_map->setVisible(false);

        // Ukladanie spojov, ktoré má vozidlo vykonať
        for(int i = 0; i < bus[0].toObject().size() - 2; i++)
        {
            time_str = bus[0].toObject().value(QString::number(i)).toString();
            tour.type = time_str.split("-")[1].toInt();
            time_str = time_str.split("-")[0];
            tour.dep_time = time_str.split(":")[0].toInt() * 3600 + time_str.split(":")[1].toInt() * 60;
            tours.insert(tours.size(), tour);
        }

        ac_bus.tour_num = tours.size() - 1;

        // Podľa zvoleného počiatočného času vypočítame aktuálny spoj pre dané vozidlo
        for (int i = 0; i <  tours.size(); i++){
            if ((tours[i].dep_time - seconds) > 0){
                ac_bus.tour_num = i - 1;
                if (ac_bus.tour_num < 0)
                   ac_bus.tour_num = 0;
                break;
            } else if ((tours[i].dep_time - seconds) == 0){
                ac_bus.tour_num = i;
                break;
            }
        }

        ac_bus.dep_times = tours;
        tours.clear();

        busses.insert(QString(Meno[j]), ac_bus);
        ac_bus.dep_times.clear();
    }
}

void generate_lines(){
    QString val, cl;
    QFile file;
    QString ac_bus_stop, next_bus_stop;
    QList<line_part> route;
    line_part part;
    bool added = true;
    int ac_point[4] = {0, 0, 0, 0}, next_point[2] = {0, 0}, color[3] = {0, 0, 0};
    file.setFileName(arguments.at(2));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument dokument = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject Linky = dokument.object();
    QStringList Linka = Linky.keys();

    // Linky sú definované zastávkami
    // Pre každú linku teda vytvoríme trasu na základe jej zastávok
    for(int j = 0; j < Linka.size(); j++){
        QJsonArray zastavky = Linky.value(QString(Linka[j])).toArray();

        // Postupne prechádzame zastávky na trase a tvoríme trasu z ukazovateľov na ulice a zastávky
        for (int i = 0; i < zastavky[0].toObject().size() - 3; i++){
            added = false;
            ac_bus_stop = zastavky[0].toObject().value(QString::number(i)).toString();
            next_bus_stop = zastavky[0].toObject().value(QString::number(i + 1)).toString();

            cl = zastavky[0].toObject().value("farba").toString();
            color[0] = cl.split(",")[0].toInt();
            color[1] = cl.split(",")[1].toInt();
            color[2] = cl.split(",")[2].toInt();

            for (int k = 0; k < bus_stops.size(); k++){
                if (bus_stops[k].name == ac_bus_stop){
                    part.on_map = bus_stops[k].on_map;
                    if (bus_stops[k].type == 4)
                        part.duration = rand() % (TRAIN_STOP_DURATION[1] - TRAIN_STOP_DURATION[0]) + TRAIN_STOP_DURATION[0];
                    else
                        part.duration = rand() % (STOP_DURATION[1] - STOP_DURATION[0]) + STOP_DURATION[0];
                    part.realduration = part.duration;
                    part.stop = true;
                    route.insert(0, part);
                    ac_point[0] = bus_stops[k].on_map->rect().x() + BUS_STOP_SIZE;
                    ac_point[1] = bus_stops[k].on_map->rect().y() + BUS_STOP_SIZE;
                    ac_point[2] = bus_stops[k].on_map->rect().x() + BUS_STOP_SIZE;
                    ac_point[3] = bus_stops[k].on_map->rect().y() + BUS_STOP_SIZE;
                }
                if (bus_stops[k].name == next_bus_stop){
                    next_point[0] = bus_stops[k].on_map->rect().x() + BUS_STOP_SIZE;
                    next_point[1] = bus_stops[k].on_map->rect().y() + BUS_STOP_SIZE;
                }
            }

            // Hľadáme spojenie medzi 2 zastávkami
            // Ak ho nájdeme, cestu si uložíme a pokračujeme ďalšou dvojicou zastávok
            added = add_connection(ac_point, next_point, &route);

            // Ak sa nám nepodarilo nájsť spojenie medzi dvoma zastávkami, použijeme pomocné funkcie a spojenie nájdeme a uložíme
            if (added == false){
                findcrossing(ac_point, next_point, &route);
            }
        }

        // Pridáme koncovú zastávku
        ac_bus_stop = zastavky[0].toObject().value(QString::number(zastavky[0].toObject().size() - 3)).toString();
        for (int k = 0; k < bus_stops.size(); k++)
            if (bus_stops[k].name == ac_bus_stop){
                part.on_map = bus_stops[k].on_map;
                if (bus_stops[k].type == 4)
                    part.duration = rand() % (TRAIN_STOP_DURATION[1] - TRAIN_STOP_DURATION[0]) + TRAIN_STOP_DURATION[0];
                else
                    part.duration = rand() % (STOP_DURATION[1] - STOP_DURATION[0]) + STOP_DURATION[0];
                part.realduration = part.duration;
                part.stop = true;
                route.insert(0, part);
            }

        // Dĺžka pobytu na koncových zastávkach je len technického charakteru
        route[0].duration = 1;
        route[0].realduration = 0;
        route[route.size() - 1].duration = 1;
        route[route.size() - 1].realduration = 0;

        bus_lines.insert(QString(Linka[j]), route);
        bus_lines_info.insert(QString(Linka[j]), {zastavky[0].toObject().value("typ").toInt(), QColor(color[0], color[1], color[2])});

        route.clear();
    }
}

bool add_connection(int *ac_point, int *next_point, QList<line_part> *route){
    bool added = false;
    line_part part;

    // Nájdeme spojenie medzi 2 zastávkami a pridáme ju do trasy linky
    // Jedná sa o 3 kombinácie rovnakých parametrov
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        // Našli sme počiatočný bod, bod zhodný so zastávkou
        if (ac_point[0] == map_str.value()[0].on_map->line().x1() && ac_point[1] == map_str.value()[0].on_map->line().y1()){
            // Skúsime prísť na druhý koniec ulice
            for (int i = 0; i < map_str.value().size(); i++){
                ac_point[0] = map_str.value()[i].on_map->line().x2();
                ac_point[1] = map_str.value()[i].on_map->line().y2();
            }
            // Ak sme našli koncový bod ulice rovný s nasledujúcou zastávkou máme spojenie a môžeme ho pridať na trasu
            // Ak nie, hľadaáme ďalej
            if (ac_point[0] == next_point[0] && ac_point[1] == next_point[1]){
                for (int i = 0; i < map_str.value().size(); i++){
                    part.on_map = map_str.value()[i].on_map;
                    part.duration = map_str.value()[i].on_map->line().length() * TRAFFIC;

                    // Čas potrebný na prejdenie ulice
                    // Čím vyššia hodnota, tým pomalší pohyb
                    switch (map_str.value()[i].type){
                        case 0:
                        case 1: part.duration = map_str.value()[i].on_map->line().length() * TRAFFIC; // MHD
                                break;
                        case 2: part.duration = map_str.value()[i].on_map->line().length() * 5; // Lanovka
                                break;
                        case 3: part.duration = map_str.value()[i].on_map->line().length() * 0.55; // Vlak
                                break;
                    }

                    part.realduration = part.duration;
                    part.stop = false;
                    route->insert(0, part);
                }
                added = true;
            } else {
                ac_point[0] = ac_point[2];
                ac_point[1] = ac_point[3];
            }

        } else if (ac_point[0] == map_str.value()[map_str.value().size() - 1].on_map->line().x1() && ac_point[1] == map_str.value()[map_str.value().size() - 1].on_map->line().y1()){
            for (int i = map_str.value().size() - 1; i >= 0; i--){
                ac_point[0] = map_str.value()[i].on_map->line().x2();
                ac_point[1] = map_str.value()[i].on_map->line().y2();
            }
            if (ac_point[0] == next_point[0] && ac_point[1] == next_point[1]){
                for (int i = map_str.value().size() - 1; i >= 0; i--){
                    part.on_map = map_str.value()[i].on_map;

                    switch (map_str.value()[i].type){
                        case 0:
                        case 1: part.duration = map_str.value()[i].on_map->line().length() * TRAFFIC;
                                break;
                        case 2: part.duration = map_str.value()[i].on_map->line().length() * 5;
                                break;
                        case 3: part.duration = map_str.value()[i].on_map->line().length() * 0.55;
                                break;
                    }

                    part.realduration = part.duration;
                    part.stop = false;
                    route->insert(0, part);
                }
                added = true;
            } else {
                ac_point[0] = ac_point[2];
                ac_point[1] = ac_point[3];
            }

        } else if (ac_point[0] == map_str.value()[0].on_map->line().x2() && ac_point[1] == map_str.value()[0].on_map->line().y2()){
            for (int i = 0; i < map_str.value().size(); i++){
                ac_point[0] = map_str.value()[i].on_map->line().x1();
                ac_point[1] = map_str.value()[i].on_map->line().y1();
            }
            if (ac_point[0] == next_point[0] && ac_point[1] == next_point[1]){
                for (int i = 0; i < map_str.value().size(); i++){
                    part.on_map = map_str.value()[i].on_map;

                    switch (map_str.value()[i].type){
                        case 0:
                        case 1: part.duration = map_str.value()[i].on_map->line().length() * TRAFFIC;
                                break;
                        case 2: part.duration = map_str.value()[i].on_map->line().length() * 5;
                                break;
                        case 3: part.duration = map_str.value()[i].on_map->line().length() * 0.55;
                                break;
                    }

                    part.realduration = part.duration;
                    part.stop = false;
                    route->insert(0, part);
                }
                added = true;
            } else {
                ac_point[0] = ac_point[2];
                ac_point[1] = ac_point[3];
            }
        }
    }
    return added;
}

void findcrossing(int *start, int *finish, QList<line_part> *route){
    int pos_finish[4] = {0, 0, 0, 0};
    int pos_finish2[4] = {0, 0, 0, 0};

    // Ak sme nenašli spojenie medzi 2 zastávkami hľadáme spojenie inak
    // Nájdeme všetky 4 možnosti, ideme teda oboma smermi zo zastávok
    cross(pos_finish, start);
    cross(pos_finish2, finish);

    // Nájdeme bod, ktorý je spoločný a prepojíme cezeň trasu linky
    if ((pos_finish[0] == pos_finish2[0] && pos_finish[1] == pos_finish2[1]) || (pos_finish[0] == pos_finish2[2] && pos_finish[1] == pos_finish2[3])){
        pos_finish[2] = pos_finish[0];
        pos_finish[3] = pos_finish[1];
        add_connection(start, pos_finish, route);
        add_connection(pos_finish, finish, route);
    } else {
        pos_finish[0] = pos_finish[2];
        pos_finish[1] = pos_finish[3];
        add_connection(start, pos_finish, route);
        add_connection(pos_finish, finish, route);
    }
}

void cross(int *pos, int *start){
    int test = 0;
    // Hľadáme súradnice konca ulice
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        if (start[0] == map_str.value()[0].on_map->line().x1() && start[1] == map_str.value()[0].on_map->line().y1()){
            for (int i = 0; i < map_str.value().size(); i++){
                pos[test] = map_str.value()[i].on_map->line().x2();
                pos[test + 1] = map_str.value()[i].on_map->line().y2();
            }
            test += 2;

        } else if (start[0] == map_str.value()[map_str.value().size() - 1].on_map->line().x1() && start[1] == map_str.value()[map_str.value().size() - 1].on_map->line().y1()){
            for (int i = map_str.value().size() - 1; i >= 0; i--){
                pos[test] = map_str.value()[i].on_map->line().x2();
                pos[test + 1] = map_str.value()[i].on_map->line().y2();
            }
            test += 2;

        } else if (start[0] == map_str.value()[0].on_map->line().x2() && start[1] == map_str.value()[0].on_map->line().y2()){
            for (int i = 0; i < map_str.value().size(); i++){
                pos[test] = map_str.value()[i].on_map->line().x1();
                pos[test + 1] = map_str.value()[i].on_map->line().y1();
            }
            test += 2;

        }
        if (test > 3){
            return;
        }
    }
}

void show_info(QGraphicsLineItem* street_ts, QGraphicsEllipseItem* bus_stop_ts, QGraphicsPolygonItem* bus_ts, QString line_ts){
    // Na základe parametrov volá príslušnú funkciu
    if (street_ts != nullptr){
        street_to_show = street_ts;
        show_street(street_to_show);
    } else if (bus_stop_ts != nullptr){
        bus_stop_to_show = bus_stop_ts;
        show_busstop(bus_stop_to_show);
    } else if (bus_ts != nullptr){
        show_bus(bus_ts);
    } else if (line_ts != nullptr){
        line_to_show = line_ts;
        show_line(line_to_show);
    }
}

void show_clean(){
    QPen pen;
    pen.setWidth(4);

    // Funkcia odstráni zobrazenie informácií, obnoví vzhľad mapy, kde nie je nič zvýraznené
    // Ulice
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        for (int i = 0; i < map_str.value().size(); i++){
            switch (map_str.value()[i].type){
            case 0:
                pen.setStyle(Qt::SolidLine);
                pen.setColor(color_li_road);
                map_str.value()[i].on_map->setPen(pen);
                break;
            case 1:
                pen.setStyle(Qt::SolidLine);
                pen.setColor(color_li_wire);
                map_str.value()[i].on_map->setPen(pen);
                break;
            case 2:
                pen.setColor(color_li_cacar);
                pen.setStyle(Qt::DotLine);
                map_str.value()[i].on_map->setPen(pen);
                break;
            case 3:
                pen.setColor(color_li_rail);
                pen.setStyle(Qt::DashDotLine);
                map_str.value()[i].on_map->setPen(pen);
                break;
            }
        }
        traf_up->setVisible(false);
        traf_down->setVisible(false);
    }

    // Zastávky
    for (int k = 0; k < bus_stops.size(); k++){
        if (bus_stops[k].type == 0 || bus_stops[k].type == 1 || bus_stops[k].type == 2)
            bus_stops[k].on_map->setBrush(color_li_stop);
        else if (bus_stops[k].type == 3)
            bus_stops[k].on_map->setBrush(color_li_carstop);
        else
            bus_stops[k].on_map->setBrush(color_li_train);
        bus_stops[k].on_map->setPen(QPen({Qt::black}, 2));
    }

    if (bus_to_show != nullptr)
        bus_to_show->setPen(QPen({Qt::black}, 0));

    // Labely
    picture->setText(" ");
    picture->setStyleSheet(QString(""));
    info->setText("");
    info->setStyleSheet(QString(""));
    info_2->setText("");
    info_2->setStyleSheet(QString(""));
    info_3->setText("");
    info_3->setStyleSheet(QString(""));
    info_4->setText("");
    info_4->setStyleSheet(QString(""));
    info_5->setText("");
    info_5->setStyleSheet(QString(""));

    info->setVisible(false);
    info_2->setVisible(false);
    info_3->setVisible(false);
    info_4->setVisible(false);
    info_5->setVisible(false);
    picture->setVisible(false);

    street_to_show = nullptr;
    bus_stop_to_show = nullptr;
    bus_to_show = nullptr;
    line_to_show = nullptr;

    combo_box->setCurrentIndex(0);
}

void show_street(QGraphicsLineItem* road){
    // Zvýrazní vybranú ulicu a zobrazí o nej podrobnosti
    for (map_str = map_streets.begin(); map_str != map_streets.end(); ++map_str){
        for (int i = 0; i < map_str.value().size(); i++){
            if (map_str.value()[i].on_map == road && (map_str.value()[i].type == 0 || map_str.value()[i].type == 1)){

                info->setVisible(true);
                info_2->setVisible(true);
                info_3->setVisible(true);

                // Formátovanie
                info->setStyleSheet(QString("padding: 10 0 10 0; font-size: 15pt; color: rgb(39,49,120); background-color: rgb(231,239,246);"
                                            "font-weight: 600; text-transform: uppercase; border: 5 solid rgb(247,31,46);"));
                info_2->setStyleSheet(QString("font-size: 13pt; color: rgb(231,239,246)"));
                info_3->setStyleSheet(QString("padding: 5 0 5 0; font-size: 15pt; color: rgb(231,239,246); background-color: hsl(" +
                                              QString::number(100 - (map_str.value()[i].traffic / 0.06)) +
                                              ",100%, 60%); font-weight: 500; text-transform: uppercase"));

                info->setText("Ulica\n" + map_str.key());

                info_2->setText("\n\nÚroveň dopravy:");
                info_3->setText(QString::number(qRound(map_str.value()[i].traffic)));

                // Zvýraznenie na mape
                for (int i = 0; i < map_str.value().size(); i++)
                    map_str.value()[i].on_map->setPen(QPen(color_li_impor, 6));

                traf_up->setVisible(true);
                traf_down->setVisible(true);

                return;
            }
        }
    }
}

void show_busstop(QGraphicsEllipseItem* bus_stop){

    // Zvýrazní zastávku na mape a vypíše informácie
    QList<int> lines;
    QString result;
    for (int k = 0; k < bus_stops.size(); k++){
        if (bus_stops[k].on_map == bus_stop){
            bus_stops[k].on_map->setBrush(color_li_impor);
            bus_stops[k].on_map->setPen(QPen({color_li_impor}, 3));

            picture->setVisible(true);
            info->setVisible(true);
            info_2->setVisible(true);
            info_5->setVisible(true);

            // Formátovanie
            info->setStyleSheet(QString("padding: 2 0 2 0; font-size: 18pt; color: rgb(21,43,100); background-color: rgb(231,239,246);"
                                        "font-weight: 250; text-transform: uppercase"));
            info_2->setStyleSheet(QString("font-size: 20pt; color: rgb(21,43,100); font-weight: 600"));
            info_5->setStyleSheet(QString("font-size: 10pt; font-weight: 400; background-color: rgb(169,171,170)"));
            picture->setPixmap(QPixmap("pic/stop_" + QString::number(bus_stops[k].type) + ".png").scaled(screen_size.width() * 0.11, screen_size.height() * 0.11,Qt::KeepAspectRatio));

            QList<QString> lines;
            QList<int> lines_to_sort;

            // Vyhľadáme linky, ktoré na zastávke zastavujú
            for (bus_lines_str = bus_lines.begin(); bus_lines_str != bus_lines.end(); ++bus_lines_str){
                for (int i = (bus_lines_str.value().size() - 1); i >= 0 ; i--){
                    if (bus_lines_str.value()[i].on_map == bus_stop){
                        if (bus_lines_str.key().toInt() == false){
                            lines.insert(0, bus_lines_str.key());
                        } else {
                            lines_to_sort.insert(0, bus_lines_str.key().toInt());
                        }
                        break;
                    }
                }
            }

            // Pretriedime ich
            std::sort(lines_to_sort.begin(), lines_to_sort.end());

            for (int i = lines_to_sort.size() - 1; i >= 0; i--)
                lines.insert(0, QString::number(lines_to_sort[i]));

            info->setText(bus_stops[k].name);

            result = "<html><body><style>"
                     "#in{text-align: center; background-color: rgb(231,239,246)}"
                     "#non{text-align: center; background-color: None}"
                     "#ni{text-align: center; background-color: rgb(21,43,100); color: rgb(231,239,246)}"
                     "#train{text-align: center; color: rgb(231,239,246); background-color:" + color_li_train.name(QColor::HexArgb) + "}"
                     "#cabcar{text-align: center; color: rgb(231,239,246); background-color:" + color_li_carstop.name(QColor::HexArgb) + "}"
                     "</style> <table border='0' align='center' width='100%' cellspacing='10' cellpadding='4'><tr>";

            int total = (lines.size() / 4) * 4;

            if ((lines.size() % 4) > 0){
                total = total + 4;
            }

            // Formátovaný výpis liniek, ktoré zastavujú na zastávke
            for (int i = 0; i < total; i++){
                if (i < lines.size()){
                    if (bus_lines_info.value(lines[i]).type == 0 || bus_lines_info.value(lines[i]).type == 1)
                        result = result + "<td id='in' width='25%'>" + lines[i] + "</td>";
                    else if (bus_lines_info.value(lines[i]).type == 2)
                        result = result + "<td id='ni' width='25%'>" + lines[i] + "</td>";
                    else if (bus_lines_info.value(lines[i]).type == 3)
                        result = result + "<td id='cabcar' width='25%'>" + lines[i] + "</td>";
                    else
                        result = result + "<td id='train' width='25%'>" + lines[i] + "</td>";

                    if ((i + 1) % 4 == 0 && i != 0 && ((total - 1) != i)){
                        result = result + "</tr><tr>";
                    }
                } else {
                    result = result + "<td id='non' width='25%'></td>";
                }
            }

            result = result + "</tr></table></body></html>";

            info_2->setText(result);
            lines_to_sort.clear();
            result = "";

            // Odchodové tabule
            // Získame dáta o spojoch a vypíšeme aktuálne najbližšie odchody
            double times;
            QList<QPair<double, bus_stop_info>> dep_info;
            bus_stop_info info;

            // Prejdeme všetky autobusy a zistíme ich aktuálny spoj, polohu
            for (int j = 0; j < lines.size(); j++)
                for (busses_ptr = busses.begin(); busses_ptr != busses.end(); ++busses_ptr){
                    if (busses_ptr.value().tour_num > (busses_ptr.value().dep_times.size() - 1) || busses_ptr.value().visible == false
                        || busses_ptr.value().ac_line != lines[j]){
                        continue;
                    }

                    bus_lines_str = bus_lines.find(lines[j]);
                    times = busses_ptr.value().dep_times[busses_ptr.value().tour_num].dep_time;

                    if (busses_ptr.value().dep_times[busses_ptr.value().tour_num].type == 0){
                        for (int i = bus_lines_str.value().size() - 1; i > 0; i--){
                            times = times + bus_lines_str.value()[i].duration;
                            if (bus_lines_str.value()[i].stop == true && bus_lines_str.value()[i].on_map == bus_stop){
                                double real_times = times + busses_ptr.value().del;
                                if ((seconds - real_times) < 0){
                                    info.time = times;
                                    info.ac_line = busses_ptr->ac_line;
                                    for (int k = 0; k < bus_stops.size(); k++)
                                        if (bus_lines_str.value()[0].on_map == bus_stops[k].on_map)
                                            info.dest = bus_stops[k].name;
                                    info.del = busses_ptr->del;
                                    dep_info.append(qMakePair(real_times, info));
                                    break;
                                }
                            }
                        }
                    } else {
                        for (int i = 0; i < bus_lines_str.value().size() - 1; i++){
                            times = times + bus_lines_str.value()[i].duration;
                            if (bus_lines_str.value()[i].stop == true && bus_lines_str.value()[i].on_map == bus_stop){
                                double real_times = times + busses_ptr.value().del;
                                if ((seconds - real_times) < 0){
                                    info.time = times;
                                    info.ac_line = busses_ptr->ac_line;
                                    for (int k = 0; k < bus_stops.size(); k++)
                                        if (bus_lines_str.value()[bus_lines_str->size() - 1].on_map == bus_stops[k].on_map)
                                            info.dest = bus_stops[k].name;
                                    info.del = busses_ptr->del;
                                    dep_info.append(qMakePair(real_times, info));
                                    break;
                                }
                            }
                        }
                    }
                }

            std::sort(dep_info.begin(), dep_info.end(), QPairFirstComparer());

            if ((lines.size() / 3) > 0 && lines.size() != 3){
                total = 6;
            } else {
                total = 3;
            }

            // Formátovaný výstup
            result = "<html><body><style>"
                     "#in_line{background-color: rgb(13,122,80); color: rgb(255,252,21); font-size: 15pt; vertical-align: middle}"
                     "#in_dep{background-color: rgb(13,122,80); color: rgb(255,252,21); font-size: 13pt; vertical-align: middle}"
                     "#in_time{text-align: right; background-color: rgb(13,122,80); color: rgb(255,252,21); font-size: 15pt; vertical-align: middle}"
                     "#info{font-size: 12pt}"
                     "#info_dep{font-size: 12pt; text-align: center}"
                     "#info_time{font-size: 12pt; text-align: right}"
                     "#non{background-color: rgb(13,122,80); font-size: 15pt}"
                     "#non_dep{background-color: rgb(13,122,80); None; font-size: 13pt}"
                     "</style> <table border='0' border-style='inset' align='center' width='96%' cellspacing='0' cellpadding='2'>";
            QString spoj;
            int times2, hou, min, sec;

            result = result + "<tr><td width='10%' id='info'>Linka</td><td width='70%' id='info_dep'>Smer</td><td width='20%' id='info_time'>Odchod</td></tr>";

            for (int k = 0; k < total; k++)
                if (k >= dep_info.size()){
                    result = result + "<tr></td><td id='in_line'></td><td id='in_dep'></td><td id='in_time'></td></tr>";
                } else {
                    // V prípade, že spoj príde na zastávku za viac ako 15 minút, vypíše sa celý čas
                    // Inak sa vypíšu minúty do príchodu
                    // V čase odchodu je započítané aj meškanie
                    if ((dep_info[k].second.time + dep_info[k].second.del - seconds) > 900){
                        times2 = qRound(dep_info[k].second.time + dep_info[k].second.del);

                        hou = (times2 / (60 * 60) % 24);
                        min = (times2 / 60) % 60;
                        sec = times2 % 60;

                        if (sec > 30){
                            if (min == 59){
                                spoj = QString("%1").arg(hou + 1, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(0, 2, 10, QLatin1Char('0'));
                            } else {
                                spoj = QString("%1").arg(hou, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(min + 1, 2, 10, QLatin1Char('0'));
                            }
                        } else {
                            spoj = QString("%1").arg(hou, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(min, 2, 10, QLatin1Char('0'));
                        }
                    } else {
                        spoj = QString::number(qRound((dep_info[k].second.time + dep_info[k].second.del - seconds) / 60));

                        if (spoj == "0")
                            spoj = "**";
                    }

                    result = result +
                             "<tr><td id='in_line'>" + dep_info[k].second.ac_line + "</td>" +
                             "<td id='in_dep'>" + dep_info[k].second.dest + "</td>" +
                             "<td id='in_time'>" + spoj + "</td></tr>";
                }

            result = result +
                    "<tr></td><td></td><td></td><td></td></tr>" +
                    "</table></body></html>";

            info_5->setText(result);

            return;
        }
    }
}

void show_line(QString number){
    // Zvýrazní trasu linky na mape
    int ac_buses = 0;
    for (bus_lines_str = bus_lines.begin(); bus_lines_str != bus_lines.end(); ++bus_lines_str){
        if (bus_lines_str.key() == number){
            for (int i = 0; i < bus_lines_str.value().size(); i++){
                if (bus_lines_str.value()[i].stop == true){
                    auto stop = static_cast<QGraphicsEllipseItem*>(bus_lines_str.value()[i].on_map);
                    stop->setPen(QPen(bus_lines_info.value(number).color, 2));
                } else {
                    auto line = static_cast<QGraphicsLineItem*>(bus_lines_str.value()[i].on_map);
                    line->setPen(QPen(bus_lines_info.value(number).color, 3));
                }
            }

            picture->setVisible(true);
            info->setVisible(true);
            info_2->setVisible(true);
            info_4->setVisible(true);
            info_5->setVisible(true);

            // Formát výstupu
            picture->setStyleSheet(QString("font-size: 40pt; padding: 20 0 20 0; color: Black; background-color:" +
                                           bus_lines_info.value(number).color.name(QColor::HexArgb) + "; font-weight: 500"));
            info->setStyleSheet(QString("font-size: 13pt; color: rgb(231,239,246)"));
            info_2->setStyleSheet(QString("font-size: 20pt; color: rgb(231,239,246)"));
            info_4->setStyleSheet(QString("margin: 0 0 0 0px; padding: 10 0 10 0; font-size: 15pt; background-color: rgb(231,239,246);"
                                        "font-weight: 250; text-transform: uppercase"));
            info_5->setStyleSheet(QString("font-size: 12pt; color: rgb(231,239,246)"));

            picture->setText(number);

            info->setText("\nPočet vozidiel na linke:");

            // Získame počet vozdiel na linke, ktoré sú v službe
            for (busses_ptr = busses.begin(); busses_ptr != busses.end(); ++busses_ptr){
                if (busses_ptr.value().ac_line == number && busses_ptr.value().visible == true){
                    ac_buses++;
                }
            }

            info_2->setText(QString::number(ac_buses));
            info_4->setText("trasa");

            // Vypíšeme trasu linky
            bus_lines_str = bus_lines.find(number);
            QString result;
            for (int i = bus_lines_str.value().size() - 1; i >= 0; i--){
                if (bus_lines_str.value()[i].stop == true)
                    for (int k = 0; k < bus_stops.size(); k++)
                        if (bus_lines_str.value()[i].on_map == bus_stops[k].on_map)
                            result = result + bus_stops[k].name + "\n";
            }

            info_5->setText(result);
            return;
        }
    }

}

void show_bus(QGraphicsPolygonItem* bus){
    // Zvýrazní vozdilo a informácie o ňom
    for (busses_ptr = busses.begin(); busses_ptr != busses.end(); ++busses_ptr){
        if (bus == busses_ptr.value().on_map){
            if (busses_ptr.value().tour_num > (busses_ptr.value().dep_times.size() - 1)){
                bus_to_show = nullptr;
                show_clean();
                return;
            }
            picture->setVisible(true);
            info->setVisible(true);
            info_2->setVisible(true);
            info_3->setVisible(true);
            info_4->setVisible(true);
            info_5->setVisible(true);

            // Formát výstupu
            info->setStyleSheet(QString("padding: 5 0 5 0; font-size: 15pt; background-color: rgb(231,239,246);"
                                        "font-weight: 250; text-transform: uppercase"));
            info_4->setStyleSheet(QString("padding: 5 0 5 0; font-size: 15pt; background-color: rgb(231,239,246);"
                                        "font-weight: 250; text-transform: uppercase"));
            info_2->setStyleSheet(QString("font-size: 13pt; color: rgb(231,239,246)"));
            info_3->setStyleSheet(QString("font-size: 13pt; color: rgb(231,239,246)"));
            info_5->setStyleSheet(QString("font-size: 12pt"));

            picture->setPixmap(QPixmap("pic/" + busses_ptr->type + ".png").scaled(screen_size.width() * 0.13, screen_size.height() * 0.13,Qt::KeepAspectRatio));
            bus->setPen(QPen(color_li_impor, 4));

            // Informácie získané z vozidla
            info->setText(busses_ptr.key());
            info_2->setText("Linka: " + busses_ptr.value().ac_line);
            info_3->setText("Meškanie: " + QString::number(qRound((busses_ptr.value().del) / 60)) + " min.");
            info_4->setText("Aktuálny spoj");

            bus_lines_str = bus_lines.find(busses_ptr.value().ac_line);
            QString result;
            bool on_stop = false;
            double times = busses_ptr.value().dep_times[busses_ptr.value().tour_num].dep_time;

            // Zobrazenie aktuálneho spoja vozidla
            // Vypíšu sa zastávky, čas odchodu podľa cestovné poriadku a poloha vozidla
            result = "<html><body><style>#in{text-align: center; background-color: #ffffff; vertical-align: middle} #non{background-color: None}"
                     "#after{background-color:#c0c5ce; color:#65737e; text-align: center; vertical-align: middle}"
                     "#ex{background-color: #d11141; text-align: center; color: #E7E6E1; vertical-align: middle}"
                     "</style> <table border='0' border-style='inset' align='center' width='100%' cellspacing='5' cellpadding='4'>";

            if (busses_ptr.value().dep_times[busses_ptr.value().tour_num].type == 0){
                for (int i = bus_lines_str.value().size() - 1; i >= 0; i--){
                    show_timetable(&times, i, &result, &on_stop, busses_ptr.value().del);
                }
            } else {
                for (int i = 0; i < bus_lines_str.value().size(); i++){
                    show_timetable(&times, i, &result, &on_stop, busses_ptr.value().del);
                }
            }

            result = result + "</table></body></html>";

            info_5->setText(result);

            // Poradie zastávok závisí na smere spoja
            for (int i = 0; i < bus_lines_str.value().size(); i++){
                if (bus_lines_str.value()[i].stop == true){
                    auto stop = static_cast<QGraphicsEllipseItem*>(bus_lines_str.value()[i].on_map);
                    stop->setPen(QPen(bus_lines_info.value(busses_ptr.value().ac_line).color, 2));
                } else {
                    auto line = static_cast<QGraphicsLineItem*>(bus_lines_str.value()[i].on_map);
                    line->setPen(QPen(bus_lines_info.value(busses_ptr.value().ac_line).color, 3));
                }
            }

            bus_to_show = bus;
            return;
        }
    }
    move_start();
}

void show_timetable(double *times, int i, QString *result, bool *on_stop, double delay){
    // Pomocná funkcia pre výpis trasy spoja
    QString spoj;
    *times = *times + bus_lines_str.value()[i].duration;

    if (bus_lines_str.value()[i].stop == true){

        double real_times = *times + delay;

        int times2 = qRound(*times);

        int hou = (times2 / (60 * 60) % 24);
        int min = (times2 / 60) % 60;
        int sec = times2 % 60;

        if (sec > 30){
            if (min == 59){
                spoj = QString("%1").arg(hou + 1, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(0, 2, 10, QLatin1Char('0'));
            } else {
                spoj = QString("%1").arg(hou, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(min + 1, 2, 10, QLatin1Char('0'));
            }
        } else {
            spoj = QString("%1").arg(hou, 2, 10, QLatin1Char('0')) + ":" + QString("%1").arg(min, 2, 10, QLatin1Char('0'));
        }

        // Formát výpisu závisí od aktuálnej polohy vozidla
        if ((seconds - real_times) < 0 && *on_stop == false){
            *on_stop = true;
            for (int k = 0; k < bus_stops.size(); k++)
                if (bus_stops[k].on_map == bus_lines_str.value()[i].on_map)
                    *result = *result + "<tr><td id='ex' width='10%'>" + spoj + "</td><td id='ex' width='90%'>" +
                              bus_stops[k].name + "</td></tr>";
        } else if (*on_stop == false){
            for (int k = 0; k < bus_stops.size(); k++)
                if (bus_stops[k].on_map == bus_lines_str.value()[i].on_map)
                    *result = *result + "<tr><td id='after' width='10%'>" + spoj + "</td><td id='after' width='90%'>" +
                              bus_stops[k].name + "</td></tr>";
        } else {
            for (int k = 0; k < bus_stops.size(); k++)
                if (bus_stops[k].on_map == bus_lines_str.value()[i].on_map)
                    *result = *result + "<tr><td id='in' width='10%'>" + spoj + "</td><td id='in' width='90%'>" +
                              bus_stops[k].name + "</td></tr>";
        }
    }
}

void show_hint(){
    show_clean();

    // Zobrazenie nápovedy
    info->setStyleSheet(QString("margin: 0 0 0 0px; padding: 10 0 10 0; font-size: 15pt; background-color: rgb(231,239,246);"
                                "font-weight: 250; text-transform: uppercase"));
    info_5->setStyleSheet(QString("font-size: 12pt"));

    info->setVisible(true);
    info_5->setVisible(true);

    info->setText("Nápoveda");

    QString result;

    result = "<html><body><style>"
             "#head{text-align: center; background-color: rgb(231,239,246); vertical-align: middle; font-size: 14pt}"
             "#text{background-color: None; color: rgb(231,239,246)}"
             "</style> <table border='0' border-style='inset' align='center' width='100%' cellspacing='0' cellpadding='4'>";

    result = result +
            "<tr><td id='head'>Okno aplikácie</td></tr>"
            "<tr><td id='text' align='justify'>Aplikácia je rozdelená na 3 časti. Ľavá časť programu sú nastavenia aplikácie. "
            "V strede sa nachádza náhľad na mapu a v pravej časti sa zobrazujú vybrané informácie."
            "<tr><td id='head'>Čas a rýchlosť simulácie</td></tr>"
            "<tr><td id='text' align='justify'>Systém má svoje vnútorné hodiny, ktoré je možné ovládať pomocou 3 tlačidiel v ľavom hornom rohu.<br>"
            "Prvé tlačidlo zabezpečuje zrýchlenie času, druhé zabezpečuje zastavenie alebo spustenie času a tretie spomalenie času.<br>"
            "Informácia o aktuálnej rýchlosti simulácie je vypísaná pod hodinami.</td></tr>"
            "<tr><td id='head'>Mapa</td></tr>"
            "<tr><td id='text' align='justify'>Mapa sa skladá z podkladu, ulíc, zastávok a vozidiel. Pre pohyb po mape chyťťe myšou ľubovoľné miesto na mape a potiahnite smerom, ktorým sa chcete pohnúť.<br>"
            "Pre priblíženie alebo oddialenie použite tlačidlá v ľavom dolnom rohu.</td></tr>"
            "<tr><td id='head'>Mapový podklad</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_city.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Obytná oblasť</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_indus.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Priemysel</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_water.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Voda</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_ground.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Voľná krajina</td></tr>"
            "<tr><td id='head'>Zobrazovanie liniek</td></tr>"
            "<tr><td id='text' align='justify'>V strede ľavej časti programu sa nachádza možnosť zobrazenia liniek. Po výbere, sa na mape zvýrazní trasa vybranej linky. "
            "Systém taktiež vypíše aktuálny počet vozidiel vykonávajúcich spoje na linke a názvy zastávok na trase linky.</td></tr>"
            "<tr><td id='head'>Zobrazovanie ulíc</td></tr>"
            "<tr><td id='text' align='justify'>Pre zobrazenie informácií o ulici je nutné na danú ulicu kliknúť na mape.  "
            "Systém vypíše názov ulice a úroveň dopravy. Úroveň dopravy je možné meniť v pravom dolnom rohu pre každú ulicu zvlášť. "
            "Po zmene intenzity dopravy dochádza k spomaleniu dopravy a meškaniu spojov.</td></tr>"
            "<tr><td id='head'>Zobrazovanie zastávok</td></tr>"
            "<tr><td id='text' align='justify'>Pre zobrazenie informácií o zastávke je nutné na danú zastávku kliknúť na mape.  "
            "Systém vypíše názov zastávky, linky, ktoré na nej zastavujú a najbližšie odchody v reálnom čase.</td></tr>"
            "<tr><td id='head'>Druhy zastávok</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_stop.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Zastávka MHD</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_carstop.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Stanica lanovky</td></tr>"
            "<tr><td id='text' align='justify'><span style='background-color:" + color_li_train.toRgb().name(QColor::HexArgb) + "'>&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;Železničná stanica</td></tr>"
            "<tr><td id='head'>Zobrazovanie vozidiel</td></tr>"
            "<tr><td id='text' align='justify'>Pre zobrazenie informácií o vozidle je nutné na dané vozidlo kliknúť na mape.  "
            "Systém vypíše číslo vozidla, linku, na ktorej jazdí, aktuálne meškanie a aktuálny spoj, ktorý vozidlo vykonáva.<br>"
            "Informácie o spoji obsahujú zastávky, pričom je zvýraznené, kde sa vozidlo nachádza.<br>"
            "Na mape sa zároveň vyznačí celá trasa vozidla.</td></tr>"
            "</table></body></html>";

    info_5->setText(result);
}

double set_bus_on_route(QString number, double time, int track_or, QGraphicsPolygonItem *bus_pos, bool *end, double delay){
    // Získame čas od začiatku spoja
    time -= delay;

    if (time < 0){
        time = 0;
    }

    // Podľa smeru vozidla na trase určíme jeho polohu
    bus_lines_str = bus_lines.find(number);
    if (bus_lines_str.key() != number)
        exit(-1);
    // z A do B
    if (track_or == 0){
        for (int i = bus_lines_str.value().size() - 2; i >= 0; i--){
            time = time - bus_lines_str.value()[i].duration;

            if (time <= 0){
                // Na uliciach môže vznikať meškanie
                if (bus_lines_str.value()[i].stop == false){
                    // Pri najnižšej úrovni dopravy sa spoj snaží meškanie eliminovať
                    if (qRound(bus_lines_str.value()[i].realduration - bus_lines_str.value()[i].duration) == 0 && delay > 0){
                        delay -= ((bus_lines_str.value()[i].duration - bus_lines_str.value()[i].duration * 0.98) / bus_lines_str.value()[i].realduration);
                    // Pri zhoršenej dopravnej situácií sa jeho rýchlosť spomaľuje a spoj naberá meškanie
                    } else {
                        delay += ((bus_lines_str.value()[i].realduration - bus_lines_str.value()[i].duration) / bus_lines_str.value()[i].realduration);
                    }
                } 

                // Umiestnenie vozidla
                put_on_route(time, bus_pos, bus_lines_str.value()[i].on_map, bus_lines_str.value()[i + 1].on_map, bus_lines_str.value()[i].duration);
                return delay;
            }
        }
        *end = true;

    // Z B do A
    } else if (track_or == 1){
        for (int i = 1; i < bus_lines_str.value().size(); i++){
            time = time - bus_lines_str.value()[i].duration;

            if (time <= 0){
                if (bus_lines_str.value()[i].stop == false){
                    if (qRound(bus_lines_str.value()[i].realduration - bus_lines_str.value()[i].duration) == 0 && delay > 0){
                        delay -= ((bus_lines_str.value()[i].duration - bus_lines_str.value()[i].duration * 0.98) / bus_lines_str.value()[i].realduration);
                    } else {
                        delay += ((bus_lines_str.value()[i].realduration - bus_lines_str.value()[i].duration) / bus_lines_str.value()[i].realduration);
                    }
                }

                put_on_route(time, bus_pos, bus_lines_str.value()[i].on_map, bus_lines_str.value()[i - 1].on_map, bus_lines_str.value()[i].duration);
                return delay;
            }
        }
        *end = true;
    }
    return 0;
}

void put_on_route(double time, QGraphicsPolygonItem *bus_pos, void *object, void *prev_obj, double duration){
    double real_pos, rot;
    int position[4] = {0, 0, 0, 0};

    // Umiestňujeme na zastávke
    if (line_or_rec(object) == 0){
        auto stop = static_cast<QGraphicsEllipseItem*>(object);
        bus_pos->setPos(stop->rect().x() + BUS_STOP_SIZE, stop->rect().y() + BUS_STOP_SIZE);
    // Umiestňujeme na ulici
    } else {
        if (line_or_rec(prev_obj) == 0){
            auto stop = static_cast<QGraphicsEllipseItem*>(prev_obj);
            position[0] = stop->rect().x() + BUS_STOP_SIZE;
            position[1] = stop->rect().y() + BUS_STOP_SIZE;
        } else {
            auto line = static_cast<QGraphicsLineItem*>(prev_obj);
            position[0] = line->line().x1();
            position[1] = line->line().y1();
            position[2] = line->line().x2();
            position[3] = line->line().y2();
        }

        auto line = static_cast<QGraphicsLineItem*>(object);

        // Orientácia vozidla
        rot  = 90 - line->line().angle();

        // Prepočítanie pomeru, času ku dĺžke, kde sa autobus nachádza
        if (((position[0] == line->line().x2()) && (position[1] == line->line().y2())) || ((position[2] == line->line().x2()) && (position[3] == line->line().y2()))){
            real_pos = time / duration * -1.0;
            bus_pos->setRotation(rot + 180);
        } else {
            real_pos = time / duration + 1;
            bus_pos->setRotation(rot);
        }

        // Umiestnenie do bodu
        bus_pos->setPos(line->line().pointAt(real_pos).x(), line->line().pointAt(real_pos).y());
    }
}

int line_or_rec(void* pointer){
    for (int i = 0; i < bus_stops.size(); i++)
        if (bus_stops[i].on_map == pointer)
            return 0;
    return 1;
}

void compute_bus_situation(){
    bool doo = false;
    int ac_tour = 0;

    // Pre každé vozidlo je vypočítaná jeho situácia
    for (busses_ptr = busses.begin(); busses_ptr != busses.end(); ++busses_ptr){
        // Vozidlo už vykonalo všetky spoje, do nasledujúceho dňa je mimo službu
        if (busses_ptr.value().tour_num >= busses_ptr.value().dep_times.size()){
            ac_tour = seconds - busses_ptr.value().dep_times[0].dep_time;
            if (ac_tour <= 0){
                busses_ptr.value().tour_num = 0;
            }
            if (busses_ptr.value().visible == true){
                busses_ptr.value().on_map->setVisible(false);
                busses_ptr.value().visible = false;
            }
            continue;
        }

        // Ak má vozidlo ešte nejaké spoje, môže byť buď na trase, na konečnej zastávke alebo má prestávku
        // Prestávka vzniká, ak je medzi dvoma spojmi viac ako hodinový rozostup
        // Počas prestávky vozidlo nie je viditeľné

        // Výpočet aktuálneho času od konečnej zastávke
        ac_tour = seconds - busses_ptr.value().dep_times[busses_ptr.value().tour_num].dep_time;

        // Vozidlo stojí na konečnej zastávke
        if (ac_tour <= 0){
            // Do ochodu ostáva viac ako hodina, vozidlo je však viditeľné a viditeľné ostane
            if (ac_tour < -3600){
                if (busses_ptr.value().visible == true){
                    busses_ptr.value().on_map->setVisible(false);
                    busses_ptr.value().visible = false;
                }
                continue;
            // Vozidlo nie je viditeľné, ale do odchodu ostávajú viac ako minúty
            } else if (busses_ptr.value().visible == false && ac_tour < -240){
                continue;
            // Vozidlu skončila prestávka, stáva sa viditeľnés
            } else {
                if (busses_ptr.value().visible == false){
                    busses_ptr.value().on_map->setVisible(true);
                    busses_ptr.value().visible = true;
                    busses_ptr.value().del = set_bus_on_route(busses_ptr.value().ac_line, 0, busses_ptr.value().dep_times[busses_ptr.value().tour_num].type,
                                                              busses_ptr.value().on_map, &doo, busses_ptr.value().del);
                }
            }
        }

        if (busses_ptr.value().visible == false){
            busses_ptr.value().on_map->setVisible(true);
            busses_ptr.value().visible = true;
            busses_ptr.value().del = set_bus_on_route(busses_ptr.value().ac_line, 0, busses_ptr.value().dep_times[busses_ptr.value().tour_num].type,
                                                      busses_ptr.value().on_map, &doo, busses_ptr.value().del);
            continue;
        }

        // Umiestnenie vozidla na trasu
        busses_ptr.value().del = set_bus_on_route(busses_ptr.value().ac_line, ac_tour, busses_ptr.value().dep_times[busses_ptr.value().tour_num].type,
                                                  busses_ptr.value().on_map, &doo, busses_ptr.value().del);

        // Ak sa vozidlo nachádza na konečnej, vykonalo spoj a prechádza na nasledujúci
        if (doo == true){
            busses_ptr.value().tour_num++;

            // Meškanie sa môže preniesť aj medzi 2 spojmi
            if (busses_ptr.value().tour_num >= busses_ptr.value().dep_times.size()){
                busses_ptr.value().del = 0;
            } else {
                busses_ptr.value().del = seconds - busses_ptr.value().dep_times[busses_ptr.value().tour_num].dep_time + 120;

                if (busses_ptr.value().del < 0){
                    busses_ptr.value().del = 0;
                }

                set_bus_on_route(busses_ptr.value().ac_line, 0, busses_ptr.value().dep_times[busses_ptr.value().tour_num].type,
                                 busses_ptr.value().on_map, &doo, busses_ptr.value().del);
            }

            doo = false;
        }
    }
}

void MainWindow::run_aplication(){
    // Získanie počiatočného času
    auto *scena = ui->graphicsView->scene();
    prog_run = true;
    timer->start();
    ui->app->setCurrentIndex(1);

    // Nastavenie počiatočného času
    seconds = ui->time_set->time().minute() * 60 + ui->time_set->time().hour() * 3600;

    // Načítanie autobusov
    generate_busses(scena);
}

void MainWindow::InitScene()
{
    if (qApp->arguments().size() < 9)
        exit(-1);

    // Kontrola a načítanie argumentov
    for (int i = 0; i < qApp->arguments().size(); i++){
        if (qApp->arguments().value(i) == "-map")
            arguments.insert(0, qApp->arguments().value(i + 1));
        else if(qApp->arguments().value(i) == "-stops")
            arguments.insert(1, qApp->arguments().value(i + 1));
        else if(qApp->arguments().value(i) == "-lines")
            arguments.insert(2, qApp->arguments().value(i + 1));
        else if(qApp->arguments().value(i) == "-busses")
            arguments.insert(3, qApp->arguments().value(i + 1));
    }

    if (arguments.size() != 4)
        exit(-1);

    // Vytvorenie grafickej scény
    auto *scena = new MapScene(ui->graphicsView);
    ui->graphicsView->setScene(scena);
    auto scene = ui->graphicsView->scene();

    // Nastavenie rozlíšenia
    QDesktopWidget* screen_widget = qApp->desktop();
    screen_size = screen_widget->availableGeometry(screen_widget->primaryScreen());
    this->setMinimumWidth(screen_size.width() * 0.6);
    this->setMinimumHeight(screen_size.height() * 0.5);

    ui->time_info->setFixedWidth(screen_size.width() * 0.15);
    ui->scrollArea->setFixedWidth(screen_size.width() * 0.15);
    ui->label_5->setFixedWidth(screen_size.width() * 0.15);
    ui->start_info->setFixedWidth(screen_size.width() * 0.15);
    ui->traffic_up->setVisible(false);
    ui->traffic_down->setVisible(false);

    ui->scrollArea->setFrameShape(QScrollArea::NoFrame);

    // Nastavenie UI
    info = ui->info;
    info_2 = ui->info_2;
    info_3 = ui->info_3;
    info_4 = ui->info_4;
    info_5 = ui->info_5;
    picture = ui->picture;
    time_label = ui->time_info;
    time_lab_speed = ui->time_speed;
    view = ui->graphicsView;
    combo_box = ui->comboBox;
    traf_up = ui->traffic_up;
    traf_down = ui->traffic_down;

    info->setVisible(false);
    info_2->setVisible(false);
    info_3->setVisible(false);
    info_4->setVisible(false);
    info_5->setVisible(false);
    picture->setVisible(false);

    // Načítanie dát a vygenerovanie máp
    generate_background(scene);
    generate_map(scene);
    generate_stops(scene);
    generate_lines();

    QList<QString> lines;
    QList<int> lines_to_sort;

    // Pridanie liniek do ComboBox
    for (bus_lines_str = bus_lines.begin(); bus_lines_str != bus_lines.end(); ++bus_lines_str){
        if (bus_lines_str.key().toInt() == false){
            lines.insert(0, bus_lines_str.key());
        } else {
            lines_to_sort.insert(0, bus_lines_str.key().toInt());
        }
    }

    std::sort(lines_to_sort.begin(), lines_to_sort.end());

    for (int i = lines_to_sort.size() - 1; i >= 0; i--)
        lines.insert(0, QString::number(lines_to_sort[i]));

    ui->comboBox->addItems(lines);

    ui->graphicsView->centerOn(QPoint(900, 600));

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}
