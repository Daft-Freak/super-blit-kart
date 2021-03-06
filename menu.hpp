#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "engine/menu.hpp"

class Menu final : public blit::Menu {
public:
    Menu(std::string_view title, std::vector<Item> items, const blit::Font &font = blit::minimal_font);

    void add_item(Item &&item);

    void set_on_item_activated(std::function<void(const Item &)> func);

    int get_current_item() const {return current_item;}

    const blit::Rect &get_display_rect() {return display_rect;}

private:
    void render_item(const Item &item, int y, int index) const override;

    void item_activated(const Item &item) override;

    std::vector<Item> items_vec;

    std::function<void(const Item &)> on_item_pressed = nullptr;
};