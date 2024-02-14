/* Copyright [2024] Mikko Memonen
 *
 * A goofy little trumpet simulator for fun and practice.
 * Works EXACTLY like a real one. */

// SDL2 libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
// Standard stuff
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
  unsigned int blow_force;     // air velocity which determines pitch
  unsigned int key_a_pressed;  // which is controlled by a, b and d
  unsigned int key_s_pressed;
  unsigned int key_d_pressed;
  unsigned int doot;  // 1 if a doot should come out, -1 to stop
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

// Play the trumpet!
void Game_PlayTrumpet(Game_Trumpet *trumpet, Game_Audio *audio);

// Check if a note is playing.
int Game_CheckIfPlaying(Game_Trumpet *trumpet);

// Draw the keys in their places.
void Game_DrawTrumpet(Game_Trumpet *trumpet, SDL_Renderer *renderer);

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

  // Initialize Game and the HORN!
  Game game = {
      .SDL_Init = -1, .MIX_Init = -1, .window = NULL, .renderer = NULL};
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
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }

  // Create the game window.
  game.window =
      SDL_CreateWindow("trumpet_tyckiter", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (game.window == NULL) {
    printf("\nCouldn't create window: %s", SDL_GetError());
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }

  // Create a renderer.
  game.renderer = SDL_CreateRenderer(game.window, -1, RENDERER_FLAGS);
  if (game.renderer == NULL) {
    printf("\nCouldn't create renderer: %s", SDL_GetError());
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }

  // Create the trumpet valves and keys textures.
  trumpet.valves_text = IMG_LoadTexture(game.renderer, VALVES_PATH);
  trumpet.keys_text = IMG_LoadTexture(game.renderer, KEY_PATH);
  if (trumpet.valves_text == NULL) {
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }
  if (trumpet.keys_text == NULL) {
    printf("Couldn't load trumpet keys!");
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }
  Game_PlaceTrumpetKeys(&trumpet);

  // Initialize the notes.
  Game_Audio audio;
  if (Game_LoadNotes(&audio, RESOURCES_SOUND_PATH)) {
    printf("\nCouldn't load DOOTS :( (%s))", SDL_GetError());
    Game_QuitAll(&game, &trumpet, NULL);
    return EXIT_FAILURE;
  }

  unsigned int stop_request = 0;
  const unsigned int valve_press_length = 40;
  unsigned int error = 0;
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
          if (!trumpet.key_1_pressed) {  // TODO: Is this checking necessary?
            trumpet.key_1_rect.y += valve_press_length;
            trumpet.key_1_pressed = 1;
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (!trumpet.key_2_pressed) {
            trumpet.key_2_rect.y += valve_press_length;
            trumpet.key_2_pressed = 1;
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (!trumpet.key_3_pressed) {
            trumpet.key_3_rect.y += valve_press_length;
            trumpet.key_3_pressed = 1;
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_A:
          if (!trumpet.key_a_pressed) {
            trumpet.key_a_pressed = 1;
            trumpet.blow_force = 6;
            trumpet.doot = 1;
          }
          break;
        case SDL_SCANCODE_S:
          if (!trumpet.key_s_pressed) {
            trumpet.key_s_pressed = 1;
            trumpet.blow_force = 13;
            trumpet.doot = 1;
          }
          break;
        case SDL_SCANCODE_D:
          if (!trumpet.key_d_pressed) {
            trumpet.key_d_pressed = 1;
            trumpet.blow_force = 18;
            trumpet.doot = 1;
          }
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
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_DOWN:
          if (trumpet.key_2_pressed) {
            trumpet.key_2_rect.y -= valve_press_length;
            trumpet.key_2_pressed = 0;
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if (trumpet.key_3_pressed) {
            trumpet.key_3_rect.y -= valve_press_length;
            trumpet.key_3_pressed = 0;
            trumpet.doot = Game_CheckIfPlaying(&trumpet);
          }
          break;
        case SDL_SCANCODE_A:
          trumpet.key_a_pressed = 0;
          break;
        case SDL_SCANCODE_S:
          trumpet.key_s_pressed = 0;
          break;
        case SDL_SCANCODE_D:
          trumpet.key_d_pressed = 0;
          break;
        }
        break;
      }
    }
    // End of event loop.

    // Handle the music.
    Game_PlayTrumpet(&trumpet, &audio);

    // Draw black backgound.
    error += SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    error += SDL_RenderClear(game.renderer);

    // Add textures to buffer and show it.
    Game_DrawTrumpet(&trumpet, game.renderer);
    SDL_RenderPresent(game.renderer);

    if (error) {
      printf("\nError in the game loop! %s", SDL_GetError());
    }

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
  if (game != NULL) {
    // Mix_OpenAudio returns 0 if OK
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
  }

  if (trumpet != NULL) {
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
  }

  if (audio != NULL) {
    // TODO: Fix magic number 19!
    printf("\nFreeing Mix Chunks..");
    for (int i = 0; i < 19; i++) {
      if (audio->notes[i] != NULL) {
        Mix_FreeChunk(audio->notes[i]);
        printf("\n  Freed chunk %d", i);
      }
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

/* int Game_CheckIfPlaying(Game_Trumpet *trumpet)
 *
 * Returns 1 if a note should be playing (a, s or d pressed)
 * Used for slurring notes (changing fingering while playing a note) */
int Game_CheckIfPlaying(Game_Trumpet *trumpet) {
  if (trumpet->key_a_pressed + trumpet->key_s_pressed +
      trumpet->key_d_pressed) {
    return 1;
  }
  return 0;
}

/* void Game_PlayTrumpet(Game_Trumpet *trumpet, Game_Audio *audio)
 *
 * Handles the playing and stopping notes depenging on the
 * trumpet state. Playing works pretty much just like a REAL trumpet.
 * (This is a C trumpet, btw.) */
void Game_PlayTrumpet(Game_Trumpet *trumpet, Game_Audio *audio) {
  if (trumpet->key_a_pressed + trumpet->key_s_pressed +
          trumpet->key_d_pressed ==
      0) {
    Mix_FadeOutChannel(1, 1);
    return;
  }
  if (trumpet->doot) {
    int note = trumpet->blow_force - trumpet->key_1_pressed * 2 -
               trumpet->key_2_pressed - trumpet->key_3_pressed * 3;
    Mix_FadeInChannel(1, audio->notes[note], 0, 1);
    trumpet->doot = 0;
  }
}

/* void Game_DrawTrumpet(Game_Trumpet *trumpet, SDL_Renderer *renderer)
 *
 * Draw the trumpet parts in their rightful places */
void Game_DrawTrumpet(Game_Trumpet *trumpet, SDL_Renderer *renderer) {
  // Keys
  SDL_Rect *keys[3] = {&trumpet->key_1_rect, &trumpet->key_2_rect,
                      &trumpet->key_3_rect};

  for (int i = 0; i < 3; i++) {
    if (SDL_RenderCopy(renderer, trumpet->keys_text, NULL, keys[i])) {
      printf("Error copying keys: %s", SDL_GetError());
    }
    // Valves
    if (SDL_RenderCopy(renderer, trumpet->valves_text,
                       NULL,  // use the whole image as source
                       NULL)  // stretch to fit the whole window
    ) {
      printf("Error copying valves: %s", SDL_GetError());
    }
  }
}
