#pragma once

#include <QRectF>
#include <QVariantMap>

#include <functional>
#include <cassert>

// TODO: Should be moved to datasetproject class
void extractClassBoxes(QVariantMap::iterator datasetIt, std::function<void(QString const&, QRectF&&)>&& extractedClassBoxFn);

auto toAbsolute(QPointF const& in, QSize const& size) noexcept -> QPoint;
auto toAbsolute(QRectF const& in, QSize const& size) noexcept -> QRect;
auto fromAbsolute(QPoint const& in, QSize const& size) noexcept -> QPointF;
auto fromAbsolute(QRect const& in, QSize const& size) noexcept -> QRectF;

#if defined(_MSC_VER) || (__cplusplus <= 201402L)
namespace std {

    template<class T>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
        assert(!(hi < lo));
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

} /// end namespace std
#endif
