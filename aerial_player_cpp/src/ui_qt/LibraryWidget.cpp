#include "LibraryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>

LibraryWidget::LibraryWidget(QWidget* parent)
    : QGroupBox("Library", parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* searchLayout = new QHBoxLayout;
    searchBox = new QLineEdit;
    searchBox->setPlaceholderText("Search tracks...");
    auto* searchBtn = new QPushButton("Search");

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(searchBtn);

    table = new QTableView;
    table->setObjectName("libraryTable");

    layout->addLayout(searchLayout);
    layout->addWidget(table);

    // Later: hook search button + table row double-click to loadTrackToDeck
}
