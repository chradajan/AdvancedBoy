#pragma once

#include <GUI/include/Bindings.hpp>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class PersistentData;

namespace gui
{
class KeyboardTab : public QWidget
{
    Q_OBJECT

public:
    KeyboardTab() = delete;
    KeyboardTab(KeyboardTab const&) = delete;
    KeyboardTab& operator=(KeyboardTab const&) = delete;
    KeyboardTab(KeyboardTab&&) = delete;
    KeyboardTab& operator=(KeyboardTab&&) = delete;

    /// @brief Initialize the keyboard tab widget.
    /// @param settings Reference to settings.
    KeyboardTab(PersistentData& settings);

    /// @brief Cancel the rebinding of a button.
    void CancelRebind();

    /// @brief Restore default keyboard bindings.
    void RestoreDefaults();

signals:
    /// @brief Emit to notify the options window to listen for next key that's pressed.
    void GetNewKeyboardBindingSignal();

public slots:
    /// @brief Slot to update a key that's being rebound.
    /// @param key Most recently pressed keyboard key.
    void SetNewKeyboardBindingSlot(Qt::Key key);

private:
    /// @brief Prepare for a keyboard binding to be updated after a button in the bindings box has been clicked.
    /// @param button Pointer to button that was clicked.
    /// @param gbaKey GBA key to set the new binding for.
    /// @param primary Whether it was the primary or secondary binding button that was clicked.
    void PrepareKeyBindingChange(QPushButton* button, GBAKey gbaKey, bool primary);

    /// @brief Update the text on all binding buttons based on current keyboard bindings.
    void UpdateButtons();

    // Rebind info
    QPushButton* buttonToUpdate_;
    GBAKey gbaKeyToUpdate_;
    bool updatePrimary_;
    QString textToRestore_;

    // Settings
    PersistentData& settings_;
};
}
