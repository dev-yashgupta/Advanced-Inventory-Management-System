#include "inventorymodel.h"
#include <QColor>
#include <QFont>

InventoryModel::InventoryModel(QObject* parent) : QAbstractTableModel(parent) {}

void InventoryModel::reload(const QList<Product>& products) {
    beginResetModel();
    m_data = products;
    endResetModel();
}

Product InventoryModel::productAt(int row) const { return m_data.at(row); }

int InventoryModel::rowCount(const QModelIndex&) const { return m_data.size(); }
int InventoryModel::columnCount(const QModelIndex&) const { return COL_COUNT; }

QVariant InventoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_data.size()) return {};
    const Product& p = m_data[index.row()];

    if (role == Qt::BackgroundRole && p.quantity <= p.reorderLevel)
        return QColor(255, 220, 220); // light red for low stock

    if (role == Qt::FontRole && p.quantity == 0) {
        QFont f; f.setBold(true); return f;
    }

    if (role != Qt::DisplayRole) return {};

    switch (index.column()) {
        case ColumnID:           return p.id;
        case ColumnName:         return p.name;
        case ColumnCategory:     return p.category;
        case ColumnSKU:          return p.sku;
        case ColumnPrice:        return QString("$%1").arg(p.price, 0, 'f', 2);
        case ColumnQuantity:     return p.quantity;
        case ColumnReorderLevel: return p.reorderLevel;
        case ColumnSupplier:     return p.supplier;
    }
    return {};
}

QVariant InventoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case ColumnID:           return "ID";
        case ColumnName:         return "Name";
        case ColumnCategory:     return "Category";
        case ColumnSKU:          return "SKU";
        case ColumnPrice:        return "Price";
        case ColumnQuantity:     return "Qty";
        case ColumnReorderLevel: return "Reorder At";
        case ColumnSupplier:     return "Supplier";
    }
    return {};
}
