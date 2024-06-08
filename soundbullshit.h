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
        if (device_id != 0) {
            SDL_CloseAudioDevice(device_id);
        }
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
            throw std::out_of_range("Invalid sound index");
        }

        SDL_AudioSpec& wav_spec = sounds[index].wav_spec;
        Uint8* wav_buffer = sounds[index].wav_buffer;
        Uint32 wav_length = sounds[index].wav_length;

        if (device_id == 0) {
            device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
            if (device_id == 0) {
                throw std::runtime_error("Failed to open audio device: " + std::string(SDL_GetError()));
            }
        }

        SDL_ClearQueuedAudio(device_id);
        if (SDL_QueueAudio(device_id, wav_buffer, wav_length) < 0) {
            throw std::runtime_error("Failed to queue audio: " + std::string(SDL_GetError()));
        }

        SDL_PauseAudioDevice(device_id, 0);
    }

private:
    struct SoundData {
        Uint8* wav_buffer = nullptr;
        Uint32 wav_length = 0;
        SDL_AudioSpec wav_spec;
    };

    std::vector<SoundData> sounds;
    SDL_AudioDeviceID device_id = 0;
};

#endif // SOUNDBULLSHIT_H_INCLUDED
