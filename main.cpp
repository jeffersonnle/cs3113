/**
* Author: Jefferson Le
* Assignment: Pong
* Date due: 
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };
enum ScaleDirection { GROWING, SHRINKING };

constexpr int WINDOW_WIDTH = 550 * 2,
WINDOW_HEIGHT = 400 * 2;

constexpr float BG_RED = 0.0f,
BG_BLUE = 0.5f,
BG_GREEN = 0.5f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// delta time variables
constexpr float MILLISECONDS_IN_SECOND = 1000.0;
float g_previous_ticks = 0.0f;

constexpr float ROT_INCREMENT = 1.0f;
//texture global variables
constexpr GLint NUMBER_OF_TEXTURES = 1,
LEVEL_OF_DETAIL = 0,
TEXTURE_BORDER = 0;

// calvin and hobbes and brawlball filepaths
constexpr char RED_SPRITE_FILEPATH[] = "red.png";
constexpr char BLUE_SPRITE_FILEPATH[] = "blue.png";
constexpr char brawlball_SPRITE_FILEPATH[] = "brawlball.png";
constexpr char YOU_WIN_FILEPATH[] = "you_win.png";

// vectors for calvin hobbes and brawlball
constexpr glm::vec3 INIT_RED_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), // red, aka left player, aka hobbes
INIT_BLUE_SCALE = glm::vec3(1.0f, 3.0f, 0.0f), // blue, aka right player, aka calvin
INIT_brawlball_SCALE = glm::vec3(1.0f, 1.0f, 0.0f); // brawlball

constexpr glm::vec3 INIT_POS_RED = glm::vec3(-4.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_BLUE = glm::vec3(4.0f, 0.0f, 0.0f);

bool game_start = false;

bool red_collision_top = false;
bool red_collision_bottom = false;
bool red_collision_top_ai = false;
bool red_collision_bottom_ai = false;

bool blue_collision_top = false;
bool blue_collision_bottom = false;

bool blue_win = false;
bool red_win = false;


bool brawlball_collision_top = false;
bool brawlball_collision_bottom = false;
bool brawlball_collision_right = false;
bool brawlball_collision_left = false;


bool ai_mode = false;

constexpr float MIN_COLLISION_DISTANCE = 1.0f;

// keep track of position using vectors
glm::vec3 g_blue_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_blue_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_red_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_red_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_brawlball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_brawlball_movement = glm::vec3(1.0f, 1.0f, 0.0f);

float brawlball_increment_x = 2.0f;
float brawlball_increment_y = 2.0f;
float increment = 3.0f;
float direction = 0.0f;
float g_blue_speed = 3.0f;
float g_red_speed = 3.0f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

// GLuint initialize textures
glm::mat4 g_view_matrix,
g_blue_matrix,
g_red_matrix,
g_brawlball_matrix,
g_win_matrix,
g_projection_matrix;

GLuint g_blue_texture_id;
GLuint g_red_texture_id;
GLuint g_brawlball_texture_id;
GLuint g_win_texture_id;
GLuint g_background_texture_id;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Project2: Calvin vs Hobbes",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);

    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_red_matrix = glm::mat4(1.0f);
    g_blue_matrix = glm::mat4(1.0f);
    g_brawlball_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_win_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_red_texture_id = load_texture(RED_SPRITE_FILEPATH);
    g_blue_texture_id = load_texture(BLUE_SPRITE_FILEPATH);
    g_brawlball_texture_id = load_texture(brawlball_SPRITE_FILEPATH);
    g_win_texture_id = load_texture(YOU_WIN_FILEPATH);
    g_background_texture_id = load_texture("background.png");



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input() {
    g_red_movement = glm::vec3(0.0f);
    g_blue_movement = glm::vec3(0.0f);
    g_brawlball_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;


        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                g_blue_movement.y = 1.0f;
                break;

            case SDLK_DOWN:
                g_blue_movement.y = -1.0f;
                break;

            case SDLK_q:
                g_app_status = TERMINATED;
                break;

            case SDLK_w:
                g_red_movement.y = 1.0f;
                break;
            case SDLK_s:
                g_red_movement.y = -1.0f;
                break;
            case SDLK_t:
                ai_mode = !(ai_mode);
                break;
            case SDLK_SPACE:
                game_start = true;
                break;

            default:
                break;
            }
        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_UP])
    {
        if (blue_collision_top == false)
        {
            g_blue_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        if (blue_collision_bottom == false)
        {
            g_blue_movement.y = -1.0f;
        }
    }

    if (key_state[SDL_SCANCODE_W])
    {
        if (ai_mode == false)
        {
            if (red_collision_top == false)
            {
                g_red_movement.y = 1.0f;
            }

        }


    }
    else if (key_state[SDL_SCANCODE_S])
    {
        if (ai_mode == false)
        {
            if (red_collision_bottom == false)
            {
                g_red_movement.y = -1.0f;
            }

        }

    }

    // normalize speeds of paddles
    if (glm::length(g_blue_movement) > 1.0f)
    {
        g_blue_movement = glm::normalize(g_blue_movement);
    }
    if (glm::length(g_red_movement) > 1.0f)
    {
        g_red_movement = glm::normalize(g_red_movement);
    }

}


void update()
{

    // delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;



    // player input on red and blue paddles
    g_blue_position += g_blue_movement * g_blue_speed * delta_time;
    g_red_position += g_red_movement * g_red_speed * delta_time;


    g_win_matrix = glm::mat4(1.0f);
    g_red_matrix = glm::mat4(1.0f);
    g_blue_matrix = glm::mat4(1.0f);
    g_brawlball_matrix = glm::mat4(1.0f);


    // victory message

    if (blue_win == false && red_win == false)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(10.0f, 0.0f, 0.0f));
    }
    if (blue_win == true)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(3.0f, 0.0f, 0.0f));
    }
    if (red_win == true)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(-3.0f, 0.0f, 0.0f));
    }

    g_win_matrix = glm::scale(g_win_matrix, glm::vec3(2.0f, 2.0f, 0.0f));


    //red, left player
    g_red_matrix = glm::translate(g_red_matrix, g_red_position);
    if (ai_mode == true)
    {
        g_red_matrix = glm::mat4(1.0f);
        if (red_collision_top_ai == true || red_collision_bottom_ai == true)
        {
            increment = -increment;
        }
        direction += increment * delta_time;
        g_red_matrix = glm::translate(g_red_matrix, glm::vec3(0.0f, direction, 0.0f));

        float y_red_distance_top_ai = (direction + INIT_POS_RED.y + INIT_RED_SCALE.y / 2.0f) - 3.75f;
        float y_red_distance_bottom_ai = (direction + INIT_POS_RED.y - INIT_RED_SCALE.y / 2.0f) + 3.75f;

        if (y_red_distance_top_ai > 0)
        {
            red_collision_top_ai = true;
        }
        else { red_collision_top_ai = false; }
        if (y_red_distance_bottom_ai < 0)
        {
            red_collision_bottom_ai = true;
        }
        else { red_collision_bottom_ai = false; }
    }
    g_red_matrix = glm::translate(g_red_matrix, INIT_POS_RED);
    g_red_matrix = glm::scale(g_red_matrix, INIT_RED_SCALE);


    // blue, right player 
    g_blue_matrix = glm::translate(g_blue_matrix, INIT_POS_BLUE);
    g_blue_matrix = glm::translate(g_blue_matrix, g_blue_position);
    g_blue_matrix = glm::scale(g_blue_matrix, INIT_BLUE_SCALE);



    // collision handling for paddles vs window
    // red (hobbes)
    if (g_red_position.y + INIT_POS_RED.y + (INIT_RED_SCALE.y / 2.0f) > 3.75f) {
        g_red_position.y = 3.75f - INIT_POS_RED.y - (INIT_RED_SCALE.y / 2.0f);
        red_collision_top = true;
    }
    else if (g_red_position.y + INIT_POS_RED.y - (INIT_RED_SCALE.y / 2.0f) < -3.75f) {
        g_red_position.y = -3.75f - INIT_POS_RED.y + (INIT_RED_SCALE.y / 2.0f);
        red_collision_bottom = true;
    }
    else {
        red_collision_top = false;
        red_collision_bottom = false;
    }
    // blue (calvin)
    if (g_blue_position.y + INIT_POS_BLUE.y + (INIT_BLUE_SCALE.y / 2.0f) > 3.75f) {
        g_blue_position.y = 3.75f - INIT_POS_BLUE.y - (INIT_BLUE_SCALE.y / 2.0f);
        blue_collision_top = true;
    }
    else if (g_blue_position.y + INIT_POS_BLUE.y - (INIT_BLUE_SCALE.y / 2.0f) < -3.75f) {
        g_blue_position.y = -3.75f - INIT_POS_BLUE.y + (INIT_BLUE_SCALE.y / 2.0f);
        blue_collision_bottom = true;
    }
    else {
        blue_collision_top = false;
        blue_collision_bottom = false;
    }


    //brawlball collision handling
    g_brawlball_matrix = glm::mat4(1.0f);
    if (brawlball_collision_top == true || brawlball_collision_bottom == true) {
        brawlball_increment_y = -brawlball_increment_y;
    }
    if (brawlball_collision_right == true || brawlball_collision_left == true) {
        brawlball_increment_x = 0;
        brawlball_increment_y = 0;
    }
    if (game_start == true) {
        g_brawlball_position.x += brawlball_increment_x * delta_time;
        g_brawlball_position.y += brawlball_increment_y * delta_time;
    }

    //top bottom ball collision
    float y_brawlball_distance_top = (g_brawlball_position.y + INIT_brawlball_SCALE.y / 2.0f) - 3.75f;
    float y_brawlball_distance_bottom = (g_brawlball_position.y - INIT_brawlball_SCALE.y / 2.0f) + 3.75f;

    //left right ball collision
    //right
    float x_brawlball_distance_right = (g_brawlball_position.x + INIT_brawlball_SCALE.x / 2.0f) - 5.0f;
    float x_brawlball_distance_left = (g_brawlball_position.x - INIT_brawlball_SCALE.x / 2.0f) + 5.0f;

    if (y_brawlball_distance_top > 0) {
        brawlball_collision_top = true;
    } else {
        brawlball_collision_top = false;
    }
    if (y_brawlball_distance_bottom < 0) {
        brawlball_collision_bottom = true;
    } else {
        brawlball_collision_bottom = false;
    }
    //left-right
    if (x_brawlball_distance_right > 0) {
        brawlball_collision_right = true;
        red_win = true;
    } else {
        brawlball_collision_right = false;
    }
    if (x_brawlball_distance_left < 0) {
        brawlball_collision_left = true;
        blue_win = true;
    } else {
        brawlball_collision_left = false;
    }

    g_brawlball_matrix = glm::mat4(1.0f);

    float collision_factor = 0.5f;

    

    // collision handling between paddle(s) and brawlball

    //blue (right)
    float x_brawlball_distance_blue = fabs(g_brawlball_position.x - INIT_POS_BLUE.x) -
        ((INIT_BLUE_SCALE.x * collision_factor + INIT_brawlball_SCALE.x * collision_factor) / 2.0f);
    float y_brawlball_distance_blue = fabs(g_brawlball_position.y - (g_blue_position.y + INIT_POS_BLUE.y)) -
        ((INIT_BLUE_SCALE.y * collision_factor + INIT_brawlball_SCALE.y * collision_factor) / 2.0f);
    //red (left)
    float x_brawlball_distance_red = fabs(g_brawlball_position.x - INIT_POS_RED.x) -
        ((INIT_RED_SCALE.x * collision_factor + INIT_brawlball_SCALE.x * collision_factor) / 2.0f);
    float y_brawlball_distance_red = fabs(g_brawlball_position.y - (g_red_position.y + INIT_POS_RED.y)) -
        ((INIT_RED_SCALE.y * collision_factor + INIT_brawlball_SCALE.y * collision_factor) / 2.0f);



    if (x_brawlball_distance_blue <= 0.0f && y_brawlball_distance_blue <= 0.0f) {
        brawlball_increment_x = -brawlball_increment_x;
    }
    if (x_brawlball_distance_red <= 0.0f && y_brawlball_distance_red <= 0.0f) {
        brawlball_increment_x = -brawlball_increment_x;
    }
    if (game_start == true) {
        g_brawlball_matrix = glm::translate(g_brawlball_matrix, g_brawlball_position);
    }

}

void draw_object(glm::mat4& object_g_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Drawing the background

// Create a model matrix for the background, covering the whole screen
    glm::mat4 background_matrix = glm::mat4(1.0f);
    background_matrix = glm::scale(background_matrix, glm::vec3(10.0f, 7.5f, 1.0f)); // Adjust scale to cover the screen

    g_shader_program.set_model_matrix(background_matrix);
    glBindTexture(GL_TEXTURE_2D, g_background_texture_id);

    // for bg
    float bg_vertices[] = {
        -0.5f, -0.5f, // bottom left
        0.5f, -0.5f,  // bottom right
        0.5f, 0.5f,   // top right
        -0.5f, -0.5f, // bottom left
        0.5f, 0.5f,   // top right
        -0.5f, 0.5f   // top left
    };

    float bg_texture_coords[] = {
        0.0f, 1.0f, // bottom left
        1.0f, 1.0f, // bottom right
        1.0f, 0.0f, // top right
        0.0f, 1.0f, // bottom left
        1.0f, 0.0f, // top right
        0.0f, 0.0f  // top left
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, bg_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, bg_texture_coords);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());


    //vertices
    float vertices[] =
    {
         -0.5f, -0.5f, 
         0.5f, -0.5f,  
         0.5f, 0.5f,
         -0.5f, -0.5f,
         0.5f, 0.5f,
         -0.5f, 0.5f
    };

    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 
        1.0f, 1.0f, 
        1.0f, 0.0f, 
        0.0f, 1.0f, 
        1.0f, 0.0f, 
        0.0f, 0.0f,  
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    //bind texture
    draw_object(g_red_matrix, g_red_texture_id);
    draw_object(g_blue_matrix, g_blue_texture_id);
    draw_object(g_brawlball_matrix, g_brawlball_texture_id);
    draw_object(g_win_matrix, g_win_texture_id);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* args[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}