#include "BoundingBoxSelector.h"

#include "Utils.h"

#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>

#include <cmath>
#include <algorithm>
#include <iomanip>
#include <fstream>

namespace {

    void drawObjectBox(QPainter& painter, QPen& pen, QRect const& rect)
    {
        painter.setPen(pen);
        painter.drawRect(rect);
    }

    void drawObjectBox(QPainter& painter, QRect const& rect, Qt::GlobalColor color, int thickWidth, Qt::PenStyle penStyle = Qt::SolidLine)
    {
        QPen pen;
        pen.setWidth(thickWidth);
        pen.setColor(color);
        pen.setStyle(penStyle);

        drawObjectBox(painter, pen, rect);
    }

    void drawObjectBoxes(QPainter& painter, BoundingBoxSelector::Label::Vector const& boundingBoxes, QVariantMap const& classes, QSize const& size, int thickWidth = 3, Qt::PenStyle penStyle = Qt::SolidLine)
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
            pen.setStyle(penStyle);
            drawObjectBox(painter, pen, toAbsolute(boundingbox.box, size));
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

    auto& objBoundingBoxes = (_isSelectionBoundingBoxFromDarknet) ? m_objBoundingBoxesFromDarknet : m_objBoundingBoxes;
    if (updateSelectedObjects() &&
        (m_selectedItem != objBoundingBoxes.end()))
    {
        if (_isSelectionBoundingBoxFromDarknet)
        {
          m_objBoundingBoxes.push_back(*m_selectedItem);
        }
        objBoundingBoxes.erase(m_selectedItem);
        m_selectedItem = objBoundingBoxes.end();
        syncClassBoxes();
        showImage(true);
        return;
    }

    QMenu* menu = new QMenu;
    auto index = 0;
    for (auto it = objBoundingBoxes.begin(); it != objBoundingBoxes.end(); ++it)
    {
        if (it->focused)
        {
            auto addClass = menu->addAction(QString::number(index++));
            QObject::connect(addClass, &QAction::triggered, this, [&, it, menu, addClass]() {
                if (_isSelectionBoundingBoxFromDarknet)
                {
                  m_objBoundingBoxes.push_back(*it);
                }
                objBoundingBoxes.erase(it);
                syncClassBoxes();
                showImage();

                addClass->deleteLater();
                menu->deleteLater();
            });
            QObject::connect(addClass, &QAction::hovered, this, [&, it]() {
                for (auto& item : objBoundingBoxes)
                {
                    item.focused = false;
                }
                it->focused = true;

                showImage(true);
            });
            it->focused = false;
        }
    }
    if (!menu->actions().isEmpty())
    {
      menu->exec(QCursor::pos());
      menu->clear();
    }
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
                              std::fabs(pointDiff.rx()),
                              std::fabs(pointDiff.ry())};

            if ((box.width() >= 0.01) && (box.height() >= 0.01))
            {
                m_objBoundingBoxes.push_back(BoundingBoxSelector::Label{_focusedClassName, box, false});

                syncClassBoxes();
// TODO: Should be deleted after test
//                // Adding box to _datasetIt
//                auto data = _datasetIt->toMap();
//                auto boxesList = data[_focusedClassName].toList();
//                boxesList.push_back(QList<QVariant>{box.x(), box.y(), box.width(), box.height()});
//                data[_focusedClassName] = boxesList;
//                _datasetIt->setValue(data);

//                emit datasetIteratorUpdated();
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
    updateMousePosition();
}

void BoundingBoxSelector::updateMousePosition()
{
    m_bLabelingStarted = false;

    setMousePosition(mapFromGlobal(QCursor::pos()));
}

void BoundingBoxSelector::setMousePosition(QPoint&& pos)
{
    // TODO: May be clamp can be replaced by this one
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
    if (_isBoxesFromDarknetVisible)
    {
      drawObjectBoxes(painter, m_objBoundingBoxesFromDarknet, *_classes, size(), penThick, Qt::PenStyle::DotLine);
    }
    this->setPixmap(QPixmap::fromImage(imageOnUi));
}

bool BoundingBoxSelector::updateSelectedObjects()
{
    auto& objBoundingBoxes = (_isSelectionBoundingBoxFromDarknet) ? m_objBoundingBoxesFromDarknet : m_objBoundingBoxes;
    auto count = 0;
    for (auto it = objBoundingBoxes.begin(); it != objBoundingBoxes.end(); ++it)
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
        m_selectedItem = objBoundingBoxes.end();
    }
    return count != 0;
}

void BoundingBoxSelector::loadClassBoxes(QVariantMap::iterator datasetIt)
{
    _datasetIt = datasetIt;
    m_objBoundingBoxes.clear();
    extractClassBoxes(_datasetIt, [&](QString const& className, QRectF&& boxRect) {
        m_objBoundingBoxes.push_back({className, boxRect, false});
    });
}

void BoundingBoxSelector::syncClassBoxes()
{
    if (!_datasetIt->isValid())
    {
        return;
    }

    QMap<QString, QVariant> classBoxes;
    for (auto const& boxObject : m_objBoundingBoxes)
    {
        auto boxesList = classBoxes[boxObject.label].toList();
        boxesList.push_back(QList<QVariant>{boxObject.box.x(), boxObject.box.y(), boxObject.box.width(), boxObject.box.height()});
        classBoxes[boxObject.label] = boxesList;
    }
    _datasetIt->setValue(classBoxes);
    emit datasetIteratorUpdated();
}

void BoundingBoxSelector::setFocusObjectLabel(QString const& className)
{
    _focusedClassName = className;
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
          if (className != "excluded_from_annotation")
          {
            annotationFile << ((!isFirst) ? "\n" : "")
                           << classes[className].toList()[0].toInt() << " "
                           << std::fixed << std::setprecision(6)
                           << (boxRect.x() + boxRect.width() / 2.) << " "
                           << (boxRect.y() + boxRect.height() / 2.) << " "
                           << boxRect.width() << " "
                           << boxRect.height();
            isFirst = false;
          }
        });
    }
}

auto BoundingBoxSelector::getCrops(QVariantMap::iterator datasetIt) const -> QMap<QString, QList<QImage>>
{
    QMap<QString, QList<QImage>> crops;
    QImage image{datasetIt.key()};
    extractClassBoxes(datasetIt, [&](QString const& className, QRectF&& boxRect) {
        auto cropped = image.copy(toAbsolute(std::move(boxRect), image.size()));
        if (!cropped.isNull())
        {
            crops[className].push_back(cropped);
        }
        // TODO: Should be moved to dedicated function
        // auto croppedImageFile = datasetIt.key().toStdString();
        // croppedImageFile = croppedImageFile.substr(croppedImageFile.find_last_of('/') + 1,
        //                                            croppedImageFile.find_last_of('.') - croppedImageFile.find_last_of('/') - 1);
        // cropped.save(QString::fromStdString(croppedImageFile) + "_cropped_" + QString::number(i) + ".png");
    });
    return crops;
}

void BoundingBoxSelector::addClassBoxesFromDarknet(Label::Vector const& boxes)
{
   m_objBoundingBoxesFromDarknet = boxes;
}

void BoundingBoxSelector::boxesFromDarknetVisible(bool enable)
{
  _isBoxesFromDarknetVisible = enable;
}

void BoundingBoxSelector::setSelectionBoundingBoxFromDarknet(bool enable)
{
  _isSelectionBoundingBoxFromDarknet = enable;
}
