#include "database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QVariant>

Database& Database::instance() {
    static Database db;
    return db;
}

bool Database::open(const QString& path) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qCritical() << "DB open failed:" << m_db.lastError().text();
        return false;
    }
    createTables();
    return true;
}

void Database::close() { m_db.close(); }

void Database::createTables() {
    QSqlQuery q;
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS products (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            name         TEXT NOT NULL,
            category     TEXT,
            sku          TEXT UNIQUE,
            price        REAL DEFAULT 0,
            quantity     INTEGER DEFAULT 0,
            reorder_level INTEGER DEFAULT 10,
            supplier     TEXT,
            description  TEXT
        )
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS stock_log (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            product_id   INTEGER,
            delta        INTEGER,
            reason       TEXT,
            timestamp    DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(product_id) REFERENCES products(id)
        )
    )");
}

static Product rowToProduct(QSqlQuery& q) {
    Product p;
    p.id           = q.value("id").toInt();
    p.name         = q.value("name").toString();
    p.category     = q.value("category").toString();
    p.sku          = q.value("sku").toString();
    p.price        = q.value("price").toDouble();
    p.quantity     = q.value("quantity").toInt();
    p.reorderLevel = q.value("reorder_level").toInt();
    p.supplier     = q.value("supplier").toString();
    p.description  = q.value("description").toString();
    return p;
}

bool Database::addProduct(Product& p) {
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO products (name,category,sku,price,quantity,reorder_level,supplier,description)
        VALUES (:name,:cat,:sku,:price,:qty,:rl,:sup,:desc)
    )");
    q.bindValue(":name", p.name);
    q.bindValue(":cat",  p.category);
    q.bindValue(":sku",  p.sku);
    q.bindValue(":price",p.price);
    q.bindValue(":qty",  p.quantity);
    q.bindValue(":rl",   p.reorderLevel);
    q.bindValue(":sup",  p.supplier);
    q.bindValue(":desc", p.description);
    if (!q.exec()) { qWarning() << q.lastError(); return false; }
    p.id = q.lastInsertId().toInt();
    return true;
}

bool Database::updateProduct(const Product& p) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE products SET name=:name,category=:cat,sku=:sku,price=:price,
        quantity=:qty,reorder_level=:rl,supplier=:sup,description=:desc
        WHERE id=:id
    )");
    q.bindValue(":name", p.name);
    q.bindValue(":cat",  p.category);
    q.bindValue(":sku",  p.sku);
    q.bindValue(":price",p.price);
    q.bindValue(":qty",  p.quantity);
    q.bindValue(":rl",   p.reorderLevel);
    q.bindValue(":sup",  p.supplier);
    q.bindValue(":desc", p.description);
    q.bindValue(":id",   p.id);
    return q.exec();
}

bool Database::deleteProduct(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM products WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

QList<Product> Database::allProducts() {
    QList<Product> list;
    QSqlQuery q("SELECT * FROM products ORDER BY name");
    while (q.next()) list << rowToProduct(q);
    return list;
}

QList<Product> Database::searchProducts(const QString& term, const QString& category) {
    QList<Product> list;
    QString sql = "SELECT * FROM products WHERE (name LIKE :t OR sku LIKE :t OR supplier LIKE :t)";
    if (!category.isEmpty()) sql += " AND category=:cat";
    sql += " ORDER BY name";
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":t", "%" + term + "%");
    if (!category.isEmpty()) q.bindValue(":cat", category);
    q.exec();
    while (q.next()) list << rowToProduct(q);
    return list;
}

QList<Product> Database::lowStockProducts() {
    QList<Product> list;
    QSqlQuery q("SELECT * FROM products WHERE quantity <= reorder_level ORDER BY quantity");
    while (q.next()) list << rowToProduct(q);
    return list;
}

QStringList Database::categories() {
    QStringList cats;
    QSqlQuery q("SELECT DISTINCT category FROM products WHERE category != '' ORDER BY category");
    while (q.next()) cats << q.value(0).toString();
    return cats;
}

bool Database::adjustStock(int productId, int delta, const QString& reason) {
    QSqlQuery q;
    q.prepare("UPDATE products SET quantity = quantity + :d WHERE id=:id");
    q.bindValue(":d",  delta);
    q.bindValue(":id", productId);
    if (!q.exec()) return false;

    q.prepare("INSERT INTO stock_log (product_id,delta,reason) VALUES (:pid,:d,:r)");
    q.bindValue(":pid", productId);
    q.bindValue(":d",   delta);
    q.bindValue(":r",   reason);
    return q.exec();
}

QList<Database::StockLog> Database::stockHistory(int productId) {
    QList<StockLog> list;
    QString sql = R"(
        SELECT l.id, l.product_id, p.name, l.delta, l.reason, l.timestamp
        FROM stock_log l JOIN products p ON p.id=l.product_id
    )";
    if (productId >= 0) sql += " WHERE l.product_id=:pid";
    sql += " ORDER BY l.timestamp DESC";
    QSqlQuery q;
    q.prepare(sql);
    if (productId >= 0) q.bindValue(":pid", productId);
    q.exec();
    while (q.next()) {
        StockLog s;
        s.id          = q.value(0).toInt();
        s.productId   = q.value(1).toInt();
        s.productName = q.value(2).toString();
        s.delta       = q.value(3).toInt();
        s.reason      = q.value(4).toString();
        s.timestamp   = q.value(5).toString();
        list << s;
    }
    return list;
}
