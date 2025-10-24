# Crackme Hash-Based Keygen

Multi-threaded bruteforce tool for finding valid keys in hash-based key validation systems.

## 🎯 Overview

Initially, this tool was developed personally for me, but I got the idea to put it on github. It performs efficient multi-threaded brute-force search to find keys that produce target hash values.

### How it works:
- Keys must start with `Ke` prefix
- Remaining characters bruteforced from charset: `A-Z0-9`
- Two target hash values: `-889275714` and `1748706157`
- Custom hash algorithm implementation
- PRNG-based output generation for valid keys

## ⚡ Features

- **Multi-threaded**: Automatically detects CPU cores, uses them to speed up bruteforce
- **Progress monitoring**: Real-time stats (made it personally for myself, but it's very convenient and looks nice)
- **Configurable length range**: Search keys from 6 to 10+ characters (but 10 keys will be found before keys with 9 characters even start :) )
- **Fast hash calculation**: Optimized inline hash function
- **Early termination**: Stops when target number of keys found (10)

## 🚀 Usage

```bash
./keygen_bruteforce
```

The tool will:
1. Detect available CPU threads
2. Search keys starting from length 6 up to 10
3. Display progress every 2 seconds
4. Stop after finding 10 valid keys (configurable)

### Example Output:
```
[FOUND #1] Key: KeABC123 -> Output: X7Y9Z2A...
[Length 6] Progress: 15.2% | Tested: 1234567/8000000 | Rate: 250000 keys/s | ETA: 27s
```

## ⚙️ Configuration

Edit `main.cpp` to modify:
- `TARGET_HASH_1`, `TARGET_HASH_2`: Target hash values
- `brute_force_keys_mt(6, 10, 10, num_threads)`: (min_len, max_len, max_keys, threads)
- `CHARSET`: Character set for bruteforce

## 📊 Performance

| Length | Combinations | Typical Time (8 cores) |
|--------|--------------|------------------------|
| 6      | 2.1B         | ~2-5 minutes          |
| 7      | 78B          | ~1-2 hours            |
| 8      | 2.8T         | ~1-2 days             |
| 9+     | 100T+        | days-weeks            |

👍 Also want to express my support and thanks to `antilagvip`, creator of this crackme. It was an interesting challenge, thanks!
crackme: `https://crackmes.one/crackme/68e6377b2d267f28f69b7447`
