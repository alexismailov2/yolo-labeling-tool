#include "Utils.h"

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
