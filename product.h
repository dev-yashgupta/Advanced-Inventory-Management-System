#pragma once
#include <QString>

struct Product {
    int     id          = 0;
    QString name;
    QString category;
    QString sku;
    double  price       = 0.0;
    int     quantity    = 0;
    int     reorderLevel= 10;
    QString supplier;
    QString description;
};
