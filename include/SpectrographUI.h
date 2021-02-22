#pragma once

#include "Waveform.h"
#include "Spectrograph.h"
#include "Engine.h"
#include "SettingsDialog.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtGui/QIcon>
#include <QCoreApplication>
#include <QStyle>
#include <QFileDialog>
#include <QtMultimedia/QAudio>

class SpectrographUI : public QMainWindow
{
    Q_OBJECT

public:
    SpectrographUI(QWidget *parent = Q_NULLPTR);

public slots:
    void stateChanged(QAudio::Mode mode, QAudio::State state);

private slots:
    void showFileDialog();
    void showSettingsDialog();
    void updateButtonStates();

private:
    void createLayouts();
    void createActions();
    void createMenus();
    void connectUI();
    void resetUI();
    void quitApplication();

    Engine* m_engine;
    SettingsDialog* m_settingsDialog;
    Waveform* m_waveform;
    Spectrograph* m_spectrograph;
    QMenu* m_fileMenu;
    QAction* m_openFileAct;
    QAction* m_exitAct;
    QPushButton* m_pauseButton;
    QIcon m_pauseIcon;
    QPushButton* m_playButton;
    QIcon m_playIcon;
    QPushButton* m_settingsButton;
    QIcon m_settingsIcon;
};
