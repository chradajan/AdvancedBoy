#pragma once

#include <QtWidgets/QWidget>

class PersistentData;

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

private:
    PersistentData& settings_;
};
}