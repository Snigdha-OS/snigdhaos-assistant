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

void SnigdhaOSAssistant::doNvidiaApply(){
    auto process = new QProcess(this);
    process->start("/usr/bin/exec-terminal", QStringList() << "sudo mhwd -a pci nonfree 0300; echo; read -p 'Press enter to exit'");
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitcode, QProcess::ExitStatus status) {
        process->deleteLater();
        updateState(State::SELECT);
    });
}

void SnigdhaOSAssistant::populateSelectWidget(){
    if (ui->selectWidget_tabs->count() > 1)
        return;

    auto desktop = qEnvironmentVariable("XDG_SESSION_DESKTOP");
    ui->checkBox_GNOME->setVisible(desktop == "gnome");
    ui->checkBox_KDE->setVisible(desktop == "kde");
    if (desktop == "kde") {
        ui->checkBox_Samba->setProperty("packages", QStringList { "printer-support", "scanner-support", "samba-support", "kdenetwork-filesharing", "skanpage" "smb4k", "print-manager", "skanlite" });
    } else if (desktop == "gnome") {
        ui->checkBox_Samba->setProperty("packages", QStringList { "printer-support", "scanner-support", "samba-support", "gvfs-smb", "simple-scan" });
    } else {
        ui->checkBox_Samba->setProperty("packages", QStringList { "printer-support", "scanner-support", "samba-support", "gvfs-smb", "simple-scan", "system-config-printer" });
    }

    bool isDesktop = false;
    auto chassis = QFile("/sys/class/dmi/id/chassis_type");
    if (chassis.open(QFile::ReadOnly)) {
        QStringList list = { "3", "4", "6", "7", "23", "24" };
        QTextStream in(&chassis);
        isDesktop = list.contains(in.readLine());
    }
    ui->checkBox_Performance->setVisible(isDesktop);

    populateSelectWidget("/usr/lib/snigdhaos-assistant/input-method.txt", "Input");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/pkgmngrs.txt", "Software centers");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/kernels.txt", "Kernels");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/office.txt", "Office");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/browsers.txt", "Browsers");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/mail.txt", "Email");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/communication.txt", "Communication");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/internet.txt", "Internet");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/audio.txt", "Audio");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/video.txt", "Video");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/graphics.txt", "Graphics");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/multimedia.txt", "Multimedia");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/development.txt", "Development");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/virtualization.txt", "Virtualization");
    populateSelectWidget("/usr/lib/snigdhaos-assistant/other.txt", "Other");
}

void SnigdhaOSAssistant::populateSelectWidget(QString filename, QString label){
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QScrollArea* scroll = new QScrollArea(ui->selectWidget_tabs);
        QWidget* tab = new QWidget(scroll);
        QVBoxLayout* layout = new QVBoxLayout(tab);
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString def = in.readLine();
            QString packages = in.readLine();
            QString display = in.readLine();
            auto checkbox = new QCheckBox(tab);
            checkbox->setChecked(def == "true");
            checkbox->setText(display);
            checkbox->setProperty("packages", packages.split(" "));
            layout->addWidget(checkbox);
        }

        scroll->setWidget(tab);
        ui->selectWidget_tabs->addTab(scroll, label);
        file.close();
    }
}

void SnigdhaOSAssistant::updateState(State state){
    if (currentState != state) {
        currentState = state;
        this->show();
        this->activateWindow();
        this->raise();

        switch (state) {
        case State::WELCOME:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_welcome);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            break;
        case State::INTERNET:
            ui->mainStackedWidget->setCurrentWidget(ui->waitingWidget);
            ui->waitingWidget_text->setText("Waiting for an internet connection...");
            doInternetUpRequest();
            break;
        case State::UPDATE:
            ui->mainStackedWidget->setCurrentWidget(ui->waitingWidget);
            ui->waitingWidget_text->setText("Waiting for update to finish...");
            doUpdate();
            break;
        case State::UPDATE_RETRY:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_updateRetry);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
            break;
        case State::NVIDIA_CHECK:
            ui->mainStackedWidget->setCurrentWidget(ui->waitingWidget);
            ui->waitingWidget_text->setText("Checking for NVIDIA drivers...");
            doNvidiaCheck();
            break;
        case State::NVIDIA:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_nvidia);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
            break;
        case State::NVIDIA_APPLY:
            ui->mainStackedWidget->setCurrentWidget(ui->waitingWidget);
            ui->waitingWidget_text->setText("Installing NVIDIA drivers...");
            doNvidiaApply();
            break;
        case State::QUIT:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_quit);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Reset);
            break;
        case State::SELECT:
            ui->mainStackedWidget->setCurrentWidget(ui->selectWidget);
            populateSelectWidget();
            break;
        case State::APPLY:
            ui->mainStackedWidget->setCurrentWidget(ui->waitingWidget);
            ui->waitingWidget_text->setText("Applying...");
            doApply();
            break;
        case State::APPLY_RETRY:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_applyRetry);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Reset);
            break;
        case State::SUCCESS:
            ui->mainStackedWidget->setCurrentWidget(ui->textWidget);
            ui->textStackedWidget->setCurrentWidget(ui->textWidget_success);
            ui->textWidget_buttonBox->setStandardButtons(QDialogButtonBox::Ok);
            break;
        }
    }
}

void SnigdhaOSAssistant::updateState(QString state){
    if (state == "POST_UPDATE")
        updateState(State::NVIDIA_CHECK);
    else if (state == "UPDATE_RETRY")
        updateState(State::UPDATE_RETRY);
    else
        updateState(State::WELCOME);
}


void SnigdhaOSAssistant::relaunchSelf(QString param){
    auto binary = QFileInfo(QCoreApplication::applicationFilePath());
    if (executable_modify_date != binary.lastModified()) {
        execlp(binary.absoluteFilePath().toUtf8().constData(), binary.fileName().toUtf8().constData(), param.toUtf8().constData(), NULL);
        exit(0);
    } else
        updateState(param);
}

void SnigdhaOSAssistant::on_textWidget_buttonBox_clicked(){
    switch (currentState) {
    case State::WELCOME:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
            updateState(State::INTERNET);
        }
        break;
    case State::UPDATE_RETRY:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Yes) {
            updateState(State::INTERNET);
        }
        break;
    case State::NVIDIA:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Yes)
            updateState(State::NVIDIA_APPLY);
        else
            updateState(State::SELECT);
        return;
    case State::APPLY_RETRY:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Yes) {
            updateState(State::APPLY);
        } else if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Reset) {
            updateState(State::SELECT);
        }
        break;
    case State::SUCCESS:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
            QApplication::quit();
        }
        break;
    case State::QUIT:
        if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::No || ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
            QApplication::quit();
        } else
            updateState(State::WELCOME);
        break;
    default:;
    }
    if (ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::No || ui->textWidget_buttonBox->standardButton(button) == QDialogButtonBox::Cancel) {
        updateState(State::QUIT);
    }
}

void SnigdhaOSAssistant::on_selectWidget_buttonBox_clicked(QAbstractButton* button){
    if (ui->selectWidget_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
        updateState(State::APPLY);
    } else
        updateState(State::QUIT);
}