QT += core gui widgets sql charts

CONFIG += c++17

TARGET = InventorySystem
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    database.cpp \
    inventorymodel.cpp \
    dialogs/addproductdialog.cpp \
    dialogs/reportdialog.cpp

HEADERS += \
    mainwindow.h \
    database.h \
    product.h \
    inventorymodel.h \
    dialogs/addproductdialog.h \
    dialogs/reportdialog.h
