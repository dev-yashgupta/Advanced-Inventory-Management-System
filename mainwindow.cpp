#include "mainwindow.h"
#include "inventorymodel.h"
#include "database.h"
#include "dialogs/addproductdialog.h"
#include "dialogs/reportdialog.h"

#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QInputDialog>
#include <QAction>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Inventory Management System");
    resize(1100, 650);
    setupUi();
    setupToolbar();
    setupStatusBar();
    refreshTable();
}

void MainWindow::setupUi() {
    m_model = new InventoryModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1); // search all columns

    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSortingEnabled(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    connect(m_table, &QTableView::doubleClicked, this, [this](const QModelIndex&){ onEditProduct(); });

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search by name, SKU, supplier...");
    m_searchBox->setClearButtonEnabled(true);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearch);

    m_categoryFilter = new QComboBox(this);
    m_categoryFilter->addItem("All Categories");
    connect(m_categoryFilter, &QComboBox::currentTextChanged, this, &MainWindow::onSearch);

    auto* filterBar = new QHBoxLayout;
    filterBar->addWidget(new QLabel("Search:"));
    filterBar->addWidget(m_searchBox, 2);
    filterBar->addWidget(new QLabel("Category:"));
    filterBar->addWidget(m_categoryFilter, 1);

    auto* central = new QWidget(this);
    auto* layout  = new QVBoxLayout(central);
    layout->addLayout(filterBar);
    layout->addWidget(m_table);
    setCentralWidget(central);
}

void MainWindow::setupToolbar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);

    auto* addAct    = tb->addAction(QIcon::fromTheme("list-add"),    "Add Product");
    auto* editAct   = tb->addAction(QIcon::fromTheme("document-edit"),"Edit Product");
    auto* deleteAct = tb->addAction(QIcon::fromTheme("list-remove"), "Delete");
    tb->addSeparator();
    auto* stockAct  = tb->addAction(QIcon::fromTheme("view-refresh"), "Adjust Stock");
    tb->addSeparator();
    auto* reportAct = tb->addAction(QIcon::fromTheme("x-office-spreadsheet"), "Reports");
    tb->addSeparator();
    auto* refreshAct= tb->addAction(QIcon::fromTheme("view-refresh"), "Refresh");

    connect(addAct,    &QAction::triggered, this, &MainWindow::onAddProduct);
    connect(editAct,   &QAction::triggered, this, &MainWindow::onEditProduct);
    connect(deleteAct, &QAction::triggered, this, &MainWindow::onDeleteProduct);
    connect(stockAct,  &QAction::triggered, this, &MainWindow::onAdjustStock);
    connect(reportAct, &QAction::triggered, this, &MainWindow::onShowReports);
    connect(refreshAct,&QAction::triggered, this, &MainWindow::onRefresh);
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::refreshTable() {
    auto term = m_searchBox->text();
    auto cat  = m_categoryFilter->currentIndex() == 0 ? QString{} : m_categoryFilter->currentText();

    QList<Product> products = term.isEmpty() && cat.isEmpty()
        ? Database::instance().allProducts()
        : Database::instance().searchProducts(term, cat);

    m_model->reload(products);
    m_table->resizeColumnsToContents();

    // refresh category combo
    QString current = m_categoryFilter->currentText();
    m_categoryFilter->blockSignals(true);
    m_categoryFilter->clear();
    m_categoryFilter->addItem("All Categories");
    m_categoryFilter->addItems(Database::instance().categories());
    int idx = m_categoryFilter->findText(current);
    m_categoryFilter->setCurrentIndex(idx >= 0 ? idx : 0);
    m_categoryFilter->blockSignals(false);

    m_statusLabel->setText(QString("%1 products | %2 low stock")
        .arg(products.size())
        .arg(Database::instance().lowStockProducts().size()));

    checkLowStock();
}

void MainWindow::checkLowStock() {
    static bool alerted = false;
    auto low = Database::instance().lowStockProducts();
    if (!low.isEmpty() && !alerted) {
        alerted = true;
        QTimer::singleShot(300, this, [this, low]{
            QMessageBox::warning(this, "Low Stock Alert",
                QString("%1 product(s) are at or below reorder level.").arg(low.size()));
        });
    }
}

void MainWindow::onAddProduct() {
    AddProductDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Product p = dlg.product();
    if (p.name.isEmpty()) { QMessageBox::warning(this, "Error", "Product name is required."); return; }
    Database::instance().addProduct(p);
    refreshTable();
}

void MainWindow::onEditProduct() {
    auto idx = m_table->currentIndex();
    if (!idx.isValid()) { QMessageBox::information(this, "Edit", "Select a product first."); return; }
    auto srcIdx = m_proxy->mapToSource(idx);
    Product p = m_model->productAt(srcIdx.row());
    AddProductDialog dlg(this, p);
    if (dlg.exec() != QDialog::Accepted) return;
    Database::instance().updateProduct(dlg.product());
    refreshTable();
}

void MainWindow::onDeleteProduct() {
    auto idx = m_table->currentIndex();
    if (!idx.isValid()) { QMessageBox::information(this, "Delete", "Select a product first."); return; }
    auto srcIdx = m_proxy->mapToSource(idx);
    Product p = m_model->productAt(srcIdx.row());
    if (QMessageBox::question(this, "Delete", QString("Delete '%1'?").arg(p.name)) != QMessageBox::Yes) return;
    Database::instance().deleteProduct(p.id);
    refreshTable();
}

void MainWindow::onAdjustStock() {
    auto idx = m_table->currentIndex();
    if (!idx.isValid()) { QMessageBox::information(this, "Adjust Stock", "Select a product first."); return; }
    auto srcIdx = m_proxy->mapToSource(idx);
    Product p = m_model->productAt(srcIdx.row());

    bool ok;
    int delta = QInputDialog::getInt(this, "Adjust Stock",
        QString("Adjust stock for '%1' (current: %2)\nEnter positive to add, negative to remove:").arg(p.name).arg(p.quantity),
        0, -p.quantity, 1000000, 1, &ok);
    if (!ok || delta == 0) return;

    QString reason = QInputDialog::getText(this, "Reason", "Reason for adjustment:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;

    Database::instance().adjustStock(p.id, delta, reason);
    refreshTable();
}

void MainWindow::onSearch() { refreshTable(); }
void MainWindow::onRefresh() { refreshTable(); }

void MainWindow::onShowReports() {
    ReportDialog dlg(this);
    dlg.exec();
}
