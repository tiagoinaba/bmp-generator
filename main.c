#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_IMAGE_SIZE 1024
#define OUTPUT_FILENAME "resultado.bmp"

#pragma pack(push, 1)

// Bitmap File Header (14 bytes)
typedef struct {
        uint16_t bfType;
        uint32_t bfSize;      // Specifies the size in bytes of the bitmap file
        uint16_t bfReserved1; // Reserved; must be 0
        uint16_t bfReserved2; // Reserved; must be 0
        uint32_t bfOffBits; // Specifies the offset in bytes from the beginning
                            // of the file to the bitmap data
} BITMAPFILEHEADER;

// Bitmap Information Header (40 bytes)
typedef struct {
        uint32_t biSize; // Specifies the number of bytes required by the struct
        int32_t biWidth; // Specifies width of the bitmap in pixels
        int32_t biHeight;    // Specifies height of the bitmap in pixels
        uint16_t biPlanes;   // Specifies the number of color planes, must be 1
        uint16_t biBitCount; // Specifies the number of bits per pixel
        uint32_t biCompression;  // Specifies the type of compression
        uint32_t biSizeImage;    // Size of the image in bytes
        int32_t biXPelsPerMeter; // Number of pixels per meter in x axis
        int32_t biYPelsPerMeter; // Number of pixels per meter in y axis
        uint32_t biClrUsed;      // Number of colors used by the bitmap
        uint32_t biClrImportant; // Number of important colors
} BITMAPINFOHEADER;

#pragma pack(pop)

typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
} color;

typedef struct {
        color *bitmap;
        size_t cols;
        size_t rows;
} image;

void image_free(image *img) {
    if (img) {
        free(img->bitmap);
        free(img);
    }
}

const color colors[4] = {{255, 255, 0}, {0, 0, 0}, {0, 0, 255}, {0, 255, 0}};

image *parse_file(FILE *f) {
    char buffer[3];
    buffer[2] = '\0';
    size_t bytes_read;
    image *img = malloc(sizeof(image));
    if (!img)
        return NULL;
    img->bitmap = malloc(sizeof(color) * MAX_IMAGE_SIZE);
    if (!img->bitmap) {
        free(img);
        return NULL;
    }

    img->cols = 0;
    img->rows = 0;
    int pixel_count = 0;

    while ((bytes_read = fread(buffer, 1, 2, f)) == 2) {
        if (pixel_count > MAX_IMAGE_SIZE) {
            printf("Erro: Imagem excede o tamanho máximo de %d pixels.\n",
                   MAX_IMAGE_SIZE);
            image_free(img);
            return NULL;
        }
        char *endptr;
        int col = strtol(buffer, &endptr, 2);
        if (col < 0 || col > 3) {
            printf("cor inválida: %s!", buffer);
            image_free(img);
            return NULL;
        }

        img->bitmap[pixel_count] = colors[col];
        pixel_count++;

        char garbage = fgetc(f);
        if (garbage == '\n') {
            if (img->cols == 0) {
                img->cols = pixel_count;
            } else if (pixel_count % img->cols != 0) {
                printf("Erro: Linhas com larguras inconsistentes.\n");
                image_free(img);
                return NULL;
            }
        } else if (garbage == EOF) {
            break;
        }
    }

    if (img->cols == 0 && pixel_count > 0) {
        img->cols = pixel_count;
    }

    if (img->cols > 0) {
        img->rows = pixel_count / img->cols;
        if (pixel_count % img->cols != 0) {
            printf("Erro: O número total de pixels não é um múltiplo da "
                   "largura da linha.\n");
			image_free(img);
			return NULL;
        }
    }

    return img;
}

int image_generate_bmp(image *img) {
    FILE *f = fopen(OUTPUT_FILENAME, "wb");
    if (f == NULL) {
        printf("impossível gerar output\n");
        return 1;
    }
    // HEADERS
    const int bytes_per_pixel = 3;
    size_t row_size_unpadded = img->cols * bytes_per_pixel;
    int padding = (4 - (row_size_unpadded % 4)) % 4;
    const uint8_t padding_bytes[3] = {0};

    const uint32_t file_header_size = sizeof(BITMAPFILEHEADER);
    const uint32_t info_header_size = sizeof(BITMAPINFOHEADER);
    const uint32_t pixel_data_offset = file_header_size + info_header_size;
    const uint32_t image_size = (row_size_unpadded + padding) * img->rows;
    const uint32_t file_size = pixel_data_offset + image_size;

    BITMAPFILEHEADER file_header = {0};
    file_header.bfType = 0x4D42;
    file_header.bfSize = file_size;
    file_header.bfOffBits = pixel_data_offset;
    BITMAPINFOHEADER info_header = {0};
    info_header.biSize = info_header_size;
    info_header.biWidth = img->cols;
    info_header.biHeight = img->rows;
    info_header.biPlanes = 1;
    info_header.biBitCount = 24;   // (RGB) 8x8x8
    info_header.biCompression = 0; // BI_RGB
    info_header.biSizeImage = image_size;
    info_header.biXPelsPerMeter = 2835;
    info_header.biYPelsPerMeter = 2835;
    fwrite(&file_header, 1, file_header_size, f);
    fwrite(&info_header, 1, info_header_size, f);

    // IMAGEM
    for (int y = img->rows - 1; y >= 0; y--) {
        for (int x = 0; x < img->cols; ++x) {
            const color current = img->bitmap[y * img->cols + x];
            fwrite(&current.b, 1, 1, f);
            fwrite(&current.g, 1, 1, f);
            fwrite(&current.r, 1, 1, f);
        }
        if (padding > 0) {
            fwrite(padding_bytes, 1, padding, f);
        }
    }
    fclose(f);
    printf("arquivo BMP '%s' escrito com sucesso!\n", OUTPUT_FILENAME);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("é necessário especificar um arquivo!\ndraw <file>\n");
        return 1;
    }
    char *filename = argv[1];
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("falha na leitura do arquivo");
        return 1;
    }
    image *img = parse_file(f);
    if (img == NULL) {
        printf("falha no parse do arquivo\n");
        return 1;
    }
    fclose(f);
    const int return_code = image_generate_bmp(img);
    image_free(img);
    return return_code;
}
