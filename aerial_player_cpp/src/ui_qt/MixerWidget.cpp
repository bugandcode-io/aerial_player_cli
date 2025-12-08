#include "MixerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

MixerWidget::MixerWidget(QWidget* parent)
    : QGroupBox("Mixer", parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* xfLabel = new QLabel("Crossfader");
    auto* xfader = new QSlider(Qt::Horizontal);
    xfader->setRange(0, 1000);
    layout->addWidget(xfLabel);
    layout->addWidget(xfader);

    auto* volLayout = new QHBoxLayout;
    auto* volLabel = new QLabel("Master");
    auto* volSlider = new QSlider(Qt::Vertical);
    volSlider->setRange(0, 100);

    volLayout->addWidget(volLabel);
    volLayout->addWidget(volSlider);
    layout->addLayout(volLayout);

    connect(xfader, &QSlider::valueChanged, this, [this](int v) {
        emit crossfaderChanged(static_cast<float>(v) / 1000.0f);
    });

    connect(volSlider, &QSlider::valueChanged, this, [this](int v) {
        emit masterVolumeChanged(static_cast<float>(v) / 100.0f);
    });
}
