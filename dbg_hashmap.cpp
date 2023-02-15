#include <iostream>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>


struct DbgFnInfo {
    size_t addr;
    size_t size;

    // other debug info
    // ....
};


template<size_t TOP_BIT_NUMBER = 8>
class DbgFnInfoTable {
    static const size_t CHUNK_SIZE = 1LU << TOP_BIT_NUMBER;
    static const size_t BUTTOM_ADDR_MASK = CHUNK_SIZE - 1;
    static const size_t TOP_ADDR_MASK = ~BUTTOM_ADDR_MASK;

public:
    void add(std::unique_ptr<DbgFnInfo> dbg_fn_info) {
        size_t fn_size = dbg_fn_info->size;
        size_t fn_addr_b = dbg_fn_info->addr;
        size_t fn_addr_e = fn_addr_b + fn_size;

        for (size_t hash_addr = fn_addr_b & TOP_ADDR_MASK; hash_addr < fn_addr_e; hash_addr += CHUNK_SIZE) {
            Key k {hash_addr, fn_addr_b, fn_size};
            hash_table_[k] = dbg_fn_info.get();
        }

        // Save dbg_fn_info
        holder_dbg_fn_info_.push_back(std::move(dbg_fn_info));
    }

    DbgFnInfo *find(size_t fn_addr) {
        size_t hash_addr = fn_addr & TOP_ADDR_MASK;
        Key k {hash_addr, fn_addr, Key::FIND_SZ};
        auto it = hash_table_.find(k);
        if (it != hash_table_.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    struct Key {
        size_t hash_addr;
        size_t fn_addr;
        size_t fn_size;

        static const size_t FIND_SZ = std::numeric_limits<size_t>::max();
        bool is_find() const {
            return fn_size == FIND_SZ;
        }
    };

    class Hasher {
    public:
        std::size_t operator()(const Key &k) const {
            return std::hash<size_t>{}(k.hash_addr);
        }
    };

    class EqualFn
    {
    public:
        bool operator() (const Key &k0, const Key &k1) const
        {
            if (k0.is_find()) {
                return k0.fn_addr >= k1.fn_addr && k0.fn_addr < (k1.fn_addr + k1.fn_size);
            } else if (k1.is_find()) {
                return k1.fn_addr >= k0.fn_addr && k1.fn_addr < (k0.fn_addr + k0.fn_size);
            }
            return k0.fn_addr == k1.fn_addr;
        }
    };

    std::vector<std::unique_ptr<DbgFnInfo>> holder_dbg_fn_info_;
    std::unordered_map<Key, DbgFnInfo *, Hasher, EqualFn> hash_table_;
};
