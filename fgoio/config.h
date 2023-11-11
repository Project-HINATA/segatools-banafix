#pragma once

#include <stddef.h>
#include <stdint.h>

#include <stdbool.h>

struct fgo_io_config {
    uint8_t vk_test;
    uint8_t vk_service;
    uint8_t vk_coin;
};

void fgo_io_config_load(
        struct fgo_io_config *cfg,
        const wchar_t *filename);
