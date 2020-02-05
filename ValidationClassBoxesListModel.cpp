#include "ValidationClassBoxesListModel.h"

#include "Utils.h"

#include <QPixmap>
#include <QFileInfo>

ValidationClassBoxesListModel::ValidationClassBoxesListModel(QObject *parent, QList<ClassBoxes>* classBoxes)
    : QAbstractListModel{parent}
    , classBoxesListPtr{classBoxes}
{
}

int ValidationClassBoxesListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return classBoxesListPtr->count();
}

QVariant ValidationClassBoxesListModel::data(const QModelIndex &index, int role) const
{
    if ((index.row() < 0) || (index.row() > classBoxesListPtr->count()))
    {
        return QVariant();
    }
    switch(role)
    {
    case Qt::DisplayRole:
        return QFileInfo(classBoxesListPtr->at(index.row()).filePath).fileName();
    case Qt::DecorationRole:
        return classBoxesListPtr->at(index.row()).pixMap;
    case Qt::UserRole + 1:
        return reinterpret_cast<quint64>(&classBoxesListPtr->at(index.row()));
    default:
        return {};
    }
}

bool ValidationClassBoxesListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    while(count--)
    {
        auto intAsPtrToClassObject = reinterpret_cast<ClassBoxes*>(parent.model()->data(parent, Qt::UserRole + 1).toLongLong());
        classBoxesListPtr->insert(classBoxesListPtr->count(), *intAsPtrToClassObject);
    }
    endInsertRows();
    return true;
}

bool ValidationClassBoxesListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for(auto current = row + count - 1; current >= row; --current)
    {
        classBoxesListPtr->removeAt(current);
    }
    endRemoveRows();
    return true;
}
