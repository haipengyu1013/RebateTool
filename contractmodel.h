#ifndef CONTRACTMODEL_H
#define CONTRACTMODEL_H

#include <QObject>
#include <QAbstractListModel>

#include "struct_type.h"

class ContractModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static ContractModel* getInstance(){
        if(m_pInstance == nullptr)
            m_pInstance = new ContractModel;
        return m_pInstance;
    }

public:
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void refresh();

    void remove(int row);

signals:

public slots:

private:
    explicit ContractModel(QObject *parent = nullptr);
    void init();

private:
    static ContractModel* m_pInstance;
    QList<Contract> m_list;
};

#endif // CONTRACTMODEL_H
