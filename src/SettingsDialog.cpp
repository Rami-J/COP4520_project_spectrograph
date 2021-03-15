#include "SettingsDialog.h"
#include "SpectrographUI.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QDebug>

SettingsDialog::SettingsDialog(const QList<QAudioDeviceInfo>& availableOutputDevices,
                               QWidget* parent)
    : QDialog(parent)
    , m_outputDeviceComboBox(new QComboBox(this))
{
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);

    // Populate output device combo box
    for (const QAudioDeviceInfo& device : availableOutputDevices)
        m_outputDeviceComboBox->addItem(device.deviceName(),
            QVariant::fromValue(device));

    // Initialize default output devices
    if (!availableOutputDevices.empty())
        m_outputDevice = availableOutputDevices.front();

    // Add widgets to layout
    QScopedPointer<QHBoxLayout> outputDeviceLayout(new QHBoxLayout);
    QLabel* outputDeviceLabel = new QLabel(tr("Output device"), this);
    outputDeviceLayout->addWidget(outputDeviceLabel);
    outputDeviceLayout->addWidget(m_outputDeviceComboBox);
    dialogLayout->addLayout(outputDeviceLayout.data());
    outputDeviceLayout.take(); // ownership transferred to dialogLayout

    // Connect
    connect(m_outputDeviceComboBox, QOverload<int>::of(&QComboBox::activated),
        this, &SettingsDialog::outputDeviceChanged);

    // Add standard buttons to layout
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogLayout->addWidget(buttonBox);

    // Connect standard buttons
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
        this, &SettingsDialog::accept);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &SettingsDialog::reject);

    setLayout(dialogLayout);
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::outputDeviceChanged(int index)
{
    m_outputDevice = m_outputDeviceComboBox->itemData(index).value<QAudioDeviceInfo>();
}

