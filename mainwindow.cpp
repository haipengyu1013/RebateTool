#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include "connection.h"
#include <QDir>
#include <QFileDialog>
#include <QtSql>
#include <QFile>
#include <QTableView>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDate>
#include "xlsxdocument.h"
#include "contractfactory.h"
#include "struct_type.h"
#include "contractmodel.h"
#include <QCompleter>
#include <QAbstractTableModel>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <iostream>
#include <QTextStream>
#include <QDebug>
#include <mainwindow.h>
#include <QString>
#include <QAxObject>
#include "objbase.h"
#include <QSqlError>

const double EPS = 1e-6;

int const MainWindow::EXIT_CODE_REBOOT = -123456789;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    createConnections();
}

void MainWindow::init()
{
    //    QRegExp regx("[0-9]+$");
    //    QValidator *validator = new QRegExpValidator(regx );



    bisRebate = false;
    fileCSVPath = "";
    activeDb = "";
    m_pSqlModel = nullptr;
    m_varListList = new QList<QList<QVariant> >;

    m_pDialog = new Dialog;
    m_pItemFilterModel = new QStandardItemModel;
    ui->listViewRules->setModel(m_pItemFilterModel);

    m_pDeleteAction = new QAction("Delete");
    ui->listViewRules->addAction(m_pDeleteAction);


    m_pItemRebateModel = new QStandardItemModel;
    ui->listViewRebate->setModel(m_pItemRebateModel);

    m_pDeleteActionForRebate = new QAction("Delete");
    ui->listViewRebate->addAction(m_pDeleteActionForRebate);

    m_pDeleteActionForContract = new QAction("Delete");
    m_pSelectAllActionForContract = new QAction("Select All");
    m_pDisSelectAllActionForContract = new QAction("DisSelect All");

    ui->listViewContract->addAction(m_pDeleteActionForContract);
    ui->listViewContract->addAction(m_pSelectAllActionForContract);
    ui->listViewContract->addAction(m_pDisSelectAllActionForContract);

    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->lEtestPercent->setValidator(validator);
    ui->lEnum->setValidator(validator);
    ui->lEPercent->setValidator(validator);
    ui->LEGrads->setValidator(validator);

    if(QFile::exists(QApplication::applicationDirPath() + "/userData.db")){
        QSqlDatabase::addDatabase("QSQLITE").setDatabaseName(QApplication::applicationDirPath() + "/userData.db");
        refresh();
    }

    //    ui->listViewContract.
    ui->listViewContract->setModel(ContractModel::getInstance());

    ui->curDate->setDate(QDate::currentDate());
    ui->labelImportInfo->setVisible(false);

}

void MainWindow::createConnections()
{
    connect(m_pDeleteAction, SIGNAL(triggered(bool)), this, SLOT(slot_DeleteItem(bool)));
    connect(m_pDeleteActionForRebate, SIGNAL(triggered(bool)), this, SLOT(slot_DeleteItemForRebateListView(bool)));
    connect(m_pDeleteActionForContract, SIGNAL(triggered(bool)), this, SLOT(slot_DeleteItemForContractListView(bool)));
    connect(m_pSelectAllActionForContract, SIGNAL(triggered(bool)), this, SLOT(slot_SelectItemForContractListView(bool)));
    connect(m_pDisSelectAllActionForContract, SIGNAL(triggered(bool)), this, SLOT(slot_DisSelectItemForContractListView(bool)));


}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh()
{

    if(m_pSqlModel == nullptr)
        m_pSqlModel = new QSqlTableModel(m_pDialog->getTableView(), QSqlDatabase::database());
    m_pSqlModel->setTable(QSqlDatabase::database().driver()->escapeIdentifier("userRebateData", QSqlDriver::TableName));
    //    model->setFilter(QObject::tr("province= 'admin'").arg("province")); //省份

    //    m_pSqlModel->setFilter("province = 'Liaoning' AND sn = '2210001440'"); //省份
    //    m_pSqlModel->setFilter("Dealer = 'GBO-SUNS'"); //代理商
    //    m_pSqlModel->setFilter("customer = 'BEIJING SUNS-REAL BIOLOGI'"); //客户
    //    m_pSqlModel->setFilter("sn = '2210001607'"); //设备

    m_pSqlModel->select();

    m_pDialog->getTableView()->setModel(m_pSqlModel);

    while (m_pSqlModel->canFetchMore()) //得到整个model数据
        m_pSqlModel->fetchMore();

    qDebug()<<"connectionNames is"<<QSqlDatabase::connectionNames()<<"count is"<<m_pSqlModel->rowCount();

    if (m_pSqlModel->lastError().type() != QSqlError::NoError)
        qDebug()<<m_pSqlModel->lastError().text();
    else if (m_pSqlModel->query().isSelect())
        qDebug()<<tr("Query OK.");
    else
        qDebug()<<tr("Query OK, number of affected rows: %1").arg(
                      m_pSqlModel->query().numRowsAffected());




    refreshProList();
    refreshDealerList();
    refreshCusList();
    refreshSnList();
    refreshDate();
}

bool MainWindow::getBisRebateForAllRevenue(QDate curDate, QDate contractDate, double monthRevenue, bool bisLessthan)
{
    qDebug()<<"getBisRebateForAllRevenue";

    double realRevenue = 0;

    qDebug()<<"curDate is"<<curDate;
    qDebug()<<"contractDate is"<<contractDate;

    //若合同没开始
    if((curDate.year() < contractDate.year()) ||
            (curDate.year() == contractDate.year() && curDate.month() < contractDate.month())) {
        return false;
    }

    //今年消耗的月份
    int tmpCurMonths = 0;
    if(curDate.month() - contractDate.month() >= 0) {
        tmpCurMonths = curDate.month() - contractDate.month() + 1;
    } else {
        tmpCurMonths = 12 +  curDate.month() - contractDate.month() ;

    }

    if(curDate.month() - contractDate.month() >= 0 ) {
        for(int monthIndex = contractDate.month(); monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();

                QString _sku = m_pSqlModel->data(index).toString();

                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                //                double _revenue = m_pSqlModel->data(index2).toString().toDouble();

                if( _year == curDate.year() && monthIndex == _month) {
                    realRevenue = realRevenue + _revenue;
                }

            }
        }

        if(bisLessthan){
            if(realRevenue < monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realRevenue >= monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }

    } else {
        //前半年
        for(int monthIndex = contractDate.month(); monthIndex <= 12; ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                //                double _revenue = m_pSqlModel->data(index2).toString().toDouble();

                if( _year == (curDate.year() - 1) && monthIndex == _month) {
                    realRevenue = realRevenue + _revenue;
                }

            }
        }

        //后半年
        for(int monthIndex = 1; monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();


                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();


                if( _year == curDate.year() && monthIndex == _month) {
                    realRevenue = realRevenue + _revenue;
                }
            }
        }

        if(bisLessthan){
            if(realRevenue < monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realRevenue >= monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }
        //        //第一年
        //        if(contractDate.year() == curDate.year() - 1) {
        //        } else {
        //        }
    }

}

bool MainWindow::getBisRebateForRevenue(QDate curDate , QDate contractDate , double monthRevenue , QStringList skuNumber , bool bisLessthan)
{
    qDebug()<<"getBisRebateForRevenue";

    double realRevenue = 0;

    qDebug()<<"curDate is"<<curDate;
    qDebug()<<"contractDate is"<<contractDate;

    //若合同没开始
    if((curDate.year() < contractDate.year()) ||
            (curDate.year() == contractDate.year() && curDate.month() < contractDate.month())) {
        return false;
    }

    //今年消耗的月份
    int tmpCurMonths = 0;
    if(curDate.month() - contractDate.month() >= 0) {
        tmpCurMonths = curDate.month() - contractDate.month() + 1;
    } else {
        tmpCurMonths = 12 +  curDate.month() - contractDate.month() ;

    }

    if(curDate.month() - contractDate.month() >= 0 ) {
        for(int monthIndex = contractDate.month(); monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue


                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();


                QString _sku = m_pSqlModel->data(index).toString();

                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();


                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex){
                    if(_sku == skuNumber.at(skuIndex).simplified() && _year == curDate.year() && monthIndex == _month) {
                        realRevenue = realRevenue + _revenue;
                    }
                }
            }
        }

        if(bisLessthan){
            if(realRevenue < monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realRevenue >= monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }

    } else {
        //前半年
        for(int monthIndex = contractDate.month(); monthIndex <= 12; ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();




                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();


                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex){
                    if(_sku == skuNumber.at(skuIndex).simplified() && _year == (curDate.year() - 1) && monthIndex == _month) {
                        realRevenue = realRevenue + _revenue;
                    }
                }
            }
        }

        //后半年
        for(int monthIndex = 1; monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                //                QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

                QModelIndex index8 = m_pSqlModel->index(i , 9); //size
                QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
                QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
                double _revenue = m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() * m_pSqlModel->data(index9).toDouble();


                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();

                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex){
                    if(_sku == skuNumber.at(skuIndex).simplified() && _year == curDate.year() && monthIndex == _month) {
                        realRevenue = realRevenue + _revenue;
                    }
                }
            }
        }

        if(bisLessthan){
            if(realRevenue < monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realRevenue >= monthRevenue * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }
        //        //第一年
        //        if(contractDate.year() == curDate.year() - 1) {
        //        } else {
        //        }
    }
}

bool MainWindow::getBisRebateForAllVolume(QDate curDate, QDate contractDate, double monthVolume, bool bisLessthan)
{
    qDebug()<<"getBisRebateForAllVolume";

    double realVolume = 0;

    qDebug()<<"curDate is"<<curDate;
    qDebug()<<"contractDate is"<<contractDate;
    //若合同没开始
    if((curDate.year() < contractDate.year()) ||
            (curDate.year() == contractDate.year() && curDate.month() < contractDate.month())) {
        return false;
    }

    //今年消耗的月份
    int tmpCurMonths = 0;
    if(curDate.month() - contractDate.month() >= 0) {
        tmpCurMonths = curDate.month() - contractDate.month() + 1;
    } else {
        tmpCurMonths = 12 +  curDate.month() - contractDate.month() ;

    }

    if(curDate.month() - contractDate.month()  >= 0 ) {
        for(int monthIndex = contractDate.month(); monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                qDebug()<<"origin date  is "<<m_pSqlModel->data(index1).toString();

                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                qDebug()<<"date  is "<<QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM");

                qDebug()<<"sku is "<<_sku<<" _year is "<<_year<<" _month is "<<_month;
                double _volume = m_pSqlModel->data(index2).toDouble();

                if( _year == curDate.year() && monthIndex == _month) {
                    realVolume = realVolume + _volume;
                }


            }
        }

        qDebug()<<"realVolume is "<<realVolume;

        if(bisLessthan){
            if(realVolume < monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realVolume >= monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }

    } else {
        //前半年
        for(int monthIndex = contractDate.month(); monthIndex <= 12; ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();

                double _volume = m_pSqlModel->data(index2).toDouble();

                if(_year == (curDate.year() - 1) && monthIndex == _month) {
                    realVolume = realVolume + _volume;
                }

            }
        }

        //后半年
        for(int monthIndex = 1; monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                double _volume = m_pSqlModel->data(index2).toDouble();
                if( _year == curDate.year() && monthIndex == _month) {
                    realVolume = realVolume + _volume;
                }

            }
        }

        if(bisLessthan){
            if(realVolume < monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realVolume >= monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }
    }

}

bool MainWindow::getBisRebateForVolume(QDate curDate , QDate contractDate , double monthVolume , QStringList skuNumber , bool bisLessthan)
{
    qDebug()<<"getBisRebateForVolume";

    double realVolume = 0;

    qDebug()<<"curDate is"<<curDate;
    qDebug()<<"contractDate is"<<contractDate;
    //若合同没开始
    if((curDate.year() < contractDate.year()) ||
            (curDate.year() == contractDate.year() && curDate.month() < contractDate.month())) {
        return false;
    }

    //今年消耗的月份
    int tmpCurMonths = 0;
    if(curDate.month() - contractDate.month() >= 0) {
        tmpCurMonths = curDate.month() - contractDate.month() + 1;
    } else {
        tmpCurMonths = 12 +  curDate.month() - contractDate.month() ;

    }

    if(curDate.month() - contractDate.month()  >= 0 ) {
        for(int monthIndex = contractDate.month(); monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                qDebug()<<"origin date  is "<<m_pSqlModel->data(index1).toString();

                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                qDebug()<<"date  is "<<QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM");

                qDebug()<<"sku is "<<_sku<<" _year is "<<_year<<" _month is "<<_month;
                double _volume = m_pSqlModel->data(index2).toDouble();

                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex) {
                    if(_sku != skuNumber.at(skuIndex).simplified())
                        continue;
                    if(_year != curDate.year())
                        continue;
                    if(monthIndex != _month)
                        continue;
                    realVolume = realVolume + _volume;

                }

            }
        }
        if(bisLessthan){
            if(realVolume < monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realVolume >= monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }

    } else {
        //前半年
        for(int monthIndex = contractDate.month(); monthIndex <= 12; ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                double _volume = m_pSqlModel->data(index2).toDouble();
                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex){
                    if(_sku == skuNumber.at(skuIndex).simplified() && _year == (curDate.year() - 1) && monthIndex == _month) {
                        realVolume = realVolume + _volume;
                    }
                }
            }
        }

        //后半年
        for(int monthIndex = 1; monthIndex <= curDate.month(); ++monthIndex) {

            for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
                QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
                QModelIndex index1 = m_pSqlModel->index(i , 1); //date
                QModelIndex index2 = m_pSqlModel->index(i , 12); //volume

                QString _sku = m_pSqlModel->data(index).toString();
                int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
                int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
                double _volume = m_pSqlModel->data(index2).toDouble();
                for(int skuIndex = 0; skuIndex < skuNumber.length(); ++skuIndex){
                    if(_sku == skuNumber.at(skuIndex).simplified() && _year == curDate.year() && monthIndex == _month) {
                        realVolume = realVolume + _volume;
                    }
                }
            }
        }

        if(bisLessthan){
            if(realVolume < monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        } else {
            if(realVolume >= monthVolume * tmpCurMonths + EPS) {
                return true;
            } else
                return false;
        }
    }
}


void MainWindow::on_btnSetting_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_btnFilter_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btnRules_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btnImport_clicked()
{

    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    QFile::remove(QApplication::applicationDirPath() + "/userData.db");

    QDir targetDir(QApplication::applicationDirPath() + "/CSV datas/");
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return;
    } else {
        targetDir.removeRecursively();
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return;
    }
    int ret;
    connection conSaveUserData;

    QString strDir = QCoreApplication::applicationDirPath() + QStringLiteral("\\CSV datas\\");
    QDir dir(strDir);
    if(!dir.exists())
    {
        dir.mkdir(strDir);
    }
    m_MsgBox.setText("导入中");
    m_MsgBox.show();
    QElapsedTimer timer;
    timer.start();

    QString strFileCSVPath = strDir + "rebate" + QDateTime::currentDateTime().toString("_yyyyMMddHHmmss") + ".CSV";
    QString directory = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("Import Rebate Data"),QDir::currentPath(),"EXCEL(*.xlsx)"));


    if(!directory.isEmpty()){
        conSaveUserData.tranExcelToCSV(directory,strFileCSVPath);
        qDebug()<<"open cost:"<<timer.elapsed()<<"ms";timer.restart();

        fileCSVPath = strFileCSVPath;
        ret = conSaveUserData.createConnection(fileCSVPath , QApplication::applicationDirPath() + "/userData.db");
        if(ret == 0){
            ui->lEImport->setText(directory);
            m_MsgBox.setText("导入成功");
            m_MsgBox.exec();
        } else if(ret == -1){
            m_MsgBox.setText("导入失败 , 打不开文件");
            m_MsgBox.exec();
        } else if(ret == -2){
            m_MsgBox.setText("导入失败 ， 连接不了数据库");
            m_MsgBox.exec();
        } else if(ret == -3){
            m_MsgBox.setText("导入失败 ， excel 表格列数不正确");
            m_MsgBox.exec();
        }
    }
    qDebug()<<"open cost:"<<timer.elapsed()<<"ms";timer.restart();

    refresh();
    ui->labelImportInfo->setVisible(false);
    m_MsgBox.hide();
}

void MainWindow::on_btnBrowse_clicked()
{
    m_pDialog->show();
    m_pDialog->getTableView()->setModel(m_pSqlModel);
}

void MainWindow::refreshProList()
{
    strlistPro.clear();
    strlistPro.append("ALL");

    QSqlQueryModel model;
    //    if(filerParam == "")
    model.setQuery("SELECT DISTINCT province FROM userRebateData ORDER BY province");
    //    else
    //        model.setQuery(QString("SELECT DISTINCT province FROM userRebateData") + QString(" Where " +filerParam));

    qDebug()<<"model count is"<<model.rowCount();

    while (model.canFetchMore()) //得到整个model数据
        model.fetchMore();

    for(int i = 0; i < model.rowCount(); ++i) {
        QModelIndex index = model.index(i , 0);

        strlistPro.append(model.data(index).toString());
    }

    qDebug()<<"strlistPro is"<<strlistPro;
    ui->comboxPro->clear();
    ui->comboxPro->addItems(strlistPro);

    ui->comboxPro->setCompleter(new QCompleter(strlistPro, this));  //需要重构因为会出现内存泄漏，若程序启动之后，点击多次import
}

void MainWindow::refreshDealerList()
{
    strlistDealer.clear();
    strlistDealer.append("ALL");

    QSqlQueryModel model;
    //    if(filerParam == "")
    model.setQuery("SELECT DISTINCT Dealer FROM userRebateData ORDER BY Dealer");
    //    else
    //        model.setQuery(QString("SELECT DISTINCT Dealer FROM userRebateData") + QString(" Where " +filerParam));

    while (model.canFetchMore()) //得到整个model数据
        model.fetchMore();

    for(int i = 0; i < model.rowCount(); ++i) {
        QModelIndex index = model.index(i , 0);

        strlistDealer.append(model.data(index).toString());
    }

    qDebug()<<"strlistDealer is"<<strlistDealer;

    ui->comboxDealer->clear();
    ui->comboxDealer->addItems(strlistDealer);
    ui->comboxDealer->setCompleter(new QCompleter(strlistDealer, this));

}

void MainWindow::refreshCusList()
{
    strlistCus.clear();
    strlistCus.append("ALL");

    QSqlQueryModel model;
    //    if(filerParam == "")
    model.setQuery("SELECT DISTINCT customer FROM userRebateData ORDER BY customer");
    //    else
    //        model.setQuery(QString("SELECT DISTINCT customer FROM userRebateData") + QString(" Where " +filerParam));

    while (model.canFetchMore()) //得到整个model数据
        model.fetchMore();

    for(int i = 0; i < model.rowCount(); ++i) {
        QModelIndex index = model.index(i , 0);

        strlistCus.append(model.data(index).toString());
    }

    qDebug()<<"strlistCus is"<<strlistCus;

    ui->comboxCus->clear();
    ui->comboxCus->addItems(strlistCus);
    ui->comboxCus->setCompleter(new QCompleter(strlistCus, this));

}

void MainWindow::refreshSnList()
{
    strlistSn.clear();
    strlistSn.append("ALL");

    QSqlQueryModel model;
    //    if(filerParam == "")
    model.setQuery("SELECT DISTINCT sn FROM userRebateData ORDER BY sn");
    //    else
    //        model.setQuery(QString("SELECT DISTINCT sn FROM userRebateData") + QString(" Where " +filerParam));

    while (model.canFetchMore()) //得到整个model数据
        model.fetchMore();

    for(int i = 0; i < model.rowCount(); ++i) {
        QModelIndex index = model.index(i , 0);

        strlistSn.append(model.data(index).toString());
    }

    qDebug()<<"strlistSn is"<<strlistSn;

    ui->comboxIntru->clear();
    ui->comboxIntru->addItems(strlistSn);

    ui->comboxIntru->setCompleter(new QCompleter(strlistSn, this));

}

void MainWindow::refreshDate()
{
    QDate d1(2000, 1, 1);
    ui->dateEditFrom->setDate(d1);

    QDate d2(2999, 1, 1);
    ui->dateEditTo->setDate(d2);

}

void MainWindow::refreshFilerParam()
{
    //    QString pro;
    //    QString dealer;
    //    QString cus;
    //    QString sn;
    //    QString createDate;

    //    if(ui->comboxPro->currentIndex() == 0 ) {
    //        pro = "";
    //    } else
    //        pro = QString("province = ") + QString("'") + ui->comboxPro->currentText() + "" +QString("'");

    //    if(ui->comboxDealer->currentIndex() == 0 ) {
    //        dealer = "";
    //    } else
    //        dealer = QString("dealer = ") + QString("'") + ui->comboxDealer->currentText() + "" +QString("'");

    //    if(ui->comboxCus->currentIndex() == 0 ) {
    //        cus = "";
    //    } else
    //        cus = QString("customer = ") + QString("'") + ui->comboxCus->currentText() + "" +QString("'");

    //    if(ui->comboxIntru->currentIndex() == 0 ) {
    //        sn = "";
    //    } else
    //        sn = QString("sn = ") + ui->comboxIntru->currentText() + "" ;


    //    createDate = QString("createDate >=") + "'" + ui->dateEditFrom->date().toString("yyyy-MM") + "'"
    //            + " AND " + QString("createDate <=")  + "'" + ui->dateEditTo->date().toString("yyyy-MM") + "'";


    //    filerParam =  pro +( pro != "" && dealer != ""  ? " AND " : "")
    //            + dealer + ( dealer != "" && cus != ""  ? " AND " : "")
    //            + cus + ( cus != "" && sn != ""  ? " AND " : "")
    //            + sn ;

    //    if(filerParam == "")
    //        filerParam = createDate;
    //    else
    //        filerParam = filerParam + " AND " + createDate;

    //    qDebug()<<"filerParam is111"<<filerParam;
    //    m_pSqlModel->setFilter(filerParam);
    //    m_pSqlModel->select();
}

bool MainWindow::rebateData(Contract *_contract)
{
    QList<OutputData> tmpDataList;
    //通过筛选条件筛选数据库
    QString pro;
    QString dealer;
    QString cus;
    QString sn;
    QString createDate;

    if(_contract->province == "ALL" ) {
        pro = "";
    } else
        pro = QString("province = ") + QString("'") + _contract->province + "" +QString("'");

    if(_contract->dealer == "ALL" ) {
        dealer = "";
    } else
        dealer = QString("dealer = ") + QString("'") + _contract->dealer + "" +QString("'");

    if(_contract->customer == "ALL" ) {
        cus = "";
    } else
        cus = QString("customer = ") + QString("'") + _contract->customer + "" +QString("'");

    if(_contract->sn == "ALL" ) {
        sn = "";
    } else
        sn = QString("sn = ") + _contract->sn + "" ;


    createDate = QString("createDate >=") + "'" + _contract->dateFrom + "'"
            + " AND " + QString("createDate <=")  + "'" + _contract->dateTo + "'";


    filerParam =  pro +( pro != "" && dealer != ""  ? " AND " : "")
            + dealer + ( dealer != "" && cus != ""  ? " AND " : "")
            + cus + ( cus != "" && sn != ""  ? " AND " : "")
            + sn ;

    if(filerParam == "")
        filerParam = createDate;
    else
        filerParam = filerParam + " AND " + createDate;

    qDebug()<<"filerPara11m is"<<filerParam<<"  ----"<<m_pSqlModel->rowCount();
    m_pSqlModel->setFilter(filerParam);
    m_pSqlModel->select();

    while (m_pSqlModel->canFetchMore()) //得到整个model数据
        m_pSqlModel->fetchMore();

    qDebug()<<"rowCount  is--------------- "<<m_pSqlModel->rowCount();

    //检查数据完整性 ， CreateDate, size , priceActive , actUnits , revenues
    for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
        //            QModelIndex index = m_pSqlModel->index(i , 21); //skucolumn
        //            qDebug()<<" revenue is "<<m_pSqlModel->data(index);
        //            qDebug()<<" revenue is "<<m_pSqlModel->data(index).toString().trimmed().remove(",").toInt();
        //        qDebug()<<" rowCount  is "<<m_pSqlModel->rowCount();

        QModelIndex index1 = m_pSqlModel->index(i , 1); //date
        qDebug()<<"origin date  is "<<m_pSqlModel->data(index1).toString();
    }


    //判断是否返利
    QList<QString> skuList;
    for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
        QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn

        QString _sku = m_pSqlModel->data(index).toString();
        skuList.append(_sku);
    }

    bisRebate = true;
    if(_contract->rebateType == 0) {//以volume返利

        //判断这个月是否返利
        for(int i = 0 ; i < _contract->params.count(); ++i) {
            QString skus = _contract->params.at(i).skus;  //数据
            bool bisLessThan = _contract->params.at(i).symbol != ">=";//大于等于或者小于


            int _yearVolume = _contract->params.at(i).value * ui->lEtestPercent->text().toUInt() / 100;  //数值

            //获得每个月价格
            double tmpVolume = _yearVolume / 12;

            qDebug()<<"tmpVolume is "<<tmpVolume;
            QStringList tmpSkuNumberList;
            //若不含有+号
            if(!skus.contains("+")) {
                if(skus == "ALL") {
                    bisRebate = getBisRebateForAllVolume(ui->curDate->date(), QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpVolume , bisLessThan);
                    if(!bisRebate)
                        break;
                }else{
                    tmpSkuNumberList.clear();
                    tmpSkuNumberList.append(skus);
                    bisRebate = getBisRebateForVolume(ui->curDate->date(), QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpVolume , tmpSkuNumberList , bisLessThan);
                }

                if(!bisRebate)
                    break;
            } else {
                if(skus.contains("ALL")) {
                    bisRebate = false;
                    break;
                }
                tmpSkuNumberList.clear();
                tmpSkuNumberList = skus.split("+" , QString::SkipEmptyParts);

                bisRebate = getBisRebateForVolume(ui->curDate->date(), QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpVolume , tmpSkuNumberList , bisLessThan);
                if(!bisRebate)
                    break;
            }
        }
    } else {
        //判断这个月是否返利
        for(int i = 0 ; i < _contract->params.count(); ++i) {
            QString skus = _contract->params.at(i).skus;  //数据
            bool bisLessThan = _contract->params.at(i).symbol != ">=";//大于等于或者小于
            int _yearRevenue = _contract->params.at(i).value* ui->lEtestPercent->text().toUInt() / 100;  //数值

            //获得每个月利润
            int tmpRevenue = _yearRevenue / 12;

            QStringList tmpSkuNumberList;


            //若不含有+号
            if(!skus.contains("+")) {
                if(skus == "ALL") {
                    bisRebate = getBisRebateForAllRevenue(ui->curDate->date(),QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpRevenue , bisLessThan);
                    if(!bisRebate)
                        break;
                } else {
                    tmpSkuNumberList.clear();
                    tmpSkuNumberList.append(skus);
                    bisRebate = getBisRebateForRevenue(ui->curDate->date(), QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpRevenue , tmpSkuNumberList , bisLessThan);
                }

                if(!bisRebate)
                    break;

            } else {
                if(skus.contains("ALL")) {
                    bisRebate = false;
                    break;
                }
                tmpSkuNumberList.clear();
                tmpSkuNumberList = skus.split("+" , QString::SkipEmptyParts);

                bisRebate = getBisRebateForRevenue(ui->curDate->date(), QDate::fromString(_contract->dateFrom, "yyyy-MM") , tmpRevenue , tmpSkuNumberList , bisLessThan);
                if(!bisRebate)
                    break;
            }
        }

    }

    qDebug()<<"bisRebate is"<<bisRebate;
    if(bisRebate) {
        //        QMessageBox msgBox;
        //        msgBox.setText("可返利, 点击确定输出返利！");
        //        msgBox.exec();
    } else {
        //        QMessageBox msgBox;
        //        msgBox.setText("该合同，当月不返利！");
        //        msgBox.exec();
        //        return false;
    }

    QDate _contractDate =  QDate::fromString(_contract->dateFrom, "yyyy-MM");
    //计算返利
    for(int formulaIndex = 0; formulaIndex < _contract->formulas.count(); ++formulaIndex) {
        Formula tmpFormula = _contract->formulas.at(formulaIndex);
        tmpDataList.clear();
        if(tmpFormula.skus.isEmpty())
            continue;

        for(int i = 0 ; i < m_pSqlModel->rowCount(); ++i) {
            QModelIndex index = m_pSqlModel->index(i , 7); //skucolumn
            QModelIndex index1 = m_pSqlModel->index(i , 1); //date
            QModelIndex index2 = m_pSqlModel->index(i , 21); //revenue

            QModelIndex index3 = m_pSqlModel->index(i , 24); //hospital code
            QModelIndex index4 = m_pSqlModel->index(i , 17); //customerName hospital name

            QModelIndex index5 = m_pSqlModel->index(i , 14); //dealer code
            QModelIndex index6 = m_pSqlModel->index(i , 15); //dealer
            QModelIndex index7 = m_pSqlModel->index(i , 8); //productName  descroption
            QModelIndex index8 = m_pSqlModel->index(i , 9); //size
            QModelIndex index9 = m_pSqlModel->index(i , 11); //daPrice
            QModelIndex index10 = m_pSqlModel->index(i , 12); //actUnit
            QModelIndex index11 = m_pSqlModel->index(i , 2); //cur sn


            QString _sku = m_pSqlModel->data(index).toString();

            int _year = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").year();
            int _month = QDate::fromString(m_pSqlModel->data(index1).toString() , "yyyy-MM").month();
            double _revenue = m_pSqlModel->data(index9).toDouble() *  m_pSqlModel->data(index10).toDouble() / m_pSqlModel->data(index8).toDouble() ;

            qDebug()<<"_year is"<<_year;
            qDebug()<<"_month is"<<_month;
            qDebug()<<"cur year is"<<ui->curDate->date().year();
            qDebug()<<"cur month is"<<ui->curDate->date().month();

            if(_sku == tmpFormula.skus) {
                qDebug()<<"tmpFormula.skus is"<<tmpFormula.skus;

            }

            if((tmpFormula.skus == "ALL" || _sku == tmpFormula.skus) ) {


                //获得前面月份数据
                if(ui->curDate->date().month() >= _contractDate.month()) { //
                    if(_year == ui->curDate->date().year()) {
                        for(int monthIndex = _contractDate.month(); monthIndex <= ui->curDate->date().month(); monthIndex++) {
                            if(monthIndex == _month) {

                                OutputData _data;
                                _data.contractName = _contract->name;
                                _data.year = QString::number(_year);
                                _data.month = QString::number(_month);
                                _data.basedType = _contract->basedType;
                                _data.sn = m_pSqlModel->data(index11).toString();
                                _data.hsCode = m_pSqlModel->data(index3).toString();
                                _data.hsName = m_pSqlModel->data(index4).toString();
                                _data.dealerCode = m_pSqlModel->data(index5).toString();
                                _data.dealer = m_pSqlModel->data(index6).toString();
                                _data.sku = m_pSqlModel->data(index).toString();
                                _data.productName = m_pSqlModel->data(index7).toString();
                                _data.size = m_pSqlModel->data(index8).toString();
                                _data.daPrice = m_pSqlModel->data(index9).toString();      //price-active
                                _data.dateFrom = _contract->dateFrom;
                                _data.dateTo = _contract->dateTo;
                                _data.yearCycle = QString::number(ui->curDate->date().year() - QDate::fromString(_contract->dateFrom, "yyyy-MM").year() + 1);
                                _data.testvolume = m_pSqlModel->data(index10).toString();           //actUnit
                                _data.revenueamt = QString::number(_revenue);

                                tmpDataList.append(_data);
                            }
                        }

                    }
                } else {
                    if(_year == ui->curDate->date().year() - 1) {
                        for(int monthIndex = _contractDate.month(); monthIndex <= 12; monthIndex++) {
                            if(monthIndex == _month
                                    ) {
                                OutputData _data;
                                _data.contractName = _contract->name;
                                _data.year = QString::number(_year);
                                _data.month = QString::number(_month);
                                _data.basedType = _contract->basedType;
                                _data.sn = m_pSqlModel->data(index11).toString();
                                _data.hsCode = m_pSqlModel->data(index3).toString();
                                _data.hsName = m_pSqlModel->data(index4).toString();
                                _data.dealerCode = m_pSqlModel->data(index5).toString();
                                _data.dealer = m_pSqlModel->data(index6).toString();
                                _data.sku = m_pSqlModel->data(index).toString();
                                _data.productName = m_pSqlModel->data(index7).toString();
                                _data.size = m_pSqlModel->data(index8).toString();
                                _data.daPrice = m_pSqlModel->data(index9).toString();      //price-active
                                _data.dateFrom = _contract->dateFrom;
                                _data.dateTo = _contract->dateTo;
                                _data.yearCycle = QString::number(ui->curDate->date().year() - QDate::fromString(_contract->dateFrom, "yyyy-MM").year() + 1);
                                _data.testvolume = m_pSqlModel->data(index10).toString();           //actUnit
                                _data.revenueamt = QString::number(_revenue);

                                tmpDataList.append(_data);
                            }
                        }

                    }

                    if(_year == ui->curDate->date().year()) {
                        for(int monthIndex = 1; monthIndex <= ui->curDate->date().month(); monthIndex++) {
                            if(monthIndex == _month) {
                                OutputData _data;
                                _data.contractName = _contract->name;
                                _data.year = QString::number(_year);
                                _data.month = QString::number(_month);
                                _data.basedType = _contract->basedType;
                                _data.sn = m_pSqlModel->data(index11).toString();
                                _data.hsCode = m_pSqlModel->data(index3).toString();
                                _data.hsName = m_pSqlModel->data(index4).toString();
                                _data.dealerCode = m_pSqlModel->data(index5).toString();
                                _data.dealer = m_pSqlModel->data(index6).toString();
                                _data.sku = m_pSqlModel->data(index).toString();
                                _data.productName = m_pSqlModel->data(index7).toString();
                                _data.size = m_pSqlModel->data(index8).toString();
                                _data.daPrice = m_pSqlModel->data(index9).toString();      //price-active
                                _data.dateFrom = _contract->dateFrom;
                                _data.dateTo = _contract->dateTo;
                                _data.yearCycle = QString::number(ui->curDate->date().year() - QDate::fromString(_contract->dateFrom, "yyyy-MM").year() + 1);
                                _data.testvolume = m_pSqlModel->data(index10).toString();           //actUnit
                                _data.revenueamt = QString::number(_revenue);


                                tmpDataList.append(_data);
                            }
                        }

                    }

                }

                qDebug()<<" bisRebate  is >>>>>>>>>"<<bisRebate;
            }
        }

        if(tmpDataList.isEmpty())
            continue;

        QList<QString> tmpFormulaSkusList;
        if(tmpFormula.skus == "ALL") {

            for(int i = 0; i < tmpDataList.count(); i++) {
                if(!tmpFormulaSkusList.contains(tmpDataList.at(i).sku.simplified())) {
                    tmpFormulaSkusList.append(tmpDataList.at(i).sku.simplified());
                }
            }
        } else {
            tmpFormulaSkusList.append(tmpFormula.skus);
        }

        for(int tmpFormulaSkusListIndex = 0;tmpFormulaSkusListIndex < tmpFormulaSkusList.count();tmpFormulaSkusListIndex++) {
            QString _tmpFormulaSkus = tmpFormulaSkusList.at(tmpFormulaSkusListIndex).simplified();
            double tmpAmount = 0;
            int tmpIndex  = 0;
            if(bisRebate) {
                qDebug()<<"bisRebate11111111111111111111111111111";
                if(tmpFormula.calculateType == 0) {     //百分比
                    for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                        if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                                && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified()) {
                            tmpDataList[outputIndex].rebatePercent = QString::number(tmpFormula.value) + " %";
                            tmpDataList[outputIndex].rebateGrads = "-";
                            tmpDataList[outputIndex].rebateAmount = QString::number((tmpDataList.at(outputIndex).testvolume.toDouble() / tmpDataList.at(outputIndex).size.toDouble()
                                                                                     * tmpDataList.at(outputIndex).daPrice.toDouble() ) * tmpFormula.value / 100);
                        }
                    }



                    //                    tmpAmount = 0;
                    //                    tmpIndex  = 0;
                    //                    for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                    //                        if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                    //                                && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified()) {
                    //                            tmpAmount = tmpAmount + (tmpDataList.at(outputIndex).testvolume.toDouble() / tmpDataList.at(outputIndex).size.toDouble() * tmpDataList.at(outputIndex).daPrice.toDouble() );
                    //                            tmpIndex = outputIndex;
                    //                        }
                    //                    }

                    //                    tmpDataList[tmpIndex].rebatePercent = QString::number(tmpFormula.value) + " %";
                    //                    tmpDataList[tmpIndex].rebateGrads = "-";


                    //                    tmpDataList[tmpIndex].rebateAmount = QString::number(tmpAmount * tmpFormula.value / 100);

                }

                if(tmpFormula.calculateType == 1) {     //梯度

                    for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                        if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                                && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified())
                        {
                            tmpDataList[outputIndex].rebatePercent = "-";
                            tmpDataList[outputIndex].rebateGrads = QString::number(tmpFormula.value);
                            tmpDataList[outputIndex].rebateAmount = QString::number((tmpDataList.at(outputIndex).testvolume.toDouble() / tmpDataList.at(outputIndex).size.toDouble() * (tmpDataList.at(outputIndex).daPrice.toDouble() - tmpFormula.value) ));
                        }
                    }

                    //                    tmpAmount = 0;
                    //                    tmpIndex  = 0;
                    //                    for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                    //                        if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                    //                                && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified())
                    //                        {
                    //                            tmpAmount = tmpAmount + (tmpDataList.at(outputIndex).testvolume.toDouble() / tmpDataList.at(outputIndex).size.toDouble() * (tmpDataList.at(outputIndex).daPrice.toDouble() - tmpFormula.value) );
                    //                            tmpIndex = outputIndex;
                    //                        }
                    //                    }

                    //                    tmpDataList[tmpIndex].rebatePercent = "-";
                    //                    tmpDataList[tmpIndex].rebateGrads = QString::number(tmpFormula.value);
                    //                    tmpDataList[tmpIndex].rebateAmount = QString::number(tmpAmount);
                }
            }
            else {
                qDebug()<<"bisRebate11111111111    false";

                if(tmpFormula.calculateType == 0) {     //百分比
                    tmpAmount = 0;
                    tmpIndex  = 0;
                    if(!tmpDataList.isEmpty()){
                        for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                            if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                                    && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified()) {
                                tmpIndex = outputIndex;
                            }
                        }
                        tmpDataList[tmpIndex].rebatePercent = QString::number(tmpFormula.value) + " %";
                        tmpDataList[tmpIndex].rebateGrads = "-";
                        tmpDataList[tmpIndex].rebateAmount = "NULL";
                    }

                }
                if(tmpFormula.calculateType == 1) {     //梯度
                    tmpAmount = 0;
                    tmpIndex  = 0;
                    if(!tmpDataList.isEmpty()){
                        for(int outputIndex = 0; outputIndex < tmpDataList.count(); outputIndex++) {
                            if(_tmpFormulaSkus == tmpDataList.at(outputIndex).sku.simplified()
                                    && _contract->name.simplified() == tmpDataList.at(outputIndex).contractName.simplified())
                            {
                                tmpIndex = outputIndex;
                            }
                        }
                        tmpDataList[tmpIndex].rebatePercent = "-";
                        tmpDataList[tmpIndex].rebateGrads = QString::number(tmpFormula.value);
                        tmpDataList[tmpIndex].rebateAmount = "NULL";
                    }
                }
            }
        }


        //将数据添加到output当中

        for(auto item : tmpDataList) {
            outPutDataList.append(item);
        }
    }


    return true;
}

void MainWindow::on_dateEditFrom_userDateChanged(const QDate &date)
{
    qDebug()<<"on_dateEditFrom_userDateChanged  "<<date;
    //    refreshFilerParam();
}


void MainWindow::on_dateEditTo_userDateChanged(const QDate &date)
{
    //    refreshFilerParam();
}


void MainWindow::on_btnAdd_clicked()
{
    if(ui->lEnum->text().isEmpty() || ui->labelLeft->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("条件栏为空，请检查！");
        msgBox.exec();
        return;
    }

    QStandardItem* item = new QStandardItem(ui->labelLeft->text() + "   " +ui->comboxS->currentText() + "   " + ui->lEnum->text());
    item->setData(ui->labelLeft->text() , Qt::UserRole + 1);
    item->setData(ui->comboxS->currentText() , Qt::UserRole + 2);
    item->setData(ui->lEnum->text() , Qt::UserRole + 3);

    m_pItemFilterModel->appendRow(item);
    ui->labelLeft->clear();
}

void MainWindow::slot_DeleteItem(bool checked)
{
    QModelIndexList list = ui->listViewRules->selectionModel()->selectedIndexes();

    for(int i = list.count() - 1; i >= 0 ; --i) {
        m_pItemFilterModel->removeRow(list.at(i).row());
    }
}

void MainWindow::slot_DeleteItemForRebateListView(bool checked)
{
    qDebug()<<"slot_DeleteItemForRebateListView";
    QModelIndexList list = ui->listViewRebate->selectionModel()->selectedIndexes();

    for(int i = list.count() - 1; i >= 0 ; --i) {
        m_pItemRebateModel->removeRow(list.at(i).row());
    }
}

void MainWindow::slot_DeleteItemForContractListView(bool checked)
{
    qDebug()<<"slot_DeleteItemForRebateListView";

    QModelIndexList list = ui->listViewContract->selectionModel()->selectedIndexes();

    for(int i = list.count() - 1; i >= 0 ; --i) {
        ContractModel::getInstance()->remove(list.at(i).row());
    }
}

void MainWindow::slot_SelectItemForContractListView(bool)
{
    ui->listViewContract->selectAll();
}

void MainWindow::slot_DisSelectItemForContractListView(bool)
{
    ui->listViewContract->clearSelection();

}

void MainWindow::on_btnReset_clicked()
{
    qDebug()<<"on_btnReset_clicked";
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    QFile::remove(QApplication::applicationDirPath() + "/userData.db");
    qApp->exit(MainWindow::EXIT_CODE_REBOOT);
}

void MainWindow::on_btnExport_clicked()
{
    QXlsx::Document xlsx;
    xlsx.write("A1", QStringLiteral("ContractName"));
    xlsx.write("B1", QStringLiteral("Oupput \nYear"));
    xlsx.write("C1", QStringLiteral("Oupput \nmonth"));
    xlsx.write("D1", QStringLiteral("Toal \nor SKU"));
    xlsx.write("E1", QStringLiteral("SN number"));
    xlsx.write("F1", QStringLiteral("Hospital \nCode"));
    xlsx.write("G1", QStringLiteral("Hospital \nname"));
    xlsx.write("H1", QStringLiteral("Dealer \ncode"));
    xlsx.write("I1", QStringLiteral("Dealer"));
    xlsx.write("J1", QStringLiteral("SKU \nnumber"));
    xlsx.write("K1", QStringLiteral("Product \nname"));
    xlsx.write("L1", QStringLiteral("Size"));
    xlsx.write("M1", QStringLiteral("DA \nprice"));
    xlsx.write("N1", QStringLiteral("Contract \nstarting \nMM/YY"));
    xlsx.write("O1", QStringLiteral("Contract \nending \nMM/YY"));
    xlsx.write("P1", QStringLiteral("Contract \nyear \ncycle \n(Y1/Y2/Y3/Y4/Y5)"));
    xlsx.write("Q1", QStringLiteral("Rebate \nBase (test \nvolume)"));
    xlsx.write("R1", QStringLiteral("Rebate \nBase (revenue \namt)"));
    xlsx.write("S1", QStringLiteral("Rebate %"));
    xlsx.write("T1", QStringLiteral("Rebate Grads"));
    xlsx.write("U1", QStringLiteral("Rebate \namount"));

    int _rowCount = outPutDataList.count();

    for(int row = 0; row < _rowCount; ++row) {
        OutputData _data = outPutDataList.at(row);
        QString _row = QString::number(row + 2);
        xlsx.write(QString("A%1").arg(_row), _data.contractName);
        xlsx.write(QString("B%1").arg(_row), _data.year);
        xlsx.write(QString("C%1").arg(_row), _data.month);
        xlsx.write(QString("D%1").arg(_row), _data.basedType == 0 ? "total" : "SKU");
        xlsx.write(QString("E%1").arg(_row), _data.sn);
        xlsx.write(QString("F%1").arg(_row), _data.hsCode);
        xlsx.write(QString("G%1").arg(_row), _data.hsName);
        xlsx.write(QString("H%1").arg(_row), _data.dealerCode);
        xlsx.write(QString("I%1").arg(_row), _data.dealer);
        xlsx.write(QString("J%1").arg(_row), _data.sku);
        xlsx.write(QString("K%1").arg(_row), _data.productName);
        xlsx.write(QString("L%1").arg(_row), _data.size);
        xlsx.write(QString("M%1").arg(_row), _data.daPrice);
        xlsx.write(QString("N%1").arg(_row), _data.dateFrom);
        xlsx.write(QString("O%1").arg(_row), _data.dateTo);
        xlsx.write(QString("P%1").arg(_row), _data.yearCycle);
        xlsx.write(QString("Q%1").arg(_row), _data.testvolume);
        xlsx.write(QString("R%1").arg(_row), _data.revenueamt);
        xlsx.write(QString("S%1").arg(_row), _data.rebatePercent);
        xlsx.write(QString("T%1").arg(_row), _data.rebateGrads);
        xlsx.write(QString("U%1").arg(_row), _data.rebateAmount);

    }

    QString modelFileName;

    QString strModelFile = QStringLiteral("\\rebateResult") + ".xlsx";
    modelFileName = QFileDialog::getSaveFileName(this,QStringLiteral("rebateResult"),strModelFile,"EXCEL (*.xlsx)");

    xlsx.setDocumentProperty("title", QStringLiteral("ExcelModel"));
    xlsx.setDocumentProperty("creator", QStringLiteral("Biomart"));
    if (!modelFileName.isNull())
    {
        if(xlsx.saveAs(modelFileName)) {
            QMessageBox msgBox;
            msgBox.setText("导出成功！");
            msgBox.exec();
        }
    }
}

void MainWindow::on_btnAddSKU_clicked()
{
    qDebug()<<"ui->comboxSKU->currentData() is"<<ui->comboxSKU->currentData();
    if(ui->comboxSKU->currentData().toString() == "ALL") {
        if(ui->labelLeft->text().contains("ALL")) {
            QMessageBox msgBox;
            msgBox.setText("条件中已增加ALL");
            msgBox.exec();
            return;
        }
    }
    if(ui->labelLeft->text().isEmpty())
        ui->labelLeft->setText(ui->comboxSKU->currentData().toString());
    else
        ui->labelLeft->setText(ui->labelLeft->text() + " + " + ui->comboxSKU->currentData().toString());
}

void MainWindow::on_btnCalCulate_clicked()
{

    m_MsgBox.setText("计算中");
    m_MsgBox.show();

    QModelIndexList indexlist =  ui->listViewContract->selectionModel()->selectedIndexes();


    outPutDataList.clear();
    for(int i = 0; i < indexlist.count(); ++i ) {
        rebateData(ContractFactory::getInstance()->getContractList()->at(indexlist.at(i).row()));
    }


    m_MsgBox.setText("计算完毕，请点击导出");
    m_MsgBox.exec();
}

void MainWindow::on_BtnCreateNew_clicked()
{

}

void MainWindow::on_BtnSave_clicked()
{
    if(ui->lEContractName->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("合同名称为空！");
        msgBox.exec();
        return;
    }

    if(m_pItemFilterModel->rowCount() == 0) {
        QMessageBox msgBox;
        msgBox.setText("返利条件为空！");
        msgBox.exec();
        return;
    }

    if(m_pItemRebateModel->rowCount() == 0) {
        QMessageBox msgBox;
        msgBox.setText("返利算法为空！");
        msgBox.exec();
        return;
    }

    Contract* _contract = new Contract;
    _contract->name = ui->lEContractName->text().simplified();
    _contract->province = ui->comboxPro->currentText().simplified();
    _contract->dealer = ui->comboxDealer->currentText().simplified();
    _contract->customer = ui->comboxCus->currentText().simplified();
    _contract->sn = ui->comboxIntru->currentText().simplified();
    _contract->basedType = ui->buttonGroup_2->checkedButton()->objectName() == "radioTotal" ? 0 : 1;
    _contract->rebateType = ui->buttonGroup->checkedButton()->objectName() == "radioVol" ? 0 : 1;
    _contract->dateFrom = ui->dateEditFrom->date().toString("yyyy-MM");
    _contract->dateTo = ui->dateEditTo->date().toString("yyyy-MM");

    for(int i = 0; i < m_pItemFilterModel->rowCount(); ++i) {

        Param param;
        QModelIndex index = m_pItemFilterModel->index(i , 0);

        param.skus = m_pItemFilterModel->data(index , Qt::UserRole + 1).toString().simplified();  //数据

        bool bisLessThan = false;
        //大于等于或者小于
        if(m_pItemFilterModel->data(index , Qt::UserRole + 2).toString() == ">=") {
            bisLessThan = false;
        } else {
            bisLessThan = true;
        }
        param.symbol = bisLessThan ? "<" : ">=";

        param.value = m_pItemFilterModel->data(index , Qt::UserRole + 3).toInt();  //数值
        _contract->params.append(param);
    }

    for(int i = 0; i < m_pItemRebateModel->rowCount(); ++i) {

        Formula formula;
        QModelIndex index = m_pItemRebateModel->index(i , 0);

        formula.skus = m_pItemRebateModel->data(index , Qt::UserRole + 1).toString().simplified();  //数据


        formula.calculateType = m_pItemRebateModel->data(index , Qt::UserRole + 2).toInt();

        formula.value = m_pItemRebateModel->data(index , Qt::UserRole + 3).toInt();  //数值
        _contract->formulas.append(formula);
    }

    ContractFactory::getInstance()->writeContractXML(_contract);

    ContractFactory::getInstance()->loadContract();
    ContractModel::getInstance()->refresh();
    QMessageBox msgBox;
    msgBox.setText("保存成功");
    msgBox.exec();
}

void MainWindow::on_btnpercentAdd_clicked()
{
    if(ui->combxSKU1->currentText().isEmpty() || ui->lEPercent->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("条件栏为空，请检查！");
        msgBox.exec();
        return;
    }

    QStringList tmpSkuList;
    if(ui->combxSKU1->currentData().toString().simplified().contains("+")) {
        tmpSkuList = ui->combxSKU1->currentData().toString().simplified().split("+" , QString::SkipEmptyParts);
    } else {
        tmpSkuList.append(ui->combxSKU1->currentData().toString().simplified());
    }
    for(int i = 0;i < tmpSkuList.count(); i++) {
        if(tmpSkuList.at(i).simplified().isEmpty())
            continue;

        QStandardItem* item = new QStandardItem(tmpSkuList.at(i).simplified() + "   " + ui->lEPercent->text() + "%");
        item->setData(tmpSkuList.at(i).simplified() , Qt::UserRole + 1);
        item->setData(0, Qt::UserRole + 2);
        item->setData(ui->lEPercent->text(), Qt::UserRole + 3);

        m_pItemRebateModel->appendRow(item);
    }
}

void MainWindow::on_btngradsAdd_clicked()
{
    if(ui->combxSKU2->currentText().isEmpty() || ui->LEGrads->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("条件栏为空，请检查！");
        msgBox.exec();
        return;
    }
    QStringList tmpSkuList;
    if(ui->combxSKU2->currentData().toString().simplified().contains("+")) {
        tmpSkuList = ui->combxSKU2->currentData().toString().simplified().split("+" , QString::SkipEmptyParts);
    } else {
        tmpSkuList.append(ui->combxSKU2->currentData().toString().simplified());
    }
    qDebug()<<"data is " << ui->combxSKU2->currentData().toString();
    for(int i = 0;i < tmpSkuList.count(); i++) {
        if(tmpSkuList.at(i).simplified().isEmpty())
            continue;

        QStandardItem* item = new QStandardItem(tmpSkuList.at(i).simplified()  + "   " +ui->LEGrads->text() +"Grads");
        item->setData(tmpSkuList.at(i).simplified() , Qt::UserRole + 1);
        item->setData(1, Qt::UserRole + 2);
        item->setData(ui->LEGrads->text(), Qt::UserRole + 3);

        m_pItemRebateModel->appendRow(item);
    }
}


void MainWindow::on_listViewContract_clicked(const QModelIndex &index)
{
    Contract* _pContract = ContractFactory::getInstance()->getContractList()->at(index.row());

    QString _info ;
    QString _name = "name: " + _pContract->name;

    QString _province;
    if(_pContract->province == "0")
        _province = "province: ALL";
    else
        _province = "province: " + _pContract->province;

    QString _dealer;
    if(_pContract->dealer == "0")
        _dealer = "dealer: ALL";
    else
        _dealer = "dealer: " + _pContract->dealer;

    QString _customer;
    if(_pContract->customer == "0")
        _customer = "customer: ALL";
    else
        _customer = "customer: " + _pContract->customer;

    QString _sn;
    if(_pContract->sn == "0")
        _sn = "sn: ALL";
    else
        _sn = "sn: " + _pContract->sn;

    QString _basedType;
    if(_pContract->basedType == 0)
        _basedType = "basedType: Total";
    else
        _basedType = "basedType: SKU";

    QString _rebateType ;
    if(_pContract->rebateType == 0)
        _rebateType = "rebateType: Volume";
    else
        _rebateType = "rebateType: Revenue";

    QString _DateFrom = "dateFrom: " + _pContract->dateFrom;
    QString _DateTo = "dateTo: " + _pContract->dateTo;

    QString _params = "params: ";
    for(int i = 0 ; i < _pContract->params.count(); ++i) {
        QString _tmp = _pContract->params.at(i).skus
                + _pContract->params.at(i).symbol
                + QString::number(_pContract->params.at(i).value);
        _params = _params + "\n" + _tmp;
    }

    QString _formulas = "formulas: ";

    for(int i = 0 ; i < _pContract->formulas.count(); ++i) {
        QString _tmp;
        if(_pContract->formulas.at(i).calculateType == 0)
            _tmp = _pContract->formulas.at(i).skus +" "+ QString::number(_pContract->formulas.at(i).value) + "%";
        else {
            _tmp = _pContract->formulas.at(i).skus +" "+ QString::number(_pContract->formulas.at(i).value) + "grads";
        }
        _formulas = _formulas + "\n" + _tmp;
    }
    _info = _name + "\n"
            + _province + "\n"
            + _dealer + "\n"
            + _customer + "\n"
            + _sn + "\n"
            + _basedType + "\n"
            + _rebateType + "\n"
            + _DateFrom + "\n"
            + _DateTo + "\n"
            + _params + "\n"
            + _formulas + "\n";

    ui->textBrowser->setText(_info);
}

void MainWindow::on_pushButton_clicked()
{
    filerParam = "";
    //    refresh();

    //    refreshProList();
    //    refreshDealerList();
    //    refreshCusList();
    //    refreshSnList();
    refreshDate();
}

void MainWindow::on_pushButton_2_clicked()
{
    m_pItemFilterModel->clear();
    ui->lEnum->clear();
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->lEPercent->clear();
    ui->LEGrads->clear();

    m_pItemRebateModel->clear();
}

void MainWindow::on_pushButton_4_clicked()
{
    QList<QStringList> varRows;
    QDir targetDir(QApplication::applicationDirPath() + "/CSV datas/");
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return;
    } else {
        targetDir.removeRecursively();
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return;
    }
    int ret;
    connection conSaveUserData;

    QString strDir = QCoreApplication::applicationDirPath() + QStringLiteral("\\CSV datas\\");
    QDir dir(strDir);
    if(!dir.exists())
    {
        dir.mkdir(strDir);
    }
    m_MsgBox.setText("导入中");
    m_MsgBox.show();
    QElapsedTimer timer;
    timer.start();

    QString strFileCSVPath = strDir + "rebate" + QDateTime::currentDateTime().toString("_yyyyMMddHHmmss") + ".CSV";
    QString directory = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("Import Filter Data"),QDir::currentPath(),"EXCEL(*.xlsx)"));


    if(!directory.isEmpty()){
        conSaveUserData.tranExcelToCSV(directory,strFileCSVPath);

        QFile file(strFileCSVPath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }

        QStringList list;
        list.clear();
        QTextStream inStream(&file);
        inStream.seek(0);



        for( QString lineStr; !inStream.atEnd(); )
        {
            lineStr = inStream.readLine();

            if(lineStr.isEmpty())
            {
                continue;
            }
            list  = conSaveUserData.splitCSVLine(lineStr);

            if(list.join("").isEmpty()) {
                continue;
            }
            QStringList tmpstrList;
            tmpstrList.append(list.at(0));
            tmpstrList.append(list.at(1));
            tmpstrList.append(list.at(2));
            varRows.append(tmpstrList);
        }
    }


    //加载SKU数据

    strlistSKU.clear();
    strlistSKUData.clear();

    QStringList rowData;

    for(int i = 0; i < varRows.count(); ++i) {
        if(i == 0) {
            strlistSKU.append("ALL");
            strlistSKUData.append("ALL");
        } else {
            rowData = varRows[i];
            if(rowData.count() > 3)
                continue;

            strlistSKU.append(rowData.at(1).simplified());
        }
    }

    ui->comboxSKU->clear();
    ui->comboxSKU->addItems(strlistSKU);
    //    ui->comboxSKU->setCompleter(new QCompleter(strlistSKU, this));
    ui->comboxSKU->setItemData(0 , "ALL");

    for(int i = 0; i < varRows.count(); ++i) {
        if(i == 0)
            continue;
        rowData = varRows[i];

        if(rowData.count() > 3)
            continue;

        ui->comboxSKU->setItemData(i , rowData.at(2).simplified());
        strlistSKUData.append(rowData.at(2).simplified());
    }

    //加载SKU 返利数据
    ui->combxSKU1->clear();
    ui->combxSKU2->clear();

    QStringList tmpstrlistSKU = strlistSKU;

    QStringList tmpstrlistSKUData = strlistSKUData;

    ui->combxSKU1->addItems(tmpstrlistSKU);
    ui->combxSKU1->setCompleter(new QCompleter(tmpstrlistSKU, this));


    for(int i = 0; i < tmpstrlistSKUData.count(); ++i) {

        ui->combxSKU1->setItemData(i , tmpstrlistSKUData.at(i));
    }

    ui->combxSKU2->addItems(tmpstrlistSKU);
    ui->combxSKU2->setCompleter(new QCompleter(tmpstrlistSKU, this));

    for(int i = 0; i < tmpstrlistSKUData.count(); ++i) {

        ui->combxSKU2->setItemData(i , tmpstrlistSKUData.at(i));
    }

    m_MsgBox.setText("导入成功");
    m_MsgBox.exec();

    //    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    //    QAxObject excel("Excel.Application");
    //    excel.setProperty("Visible", false); //NO show
    //    excel.setProperty("DisplayAlerts", false);//NO warning
    //    QAxObject * workBooks = excel.querySubObject("WorkBooks"); //获取工作簿集合
    //    QString directory = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("Import SKU Data"),QDir::currentPath(),"EXCEL(*.xlsx)"));
    //    if(directory.isEmpty())
    //        return;

    //    workBooks->dynamicCall("Open(const QString&)", QString(directory)); //打开excel
    //    QAxObject *workbook = excel.querySubObject("ActiveWorkBook");//获取当前激活的工作簿
    //    QAxObject * worksheet = workbook->querySubObject("Worksheets(int)", 1);//获取第一个sheet
    //    QAxObject *usedRange = worksheet->querySubObject("UsedRange");
    //    if(NULL == usedRange || usedRange->isNull())
    //    {
    //        return ;
    //    }
    //    QVariant var = usedRange->dynamicCall("Value");

    //    QVariantList varRows = var.toList();
    //    if(varRows.isEmpty())
    //    {
    //        return;
    //    }
    //    const int rowCount = varRows.size();


    //    delete usedRange;
    //    excel.dynamicCall("Quit(void)");//退出excel


    //    //加载SKU数据

    //    strlistSKU.clear();
    //    strlistSKUData.clear();

    //    QVariantList rowData;

    //    for(int i = 0; i < rowCount; ++i) {
    //        if(i == 0) {
    //            strlistSKU.append("ALL");
    //            strlistSKUData.append("ALL");
    //        } else {
    //            rowData = varRows[i].toList();
    //            if(rowData.count() > 3)
    //                continue;

    //            strlistSKU.append(rowData.at(1).toString());
    //        }
    //    }

    //    ui->comboxSKU->clear();
    //    ui->comboxSKU->addItems(strlistSKU);
    //    //    ui->comboxSKU->setCompleter(new QCompleter(strlistSKU, this));
    //    ui->comboxSKU->setItemData(0 , "ALL");

    //    for(int i = 0; i < rowCount; ++i) {
    //        if(i == 0)
    //            continue;
    //        rowData = varRows[i].toList();

    //        if(rowData.count() > 3)
    //            continue;

    //        ui->comboxSKU->setItemData(i , rowData.at(2).toString());
    //        strlistSKUData.append(rowData.at(2).toString());
    //    }

    //    //加载SKU 返利数据
    //    ui->combxSKU1->clear();
    //    ui->combxSKU2->clear();

    //    QStringList tmpstrlistSKU = strlistSKU;

    //    QStringList tmpstrlistSKUData = strlistSKUData;

    //    ui->combxSKU1->addItems(tmpstrlistSKU);
    //    ui->combxSKU1->setCompleter(new QCompleter(tmpstrlistSKU, this));


    //    for(int i = 0; i < tmpstrlistSKUData.count(); ++i) {

    //        ui->combxSKU1->setItemData(i , tmpstrlistSKUData.at(i));
    //    }

    //    ui->combxSKU2->addItems(tmpstrlistSKU);
    //    ui->combxSKU2->setCompleter(new QCompleter(tmpstrlistSKU, this));

    //    for(int i = 0; i < tmpstrlistSKUData.count(); ++i) {

    //        ui->combxSKU2->setItemData(i , tmpstrlistSKUData.at(i));
    //    }
}
