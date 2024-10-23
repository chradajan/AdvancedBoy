#pragma once

#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QWidget>

namespace gui
{
class OptionsWindow : public QWidget
{
    Q_OBJECT

public:
    OptionsWindow() = delete;
    OptionsWindow(OptionsWindow const&) = delete;
    OptionsWindow& operator=(OptionsWindow const&) = delete;
    OptionsWindow(OptionsWindow&&) = delete;
    OptionsWindow& operator=(OptionsWindow&&) = delete;

    /// @brief Initialize the settings menu.
    OptionsWindow(PersistentData& settings);

signals:
    /// @brief Signal to emit when emulation audio output needs to be adjusted.
    void UpdateAudioSignal(PersistentData::AudioSettings audioSettings);

private slots:
    /// @brief Determine which tab is currently active and restore its default settings.
    void RestoreDefaultsSlot();

private:
    PersistentData& settings_;
};
}
