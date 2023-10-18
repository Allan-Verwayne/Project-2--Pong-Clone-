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

const char PLAYER_ONE_SPRITE_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/Project_2/sprites/tinkaton2.png";
const char PLAYER_TWO_SPRITE_FILEPATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_2/Project_2/sprites/tinkaton3.png";

// Game Variables
SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram shader_program;
GLuint player_one_texture_id;
GLuint player_two_texture_id;
glm::mat4 model_matrix;
glm::mat4 model_two_matrix;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

//bool single_player;

//Delta Time Variables
float previous_ticks = 0.0f;
const float MILLISECONDS_IN_SECOND = 1000.0;

// Sprite Variables
glm::vec3 player_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_movement = glm::vec3(-4.0f, 0.0f, 0.0f);

glm::vec3 player_two_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_two_movement = glm::vec3(4.0f, 0.0f, 0.0f);
                                                               
//glm::vec3 player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 player_rotation    = glm::vec3(0.0f, 0.0f, 0.0f);
                                                               
const float player_speed = 3.0f;  // move 1 unit per second
const float player_two_speed = 3.0f;  // move 1 unit per second

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
    
    player_one_texture_id = load_texture(PLAYER_ONE_SPRITE_FILEPATH);
    player_two_texture_id = load_texture(PLAYER_TWO_SPRITE_FILEPATH);
    model_matrix      = glm::mat4(1.0f);
    model_two_matrix  = glm::mat4(1.0f);
    view_matrix       = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    // starting positions
    model_matrix     = glm::translate(model_matrix, player_movement);
    model_two_matrix = glm::translate(model_two_matrix, player_two_movement);
    player_position     += player_movement;
    player_two_position += player_two_movement;
    
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
    player_movement     = glm::vec3(0.0f);
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
                        player_movement.y = 1.0f;
                        break;
                    // move down
                    case SDLK_s:
                        player_movement.y = -1.0f;
                        break;
                        
                //          player two           //
                    // move up
                    case SDLK_UP:
                        player_two_movement.y = 1.0f;
                        break;
                    // move down
                    case SDLK_DOWN:
                        player_two_movement.y = -1.0f;
                        break;
                        
                    // change game between one and two players
                    case SDLK_t:
//                        single_player = !single_player;
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
        player_movement.y = 1.0f;
    }
    // holding down
    else if (key_state[SDL_SCANCODE_S]) {
        player_movement.y = -1.0f;
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
    
    // P2 speed restrictor / normalizer
    if (glm::length(player_two_movement) > 1.0f) {
        player_two_movement = glm::normalize(player_two_movement);
    }
    // P1 speed restrictor / normalizer
    if (glm::length(player_movement) > 1.0f) {
        player_movement = glm::normalize(player_movement);
    }
}

void update() {
    // delta time
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    // player movement
    player_position     += player_movement * player_speed * delta_time;
    player_two_position += player_two_movement * player_speed * delta_time;
    model_matrix     = glm::mat4(1.0f);
    model_two_matrix = glm::mat4(1.0f);
    model_matrix     = glm::translate(model_matrix, player_position);
    model_two_matrix = glm::translate(model_two_matrix, player_two_position);
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
    draw_object(model_matrix, player_one_texture_id);
    draw_object(model_two_matrix, player_two_texture_id);
    
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
