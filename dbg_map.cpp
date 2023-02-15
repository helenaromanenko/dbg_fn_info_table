#include <iostream>
#include <map>

struct DbgFnInfo {
    size_t addr;
    size_t size;

    // other debug info
    // ...
};


class DbgFnInfoTable {
public:
    void add(std::unique_ptr<DbgFnInfo> dbg_fn_info) {
        map_[dbg_fn_info->addr] = std::move(dbg_fn_info);
    }

    DbgFnInfo *find(size_t addr) {
        if (map_.empty()) {
            return nullptr;
        }
        auto it = map_.upper_bound(addr);
        --it;
        if (it != map_.end()) {
            if (addr >= it->second->addr && addr < (it->second->addr + it->second->size)) {
                return it->second.get();
            }
        }
        return nullptr;
    }

private:
    std::map<size_t, std::unique_ptr<DbgFnInfo>> map_;
};
