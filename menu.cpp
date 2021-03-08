#include "menu.hpp"

#include "engine/engine.hpp"

Menu::Menu(std::string_view title, std::vector<Item> items, const blit::Font &font) : blit::Menu(title, nullptr, 0, font), items_vec(std::move(items)) {
    this->items = items_vec.data();
    num_items = items_vec.size();

    item_h = font.char_h + 10;
    item_adjust_y = 0;

    header_h = title.empty() ? 0 : item_h;
    footer_h = 0;
    margin_y = 0;

    background_colour = blit::Pen(0x11, 0x11, 0x11);
    foreground_colour = blit::Pen(0xF7, 0xF7, 0xF7);
    selected_item_background = blit::Pen(0x22, 0x22, 0x22);
}

void Menu::set_on_item_activated(void (*func)(const Item &)) {
    on_item_pressed = func;
}

void Menu::item_activated(const Item &item) {
    if(on_item_pressed)
        on_item_pressed(item);
}