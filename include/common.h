#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <cstring>

enum state {
    NO = 0,
    YES = 1,
    IDLE = 2,
    ADDRESS = 3,
    KEY = 4,
    CODE = 5
};

enum log_event {
    EVENT_ACCESS_OK     = 0x01,
    EVENT_ACCESS_DENIED = 0x02,
};

#define CONFIG_VERSION 1

// flat_count = max_address - min_address + 1
struct config_block {
    uint8_t  header[8];  // "DOMOFON\0"
    uint8_t  version;
    uint8_t log_capacity;  // maksymalna liczba wpisów w ring buffer
    uint8_t log_head;      // indeks kolejnego wolnego miejsca
    uint8_t log_count;     // liczba zapisanych wpisów (do min: log_capacity)
    uint16_t min_address;
    uint16_t max_address;
} __attribute__((packed));

// Układ FRAM:
// [0x0000]                    struct config_block
// [sizeof(config_block)]      struct flat[max_address - min_address + 1]
// [po flat[]]                 struct log_entry[log_capacity]  <- ring buffer

struct flat {
    uint16_t address;
    uint16_t pin;
    uint8_t  last_access[6];  // BCD: YY MM DD HH MM SS
    uint8_t  last_denied[6];  // BCD: YY MM DD HH MM SS
} __attribute__((packed));

struct log_entry {
    uint8_t  timestamp[6];  // BCD: YY MM DD HH MM SS
    uint16_t address;
    uint8_t  event;         // enum log_event
} __attribute__((packed));  // 9 B

inline bool validate_config(const uint8_t *buf, size_t len, bool header_only = false) {
    if (len < sizeof(config_block)) return false;
    const config_block *cfg = (const config_block *) buf;
    if (strcmp((char *)cfg->header, "DOMOFON") != 0) return false;
    if (cfg->version != CONFIG_VERSION) return false;
    if (cfg->min_address > cfg->max_address) return false;
    if (!header_only) {
        if (len < sizeof(config_block) + (cfg->max_address - cfg->min_address + 1) * sizeof(flat)) return false;
    }
    return true;
}

#endif // COMMON_H
