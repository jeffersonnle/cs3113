#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8


constexpr char SPRITESHEET_FILEPATH[] = "assets/redfrompokemon.png",
PLATFORM_FILEPATH[] = "assets/tileset.png",
ENEMY_FILEPATH[] = "assets/fang.png",
VICTORY_FILEPATH[] = "assets/victory.png";

unsigned int LEVEL_DATAC[] =
{
2, 2, 2, 2, 0, 0, 2, 2, 2, 0, 0, 2, 2, 2,
2, 2, 2, 2, 0, 0, 2, 2, 2, 0, 0, 2, 2, 2,
2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2


};

LevelC::~LevelC()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATAC, map_texture_id, 1.0f, 4, 1);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    GLuint victory_texture_id = Utility::load_texture(VICTORY_FILEPATH);


    int player_walking_animation[4][4] =
    {
    { 1, 5, 9, 13 },  // for George to move to the left,
    { 3, 7, 11, 15 }, // for George to move to the right,
    { 2, 6, 10, 14 }, // for George to move upwards,
    { 0, 4, 8, 12 }   // for George to move downwards
    };


    /*
    int player_walking_animation[1][1] =
    {
    { 0 }   // for George to move downwards
    };
    */




    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        0.9f,                      // width
        0.9f,                       // height
        PLAYER
    );

    m_game_state.player->set_position(glm::vec3(7.0f, -6.0f, 0.0f));

    // Jumping
    // m_game_state.player->set_jumping_power(3.0f);

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        //m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, PACER, IDLE);
    }
    
   


    m_game_state.enemies[0].set_position(glm::vec3(7.0f, -3.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    m_game_state.enemies[1].set_position(glm::vec3(4.0f, -4.2f, 0.0f));
    m_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    m_game_state.enemies[2].set_position(glm::vec3(9.0f, -1.2f, 0.0f));
    //m_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/crypto.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);

}

void LevelC::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }

    float player_y = m_game_state.player->get_position().y;
    float top_boundary = 0.0f;  // Top of the map
    float tolerance = 0.01f;    // Small tolerance for floating-point precision

    if (player_y >= top_boundary - tolerance)
    {
        survive_level = true;
        std::cout << "Player has reached the top of the map!" << std::endl;
        
    }


}


void LevelC::render(ShaderProgram* g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++)
        m_game_state.enemies[i].render(g_shader_program);

    /*
    if (survive_level == true) {
        GLuint victory_texture_id = Utility::load_texture(VICTORY_FILEPATH);
        glm::mat4 victory_model_matrix = glm::mat4(1.0f);

        victory_model_matrix = glm::translate(victory_model_matrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Adjust as necessary
        victory_model_matrix = glm::scale(victory_model_matrix, glm::vec3(3.0f, 3.0f, 1.0f)); // Scale it to fit

        g_shader_program->set_model_matrix(victory_model_matrix);

        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, victory_texture_id);

        glVertexAttribPointer(g_shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(g_shader_program->get_position_attribute());

        glVertexAttribPointer(g_shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(g_shader_program->get_position_attribute());
        glDisableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());

    }
    */
}
