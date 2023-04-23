#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct idac_shifter_config {
    bool auto_neutral;
};

struct idac_di_config {
    wchar_t device_name[64];
    wchar_t shifter_name[64];
    wchar_t brake_axis[16];
    wchar_t accel_axis[16];
    uint8_t start;
    uint8_t view_chg;
    uint8_t shift_dn;
    uint8_t shift_up;
    uint8_t gear[6];
    bool reverse_brake_axis;
    bool reverse_accel_axis;
};

struct idac_xi_config {
    bool single_stick_steering;
};

struct idac_io_config {
    uint8_t vk_test;
    uint8_t vk_service;
    uint8_t vk_coin;
    wchar_t mode[8];
    int restrict_;
    struct idac_shifter_config shifter;
    struct idac_di_config di;
    struct idac_xi_config xi;
};

void idac_di_config_load(struct idac_di_config *cfg, const wchar_t *filename);
void idac_xi_config_load(struct idac_xi_config *cfg, const wchar_t *filename);
void idac_io_config_load(struct idac_io_config *cfg, const wchar_t *filename);
void idac_shifter_config_load(
        struct idac_shifter_config *cfg,
        const wchar_t *filename);
