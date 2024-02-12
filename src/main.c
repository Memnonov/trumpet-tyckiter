/* Copyright [2024] Mikko Memonen */

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

// Main function!
int main(void) {
  // MY CONSTANTS
  const Uint32 INIT_FLAGS = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS;
  const Uint32 RENDERER_FLAGS =
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  const int WINDOW_WIDTH = 640;
  const int WINDOW_HEIGHT = 480;
  // TODO(mikko): hoija tää käyttään suhteellista polokua!
  // täähän pyrii nyt vaan, jos pwd on juuri...
  const char *VALVES_PATH = "resources/trumpet_valves.png";
  const char *KEY_PATH = "resources/trumpet_key.png";

  // Initialize SDL.
  if (SDL_Init(INIT_FLAGS)) {
    printf("\nCouldn't initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  // Create the game window.
  SDL_Window *g_window = NULL;
  g_window =
      SDL_CreateWindow("trumpet_tyckiter", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (g_window == NULL) {
    printf("\nCouldn't create window: %s", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create a renderer.
  SDL_Renderer *g_renderer = NULL;
  g_renderer = SDL_CreateRenderer(g_window, -1, RENDERER_FLAGS);
  if (g_renderer == NULL) {
    printf("\nCouldn't create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create the trumpet valves and keys textures.
  SDL_Texture *trumpet_valves_texture = NULL;
  SDL_Texture *trumpet_key_texture = NULL;
  trumpet_valves_texture = IMG_LoadTexture(g_renderer, VALVES_PATH);
  trumpet_key_texture = IMG_LoadTexture(g_renderer, KEY_PATH);
  if (trumpet_valves_texture == NULL) {
    printf("Couldn't load trumpet valves!");
  }
  if (trumpet_key_texture == NULL) {
    printf("Couldn't load trumpet keys!");
  }
  const unsigned int valve_press_length = 40;
  const unsigned int key_gap = 95;

  // Create a rectangle for a key
  // AND! Query the dimensions from the texture!
  // This is repetition and DUMB. Fix it later...
  SDL_Rect key_1_rect;
  SDL_Rect key_2_rect;
  SDL_Rect key_3_rect;
  SDL_QueryTexture(trumpet_key_texture, NULL, NULL,
                   &key_1_rect.w, &key_1_rect.h);
  SDL_QueryTexture(trumpet_key_texture, NULL, NULL,
                   &key_2_rect.w, &key_2_rect.h);
  SDL_QueryTexture(trumpet_key_texture, NULL, NULL,
                   &key_3_rect.w, &key_3_rect.h);
  key_1_rect.x = 170;
  key_1_rect.y = 150;
  SDL_QueryTexture(trumpet_key_texture, NULL, NULL,
                   &key_1_rect.w, &key_1_rect.h);
  key_2_rect.x = key_1_rect.x + key_gap;
  key_2_rect.y = 150;
  SDL_QueryTexture(trumpet_key_texture, NULL, NULL,
                   &key_1_rect.w, &key_1_rect.h);
  key_3_rect.x = key_2_rect.x + key_gap;
  key_3_rect.y = 150;

  unsigned int stop_request = 0;
  unsigned int key_1_pressed = 0;
  unsigned int key_2_pressed = 0;
  unsigned int key_3_pressed = 0;
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
          if (!key_1_pressed) {
            key_1_rect.y += valve_press_length;
            key_1_pressed = 1;
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (!key_2_pressed) {
            key_2_rect.y += valve_press_length;
            key_2_pressed = 1;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (!key_3_pressed) {
            key_3_rect.y += valve_press_length;
            key_3_pressed = 1;
          }
          break;
        }
        break;

      // KEYUP switch
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_LEFT:
          if (key_1_pressed) {
          key_1_rect.y -= valve_press_length;
          key_1_pressed = 0;
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (key_2_pressed) {
            key_2_rect.y -= valve_press_length;
            key_2_pressed = 0;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (key_3_pressed) {
            key_3_rect.y -= valve_press_length;
            key_3_pressed = 0;
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
    if (SDL_RenderCopy(g_renderer, trumpet_key_texture,
                       NULL,  // use the whole image as source
                       &key_1_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(g_renderer, trumpet_key_texture,
                       NULL,  // use the whole image as source
                       &key_2_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(g_renderer, trumpet_key_texture,
                       NULL,  // use the whole image as source
                       &key_3_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    // Valves
    if (SDL_RenderCopy(g_renderer, trumpet_valves_texture,
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
  SDL_DestroyTexture(trumpet_valves_texture);
  SDL_DestroyTexture(trumpet_key_texture);
  SDL_Quit();
  printf("Program exited succesfully!");

  return EXIT_SUCCESS;
}
