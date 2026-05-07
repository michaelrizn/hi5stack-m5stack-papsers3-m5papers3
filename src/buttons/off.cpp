#include "off.h"
#include "../ui.h"
#include "../power_shutdown.h"
#include "../screens/off_screen.h"
extern Message currentMessage;

void showOffScreen() {
    footer.setVisible(false);
    currentMessage.text = "";

    screens::drawOffScreen();

    power_shutdown::powerOff();
}
