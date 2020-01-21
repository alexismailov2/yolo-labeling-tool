#pragma once

#include <memory>

#include <QString>
#include <QVariant>

class DatasetProject
{
public:
    DatasetProject();
    ~DatasetProject();
    DatasetProject(DatasetProject&&) noexcept = default;
    DatasetProject& operator=(DatasetProject&&) noexcept = default;

    bool loadFromFile(QString const& filePath);

    auto get(QString&& key, std::function<QVariantMap()>&& defaultGetter = []() -> QVariantMap { return {}; }) -> QVariantMap;
    void set(QString&& key, QVariantMap const& data);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
