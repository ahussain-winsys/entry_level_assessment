#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>

#define INPUT_COUNT 4
#define OUTPUT_COUNT 4
#define CHIP_NAME "gpiochip0" // Adjust this to match your GPIO chip

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *input_lines[INPUT_COUNT];
    struct gpiod_line *output_lines[OUTPUT_COUNT];
    int gpio_inputs[INPUT_COUNT] = {4, 5, 6, 7};  // GPIO lines for inputs
    int gpio_outputs[OUTPUT_COUNT] = {0, 1, 2, 3}; // GPIO lines for outputs
    int i, j, value;
    int status = 1; // Assume pass

    // Open the GPIO chip
    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }

    // Get input lines
    for (i = 0; i < INPUT_COUNT; i++) {
        input_lines[i] = gpiod_chip_get_line(chip, gpio_inputs[i]);
        if (!input_lines[i] || gpiod_line_request_input(input_lines[i], "gpio_test") < 0) {
            perror("Failed to request input line");
            gpiod_chip_close(chip);
            return 1;
        }
    }

    // Get output lines
    for (i = 0; i < OUTPUT_COUNT; i++) {
        output_lines[i] = gpiod_chip_get_line(chip, gpio_outputs[i]);
        if (!output_lines[i] || gpiod_line_request_output(output_lines[i], "gpio_test", 0) < 0) {
            perror("Failed to request output line");
            gpiod_chip_close(chip);
            return 1;
        }
    }

    // Main loop
    for (i = 0; i < (1 << OUTPUT_COUNT); i++) {
        int val[OUTPUT_COUNT];
        for (j = 0; j < OUTPUT_COUNT; j++) {
            val[j] = (i >> j) & 1; // Get the j-th bit
        }

        printf("OUT\t");
        for (j = 0; j < OUTPUT_COUNT; j++) {
            printf("%d ", val[j]);
            if (gpiod_line_set_value(output_lines[j], val[j]) < 0) {
                perror("Failed to set output line");
                status = 0;
                break;
            }
        }
        printf("\n");

        if (!status) break;

        sleep(1); // Brief delay to ensure values are set

        printf("IN\t");
        for (j = 0; j < INPUT_COUNT; j++) {
            value = gpiod_line_get_value(input_lines[j]);
            if (value < 0) {
                perror("Failed to read input line");
                status = 0;
                break;
            }
            printf("%d ", value);
        }
        printf("\n");

        if (!status) break;

        // Check if input values match output values
        int match = 1;
        for (j = 0; j < OUTPUT_COUNT; j++) {
            if (val[j] != (gpiod_line_get_value(input_lines[j]) == 1)) {
                match = 0;
                break;
            }
        }

        if (!match) {
            status = 0;
            break;
        }
    }

    // Report the final status
    if (status) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }

    // Cleanup
    for (i = 0; i < INPUT_COUNT; i++) {
        gpiod_line_release(input_lines[i]);
    }
    for (i = 0; i < OUTPUT_COUNT; i++) {
        gpiod_line_release(output_lines[i]);
    }
    gpiod_chip_close(chip);

    return status ? 0 : 1;
}