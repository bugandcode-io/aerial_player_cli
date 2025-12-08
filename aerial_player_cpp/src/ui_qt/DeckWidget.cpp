#include "DeckWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QSlider>

DeckWidget::DeckWidget(const QString& title, QWidget* parent)
    : QGroupBox(title, parent)
{
    auto* main = new QVBoxLayout(this);

    auto* trackTitle = new QLabel("No track loaded");
    trackTitle->setObjectName("trackTitle");
    main->addWidget(trackTitle);

    auto* waveform = new QFrame;
    waveform->setFrameShape(QFrame::StyledPanel);
    waveform->setMinimumHeight(80);
    waveform->setObjectName("waveform");
    main->addWidget(waveform);

    auto* controls = new QHBoxLayout;
    auto* playBtn = new QPushButton("▶");
    auto* pauseBtn = new QPushButton("⏸");
    auto* cueBtn = new QPushButton("CUE");

    controls->addWidget(playBtn);
    controls->addWidget(pauseBtn);
    controls->addWidget(cueBtn);
    controls->addStretch();

    main->addLayout(controls);

    auto* posSlider = new QSlider(Qt::Horizontal);
    posSlider->setRange(0, 1000);
    main->addWidget(posSlider);

    auto* pitchLayout = new QHBoxLayout;
    auto* pitchLabel = new QLabel("Pitch");
    auto* pitchSlider = new QSlider(Qt::Horizontal);
    pitchSlider->setRange(-8, 8);

    pitchLayout->addWidget(pitchLabel);
    pitchLayout->addWidget(pitchSlider);
    main->addLayout(pitchLayout);

    // Signals (for later wiring to engine)
    connect(playBtn, &QPushButton::clicked, this, &DeckWidget::playRequested);
    connect(pauseBtn, &QPushButton::clicked, this, &DeckWidget::pauseRequested);
    connect(cueBtn, &QPushButton::clicked, this, &DeckWidget::cueRequested);

    connect(posSlider, &QSlider::valueChanged, this, [this](int v) {
        emit seekRequested(static_cast<double>(v) / 10.0); // just a placeholder mapping
    });

    connect(pitchSlider, &QSlider::valueChanged, this, [this](int v) {
        emit pitchChanged(static_cast<double>(v)); // -8..+8 percent
    });
}
