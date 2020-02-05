#pragma once

#include <QAbstractListModel>
#include <QRectF>
#include <QPixmap>

struct ClassBoxes
{
    QVariantMap::iterator it;
    QString filePath;
    QRectF boxCoords;
    QPixmap pixMap;
};

class ValidationClassBoxesListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ValidationClassBoxesListModel(QObject *parent = nullptr, QList<ClassBoxes>* classBoxes = nullptr);

    auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    QList<ClassBoxes>* classBoxesListPtr;
};
