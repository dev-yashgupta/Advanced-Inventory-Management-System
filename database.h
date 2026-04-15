#pragma once
#include <QtSql/QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QList>
#include "product.h"

class Database {
public:
    static Database& instance();

    bool        open(const QString& path = "inventory.db");
    void        close();

    // Products
    bool        addProduct(Product& p);
    bool        updateProduct(const Product& p);
    bool        deleteProduct(int id);
    QList<Product> allProducts();
    QList<Product> searchProducts(const QString& term, const QString& category = {});
    QList<Product> lowStockProducts();

    // Categories
    QStringList categories();

    // Stock adjustment
    bool        adjustStock(int productId, int delta, const QString& reason);

    // Transaction log
    struct StockLog {
        int     id;
        int     productId;
        QString productName;
        int     delta;
        QString reason;
        QString timestamp;
    };
    QList<StockLog> stockHistory(int productId = -1);

private:
    Database() = default;
    void createTables();
    QSqlDatabase m_db;
};
