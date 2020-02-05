#include "DatasetProject.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QFile>

#include <QDebug>

namespace {

    auto loadJsonObjectFromFile(QString const& filePath) -> QJsonObject
    {
        QJsonObject jsonObjectProject;
        QFile projectFile(filePath);
        if (projectFile.open(QIODevice::ReadOnly))
        {
            QJsonParseError parseError{};
            jsonObjectProject = QJsonDocument::fromJson(projectFile.readAll(), &parseError).object();
            if (parseError.error)
            {
                qDebug() << "File " << filePath << " is empty, return an empty object";
            }
        }
        return jsonObjectProject;
    }

    bool saveJsonObjectToFile(QString const& filePath, QJsonObject const& jsonObject)
    {
        QFile projectFile(filePath);
        if (projectFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
          return projectFile.write(QJsonDocument{jsonObject}.toJson()) != -1;
        }
        return false;
    }

} /// end namespace anonymous

class DatasetProject::Impl
{
public:
    Impl(QString const& filePath)
      : _projectFilePath{filePath}
      , _projectJsonObject{loadJsonObjectFromFile(_projectFilePath)}
    {
    }

    ~Impl()
    {
        if (!saveJsonObjectToFile(_projectFilePath, _projectJsonObject))
        {
            qDebug() << "An error occured during saving to file";
        }
    }

    auto get(QString const& key) -> QVariantMap
    {
        return _projectJsonObject[key].toObject().toVariantMap();
    }

    void set(QString const& key, QVariantMap const& data)
    {
        _projectJsonObject[key] = QJsonObject::fromVariantMap(data);
    }

private:
    QString     _projectFilePath;
    QJsonObject _projectJsonObject;
};

DatasetProject::DatasetProject() = default;
DatasetProject::~DatasetProject() = default;

void DatasetProject::loadFromFile(const QString &filePath)
{
    _impl = std::make_unique<Impl>(filePath);
}

auto DatasetProject::get(QString const& key, std::function<QVariantMap()>&& defaultGetter) -> QVariantMap
{
    auto data = _impl->get(key);
    if (data.isEmpty())
    {
        data = defaultGetter();
        set(key, data);
    }
    return data;
}

void DatasetProject::set(QString const& key, QVariantMap const& data)
{
    _impl->set(key, data);
}
