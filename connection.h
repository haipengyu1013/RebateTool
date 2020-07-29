#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>

class connection
{
public:
    connection();
    int createConnection(QString importFilepath, QString outputFilepath);

    bool saveUserData(QString filePathName,QString outputFilepath);
    bool tranExcelToCSV(QString InputExcel,QString OutputCSV);
    QStringList  splitCSVLine(const QString &lineStr);
};

#endif // CONNECTION_H
