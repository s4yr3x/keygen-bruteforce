// main.cpp
// made with love by sayrex
// github.com/s4yr3x
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
const int TARGET_HASH_1 = -889275714;
const int TARGET_HASH_2 = 1748706157;

// кодировка
const char CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int CHARSET_SIZE = 36;

std::mutex output_mutex;
std::atomic<int> found_count(0);
std::atomic<long long> total_tested(0);
std::atomic<bool> should_stop(false);

// высчитываем хеш дл€ заданного ключа (сделал дл€ повышени€ производительности)
inline int calculate_hash(const char* key, int length) {
	int hash = 305419896;
	for (int i = 0; i < length; i++) {
		hash = ((key[i] + 131 * hash) ^ 0x55555555);
	}
	return hash;
}

// генерим выходную строку дл€ правильного ключа
std::string generate_output(const char* key, int key_size) {
	int hash = calculate_hash(key, key_size);

	unsigned int seed = hash ^ 0x12345678;
	seed = (seed << 3) | (seed >> 29); // ROR 29
	unsigned int prng_state = 1103515245 * seed - 1082068167;

	int length = (prng_state % 33) + 10;

	std::string output;
	output.reserve(length);
	for (int i = 0; i < length; i++) {
		prng_state = 1103515245 * prng_state + 12345;
		output += ALPHABET[(prng_state >> 8) & 0x1F];
	}

	return output;
}

// индекс в key строку
void index_to_key(long long index, char* key, int len) {
	key[0] = 'K';
	key[1] = 'e';
	key[len] = '\0';

	for (int i = len - 1; i >= 2; i--) {
		key[i] = CHARSET[index % CHARSET_SIZE];
		index /= CHARSET_SIZE;
	}
}

// функци€ потока workers
void search_worker(int len, long long start_idx, long long end_idx, int max_keys, int thread_id) {
	char key[64];
	long long local_tested = 0;

	for (long long idx = start_idx; idx < end_idx && !should_stop; idx++) {
		index_to_key(idx, key, len);

		int hash = calculate_hash(key, len);

		if (hash == TARGET_HASH_1 || hash == TARGET_HASH_2) {
			if (found_count >= max_keys) {
				should_stop = true;
				break;
			}

			std::string output = generate_output(key, len);

			{
				std::lock_guard<std::mutex> lock(output_mutex);
				std::cout << "[FOUND #" << (found_count + 1) << "] Key: " << key
					<< " -> Output: " << output << std::endl;
			}

			found_count++;
		}

		local_tested++;
		if (local_tested % 50000 == 0) {
			total_tested += 50000;
			local_tested = 0;
		}
	}

	total_tested += local_tested;
}

// поток, в котором мы получаем всю информацию в красивом виде
void progress_monitor(long long total_combinations, int len) {
	auto start_time = std::chrono::steady_clock::now();

	while (!should_stop) {
		std::this_thread::sleep_for(std::chrono::seconds(2));

		long long tested = total_tested.load();
		auto current_time = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

		if (elapsed > 0 && tested > 0) {
			double percent = (tested * 100.0) / total_combinations;
			long long rate = tested / elapsed;
			long long remaining = (total_combinations - tested) / (rate > 0 ? rate : 1);

			std::lock_guard<std::mutex> lock(output_mutex);
			std::cout << "[Length " << len << "] Progress: " << percent << "% | "
				<< "Tested: " << tested << "/" << total_combinations << " | "
				<< "Rate: " << rate << " keys/s | "
				<< "ETA: " << remaining << "s | "
				<< "Found: " << found_count << std::endl;
		}
	}
}

// основной поток брута ключей
void brute_force_keys_mt(int min_length, int max_length, int max_keys_to_find, int num_threads) {
	std::cout << "Searching for valid keys starting with 'Ke'..." << std::endl;
	std::cout << "Length range: " << min_length << "-" << max_length << std::endl;
	std::cout << "Target hashes: " << TARGET_HASH_1 << " or " << TARGET_HASH_2 << std::endl;
	std::cout << "Threads: " << num_threads << std::endl;
	std::cout << "Charset: " << CHARSET << " (" << CHARSET_SIZE << " chars)" << std::endl;
	std::cout << std::endl;

	for (int len = min_length; len <= max_length && found_count < max_keys_to_find; len++) {
		long long total_combinations = 1;
		for (int i = 2; i < len; i++) {
			total_combinations *= CHARSET_SIZE;
		}

		std::cout << "=== Testing length " << len << " ===" << std::endl;
		std::cout << "Total combinations: " << total_combinations << std::endl;

		should_stop = false;
		total_tested = 0;

		// создаем поток» workers
		std::vector<std::thread> workers;
		long long chunk_size = total_combinations / num_threads;

		// начинаем чекать
		std::thread monitor(progress_monitor, total_combinations, len);

		auto start_time = std::chrono::steady_clock::now();

		for (int i = 0; i < num_threads; i++) {
			long long start_idx = i * chunk_size;
			long long end_idx = (i == num_threads - 1) ? total_combinations : (i + 1) * chunk_size;

			workers.emplace_back(search_worker, len, start_idx, end_idx, max_keys_to_find, i);
		}

		// ждем все потоки
		for (auto& worker : workers) {
			worker.join();
		}

		should_stop = true;
		monitor.join();

		auto end_time = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

		std::cout << "Length " << len << " completed in " << elapsed << " seconds" << std::endl;
		std::cout << "Tested: " << total_tested << " combinations" << std::endl;
		std::cout << std::endl;

		if (found_count >= max_keys_to_find) {
			std::cout << "Reached maximum keys target (" << max_keys_to_find << "). Stopping." << std::endl;
			break;
		}
	}

	std::cout << "=== Search Complete ===" << std::endl;
	std::cout << "Total valid keys found: " << found_count << std::endl;
}

int main() {
	std::cout << "=== Multi-Threaded Key Brute Force Tool ===" << std::endl << std::endl;

	// получаем количество потоков
	int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) num_threads = 4; // если не сработало

	std::cout << "Detected " << num_threads << " CPU threads" << std::endl << std::endl;

	// ищем ключи
	// начинаем с коротких и постепенно увеличиваем
	brute_force_keys_mt(6, 10, 10, num_threads);

	// советики
	std::cout << std::endl << "=== Tips ===" << std::endl;
	std::cout << "- Shorter keys are found much faster" << std::endl;
	std::cout << "- Length 6-8: seconds to minutes" << std::endl;
	std::cout << "- Length 9-10: minutes to hours" << std::endl;
	std::cout << "- Length 11+: hours to days" << std::endl;

	return 0;
}