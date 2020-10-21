#include "binary_display.h"

static color img_buf[27] = {0}; 

void set_binary_display(time t) {
    for(int i = 0; i < 5; i++) {
        if(t.hour & 0x01) {
            img_buf[22 + i] = (color){0x10, 0x10, 0x00};
        } else {
            img_buf[22 + i] = (color){0};
        }
        t.hour >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.minute & 0x01) {
            img_buf[21 - i] = (color){0x00, 0x10, 0x10};
        } else {
            img_buf[21 - i] = (color){0};
        }
        t.minute >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.second & 0x01) {
            img_buf[10 + i] = (color){0x10, 0x00, 0x10};
        } else {
            img_buf[10 + i] = (color){0};
        }
        t.second >>= 1;
    }

    // img_buf[3] = (color){0x00, 0x00, 0x00};

    sendbyte((void*) img_buf, 27 * sizeof(color));
}