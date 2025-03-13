#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

int main() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return -1;
    }

    // Load your MP3 or WAV file
    Mix_Music *music = Mix_LoadMUS("your_audio_file.mp3");
    if (!music) {
        printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
        return -1;
    }

    // Play the audio
    if (Mix_PlayMusic(music, -1) == -1) {
        printf("Mix_PlayMusic failed: %s\n", Mix_GetError());
        return -1;
    }

    // Wait for the music to finish playing
    SDL_Delay(10000); // Adjust delay as necessary for the song's length

    // Clean up
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}

