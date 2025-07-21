#ifndef FOOTER_H
#define FOOTER_H

#include <M5Unified.h>
#include <String>
#include <functional>

struct RowPosition;

const int MAX_FOOTER_BUTTONS = 4;
struct FooterButton {
    String label;
    std::function<void()> action;
};


class Footer {
public:
    Footer();
    void setButtons(const FooterButton* buttons, int count);
    void draw(bool visible);
    void setVisible(bool visible);
    bool isVisible() const;
    void invokeButtonAction(int index);
    const FooterButton* getButtons() const { return buttons; }
    int getButtonCount() const { return buttonCount; }

private:
    FooterButton buttons[MAX_FOOTER_BUTTONS];
    int buttonCount;
    bool visible;
};

#endif