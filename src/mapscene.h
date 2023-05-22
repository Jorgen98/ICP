/**
 * @mainpage Projekt ICP
 * @link
 * mapscene.h
 * @endlink
 *
 * @file mapscene.h
 * @brief Projekt ICP - Simulacia liniek verejnej dopravy
 * @author Pavol Dubovec, xdubov02
 * @author Juraj Lazur, xlazur00
 * @version 1.0
 */

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>

// Graficka scena
class MapScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MapScene(QObject *parent = nullptr);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:

};

#endif // MAPSCENE_H
