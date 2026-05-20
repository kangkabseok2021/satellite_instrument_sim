#pragma once
#include <random>
#include <cstdint>

class AtmosphericModel {
public:
    struct Config {
        double I0          = 1000.0;
        double shot_sigma  = 5.0;
        double solar_bg    = 0.0;
        bool   solar_contaminated = false;
    };

    AtmosphericModel();
    explicit AtmosphericModel(const Config& cfg);

    struct Sample { int16_t backscatter_raw; uint8_t snr_x10; uint8_t solar_flag; };
    Sample sample(uint16_t altitude_m);

private:
    Config cfg_;
    std::mt19937 rng_;
    std::normal_distribution<double> shot_noise_;
};
