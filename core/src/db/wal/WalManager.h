// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License.

#pragma once

#include <atomic>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "WalBuffer.h"
#include "WalDefinations.h"
#include "WalFileHandler.h"
#include "WalMetaHandler.h"
#include "utils/Error.h"

namespace milvus {
namespace engine {
namespace wal {

class WalManager {
 public:
    explicit WalManager(const MXLogConfiguration& config);
    ~WalManager();

    /*
     * init
     * @param meta
     * @retval error_code
     */
    ErrorCode
    Init(const meta::MetaPtr& meta);

    /*
     * Get next recovery
     * @param record[out]: record
     * @retval error_code
     */
    ErrorCode
    GetNextRecovery(MXLogRecord& record);

    /*
     * Get next record
     * @param record[out]: record
     * @retval error_code
     */
    ErrorCode
    GetNextRecord(MXLogRecord& record);

    /*
     * Create collection
     * @param collection_id: collection id
     * @retval lsn
     */
    uint64_t
    CreateCollection(const std::string& collection_id);

    /*
     * Create partition
     * @param collection_id: collection id
     * @param partition_tag: partition tag
     * @retval lsn
     */
    uint64_t
    CreatePartition(const std::string& collection_id, const std::string& partition_tag);

    /*
     * Drop collection
     * @param collection_id: collection id
     * @retval none
     */
    void
    DropCollection(const std::string& collection_id);

    /*
     * Drop partition
     * @param collection_id: collection id
     * @param partition_tag: partition tag
     * @retval none
     */
    void
    DropPartition(const std::string& collection_id, const std::string& partition_tag);

    /*
     * Collection is flushed (update flushed_lsn)
     * @param collection_id: collection id
     * @param lsn: flushed lsn
     */
    void
    CollectionFlushed(const std::string& collection_id, uint64_t lsn);

    /*
     * Collection is updated (update wal_lsn)
     * @param collection_id: collection id
     * @param partition_tag: partition_tag
     * @param lsn: flushed lsn
     */
    void
    CollectionUpdated(const std::string& collection_id, uint64_t lsn);

    /*
     * Insert
     * @param collection_id: collection id
     * @param collection_id: partition tag
     * @param vector_ids: vector ids
     * @param vectors: vectors
     */
    template <typename T>
    bool
    Insert(const std::string& collection_id, const std::string& partition_tag, const IDNumbers& vector_ids,
           const std::vector<T>& vectors);

    /*
     * Insert
     * @param collection_id: collection id
     * @param partition_tag: partition tag
     * @param vector_ids: vector ids
     */
    bool
    DeleteById(const std::string& collection_id, const std::string& partition_tag, const IDNumbers& vector_ids);

    /*
     * Get flush lsn
     * @param collection_id: collection id (empty means all tables)
     * @retval if there is something not flushed, return lsn;
     *         else, return 0
     */
    uint64_t
    Flush(const std::string& collection_id = "");

    void
    RemoveOldFiles(uint64_t flushed_lsn);

    /*
     * Get the LSN of the last inserting or deleting operation
     * @retval lsn
     */
    uint64_t
    GetLastAppliedLsn() {
        return last_applied_lsn_;
    }

 private:
    WalManager
    operator=(WalManager&);

    MXLogConfiguration mxlog_config_;

    MXLogBufferPtr p_buffer_;
    MXLogMetaHandlerPtr p_meta_handler_;

    struct CollectionLsn {
        uint64_t flush_lsn;
        uint64_t wal_lsn;
        std::map<std::string, uint64_t> part_create_lsn;
    };
    std::mutex mutex_;
    std::map<std::string, CollectionLsn> collections_;
    std::atomic<uint64_t> last_applied_lsn_;

    // if multi-thread call Flush(), use list
    struct FlushInfo {
        std::string collection_id_;
        uint64_t lsn_ = 0;

        bool
        IsValid() {
            return (lsn_ != 0);
        }
        void
        Clear() {
            lsn_ = 0;
        }
    };
    FlushInfo flush_info_;
};

extern template bool
WalManager::Insert<float>(const std::string& collection_id, const std::string& partition_tag,
                          const IDNumbers& vector_ids, const std::vector<float>& vectors);

extern template bool
WalManager::Insert<uint8_t>(const std::string& collection_id, const std::string& partition_tag,
                            const IDNumbers& vector_ids, const std::vector<uint8_t>& vectors);

}  // namespace wal
}  // namespace engine
}  // namespace milvus
