#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QCloseEvent>

#include "qextserialport.h"
#include "editNumber.h"

#define CONFIG_FILE "config"
#define DB_FILE "db"
#define CALLS_FILE "calls"

namespace Ui {
    class configWindow;
}

// Struct to manage db entries
typedef struct DbEntry_t
{
        QString phoneNumber;
        QString description;
        QString blocked;

        DbEntry_t(const QString &_phoneNumber, const QString &_description,
                  const QString &_blocked) :
            phoneNumber(_phoneNumber),
            description(_description),
            blocked(_blocked)
        {}

} DbEntry;

class configWindow : public QDialog
{
    Q_OBJECT

private slots:
    void slTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void slSaveAndRun();
    void slModemDataAvailable();
    void slNumberCellClicked(int row, int column);
    void slNumberCellDoubleClicked(int row, int column);
    void slNumberAdd();
    void slNumberEdit();
    void slNumberDelete();
    void slRecentCellClicked(int row, int column);
    void slRecentCellDoubleClicked(int row, int column);
    void slRecentAdd();
    void slClearRecent();
    void slHangup();

public:
    explicit configWindow(QWidget *parent = 0);
    ~configWindow();

protected:
    void closeEvent(QCloseEvent *e);

private:
    Ui::configWindow *ui;

    QString userFolder;
    QSystemTrayIcon *trayIcon;
    QextSerialPort *modem;
    QString buffer;
    QIcon icon;
    QList<DbEntry> numbersDb;

    bool loadConfig();
    void loadDb();
    void saveDb();
    void startMonitoring();
    void addToLog(const QString &msg);
    void processModemLine(const QString &data);
    void processIncomingCall(const QString &number);
    void updateNumbersTable();
    bool numberExistsInDb(const QString &number);

    QString port, callerIdCommand, callerIdVariable, callerIdSeparator;
    QString answerCommand, hangupCommand;
    BaudRateType baudRate;
    DataBitsType dataBits;
    ParityType parity;
    StopBitsType stopBits;
    FlowType flowControl;
};

#endif // CONFIGWINDOW_H
