#include "SpectrographUI.h"

static const QString WAV_ICON_PATH = ":/images/audio-icon.png";
static const QString SETTINGS_ICON_PATH = ":/images/settings.png";
static const int DEFAULT_SAMPLE_COUNT = 2000;

SpectrographUI::SpectrographUI(QWidget *parent)
    : QMainWindow(parent)
    , m_chart(new QChart)
    , m_series(new QLineSeries)
    , m_engine(new Engine(this))
    , m_openWavFileButton(new QPushButton(this))
    , m_pauseButton(new QPushButton(this))
    , m_playButton(new QPushButton(this))
    , m_settingsButton(new QPushButton(this))
    , m_settingsDialog(new SettingsDialog(m_engine->availableAudioOutputDevices(),
                                          this))
{
    createLayouts();
    createActions();
    createMenus();
    connectUI();

    setWindowTitle(tr("Spectrograph"));
    setMinimumSize(1000, 800);
    resize(480, 320);
}

void SpectrographUI::createLayouts()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);

    QLabel* waveformLabel = new QLabel("Spectrum Waveform");
    waveformLabel->setAlignment(Qt::AlignCenter);

    QChartView* chartView = new QChartView(m_chart);
    chartView->setMinimumSize(800, 600);
    m_chart->addSeries(m_series);
    QValueAxis* axisX = new QValueAxis;
    //axisX->setRange(0, AudioFileStream::sampleCount);
    axisX->setRange(0, DEFAULT_SAMPLE_COUNT);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Samples");
    QValueAxis* axisY = new QValueAxis;
    axisY->setRange(-1.5, 1.5);
    axisY->setTitleText("Audio level");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);
    m_chart->legend()->hide();
    updateChartTitle();

    mainLayout->addWidget(waveformLabel);
    mainLayout->addWidget(chartView);

    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    QAudioFormat desire_audio_romat = device.preferredFormat();

    m_device = new AudioFileStream(m_series, this);
    //m_device->init(m_engine->getAudioOutputDevice().preferredFormat());
    m_device->init(desire_audio_romat);
    m_device->setSampleCount(DEFAULT_SAMPLE_COUNT);

    m_audioOutput = new QAudioOutput(desire_audio_romat, this);
    m_audioOutput->start(m_device);

    //m_engine->setAudioFileStream(m_device);

    // Button panel
    const QSize buttonSize(30, 30);

    m_wavFileIcon = QIcon(WAV_ICON_PATH);
    m_openWavFileButton->setIcon(m_wavFileIcon);
    m_openWavFileButton->setEnabled(true);
    m_openWavFileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_openWavFileButton->setMinimumSize(buttonSize);

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

    m_settingsIcon = QIcon(SETTINGS_ICON_PATH);
    m_settingsButton->setIcon(m_settingsIcon);
    m_settingsButton->setEnabled(true);
    m_settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_settingsButton->setMinimumSize(buttonSize);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_openWavFileButton);
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
    bottomPaneLayout.take(); // ownership transferred to mainLayout

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

void SpectrographUI::updateChartTitle()
{
    m_chart->setTitle("Audio Output: " + m_engine->getAudioOutputDevice().deviceName());
}

Engine* SpectrographUI::getEngine()
{
    return m_engine;
}

void SpectrographUI::showWarningDialog(QString msg, QString informativeMsg)
{
	QMessageBox msgBox;
	msgBox.setText(msg);
	msgBox.setInformativeText(informativeMsg);
    msgBox.setIcon(QMessageBox::Icon::Warning);
	msgBox.exec();
}

void SpectrographUI::showFileDialog()
{
    const QString dir;
    const QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Audio file"), dir, "(*.wav *.mp3 *.mp4)");
    if (fileNames.count()) {
        resetUI();

        m_currentFilePath = fileNames.front();

        m_device->stop();
        m_audioOutput->reset();
        m_audioOutput->start(m_device);
        m_playButton->setEnabled(true);
        m_device->loadFile(m_currentFilePath);

        /*if (!m_engine->loadFile(fileNames.front()))
		{
            showWarningDialog("Failed to load WAV file.",
                              "The file may contain invalid headers.");
		}*/
        //m_device->play(fileNames.front());

        //updateButtonStates();
    }
}

void SpectrographUI::showSettingsDialog()
{
    m_settingsDialog->exec();
    if (m_settingsDialog->result() == QDialog::Accepted) {
        m_engine->setAudioOutputDevice(m_settingsDialog->outputDevice());
        m_engine->setWindowFunction(m_settingsDialog->windowFunction());
        updateChartTitle();
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
    connect(m_openWavFileButton, &QPushButton::clicked,
        this, &SpectrographUI::showFileDialog);

    //connect(m_pauseButton, &QPushButton::clicked,
        //m_engine, &Engine::suspend);

    //connect(m_playButton, &QPushButton::clicked,
        //m_engine, &Engine::startPlayback);

    connect(m_playButton, &QPushButton::clicked,
        this, &SpectrographUI::startPlayback);

    connect(m_pauseButton, &QPushButton::clicked,
            this, &SpectrographUI::pausePlayback);

    connect(m_settingsButton, &QPushButton::clicked,
        this, &SpectrographUI::showSettingsDialog);

    connect(m_device, &AudioFileStream::stateChanged,
            this, &SpectrographUI::stateChanged);

    //connect(m_engine, &Engine::stateChanged,
        //this, &SpectrographUI::stateChanged);

    //connect(m_engine, &Engine::formatChanged,
        //this, &SpectrographUI::formatChanged);

    //m_progressBar->bufferLengthChanged(m_engine->bufferLength());

    //connect(m_engine, &Engine::bufferLengthChanged,
        //this, &SpectrographUI::bufferLengthChanged);

    //connect(m_engine, &Engine::dataLengthChanged,
        //this, &SpectrographUI::updateButtonStates);

    //connect(m_engine, &Engine::playPositionChanged,
        //m_progressBar, &ProgressBar::playPositionChanged);

    //connect(m_engine, &Engine::playPositionChanged,
        //this, &SpectrographUI::audioPositionChanged);

    /*connect(m_engine, &Engine::levelChanged,
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
    //connect(m_engine, &Engine::bufferChanged,
        //m_waveform, &Waveform::bufferChanged);
#endif
}

void SpectrographUI::startPlayback()
{
    m_device->play(m_currentFilePath);
    m_playButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
}

void SpectrographUI::pausePlayback()
{
    //m_device->stop();
    m_device->pause();
    m_playButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
}

/*void SpectrographUI::stateChanged(QAudio::Mode mode, QAudio::State state)
{
    Q_UNUSED(mode);

    updateButtonStates();

    if (QAudio::ActiveState != state &&
        QAudio::SuspendedState != state &&
        QAudio::InterruptedState != state) {
        //m_levelMeter->reset();
        //m_spectrograph->reset();
    }
}*/

void SpectrographUI::stateChanged(AudioFileStream::State state)
{
    switch (state)
    {
        case AudioFileStream::State::Playing:
        {
            break;
        }
        case AudioFileStream::State::Paused:
        {
            break;
        }
        case AudioFileStream::State::Stopped:
        {
            m_pauseButton->setEnabled(false);
            m_playButton->setEnabled(true);
            break;
        }
    }
}

void SpectrographUI::formatChanged(const QAudioFormat& format)
{
    //infoMessage(formatToString(format), NullMessageTimeout);

    #ifndef DISABLE_WAVEFORM
        if (QAudioFormat() != format) {
            //m_waveform->initialize(format, WaveformTileLength,
                //WaveformWindowDuration);
        }
    #endif
}

void SpectrographUI::bufferLengthChanged(qint64 length)
{
    //m_progressBar->bufferLengthChanged(length);
}

void SpectrographUI::audioPositionChanged(qint64 position)
{
#ifndef DISABLE_WAVEFORM
    //m_waveform->audioPositionChanged(position);
#else
    Q_UNUSED(position)
#endif
}

void SpectrographUI::resetUI()
{
    //m_waveform->reset();
    //m_spectrograph->reset();
}

void SpectrographUI::quitApplication()
{
    QCoreApplication::quit();
}
