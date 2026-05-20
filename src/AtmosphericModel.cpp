#include "AtmosphericModel.h"
#include <cmath>
#include <algorithm>

AtmosphericModel::AtmosphericModel()
    : AtmosphericModel(Config{}) {}

AtmosphericModel::AtmosphericModel(const Config& cfg)
    : cfg_(cfg)
    , rng_(std::random_device{}())
    , shot_noise_(0.0, cfg.shot_sigma)
{}

AtmosphericModel::Sample AtmosphericModel::sample(uint16_t altitude_m) {
    double r = static_cast<double>(altitude_m);

    // Beer-Lambert: boundary layer vs free troposphere extinction
    double beta_ext = (r < 3000.0) ? 1e-4 : 1e-5;   // 1/m
    double signal   = cfg_.I0 * std::exp(-2.0 * beta_ext * r) * beta_ext;

    // Additive noise components
    double noise  = (cfg_.shot_sigma > 0.0) ? shot_noise_(rng_) : 0.0;
    double solar  = cfg_.solar_contaminated ? cfg_.solar_bg : 0.0;
    double total  = signal + noise + solar;

    // Scale to int16 fixed-point (x1000), clamped
    double scaled = total * 1000.0;
    int16_t raw = static_cast<int16_t>(
        std::clamp(scaled,
                   static_cast<double>(INT16_MIN),
                   static_cast<double>(INT16_MAX)));

    // SNR x10: signal-to-noise ratio scaled by I0 and encoded x10.
    // Normalise noise by I0 so the ratio is relative to the source power.
    double snr_linear = (cfg_.shot_sigma > 0.0)
        ? std::max(signal, 0.0) * cfg_.I0 / cfg_.shot_sigma
        : 25.0 * cfg_.I0;
    uint8_t snr = static_cast<uint8_t>(std::clamp(snr_linear * 10.0, 0.0, 255.0));

    uint8_t solar_flag = cfg_.solar_contaminated ? 1u : 0u;

    return { raw, snr, solar_flag };
}
