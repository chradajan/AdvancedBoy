#pragma once

#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QWidget>

namespace gui
{
class AudioTab : public QWidget
{
    Q_OBJECT

public:
    AudioTab() = delete;
    AudioTab(AudioTab const&) = delete;
    AudioTab& operator=(AudioTab const&) = delete;
    AudioTab(AudioTab&&) = delete;
    AudioTab& operator=(AudioTab&&) = delete;

    /// @brief Initialize the audio tab widget.
    /// @param settings Reference to settings.
    AudioTab(PersistentData& settings);

    /// @brief Restore default audio settings.
    void RestoreDefaults();

signals:
    /// @brief Signal to emit when emulation audio output needs to be adjusted.
    void UpdateAudioSignal(PersistentData::AudioSettings audioSettings);

private slots:
    /// @brief Slot to handle the mute option being toggled.
    /// @param state Whether the mute box is checked.
    void MuteToggledSlot(bool state);

    /// @brief Slot to handle the volume slider changing.
    /// @param volume New volume value.
    void VolumeChangedSlot(int volume);

    /// @brief Slot to handle an APU channel box being toggled.
    /// @param channel Which channel was toggled.
    /// @param state Whether the channel should be enabled/disabled.
    void ApuChannelToggledSlot(PersistentData::Channel channel, bool state);

private:
    PersistentData& settings_;
};
}
