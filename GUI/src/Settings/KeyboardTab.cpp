#include <GUI/include/Settings/KeyboardTab.hpp>
#include <GUI/include/PersistentData.hpp>
#include <GUI/include/Bindings.hpp>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace gui
{
KeyboardTab::KeyboardTab(PersistentData& settings) : buttonToUpdate_(nullptr), settings_(settings)
{
    // Establish layout
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(new QLabel("Key"), 0, 0);

    QLabel* binding1Label = new QLabel("Binding 1");
    binding1Label->setAlignment(Qt::AlignCenter);
    layout->addWidget(binding1Label, 0, 1);

    QLabel* binding2Label = new QLabel("Binding 2");
    binding2Label->setAlignment(Qt::AlignCenter);
    layout->addWidget(binding2Label, 0, 2);

    int row = 1;

    for (auto [buttonName, gbaKey] : BUTTON_NAMES)
    {
        QLabel* gbaKeyLabel = new QLabel(buttonName);
        layout->addWidget(gbaKeyLabel, row, 0);

        QPushButton* primaryButton = new QPushButton;
        primaryButton->setFocusPolicy(Qt::NoFocus);
        primaryButton->setObjectName(buttonName + "Primary");
        connect(primaryButton, &QPushButton::clicked,
                this, [=, this] () { this->PrepareKeyBindingChange(primaryButton, gbaKey, true); });
        layout->addWidget(primaryButton, row, 1);

        QPushButton* secondaryButton = new QPushButton;
        secondaryButton->setFocusPolicy(Qt::NoFocus);
        secondaryButton->setObjectName(buttonName + "Secondary");
        connect(secondaryButton, &QPushButton::clicked,
                this, [=, this] () { this->PrepareKeyBindingChange(secondaryButton, gbaKey, false); });
        layout->addWidget(secondaryButton, row, 2);

        ++row;
    }

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(spacer, row, 0);

    setLayout(layout);

    // Update button names
    UpdateButtons();
}

void KeyboardTab::CancelRebind()
{
    if (buttonToUpdate_ != nullptr)
    {
        buttonToUpdate_->setText(textToRestore_);
        buttonToUpdate_ = nullptr;
    }
}

void KeyboardTab::RestoreDefaults()
{
    settings_.RestoreDefaultKeyboardBindings();
    UpdateButtons();
}

void KeyboardTab::SetNewKeyboardBindingSlot(Qt::Key key)
{
    if (buttonToUpdate_ == nullptr)
    {
        return;
    }

    if (key == Qt::Key_Escape)
    {
        buttonToUpdate_->setText(textToRestore_);
        buttonToUpdate_ = nullptr;
    }
    else
    {
        settings_.SetKeyboardBinding(gbaKeyToUpdate_, key, updatePrimary_);
        buttonToUpdate_->setText(GetKeyboardBindingName(key));
        buttonToUpdate_ = nullptr;
    }
}

void KeyboardTab::PrepareKeyBindingChange(QPushButton* button, GBAKey gbaKey, bool primary)
{
    if (buttonToUpdate_ == button)
    {
        return;
    }

    if (buttonToUpdate_ != nullptr)
    {
        buttonToUpdate_->setText(textToRestore_);
    }

    textToRestore_ = button->text();
    button->setText("...");
    buttonToUpdate_ = button;
    gbaKeyToUpdate_ = gbaKey;
    updatePrimary_ = primary;
    emit GetNewKeyboardBindingSignal();
}

void KeyboardTab::UpdateButtons()
{
    for (auto [buttonName, gbaKey] : BUTTON_NAMES)
    {
        QPushButton* primaryButton = findChild<QPushButton*>(buttonName + "Primary");
        primaryButton->setText(GetKeyboardBindingName(settings_.GetKeyboardBinding(gbaKey, true)));

        QPushButton* secondaryButton = findChild<QPushButton*>(buttonName + "Secondary");
        secondaryButton->setText(GetKeyboardBindingName(settings_.GetKeyboardBinding(gbaKey, false)));
    }
}
}
