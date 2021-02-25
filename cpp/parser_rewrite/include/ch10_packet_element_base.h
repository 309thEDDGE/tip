
#ifndef CH10_PACKET_ELEMENT_BASE_H_
#define CH10_PACKET_ELEMENT_BASE_H_

#include <cstddef>
#include <cstdint>

class Ch10PacketElementBase
{
    protected:
    size_t size_;

    public:
    const size_t& size = size_;
    Ch10PacketElementBase(size_t size) : size_(size) {}
    virtual void Set(const uint8_t* data) = 0;

};

#endif