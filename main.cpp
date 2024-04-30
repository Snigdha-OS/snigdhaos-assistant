#include "snigdhaosassistant.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    SetupAssistant w(nullptr, a.arguments().length() > 1 ? a.arguments()[1] : "");
    w.show();
    return a.exec();
}
