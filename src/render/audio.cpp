#include "audio.hpp"
#include "sdl_system.hpp"
namespace Audio
{

Sound::Sound(const char *wavPath)
{
    c = Mix_LoadWAV(wavPath);
    if (c == NULL)
        printf("Failed to load sound %s, error: %s\n", wavPath, SDL_GetError());
}

Sound::~Sound() { Mix_FreeChunk(c); }

void Sound::play() { Mix_PlayChannel(-1, c, 0); }

Music::Music(const char *wavPath) : m{NULL}
{
    m = Mix_LoadMUS(wavPath);
    if (m == NULL)
    {
        printf("Couldn't load music %s, Error: %s\n", wavPath, Mix_GetError());
        return;
    }
}

Music::~Music() { Mix_FreeMusic(m); }

void Music::play()
{
    if (Mix_PlayingMusic())
        Mix_FadeOutMusic(1000);
    Mix_FadeInMusic(m, -1, 1000);
}

int System::init()
{
    if (Mix_Init(0))
    {
        printf(
            "Failed to initialize audio system, Error: %s\n", Mix_GetError());
        return 1;
    };

    if (SDL_System::initSDL(SDL_INIT_AUDIO))
    {
        printf("Failed to initialize SDL Audio, Error: %s", SDL_GetError());
        return 1;
    }
    Mix_OpenAudio(AUDIO_F32SYS, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);

    Mix_VolumeMusic(MIX_MAX_VOLUME - 1);
    Mix_MasterVolume(MIX_MAX_VOLUME - 1);

    return 0;
}

void System::quit()
{
    Mix_MasterVolume(0);
    Mix_Quit();
}

} // namespace Audio