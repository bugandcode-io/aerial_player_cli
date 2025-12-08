#pragma once

#include <QGroupBox>

class MixerWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit MixerWidget(QWidget* parent = nullptr);

signals:
    void crossfaderChanged(float value01);
    void masterVolumeChanged(float value01);
};
