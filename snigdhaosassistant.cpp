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


void SnigdhaOSAssistant::doUpdate(){
    if (qEnvironmentVariableIsSet("SETUP_ASSISTANT_SELFUPDATE")) {
        updateState(State::SELECT);
        return;
    }
    auto process = new QProcess(this);
    QTemporaryFile* file = new QTemporaryFile(this);
    file->open();
    file->setAutoRemove(true);
    process->start("/usr/bin/exec-terminal", QStringList() << QString("sudo pacman -Syyu 2>&1 && rm \"" + file->fileName() + "\"; read -p 'Press enter to exit'"));
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process, file](int exitcode, QProcess::ExitStatus status) {
        process->deleteLater();
        file->deleteLater();
        if (exitcode == 0 && !file->exists()) {
            relaunchSelf("POST_UPDATE");
        } else {
            relaunchSelf("UPDATE_RETRY");
        }
    });
}

void SnigdhaOSAssistant::doApply(){
    QStringList packages;
    QStringList setup_commands;
    QStringList prepare_commands;
    auto checkboxList = ui->selectWidget_tabs->findChildren<QCheckBox*>();
    for (auto checkbox : checkboxList) {
        if (checkbox->isChecked()) {
            packages += checkbox->property("packages").toStringList();
            setup_commands += checkbox->property("setup_commands").toStringList();
            prepare_commands += checkbox->property("prepare_commands").toStringList();
        }
    }

    if (packages.empty()) {
        updateState(State::SUCCESS);
        return;
    }

    if (packages.contains("libreoffice-fresh"))
        packages.removeAll("libreoffice-still");

    if (packages.contains("podman"))
        setup_commands += "systemctl enable --now podman.socket";
    if (packages.contains("docker"))
        setup_commands += "systemctl enable --now docker.socket";
    if (packages.contains("virt-manager-meta") && packages.contains("gnome-boxes"))
        setup_commands += "systemctl enable --now libvirtd";

    packages.removeDuplicates();

    QTemporaryFile* prepareFile = new QTemporaryFile(this);
    prepareFile->setAutoRemove(true);
    prepareFile->open();
    QTextStream prepareStream(prepareFile);
    prepareStream << prepare_commands.join('\n');
    prepareFile->close();
    QTemporaryFile* packagesFile = new QTemporaryFile(this);
    packagesFile->setAutoRemove(true);
    packagesFile->open();
    QTextStream packagesStream(packagesFile);
    packagesStream << packages.join(' ');
    packagesFile->close();
    QTemporaryFile* setupFile = new QTemporaryFile(this);
    setupFile->setAutoRemove(true);
    setupFile->open();
    QTextStream setupStream(setupFile);
    setupStream << setup_commands.join('\n');
    setupFile->close();

    auto process = new QProcess(this);
    process->start("/usr/bin/exec-terminal", QStringList() << QString("/usr/lib/snigdhaos-assistant/apply.sh \"") + prepareFile->fileName() + "\" \"" + packagesFile->fileName() + "\" \"" + setupFile->fileName() + "\"");
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process, prepareFile, packagesFile, setupFile](int exitcode, QProcess::ExitStatus status) {
        process->deleteLater();
        prepareFile->deleteLater();
        packagesFile->deleteLater();
        setupFile->deleteLater();

        if (exitcode == 0 && !packagesFile->exists()) {
            updateState(State::SUCCESS);
        } else {
            updateState(State::APPLY_RETRY);
        }
    });
}

void SnigdhaOSAssistant::doNvidiaCheck(){
    auto process = new QProcess(this);
    process->start("mhwd", QStringList() << "-li"
                                         << "--pci");
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitcode, QProcess::ExitStatus status) {
        process->deleteLater();
        if (exitcode == 0 && !QString(process->readAllStandardOutput()).contains("nvidia")) {
            auto process2 = new QProcess(this);
            process2->start("mhwd", QStringList() << "-l"
                                                  << "--pci");
            connect(process2, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process2](int exitcode, QProcess::ExitStatus status) {
                process2->deleteLater();
                if (exitcode == 0 && QString(process2->readAllStandardOutput()).contains("nvidia"))
                    updateState(State::NVIDIA);
                else
                    updateState(State::SELECT);
            });
        } else {
            updateState(State::SELECT);
        }
    });
}

