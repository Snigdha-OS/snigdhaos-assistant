/*
* Original Author : Garuda Linux & Team.
* Thanks for developing this application.
* love from Bangladesh
*/

#ifndef SNIGDHAOSASSISTANT_H
#define SNIGDHAOSASSISTANT_H

#include <QMainWindow>
#include <QAbstractButton>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
class SnigdhaOSAssistant;
}
QT_END_NAMESPACE

class SnigdhaOSAssistant : public QMainWindow
{
    Q_OBJECT

public:
    enum class State {
        QUIT,
        WELCOME,
        INTERNET,
        UPDATE,
        UPDATE_RETRY,
        NVIDIA_CHECK,
        NVIDIA,
        NVIDIA_APPLY,
        SELECT,
        APPLY,
        APPLY_RETRY,
        SUCCESS
    };

    SnigdhaOSAssistant(QWidget* parent = nullptr, QString state = "WELCOME");
    ~SnigdhaOSAssistant();

private slots:
    void on_textWidget_buttonBox_clicked(QAbstractButton* button);
    void on_selectWidget_buttonBox_clicked(QAbstractButton* button);


private:
    Ui::SnigdhaOSAssistant *ui;
    QDateTime executable_modify_date;
    State currentState;
    void doInternetUpRequest();
    void doUpdate();
    void doApply();
    void doNvidiaCheck();
    void doNvidiaApply();
    void populateSelectWidget();
    void populateSelectWidget(QString filename, QString label);
    void updateState(State state);
    void updateState(QString state);
    void relaunchSelf(QString param);
};
#endif // SNIGDHAOSASSISTANT_H
