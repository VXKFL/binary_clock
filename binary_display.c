#include "binary_display.h"

static struct colour img_buf[60] = {0}; 

void set_binary_display(struct time t) {
    for(int i = 0; i < 5; i++) {
        if(t.hour & 0x01) {
            img_buf[0 + i] = (struct colour){0xff, 0xff, 0x00};
        } else {
            img_buf[0 + i] = (struct colour){0};
        }
        t.hour >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.minute & 0x01) {
            img_buf[5 + i] = (struct colour){0x00, 0xff, 0xff};
        } else {
            img_buf[5 + i] = (struct colour){0};
        }
        t.minute >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.second & 0x01) {
            img_buf[11 + i] = (struct colour){0xff, 0x00, 0xff};
        } else {
            img_buf[11 + i] = (struct colour){0};
        }
        t.second >>= 1;
    }

    // img_buf[3] = (struct colour){0x00, 0x00, 0x00};

    sendbyte((void*) img_buf, 60 * sizeof(struct colour));
}