#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct swdc_shifter_config {
    bool auto_neutral;
};

struct swdc_di_config {
    wchar_t device_name[64];
    wchar_t brake_axis[16];
    wchar_t accel_axis[16];
    uint8_t start;
    uint8_t view_chg;
    uint8_t shift_dn;
    uint8_t shift_up;
    uint8_t wheel_green;
    uint8_t wheel_red;
    uint8_t wheel_blue;
    uint8_t wheel_yellow;
    bool reverse_brake_axis;
    bool reverse_accel_axis;
};

struct swdc_xi_config {
    bool single_stick_steering;
    bool linear_steering;
    uint16_t left_stick_deadzone;
    uint16_t right_stick_deadzone;
};

struct swdc_io_config {
    uint8_t vk_test;
    uint8_t vk_service;
    uint8_t vk_coin;
    wchar_t mode[8];
    int restrict_;
    struct swdc_shifter_config shifter;
    struct swdc_di_config di;
    struct swdc_xi_config xi;
};

void swdc_di_config_load(struct swdc_di_config *cfg, const wchar_t *filename);
void swdc_xi_config_load(struct swdc_xi_config *cfg, const wchar_t *filename);
void swdc_io_config_load(struct swdc_io_config *cfg, const wchar_t *filename);
void swdc_shifter_config_load(
        struct swdc_shifter_config *cfg,
        const wchar_t *filename);
