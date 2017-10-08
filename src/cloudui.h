#ifndef CLOUDUI_H
#define CLOUDUI_H

#include <QDialog>

namespace Ui {
class CloudUI;
}

class CloudUI : public QDialog
{
    Q_OBJECT

public:
    explicit CloudUI(QWidget *parent = 0);
    ~CloudUI();

private:
    Ui::CloudUI *ui;
};

#endif // CLOUDUI_H
