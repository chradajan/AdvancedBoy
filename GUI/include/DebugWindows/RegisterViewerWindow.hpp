#pragma once

#include <GBA/include/Utilities/Types.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QWidget>

namespace gui
{
class RegisterViewerWindow : public QWidget
{
    Q_OBJECT

public:
    RegisterViewerWindow(RegisterViewerWindow const&) = delete;
    RegisterViewerWindow& operator=(RegisterViewerWindow const&) = delete;
    RegisterViewerWindow(RegisterViewerWindow&&) = delete;
    RegisterViewerWindow& operator=(RegisterViewerWindow&&) = delete;

    /// @brief Initialize the I/O register viewer window.
    RegisterViewerWindow();

public slots:
    /// @brief Refresh the selected register's displayed data.
    void UpdateRegisterViewSlot() { UpdateSelectedRegisterData(); }

private:
    /// @brief Refresh the selected register's displayed data.
    void UpdateSelectedRegisterData();

    /// @brief Generate the dropdown menu that contains a list of all viewable I/O registers.
    /// @return QComboBox containing selectable registers.
    [[nodiscard]] QComboBox* CreateDropDown();

    /// @brief Generate a form with all the fields for the selected register.
    /// @return QGroupBox containing fields for selected register.
    [[nodiscard]] QGroupBox* CreateRegisterData();

    // Data
    QComboBox* registerSelect_;
    QGroupBox* registerData_;
};
}  // namespace gui
