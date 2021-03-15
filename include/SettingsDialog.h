#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QAudioDeviceInfo>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QSlider;
class QSpinBox;
class QGridLayout;
QT_END_NAMESPACE

/**
 * Dialog used to control settings such as the audio input / output device
 * and the windowing function.
 * 
 * Based on Qt Multimedia Spectrum example https://doc.qt.io/qt-5/qtmultimedia-multimedia-spectrum-example.html
 */
    class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(const QList<QAudioDeviceInfo>& availableOutputDevices,
                   QWidget* parent = 0);
    ~SettingsDialog();

    const QAudioDeviceInfo& outputDevice() const { return m_outputDevice; }

private slots:
    void outputDeviceChanged(int index);

private:
    QAudioDeviceInfo m_outputDevice;

    QComboBox* m_outputDeviceComboBox;
};

#endif // SETTINGSDIALOG_H
