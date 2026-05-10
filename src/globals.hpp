#pragma once

#include <cstdint>

const uint8_t kPROTOCOL_GET = 0x33;
const uint8_t kPROTOCOL_SET = 0x32;

const uint8_t kPROTOCOL_FAN = 0x51;
const uint8_t kPROTOCOL_LIGHT = 0x52;

// credit: https://github.com/devcompl/riingplusapi
class RGB {
    class Mode {
        const uint8_t kLOW = 0x00;
        const uint8_t kPECTRUM = 0x04;
        const uint8_t kIPPLE = 0x08;
        const uint8_t kLINK = 0x0c;
        const uint8_t kULSE = 0x10;
        const uint8_t kAVE = 0x14;
        const uint8_t kY_LED = 0x18;
        const uint8_t kULL = 0x19;
    };

    class Speed {
        const uint8_t kLOW = 0x03;
        const uint8_t kORMAL = 0x02;
        const uint8_t kAST = 0x01;
        const uint8_t kXTREME = 0x00;
    };
};
