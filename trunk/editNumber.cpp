#include "editNumber.h"
#include "ui_editNumber.h"

editNumber::editNumber(QWidget *parent, const QString &number,
                       const QString &description, const QString blocked) :
    QDialog(parent),
    ui(new Ui::editNumber)
{
    ui->setupUi(this);

    if (!number.isEmpty())
    {
        ui->leNumber->setEnabled(false);
        this->setWindowTitle(tr("Edit number"));
    }

    ui->leNumber->setText(number);
    ui->leDescription->setText(description);
    ui->rbBlocked->setChecked(blocked == "1");
}

editNumber::~editNumber()
{
    delete ui;
}

QString editNumber::getNumber()
{
    return ui->leNumber->text();
}

QString editNumber::getDescription()
{
    return ui->leDescription->text();
}

QString editNumber::getBlocked()
{
    return (ui->rbBlocked->isChecked()) ? "1" : "0";
}
