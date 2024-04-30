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
