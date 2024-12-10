#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define VOLUME_SIZE 128
#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 512
#define MAX_STEPS 256
#define STEP_SIZE 0.01

float volume[VOLUME_SIZE][VOLUME_SIZE][VOLUME_SIZE];

struct Point {
    float x, y, z;
};

int euclidian_distance(struct Point x, struct Point y) {
    return sqrt(pow(x.x - y.x, 2) + pow(x.y - y.y, 2) + pow(x.z - y.z, 2));
}

float sample_volume(float x, float y, float z) {
    int ix = (int)(x * VOLUME_SIZE);
    int iy = (int)(y * VOLUME_SIZE);
    int iz = (int)(z * VOLUME_SIZE);

    if (ix < 0 || iy < 0 || iz < 0 || ix >= VOLUME_SIZE || iy >= VOLUME_SIZE || iz >= VOLUME_SIZE)
        return 0.0f;

    return volume[ix][iy][iz];
}

float ray_march(float start_x, float start_y, float start_z, float dir_x, float dir_y, float dir_z) {
    float pos_x = start_x, pos_y = start_y, pos_z = start_z;
    float density = 0.0f;

    for (int step = 0; step < MAX_STEPS; ++step) {
        density += sample_volume(pos_x, pos_y, pos_z) * STEP_SIZE;

        pos_x += dir_x * STEP_SIZE;
        pos_y += dir_y * STEP_SIZE;
        pos_z += dir_z * STEP_SIZE;

        if (pos_x < 0 || pos_x >= VOLUME_SIZE || pos_y < 0 || pos_y >= VOLUME_SIZE || pos_z < 0 || pos_z >= VOLUME_SIZE) {
            break;
        }
    }

    density = fmin(density, 1.0f);

    return density;
}

void test_ray_march() {
    float density = ray_march(0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f);
    printf("Ray march density: %f\n", density);
}

void render_image(unsigned char *image) {
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            float nx = x / (float)IMAGE_WIDTH;
            float ny = y / (float)IMAGE_HEIGHT;

            float ray_dir_x = nx - 0.5f; // Map to [-0.5, 0.5]
            float ray_dir_y = ny - 0.5f;
            float ray_dir_z = 1.0f;      // Forward direction

            float length = sqrt(ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y + ray_dir_z * ray_dir_z);
            ray_dir_x /= length;
            ray_dir_y /= length;
            ray_dir_z /= length;

            float density = ray_march(0.5f, 0.3f, 0.0f, ray_dir_x, ray_dir_y, ray_dir_z);

            // Map density to grayscale
            unsigned char color = (unsigned char)(density * 255);
            int index = (y * IMAGE_WIDTH + x) * 3;
            image[index] = color;       // R
            image[index + 1] = color;   // G
            image[index + 2] = color;   // B
        }
    }
}

void initialize_volume() {
    int center_x = VOLUME_SIZE / 2;
    int center_y = VOLUME_SIZE / 2;
    int center_z = VOLUME_SIZE / 2;
    int radius = VOLUME_SIZE / 4;

    for (int x = 0; x < VOLUME_SIZE; ++x) {
        for (int y = 0; y < VOLUME_SIZE; ++y) {
            for (int z = 0; z < VOLUME_SIZE; ++z) {
                struct Point p1 = {x, y, z};
                struct Point p2 = {center_x, center_y, center_z};
                float dist = euclidian_distance(p1, p2);
                volume[x][y][z] = (dist <= radius) ? 1.0f : 0.0f;

            }
        }
    }
}

void print_volume_slice(int z) {
    printf("Slice at z=%d:\n", z);
    for (int y = 0; y < VOLUME_SIZE; y++) {
        for (int x = 0; x < VOLUME_SIZE; x++) {
            printf("%c", volume[x][y][z] > 0.0f ? '#' : '.');
        }
        printf("\n");
    }
}

void save_image(const char *filename, unsigned char *image) {
    FILE *f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);
    fwrite(image, 1, IMAGE_WIDTH * IMAGE_HEIGHT * 3, f);
    fclose(f);
}

void test() {
    unsigned char *image = (unsigned char *)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    // Fill image with gradient
    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        for (int x = 0; x < IMAGE_HEIGHT; ++x) {
            int index = (y * IMAGE_WIDTH + x) * 3;
            image[index] = (unsigned char)(x / (float)IMAGE_WIDTH * 255); // red
            image[index + 1] = (unsigned char)(x / (float)IMAGE_HEIGHT * 255); // green
            image[index + 2] = 128; // blue
        }
    }

    save_image("gradient.ppm", image);

    print_volume_slice(VOLUME_SIZE / 2 - 10);
    print_volume_slice(VOLUME_SIZE / 2);
    print_volume_slice(VOLUME_SIZE / 2 + 10);

    free(image);
}

int main() {
    initialize_volume();

    unsigned char *image = (unsigned char *)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3);
    render_image(image);
    save_image("output.ppm", image);

    free(image);

    printf("Image saved as output.ppm\n");
    return 0;
}