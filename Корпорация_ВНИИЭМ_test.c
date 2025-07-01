#include <stdio.h>
#include <stdlib.h>

int main() {

    FILE *input_file, *output_file;
    unsigned char byte;
    int channel0, channel1, channel_and;
    int count = 0;

    // Открываем входной бинарный файл
    input_file = fopen("udk_dump.bin", "rb");

    if (input_file == NULL) {
        perror("Ошибка открытия файла udk_dump.bin");
        return EXIT_FAILURE;
    }

    // Открываем выходной CSV файл
    output_file = fopen("output.csv", "w");
    if (output_file == NULL) {
        perror("Ошибка создания файла output.csv");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    // Записываем заголовок CSV
    fprintf(output_file, "Channel_0,Channel_1,Channel_0 & Channel_1\n");

    // Читаем файл побайтно и обрабатываем 
    while (fread(&byte, sizeof(unsigned char), 1, input_file) == 1) {

        // Извлекаем нулевой и первый бит
        channel0 = (byte >> 0) & 1; 
        channel1 = (byte >> 1) & 1;
        
        channel_and = channel0 & channel1;
        
        // Записываем данные в CSV
        fprintf(output_file, "%d,%d,%d\n", channel0, channel1, channel_and);
        count++;
    }

    // Закрываем файлы
    fclose(input_file);
    fclose(output_file);

    printf("Обработано %d отсчетов. Результат сохранен в output.csv\n", count);

    return EXIT_SUCCESS;
}
