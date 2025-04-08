#ifndef ZOOMGRAPHICSVIEW_H
#define ZOOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QTimer>

class ZoomGraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ZoomGraphicsView(QWidget *parent = nullptr);
    void setImage(const QPixmap &pixmap);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QGraphicsPixmapItem *pixmapItem;
    QVector<double> zoomLevels;
    int currentZoomLevelIndex;
    QLabel *zoomLabel;
    QTimer *hideZoomLabelTimer;
    void updateScale();
    void showZoomPercentage();
};

#endif // ZOOMGRAPHICSVIEW_H
