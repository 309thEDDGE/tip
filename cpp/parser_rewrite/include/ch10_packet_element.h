
#ifndef CH10_PACKET_ELEMENT_H_
#define CH10_PACKET_ELEMENT_H_

#include "ch10_packet_element_base.h"

template<class T>
class Ch10PacketElement : public Ch10PacketElementBase
{
    private:
    const T* element_;

    public: 
    const T* const * const element = &element_;
    Ch10PacketElement() : Ch10PacketElementBase(sizeof(T)), element_(nullptr) {}
    void Set(const uint8_t* data) override;

};

template<class T>
void Ch10PacketElement<T>::Set(const uint8_t* data)
{
    // Set the address of the element to the current data address.
    element_ = (const T*)data;
}

#endif