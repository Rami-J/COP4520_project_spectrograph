#include "SpectrographUI.h"

SpectrographUI::SpectrographUI(QWidget *parent)
    : QMainWindow(parent)
    , m_engine(new Engine(this))
    , m_waveform(new Waveform(this))
    , m_pauseButton(new QPushButton(this))
    , m_playButton(new QPushButton(this))
    , m_settingsButton(new QPushButton(this))
    , m_settingsDialog(new SettingsDialog(
                    m_engine->availableAudioInputDevices(),
                    m_engine->availableAudioOutputDevices(),
                    this))
{
    createLayouts();
    createActions();
    createMenus();
    connectUI();

    setWindowTitle(tr("Spectrograph"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void SpectrographUI::createLayouts()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);;
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    QHBoxLayout* mediaLayout = new QHBoxLayout(this);;
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);

    QScopedPointer<QHBoxLayout> waveformLayout(new QHBoxLayout);
    waveformLayout->setContentsMargins(0, 0, 0, 0);
    m_waveform->setLayout(waveformLayout.data());
    waveformLayout.take();
    mainLayout->addWidget(new QLabel("Spectrum Waveform"));
    mainLayout->addWidget(m_waveform);

    // Button panel
    const QSize buttonSize(30, 30);

    m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    m_pauseButton->setIcon(m_pauseIcon);
    m_pauseButton->setEnabled(false);
    m_pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pauseButton->setMinimumSize(buttonSize);

    m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    m_playButton->setIcon(m_playIcon);
    m_playButton->setEnabled(false);
    m_playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_playButton->setMinimumSize(buttonSize);

    m_settingsIcon = QIcon(":/images/settings.png");
    m_settingsButton->setIcon(m_settingsIcon);
    m_settingsButton->setEnabled(true);
    m_settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_settingsButton->setMinimumSize(buttonSize);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_pauseButton);
    buttonPanelLayout->addWidget(m_playButton);
    buttonPanelLayout->addWidget(m_settingsButton);

    QWidget* buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonPanel->setLayout(buttonPanelLayout.data());
    buttonPanelLayout.take(); // ownership transferred to buttonPanel

    QScopedPointer<QHBoxLayout> bottomPaneLayout(new QHBoxLayout);
    bottomPaneLayout->addWidget(buttonPanel);
    mainLayout->addLayout(bottomPaneLayout.data());
    bottomPaneLayout.take(); // ownership transferred to windowLayout


    setCentralWidget(centralWidget);
}

void SpectrographUI::createActions()
{
    m_openFileAct = new QAction(tr("&Open Audio File"), this);
    m_openFileAct->setShortcuts(QKeySequence::Open);
    m_openFileAct->setStatusTip(tr("Open an audio file to analyze"));
    connect(m_openFileAct, &QAction::triggered, this, &SpectrographUI::showFileDialog);

    m_exitAct = new QAction(tr("&Exit"), this);
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, &QAction::triggered, this, &SpectrographUI::quitApplication);
}

void SpectrographUI::createMenus()
{   
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openFileAct);
    m_fileMenu->addAction(m_exitAct);
}

void SpectrographUI::showFileDialog()
{
    const QString dir;
    const QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open WAV file"), dir, "*.wav");
    if (fileNames.count()) {
        resetUI();
        m_engine->loadFile(fileNames.front());
        updateButtonStates();
    }
}

void SpectrographUI::showSettingsDialog()
{
    m_settingsDialog->exec();
    if (m_settingsDialog->result() == QDialog::Accepted) {
        m_engine->setAudioInputDevice(m_settingsDialog->inputDevice());
        m_engine->setAudioOutputDevice(m_settingsDialog->outputDevice());
        m_engine->setWindowFunction(m_settingsDialog->windowFunction());
    }
}

void SpectrographUI::updateButtonStates()
{
    const bool pauseEnabled = (QAudio::ActiveState == m_engine->state() ||
                               QAudio::IdleState == m_engine->state());
    m_pauseButton->setEnabled(pauseEnabled);

    const bool playEnabled = (QAudio::AudioOutput != m_engine->mode() ||
                              (QAudio::ActiveState != m_engine->state() &&
                               QAudio::IdleState != m_engine->state() &&
                               QAudio::InterruptedState != m_engine->state()));
    m_playButton->setEnabled(playEnabled);
}

void SpectrographUI::connectUI()
{
    connect(m_pauseButton, &QPushButton::clicked,
        m_engine, &Engine::suspend);

    connect(m_playButton, &QPushButton::clicked,
        m_engine, &Engine::startPlayback);

    connect(m_settingsButton, &QPushButton::clicked,
        this, &SpectrographUI::showSettingsDialog);

    connect(m_engine, &Engine::stateChanged,
        this, &SpectrographUI::stateChanged);

    /*connect(m_engine, &Engine::formatChanged,
        this, &SpectrographUI::formatChanged);

    m_progressBar->bufferLengthChanged(m_engine->bufferLength());

    connect(m_engine, &Engine::bufferLengthChanged,
        this, &SpectrographUI::bufferLengthChanged);*/

    connect(m_engine, &Engine::dataLengthChanged,
        this, &SpectrographUI::updateButtonStates);

   /* connect(m_engine, &Engine::recordPositionChanged,
        m_progressBar, &ProgressBar::recordPositionChanged);

    connect(m_engine, &Engine::playPositionChanged,
        m_progressBar, &ProgressBar::playPositionChanged);

    connect(m_engine, &Engine::recordPositionChanged,
        this, &MainWidget::audioPositionChanged);

    connect(m_engine, &Engine::playPositionChanged,
        this, &MainWidget::audioPositionChanged);

    connect(m_engine, &Engine::levelChanged,
        m_levelMeter, &LevelMeter::levelChanged);

    connect(m_engine, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&Engine::spectrumChanged),
        this, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&MainWidget::spectrumChanged));

    connect(m_engine, &Engine::infoMessage,
        this, &MainWidget::infoMessage);

    connect(m_engine, &Engine::errorMessage,
        this, &MainWidget::errorMessage);

    connect(m_spectrograph, &Spectrograph::infoMessage,
        this, &MainWidget::infoMessage);*/

#ifndef DISABLE_WAVEFORM
    connect(m_engine, &Engine::bufferChanged,
        m_waveform, &Waveform::bufferChanged);
#endif
}

void SpectrographUI::stateChanged(QAudio::Mode mode, QAudio::State state)
{
    Q_UNUSED(mode);

    updateButtonStates();

    if (QAudio::ActiveState != state &&
        QAudio::SuspendedState != state &&
        QAudio::InterruptedState != state) {
        //m_levelMeter->reset();
        //m_spectrograph->reset();
    }
}

void SpectrographUI::resetUI()
{
    m_waveform->reset();
    //m_spectrograph->reset();
}

void SpectrographUI::quitApplication()
{
    QCoreApplication::quit();
}