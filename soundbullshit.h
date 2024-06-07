#ifndef SOUNDBULLSHIT_H_INCLUDED
#define SOUNDBULLSHIT_H_INCLUDED

class AudioPlayer {
public:

    AudioPlayer() : audio_pos(nullptr), audio_len(0), wav_buffer(nullptr) {
    }

    ~AudioPlayer() {
        if (wav_buffer) {
            SDL_FreeWAV(wav_buffer);
        }
        SDL_CloseAudio();
    }

    void loadAudio(const std::string& filePath) {
        // Load the WAV file
        if (SDL_LoadWAV(filePath.c_str(), &wav_spec, &wav_buffer, &wav_length) == NULL) {
            throw std::runtime_error("Failed to load WAV file: " + std::string(SDL_GetError()));
        }

        // Set our audio position and length
        audio_pos = wav_buffer;
        audio_len = wav_length;

        // Open the audio device
        if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
            throw std::runtime_error("Couldn't open audio: " + std::string(SDL_GetError()));
        }
        SDL_QueueAudio(1, wav_buffer, wav_length);
    }

    void play() {
        SDL_PauseAudio(0);
    }

    void stop() {
        //if(audio_len <= 0)
            //SDL_PauseAudio(1);
    }


private:
    Uint8* audio_pos;
    Uint32 audio_len;
    Uint8* wav_buffer;
    Uint32 wav_length;
    SDL_AudioSpec wav_spec;
};

#endif // SOUNDBULLSHIT_H_INCLUDED
