#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace Audio
{

class System;

class Sound
{
  public:
    Sound(const char *wavPath);
    ~Sound();
    void play();

  private:
    Mix_Chunk *c;
};

class Music
{
  public:
    Music(const char *wavPath);
    ~Music();
    void play();
    void setPause(bool pause);
    void stop();
    void setVolume(int v)
    {
        volume = v;
        Mix_VolumeMusic(volume);
    }

  private:
    Mix_Music *m;
    int volume;

    bool paused;
};

class System
{
  public:
    System()         = delete;
    ~System()        = delete;
    System(System &) = delete;
    static Sound createSound(const char *wavPath);
    static int init();
    static void quit();
};
} // namespace Audio