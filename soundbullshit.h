#ifndef SOUNDBULLSHIT_H_INCLUDED
#define SOUNDBULLSHIT_H_INCLUDED

class AudioPlayer {
public:

    AudioPlayer() = default;

    ~AudioPlayer() {
        for (auto& sound : sounds) {
            if (sound.wav_buffer) {
                SDL_FreeWAV(sound.wav_buffer);
            }
        }
        SDL_CloseAudio();
        SDL_Quit();
    }

    void loadAudio(const std::string& filePath) {
        SoundData sound;
        if (SDL_LoadWAV(filePath.c_str(), &sound.wav_spec, &sound.wav_buffer, &sound.wav_length) == NULL) {
            throw std::runtime_error("Failed to load WAV file: " + std::string(SDL_GetError()));
        }
        sounds.push_back(sound);
    }

    void play(int index) {
        if (index < 0 || index >= sounds.size()) {
            throw std::out_of_range("Sound index out of range");
        }

        SDL_AudioSpec& wav_spec = sounds[index].wav_spec;
        Uint8* wav_buffer = sounds[index].wav_buffer;
        Uint32 wav_length = sounds[index].wav_length;

        if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
            throw std::runtime_error("Couldn't open audio: " + std::string(SDL_GetError()));
        }

        SDL_QueueAudio(1, wav_buffer, wav_length);
        SDL_PauseAudio(0);
    }

private:
    struct SoundData {
        Uint8* wav_buffer = nullptr;
        Uint32 wav_length = 0;
        SDL_AudioSpec wav_spec;
    };

    std::vector<SoundData> sounds;
};

#endif // SOUNDBULLSHIT_H_INCLUDED
