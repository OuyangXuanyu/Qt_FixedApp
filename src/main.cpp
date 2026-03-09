#include <QApplication>
#include "./ui/manager_of_ui.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
    std::cout.imbue(std::locale(".UTF8"));
#endif
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app_ex(argc, argv);
    // const auto ui_manager = new MyApp::UI::UiManager();  // 堆上对象
    MyApp::UI::UiManager ui_manager;
    ui_manager.show_login_dialog();

    // std::cout << ui_manager.baseDir_AppData.toStdString() << std::endl;
    //
    // auto pd = PatientDatabase(ui_manager.baseDir_AppData.toStdString());
    // std::cout << typeid(pd).name() << std::endl;
    //
    // std::cout << hashPasswordQt("1").toStdString() << std::endl;

    return QApplication::exec();
}