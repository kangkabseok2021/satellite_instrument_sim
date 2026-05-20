#include <gtest/gtest.h>
#include "AtmosphericModel.h"

TEST(AtmosphericModel, BackscatterDecaysWithRange) {
    AtmosphericModel::Config cfg;
    cfg.shot_sigma = 0.0;
    cfg.solar_bg   = 0.0;
    AtmosphericModel m(cfg);
    auto s_near = m.sample(1000);
    auto s_far  = m.sample(20000);
    EXPECT_GT(s_near.backscatter_raw, s_far.backscatter_raw);
}

TEST(AtmosphericModel, BackscatterPositiveAtLowAltitude) {
    AtmosphericModel::Config cfg;
    cfg.shot_sigma = 0.0;
    AtmosphericModel m(cfg);
    EXPECT_GT(m.sample(500).backscatter_raw, 0);
}

TEST(AtmosphericModel, SolarFlagClearWhenNotContaminated) {
    AtmosphericModel::Config cfg;
    AtmosphericModel m(cfg);
    EXPECT_EQ(m.sample(5000).solar_flag, 0u);
}

TEST(AtmosphericModel, SolarFlagSetWhenContaminated) {
    AtmosphericModel::Config cfg;
    cfg.solar_contaminated = true;
    cfg.solar_bg = 50.0;
    AtmosphericModel m(cfg);
    EXPECT_EQ(m.sample(5000).solar_flag, 1u);
}

TEST(AtmosphericModel, SnrDecreasesWithRange) {
    AtmosphericModel::Config cfg;
    cfg.shot_sigma = 1.0;
    AtmosphericModel m(cfg);
    double snr_near = 0, snr_far = 0;
    for (int i = 0; i < 100; ++i) {
        snr_near += m.sample(500).snr_x10;
        snr_far  += m.sample(25000).snr_x10;
    }
    EXPECT_GT(snr_near / 100.0, snr_far / 100.0);
}
