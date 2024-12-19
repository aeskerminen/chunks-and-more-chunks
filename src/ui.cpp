#include "ui.h"

struct nk_context *ctx = nullptr;

void init_ui(SDL_Window *window, SDL_Renderer *renderer)
{
    float font_scale = 1;

    ctx = nk_sdl_init(window, renderer);

    struct nk_font_atlas *atlas;
    struct nk_font_config config = nk_font_config(0);
    struct nk_font *font;

    nk_sdl_font_stash_begin(&atlas);
    font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
    nk_sdl_font_stash_end();

    font->handle.height /= font_scale;
    nk_style_set_font(ctx, &font->handle);
}

void render_inventory(const player &player)
{
    if (nk_begin(ctx, "Inventory", nk_rect(20, 20, 300, 200),
                 NK_WINDOW_BORDER))
    {
        nk_layout_row_static(ctx, 30, 80, 1);
        nk_label(ctx, "Inventory", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 0, 6);
        for (auto x : player.inv.contents)
        {

            char count_p[64];
            char buff[128];

            sprintf(count_p, "%d", x.count);

            strcpy(buff, TTypeStrings[x.item_id]);
            strcat(buff, ", ");
            strcat(buff, count_p);
            nk_label(ctx, buff, NK_TEXT_CENTERED);
        }
    }
    nk_end(ctx);

    nk_sdl_render(NK_ANTI_ALIASING_ON);
}
