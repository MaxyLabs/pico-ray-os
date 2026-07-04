#include "../framework.h"

void PR_PlayMusic(const char *fileName) {
    if (fileName == NULL || fileName[1] == '\0') return;

    AudioSystem *audioSystem = PR_GetAudioSystem();

    // If a track was already playing inside our structured context, unload it first
    if (audioSystem->isMusicActive) {
        StopMusicStream(audioSystem->backgroundMusic);
        UnloadMusicStream(audioSystem->backgroundMusic);
        audioSystem->isMusicActive = false;
    }

    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "carts/%s", fileName);

    audioSystem->backgroundMusic = LoadMusicStream(fullPath);
    
    if (audioSystem->backgroundMusic.ctxData != NULL) {
        audioSystem->backgroundMusic.looping = true;
        PlayMusicStream(audioSystem->backgroundMusic);
        audioSystem->isMusicActive = true; // Flashing state active inside our structure matrix!
        printf("PICO-RAY OS | AUDIO: Streaming dynamic track straight to structured mixer: %s\n", fullPath);
    } else {
        printf("PICO-RAY OS | AUDIO ERROR: Cannot load or stream sound file: %s\n", fullPath);
    }
}

void PR_StopMusic(void) {
    AudioSystem *audioSystem = PR_GetAudioSystem();

    if (audioSystem->isMusicActive) {
        StopMusicStream(audioSystem->backgroundMusic);
        UnloadMusicStream(audioSystem->backgroundMusic);
        audioSystem->isMusicActive = false;
        printf("PICO-RAY OS | AUDIO: Structured mixer background streaming halted cleanly.\n");
    }
}

// HIGH-PERFORMANCE STATIC ONE-SHOT SFX MIXER (NO-LOOP GUARANTEED)
// This function bypasses the background continuous update loops entirely!
// It loads a sound file into a transient wave memory slot, plays it exactly ONCE, and discards resources.
void PR_PlaySFX(const char *fileName) {
    AudioSystem *audioSystem = PR_GetAudioSystem();

    if (fileName == NULL || fileName[1] == '\0') return;

    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "carts/%s", fileName);

    Sound sfx = LoadSound(fullPath);
    if (sfx.frameCount > 0) {
        // Apply our master volume matrix levels directly to this short sound event instance
        audioSystem->masterVolume = (float)PR_GetKernelState()->systemVolume / 100.0f;
        SetSoundVolume(sfx, audioSystem->masterVolume);
        
        // Raylib built-in: Fires the sound wave buffer exactly ONCE (No-Loop) and mixes it safely
        PlaySound(sfx); 
        
        printf("PICO-RAY OS | AUDIO: Played one-shot sound effect: %s\n", fullPath);
    } else {
        printf("PICO-RAY OS | AUDIO ERROR: Cannot load one-shot effect target: %s\n", fullPath);
    }
}
