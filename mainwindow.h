#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialog.h"
#include "struct_type.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QSqlTableModel;
class QStandardItemModel;
class QAction;
class QMessageBox;
class QAbstractTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static int const EXIT_CODE_REBOOT;

    void refresh();

public:
    bool getBisRebateForAllRevenue(QDate curDate , QDate contractDate , double monthRevenue , bool bisLessthan);
    bool getBisRebateForRevenue(QDate curDate , QDate contractDate , double monthRevenue , QStringList skuNumber , bool bisLessthan);

    bool getBisRebateForAllVolume(QDate curDate , QDate contractDate , double monthVolume , bool bisLessthan);
    bool getBisRebateForVolume(QDate curDate , QDate contractDate , double monthVolume , QStringList skuNumber , bool bisLessthan);


private slots:
    void on_btnSetting_clicked();

    void on_btnFilter_clicked();

    void on_btnRules_clicked();

    void on_btnImport_clicked();

    void on_btnBrowse_clicked();

    void on_dateEditFrom_userDateChanged(const QDate &date);

    void on_dateEditTo_userDateChanged(const QDate &date);

    void on_btnAdd_clicked();

    void slot_DeleteItem(bool);
    void slot_DeleteItemForRebateListView(bool);
    void slot_DeleteItemForContractListView(bool);
    void slot_SelectItemForContractListView(bool);
    void slot_DisSelectItemForContractListView(bool);

    void on_btnReset_clicked();

    void on_btnExport_clicked();
    
    void on_btnAddSKU_clicked();

    void on_btnCalCulate_clicked();


    void on_BtnCreateNew_clicked();

    void on_BtnSave_clicked();

    void on_btnpercentAdd_clicked();

    void on_btngradsAdd_clicked();

    void on_listViewContract_clicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    void init();
    void createConnections();

    void refreshProList();
    void refreshDealerList();
    void refreshCusList();
    void refreshSnList();
    void refreshDate();

    void refreshFilerParam();

    bool rebateData(Contract* _contract);

private:
    Ui::MainWindow *ui;

    QString fileCSVPath;
    QString activeDb;
    QSqlTableModel * m_pSqlModel;
    Dialog* m_pDialog;

    QStringList strlistPro;
    QStringList strlistDealer;
    QStringList strlistCus;
    QStringList strlistSn;
    QStringList strlistSKU;
    QStringList strlistSKUData;

    QString filerParam;

    QStandardItemModel* m_pItemFilterModel;
    QAction * m_pDeleteAction ;

    QStandardItemModel* m_pItemRebateModel;
    QAction * m_pDeleteActionForRebate ;

    QAction * m_pDeleteActionForContract;
    QAction * m_pSelectAllActionForContract;
    QAction * m_pDisSelectAllActionForContract;


    QList<OutputData> outPutDataList;
    bool bisRebate;

    QMessageBox m_MsgBox;
    QList<QList<QVariant> >* m_varListList;

};
#endif // MAINWINDOW_H
