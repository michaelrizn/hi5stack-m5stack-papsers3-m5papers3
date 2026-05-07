#include "off_screen.h"
#include "../ui.h"

namespace screens {
    void drawOffScreen() {

        ::currentScreen = CLEAR_SCREEN;
        ::renderCurrentScreen();


        ::bufferRow("Powering off...", 2);
        ::bufferRow("For full shutdown, double-click", 3);
        ::bufferRow("the side button.", 4);


        ::drawRowsBuffered();
        M5.Display.display();
    }
}
