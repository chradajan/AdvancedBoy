#include <GUI/include/Settings/GamepadTab.hpp>
#include <array>
#include <cstring>
#include <ranges>
#include <utility>
#include <GUI/include/Bindings.hpp>
#include <GUI/include/PersistentData.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <SDL2/SDL.h>

namespace gui
{
static const std::array<std::pair<QString, GBAKey>, 10> buttonNames = {{
    {"Up", GBAKey::UP},
    {"Down", GBAKey::DOWN},
    {"Left", GBAKey::LEFT},
    {"Right", GBAKey::RIGHT},
    {"L", GBAKey::L},
    {"R", GBAKey::R},
    {"A", GBAKey::A},
    {"B", GBAKey::B},
    {"Start", GBAKey::START},
    {"Select", GBAKey::SELECT},
}};

///---------------------------------------------------------------------------------------------------------------------------------
/// Deadzone Slider
///---------------------------------------------------------------------------------------------------------------------------------

DeadzoneSlider::DeadzoneSlider(int deadzone)
{
    QHBoxLayout* layout = new QHBoxLayout;

    slider_ = new QSlider(Qt::Horizontal);
    slider_->setMinimum(0);
    slider_->setMaximum(100);
    slider_->setValue(deadzone);
    connect(slider_, &QSlider::valueChanged, this, &DeadzoneSlider::DeadzoneChangedSlot);
    layout->addWidget(slider_);

    if ((deadzone < 0) || (deadzone > 100))
    {
        deadzone = 5;
    }

    label_ = new QLabel(QString::number(deadzone) + "%");
    layout->addWidget(label_);

    setLayout(layout);
}

void DeadzoneSlider::SetDeadzone(int deadzone)
{
    if ((deadzone < 0) || (deadzone > 100))
    {
        deadzone = 5;
    }

    slider_->blockSignals(true);
    slider_->setValue(deadzone);
    slider_->blockSignals(false);
    label_->setText(QString::number(deadzone) + "%");
}

void DeadzoneSlider::DeadzoneChangedSlot(int deadzone)
{
    label_->setText(QString::number(deadzone) + "%");
    emit DeadzoneChangedSignal(deadzone);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Gamepad Tab
///---------------------------------------------------------------------------------------------------------------------------------


GamepadTab::GamepadTab(PersistentData& settings) : settings_(settings)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(CreateGamepadSelectBox());
    layout->addWidget(CreateBindingsBox());
    layout->addStretch();
    setLayout(layout);

    gamepad_ = nullptr;
    joystickID_ = -1;

    buttonToUpdate_ = nullptr;
    gbaKeyToUpdate_ = GBAKey::INVALID;
    updatePrimary_ = false;
    textToRestore_ = "";

    PopulateGamepadDropdown();
    UpdateButtons();
}

void GamepadTab::UpdateGamepadList()
{
    if (buttonToUpdate_ != nullptr)
    {
        buttonToUpdate_->setText(textToRestore_);
        buttonToUpdate_ = nullptr;
    }

    PopulateGamepadDropdown();
    UpdateButtons();
}

void GamepadTab::CancelRebind()
{
    if (buttonToUpdate_ != nullptr)
    {
        buttonToUpdate_->setText(textToRestore_);
        buttonToUpdate_ = nullptr;
    }
}

void GamepadTab::RestoreDefaults()
{
    buttonToUpdate_ = nullptr;
    settings_.RestoreDefaultGamepadBindings(false);
    UpdateButtons();
    findChild<DeadzoneSlider*>("DeadzoneSlider")->SetDeadzone(settings_.GetDeadzone());
    emit BindingsChangedSignal();
}

void GamepadTab::SetNewGamepadBindingSlot(GamepadBinding newBinding)
{
    if (buttonToUpdate_ != nullptr)
    {
        settings_.SetGamepadBinding(gbaKeyToUpdate_, newBinding, updatePrimary_);
        buttonToUpdate_->setText(newBinding.ToString(gamepad_));
        buttonToUpdate_ = nullptr;
        emit BindingsChangedSignal();
    }
}

void GamepadTab::GamepadSelectionSlot(int index)
{
    if ((index != -1) && (static_cast<size_t>(index) < joystickIndexes_.size()))
    {
        int joystickIndex = joystickIndexes_[index];
        settings_.SetGUID(SDL_JoystickGetDeviceGUID(joystickIndex));
        gamepad_ = SDL_GameControllerOpen(joystickIndex);
        joystickID_ = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_));
    }
    else
    {
        gamepad_ = nullptr;
        joystickID_ = -1;
    }

    if (buttonToUpdate_ != nullptr)
    {
        buttonToUpdate_->setText(textToRestore_);
        buttonToUpdate_ = nullptr;
    }

    UpdateButtons();
    emit SetGamepadSignal(gamepad_);
}

void GamepadTab::DeadzoneChangedSlot(int deadzone)
{
    settings_.SetDeadzone(deadzone);
    emit BindingsChangedSignal();
}

QGroupBox* GamepadTab::CreateGamepadSelectBox() const
{
    QGroupBox* gamepadSelectBox = new QGroupBox("Gamepads");
    QFormLayout* layout = new QFormLayout;

    QComboBox* dropdown = new QComboBox;
    connect(dropdown, &QComboBox::currentIndexChanged, this, &GamepadTab::GamepadSelectionSlot);
    dropdown->setObjectName("GamepadDropdown");
    layout->addRow("Select Gamepad", dropdown);

    DeadzoneSlider* deadzoneSlider = new DeadzoneSlider(settings_.GetDeadzone());
    deadzoneSlider->setObjectName("DeadzoneSlider");
    connect(deadzoneSlider, &DeadzoneSlider::DeadzoneChangedSignal, this, &GamepadTab::DeadzoneChangedSlot);
    layout->addRow("Deadzone", deadzoneSlider);

    gamepadSelectBox->setLayout(layout);
    return gamepadSelectBox;
}

QGroupBox* GamepadTab::CreateBindingsBox()
{
    QGroupBox* bindingsBox = new QGroupBox("Bindings");
    QGridLayout* layout = new QGridLayout;

    // Columns headers
    layout->addWidget(new QLabel("Button"), 0, 0);
    layout->addWidget(new QLabel("Primary Binding"), 0, 1);
    layout->addWidget(new QLabel("Secondary Binding"), 0, 2);

    // Button labels and QPushButtons
    int row = 1;

    for (auto [buttonName, gbaKey] : buttonNames)
    {
        layout->addWidget(new QLabel(buttonName), row, 0);

        QPushButton* primaryButton = new QPushButton;
        primaryButton->setObjectName(buttonName + "Primary");
        connect(primaryButton, &QPushButton::clicked,
                this, [=, this] () { this->PrepareKeyBindingChange(primaryButton, gbaKey, true); });
        layout->addWidget(primaryButton, row, 1);

        QPushButton* secondaryButton = new QPushButton;
        secondaryButton->setObjectName(buttonName + "Secondary");
        connect(secondaryButton, &QPushButton::clicked,
                this, [=, this] () { this->PrepareKeyBindingChange(secondaryButton, gbaKey, false); });
        layout->addWidget(secondaryButton, row, 2);

        ++row;
    }

    bindingsBox->setLayout(layout);
    return bindingsBox;
}

void GamepadTab::PrepareKeyBindingChange(QPushButton* button, GBAKey gbaKey, bool primary)
{
    if ((joystickID_ == -1) || (buttonToUpdate_ != nullptr))
    {
        return;
    }

    textToRestore_ = button->text();
    button->setText("...");
    buttonToUpdate_ = button;
    gbaKeyToUpdate_ = gbaKey;
    updatePrimary_ = primary;
    emit GetNewGamepadBindingSignal(joystickID_);
}

void GamepadTab::UpdateButtons()
{
    for (auto [buttonName, gbaKey] : buttonNames)
    {
        QPushButton* primaryButton = findChild<QPushButton*>(buttonName + "Primary");
        primaryButton->setText(settings_.GetGamepadBinding(gbaKey, true).ToString(gamepad_));

        QPushButton* secondaryButton = findChild<QPushButton*>(buttonName + "Secondary");
        secondaryButton->setText(settings_.GetGamepadBinding(gbaKey, false).ToString(gamepad_));
    }
}

void GamepadTab::PopulateGamepadDropdown()
{
    joystickIndexes_.clear();
    auto* dropdown = findChild<QComboBox*>("GamepadDropdown");
    dropdown->blockSignals(true);
    dropdown->clear();
    int dropdownIndex = 0;
    SDL_JoystickGUID savedGUID = settings_.GetGUID();

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            joystickIndexes_.push_back(i);
            const char* gamepadName = SDL_GameControllerNameForIndex(i);
            QString gamepadStr = (gamepadName == nullptr) ? "UNKNOWN" : gamepadName;
            dropdown->addItem(gamepadStr);
            SDL_JoystickGUID currGUID = SDL_JoystickGetDeviceGUID(i);

            if (std::ranges::equal(savedGUID.data, currGUID.data))
            {
                dropdownIndex = i;
            }
        }
    }

    if (dropdown->count() > 0)
    {
        int joystickIndex = joystickIndexes_[dropdownIndex];
        settings_.SetGUID(SDL_JoystickGetDeviceGUID(joystickIndex));
        dropdown->setCurrentIndex(dropdownIndex);
        gamepad_ = SDL_GameControllerOpen(joystickIndex);
        joystickID_ = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_));
    }
    else
    {
        gamepad_ = nullptr;
        joystickID_ = -1;
        buttonToUpdate_ = nullptr;
    }

    emit SetGamepadSignal(gamepad_);
    dropdown->blockSignals(false);
}
}
