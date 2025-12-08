#pragma once

#include <QGroupBox>
#include <string>

enum class DeckId { A, B };

class QLineEdit;
class QTableView;

class LibraryWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget* parent = nullptr);

signals:
    void loadTrackToDeck(DeckId deck, std::string trackId);

private:
    QLineEdit* searchBox;
    QTableView* table;
};
