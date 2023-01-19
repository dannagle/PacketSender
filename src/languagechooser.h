#ifndef LANGUAGECHOOSER_H
#define LANGUAGECHOOSER_H

#include <QDialog>

namespace Ui {
class LanguageChooser;
}

class LanguageChooser : public QDialog
{
    Q_OBJECT

public:
    explicit LanguageChooser(QWidget *parent = nullptr);
    ~LanguageChooser();

private:
    Ui::LanguageChooser *ui;
};

#endif // LANGUAGECHOOSER_H
