#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "QFileHelper.h"
#include "StaticValue.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

#if _MSC_VER >= 1600

#pragma execution_character_set("utf-8")

#endif

namespace {

bool ensureRouteColumn(QSqlDatabase &db, const QString &columnName)
{
    QSqlQuery query(db);
    if (!query.exec("PRAGMA table_info(route)"))
    {
        qDebug() << "Read route schema failed:" << query.lastError().text();
        return false;
    }

    while (query.next())
    {
        if (query.value(1).toString() == columnName)
        {
            return true;
        }
    }

    QSqlQuery alter(db);
    const QString sql = QString("ALTER TABLE route ADD COLUMN %1 INTEGER DEFAULT 0")
                            .arg(columnName);
    if (!alter.exec(sql))
    {
        qDebug() << "Migrate route schema failed:" << sql << alter.lastError().text();
        return false;
    }

    qDebug() << "Added missing route column:" << columnName;
    return true;
}

bool ensureDatabaseSchema(QSqlDatabase &db)
{
    return ensureRouteColumn(db, "ClockStyle")
        && ensureRouteColumn(db, "TimeLeadValue");
}

} // namespace

void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    QString txt;
    switch (type)
    {
    case QtDebugMsg:     //调试信息提示
        txt = QString("Debug: %1\n").arg(msg);
        break;
    case QtWarningMsg:  //一般的warning提示
        txt = QString("Warning: %1\n").arg(msg);
        break;
    case QtCriticalMsg:    //严重错误提示
        txt = QString("Critical: %1\n").arg(msg);
        break;
    case QtFatalMsg:      //致命错误提示
        txt = QString("Fatal: %1\n").arg(msg);
        abort();
    default:
        break;
    }
    QFile outFile("music_water_Log.txt");
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream ts(&outFile);
        ts << txt << "\r\n";
    }
}
void toHTTP(bool start)
{
//    QNetworkRequest request;
//    request.setUrl(QUrl("http://qt-project.org"));
//    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

//    QNetworkReply *reply = manager->get(request);
//    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
//            this, SLOT(slotError(QNetworkReply::NetworkError)));
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
//            this, SLOT(slotSslErrors(QList<QSslError>)));
}

//一小段测试程序，不会被调用
void testDomino()
{
    QList<QList<int>> show_teams;
    for(int i = 0; i < 10; ++i)
    {
        QList<int> show_team;
        show_team.append(i);
        show_teams.append(show_team);
    }

    for(int j = 0 ; j < 100000; j+=100)
    {
        int iHeartTime = j;
        int loop_times = -1;
        int delay_time = 100;
        int keep_time = 1000;
        int close_perial_time = keep_time + (delay_time * (show_teams.size() - 1));
        int show_perial_time = delay_time * show_teams.size();//周期时间为延迟时间乘以表演编队的个数
        int current_loop_time = iHeartTime/show_perial_time;    //循环了多少个周期
        if(loop_times != -1 && current_loop_time >= loop_times)  //无线循环和超出循环次数时
        {
            return;
        }
        int iShowHeartTime = iHeartTime%show_perial_time;
        int show_team_id = iShowHeartTime/delay_time; //显示第几个编队

        int close_team_id = -1;
        int iCloseHeartTime = iHeartTime%close_perial_time;
        if(iHeartTime != 0)
        {
            if(iCloseHeartTime == 0)
                close_team_id = show_teams.size() - 1;
            else if(iCloseHeartTime >= keep_time)
                close_team_id = (iCloseHeartTime - keep_time) / delay_time;
        }
//        if(iHeartTime % close_perial_time == 0 && iHeartTime != 0)
//            close_team_id = show_teams.size() - 1;
//        else
//        {
//            if(iHeartTime >= keep_time)
//                close_team_id = (iHeartTime-keep_time)/keep_time;//关闭第几个编队
//        }



        if(close_team_id >= show_teams.size())
        {
            qDebug() << "close_team_id is large";
            return;
        }

        if(close_team_id != -1)
        {
            QList<int> close_device_list = show_teams.at(close_team_id);
            for(int k = 0; k < close_device_list.size(); ++k)
            {
                int device_id = close_device_list.at(k);
                qDebug() << "close_team_id:"<<device_id;
//                int device_id = close_device_list.at(j);
//                int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
//                if(device_state > 0)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
//                {
//                    int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
//                    StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
//                    StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
//                }
            }
        }

        if(show_team_id < show_teams.size() && show_team_id != close_team_id)
        {
            QList<int> show_device_list = show_teams.at(show_team_id);
            for(int k = 0; k < show_device_list.size(); ++k)
            {
                int device_id = show_device_list.at(k);
                qDebug() << "show_team_id:"<<device_id;
//                int device_id = show_device_list.at(i);
//                int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
//                if(device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
//                {
//                    int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
//                    StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
//                    StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 1;
//                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
	//if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)){
	//	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	//}

    QApplication a(argc, argv);
    QApplication::addLibraryPath("./plugins");
    QTranslator oTranslator;
    const bool translatorLoaded = oTranslator.load(":/qt_zh_CN");
    Q_UNUSED(translatorLoaded);
    a.installTranslator(&oTranslator);
    a.setQuitOnLastWindowClosed(false);

    QString qssPath = QCoreApplication::applicationDirPath() + QObject::tr("/dark.qss");
    QFile qssFile(qssPath);
    QString qss;
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qss = QString::fromUtf8(qssFile.readAll());
        qssFile.close();
    }
    if (qss.isEmpty())
    {
        qss = QObject::tr(
            "QWidget { background-color: #1e1e1e; color: #dcdcdc; }"
            "QDialog, QMainWindow { background-color: #1e1e1e; color: #dcdcdc; }"
            "QMenuBar, QMenu { background-color: #252526; color: #dcdcdc; }"
            "QMenu::item:selected { background-color: #3a3d41; }"
            "QPushButton { background-color: #3a3d41; border: 1px solid #555555; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #444444; }"
            "QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QSpinBox, QDoubleSpinBox {"
            "background-color: #252526; border: 1px solid #555555; color: #dcdcdc; }"
            "QTreeView, QListView, QTableView { background-color: #1e1e1e; color: #dcdcdc; }"
            "QHeaderView::section { background-color: #2d2d30; color: #dcdcdc; border: 1px solid #3a3d41; }"
            "QGraphicsView { background-color: #eef2f7; color: #111827; border: 1px solid #3a3d41; }"
            "QTreeView#leftTreeView { background-color: #f5f5f5; color: #000000; selection-background-color: #cce8ff; selection-color: #000000; }"
            "QTreeView#leftTreeView::item { padding: 4px 6px; }"
        );
    }
    if (!qss.isEmpty())
    {
        a.setStyleSheet(qss);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("water.db");
    db.setConnectOptions();
    if(!db.open())
    {
        qDebug() << "Open DataBase false!" << db.lastError().text();
        db.close();
        return 0;
    }
    qDebug() <<"open sqlite DB success!";

    if (!ensureDatabaseSchema(db))
    {
        qDebug() << "Database schema migration failed.";
        db.close();
        return 0;
    }

	//读取数据库
    StaticValue::GetInstance()->Parse();

    MainWindow w;
    w.show();
    //1 软件启动 发送到启动状态到web服务器

    return a.exec();
}
