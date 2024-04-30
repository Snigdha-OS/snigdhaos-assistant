#include "snigdhaosassistant.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SnigdhaOSAssistant w;
    w.show();
    return a.exec();
}
