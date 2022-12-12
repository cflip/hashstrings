#include <stdio.h>
#include <math.h>

#define STRING_LENGTH 4

const int n = 26;
const int num_elements = pow(n, STRING_LENGTH);
const int threads_per_block = 256;

__device__ long compute_hash_code(const char* str, int n)
{
	long result = 0;
	for (int i = 0; i < n; i++)
		result = 31 * result + str[i];
	return result;
}

__global__ void find_from_generated_strings(long* hash_to_find, char* result_str,
		int total_elements)
{
	char str[STRING_LENGTH + 1];

	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if (i >= total_elements)
		return;

	for (int j = 0; j < STRING_LENGTH; j++) {
		int string_index = STRING_LENGTH - (j + 1);
		int char_index = (i / (int)pow(n, j)) % n;
		str[string_index] = 'a' + char_index;
	}

	if (compute_hash_code(str, STRING_LENGTH) == *hash_to_find) {
		memcpy(result_str, str, STRING_LENGTH);
		return;
	}
}

int main()
{
	long hash_to_find = 3446974;
	char result[STRING_LENGTH + 1];

	long* to_find_on_device;
	char* result_string_on_device;

	cudaMalloc(&to_find_on_device, sizeof(long));
	cudaMalloc(&result_string_on_device, STRING_LENGTH + 1);

	cudaMemcpy(to_find_on_device, &hash_to_find, sizeof(int),
			cudaMemcpyHostToDevice);

	const int num_blocks = (num_elements + threads_per_block - 1) /
		threads_per_block;
	printf("We have %d items to compute\n", num_elements);
	printf("Running with %d blocks and %d threads per block\n", num_blocks,
			threads_per_block);
	find_from_generated_strings<<<num_blocks, threads_per_block>>>(to_find_on_device,
			result_string_on_device, num_elements);

	cudaMemcpy(result, result_string_on_device, STRING_LENGTH + 1, cudaMemcpyDeviceToHost);

	printf("%s\n", result);

	cudaFree(to_find_on_device);
	cudaFree(result_string_on_device);

	return 0;
}
