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
#include <string.h>

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
} Game_Trumpet;

typedef struct {
  Mix_Chunk *notes[19];
} Game_Audio;

// Struct for keeping track of initialized modules, windows etc.
typedef struct {
  int SDL_Init;
  int MIX_Init;
  SDL_Window *window;
  SDL_Renderer *renderer;
} Game;

// Automagic cleanup.
void Game_QuitAll(Game *game, Game_Trumpet *trumpet, Game_Audio *audio);

// Put the keys in their place.
void Game_PlaceTrumpetKeys(Game_Trumpet *trumpet);

// Load notes into the horn.
int Game_LoadNotes(Game_Audio *audio, const char *path);

// Main function!
int main(void) {
  // MY CONSTANTS
  const Uint32 INIT_FLAGS =
      SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
  const Uint32 RENDERER_FLAGS =
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  const int WINDOW_WIDTH = 640;
  const int WINDOW_HEIGHT = 480;
  // CWD has to be root, this should be improved
  const char *VALVES_PATH = "resources/trumpet_valves.png";
  const char *KEY_PATH = "resources/trumpet_key.png";
  const char *RESOURCES_SOUND_PATH = "resources/sound/";
  const char *TEST_SOUND_F4 = "resources/sound/6.wav";  // for testing

  // Initialize Game.
  Game game = {
      .SDL_Init = -1, .MIX_Init = -1, .window = NULL, .renderer = NULL};

  // Initialize THE HORN!
  Game_Trumpet trumpet = {.valves_text = NULL,
                          .keys_text = NULL,
                          .key_1_pressed = 0,
                          .key_2_pressed = 0,
                          .key_3_pressed = 0};


  // Initialize SDL.
  game.SDL_Init = SDL_Init(INIT_FLAGS);
  // 0 if OK
  if (game.SDL_Init) {
    printf("\nCouldn't initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  /* SDL_mixer will handle initialization on the fly!
   * You can use Mix_Init() with flags to ensure that a
   * type of audio can be played.                         */

  // Open the default audio device. Initializes the mixer too(?)
  game.MIX_Init = Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 2048);
  // 0 if OK
  if (game.MIX_Init) {
    printf("\nCouldn't open audio device: %s", SDL_GetError());
    // TODO: Have these checks use the Game_QuitAll() function!
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create the game window.
  game.window =
      SDL_CreateWindow("trumpet_tyckiter", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (game.window == NULL) {
    printf("\nCouldn't create window: %s", SDL_GetError());
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create a renderer.
  game.renderer = SDL_CreateRenderer(game.window, -1, RENDERER_FLAGS);
  if (game.renderer == NULL) {
    printf("\nCouldn't create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(game.window);
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create the trumpet valves and keys textures.
  // SDL_Texture *trumpet_valves_texture = NULL;  // Now in the Game_Trumpet
  // struct SDL_Texture *trumpet_key_texture = NULL;     // These too
  trumpet.valves_text = IMG_LoadTexture(game.renderer, VALVES_PATH);
  trumpet.keys_text = IMG_LoadTexture(game.renderer, KEY_PATH);
  if (trumpet.valves_text == NULL) {
    printf("Couldn't load trumpet valves!");
    SDL_DestroyWindow(game.window);
    SDL_DestroyRenderer(game.renderer);
    SDL_CloseAudio();
    SDL_Quit();
  }
  if (trumpet.keys_text == NULL) {
    printf("Couldn't load trumpet keys!");
    SDL_DestroyWindow(game.window);
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyTexture(trumpet.valves_text);
    SDL_CloseAudio();
    SDL_Quit();
  }

  Game_PlaceTrumpetKeys(&trumpet);

  // Load a sound effect.
  // Initialize the notes.
  Game_Audio audio = {
    .notes[0] = Mix_LoadWAV(TEST_SOUND_F4)
  };
  if (audio.notes[0] == NULL) {
    printf("Couldn't load DOOTs : (  (%s))", SDL_GetError());
    SDL_DestroyWindow(game.window);
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyTexture(trumpet.keys_text);
    SDL_DestroyTexture(trumpet.valves_text);
    SDL_CloseAudio();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  Game_LoadNotes(&audio, RESOURCES_SOUND_PATH);

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
          Mix_PlayChannel(1, audio.notes[0], 0);
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
    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game.renderer);

    // Add textures to buffer.
    // Keys
    if (SDL_RenderCopy(game.renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_1_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(game.renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_2_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(game.renderer, trumpet.keys_text,
                       NULL,                 // use the whole image as source
                       &trumpet.key_3_rect)  // stretch to fit the whole window
    ) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    // Valves
    if (SDL_RenderCopy(game.renderer, trumpet.valves_text,
                       NULL,  // use the whole image as source
                       NULL)  // stretch to fit the whole window
    ) {
      printf("Error copying valves: %s", SDL_GetError());
    }

    // Show buffer.
    SDL_RenderPresent(game.renderer);

    // Emulate 60fps
    SDL_Delay(1000 / 60);
  }

  Game_QuitAll(&game, &trumpet, &audio);
  printf("\nProgram exited succesfully!");

  return EXIT_SUCCESS;
}

/* void Game_QuitAll(void) (might need arguments eventually!)
 *
 * Conveniently closes all initiated SDL2 modules. */
void Game_QuitAll(Game *game, Game_Trumpet *trumpet, Game_Audio *audio) {
  // SDL_init       returns 0 if OK (SDL_Quit() deals with this?)
  // Mix_OpenAudio  returns 0 if OK
  printf("\nClosing audio...");
  if (game->MIX_Init == 0) {
    printf("\n  Audio closed!");
    SDL_CloseAudio();
  }

  // Destroy window and renderer
  printf("\nDestroying Window...");
  if (game->window != NULL) {
    SDL_DestroyWindow(game->window);
    printf("\n  Window destroyed!");
  }
  printf("\nDestroying Renderer...");
  if (game->renderer != NULL) {
    SDL_DestroyRenderer(game->renderer);
    printf("\n  Renderer destroyed!");
  }

  // Game_Trumpet has pointers to valves_text and keys_text
  printf("\nDestroying keys...");
  if (trumpet->keys_text != NULL) {
    SDL_DestroyTexture(trumpet->keys_text);
    printf("\n  Keys destroyed!");
  }
  printf("\nDestroying valves...");
  if (trumpet->valves_text != NULL) {
    SDL_DestroyTexture(trumpet->valves_text);
    printf("\n  Valves destroyed!");
  }

  // TODO: Fix magic number 19!
  printf("\nFreeing Mix Chunks..");
  for (int i = 0; i < 19; i++) {
    if (audio->notes[i] != NULL) {
      Mix_FreeChunk(audio->notes[i]);
      printf("\n  Freed chunk %d", i);
    }
  }

  if (game->MIX_Init == 0) {
    Mix_Quit();
    printf("\nMix Quit OK!");
  }
  if (game->SDL_Init == 0) {
    SDL_Quit();
    printf("\nSDL Quit OK!");
  }
}

/* void Game_PlaceTrumpetKeys(Game_Trumpet *trumpet)
 *
 * Initializes the rectangles for the key locations. */
void Game_PlaceTrumpetKeys(Game_Trumpet *trumpet) {
  const unsigned int key_gap = 95;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_2_rect.w,
                   &trumpet->key_2_rect.h);
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_3_rect.w,
                   &trumpet->key_3_rect.h);
  // Spread the keys.
  trumpet->key_1_rect.y = trumpet->key_2_rect.y = trumpet->key_3_rect.y = 150;
  trumpet->key_1_rect.x = 170;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  trumpet->key_2_rect.x = trumpet->key_1_rect.x + key_gap;
  SDL_QueryTexture(trumpet->keys_text, NULL, NULL, &trumpet->key_1_rect.w,
                   &trumpet->key_1_rect.h);
  trumpet->key_3_rect.x = trumpet->key_2_rect.x + key_gap;
}

/* int Game_LoadNotes(Game_Audio *audio)
 *
 * Loads the trumpet noises into an array of *Mix_Chunk 
 * in the Game_Audio struct
 *
 * return 0 if OK, else -1 */
int Game_LoadNotes(Game_Audio *audio, const char *path) {
  const int FPATH_BUFFER_SIZE = 32;
  const int FNAME_BUFFER_SIZE = 8;
  char filepath[FPATH_BUFFER_SIZE];
  char filename[FNAME_BUFFER_SIZE];
  printf("\nLoading notes...");
  for (int i = 0; i < 19; i++) {
    strcpy(filepath, path);
    snprintf(filename, FNAME_BUFFER_SIZE, "%d", i);
    strcat(filepath, filename);
    strcat(filepath, ".wav");
    printf("\n  Loading filepath: %s", filepath);
    audio->notes[i] = Mix_LoadWAV(filepath);
    if (audio->notes[i] == NULL) {
      printf("\n  Error loading: %s", filepath);
      return -1;
    }
  }
  printf("Notes done!");
  return 0;
}
