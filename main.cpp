#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "database.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Inventory Management System");
    app.setOrganizationName("MyOrg");

    if (!Database::instance().open()) {
        QMessageBox::critical(nullptr, "Error", "Failed to open database.");
        return 1;
    }

    MainWindow w;
    w.show();

    int ret = app.exec();
    Database::instance().close();
    return ret;
}
