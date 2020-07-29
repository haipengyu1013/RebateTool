#ifndef CONTRACTFACTORY_H
#define CONTRACTFACTORY_H

#include <QObject>
#include <QDomDocument>
#include "struct_type.h"

class ContractFactory : public QObject
{
    Q_OBJECT
public:

    static ContractFactory* getInstance() {
        if(m_pInstance == nullptr)
            m_pInstance = new ContractFactory;
        return m_pInstance;
    }

    /************************************灯库初始化加载操作***********************************************/
    void init(); //初始化获得灯库信息
    void setContractDataPath(const QString _path);
    bool loadContract();
    void checkContractXMLValid();
    void clearAndCreate(QString _path );


//    bool addLibraryToXML( LibraryTypeStruct* _library);
//    bool removeLibrary(LibraryTypeStruct* _library);
//    bool modifyLibraryToXML(LibraryTypeStruct* _library);
    void removeContract(int row);
    void createNewContract();
    void writeContractXML(Contract* _contract);

    QList<Contract*>* getContractList();


signals:

private:
    explicit ContractFactory(QObject *parent = nullptr);
    static ContractFactory* m_pInstance;

    QList<Contract*>* m_pContractList;           // 在内存里的灯库


    QString m_ContractDataPath;

    QDomDocument m_ContractDoc;

};

#endif // CONTRACTFACTORY_H
