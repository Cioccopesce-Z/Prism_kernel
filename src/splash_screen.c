#include "stdf.h"
#include "splash_screen.h"
#include "kernel.h"

void prism_screen(void){

    print_at_color(
        "+------------------------------------------------------------------------------+",
        1, 0, VGA_LIGHT_CYAN
    );

    print_at_color(
        "|                                                                              |",
        2, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                            P R I S M   K E R N E L                           |",
        3, 0, VGA_LIGHT_MAGENTA
    );

    print_at_color(
        "|                                                                              |",
        4, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                        +------------------------+                            |",
        5, 0, VGA_LIGHT_CYAN
    );

    print_at_color(
        "|                        |                        |                            |",
        6, 0, VGA_LIGHT_CYAN
    );

    print_at_color(
        "|                        |      P R I S M         |                            |",
        7, 0, VGA_LIGHT_MAGENTA
    );

    print_at_color(
        "|                        |        KERNEL          |                            |",
        8, 0, VGA_LIGHT_RED
    );

    print_at_color(
        "|                        |                        |                            |",
        9, 0, VGA_LIGHT_CYAN
    );

    print_at_color(
        "|                        +------------------------+                            |",
        10, 0, VGA_LIGHT_CYAN
    );

    print_at_color(
        "|                                                                              |",
        11, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                                                                              |",
        12, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                       SYSTEM INITIALIZED                                     |",
        13, 0, VGA_LIGHT_GREEN
    );

    print_at_color(
        "|                                                                              |",
        14, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                         [ KERNEL I MEAN U CAN SEE IT WORKING ]               |",
        15, 0, VGA_LIGHT_GREEN
    );

    print_at_color(
        "|                         [ IDT SEEMS TO BE OK ]                               |",
        16, 0, VGA_LIGHT_GREEN
    );

    print_at_color(
        "|                         [ VGA IF U SEE THIS IT WORKS ]                       |",
        17, 0, VGA_LIGHT_GREEN
    );

    print_at_color(
        "|                                                                              |",
        18, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "|                                                                              |",
        19, 0, VGA_RED
    );

    print_at_color(
        "|                                                                              |",
        20, 0, VGA_YELLOW
    );

    print_at_color(
        "|                                                                              |",
        21, 0, VGA_LIGHT_BLUE
    );

    print_at_color(
        "+------------------------------------------------------------------------------+",
        22, 0, VGA_LIGHT_MAGENTA
    );

    print_at_color(
        " ",
        23, 0, VGA_RED
    );

    print_at_color(
        "                         PRISM OS v0.0.4",
        24, 0, VGA_LIGHT_GRAY
    );
}