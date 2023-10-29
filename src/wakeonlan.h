#ifndef WAKEONLAN_H
#define WAKEONLAN_H

#include <QDialog>

namespace Ui {
class WakeOnLAN;
}

class WakeOnLAN : public QDialog
{
    Q_OBJECT

public:
    explicit WakeOnLAN(QWidget *parent = nullptr);
    ~WakeOnLAN();

private:
    Ui::WakeOnLAN *ui;
};

#endif // WAKEONLAN_H
