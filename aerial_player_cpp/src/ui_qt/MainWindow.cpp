#include "MainWindow.h"
#include "DeckWidget.h"
#include "MixerWidget.h"
#include "LibraryWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    root->addWidget(createTopBar());
    root->addWidget(createCenterArea(), 2);
    root->addWidget(createBottomArea(), 1);

    setCentralWidget(central);
    setWindowTitle("Aerial DJ");
    resize(1200, 800);
}

QWidget* MainWindow::createTopBar()
{
    auto* w = new QWidget;
    auto* layout = new QHBoxLayout(w);

    // Use YOUR existing Aerial icon later
    auto* logo = new QLabel("Aerial");
    logo->setStyleSheet("font-size: 20px; font-weight: bold;");

    auto* session = new QLabel("Session: 00:00:00");
    session->setObjectName("sessionLabel");

    layout->addWidget(logo);
    layout->addStretch();
    layout->addWidget(session);

    return w;
}

QWidget* MainWindow::createCenterArea()
{
    auto* w = new QWidget;
    auto* layout = new QHBoxLayout(w);

    deckA = new DeckWidget("Deck A", this);
    mixer = new MixerWidget(this);
    deckB = new DeckWidget("Deck B", this);

    layout->addWidget(deckA, 3);
    layout->addWidget(mixer, 2);
    layout->addWidget(deckB, 3);

    return w;
}

QWidget* MainWindow::createBottomArea()
{
    library = new LibraryWidget(this);
    return library;
}
