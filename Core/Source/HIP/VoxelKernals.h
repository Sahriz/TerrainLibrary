#pragma once

extern "C" {
    void testHIP();
    void launchExampleKernel(float* output, int size);
}