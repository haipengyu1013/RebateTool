#include "contractfactory.h"
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QXmlStreamReader>

ContractFactory* ContractFactory::m_pInstance = nullptr;
ContractFactory::ContractFactory(QObject *parent) : QObject(parent)
{
    init();
}
void ContractFactory::init()
{
    m_pContractList = new QList<Contract*>;        // 使用中的灯库
    m_ContractDataPath = "Data/contracts";
}

void ContractFactory::setContractDataPath(const QString _path)
{
    m_ContractDataPath = _path;
    checkContractXMLValid();
}

bool ContractFactory::loadContract()
{
    qDebug()<<"loadContract";
    m_pContractList->clear();
    QDir sourceDir(m_ContractDataPath);

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        QFile file(fileInfo.absoluteFilePath());
        qDebug()<<"file is "<<fileInfo.fileName();

        if(!file.open(QFile::ReadOnly)) {
            qDebug()<<"file not open";
            return false;
        }

        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {
            QXmlStreamReader::TokenType nType = reader.readNext();

            switch (nType) {
            case QXmlStreamReader::StartElement: {  // 开始元素
                QString strElementName = reader.name().toString();
                                    qDebug() << "**********strElementName is "<<strElementName;

                if(strElementName == "Info") {
                                            qDebug() << "**********Info";

                }

                if(strElementName == "Contract") {
                                            qDebug() << "**********Contract";
                    QXmlStreamAttributes contractAttributes = reader.attributes();

                    Contract* _contract = new Contract;
                    _contract->name =  contractAttributes.value("name").toString();
                    _contract->province = contractAttributes.value("province").toString();
                    _contract->dealer = contractAttributes.value("dealer").toString();
                    _contract->customer = contractAttributes.value("customer").toString();
                    _contract->sn = contractAttributes.value("sn").toString();
                    _contract->basedType = static_cast<quint16>(contractAttributes.value("basedType").toInt());
                    _contract->rebateType = static_cast<quint16>(contractAttributes.value("rebateType").toInt());
                    _contract->dateFrom = contractAttributes.value("dateFrom").toString();
                    _contract->dateTo = contractAttributes.value("dateTo").toString();

                    qDebug()<<"name:"<<contractAttributes.value("name").toString()
                           <<"province:"<<contractAttributes.value("province").toString()
                          <<"dealer:"<<contractAttributes.value("dealer").toString()
                         <<"customer:"<<contractAttributes.value("customer").toString()
                        <<"sn:"<<contractAttributes.value("sn").toString()
                       <<"basedType:"<<static_cast<quint16>(contractAttributes.value("basedType").toInt())
                      <<"rebateType:"<<static_cast<quint16>(contractAttributes.value("rebateType").toInt())
                     <<"dateFrom:"<<contractAttributes.value("dateFrom").toString()
                    <<"dateTo:"<<contractAttributes.value("dateTo").toString()
                    <<">>";

                    while (!reader.atEnd()) {
                        reader.readNext();
                        if (reader.isStartElement()) {  // 开始元素
                            QString strElementName = reader.name().toString();
                            qDebug()<<"strElementName is"<<strElementName;

                            if (QString::compare(strElementName, "Params") == 0) {
                                while (!reader.atEnd()) {
                                    reader.readNext();
                                    if (reader.isStartElement()) {  // 开始元素
                                        QString strElementName = reader.name().toString();
                                        if (QString::compare(strElementName, "Param") == 0) {

                                            QXmlStreamAttributes paramAttributes = reader.attributes();

                                            Param _param;
                                            _param.skus = paramAttributes.value("skus").toString();
                                            _param.symbol = paramAttributes.value("symbol").toString();
                                            _param.value = paramAttributes.value("value").toInt();

                                            qDebug()<<"skus:"<<paramAttributes.value("skus").toString()
                                                   <<"symbol:"<<paramAttributes.value("symbol").toString()
                                                  <<"value:"<<paramAttributes.value("value")
                                                 <<">>";

                                            _contract->params.append(_param);
                                        }
                                    } else if (reader.isEndElement()) {  // 结束元素
                                        QString strElementName = reader.name().toString();
                                        if (QString::compare(strElementName, "Params") == 0) {
                                            break;  // 跳出循环
                                        }
                                    }
                                }

                            }
                            if (QString::compare(strElementName, "Formulas") == 0) {
                                while (!reader.atEnd()) {
                                    reader.readNext();
                                    if (reader.isStartElement()) {  // 开始元素
                                        QString strElementName = reader.name().toString();
                                        if (QString::compare(strElementName, "Formula") == 0) {

                                            QXmlStreamAttributes formulaAttributes = reader.attributes();

                                            Formula _formula;
                                            _formula.skus = formulaAttributes.value("skus").toString();
                                            _formula.calculateType = formulaAttributes.value("calculateType").toInt();
                                            _formula.value = formulaAttributes.value("value").toInt();

                                            qDebug()<<"skus:"<<formulaAttributes.value("skus").toString()
                                                   <<"calculateType:"<<formulaAttributes.value("calculateType").toInt()
                                                  <<"value:"<<formulaAttributes.value("value").toInt()
                                                 <<">>";

                                            _contract->formulas.append(_formula);
                                        }
                                    } else if (reader.isEndElement()) {  // 结束元素
                                        QString strElementName = reader.name().toString();
                                        if (QString::compare(strElementName, "Formulas") == 0) {
                                            break;  // 跳出循环
                                        }
                                    }
                                }

                            }

                        } else if (reader.isEndElement()) {  // 结束元素
                            QString strElementName = reader.name().toString();
                            if (QString::compare(strElementName, "Contract") == 0) {
                                break;  // 跳出循环
                            }
                        }
                    }
                    m_pContractList->append(_contract);
                }
                break;
            }
            }
        }
    }
    qDebug()<<"m_pContractList count "<<m_pContractList->count();

    return true;
}

void ContractFactory::checkContractXMLValid()
{
    QDir sourceDir(m_ContractDataPath);
    if(sourceDir.exists()) {
        QFileInfoList fileInfoList = sourceDir.entryInfoList();
        foreach(QFileInfo fileInfo, fileInfoList){
            if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            QFile file(fileInfo.absoluteFilePath());
            qDebug()<<"file is "<<fileInfo.fileName();

            if(!file.open(QFile::ReadOnly)) {
                qDebug()<<"file not open";
                return ;
            }

            QXmlStreamReader reader(&file);

            while (!reader.atEnd()) {
                reader.readNext();
            }

            if (reader.hasError()) {  // 解析出错
                file.close();
                qDebug()<<"添加XML内容"<<reader.errorString();

                //                clearAndCreate(m_ContractDataPath);
            }
        }
    } else {
        sourceDir.mkpath(m_ContractDataPath);
    }
}

void ContractFactory::clearAndCreate(QString _path )
{
    QFile filetmp(_path); //相对路径、绝对路径、资源路径都行
    filetmp.open(QFile::WriteOnly|QFile::Truncate);

    QDomDocument doc;

    //写入xml头部
    QDomProcessingInstruction _instruction; //添加处理命令
    _instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(_instruction);

    //添加根节点
    QDomElement root = doc.createElement("DIASORIN");

    root.setAttribute("stream_vers" , "0");
    root.setAttribute("minor_vers" , "0");
    root.setAttribute("major_vers" , "1");
    doc.appendChild(root);

    //输出到文件
    QTextStream out_stream(&filetmp);
    doc.save(out_stream,4 , QDomNode::EncodingFromTextStream); //缩进4格
    filetmp.flush();
    filetmp.close();
}

void ContractFactory::removeContract(int row)
{
    Contract* tmpContract = m_pContractList->at(row);

    QString _path = m_ContractDataPath +"/" + tmpContract->name + ".xml";
    QFile::remove(_path);

    delete tmpContract;
    tmpContract = nullptr;
    m_pContractList->removeAt(row);
}

void ContractFactory::createNewContract()
{
    Contract* newContractType = new Contract;
    m_pContractList->append(newContractType);
}

void ContractFactory::writeContractXML( Contract* _contract)
{
    if(_contract == nullptr)
        return;

    QDomDocument doctmp;

    //写入xml头部
    QDomProcessingInstruction _instruction; //添加处理命令
    _instruction = doctmp.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doctmp.appendChild(_instruction);

    //添加根节点
    QDomElement root = doctmp.createElement("DIASORIN");

    root.setAttribute("stream_vers" , "0");
    root.setAttribute("minor_vers" , "0");
    root.setAttribute("major_vers" , "1");
    doctmp.appendChild(root);


    QDomElement contranctType = doctmp.createElement("Contract");

    contranctType.setAttribute("name" , _contract->name);
    contranctType.setAttribute("province" , _contract->province);
    contranctType.setAttribute("dealer" , _contract->dealer);

    contranctType.setAttribute("customer" , _contract->customer);
    contranctType.setAttribute("sn" , _contract->sn);
    contranctType.setAttribute("basedType" ,QString::number( _contract->basedType));

    contranctType.setAttribute("rebateType" , QString::number( _contract->rebateType));
    contranctType.setAttribute("dateFrom" , _contract->dateFrom);
    contranctType.setAttribute("dateTo" , _contract->dateTo);

    QDomElement params = doctmp.createElement("Params");

    for (auto _param : _contract->params) {

        QDomElement paramType = doctmp.createElement("Param");
        paramType.setAttribute("skus" , _param.skus);
        paramType.setAttribute("symbol" , _param.symbol);
        paramType.setAttribute("value" , QString::number(_param.value));
        params.appendChild(paramType);
    }
    contranctType.appendChild(params);

    QDomElement formulas = doctmp.createElement("Formulas");

    for (auto _formula : _contract->formulas) {
        QDomElement formulaType = doctmp.createElement("Formula");
        formulaType.setAttribute("skus" , _formula.skus);
        formulaType.setAttribute("calculateType" ,QString::number( _formula.calculateType));
        formulaType.setAttribute("value" , QString::number(_formula.value));
        formulas.appendChild(formulaType);
    }
    contranctType.appendChild(formulas);

    root.appendChild(contranctType);


    QString _path = m_ContractDataPath +"/" + _contract->name + ".xml";
    QFile filetmp(_path); //相对路径、绝对路径、资源路径都行
    filetmp.open(QFile::WriteOnly|QFile::Truncate);

    //输出到文件
    QTextStream out_stream(&filetmp);
    doctmp.save(out_stream,4 , QDomNode::EncodingFromTextStream); //缩进4格
    filetmp.flush();
    filetmp.close();
}

QList<Contract *> *ContractFactory::getContractList()
{
    return m_pContractList;
}

