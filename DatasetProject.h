#pragma once

#include <memory>

#include <QString>
#include <QVariant>

class DatasetProject
{
public:
    enum class eItemType
    {
        TRAIN = 0,
        VALID,
        EXCLUDED
    };

public:
    DatasetProject();
    ~DatasetProject();
    DatasetProject(DatasetProject&&) noexcept = default;
    DatasetProject& operator=(DatasetProject&&) noexcept = default;

    void loadFromFile(QString const& filePath);

    auto get(QString const& key, std::function<QVariantMap()>&& defaultGetter = []() -> QVariantMap { return {}; }) -> QVariantMap;
    void set(QString const& key, QVariantMap const& data);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
