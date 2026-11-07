#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE (sizeof(CustomFormat) - sizeof(uint8_t*))
#define MAGIC 0xDEADBEEFDEADBEEF

// high-level specification of our custom format, that will need to be written out in the file
typedef struct CustomFormat {
	struct {
		uint64_t magic; // if this value is here, its probably fine?
		uint32_t sem_ver;
		uint32_t data_length; // arbitrary, under 32 bit sized - 4 GB addressible max
		uint8_t padding[64 - 2*sizeof(uint32_t) - sizeof(uint64_t)];
	}; // Reserved header space: 64 bytes
	// start of data
	uint8_t* data;
} CustomFormat;

void serialize(uint8_t* data, uint32_t data_length, const char* out_file_name) {
	CustomFormat cf;
	memset(&cf, 0, sizeof(CustomFormat)); // zero out the entire data beforehand
	cf.magic = MAGIC;
	cf.sem_ver = 0b00000100;
	cf.data_length = data_length;
	// padding was memset to 0

	FILE* file = fopen(out_file_name, "w"); // write
	uint64_t written = fwrite(&cf, HEADER_SIZE, 1, file);
	written = fwrite(data, 1, data_length, file);
	fclose(file);
}

int deserialize(const char* file_path, CustomFormat* out_cf) {
	// we aren't checking the file extension (alot of programs don't either)
	FILE* file = fopen(file_path, "rb"); // read, binary
	if (file == NULL)
		return -1;

	fread(out_cf, 1, HEADER_SIZE, file);
	if (out_cf->magic != MAGIC) // not expected value, abort
		return -1;
	out_cf->data = malloc(out_cf->data_length); // allocate data for our memory
	fread(out_cf->data, 1, out_cf->data_length, file);
	fclose(file);

	return 0;
}


int main(int argc, const char* argv[]) {
	serialize("abcd", sizeof("abcd"), "out.bft");
	CustomFormat loaded;
	int result = deserialize("out.bft", &loaded);
	if (result != 0) {
		printf("failed to read custom format file!");
		return -1;
	}

	// read long hex, unsigned int, char string
	printf("Format data: magic: %llx, len: %u, data: %s\n", loaded.magic, loaded.data_length, loaded.data);
	free(loaded.data);

	return 0;
}