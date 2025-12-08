#pragma once

#include <QMainWindow>

class DeckWidget;
class MixerWidget;
class LibraryWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    DeckWidget* deckA;
    DeckWidget* deckB;
    MixerWidget* mixer;
    LibraryWidget* library;

    QWidget* createTopBar();
    QWidget* createCenterArea();
    QWidget* createBottomArea();
};
