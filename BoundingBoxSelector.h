#ifndef LABEL_IMG_H
#define LABEL_IMG_H

#include <QObject>
#include <QLabel>
#include <QImage>
#include <QMouseEvent>
#include <QMap>

#include <iostream>
#include <fstream>

class BoundingBoxSelector
   : public QLabel
{
    Q_OBJECT

public:
    struct Label
    {
        using Vector = std::vector<Label>;

        QString label;
        QRectF box;
        bool   focused;
    };

public:
    BoundingBoxSelector(QWidget *parent = nullptr);
    void init();
    bool openImage(QString const&);
    void showImage(bool menuHandling = false);

    void loadClassBoxes(QVariantMap::iterator);
    void syncClassBoxes();
    void setFocusObjectLabel(QString const&);

    void setClasses(QVariantMap* classes);
    void clearAllClassBoxex();

    auto importClassBoxesFromAnnotationFile(QString const&, QVariantMap&) -> QVariantMap;
    void exportClassBoxesToAnnotationFile(QVariantMap::iterator, QVariantMap const&) const;
    auto getCrops(QVariantMap::iterator) const -> QMap<QString, QList<QImage>>;

signals:
    void Mouse_Moved();
    void Mouse_Pressed();
    void Mouse_Release();
    void datasetIteratorUpdated();

private:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint &pos);

private:
    void setMousePosition(QPoint&& pos = {});
    void updateMousePosition();
    void drawCrossLine(QPainter& , QColor , int thickWidth = 3);
    bool updateSelectedObjects();

private:
    QString _focusedClassName;

    QImage m_inputImg;
    QImage m_inputImgScaled;

    // TODO: Should be used instead of m_MousePos, m_MousePosPrev and m_bLabelingStarted
    //QRect        _selectedRect;
    QPointF      m_MousePos;
    QPointF      m_MousePosPrev;
    bool         m_bLabelingStarted{};
    QVariantMap* _classes{};
    QVariantMap::iterator _datasetIt{};

    Label::Vector m_objBoundingBoxes;
    Label::Vector::iterator m_selectedItem;
};

auto toTxtExtention(QString const& filePath) -> QString;

#endif // LABEL_IMG_H
