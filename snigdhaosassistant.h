#ifndef SNIGDHAOSASSISTANT_H
#define SNIGDHAOSASSISTANT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class SnigdhaOSAssistant;
}
QT_END_NAMESPACE

class SnigdhaOSAssistant : public QMainWindow
{
    Q_OBJECT

public:
    SnigdhaOSAssistant(QWidget *parent = nullptr);
    ~SnigdhaOSAssistant();

private:
    Ui::SnigdhaOSAssistant *ui;
};
#endif // SNIGDHAOSASSISTANT_H
