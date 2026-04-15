#pragma once
#include <QAbstractTableModel>
#include <QList>
#include "product.h"

class InventoryModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColumnID,
        ColumnName,
        ColumnCategory,
        ColumnSKU,
        ColumnPrice,
        ColumnQuantity,
        ColumnReorderLevel,
        ColumnSupplier,
        COL_COUNT
    };

    explicit InventoryModel(QObject* parent = nullptr);

    void reload(const QList<Product>& products);
    Product productAt(int row) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<Product> m_data;
};
