#include <iostream>

#include "vv-aes.h"
//#include <chrono>

void create_substitution_lut(uint8_t* lut) {
    for (int i = 0; i < UNIQUE_CHARACTERS; i++) {
        lut[originalCharacter[i]] = substitutedCharacter[i];
    }
}

void substitute_bytes(const uint8_t* lut) {
    for (int column = 0; column < BLOCK_SIZE; ++column) {
        for (int row = 0; row < BLOCK_SIZE; ++row) {
            message[row][column] = lut[message[row][column]];
        }
    }
}

/*
 * This function shifts (rotates) a row in the message array by one place to the left.
 * @param row The row which to shift.
 */
void shift_row(int row) {
    uint8_t first = message[row][0];

    for (int i = 0; i < BLOCK_SIZE - 1; ++i) {
        message[row][i] = message[row][i + 1];
    }
    message[row][BLOCK_SIZE - 1] = first;
}

/*
 * This function shifts each row by the number of places it is meant to be shifted according to the AES specification.
 * Row zero is shifted by zero places. Row one by one, etc.
 * This corresponds to step 2.2 in the VV-AES explanation.
 */
void shift_rows() {
    // Shift each row, where the row index corresponds to how many columns the data is shifted.
    for (int row = 0; row < BLOCK_SIZE; ++row) {
        for (int shifts = 0; shifts < row; ++shifts) {
            shift_row(row);
        }
    }
}

/*
 * This function creates a 256x9 array where powers[x][n] gives x^n.
 */
void precompute_powers(uint8_t powers[UNIQUE_CHARACTERS][BLOCK_SIZE+1]) {
    for (int x = 0; x < UNIQUE_CHARACTERS; ++x) {
        powers[x][0] = 1;  // x^0 is always 1
        for (int n = 1; n <= BLOCK_SIZE; ++n) {
            powers[x][n] = powers[x][n-1] * x;
        }
    }
}

/*
 * This function calculates x^n for polynomial evaluation.
 */
int power(int x, int n) {
    // Calculates x^n
    if (n == 0) {
        return 1;
    }
    return x * power(x, n - 1);
}

/*
 * This function evaluates four different polynomials, one for each row in the column.
 * Each polynomial evaluated is of the form
 * m'[row, column] = c[r][3] m[3][column]^4 + c[r][2] m[2][column]^3 + c[r][1] m[1][column]^2 + c[r][0]m[0][column]^1
 * where m' is the new message value, c[r] is an array of polynomial coefficients for the current result row (each
 * result row gets a different polynomial), and m is the current message value.
 *
 */
void multiply_with_polynomial(int column, uint8_t powers[UNIQUE_CHARACTERS][BLOCK_SIZE+1]) {
    for (int row = 0; row < BLOCK_SIZE; ++row) {
        int result = 0;
        for (int degree = 0; degree < BLOCK_SIZE; degree++) {
            result += polynomialCoefficients[row][degree] * powers[message[degree][column]][degree + 1];
        }
        message[row][column] = result;
    }
}

/*
 * For each column, mix the values by evaluating them as parameters of multiple polynomials.
 * This corresponds to step 2.3 in the VV-AES explanation.
 */
void mix_columns(uint8_t powers[UNIQUE_CHARACTERS][BLOCK_SIZE+1]) {
    for (int column = 0; column < BLOCK_SIZE; ++column) {
        multiply_with_polynomial(column, powers);
    }
}

/*
 * Add the current key to the message using the XOR operation.
 */
void add_key() {
    for (int column = 0; column < BLOCK_SIZE; column++) {
        for (int row = 0; row < BLOCK_SIZE; ++row) {
            // ^ == XOR
            message[row][column] = message[row][column] ^ key[row][column];
        }
    }
}

/*
 * Your main encryption routine.
 */
int main() {
    uint8_t lut[UNIQUE_CHARACTERS];
    uint8_t powers[UNIQUE_CHARACTERS][BLOCK_SIZE+1];

    // Receive the problem from the system.
    readInput();

//    std::chrono::high_resolution_clock::time_point start, stop;
//    start = std::chrono::high_resolution_clock::now();

    precompute_powers(powers);
    create_substitution_lut(lut);
    // For extra security (and because Varys wasn't able to find enough test messages to keep you occupied) each message
    // is put through VV-AES lots of times. If we can't stop the adverse Maesters from decrypting our highly secure
    // encryption scheme, we can at least slow them down.
    for (int i = 0; i < ITERATIONS; i++) {
        // For each message, we use a predetermined key (e.g. the password). In our case, its just pseudo random.
        set_next_key();

        // First, we add the key to the message once:
        add_key();

        // We do 9+1 rounds for 128 bit keys
        for (int round = 0; round < ROUNDS; round++) {
            //In each round, we use a different key derived from the original (refer to the key schedule).
            set_next_key();

            // These are the four steps described in the slides.
            substitute_bytes(lut);
            shift_rows();
            mix_columns(powers);
            add_key();
        }
        // Final round
        substitute_bytes(lut);
        shift_rows();
        add_key();
    }

    // Submit our solution back to the system.
    writeOutput();
//    stop = std::chrono::high_resolution_clock::now();
//    int time_in_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
//    std::cout << std::dec << "Operations executed in " << time_in_microseconds << " microseconds" << std::endl;

    return EXIT_SUCCESS;
}
