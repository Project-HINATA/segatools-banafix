#pragma once

struct openssl_patch_config {
    int enable; 
};

HRESULT openssl_patch_apply(const struct openssl_patch_config *cfg);  
