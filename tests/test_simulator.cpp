#include <gtest/gtest.h>
#include "SpscQueue.h"
#include "TelemetryFrame.h"

TEST(SpscQueue, PushPopRoundtrip) {
    SpscQueue<TelemetryFrame, 4> q;
    TelemetryFrame f{};
    f.altitude_m = 1234;
    EXPECT_TRUE(q.push(f));
    auto result = q.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->altitude_m, 1234u);
}

TEST(SpscQueue, FullReturnsFalse) {
    SpscQueue<TelemetryFrame, 4> q;
    TelemetryFrame f{};
    // capacity is N-1 = 3 usable slots
    EXPECT_TRUE(q.push(f));
    EXPECT_TRUE(q.push(f));
    EXPECT_TRUE(q.push(f));
    EXPECT_FALSE(q.push(f));  // queue full
}

TEST(SpscQueue, EmptyReturnsNullopt) {
    SpscQueue<TelemetryFrame, 4> q;
    EXPECT_FALSE(q.pop().has_value());
}
