#include "connection.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <iostream>
#include <QTextStream>
#include <QDebug>
#include <mainwindow.h>
#include <QString>
#include <QAxObject>
#include "objbase.h"
#include <qdir.h>
#include <QSqlError>
#include <QDate>

connection::connection()
{

}

/***s: import data of CSV file to sqlite*/
int connection::createConnection(QString inputFilepath,QString outputFilepath)
{
    qDebug()<<"inputFilepath is "<<inputFilepath;
    qDebug()<<"outputFilepath is "<<outputFilepath;

    QFile file(inputFilepath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
//        std::cerr << "Cannot open file for reading: "
//                  << qPrintable(file.errorString()) << std::endl;
        return -1;
    }

    QStringList list;
    list.clear();
    QTextStream inStream(&file);

    //检查数据
    for( QString lineStr; !inStream.atEnd(); )
    {
        lineStr = inStream.readLine();

        if(lineStr.isEmpty())
        {
            continue;
        }
        list  = splitCSVLine(lineStr);

        if(list.join("").isEmpty()) {
            continue;
        }

        if(list.count() != 25)
            return -3;
    }


    QSqlDatabase db;
    if(!QSqlDatabase::database().isValid())
        db = QSqlDatabase::addDatabase("QSQLITE"); //打开一个新连接， 数据库
    else
        db = QSqlDatabase::database(); //已有连接 直接连接

    db.setDatabaseName(outputFilepath);
    if(!db.open())
    {
//        QMessageBox::critical(0 , "Can not open database",
//                              "Unable to establish a database connection.",QMessageBox::Cancel);
//        std::cerr<<"stop!";
        return -2;
    }

    db.transaction();

    QSqlQuery query;

    query.exec("create table userRebateData(id integer primary key"
               ", createDate date, sn varchar, groupCode varchar, groupName varchar"
               ", customerCode varchar, customer varchar, productCode varchar, description varchar"
               ", size varchar, priceLatest varchar, priceActive varchar, actUnits varchar"
               ", installation date, chiCode varchar, dealer varchar, region varchar"
               ", customerName varchar, productFamily varchar, productFamilyHFM varchar"
               ", class varchar, revenues varchar, province varchar, rentSold varchar, hospitalCode varchar"
               ")");


    list.clear();
    inStream.seek(0);

    for( QString lineStr; !inStream.atEnd(); )
    {
        lineStr = inStream.readLine();

        if(lineStr.isEmpty())
        {
            continue;
        }
        list  = splitCSVLine(lineStr);

        if(list.join("").isEmpty()) {
            continue;
        }
        if(list.count() != 25)
            return -3;

        query.prepare("INSERT INTO userRebateData ("
                      "createDate, sn, groupCode, groupName"
                      ", customerCode, customer, productCode, description"
                      ", size, priceLatest, priceActive, actUnits"
                      ", installation, chiCode, dealer, region"
                      ", customerName, productFamily, productFamilyHFM"
                      ", class, revenues, province, rentSold, hospitalCode"
                      ")"
                      " VALUES ("
                      ":createDate ,:sn ,:groupCode ,:groupName"
                      ", :customerCode, :customer, :productCode, :description"
                      ", :size, :priceLatest, :priceActive, :actUnits"
                      ", :installation, :chiCode, :dealer, :region"
                      ", :customerName, :productFamily, :productFamilyHFM"
                      ", :class, :revenues, :province, :rentSold, :hospitalCode"
                      ")");

        QDate tmpDate(list.at(0).toInt() , list.at(1).toInt() , 1);
        query.bindValue(":createDate", tmpDate.toString("yyyy-MM"));
        query.bindValue(":sn", list.at(2).simplified());
        query.bindValue(":groupCode", list.at(3).simplified());
        query.bindValue(":groupName", list.at(4).simplified());
        query.bindValue(":customerCode", list.at(5).simplified());
        query.bindValue(":customer", list.at(6).simplified().toLower());//全部去掉回车空格等以及大写，以防有大小写之分
        query.bindValue(":productCode", list.at(7).simplified());
        query.bindValue(":description", list.at(8).simplified());
        query.bindValue(":size", list.at(9).simplified());
        query.bindValue(":priceLatest", list.at(10).simplified());
        query.bindValue(":priceActive", list.at(11).simplified());
        query.bindValue(":actUnits", list.at(12).simplified());
        query.bindValue(":installation", list.at(13).simplified());
        query.bindValue(":chiCode", list.at(14).simplified());
        query.bindValue(":dealer", list.at(15).simplified().toLower());
        query.bindValue(":region", list.at(16).simplified());
        query.bindValue(":customerName", list.at(17).simplified());
        query.bindValue(":productFamily", list.at(18).simplified());
        query.bindValue(":productFamilyHFM", list.at(19).simplified());
        query.bindValue(":class", list.at(20).simplified());
        query.bindValue(":revenues", list.at(21).simplified());
        query.bindValue(":province", list.at(22).simplified().toLower());
        query.bindValue(":rentSold", list.at(23).simplified());
        query.bindValue(":hospitalCode", list.at(24).simplified());

        query.exec();
    }

    if(!db.commit()){
        qDebug()<<"ERROR commit  "<<db.lastError();
    }
    query.clear();
    db.close();

    qDebug()<<"success";

    return 0;
}

/***Function: save the the path of output file*/
bool  connection::saveUserData(QString filePathName,QString outputFilepath){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(outputFilepath);
    if(!db.open())
    {
        QMessageBox::critical(0 , "Can not open database",
                              "Unable to establish a database connection.",QMessageBox::Cancel);
        std::cerr<<"stop!";
        return false;
    }
    QSqlQuery query;
    query.exec("DELETE FROM userRebateData");

    query.exec("create table userRebateData(id integer primary key"
               ", createDate date, sn varchar, groupCode varchar , groupName varchar"
               ", customerCode varchar , customer varchar , productCode varchar , description varchar"
               ", size varchar , price-latest varchar , price-active varchar , actUnits varchar "
               ", installation date , chiCode varchar , dealer varchar , region varchar "
               ", customerName varchar , productFamily varchar , productFamily-HFM varchar "
               ", class varchar , revenues varchar , province varchar , rentSold varchar , hospitalCode varchar"
               ")");

    query.prepare("INSERT INTO userRebateData (scholarshipPath)"
                  " VALUES (:scholarshipPath)");
    query.bindValue(":scholarshipPath", filePathName);
    query.exec();
    query.clear();
    db.close();
    return true;
}

/***Function: transfer excel file to CSV*/
bool connection::tranExcelToCSV(QString InputExcel,QString OutputCSV){

    //Using QAxObject in a background thread must be initialized first
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", false); //NO show
    excel.setProperty("DisplayAlerts", false);//NO warning
    QAxObject * workBooks = excel.querySubObject("WorkBooks"); //获取工作簿集合

    workBooks->dynamicCall("Open(const QString&)", QString(InputExcel)); //打开excel
    QAxObject *workbook = excel.querySubObject("ActiveWorkBook");//获取当前激活的工作簿

    QAxObject * worksheet = workbook->querySubObject("Worksheets(int)", 1);//获取第一个sheet
    QAxObject *first_sheet = worksheet->querySubObject("Rows(int)", 1);//删除第一行
    first_sheet->dynamicCall("delete");
    worksheet->dynamicCall("SaveAs(const QString&,int)",QDir::toNativeSeparators(OutputCSV),6); //保存成csv
    workbook->dynamicCall("Close (Boolean)", false); //关闭excel
    excel.dynamicCall("Quit(void)");//退出excel
    return true;
}

QStringList connection::splitCSVLine(const QString &lineStr)
{
    QStringList strList;
    QString str;

    int length = lineStr.length();
    int quoteCount = 0;
    int repeatQuoteCount = 0;

    for(int i = 0; i < length; ++i)
    {
        if(lineStr[i] != '\"')
        {
            repeatQuoteCount = 0;
            if(lineStr[i] != ',')
            {
                str.append(lineStr[i]);
            }
            else
            {
                if(quoteCount % 2)
                {
                    str.append(',');
                }
                else
                {
                    strList.append(str);
                    quoteCount = 0;
                    str.clear();
                }
            }
        }
        else
        {
            ++quoteCount;
            ++repeatQuoteCount;
            if(repeatQuoteCount == 4)
            {
                str.append('\"');
                repeatQuoteCount = 0;
                quoteCount -= 4;
            }
        }
    }
    strList.append(str);

    return qMove(strList);
}
