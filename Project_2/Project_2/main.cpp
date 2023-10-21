/*
* Author: Allan Verwayne
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <ctime>
#include "cmath"

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

// Window Globals
const int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Shader Globals
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

const char BACKGROUND_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/assets/heaven.jpg";
const char PLAYER_ONE_SPRITE_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/assets/chibi_may.png";
const char PLAYER_TWO_SPRITE_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/assets/chibi_bkn.png";
const char BALL_SPRITE_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/assets/stun_edge.png";

// Game Variables
SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram shader_program;

GLuint background_texture_id;
GLuint player_one_texture_id;
GLuint player_two_texture_id;
GLuint ball_texture_id;

glm::mat4 background_matrix;
glm::mat4 model_one_matrix;
glm::mat4 model_two_matrix;
glm::mat4 model_three_matrix;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

bool single_player = false;
bool play = false;

// Delta Time Variables
float previous_ticks = 0.0f;
const float MILLISECONDS_IN_SECOND = 1000.0;

// Collision Global
const float HEIGHT = 0.6f;
const float WIDTH = 0.3f;
const float collision_factor = 1.0f;

// Sprite Variables
glm::vec3 player_one_position = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 player_one_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_two_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 player_two_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.75f, 1.0f, 0.0f);

const float LR_wall = 5.99f;
const float TB_wall = 3.5f;
                                                               
// Movement Globals
const float player_speed = 3.0f;  // move n units per second
const float COM_speed = 6.5f;
float ball_speed = 2.0f;
float DIRECTION = 1.0f;

// load_texture Function
GLuint load_texture(const char* filepath) {
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct." << std::endl;
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}
// draw_object Function
void draw_object(glm::mat4 &obj_model_matrix, GLuint &obj_texture_id) {
    shader_program.set_model_matrix(obj_model_matrix);
    glBindTexture(GL_TEXTURE_2D, obj_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// check_collision Function
bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b) {
    float x_distance = fabs(position_a.x - position_b.x) - ((collision_factor * (WIDTH + WIDTH)) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((collision_factor * (HEIGHT + HEIGHT)) / 2.0f);
    std::cout << std::time(nullptr) << ": hit.\n";
    return (x_distance < 0.0f && y_distance < 0.0f);
}

// initialise Function
void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Pong Clone: --- Edition",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    background_texture_id = load_texture(BACKGROUND_FILEPATH);
    player_one_texture_id = load_texture(PLAYER_ONE_SPRITE_FILEPATH);
    player_two_texture_id = load_texture(PLAYER_TWO_SPRITE_FILEPATH);
    ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    background_matrix   = glm::mat4(1.0f);
    model_one_matrix    = glm::mat4(1.0f);
    model_two_matrix    = glm::mat4(1.0f);
    model_three_matrix  = glm::mat4(1.0f);
    view_matrix         = glm::mat4(1.0f);
    projection_matrix   = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    // starting positions
    model_one_matrix  = glm::translate(model_one_matrix, glm::vec3(-4.0f, 0.0f, 0.0f));
    model_two_matrix  = glm::translate(model_two_matrix, glm::vec3(4.0f, 0.0f, 0.0f));
    background_matrix = glm::scale(background_matrix, glm::vec3(10.0f, 8.0f, 0.0f));
    
    shader_program.set_projection_matrix(projection_matrix);
    shader_program.set_view_matrix(view_matrix);
    
    glUseProgram(shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// process_input Function
void process_input() {
    // no movement
    player_one_movement = glm::vec3(0.0f);
    player_two_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            // end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                        
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                //          player one           //
                    // move up
                    case SDLK_w:
                        player_one_movement.y = 1.0f;
                        break;
                    // move down
                    case SDLK_s:
                        player_one_movement.y = -1.0f;
                        break;
                        
                //          player two           //
                    // move up
                    case SDLK_UP:
                        if (!single_player) {
                            player_two_movement.y = 1.0f;
                        }
                        break;
                    // move down
                    case SDLK_DOWN:
                        if (!single_player) {
                            player_two_movement.y = -1.0f;
                        }
                        break;
                        
                    // change game between one and two players
                    case SDLK_t:
                        single_player = !single_player;
                        break;
                        
                    case SDLK_p:
                        play = !play;
                        break;
                        
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    //          player one          //
    // holding up
    if (key_state[SDL_SCANCODE_W]) {
        player_one_movement.y = 1.0f;
    }
    // holding down
    else if (key_state[SDL_SCANCODE_S]) {
        player_one_movement.y = -1.0f;
    }
    
    //          player two          //
    // holding up
    if (key_state[SDL_SCANCODE_UP]) {
        player_two_movement.y = 1.0f;
    }
    // holding down
    else if (key_state[SDL_SCANCODE_DOWN]) {
        player_two_movement.y = -1.0f;
    }
    
    // P1 speed restrictor / normalizer
    if (glm::length(player_one_movement) > 1.0f) {
        player_one_movement = glm::normalize(player_one_movement);
    }
    
    // P2 speed restrictor / normalizer
    if (glm::length(player_two_movement) > 1.0f) {
        player_two_movement = glm::normalize(player_two_movement);
    }
    
}

void update() {
    // ball and paddle / ball and wall / paddle and wall collisions
    if (check_collision(player_one_position, ball_position) || check_collision(player_two_position, ball_position)) {
        ball_movement.x *= -1.0;
        ball_speed += 0.10f;
    }
    else if (ball_position.y > TB_wall) {
        ball_position.y -= 0.05f;
        ball_movement.y *= -1;
    }
    else if (ball_position.y < -TB_wall) {
        ball_position.y += 0.05f;
        ball_movement.y *= -1;
    }
    else if (ball_position.x > LR_wall ||
             ball_position.x < -LR_wall) {
        game_is_running = false;
        
    }
    
    // delta time
    float ticks      = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks   = ticks;
    
    // player one movement
    model_one_matrix    = glm::mat4(1.0f);
    player_one_position += player_one_movement * player_speed * delta_time;
    model_one_matrix    = glm::translate(model_one_matrix, player_one_position);
    
    if (player_one_position.y > TB_wall) {player_one_position.y = TB_wall;}
    else if (player_one_position.y < -TB_wall) {player_one_position.y = -TB_wall;}
    
    // player two movement
    
    //switch from single to two player mode
    if (!single_player) {
        model_two_matrix    = glm::mat4(1.0f);
        player_two_position += player_two_movement * player_speed * delta_time;
        model_two_matrix    = glm::translate(model_two_matrix, player_two_position);
        
        if (player_two_position.y > TB_wall) {player_two_position.y = TB_wall;}
        else if (player_two_position.y < -TB_wall) {player_two_position.y = -TB_wall;}
    }
    else {
        model_two_matrix      = glm::mat4(1.0f);
        player_two_position.y += DIRECTION * COM_speed * delta_time;
        model_two_matrix      = glm::translate(model_two_matrix, glm::vec3(4.0f, player_two_position.y, 0.0f));
        
        if (player_two_position.y > TB_wall ||
            player_two_position.y < -TB_wall) {
            DIRECTION *= -1;
            player_two_position.y = -DIRECTION * TB_wall;
        }
    }
    
    // scaling for players
    model_one_matrix = glm::scale(model_one_matrix, glm::vec3(1.5f, 1.5f, 0.0f));
    model_two_matrix = glm::scale(model_two_matrix, glm::vec3(1.5f, 1.5f, 0.0f));
    
    // ball movement
    if (play) {
        model_three_matrix = glm::mat4(1.0f);
        ball_position      += ball_movement * ball_speed * delta_time;
        model_three_matrix = glm::translate(model_three_matrix, ball_position);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(shader_program.get_position_attribute());
    
    glVertexAttribPointer(shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(shader_program.get_tex_coordinate_attribute());
    
    // bind textures
    draw_object(background_matrix, background_texture_id);
    draw_object(model_one_matrix, player_one_texture_id);
    draw_object(model_two_matrix, player_two_texture_id);
    draw_object(model_three_matrix, ball_texture_id);
    
    // disable two attribute arrays
    glDisableVertexAttribArray(shader_program.get_position_attribute());
    glDisableVertexAttribArray(shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

// main Function
int main(int argc, char* argv[]) {
    initialise();
    
    while (game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
