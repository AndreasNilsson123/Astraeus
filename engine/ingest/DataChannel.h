#ifndef ASTRAEUS_DATA_CHANNEL_H
#define ASTRAEUS_DATA_CHANNEL_H

#include <cstdint>

namespace astraeus {

/**
 * DataChannel interface for receiving external simulation data.
 * This is a stub interface - actual implementations would handle
 * networking, file I/O, shared memory, etc.
 */
class DataChannel {
public:
    virtual ~DataChannel() = default;
    
    /**
     * Poll for new data.
     * @param buffer Output buffer for data
     * @param max_size Maximum bytes to read
     * @param bytes_read Output: actual bytes read
     * @return true if data was available
     */
    virtual bool poll(void* buffer, uint32_t max_size, uint32_t* bytes_read) = 0;
    
    /**
     * Check if channel has data available.
     */
    virtual bool has_data() const = 0;
    
    /**
     * Get channel name/identifier.
     */
    virtual const char* get_name() const = 0;
};

} // namespace astraeus

#endif // ASTRAEUS_DATA_CHANNEL_H
