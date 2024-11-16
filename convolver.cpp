#include <iostream>
#include "AudioFile.h"
#include "benchmarker.h"

static constexpr size_t BLOCK_SIZE = 64;
static constexpr size_t SAMPLERATE_K = 48;
static constexpr float BLOCK_TIME = 1.f/SAMPLERATE_K * BLOCK_SIZE;
static constexpr size_t FIR_SIZE = BLOCK_SIZE * 4;

class Convolver
{
    public:
        Convolver() = default;

        void import_audio()
        {
            std::string sourceFileName, impulseFileName;

            std::cout << "Enter the file name for the source audio file: ";
            std::cin >> sourceFileName;
            std::cout << "Enter the file name for the impulse audio file: ";
            std::cin >> impulseFileName;
           
            bool loadedSource = sourceFile.load(sourceFileName);
            bool loadedImpulse = impulseFile.load(impulseFileName);

            if(!loadedSource || !loadedImpulse){std::cerr << "Failed to load audio files." << std::endl;return;}
            else {std::cout << "Loaded audio files." << std::endl;}

            sourceSamples = sourceFile.samples[0];
            impulseSamples = impulseFile.samples[0];
            outputSamples.resize(sourceSamples.size() + impulseSamples.size() - 1);
        }

        void import_block()
        {
            std::string sourceFileName, impulseFileName;
            if (sourceFileName.empty()) sourceFileName = "voice.wav";
            if (impulseFileName.empty()) impulseFileName = "noise.wav";
            bool loadedSource = sourceFile.load(sourceFileName);
            bool loadedImpulse = impulseFile.load(impulseFileName);

            if(!loadedSource || !loadedImpulse){std::cerr << "Failed to load audio files." << std::endl;return;}
            else {std::cout << "Loaded audio files." << std::endl;}

            sourceSamples = std::vector<float>(sourceFile.samples[0].begin(), sourceFile.samples[0].begin() + std::min<size_t>(BLOCK_SIZE, sourceFile.samples[0].size()));
            impulseSamples = std::vector<float>(impulseFile.samples[0].begin(), impulseFile.samples[0].begin() + std::min<size_t>(FIR_SIZE, impulseFile.samples[0].size()));
            outputSamples.resize(sourceSamples.size() + impulseSamples.size() - 1);
        }

        void export_audio()
        {
            AudioFile<float> outputFile;
            outputFile.samples[0] = outputSamples;
            outputFile.save("output.wav");
            std::cout << "Audio file exported to output.wav" << std::endl;
        }

        void process()
        {
            // Perform the convolution
            for (int i = 0; i < outputSamples.size(); ++i)
            {
                outputSamples[i] = 0.0f;

                for (int j = 0; j < impulseSamples.size(); ++j)
                {
                    if (i - j >= 0 && i - j < sourceSamples.size())
                    {
                        outputSamples[i] += sourceSamples[i - j] * impulseSamples[j];
                    }
                }
            }
        }

        void normalise()
        {
            // Calculate the gain of the FIR filter
            float gain = 0.0f;
            for (const auto& sample : impulseSamples)
            {
                gain += sample;
            }

            // Attenuate the outputSamples to compensate for the gain
            if (gain != 0.0f)
            {
                for (auto& sample : outputSamples)
                {
                sample /= gain;
                }
            }
        }

    private:
        AudioFile<float> sourceFile;
        AudioFile<float> impulseFile;
        std::vector<float> sourceSamples;
        std::vector<float> impulseSamples;
        std::vector<float> outputSamples;
        bool convolving{false};
};

int main()
{
    Convolver convolver;

    convolver.import_block();
    std::vector<std::string> test_names = {"Convolution Process"};
    Benchmark benchmark(test_names);

    while(!benchmark.complete())
    {
        benchmark.from(0);
        convolver.process();
        benchmark.to(0);
    }

    float average_time = benchmark.get_average_time();

    std::cout << "FIR Duration = " << 1.f/SAMPLERATE_K * FIR_SIZE  << "ms" << std::endl;
    std::cout << "Average time: " << average_time << "ms" << std::endl;
    std::cout << "Block time: " << BLOCK_TIME << "ms" << std::endl;
    std::cout << "Percentage of DSP time: " << average_time / BLOCK_TIME * 100 << "%" << std::endl;

    convolver.normalise();
    convolver.export_audio();
    
    return 0;
}
