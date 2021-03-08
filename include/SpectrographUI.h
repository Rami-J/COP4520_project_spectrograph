#pragma once

#include "Waveform.h"
#include "Spectrograph.h"
#include "Engine.h"
#include "SettingsDialog.h"
#include "ProgressBar.h"
#include "AudioFileStream.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtGui/QIcon>
#include <QCoreApplication>
#include <QStyle>
#include <QString>
#include <QFileDialog>
#include <QtMultimedia/QAudio>
#include <QtMultimedia/QAudioOutput>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE


class SpectrographUI : public QMainWindow
{
    Q_OBJECT

public:
    SpectrographUI(QWidget *parent = Q_NULLPTR);
    Engine* getEngine();

public slots:
    //void stateChanged(QAudio::Mode mode, QAudio::State state);
    void stateChanged(AudioFileStream::State state);
    void formatChanged(const QAudioFormat& format);
    void bufferLengthChanged(qint64 length);
    void audioPositionChanged(qint64 position);

private slots:
    void showFileDialog();
    void showSettingsDialog();
    void updateButtonStates();
    void startPlayback();
    void pausePlayback();

private:
    void createLayouts();
    void createActions();
    void createMenus();
    void updateChartTitle();
    void connectUI();
    void resetUI();
	void showWarningDialog(QString msg, QString informativeMsg = "");
    void quitApplication();

    AudioFileStream* m_device = nullptr;
    QChart* m_chart;
    QLineSeries* m_series;
    QAudioOutput* m_audioOutput = nullptr;

    Engine* m_engine;
    //ProgressBar* m_progressBar;
    SettingsDialog* m_settingsDialog;
    Waveform* m_waveform;
    //Spectrograph* m_spectrograph;
    QMenu* m_fileMenu;
    QAction* m_openFileAct;
    QAction* m_exitAct;
    QPushButton* m_openWavFileButton;
    QIcon m_wavFileIcon;
    QPushButton* m_pauseButton;
    QIcon m_pauseIcon;
    QPushButton* m_playButton;
    QIcon m_playIcon;
    QPushButton* m_settingsButton;
    QIcon m_settingsIcon;

    QString m_currentFilePath;
};
