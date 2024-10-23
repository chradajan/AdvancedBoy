#include <GUI/include/Settings/AudioTab.hpp>
#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSlider>

namespace gui
{
AudioTab::AudioTab(PersistentData& settings) : settings_(settings)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;

    // Volume control
    QFormLayout* volumeControlLayout = new QFormLayout;

    QCheckBox* muteBox = new QCheckBox;
    muteBox->setObjectName("MuteBox");
    muteBox->setChecked(settings.GetMuted());
    connect(muteBox, &QCheckBox::toggled, this, &AudioTab::MuteToggledSlot);
    volumeControlLayout->addRow("Mute", muteBox);

    QSlider* slider = new QSlider;
    slider->setObjectName("VolumeSlider");
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->setOrientation(Qt::Orientation::Horizontal);
    slider->setValue(settings.GetVolume());
    connect(slider, &QSlider::valueChanged, this, &AudioTab::VolumeChangedSlot);
    volumeControlLayout->addRow("Volume", slider);

    QGroupBox* volumeControlGroup = new QGroupBox("Volume");
    volumeControlGroup->setLayout(volumeControlLayout);
    mainLayout->addWidget(volumeControlGroup);

    // APU channels
    QFormLayout* apuChannelsLayout = new QFormLayout;

    QCheckBox* channel1Box = new QCheckBox;
    channel1Box->setObjectName("Channel1Box");
    channel1Box->setChecked(settings.GetChannelEnabled(PersistentData::Channel::ONE));
    connect(channel1Box, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::ONE, channel1Box->isChecked()); });
    apuChannelsLayout->addRow("Channel 1", channel1Box);

    QCheckBox* channel2Box = new QCheckBox;
    channel2Box->setObjectName("Channel2Box");
    channel2Box->setChecked(settings.GetChannelEnabled(PersistentData::Channel::TWO));
    connect(channel2Box, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::TWO, channel2Box->isChecked()); });
    apuChannelsLayout->addRow("Channel 2", channel2Box);

    QCheckBox* channel3Box = new QCheckBox;
    channel3Box->setObjectName("Channel3Box");
    channel3Box->setChecked(settings.GetChannelEnabled(PersistentData::Channel::THREE));
    connect(channel3Box, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::THREE, channel3Box->isChecked()); });
    apuChannelsLayout->addRow("Channel 3", channel3Box);

    QCheckBox* channel4Box = new QCheckBox;
    channel4Box->setObjectName("Channel4Box");
    channel4Box->setChecked(settings.GetChannelEnabled(PersistentData::Channel::FOUR));
    connect(channel4Box, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::FOUR, channel4Box->isChecked()); });
    apuChannelsLayout->addRow("Channel 4", channel4Box);

    QCheckBox* fifoABox = new QCheckBox;
    fifoABox->setObjectName("FifoABox");
    fifoABox->setChecked(settings.GetChannelEnabled(PersistentData::Channel::FIFO_A));
    connect(fifoABox, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::FIFO_A, fifoABox->isChecked()); });
    apuChannelsLayout->addRow("FIFO A", fifoABox);

    QCheckBox* fifoBBox = new QCheckBox;
    fifoBBox->setObjectName("FifoBBox");
    fifoBBox->setChecked(settings.GetChannelEnabled(PersistentData::Channel::FIFO_B));
    connect(fifoBBox, &QCheckBox::toggled,
            this, [=, this] () { this->ApuChannelToggledSlot(PersistentData::Channel::FIFO_B, fifoBBox->isChecked()); });
    apuChannelsLayout->addRow("FIFO B", fifoBBox);

    QGroupBox* apuChannelsGroup = new QGroupBox("APU Channels");
    apuChannelsGroup->setLayout(apuChannelsLayout);
    mainLayout->addWidget(apuChannelsGroup);

    setLayout(mainLayout);
}

void AudioTab::RestoreDefaults()
{
    settings_.RestoreDefaultAudioSettings();
    auto audioSettings = settings_.GetAudioSettings();

    QCheckBox* muteBox = findChild<QCheckBox*>("MuteBox");
    muteBox->blockSignals(true);
    muteBox->setChecked(audioSettings.mute);
    muteBox->blockSignals(false);

    QSlider* slider = findChild<QSlider*>("VolumeSlider");
    slider->blockSignals(true);
    slider->setValue(audioSettings.volume);
    slider->blockSignals(false);

    QCheckBox* channel1 = findChild<QCheckBox*>("Channel1Box");
    channel1->blockSignals(true);
    channel1->setChecked(audioSettings.channel1);
    channel1->blockSignals(false);

    QCheckBox* channel2 = findChild<QCheckBox*>("Channel2Box");
    channel2->blockSignals(true);
    channel2->setChecked(audioSettings.channel2);
    channel2->blockSignals(false);

    QCheckBox* channel3 = findChild<QCheckBox*>("Channel3Box");
    channel3->blockSignals(true);
    channel3->setChecked(audioSettings.channel3);
    channel3->blockSignals(false);

    QCheckBox* channel4 = findChild<QCheckBox*>("Channel4Box");
    channel4->blockSignals(true);
    channel4->setChecked(audioSettings.channel4);
    channel4->blockSignals(false);

    QCheckBox* fifoA = findChild<QCheckBox*>("FifoABox");
    fifoA->blockSignals(true);
    fifoA->setChecked(audioSettings.fifoA);
    fifoA->blockSignals(false);

    QCheckBox* fifoB = findChild<QCheckBox*>("FifoBBox");
    fifoB->blockSignals(true);
    fifoB->setChecked(audioSettings.fifoB);
    fifoB->blockSignals(false);

    emit UpdateAudioSignal(audioSettings);
}

void AudioTab::MuteToggledSlot(bool state)
{
    settings_.SetMuted(state);
    emit UpdateAudioSignal(settings_.GetAudioSettings());
}

void AudioTab::VolumeChangedSlot(int volume)
{
    settings_.SetVolume(volume);
    emit UpdateAudioSignal(settings_.GetAudioSettings());
}

void AudioTab::ApuChannelToggledSlot(PersistentData::Channel channel, bool state)
{
    settings_.SetChannelEnabled(channel, state);
    emit UpdateAudioSignal(settings_.GetAudioSettings());
}
}
