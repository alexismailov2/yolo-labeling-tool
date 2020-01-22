#ifndef LABEL_IMG_H
#define LABEL_IMG_H

#include <QObject>
#include <QLabel>
#include <QImage>
#include <QMouseEvent>
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

// TODO: May be should be private
public:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onCustomContextMenuRequested(const QPoint &pos);

signals:
    void Mouse_Moved();
    void Mouse_Pressed();
    void Mouse_Release();
    void datasetIteratorUpdated();

public:
    BoundingBoxSelector(QWidget *parent = nullptr);
    void init();
    bool openImage(QString const&);
    void showImage(bool menuHandling = false);

    void loadClassBoxes(QVariantMap::iterator);
    void setFocusObjectLabel(QString const&);

    bool isOpened();
    auto crop(QRect) -> QImage;

    void setClasses(QVariantMap* classes);
    void clearAllClassBoxex();

    auto importClassBoxesFromAnnotationFile(QString const&, QVariantMap&) -> QVariantMap;
    void exportClassBoxesToAnnotationFile(QVariantMap::iterator, QStringList const&) const;

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

#endif // LABEL_IMG_H
