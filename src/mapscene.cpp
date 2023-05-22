/**
 * @mainpage Projekt ICP
 * @link
 * mapscene.cpp
 * @endlink
 *
 * @file mapscene.cpp
 * @brief Projekt ICP - Simulacia liniek verejnej dopravy
 * @author Pavol Dubovec, xdubov02
 * @author Juraj Lazur, xlazur00
 * @version 1.0
 */

#include "mapscene.h"
#include "mainwindow.h"
#include <QtDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>

QColor ground = QColor::fromRgb(201,232,201);

MapScene::MapScene(QObject *parent) : QGraphicsScene(parent)
{
    MapScene::setBackgroundBrush(ground);
}

// Funkcia rozpoznávajúca, na čo používateľ klikol
// Podľa toho potom volá príslušnú funkciu
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    show_clean();

    for (auto* item : items(event->scenePos())){
        auto road = dynamic_cast<QGraphicsLineItem*>(item);
            if (road){
                show_info(road,nullptr, nullptr, nullptr);
                return;
            }
        auto stop = dynamic_cast<QGraphicsEllipseItem*>(item);
            if (stop){
                show_info(nullptr, stop, nullptr, nullptr);
                return;
            }
        auto bus = dynamic_cast<QGraphicsPolygonItem*>(item);
            if (bus){
                show_info(nullptr, nullptr, bus, nullptr);
                return;
            }
    }

    move_start();
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    move_end();
}
