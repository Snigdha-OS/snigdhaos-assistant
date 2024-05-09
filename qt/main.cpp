/*
* main function of the application, responsible for starting the Qt application and 
* creating an instance of the SnigdhaOSAssistant class.
*/

#include "snigdhaosassistant.h"

#include <QApplication>
/*
* creates a QApplication object, which manages the GUI application's control flow and main settings.
*/
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    /*
    * creates an instance of the SnigdhaOSAssistant class named w. 
    * it passes nullptr as the parent widget and either the second command line argument (argv[1]) 
    * or an empty string as the initial state of the application.
    */
    SnigdhaOSAssistant w(nullptr, a.arguments().length() > 1 ? a.arguments()[1] : "");
    /*
    * displays the main window of the application (w) on the screen.
    */
    w.show();
    /*
    * starts the event loop of the application by calling the exec() function of the QApplication object (a). 
    * the event loop waits for events to occur and dispatches them to event handlers. 
    * the function will return when the application exits, typically after the main window is closed.
    */
    return a.exec();
}
