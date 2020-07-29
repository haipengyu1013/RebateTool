#ifndef STRUCT_TYPE_H
#define STRUCT_TYPE_H
#include <QString>
#include <QStringList>
//struct Province {
//    QString
//};

struct Rules {
// QMap<QString , QMap<QString , QMap<QString , QString> > >
};

struct Param {
    QString skus;
    QString symbol;
    int value;
    Param() {
        skus = "";
        symbol = "";
        value = 0;
    }
};

struct Formula {
    QString skus;
    int calculateType;      //0: 百分比 1： 梯度
    int value;
    Formula() {
        skus = "";
        calculateType = 0;
        value = 0;
    }
};

struct Contract  {
    QString name;
    QString province;
    QString dealer;
    QString customer;
    QString sn;
    int basedType;              //0: total 1:SKU
    int rebateType;             //0: volume 1:revenue
    QString dateFrom;
    QString dateTo;
    QList<Param> params;         //返利条件判断是否返利
    QList<Formula> formulas;       //返利计算结果

    Contract() {
        name = "";
        province = "";
        dealer = "";
        customer = "";
        sn = "";
        sn = "";
        basedType = 0;
        rebateType = 0;
        dateFrom = "";
        dateTo = "";
    }
};

struct OutputData {
    QString contractName;
    QString year;
    QString month;
    int basedType;
    QString sn;
    QString hsCode;
    QString hsName;
    QString dealerCode;
    QString dealer;
    QString sku;
    QString productName;
    QString size;
    QString daPrice;                //price-active
    QString dateFrom;
    QString dateTo;
    QString yearCycle;
    QString testvolume;             //actUnit
    QString revenueamt;
    QString rebatePercent;
    QString rebateGrads;
    QString rebateAmount;

    OutputData() {
        year = "";
        month = "";
        basedType = 0;
        sn = "";
        hsCode = "";
        hsName = "";
        dealerCode = "";
        dealer = "";
        sku = "";
        productName = "";
        size = "";
        daPrice = "";                //price-active
        dateFrom = "";
        dateTo = "";
        yearCycle = "";
        testvolume = "";             //actUnit
        revenueamt = "";
        rebatePercent = "";
        rebateGrads = "";
        rebateAmount = "";
    }
};

#endif // STRUCT_TYPE_H
