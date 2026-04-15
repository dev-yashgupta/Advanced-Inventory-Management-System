#pragma once
#include <QMainWindow>

class InventoryModel;
class QTableView;
class QLineEdit;
class QComboBox;
class QSortFilterProxyModel;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void onAdjustStock();
    void onSearch();
    void onShowReports();
    void onRefresh();

private:
    void setupUi();
    void setupToolbar();
    void setupStatusBar();
    void refreshTable();
    void checkLowStock();

    InventoryModel*        m_model;
    QSortFilterProxyModel* m_proxy;
    QTableView*            m_table;
    QLineEdit*             m_searchBox;
    QComboBox*             m_categoryFilter;
    QLabel*                m_statusLabel;
};
