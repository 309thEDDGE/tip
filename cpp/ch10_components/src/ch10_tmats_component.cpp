#include "ch10_tmats_component.h"

Ch10Status Ch10TMATSComponent::Parse(const uint8_t*& data)
{
    // Parse the TMATS CSDW. Currently, data parsed during this call are not used.
    // Keep the call in because it advances the data pointer by the
    // size of the TMATS CSDW.
    ParseElements(tmats_elems_vec_, data);

    // Calculate TMATS body (read: payload) size and save string
    // in vector that is passed in as argument.
    int tmats_byte_length = static_cast<int>(ctx_->data_size) - static_cast<int>(tmats_csdw_elem_.size);
    if (tmats_byte_length > 0)
    {
        std::string tmats_body(reinterpret_cast<const char*>(data), 
            static_cast<size_t>(tmats_byte_length));
        ctx_->AddTMATSMatter(tmats_body);
    }

    return Ch10Status::OK;
}