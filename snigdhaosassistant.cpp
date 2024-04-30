/*
* Original Author : Garuda Linux & Team.
* Thanks for developing this application.
* love from Bangladesh
*/

#include "snigdhaosassistant.h"
#include "./ui_snigdhaosassistant.h"
#include <QCheckBox>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QScrollArea>
#include <QTemporaryFile>
#include <QTimer>
#include <QtNetwork/QNetworkReply>
#include <unistd.h>

const char* INTERNET_CHECK_URL = "https://snigdhaos.org";

SnigdhaOSAssistant::SnigdhaOSAssistant(QWidget *parent, QString state)
    : QMainWindow(parent)
    , ui(new Ui::SnigdhaOSAssistant)
{
    this->setWindowIcon(QIcon("/usr/share/pixmaps/snigdhaos-assistant.svg"));
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);
    executable_modify_date = QFileInfo(QCoreApplication::applicationFilePath()).lastModified();
    updateState(state);
}

SnigdhaOSAssistant::~SnigdhaOSAssistant()
{
    delete ui;
}

void SnigdhaOSAssistant::doInternetUpRequest(){
    QNetworkAccessManager* network_manager = new QNetworkAccessManager();
    auto network_reply = network_manager->head(QNetworkRequest(QString(INTERNET_CHECK_URL)));

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(5000);

    // Did we time out? Try again!
    connect(timer, &QTimer::timeout, this, [this, timer, network_reply, network_manager]() {
        timer->deleteLater();
        network_reply->abort();
        network_reply->deleteLater();
        network_manager->deleteLater();
        doInternetUpRequest();
    });

    // Request is done!
    connect(network_reply, &QNetworkReply::finished, this, [this, timer, network_reply, network_manager]() {
        timer->stop();
        timer->deleteLater();
        network_reply->deleteLater();
        network_manager->deleteLater();
        if (network_reply->error() == network_reply->NoError) {
            // Wooo!
            updateState(State::UPDATE);
        }
        // Boo!
        else
            doInternetUpRequest();
    });

}
