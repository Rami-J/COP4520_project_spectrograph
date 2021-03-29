#include "SpectrographUI.h"

static const QString WAV_ICON_PATH = ":/images/audio-icon.png";
static const QString SETTINGS_ICON_PATH = ":/images/settings.png";
static const QString VOLUME_ICON_PATH = ":/images/volume.png";
static const QString VOLUME_MUTED_ICON_PATH = ":/images/volume_muted.png";
static const QString SPECTROGRAPH_TITLE = "Spectrograph (Frequency vs. Amplitude)";
static const int MIN_VOLUME = 0;
static const int MAX_VOLUME = 100;

SpectrographUI::SpectrographUI(QWidget *parent)
    : QMainWindow(parent)
    , m_waveform(new Waveform("Audio Output: Default", Waveform::DEFAULT_SAMPLE_COUNT, this))
    , m_spectrograph(new Spectrograph(SPECTROGRAPH_TITLE, this))
    , m_volumeSlider(new QSlider(Qt::Horizontal, this))
    , m_openWavFileButton(new QPushButton(this))
    , m_pauseButton(new QPushButton(this))
    , m_playButton(new QPushButton(this))
    , m_settingsButton(new QPushButton(this))
    , m_volumeMuteButton(new QPushButton(this))
    , m_settingsDialog(new SettingsDialog(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput),
                                          this))
    , m_volumeMuted(false)
{
    createLayouts();
    createActions();
    createMenus();
    connectUI();

    setWindowTitle(tr("Spectrograph"));
    setMinimumSize(800, 740);
    resize(800, 740);
}

void SpectrographUI::createLayouts()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);

    QLabel* waveformLabel = new QLabel("Spectrum Waveform");
    waveformLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(waveformLabel);
    mainLayout->addWidget(m_waveform->getChartView());
    mainLayout->addWidget(m_spectrograph->getChartView());

    m_deviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    QAudioFormat desired_audio_format = m_deviceInfo.preferredFormat();

    qDebug() << "SpectrographUI::createLayouts() Your device is " << m_deviceInfo.deviceName();
    qDebug() << "SpectrographUI::createLayouts() Your preferred device format = " << m_deviceInfo.preferredFormat();

    m_device = new AudioFileStream(m_waveform, m_spectrograph, this);
    if (!m_device->init(desired_audio_format))
    {
        showWarningDialog("Failed to initialize audio file stream with default output device!");
    }
    
    m_audioOutput = new QAudioOutput(desired_audio_format, this);
    m_audioOutput->start(m_device);

    updateChartTitle();

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

    m_volumeOnIcon = QIcon(VOLUME_ICON_PATH);
    m_volumeMutedIcon = QIcon(VOLUME_MUTED_ICON_PATH);
    m_volumeMuteButton->setIcon(m_volumeOnIcon);
    m_volumeMuteButton->setEnabled(true);
    m_volumeMuteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_volumeMuteButton->setMinimumSize(buttonSize);

    m_volumeSlider->setMaximumWidth(m_volumeSlider->width());

    QHBoxLayout* volumeSliderLayout = new QHBoxLayout(this);
    volumeSliderLayout->setAlignment(Qt::AlignRight);
    volumeSliderLayout->addWidget(m_volumeMuteButton);
    volumeSliderLayout->addWidget(m_volumeSlider);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_openWavFileButton);
    buttonPanelLayout->addWidget(m_pauseButton);
    buttonPanelLayout->addWidget(m_playButton);
    buttonPanelLayout->addWidget(m_settingsButton);
    buttonPanelLayout->addLayout(volumeSliderLayout);

    m_volumeSlider->setMinimum(MIN_VOLUME);
    m_volumeSlider->setMaximum(MAX_VOLUME);
    m_volumeSlider->setValue(MAX_VOLUME);

    QWidget* buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    m_waveform->getChart()->setTitle("Audio Output: " + m_deviceInfo.deviceName());
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

        m_device->cancelSpectrum();
        m_device->stop();
        m_audioOutput->reset();
        m_audioOutput->stop();

        if (!m_device->loadFile(m_currentFilePath))
        {
            showWarningDialog("Failed to load audio file.");
            return;
        }

        m_audioOutput->start(m_device);
        m_playButton->setEnabled(true);
    }
}

void SpectrographUI::showSettingsDialog()
{
    m_settingsDialog->exec();
    if (m_settingsDialog->result() == QDialog::Accepted) 
    {
        if (!setAudioOutputDevice(m_settingsDialog->outputDevice()))
            return;

        updateChartTitle();
    }
}

bool SpectrographUI::setAudioOutputDevice(const QAudioDeviceInfo& device)
{
    if (device.deviceName() != m_deviceInfo.deviceName())
    {
        if (m_device->getState() == AudioFileStream::State::Playing)
        {
            m_device->pause();
        }

        qDebug() << "SpectrographUI::setAudioOutputDevice() Your device is " << device.deviceName();
        qDebug() << "SpectrographUI::setAudioOutputDevice() Your preferred device format = " << device.preferredFormat();

        if (!m_device->setFormat(device.preferredFormat()))
        {
            showWarningDialog("Output device format not supported.");
            return false;
        }

        m_deviceInfo = device;
        m_audioOutput->reset();
        m_audioOutput->stop();
        delete m_audioOutput;

        m_audioOutput = new QAudioOutput(m_deviceInfo, m_device->getFormat(), this);
        m_audioOutput->start(m_device);

        if (m_device->getState() == AudioFileStream::State::Paused)
        {
            if (!m_device->play(m_currentFilePath))
            {
                showWarningDialog("Failed to play audio file.", "File: " + m_currentFilePath);
                return false;
            }
        }

        return true;
    }
}

void SpectrographUI::connectUI()
{
    connect(m_openWavFileButton, &QPushButton::clicked,
        this, &SpectrographUI::showFileDialog);

    connect(m_volumeMuteButton, &QPushButton::clicked,
            this, &SpectrographUI::toggleVolumeMute);

    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &SpectrographUI::volumeChanged);

    connect(m_playButton, &QPushButton::clicked,
        this, &SpectrographUI::startPlayback);

    connect(m_pauseButton, &QPushButton::clicked,
            this, &SpectrographUI::pausePlayback);

    connect(m_settingsButton, &QPushButton::clicked,
        this, &SpectrographUI::showSettingsDialog);

    connect(m_device, &AudioFileStream::stateChanged,
            this, &SpectrographUI::stateChanged);

    /*
    connect(m_spectrograph, &Spectrograph::infoMessage,
        this, &MainWidget::infoMessage);
    */
}

void SpectrographUI::startPlayback()
{
    if (!m_device->play(m_currentFilePath))
    {
        showWarningDialog("Failed to play audio file.", "File: " + m_currentFilePath);
        return;
    }

    m_playButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
}

void SpectrographUI::pausePlayback()
{
    m_device->pause();
    m_playButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
}

void SpectrographUI::toggleVolumeMute()
{
    m_volumeMuted = !m_volumeMuted;
    
    if (m_volumeMuted)
    {
        m_volumeSlider->setValue(MIN_VOLUME);
        m_volumeMuteButton->setIcon(m_volumeMutedIcon);
    }
    else
    {
        m_volumeSlider->setValue(MAX_VOLUME);
        m_volumeMuteButton->setIcon(m_volumeOnIcon);
    }
}

void SpectrographUI::volumeChanged(int value)
{
    if (value == MIN_VOLUME)
    {
        m_volumeMuted = true;
        m_volumeMuteButton->setIcon(m_volumeMutedIcon);
    }
    else if (value > MIN_VOLUME && m_volumeMuted == true)
    {
        m_volumeMuted = false;
        m_volumeMuteButton->setIcon(m_volumeOnIcon);
    }

    qreal volume = qreal(value) / 100;
    m_audioOutput->setVolume(volume);
}

void SpectrographUI::stateChanged(AudioFileStream::State state)
{
    switch (state)
    {
        case AudioFileStream::State::Playing:
        {
            m_pauseButton->setEnabled(true);
            m_playButton->setEnabled(false);
            break;
        }
        case AudioFileStream::State::Paused:
        {
            m_pauseButton->setEnabled(false);
            m_playButton->setEnabled(true);
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

void SpectrographUI::resetUI()
{
    //m_spectrograph->reset();
}

void SpectrographUI::quitApplication()
{
    QCoreApplication::quit();
}
