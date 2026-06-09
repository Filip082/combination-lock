#include "keypad.h"

uint8_t column_pins[] = COL_PINS;
uint8_t row_pins[] = ROW_PINS;

const uint8_t keys [4][4] = KEYPAD_LAYOUT;

static uint8_t          queue[QUEUE_SIZE];
static volatile uint8_t q_head = 0;
static volatile uint8_t q_tail = 0;

static void keyEnqueue(uint8_t key) {
    uint8_t next = (q_head + 1) % QUEUE_SIZE;
    if (next != q_tail) {           // drop silently when full
        queue[q_head] = key;
        q_head = next;
    }
}

uint8_t keyDequeue(uint8_t *key) {
    if (q_head == q_tail) 
        return 0;

    *key = queue[q_tail];
    q_tail = (q_tail + 1) % QUEUE_SIZE;

    if (q_head == q_tail)
        gpio_put(IRQ_PIN, 0);

    return 1;
}

uint8_t keyPending() {
    return q_head != q_tail;
}

static bool readColumn(repeating_timer_t *rt) {
    static uint8_t column = 0;

    for (int i = 0; i < 4 && column > 0; ++i)
        if (!gpio_get(row_pins[i]))
            keyEnqueue(keys[i][column - 1]);

    gpio_set_dir_in_masked(COL_PINS_MASK);
    gpio_set_dir(column_pins[column], GPIO_OUT);
    ++column;

    if (column < 5) return true;

    column = 0;
    gpio_set_dir_out_masked(COL_PINS_MASK);
    irq_clear(IO_IRQ_BANK0);
    irq_set_enabled(IO_IRQ_BANK0, true);
    cancel_repeating_timer(rt);

    gpio_put(IRQ_PIN, 1);

    return true;
}

static void gpio_callback(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);

    static uint32_t last_time = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_time < 200) return;
    last_time = now;

    irq_set_enabled(IO_IRQ_BANK0, false);

    // czy tu jest potrzebny timer?
    static repeating_timer_t scan_timer;
    add_repeating_timer_us(20U, readColumn, &scan_timer, &scan_timer);
}

void initKeypad() {
    gpio_init_mask(COL_PINS_MASK);
    for (uint i = 0; i < 4; ++i)
        gpio_disable_pulls(column_pins[i]);
    gpio_clr_mask(COL_PINS_MASK);
    gpio_set_dir_out_masked(COL_PINS_MASK);

    gpio_init_mask(ROW_PINS_MASK);
    for (uint i = 0; i < 4; ++i) {
        gpio_disable_pulls(row_pins[i]);
        if (i == 0)
            gpio_set_irq_enabled_with_callback(row_pins[i], GPIO_IRQ_EDGE_FALL, true, gpio_callback);
        else
            gpio_set_irq_enabled(row_pins[i], GPIO_IRQ_EDGE_FALL, true);
    }

    gpio_init(IRQ_PIN);
    gpio_set_dir(IRQ_PIN, GPIO_OUT);
    gpio_put(IRQ_PIN, 0);
}
