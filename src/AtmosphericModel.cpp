#include "AtmosphericModel.h"
AtmosphericModel::AtmosphericModel()
    : AtmosphericModel(Config{}) {}
AtmosphericModel::AtmosphericModel(const Config& cfg)
    : cfg_(cfg), rng_(std::random_device{}()), shot_noise_(0.0, cfg.shot_sigma) {}
AtmosphericModel::Sample AtmosphericModel::sample(uint16_t) { return {0, 0, 0}; }
