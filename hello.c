// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <SDL/SDL.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* Function Prototypes */
void PrintKeyInfo(SDL_KeyboardEvent *key);
void PrintModifiers(unsigned short mod);
void one_iter();
SDL_Event event;

int quit = 0;
SDL_Surface *screen = nullptr;
bool colorFlag = false;

void paintScreen() {
  colorFlag = !colorFlag;
#ifdef TEST_SDL_LOCK_OPTS
  EM_ASM("SDL.defaults.copyOnLock = false; SDL.defaults.discardOnLock = true; "
         "SDL.defaults.opaqueFrontBuffer = false;");
#endif

  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
#ifdef TEST_SDL_LOCK_OPTS
      // Alpha behaves like in the browser, so write proper opaque pixels.
      int alpha = 255;
#else
      // To emulate native behavior with blitting to screen, alpha component is
      // ignored. Test that it is so by outputting data (and testing that it
      // does get discarded)
      int alpha = (i + j) % 255;
#endif

      if (colorFlag) {
        *((Uint32 *)screen->pixels + i * 256 + j) =
            SDL_MapRGBA(screen->format, j, i, i, alpha);
      } else {
        *((Uint32 *)screen->pixels + i * 256 + j) =
            SDL_MapRGBA(screen->format, i, j, 255 - i, alpha);
      }
    }
  }
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
  SDL_Flip(screen);
}

extern "C" int main(int argc, char **argv) {

  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  paintScreen();

  printf("you should see a smoothly-colored square - no sharp lines but the "
         "square borders!\n");
  printf("and here is some text that should be HTML-friendly: amp: |&| "
         "double-quote: |\"| quote: |'| less-than, greater-than, html-like "
         "tags: |<cheez></cheez>|\nanother line.\n");

  /* Loop until an SDL_QUIT event is found */
  emscripten_set_main_loop(one_iter, 60, 1);
  while (!quit) {

    one_iter();
    SDL_Delay(500);
  }

  SDL_Quit();

  return 0;
}

void one_iter() {

  /* Poll for events */
  while (SDL_PollEvent(&event)) {

    switch (event.type) {
    /* Keyboard event */
    /* Pass the event data onto PrintKeyInfo() */
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      PrintKeyInfo(&event.key);
      break;

    /* SDL_QUIT event (window close) */
    case SDL_QUIT:
      quit = 1;
      break;

    default:
      break;
    }
  }
}

/* Print all information about a key event */
void PrintKeyInfo(SDL_KeyboardEvent *key) {

  paintScreen();

  /* Is it a release or a press? */
  if (key->type == SDL_KEYUP)
    printf("Release:- ");
  else
    printf("Press:- ");

  /* Print the hardware scancode first */
  printf("Scancode: 0x%02X", key->keysym.scancode);
  /* Print the name of the key */
  printf(", Name: %s", SDL_GetKeyName(key->keysym.sym));
  /* We want to print the unicode info, but we need to make */
  /* sure its a press event first (remember, release events */
  /* don't have unicode info                                */
  if (key->type == SDL_KEYDOWN) {
    /* If the Unicode value is less than 0x80 then the    */
    /* unicode value can be used to get a printable       */
    /* representation of the key, using (char)unicode.    */
    printf(", Unicode: ");
    if (key->keysym.unicode < 0x80 && key->keysym.unicode > 0) {
      printf("%c (0x%04X)", (char)key->keysym.unicode, key->keysym.unicode);
    } else {
      printf("? (0x%04X)", key->keysym.unicode);
    }
  }
  printf("\n");
  /* Print modifier info */
  PrintModifiers(key->keysym.mod);
}

/* Print modifier info */
void PrintModifiers(unsigned short mod) {
  printf("Modifers: ");

  /* If there are none then say so and return */
  if (mod == KMOD_NONE) {
    printf("None\n");
    return;
  }

  /* Check for the presence of each SDLMod value */
  /* This looks messy, but there really isn't    */
  /* a clearer way.                              */
  if (mod & KMOD_NUM)
    printf("NUMLOCK ");
  if (mod & KMOD_CAPS)
    printf("CAPSLOCK ");
  if (mod & KMOD_LCTRL)
    printf("LCTRL ");
  if (mod & KMOD_RCTRL)
    printf("RCTRL ");
  if (mod & KMOD_RSHIFT)
    printf("RSHIFT ");
  if (mod & KMOD_LSHIFT)
    printf("LSHIFT ");
  if (mod & KMOD_RALT)
    printf("RALT ");
  if (mod & KMOD_LALT)
    printf("LALT ");
  if (mod & KMOD_CTRL)
    printf("CTRL ");
  if (mod & KMOD_SHIFT)
    printf("SHIFT ");
  if (mod & KMOD_ALT)
    printf("ALT ");
  printf("\n");
}