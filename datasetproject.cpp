#include "datasetproject.h"

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
                throw std::runtime_error(parseError.errorString().toStdString());
            }
            projectFile.close();
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

    auto get(QString&& key) -> QVariantMap
    {
        return _projectJsonObject[key].toObject().toVariantMap();
    }

    void set(QString&& key, QVariantMap const& data)
    {
        _projectJsonObject[key] = QJsonObject::fromVariantMap(data);
    }

private:
    QString     _projectFilePath;
    QJsonObject _projectJsonObject;
};

DatasetProject::DatasetProject() = default;
DatasetProject::~DatasetProject() = default;

bool DatasetProject::loadFromFile(const QString &filePath)
{
    try
    {
        _impl = std::make_unique<Impl>(filePath);
    }
    catch (std::runtime_error const& ex)
    {
        qDebug() << ex.what();
        return false;
    }
    return true;
}

auto DatasetProject::get(QString&& key, std::function<QVariantMap()>&& defaultGetter) -> QVariantMap
{                            
    auto data = _impl->get(std::move(key));
    if (data.isEmpty())
    {
        data = defaultGetter();
        set("dataset_list", data);
    }
    return data;
}

void DatasetProject::set(QString&& key, QVariantMap const& data)
{
    _impl->set(std::move(key), data);
}
