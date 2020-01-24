#include "BoundingBoxSelector.h"

#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>

#include <cmath>
#include <algorithm>
#include <iomanip>

#if defined(_MSC_VER)
namespace std {

    template<class T>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
        assert(!(hi < lo));
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

} /// end namespace std
#endif

namespace {
    auto toAbsolute(QPointF const& in, QSize const& size) noexcept -> QPoint
    {
        return QPoint{static_cast<int>(in.x() * size.width() + 0.5),
                      static_cast<int>(in.y() * size.height() + 0.5)};
    }

    auto toAbsolute(QRectF const& in, QSize const& size) noexcept -> QRect
    {
        return QRect{static_cast<int>(in.x() * size.width() + 0.5),
                     static_cast<int>(in.y() * size.height() + 0.5),
                     static_cast<int>(in.width() * size.width() + 0.5),
                     static_cast<int>(in.height()* size.height() + 0.5)};
    }

    auto fromAbsolute(QPoint const& in, QSize const& size) noexcept -> QPointF
    {
        return QPointF{static_cast<qreal>(in.x()) / size.width(),
                       static_cast<qreal>(in.y()) / size.height()};
    }

    auto fromAbsolute(QRect const& in, QSize const& size) noexcept -> QRectF
    {
        return QRectF{static_cast<qreal>(in.x()) / size.width(),
                      static_cast<qreal>(in.y()) / size.height(),
                      static_cast<qreal>(in.width()) / size.width(),
                      static_cast<qreal>(in.height()) / size.height()};
    }

    void drawObjectBox(QPainter& painter, QPen& pen, QRect const& rect)
    {
        painter.setPen(pen);
        painter.drawRect(rect);
    }

    void drawObjectBox(QPainter& painter, QRect const& rect, Qt::GlobalColor color, int thickWidth)
    {
        QPen pen;
        pen.setWidth(thickWidth);
        pen.setColor(color);

        drawObjectBox(painter, pen, rect);
    }

    void drawObjectBoxes(QPainter& painter, BoundingBoxSelector::Label::Vector const& boundingBoxes, QVariantMap const& classes, QSize const& size, int thickWidth = 3)
    {
        QPen pen;
        pen.setWidth(thickWidth);

        for (auto const& boundingbox : boundingBoxes)
        {
            auto const color = ((classes.contains(boundingbox.label)) && (classes[boundingbox.label].toList().size() == 2))
                                   ? QColor::fromRgba(static_cast<uint32_t>(classes[boundingbox.label].toList()[1].toInt()))
                                   : QColor(0, 0, 0, 255);
            qDebug() << boundingbox.label << ", " << color;
            pen.setColor((boundingbox.focused) ? Qt::magenta : color);
            drawObjectBox(painter, pen, toAbsolute(boundingbox.box, size));
        }
    }

    void extractClassBoxes(QVariantMap::iterator datasetIt, std::function<void(QString const&, QRectF&&)>&& extractedClassBoxFn)
    {
        if (!datasetIt->isValid())
        {
            return;
        }
        auto const classBoxes = datasetIt.value().toMap();
        for (auto classBoxesIt = classBoxes.begin(); classBoxesIt != classBoxes.end(); ++classBoxesIt)
        {
            for (auto box : classBoxesIt.value().toList())
            {
                auto const boxCoords = box.toList();
                extractedClassBoxFn(classBoxesIt.key(), QRectF{boxCoords[0].toReal(), boxCoords[1].toReal(), boxCoords[2].toReal(), boxCoords[3].toReal()});
            }
        }
    }

} /// end namespace anonymous

auto toTxtExtention(QString const& filePath) -> QString
{
    std::string filePathStl = filePath.toStdString();
    std::string strLabelData = filePathStl.substr(0, filePathStl.find_last_of('.')) + ".txt";

    return QString().fromStdString(strLabelData);
}

BoundingBoxSelector::BoundingBoxSelector(QWidget *parent)
    : QLabel(parent)
{
    init();

    QObject::connect(this, &QLabel::customContextMenuRequested, this, &BoundingBoxSelector::onCustomContextMenuRequested);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void BoundingBoxSelector::onCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)

    if (updateSelectedObjects() &&
        (m_selectedItem != m_objBoundingBoxes.end()))
    {
        m_objBoundingBoxes.erase(m_selectedItem);
        m_selectedItem = m_objBoundingBoxes.end();
        showImage(true);
        return;
    }

    QMenu* menu = new QMenu;
    auto index = 0;
    for (auto it = m_objBoundingBoxes.begin(); it != m_objBoundingBoxes.end(); ++it)
    {
        if (it->focused)
        {
            auto addClass = menu->addAction(QString::number(index++));
            QObject::connect(addClass, &QAction::triggered, this, [&, it, menu, addClass]() {
                m_objBoundingBoxes.erase(it);
                showImage();

                addClass->deleteLater();
                menu->deleteLater();
            });
            QObject::connect(addClass, &QAction::hovered, this, [&, it]() {
                for (auto& item : m_objBoundingBoxes)
                {
                    item.focused = false;
                }
                it->focused = true;

                showImage(true);
            });
            it->focused = false;
        }
    }
    menu->exec(QCursor::pos());
    menu->clear();
}

void BoundingBoxSelector::mouseMoveEvent(QMouseEvent *ev)
{
    setMousePosition(ev->pos());
    showImage();
    emit Mouse_Moved();
}

void BoundingBoxSelector::mousePressEvent(QMouseEvent *ev)
{
    setMousePosition(ev->pos());
    if (!_classes || _classes->isEmpty())
    {
        QMessageBox::information(this, "Error", "Class list is empty, you should create at least one class!");
        return;
    }
    switch (ev->button())
    {
    case Qt::LeftButton:
        if(m_bLabelingStarted == false)
        {
            m_MousePosPrev = m_MousePos;
            m_bLabelingStarted = true;
        }
        else
        {
            auto pointDiff = m_MousePosPrev - m_MousePos;
            auto box = QRectF{std::min(m_MousePosPrev.x(), m_MousePos.x()),
                               std::min(m_MousePosPrev.y(), m_MousePos.y()),
                               fabs(pointDiff.rx()),
                               fabs(pointDiff.ry())};

            if ((box.width() >= 0.01) && (box.height() >= 0.01))
            {
                m_objBoundingBoxes.push_back(BoundingBoxSelector::Label{_focusedClassName, box, false});

                // Adding box to _datasetIt
                auto data = _datasetIt->toMap();
                auto boxesList = data[_focusedClassName].toList();
                boxesList.push_back(QList<QVariant>{box.x(), box.y(), box.width(), box.height()});
                data[_focusedClassName] = boxesList;
                _datasetIt->setValue(data);

                emit datasetIteratorUpdated();
            }
            m_bLabelingStarted = false;
            showImage();
        }
        break;
    default:
        break;
    }
    emit Mouse_Pressed();
}

void BoundingBoxSelector::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    emit Mouse_Release();
}

void BoundingBoxSelector::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_inputImgScaled = m_inputImg.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void BoundingBoxSelector::init()
{
    // TODO: Should be deletedt
    //m_focusedObjectLabel = 0;

    updateMousePosition();
}

void BoundingBoxSelector::updateMousePosition()
{
    // TODO: Should be deleted after test
    //m_objBoundingBoxes.clear();
    m_bLabelingStarted = false;

    setMousePosition(mapFromGlobal(QCursor::pos()));
}

void BoundingBoxSelector::setMousePosition(QPoint&& pos)
{
    // TODO: MAy be clamp can be replaced by this one
    //auto intersected = QRect{pos, pos}.intersected(rect());
    //qDebug() << ", pos: " << pos << ", rect: " << rect() << "intersected: " << intersected;

    pos.rx() = std::clamp(pos.x(), 0, this->width() - 1);
    pos.ry() = std::clamp(pos.y(), 0, this->height() - 1);

    m_MousePos = fromAbsolute(std::move(pos), size());
}

bool BoundingBoxSelector::openImage(QString const& qstrImg)
{
    QImage img(qstrImg);
    if (img.isNull())
    {
        m_inputImg = QImage{};
        return false;
    }

    // TODO: Should be thought if copy needed
    m_inputImg = img.copy().convertToFormat(QImage::Format_RGB888);
    m_inputImgScaled = m_inputImg.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    updateMousePosition();
    return true;
}

void BoundingBoxSelector::showImage(bool menuHandling)
{
    if(m_inputImgScaled.isNull())
    {
        return;
    }

    auto imageOnUi = m_inputImgScaled.copy();

    QPainter painter(&imageOnUi);

    auto const penThick = 3;

    QColor crossLineColor(255, 187, 0);

    drawCrossLine(painter, crossLineColor, penThick);
    if (m_bLabelingStarted == true)
    {
        drawObjectBox(painter, toAbsolute({m_MousePosPrev, m_MousePos}, size()), Qt::magenta, penThick);
    }
    else if (!menuHandling)
    {
        updateSelectedObjects();
    }
    drawObjectBoxes(painter, m_objBoundingBoxes, *_classes, size(), penThick);
    this->setPixmap(QPixmap::fromImage(imageOnUi));
}

bool BoundingBoxSelector::updateSelectedObjects()
{
    auto count = 0;
    for (auto it = m_objBoundingBoxes.begin(); it != m_objBoundingBoxes.end(); ++it)
    {
        it->focused = it->box.contains(m_MousePos);
        if (it->focused)
        {
            count++;
            if (count == 1)
            {
                m_selectedItem = it;
            }
        }
    }
    if (count > 1)
    {
        m_selectedItem = m_objBoundingBoxes.end();
    }
    return count != 0;
}

void BoundingBoxSelector::loadClassBoxes(QVariantMap::iterator datasetIt)
{
    _datasetIt = datasetIt;
    m_objBoundingBoxes.clear();
    extractClassBoxes(_datasetIt, [&](QString const& className, QRectF&& boxRect){
        m_objBoundingBoxes.push_back({className, boxRect, false});
    });
// TODO: Should be deleted after test
//    auto const classBoxes = _datasetIt.value().toMap();
//    for (auto classBoxesIt = classBoxes.begin(); classBoxesIt != classBoxes.end(); ++classBoxesIt)
//    {
//        for (auto box : classBoxesIt.value().toList())
//        {
//            auto const boxCoords = box.toList();
//            auto const boxRect = QRectF{boxCoords[0].toReal(), boxCoords[1].toReal(), boxCoords[2].toReal(), boxCoords[3].toReal()};
//            m_objBoundingBoxes.push_back({classBoxesIt.key(), boxRect, false});
//        }
//    }
}

void BoundingBoxSelector::setFocusObjectLabel(QString const& className)
{
    _focusedClassName = className;
}

bool BoundingBoxSelector::isOpened()
{
    return !m_inputImgScaled.isNull();
}

auto BoundingBoxSelector::crop(QRect rect) -> QImage
{
    return m_inputImgScaled.copy(rect);
}

void BoundingBoxSelector::drawCrossLine(QPainter& painter, QColor color, int thickWidth)
{
    if (m_MousePos == QPointF(0., 0.)) return;

    QPen pen;
    pen.setWidth(thickWidth);
    pen.setColor(color);

    painter.setPen(pen);

    QPoint absolutePoint = toAbsolute(m_MousePos, size());

    painter.drawLine(QPoint(absolutePoint.x(), 0), QPoint(absolutePoint.x(), height() - 1));
    painter.drawLine(QPoint(0, absolutePoint.y()), QPoint(width() - 1, absolutePoint.y()));
}

void BoundingBoxSelector::setClasses(QVariantMap *classes)
{
    _classes = classes;
}

void BoundingBoxSelector::clearAllClassBoxex()
{
    _datasetIt->setValue(QVariant{});
    m_objBoundingBoxes.clear();
    showImage();
}

auto BoundingBoxSelector::importClassBoxesFromAnnotationFile(QString const& labelFilePath, QVariantMap& classList) -> QVariantMap
{
    QVariantMap classBoxesData;
    std::ifstream inputFile(toTxtExtention(labelFilePath).toStdString());
    if(inputFile.is_open())
    {
        qreal inputFileValue;
        QVector<qreal> inputFileValues;
        while(inputFile >> inputFileValue)
        {
            inputFileValues.push_back(inputFileValue);
        }

        for(int i = 0; i < inputFileValues.size(); i += 5)
        {
            try
            {
                auto classIndex = static_cast<int>(inputFileValues.at(i));
                qreal width = inputFileValues.at(i + 3);
                qreal height = inputFileValues.at(i + 4);
                qreal x = inputFileValues.at(i + 1) - width/2.;
                qreal y = inputFileValues.at(i + 2) - height/2.;

                auto found = std::find_if(classList.cbegin(), classList.cend(), [&](auto const& item) {
                  return item.toList()[0] == static_cast<int>(inputFileValues.at(i));
                });
                auto className = (found != classList.cend()) ? found.key() : QString::number(classIndex);
                if (found == classList.cend())
                {
                    QList<QVariant> classData;
                    classData.push_back(classIndex);
                    classData.push_back(-1);
                    classList[className] = classData;
                }
                m_objBoundingBoxes.push_back({className, QRectF{x, y, width, height}, false});

                auto boxesList = classBoxesData[className].toList();
                boxesList.push_back(QList<QVariant>{x, y, width, height});
                classBoxesData[className] = boxesList;
#if 0
                // Adding box to _datasetIt
                auto data = _datasetIt->toMap();
                auto boxesList = data[_focusedClassName].toList();
                boxesList.push_back(QList<QVariant>{x, y, width, height});
                data[_focusedClassName] = boxesList;
                _datasetIt->setValue(data);
#endif
                //emit datasetIteratorUpdated();
            }
            catch (std::out_of_range const& e)
            {
                qDebug() << "loadLabelData: Out of Range error." << e.what();
            }
        }
    }
    return classBoxesData;
}

// TODO: Should be moved to static function of this class or moved to datasetproject
void BoundingBoxSelector::exportClassBoxesToAnnotationFile(QVariantMap::iterator datasetIt, QVariantMap const& classes) const
{
    std::ofstream annotationFile(toTxtExtention(datasetIt.key()).toStdString());
    if (annotationFile.is_open())
    {
        bool isFirst = true;
        extractClassBoxes(datasetIt, [&](QString const& className, QRectF&& boxRect) {
            annotationFile << ((!isFirst) ? "\n" : "")
                           << classes[className].toList()[0].toInt() << " "
                           << std::fixed << std::setprecision(6)
                           << (boxRect.x() + boxRect.width() / 2.) << " "
                           << (boxRect.y() + boxRect.height() / 2.) << " "
                           << boxRect.width() << " "
                           << boxRect.height();
            isFirst = false;
        });
// TODO: Should be moved to dedicated function
//            if(ui->checkBox_cropping->isChecked())
//            {
//                QImage cropped = crop(toAbsolute(objBox.box, m_inputImgScaled.size()));
//                if(!cropped.isNull())
//                {
//                    string strImgFile   = m_imgList.at(m_imgIndex).toStdString();
//                    strImgFile = strImgFile.substr(strImgFile.find_last_of('/') + 1,
//                                                   strImgFile.find_last_of('.') - strImgFile.find_last_of('/') - 1);
//                    cropped.save(QString().fromStdString(strImgFile) + "_cropped_" + QString::number(i) + ".png");
//                }
//            }
    }

    // TODO: May be it is not needed
#if 0
        ui->textEdit_log->setText(qstrOutputLabelData + " saved.");
    }
#endif
}
