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

private slots:
    void on_englishLabel_clicked();

    void on_spanishLabel_clicked();

private:
    Ui::LanguageChooser *ui;
    void englishClicked();
    void spanishClicked();
};

#endif // LANGUAGECHOOSER_H
