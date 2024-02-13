/* Copyright [2024] Mikko Memonen */

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

// Struct for the trumpet: graphics and keys pressed etc.
// Also the SDL_Rects for the key rectangles.
typedef struct {
  SDL_Texture *valves_text;
  SDL_Texture *keys_text;
  unsigned int key_1_pressed;
  unsigned int key_2_pressed;
  unsigned int key_3_pressed;
  SDL_Rect key_1_rect;
  SDL_Rect key_2_rect;
  SDL_Rect key_3_rect;
} GAME_Trumpet;

void GAME_QuitAll(void);  // automagic cleanup
void GAME_PlaceTrumpetKeys(GAME_Trumpet *trumpet);  // Init the valve rectangles

// Main function!
int main(void) {
  // MY CONSTANTS
  const Uint32 INIT_FLAGS =
      SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
  const Uint32 RENDERER_FLAGS =
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  const int WINDOW_WIDTH = 640;
  const int WINDOW_HEIGHT = 480;
  // TODO(mikko): hoija tää käyttään suhteellista polokua!
  // täähän pyrii nyt vaan, jos pwd on juuri...
  const char *VALVES_PATH = "resources/trumpet_valves.png";
  const char *KEY_PATH = "resources/trumpet_key.png";
  const char *SOUND_FANFARE = "resources/trumpet_fanfare.wav";

  // Initialize THE HORN!
  GAME_Trumpet trumpet = {.valves_text = NULL,
                          .keys_text = NULL,
                          .key_1_pressed = 0,
                          .key_2_pressed = 0,
                          .key_3_pressed = 0};

  // Initialize SDL.
  if (SDL_Init(INIT_FLAGS)) {
    printf("\nCouldn't initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  /* SDL_mixer will handle initialization on the fly!
   * You can use Mix_Init() with flags to ensure that a
   * type of audio can be played.                         */

  // Open the default audio device. Initializes the mixer too(?)
  if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 2048) == -1) {
    printf("\nCouldn't open audio device: %s", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create the game window.
  SDL_Window *g_window = NULL;
  g_window =
      SDL_CreateWindow("trumpet_tyckiter", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (g_window == NULL) {
    printf("\nCouldn't create window: %s", SDL_GetError());
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create a renderer.
  SDL_Renderer *g_renderer = NULL;
  g_renderer = SDL_CreateRenderer(g_window, -1, RENDERER_FLAGS);
  if (g_renderer == NULL) {
    printf("\nCouldn't create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(g_window);
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create the trumpet valves and keys textures.
  // SDL_Texture *trumpet_valves_texture = NULL;  // Now in the GAME_Trumpet
  // struct SDL_Texture *trumpet_key_texture = NULL;     // These too
  trumpet.valves_text = IMG_LoadTexture(g_renderer, VALVES_PATH);
  trumpet.keys_text = IMG_LoadTexture(g_renderer, KEY_PATH);
  if (trumpet.valves_text == NULL) {
    printf("Couldn't load trumpet valves!");
    SDL_DestroyWindow(g_window);
    SDL_DestroyRenderer(g_renderer);
    SDL_CloseAudio();
    SDL_Quit();
  }
  if (trumpet.keys_text == NULL) {
    printf("Couldn't load trumpet keys!");
    SDL_DestroyWindow(g_window);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyTexture(trumpet.valves_text);
    SDL_CloseAudio();
    SDL_Quit();
  }

  GAME_PlaceTrumpetKeys(&trumpet);

  // Load a sound effect.
  Mix_Chunk *sound_fanfare = Mix_LoadWAV(SOUND_FANFARE);
  if (sound_fanfare == NULL) {
    printf("Couldn't load fanfare : (  (%s))", SDL_GetError());
    SDL_DestroyWindow(g_window);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyTexture(trumpet.keys_text);
    SDL_DestroyTexture(trumpet.valves_text);
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  unsigned int stop_request = 0;
  const unsigned int valve_press_length = 40;
  // MAIN LOOP TIME! ------------------------------------
  while (!stop_request) {
    // Process events.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_QUIT:
        stop_request = 1;
        break;

      // KEYDOWN switch
      case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_LEFT:
          if (!trumpet.key_1_pressed) {
            trumpet.key_1_rect.y += valve_press_length;
            trumpet.key_1_pressed = 1;
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (!trumpet.key_2_pressed) {
            trumpet.key_2_rect.y += valve_press_length;
            trumpet.key_2_pressed = 1;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (!trumpet.key_3_pressed) {
            trumpet.key_3_rect.y += valve_press_length;
            trumpet.key_3_pressed = 1;
          }
          break;
        case SDL_SCANCODE_A:
          Mix_PlayChannel(1, sound_fanfare, 0);
          break;
        }
        break;

      // KEYUP switch
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_LEFT:
          if (trumpet.key_1_pressed) {
            trumpet.key_1_rect.y -= valve_press_length;
            trumpet.key_1_pressed = 0;
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (trumpet.key_2_pressed) {
            trumpet.key_2_rect.y -= valve_press_length;
            trumpet.key_2_pressed = 0;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (trumpet.key_3_pressed) {
            trumpet.key_3_rect.y -= valve_press_length;
            trumpet.key_3_pressed = 0;
          }
          break;
        }
        break;
      }
    }
    // End of event loop.

    // Draw black backgound.
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);

    // Add textures to buffer.
    // Keys
    if (SDL_RenderCopy(g_renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_1_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(g_renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_2_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(g_renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_3_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    // Valves
    if (SDL_RenderCopy(g_renderer, trumpet.valves_text,
                       NULL,  // use the whole image as source
                       NULL)  // stretch to fit the whole window
    ) {
      printf("Error copying valves: %s", SDL_GetError());
    }

    // Show buffer.
    SDL_RenderPresent(g_renderer);

    // Emulate 60fps
    SDL_Delay(1000 / 60);
  }

  // // Wait to see the results.
  // SDL_Delay(3000);

  // Close all
  SDL_DestroyWindow(g_window);
  SDL_DestroyRenderer(g_renderer);
  SDL_DestroyTexture(trumpet.valves_text);
  SDL_DestroyTexture(trumpet.keys_text);
  Mix_FreeChunk(sound_fanfare);
  SDL_CloseAudio();
  SDL_Quit();
  printf("Program exited succesfully!");

  return EXIT_SUCCESS;
}

/* void GAME_QuitAll(void) (might need arguments eventually!)
 *
 * Conveniently closes all initiated SDL2 modules. */
void GAME_QuitAll(void) {
  // SDL_init       returns 0 if OK
  // Mix_OpenAudio  returns 0 if OK
  //

  SDL_Quit();
}

/* void GAME_PlaceTrumpetKeys(GAME_Trumpet *trumpet)
 *
 * Initializes the rectangles for the key locations. */

void GAME_PlaceTrumpetKeys(GAME_Trumpet *trumpet) {
  const unsigned int key_gap = 95;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_2_rect.w,
                   &trumpet->key_2_rect.h);
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_3_rect.w,
                   &trumpet->key_3_rect.h);
  // Spread the keys.
  trumpet->key_1_rect.x = 170;
  trumpet->key_1_rect.y = 150;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  trumpet->key_2_rect.x = trumpet->key_1_rect.x + key_gap;
  trumpet->key_2_rect.y = 150;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  trumpet->key_3_rect.x = trumpet->key_2_rect.x + key_gap;
  trumpet->key_3_rect.y = 150;
}
