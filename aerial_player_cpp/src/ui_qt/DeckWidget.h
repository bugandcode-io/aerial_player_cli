#pragma once

#include <QGroupBox>

class DeckWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit DeckWidget(const QString& title, QWidget* parent = nullptr);

signals:
    void playRequested();
    void pauseRequested();
    void cueRequested();
    void seekRequested(double seconds);
    void pitchChanged(double percent);
};
