#include "configWindow.h"
#include "ui_configWindow.h"
#include <QDebug>

configWindow::configWindow(QWidget *parent) :
    QDialog(parent),
    modem(NULL),
    buffer(""),
    icon(QIcon(":/res/resources/icon.png")),
    ui(new Ui::configWindow)
{
    ui->setupUi(this);

    // Setup tray icon
    this->trayIcon = new QSystemTrayIcon(this);
    connect(this->trayIcon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(slTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    this->trayIcon->setIcon(this->icon);
    this->trayIcon->setToolTip(tr("Click to open Linux Caller Id"));
    this->trayIcon->show();

    // Get user folder and create app folder
    const QStringList env(QProcess::systemEnvironment());
    const ushort max = env.size();
    for (uint x=0; x<max; ++x)
    {
        const QStringList parts(env.at(x).split("="));
        if (parts.at(0) == "HOME" && parts.count() > 1)
        {
            this->userFolder = parts.at(1) + "/.LinuxCallerId/";
            break;
        }
    }
    QDir dir;
    if (!dir.exists(this->userFolder))
        dir.mkdir(this->userFolder);

    // Set default values
    this->port = "/dev/ttyACM0";
    this->baudRate = BAUD9600;
    this->dataBits = DATA_8;
    this->parity = PAR_NONE;
    this->stopBits = STOP_1;
    this->flowControl = FLOW_OFF;
    this->callerIdCommand = "AT+VCID=1";
    this->callerIdVariable = "NMBR";
    this->callerIdSeparator = "=";
    this->answerCommand = "ATA";
    this->hangupCommand = "ATH0";

    // Populate config combos
    ui->cbBaudRate->addItem("1200", BAUD1200);
    ui->cbBaudRate->addItem("2400", BAUD2400);
    ui->cbBaudRate->addItem("4800", BAUD4800);
    ui->cbBaudRate->addItem("9600", BAUD9600);
    ui->cbBaudRate->addItem("19200", BAUD19200);

    ui->cbParity->addItem("NONE", PAR_NONE);
    ui->cbParity->addItem("ODD", PAR_ODD);
    ui->cbParity->addItem("EVEN", PAR_EVEN);

    ui->cbDataBits->addItem("5", DATA_5);
    ui->cbDataBits->addItem("6", DATA_6);
    ui->cbDataBits->addItem("7", DATA_7);
    ui->cbDataBits->addItem("8", DATA_8);

    ui->cbStopBits->addItem("1", STOP_1);
    ui->cbStopBits->addItem("2", STOP_2);

    ui->cbFlowControl->addItem("None", FLOW_OFF);
    ui->cbFlowControl->addItem("RTS/CTS", FLOW_HARDWARE);
    ui->cbFlowControl->addItem("XON/XOFF", FLOW_XONXOFF);

    connect(ui->btSave, SIGNAL(clicked()), this, SLOT(slSaveAndRun()));

    // Set up numbers db table widget
    QStringList headers;
    headers << tr("Number") << tr("Blocked") << tr("Description");
    ui->twDb->setHorizontalHeaderLabels(headers);

    connect(ui->twDb,           SIGNAL(cellClicked(int,int)),
            this,               SLOT(slNumberCellClicked(int, int)));
    connect(ui->twDb,           SIGNAL(cellDoubleClicked(int,int)),
            this,               SLOT(slNumberCellDoubleClicked(int, int)));
    connect(ui->btAddNumber,    SIGNAL(clicked()),
            this,               SLOT(slNumberAdd()));
    connect(ui->btEditNumber,   SIGNAL(clicked()),
            this,               SLOT(slNumberEdit()));
    connect(ui->btDeleteNumber, SIGNAL(clicked()),
            this,               SLOT(slNumberDelete()));

    // Set up recent calls table widget
    headers.clear();
    headers << tr("Date") << tr("Number") << tr("Description");
    ui->twRecent->setHorizontalHeaderLabels(headers);

    connect(ui->twRecent,       SIGNAL(cellClicked(int,int)),
            this,               SLOT(slRecentCellClicked(int, int)));
    connect(ui->twRecent,       SIGNAL(cellDoubleClicked(int,int)),
            this,               SLOT(slRecentCellDoubleClicked(int, int)));
    connect(ui->btAddRecent,    SIGNAL(clicked()),
            this,               SLOT(slRecentAdd()));
    connect(ui->btClearRecent,  SIGNAL(clicked()),
            this,               SLOT(slClearRecent()));

    // Load numbers db
    this->loadDb();

    // Load config data
    if (this->loadConfig())
    {
        // Config loaded, go ahead and open port
        this->startMonitoring();

        // Hide main window
        // Now commented since some Linux desktops have problem showing the tray icon (v1.1)
        //QMetaObject::invokeMethod(this, "hide", Qt::QueuedConnection);
    }
    else ui->tabWidget->setCurrentIndex(2);
}

configWindow::~configWindow()
{
    if (this->modem != NULL)
        this->modem->close();

    delete ui;
}

void configWindow::slTrayIconActivated(QSystemTrayIcon::ActivationReason)
{
    if (this->isVisible())
        this->hide();
    else this->show();
}

bool configWindow::loadConfig()
{
    bool configExists = QFile(this->userFolder + CONFIG_FILE).exists();

    // Load and parse config file if exists
    if (configExists)
    {
        QFile file(this->userFolder + CONFIG_FILE);
        if (file.open(QFile::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            QString data("");
            while (!in.atEnd())
            {
                QString line(in.readLine());

                if (line.at(0) == '#')
                    continue;

                QStringList parts = line.split("<->");
                if (parts.count() != 2)
                    continue;
                parts[0] = parts.at(0).trimmed().toLower();
                parts[1] = parts.at(1).trimmed();

                if (parts.at(0) == "port")
                    this->port = parts.at(1);
                if (parts.at(0) == "baud_rate")
                    this->baudRate = (BaudRateType) parts.at(1).toInt();
                if (parts.at(0) == "data_bits")
                    this->dataBits = (DataBitsType) parts.at(1).toInt();
                if (parts.at(0) == "parity")
                    this->parity = (ParityType) parts.at(1).toInt();
                if (parts.at(0) == "stop_bits")
                    this->stopBits = (StopBitsType) parts.at(1).toInt();
                if (parts.at(0) == "flow_control")
                    this->flowControl = (FlowType) parts.at(1).toInt();
                if (parts.at(0) == "caller_id_command")
                    this->callerIdCommand = parts.at(1);
                if (parts.at(0) == "caller_id_variable")
                    this->callerIdVariable = parts.at(1);
                if (parts.at(0) == "caller_id_separator")
                    this->callerIdSeparator= parts.at(1);
                if (parts.at(0) == "answer_command")
                    this->answerCommand= parts.at(1);
                if (parts.at(0) == "hangup_command")
                    this->hangupCommand= parts.at(1);
            }
            file.close();
        }
    }

    // Update widgets in window with loaded values (if not loaded, use defaults)
    ui->lePort->setText(this->port);
    ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->findData(this->baudRate));
    ui->cbDataBits->setCurrentIndex(ui->cbDataBits->findData(this->dataBits));
    ui->cbParity->setCurrentIndex(ui->cbParity->findData(this->parity));
    ui->cbStopBits->setCurrentIndex(ui->cbStopBits->findData(this->stopBits));
    ui->cbFlowControl->setCurrentIndex(
                                ui->cbFlowControl->findData(this->flowControl));
    ui->leCallIdCommand->setText(this->callerIdCommand);
    ui->leCallerIdVariable->setText(this->callerIdVariable);
    ui->leCallerIdSeparator->setText(this->callerIdSeparator);
    ui->leAnswerCommand->setText(this->answerCommand);
    ui->leHangupCommand->setText(this->hangupCommand);

    return configExists;
}

void configWindow::loadDb()
{
    bool dbExists = QFile(this->userFolder + DB_FILE).exists();
    if (dbExists)
    {
        QFile file(this->userFolder + DB_FILE);
        if (file.open(QFile::ReadOnly | QIODevice::Text))
        {
            this->numbersDb.clear();

            QTextStream in(&file);
            QString data("");
            while (!in.atEnd())
            {
                QString line = in.readLine();

                if (line.at(0) == '#')
                    continue;

                QStringList parts = line.split("=");
                if (parts.count() != 3)
                    continue;

                this->numbersDb << DbEntry(parts.at(0).trimmed(),
                                           parts.at(1).trimmed(),
                                           parts.at(2).trimmed());
            }
            file.close();
        }
    }
    this->updateNumbersTable();
}

void configWindow::saveDb()
{
    QFile configFile(this->userFolder + DB_FILE);
    if (configFile.open(QIODevice::WriteOnly))
    {
        QString data("# Numbers database.\n# Format: NUMBER=DESCRIPTION=BLOCKED"
                     "\n# BLOCKED values can be 0 (unblocked) or 1 (blocked)");

        foreach (const DbEntry &entry, this->numbersDb)
        {
            data += "\n" + entry.phoneNumber + "=" + entry.description
                    + "=" + entry.blocked;
        }

        QTextStream out(&configFile);
        out << data;
        configFile.close();
    }
}

void configWindow::slSaveAndRun()
{
    ui->teLog->clear();

    this->port = ui->lePort->text();
    this->baudRate = (BaudRateType)
            ui->cbBaudRate->itemData(ui->cbBaudRate->currentIndex()).toInt();
    this->dataBits = (DataBitsType)
            ui->cbDataBits->itemData(ui->cbDataBits->currentIndex()).toInt();
    this->parity = (ParityType)
            ui->cbParity->itemData(ui->cbParity->currentIndex()).toInt();
    this->stopBits = (StopBitsType)
            ui->cbStopBits->itemData(ui->cbStopBits->currentIndex()).toInt();
    this->flowControl = (FlowType)
         ui->cbFlowControl->itemData(ui->cbFlowControl->currentIndex()).toInt();
    this->callerIdCommand = ui->leCallIdCommand->text();
    this->callerIdVariable = ui->leCallerIdVariable->text();
    this->callerIdSeparator = ui->leCallerIdSeparator->text();
    this->answerCommand = ui->leAnswerCommand->text();
    this->hangupCommand = ui->leHangupCommand->text();

    QFile configFile(this->userFolder + CONFIG_FILE);
    if (configFile.open(QIODevice::WriteOnly))
    {
        QString data("# Modem configuration");
        data += "\nport<->" + this->port;
        data += "\nbaud_rate<->" + QString::number(this->baudRate);
        data += "\ndata_bits<->" + QString::number(this->dataBits);
        data += "\nparity<->" + QString::number(this->parity);
        data += "\nstop_bits<->" + QString::number(this->stopBits);
        data += "\nflow_control<->" + QString::number(this->flowControl);
        data += "\ncaller_id_command<->" + this->callerIdCommand;
        data += "\ncaller_id_variable<->" + this->callerIdVariable;
        data += "\ncaller_id_separator<->" + this->callerIdSeparator;
        data += "\nanswer_command<->" + this->answerCommand;
        data += "\nhangup_command<->" + this->hangupCommand;

        QTextStream out(&configFile);
        out << data;
        configFile.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Error writing file"),
                             tr("Could not write config file at\n\n%1")
                             .arg(configFile.fileName()));
    }


    this->startMonitoring();
}

void configWindow::startMonitoring()
{
    // Delete old instances
    if (this->modem != NULL)
    {
        this->modem->close();
        delete this->modem;
    }

    // Set up port
    this->modem = new QextSerialPort(this->port,
                                     QextSerialPort::EventDriven, this);

    this->modem->setBaudRate(this->baudRate);
    this->modem->setDataBits(this->dataBits);
    this->modem->setParity(this->parity);
    this->modem->setStopBits(this->stopBits);
    this->modem->setFlowControl(this->flowControl);

    connect(this->modem, SIGNAL(readyRead()),
            this,        SLOT(slModemDataAvailable()));

    // Try to open port
    if (!this->modem->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, tr("Error opening modem"),
                             tr("Could not open modem, please check config"));
        delete this->modem;
        this->modem = NULL;
        this->show();
        return;
    }

    // Try to send caller id activation
    if (!this->callerIdCommand.isEmpty())
    {
        QString command = this->callerIdCommand + "\r";
        if (this->modem->write(command.toLatin1(), command.length()) == -1)
        {
            QMessageBox::warning(this, tr("Error sending data"),
                                 tr("Error sending data, please check config"));
            delete this->modem;
            this->modem = NULL;
            this->show();
            return;
        }
    }
}

void configWindow::slModemDataAvailable()
{
    // Add data to buffer
    this->buffer += this->modem->readAll();

    // Process complete lines
    QStringList lines(this->buffer.split("\r"));
    ushort limit = lines.count() -1;
    for (ushort x = 0; x < limit; ++x)
    {
        if (!lines.at(x).trimmed().isEmpty())
            this->processModemLine(lines.at(x).trimmed());
    }
    this->buffer = lines.at(limit);
}

void configWindow::processModemLine(const QString &data)
{
    // Add to log
    this->addToLog(data);

    // Check for caller id variable
    if (data.toLower().contains(this->callerIdVariable.toLower()))
    {
        // Extract number
        QStringList parts(data.toLower().split(
                                            this->callerIdSeparator.toLower()));
        if (parts.count() == 2)
        {
            QString callerId(parts.at(1).trimmed());
            this->processIncomingCall(callerId);
        }
    }

}

void configWindow::addToLog(const QString &msg)
{
    QString data(QDateTime::currentDateTime().toString());
    data += " " + msg;
    ui->teLog->append(data);
}

void configWindow::processIncomingCall(const QString &number)
{
    // Check if number is in database
    QString msg("");
    QString description("");
    bool blocked = false;

    int count = this->numbersDb.count();
    for (int x = 0; x < count; ++x)
    {
        if (this->numbersDb.at(x).phoneNumber == number)
        {
            description = this->numbersDb.at(x).description;
            msg += description;
            if (this->numbersDb.at(x).blocked == "1")
            {
                msg += "\n\nBLOCKED";
                blocked = true;
            }
            else msg += "\n\nNot blocked";
            break;
        }
    }

    // Add to recent calls table
    int row = ui->twRecent->rowCount();
    ui->twRecent->insertRow(row);

    QTableWidgetItem *colDate =
                new QTableWidgetItem(QDateTime::currentDateTime().toString());
    colDate->setData(Qt::UserRole, number);
    ui->twRecent->setItem(row, 0, colDate);

    QTableWidgetItem *colNumber = new QTableWidgetItem(number);
    colNumber->setData(Qt::UserRole, number);
    ui->twRecent->setItem(row, 1, colNumber);

    QTableWidgetItem *colDesc = new QTableWidgetItem(description);
    colDesc->setData(Qt::UserRole, number);
    ui->twRecent->setItem(row, 2, colDesc);

    // Show notification
    QString title(tr("Incoming call - %1").arg(number));
    if (msg.isEmpty())
        msg = tr("Not in database, will not be blocked");

    this->trayIcon->showMessage(title, msg, QSystemTrayIcon::Information);

    // Block call if needed (answer and hang up)
    if (blocked)
    {
        QString answer = this->answerCommand + "\r";
        this->modem->write(answer.toLatin1(), answer.length());
        QTimer::singleShot(2000, this, SLOT(slHangup()));
    }

    // Add call to calls log
    QFile callsFile(this->userFolder + CALLS_FILE);
    if (callsFile.open(QIODevice::Append))
    {
        QString data(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        data += " " + number + " " + description + "\n";

        QTextStream out(&callsFile);
        out << data;
        callsFile.close();
    }
}

void configWindow::slHangup()
{
    QString hangup = this->hangupCommand + "\r";
    this->modem->write(hangup.toLatin1(), hangup.length());
}

void configWindow::updateNumbersTable()
{
    int rowCount = this->numbersDb.count();
    ui->twDb->clearContents();
    ui->twDb->setRowCount(rowCount);
    ui->twDb->setSortingEnabled(false);

    for (int x = 0; x < rowCount; ++x)
    {
        DbEntry entry = this->numbersDb.at(x);

        QTableWidgetItem *colNumber = new QTableWidgetItem(entry.phoneNumber);
        colNumber->setData(Qt::UserRole, entry.phoneNumber);
        ui->twDb->setItem(x, 0, colNumber);

        QTableWidgetItem *colBlocked =
              new QTableWidgetItem(entry.blocked == "1" ? tr("Yes") : tr("No"));
        colBlocked->setData(Qt::UserRole, entry.phoneNumber);
        ui->twDb->setItem(x, 1, colBlocked);

        QTableWidgetItem *colDesc = new QTableWidgetItem(entry.description);
        colDesc->setData(Qt::UserRole, entry.phoneNumber);
        ui->twDb->setItem(x, 2, colDesc);
    }

    ui->twDb->setSortingEnabled(true);
}

void configWindow::slNumberCellClicked(int row, int column)
{
    QTableWidgetItem *currentItem(ui->twDb->item(row, column));
    if (currentItem == NULL)
    {
        ui->btEditNumber->setEnabled(false);
        ui->btDeleteNumber->setEnabled(false);
        return;
    }

    ui->btEditNumber->setEnabled(true);
    ui->btDeleteNumber->setEnabled(true);
}

void configWindow::slNumberCellDoubleClicked(int row, int column)
{
    this->slNumberEdit();
}

void configWindow::slNumberAdd()
{
    editNumber dialog(this);
    if (dialog.exec())
    {
        // Get data
        DbEntry newEntry(dialog.getNumber(), dialog.getDescription(),
                         dialog.getBlocked());

        // Check number is not already in db
        if (this->numberExistsInDb(newEntry.phoneNumber))
        {
            QMessageBox::information(this, tr("Number exists"),
                                     tr("The number\n\n%1\n\n"
                                        "is already in the database.")
                                     .arg(newEntry.phoneNumber));
            return;
        }

        // Add to db
        this->numbersDb << newEntry;
        this->saveDb();
        this->updateNumbersTable();
    }
}

void configWindow::slNumberEdit()
{
    int row = ui->twDb->currentRow();

    if (row < 0)
        return;

    QString number(ui->twDb->item(row, 0)->data(Qt::UserRole).toString());

    int count = this->numbersDb.count();
    for (int x = 0; x < count; ++x)
    {
        if (this->numbersDb.at(x).phoneNumber == number)
        {
            editNumber dialog(this, number,
                              this->numbersDb.at(x).description,
                              this->numbersDb.at(x).blocked);
            if (dialog.exec())
            {
                this->numbersDb[x] = DbEntry(number, dialog.getDescription(),
                                             dialog.getBlocked());
                this->saveDb();
                this->updateNumbersTable();
                break;
            }
        }
    }
}

void configWindow::slNumberDelete()
{
    int row = ui->twDb->currentRow();

    if (row < 0)
        return;

    QString number(ui->twDb->item(row, 0)->data(Qt::UserRole).toString());

    QMessageBox msgBox;

    msgBox.setWindowTitle(tr("Delete number"));
    msgBox.setText(tr("Delete number\n\n%1\n\nfrom the database?").arg(number));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowIcon(this->icon);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        QMutableListIterator<DbEntry> i(this->numbersDb);
        while (i.hasNext())
        {
            DbEntry entry = i.next();
            if (entry.phoneNumber == number)
            {
                i.remove();
                break;
            }
        }
        this->saveDb();
        this->updateNumbersTable();
    }
}

void configWindow::slClearRecent()
{
    ui->twRecent->clearContents();
    ui->twRecent->setRowCount(0);
}

void configWindow::slRecentCellClicked(int row, int column)
{
    QTableWidgetItem *currentItem(ui->twRecent->item(row, column));
    if (currentItem == NULL)
    {
        ui->btAddRecent->setEnabled(false);
        return;
    }

    ui->btAddRecent->setEnabled(true);
}

void configWindow::slRecentCellDoubleClicked(int row, int column)
{
    this->slRecentAdd();
}

void configWindow::slRecentAdd()
{
    int row = ui->twRecent->currentRow();

    if (row < 0)
        return;

    QString number(ui->twRecent->item(row, 0)->data(Qt::UserRole).toString());

    editNumber dialog(this, number);
    if (dialog.exec())
    {
        if (this->numberExistsInDb(number))
        {
            QMessageBox::information(this, tr("Number exists"),
                                     tr("The number\n\n%1\n\n"
                                        "is already in the database.")
                                     .arg(number));
            return;
        }

        this->numbersDb << DbEntry(number, dialog.getDescription(),
                                   dialog.getBlocked());
        this->saveDb();
        this->updateNumbersTable();
        ui->twRecent->item(row, 2)->setText(dialog.getDescription());
    }
}

bool configWindow::numberExistsInDb(const QString &number)
{
    // Check number is not already in db
    foreach (const DbEntry &entry, this->numbersDb)
    {
        if (entry.phoneNumber == number)
            return true;
    }

    return false;
}

void configWindow::closeEvent(QCloseEvent *e)
{
    QMessageBox msgBox;

    msgBox.setWindowTitle(tr("Exit program?"));
    msgBox.setText(tr("If you want to hide this "
                      "window but keep the program running, just click in "
                      "the blue phone icon in the system tray.\n\n"
                      "Do you want to exit?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowIcon(this->icon);

    if (msgBox.exec() != QMessageBox::Yes)
        e->ignore();
}
