char* load(char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file)
        return 0;
    fseek(file, 0, SEEK_END);
    unsigned int length = (unsigned int)ftell(file);
    rewind(file);
    char* buffer = malloc(length + 1);
    fread(buffer, length, 1, file);
    fclose(file);
    buffer[length] = '\0';
    return buffer;
}

unsigned int size(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file)
        return 0;
    fseek(file, 0, SEEK_END);
    unsigned int length = (unsigned int)ftell(file);
    fclose(file);
    return length;
}

void save(char* filename, void* buffer, int length) {
    FILE *file = fopen(filename, "wb");
    if(file) {
        fwrite(buffer, length, 1, file);
        fclose(file);
    }
}
