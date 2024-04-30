/*
* Original Author : Garuda Linux & Team.
* Thanks for developing this application.
* love from Bangladesh
*/

#include "snigdhaosassistant.h"
#include "./ui_snigdhaosassistant.h"

SnigdhaOSAssistant::SnigdhaOSAssistant(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SnigdhaOSAssistant)
{
    ui->setupUi(this);
}

SnigdhaOSAssistant::~SnigdhaOSAssistant()
{
    delete ui;
}
