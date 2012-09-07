#ifndef EDITNUMBER_H
#define EDITNUMBER_H

#include <QDialog>

namespace Ui {
    class editNumber;
}

class editNumber : public QDialog
{
    Q_OBJECT

public:
    explicit editNumber(QWidget *parent = 0, const QString &number = "",
                        const QString &description = "",
                        const QString blocked = "0");
    ~editNumber();

    QString getNumber();
    QString getDescription();
    QString getBlocked();

private:
    Ui::editNumber *ui;
};

#endif // EDITNUMBER_H
