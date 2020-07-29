#include "contractmodel.h"
#include "contractfactory.h"
#include <QDebug>

ContractModel* ContractModel::m_pInstance = nullptr;
ContractModel::ContractModel(QObject *parent)
    : QAbstractListModel(parent)
{
    init();
}
void ContractModel::init()
{
    ContractFactory::getInstance()->loadContract();

    qDebug()<<" count is "<<rowCount();
}
QHash<int, QByteArray> ContractModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "itemName";

    return roles;
}

int ContractModel::rowCount(const QModelIndex &parent) const
{
    return ContractFactory::getInstance()->getContractList()->count();
}

QVariant ContractModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= ContractFactory::getInstance()->getContractList()->count())
        return QVariant();

    if (role == Qt::DisplayRole)
        return ContractFactory::getInstance()->getContractList()->at(index.row())->name;

    if (role == Qt::UserRole + 1)
        return ContractFactory::getInstance()->getContractList()->at(index.row())->name;

    return QVariant();
}

void ContractModel::refresh()
{
    beginResetModel();
    endResetModel();
}

void ContractModel::remove(int row)
{
    if(row < 0 || row >= ContractFactory::getInstance()->getContractList()->count())
        return;
    qDebug()<<"remove2222   "<<row;

    beginRemoveRows(QModelIndex() , row , row);
    ContractFactory::getInstance()->removeContract(row);
    endRemoveRows();
}
