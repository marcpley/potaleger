#include "zoomgraphicsview.h"
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QVBoxLayout>

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent)
    : QGraphicsView(parent), pixmapItem(new QGraphicsPixmapItem), currentZoomLevelIndex(8) { // Start at 100%
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setCacheMode(QGraphicsView::CacheBackground);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    QGraphicsScene *scene=new QGraphicsScene(this);
    scene->addItem(pixmapItem);
    setScene(scene);

    // Define zoom levels
    zoomLevels={0.05, 0.10, 0.15, 0.20, 0.25, 0.3333, 0.50, 0.75, 1.0, 1.25, 1.50, 2.0, 2.50, 3.0, 4.0, 5.0};

    // Create zoom label
    zoomLabel=new QLabel(this);
    zoomLabel->setStyleSheet("QLabel { background-color : lightgray; color : black; }");
    zoomLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    zoomLabel->setMargin(5);
    zoomLabel->hide();

    // Create timer to hide zoom label
    hideZoomLabelTimer=new QTimer(this);
    hideZoomLabelTimer->setSingleShot(true);
    connect(hideZoomLabelTimer, &QTimer::timeout, zoomLabel, &QLabel::hide);
}

void ZoomGraphicsView::setImage(const QPixmap &pixmap) {
    resetTransform(); // Reset the transformation (zoom, rotation, etc.)
    currentZoomLevelIndex=8;
    pixmapItem->setPixmap(pixmap);
    scene()->setSceneRect(pixmapItem->boundingRect());
    updateScale();
    centerOn(pixmapItem);
}

void ZoomGraphicsView::wheelEvent(QWheelEvent *event) {
    // Zoom avant
    if (event->angleDelta().y() > 0) {
        if (currentZoomLevelIndex < zoomLevels.size() - 1) {
            currentZoomLevelIndex++;
        }
    }
    // Zoom arrière
    else {
        if (currentZoomLevelIndex > 0) {
            currentZoomLevelIndex--;
        }
    }
    updateScale();
    showZoomPercentage();
}

void ZoomGraphicsView::mousePressEvent(QMouseEvent *event) {
    if (event->button()==Qt::LeftButton) {
        // Convertir les coordonnées du clic de souris en coordonnées de la scène
        QPointF scenePoint=mapToScene(event->pos());

        // Centrer la vue sur le point de la scène où le clic a eu lieu
        centerOn(scenePoint);
    } else {
        // Appeler le comportement par défaut pour les autres boutons de la souris
        QGraphicsView::mousePressEvent(event);
    }
}

void ZoomGraphicsView::updateScale() {
    double scaleFactor=zoomLevels[currentZoomLevelIndex];
    QTransform t;
    t.scale(scaleFactor, scaleFactor);
    setTransform(t);
}

void ZoomGraphicsView::showZoomPercentage() {
    int zoomPercentage=static_cast<int>(zoomLevels[currentZoomLevelIndex] * 100);
    zoomLabel->setText(QString::number(zoomPercentage) + "%");
    zoomLabel->adjustSize();
    QPoint mousePos=mapFromGlobal(QCursor::pos());
    zoomLabel->move(mousePos-QPoint(10, 40));
    zoomLabel->show();
    hideZoomLabelTimer->start(1000); // Hide after 1 second
}
