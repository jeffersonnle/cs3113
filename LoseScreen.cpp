#include "LoseScreen.h"
#include "MenuScreen.h"
#include "Utility.h"

#define LEVEL_WIDTH 2
#define LEVEL_HEIGHT 2

constexpr char SPRITESHEET_FILEPATH[] = "assets/redfrompokemon.png",
PLATFORM_FILEPATH[] = "assets/tileset.png",
ENEMY_FILEPATH[] = "assets/mortis.png",
FONT_SPRITE_FILEPATH[] = "assets/font1.png";

GLuint g_font_texture_id3;

unsigned int LEVEL_DATALOSE[] =
{
    0, 0,
    0, 0
};

LoseScreen::~LoseScreen()
{
    delete m_game_state.player;
    delete m_game_state.map;
}

void LoseScreen::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATALOSE, map_texture_id, 1.0f, 4, 1);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    g_font_texture_id3 = Utility::load_texture(FONT_SPRITE_FILEPATH);

    int player_walking_animation[4][4] =
    {
    { 1, 5, 9, 13 },  // for George to move to the left,
    { 3, 7, 11, 15 }, // for George to move to the right,
    { 2, 6, 10, 14 }, // for George to move upwards,
    { 0, 4, 8, 12 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.905f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        0.9f,                      // width
        0.9f,                      // height
        PLAYER
    );
    m_game_state.player->set_position(glm::vec3(5.0f, -3.0f, 0.0f));
}

void LoseScreen::update(float delta_time)
{
    // Update logic if needed, e.g., player actions for exiting or restarting.
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
}

void LoseScreen::render(ShaderProgram* g_shader_program)
{
    Utility::draw_text(g_shader_program, g_font_texture_id3, "NOOOOOOOO!!", 0.4, 0.00001f, glm::vec3(1.0f, -2.0f, 0.0f));
    Utility::draw_text(g_shader_program, g_font_texture_id3, "YOU WERE TAGGED!", 0.35, 0.03f, glm::vec3(2.0f, -3.0f, 0.0f));
    Utility::draw_text(g_shader_program, g_font_texture_id3, "YOU FAILED BRAWLTAG...", 0.3, 0.03f, glm::vec3(2.0f, -5.0f, 0.0f));
    // Uncomment if rendering map and player is necessary:
    // m_game_state.map->render(g_shader_program);
    // m_game_state.player->render(g_shader_program);
}
