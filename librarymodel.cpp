//#include "librarymodel.h"
//#include "libraryfactory.h"
//#include <QDebug>
//#include <QThread>
//#include <QDir>
//#include "enum_type.h"
//#include "../others/tools.h"
//#include "libraryInusemodel.h"
//#include "module/librarymodulemodel.h"
//#include "rangeset/libraryrangesetmodel.h"
//#include "instance/libraryinstancemodel.h"
//#include "macro/librarymacromodel.h"
//#include <QCoreApplication>

//LibraryModel* LibraryModel::m_pInstance = nullptr;
//LibraryModel::LibraryModel(QObject *parent)
//    : QAbstractListModel(parent)
//{

//}

//void LibraryModel::init()
//{
//    m_list.clear();
//}

//void LibraryModel::remove(int row)
//{
//    if(row < 0 || row >= m_list.count())
//        return;

//    beginRemoveRows(QModelIndex() , row , row);
//    LibraryFactory::getInstance()->removeLibrary(m_list.at(row).libraryType);
//    m_list.removeAt(row);
//    endRemoveRows();

//    LibraryFactory::getInstance()->loadLibraryInUse();
//    LibraryInUseModel::getInstance()->slot_loadData();

//    if(row == curOpenIndex()) {
//        setCurOpenIndex(65535);
//    }
//}

//void LibraryModel::insert(int row)
//{
//    beginInsertRows(QModelIndex() , row , row);
//    QList<LibraryTypeStruct *> * tmpLibraryTypeStructList  = LibraryFactory::getInstance()->getLibraryListFromMemory();
//    if(row < tmpLibraryTypeStructList->count()) {
//        LibraryItem libraryiItem;
//        libraryiItem.libraryType = tmpLibraryTypeStructList->at(row);
//        m_list.insert(row , libraryiItem);
//    }

//    endInsertRows();
//}

//int LibraryModel::importLibrary(const QString &_path)
//{
//    LibraryOperateError ret = LibraryFactory::getInstance()->importLibrary(_path);
//    if(ret == LIBRARYOPERATE_NULL) {
//        return  -1;
//    } else if(ret == LIBRARYOPERATE_ERROR) {
//        return  -2;
//    } else if(ret == LIBRARYOPERATE_REDEFIE) {
//        return  -3;
//    } else {
//        LibraryFactory::getInstance()->loadLibraryInUse();
//        LibraryInUseModel::getInstance()->slot_loadData();
//        return  0;
//    }
//}

//bool LibraryModel::exportLibrary(int row, QVariant _usblist)
//{
//    if (row < 0 || row >= m_list.count())
//        return false;
//    if (m_list.at(row).libraryType->info->manufacturer.isEmpty() ||
//            m_list.at(row).libraryType->info->mode.isEmpty() ||
//            m_list.at(row).libraryType->info->name.isEmpty())
//        return false;

//    QList<QVariant> usblist = _usblist.toList();

//    for(auto str: usblist) {
//        if(str.toString() == "Admin")
//            LibraryFactory::getInstance()->exportLibrary(m_list.at(row).libraryType , qApp->applicationDirPath() + "/Admin/Library");
//        else {
//            LibraryFactory::getInstance()->exportLibrary(m_list.at(row).libraryType , (Tools::getInstance()->usbpath() + str.toString() + "/KINGKONG/Library/"));
//        }

//    }

//    return true;
//}

//void LibraryModel::filterLibrary(QString _name, QString _manufactory)
//{
//    QList<LibraryTypeStruct *> * tmpLibraryTypeStructList  = LibraryFactory::getInstance()->getLibraryListFromMemory();

//    QList<LibraryItem>  mainlist;

//    for(int i = 0; i < tmpLibraryTypeStructList->count(); ++i){
//        LibraryItem libraryiItem;
//        libraryiItem.checked  = false;
//        libraryiItem.libraryType = tmpLibraryTypeStructList->at(i);
//        mainlist.append(libraryiItem);
//    }


//    QList<LibraryItem> list;
//    if(mainlist.count() != 0)
//        list.append(mainlist.at(0));

//    for(auto item : mainlist) {

//        if(item.libraryType->info->name.contains(_name , Qt::CaseInsensitive)
//                && item.libraryType->info->manufacturer.contains(_manufactory , Qt::CaseInsensitive)) {
//            list.append(item);
//        }
//    }

//    beginResetModel();
//    m_list = list;
//    endResetModel();
//}

//quint16 LibraryModel::curOpenIndex() const
//{
//    return m_CurOpenIndex;
//}

//void LibraryModel::setCurOpenIndex(const quint16 &curOpenIndex)
//{
//    m_CurOpenIndex = curOpenIndex;
//    emit curOpenIndexChanged();

//    openLibrary(curOpenIndex);
//}

//QHash<int, QByteArray> LibraryModel::roleNames() const
//{
//    QHash<int, QByteArray> roles;
//    roles[Qt::UserRole + 1] = "itemChecked";
//    roles[Qt::UserRole + 2] = "itemInfo";

//    return roles;
//}

//int LibraryModel::rowCount(const QModelIndex &parent) const
//{
//    return m_list.count();
//}

//QVariant LibraryModel::data(const QModelIndex &index, int role) const
//{
//    if (index.row() < 0 || index.row() >= m_list.count())
//        return QVariant();

//    if (role == Qt::UserRole + 1)
//        return m_list.at(index.row()).checked;
//    if(m_list.at(index.row()).libraryType != nullptr) {
//        if (role == Qt::UserRole + 2) {
//            /*
//             * 获得灯库信息
//             * QString name;                         // 灯具名称
//             * QString mode;                         // 模式
//             * QString manufacturer;                 // 制造商
//             * QString type;                         // 类型
//             * QString beam;                         // 光束
//            */
//            QMap<QString , QVariant> infoMap;
//            infoMap.insert("name",m_list.at(index.row()).libraryType->info->name);
//            infoMap.insert("mode",m_list.at(index.row()).libraryType->info->mode);
//            infoMap.insert("manufacturer",m_list.at(index.row()).libraryType->info->manufacturer);
//            infoMap.insert("type",m_list.at(index.row()).libraryType->info->type);
//            infoMap.insert("beam",m_list.at(index.row()).libraryType->info->beam);
//            infoMap.insert("instanceCount",m_list.at(index.row()).libraryType->instances->count());

//            QList<int> tmplist;
//            if(m_list.at(index.row()).libraryType->instances->count() == 0) {
//                infoMap.insert("channelCount", tmplist.count());
//                return infoMap;
//            }
//            if(m_list.at(index.row()).libraryType->modules->count() == 0) {
//                infoMap.insert("channelCount", tmplist.count());
//                return infoMap;
//            }


//            for(int i = 0; i < m_list.at(index.row()).libraryType->instances->count(); ++i) {
//                int k = m_list.at(index.row()).libraryType->instances->at(i)->module;
//                if(k >=  m_list.at(index.row()).libraryType->modules->count()){
//                    infoMap.insert("channelCount", tmplist.count());
//                    continue;
//                }

//                if(m_list.at(index.row()).libraryType->modules->at(k)->channels->count() == 0){
//                    continue;
//                }

//                for(int j = 0; j < m_list.at(index.row()).libraryType->modules->at(k)->channels->count(); ++j) {
//                    int n = 0;

//                    if(m_list.at(index.row()).libraryType->instances->at(i)->patch > 0 && m_list.at(index.row()).libraryType->modules->at(k)->channels->at(j)->fine > 0)
//                    {
//                        n = m_list.at(index.row()).libraryType->instances->at(i)->patch + m_list.at(index.row()).libraryType->modules->at(k)->channels->at(j)->fine - 1;
//                        while (tmplist.count() < n)
//                        {
//                            tmplist.append(-1);
//                        }
//                        tmplist[n - 1] = i;
//                    }
//                    if(m_list.at(index.row()).libraryType->instances->at(i)->patch > 0 && m_list.at(index.row()).libraryType->modules->at(k)->channels->at(j)->coarse > 0)
//                    {
//                        n = m_list.at(index.row()).libraryType->instances->at(i)->patch + m_list.at(index.row()).libraryType->modules->at(k)->channels->at(j)->coarse - 1;
//                        while (tmplist.count() < n)
//                        {
//                            tmplist.append(-1);
//                        }
//                        tmplist[n - 1] = i;
//                    }
//                }
//            }

//            infoMap.insert("channelCount", tmplist.count());
//            return infoMap;
//        }
//    }

//    return QVariant();
//}


//void LibraryModel::update()
//{

//    qDebug()<<"LibraryModel::update()";

//}

//void LibraryModel::openLibrary(int row)
//{
//    if(row < 0 || row >= m_list.count() || row == 65535){
//        LibraryModuleModel::getInstance()->clear();
//        LibraryRangeSetModel::getInstance()->clear();

//        LibraryInstanceModel::getInstance()->clear();
//        LibraryMacroModel::getInstance()->clear();
//        return;
//    }

//    LibraryModuleModel::getInstance()->update(m_list.at(row).libraryType);
//    LibraryRangeSetModel::getInstance()->update(m_list.at(row).libraryType);
//    LibraryInstanceModel::getInstance()->update(m_list.at(row).libraryType);
//    LibraryMacroModel::getInstance()->update(m_list.at(row).libraryType);
//}

//void LibraryModel::slot_loadData()
//{
//    qDebug()<<"void LibraryModel::slot_loadData()";

//    init();
//    LibraryFactory::getInstance()->createNewLibrary();
//    QList<LibraryTypeStruct *> * tmpLibraryTypeStructList  = LibraryFactory::getInstance()->getLibraryListFromMemory();

//    qDebug()<<"tmpLibraryTypeStructList->count() is "<<tmpLibraryTypeStructList->count();
//    QList<LibraryItem>  list;

//    for(int i = 0; i < tmpLibraryTypeStructList->count(); ++i){
//        LibraryItem libraryiItem;
//        libraryiItem.checked  = false;
//        libraryiItem.libraryType = tmpLibraryTypeStructList->at(i);
//        list.append(libraryiItem);
//    }

//    beginResetModel();
//    m_list = list;
//    endResetModel();

//    emit loadlibraryDataFinished();
//}

//QVariant LibraryModel::value(int row, const QString &role , int moduleindex , int moduleRegularindex , int rangeSetIndex, int macroIndex)
//{
//    if (row < 0 || row >= m_list.count() || moduleindex < 0 || moduleRegularindex < 0 || rangeSetIndex < 0 || macroIndex < 0)
//        return "";

//    if (role == "itemChecked")
//        return m_list.at(row).checked;
//    if (role == "itemInfo") {
//        QMap<QString , QVariant> infoMap;
//        infoMap.insert("name",m_list.at(row).libraryType->info->name);
//        infoMap.insert("mode",m_list.at(row).libraryType->info->mode);
//        infoMap.insert("manufacturer",m_list.at(row).libraryType->info->manufacturer);
//        infoMap.insert("type",m_list.at(row).libraryType->info->type);
//        infoMap.insert("beam",m_list.at(row).libraryType->info->beam);
//        infoMap.insert("instanceCount",m_list.at(row).libraryType->instances->count());
//        infoMap.insert("channelCount", LibraryFactory::getInstance()->getChannelCount(row));

//        return infoMap;
//    }

//    return QVariant();
//}

//void LibraryModel::setValue(int row, const QString &role, const QVariant &value)
//{
//    qDebug()<<"role is " << role << "value is " << value<<"row is "<<row;
//    if (row < 0 || row >= m_list.count())
//        return;

//    if (row < 0 || row >= m_list.count())
//        return;

//    LibraryItem item = m_list.at(row);

//    if (role == "itemChecked") {
//        item.checked = value.toBool();
//    }
//    if(item.libraryType != nullptr) {
//        if (role == "itemInfo") {
//            /*
//             * 获得灯库信息
//             * QString name;                         // 灯具名称
//             * QString mode;                         // 模式
//             * QString manufacturer;                 // 制造商
//             * QString type;                         // 类型
//             * QString beam;                         // 光束
//            */
//            QMap<QString , QVariant> tmpMap = value.toMap();
//            item.libraryType->info->name = tmpMap.value("name").toString();
//            item.libraryType->info->mode = tmpMap.value("mode").toString();
//            item.libraryType->info->manufacturer = tmpMap.value("manufacturer").toString();
//            item.libraryType->info->type = tmpMap.value("type").toString();
//            item.libraryType->info->beam = tmpMap.value("beam").toString();
//        }
//    }


//    m_list.replace(row, item);

//    emit dataChanged(index(row), index(row));
//}

//bool LibraryModel::createNewLibrary()
//{
//    qDebug()<<"bool LibraryModel::createNewLibrary()";

//    if( LibraryOperateError::LIBRARYOPERATE_ERROR == LibraryFactory::getInstance()->createNewLibrary())
//        return false;
//    else {
//        insert(rowCount());
//        setCurOpenIndex(rowCount() - 1);
//        return true;
//    }
//}

//int LibraryModel::saveLibrary()
//{
//    int row = curOpenIndex();
//    if (row < 0 || row >= m_list.count())
//        return 0;

//    LibraryItem item = m_list.at(row);
//    qDebug()<<"row is "<<row<<"count is "<<LibraryFactory::getInstance()->getLibraryListFromMemory()->count();

//    LibraryOperateError ret =  LibraryFactory::getInstance()->modifyLibraryToXML(item.libraryType);
//    if( LibraryOperateError::LIBRARYOPERATE_ERROR ==  ret)
//        return 0;
//    else if( LibraryOperateError::LIBRARYOPERATE_REDEFIE ==  ret)
//        return 1;
//    else if( LibraryOperateError::LIBRARYOPERATE_NULL == ret)
//        return 2;
//    else{
//        LibraryFactory::getInstance()->loadLibraryInUse();
//        LibraryInUseModel::getInstance()->slot_loadData();
//        return 3;
//    }
//}

//void LibraryModel::removeSelectedItems()
//{

//    for(int i = 0; i < m_list.count(); ++i) {
//        if(m_list.at(i).checked)
//            remove(i);
//    }
//}

