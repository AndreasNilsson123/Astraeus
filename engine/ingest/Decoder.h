#ifndef ASTRAEUS_DECODER_H
#define ASTRAEUS_DECODER_H

#include <cstdint>

namespace astraeus {

class SnapshotStore;
class TimeSync;

/**
 * Decoder interface for converting external data formats into engine snapshots.
 * Implementations handle specific binary/text formats (Fixed Binary, JSON, Protobuf, etc.).
 */
class Decoder {
public:
    virtual ~Decoder() = default;
    
    /**
     * Decode data and write to snapshot store.
     * @param data Raw data bytes
     * @param size Size in bytes
     * @param store Target snapshot store
     * @param time_sync Time synchronization manager
     * @return true on success
     */
    virtual bool decode(const void* data, uint32_t size, 
                       SnapshotStore* store, TimeSync* time_sync) = 0;
    
    /**
     * Get decoder name.
     */
    virtual const char* get_name() const = 0;
    
    /**
     * Validate data format (quick check before full decode).
     */
    virtual bool validate(const void* data, uint32_t size) const = 0;
};

} // namespace astraeus

#endif // ASTRAEUS_DECODER_H
