#ifndef MULTICASTSETUP_H
#define MULTICASTSETUP_H

#include <QDialog>

namespace Ui {
class MulticastSetup;
}

class MulticastSetup : public QDialog
{
    Q_OBJECT

public:
    explicit MulticastSetup(QWidget *parent = nullptr);
    ~MulticastSetup();

private:
    Ui::MulticastSetup *ui;
};

#endif // MULTICASTSETUP_H
