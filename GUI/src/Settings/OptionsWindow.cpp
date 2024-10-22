#include <GUI/include/Settings/OptionsWindow.hpp>
#include <GUI/include/PersistentData.hpp>

namespace gui
{
OptionsWindow::OptionsWindow(PersistentData& settings) : settings_(settings)
{
    setWindowTitle("Options");
}
}
