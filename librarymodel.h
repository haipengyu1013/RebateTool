//#ifndef LIBRARYMODEL_H
//#define LIBRARYMODEL_H

//#include <QObject>
//#include <QAbstractListModel>

//#include "struct_type.h"

//struct LibraryItem
//{
//    bool checked = false;
//    LibraryTypeStruct* libraryType;
//    LibraryItem() {
//        libraryType = nullptr;
//    }
//};

//enum LibrarySaveError
//{
//    LIBRARYSAVE_ERROR,
//    LIBRARYSAVE_REDIFINE ,
//    LIBRARYSAVE_NULL,
//    LIBRARYSAVE_SUCCESS
//};

//class LibraryModel : public QAbstractListModel
//{
//    Q_OBJECT
//    Q_PROPERTY(quint16 curOpenIndex READ curOpenIndex WRITE setCurOpenIndex NOTIFY curOpenIndexChanged)

//public:
//    static LibraryModel* getInstance(){
//        if(m_pInstance == nullptr)
//            m_pInstance = new LibraryModel;
//        return m_pInstance;
//    }

//public:
//    QHash<int, QByteArray> roleNames() const;
//    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

//public:
//    Q_INVOKABLE QVariant value(int row, const QString &role , int moduleindex = 0 , int modulerangeindex = 0 , int rangeSetIndex = 0 , int macroIndex = 0);
//    Q_INVOKABLE void setValue(int row, const QString &role, const QVariant &value);

//    Q_INVOKABLE bool createNewLibrary();
//    Q_INVOKABLE int saveLibrary();
//    Q_INVOKABLE void removeSelectedItems();

//    Q_INVOKABLE void remove(int row);
//    Q_INVOKABLE void insert(int row);

//    //***************导入导出*******************//
//    Q_INVOKABLE int importLibrary(const QString& _path);
//    Q_INVOKABLE bool exportLibrary(int row , QVariant _usblist);

//    //***************过滤*******************//
//    Q_INVOKABLE void filterLibrary(QString _name , QString _manufactory);

//    Q_INVOKABLE void update();

//    Q_INVOKABLE void openLibrary(int row);
//public:
//    quint16 curOpenIndex() const;
//    void setCurOpenIndex(const quint16 &curOpenIndex);

//signals:
//    void loadlibraryDataFinished();
//    void curOpenIndexChanged();

//public slots:
//    void slot_loadData();

//private:
//    explicit LibraryModel(QObject *parent = nullptr);
//    void init();

//private:
//    static LibraryModel *m_pInstance;
//    QList<LibraryItem> m_list;

//    quint16 m_CurOpenIndex;
//};

//#endif // LIBRARYMODEL_H
