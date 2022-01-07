#include "icd_element.h"

// const int ICDElement::kFillElementCount = 27;

ICDElement::ICDElement() : msg_name_(""), elem_name_(""), xmit_word_(UINT16_MAX), dest_word_(UINT16_MAX), msg_word_count_(UINT8_MAX), bus_name_(""), xmit_lru_name_(""), xmit_lru_addr_(UINT8_MAX), dest_lru_name_(""), dest_lru_addr_(UINT8_MAX), xmit_lru_subaddr_(UINT8_MAX), dest_lru_subaddr_(UINT8_MAX), rate_(0.0), offset_(UINT8_MAX), elem_word_count_(UINT8_MAX), schema_(ICDElementSchema::BAD), is_bitlevel_(false), is_multiformat_(false), bitmsb_(UINT8_MAX), bitlsb_(UINT8_MAX), bit_count_(UINT8_MAX), classification_(0), description_(""), msb_val_(0.0), uom_(""), pt(), label_(0), sdi_(-1), channel_id_(UINT16_MAX)
{
}

ICDElement::~ICDElement()
{
}

ICDElement::ICDElement(const ICDElement& C)
{
    msg_name_ = C.msg_name_;
    elem_name_ = C.elem_name_;
    xmit_word_ = C.xmit_word_;
    dest_word_ = C.dest_word_;
    msg_word_count_ = C.msg_word_count_;
    bus_name_ = C.bus_name_;
    xmit_lru_name_ = C.xmit_lru_name_;
    xmit_lru_addr_ = C.xmit_lru_addr_;
    dest_lru_name_ = C.dest_lru_name_;
    dest_lru_addr_ = C.dest_lru_addr_;
    xmit_lru_subaddr_ = C.xmit_lru_subaddr_;
    dest_lru_subaddr_ = C.dest_lru_subaddr_;
    rate_ = C.rate_;
    offset_ = C.offset_;
    elem_word_count_ = C.elem_word_count_;
    schema_ = C.schema_;
    is_bitlevel_ = C.is_bitlevel_;
    is_multiformat_ = C.is_multiformat_;
    bitmsb_ = C.bitmsb_;
    bitlsb_ = C.bitlsb_;
    bit_count_ = C.bit_count_;
    classification_ = C.classification_;
    description_ = C.description_;
    msb_val_ = C.msb_val_;
    uom_ = C.uom_;
    channel_id_ = C.channel_id_;
    label_ = C.label_;
    sdi_ = C.sdi_;
}

ICDElement& ICDElement::operator=(const ICDElement& C)
{
    msg_name_ = C.msg_name_;
    elem_name_ = C.elem_name_;
    xmit_word_ = C.xmit_word_;
    dest_word_ = C.dest_word_;
    msg_word_count_ = C.msg_word_count_;
    bus_name_ = C.bus_name_;
    xmit_lru_name_ = C.xmit_lru_name_;
    xmit_lru_addr_ = C.xmit_lru_addr_;
    dest_lru_name_ = C.dest_lru_name_;
    dest_lru_addr_ = C.dest_lru_addr_;
    xmit_lru_subaddr_ = C.xmit_lru_subaddr_;
    dest_lru_subaddr_ = C.dest_lru_subaddr_;
    rate_ = C.rate_;
    offset_ = C.offset_;
    elem_word_count_ = C.elem_word_count_;
    schema_ = C.schema_;
    is_bitlevel_ = C.is_bitlevel_;
    is_multiformat_ = C.is_multiformat_;
    bitmsb_ = C.bitmsb_;
    bitlsb_ = C.bitlsb_;
    bit_count_ = C.bit_count_;
    classification_ = C.classification_;
    description_ = C.description_;
    msb_val_ = C.msb_val_;
    uom_ = C.uom_;
    channel_id_ = C.channel_id_;
    label_ = C.label_;
    sdi_ = C.sdi_;

    return *this;
}

bool ICDElement::Fill(const std::string& icdelem_str)
{
    // Input string can't be empty.
    if (icdelem_str == "")
        return false;

    // Remove the last character if trailing comma or if trailing comma
    // followed by newline.
    std::string in_str = icdelem_str;
    if (pt.CheckForExisitingChar(in_str, '\n'))
    {
        in_str = pt.RemoveTrailingChar(in_str, '\n');
    }

    // Remove trailing comma if present.
    in_str = pt.RemoveTrailingChar(in_str, ',');

    // Split input string.
    std::vector<std::string> split_data = pt.Split(in_str, ',');

    // Input string must currently have 27 elements.
    if (split_data.size() != kFillElementCount)
    {
        //printf("split_data count: %zu\n", split_data.size());
        return false;
    }

    // Fill class member data. Cast as necessary.
    if (!FillElements(split_data))
    {
        return false;
    }

    return true;
}

bool ICDElement::FillElements(const std::vector<std::string>& input_str_vec)
{
    int int_val = 0;
    double double_val = 0.0;
    std::string curr_str = "";

    // string, msg_name
    curr_str = input_str_vec[0];
    msg_name_ = curr_str;

    // string, elem_name
    curr_str = input_str_vec[1];
    elem_name_ = curr_str;

    // uint16, xmit_word
    curr_str = input_str_vec[2];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at xmit_name\n");
        return false;
    }
    xmit_word_ = int_val;

    // uint16, dest_word
    curr_str = input_str_vec[3];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at dest_word\n");
        return false;
    }
    dest_word_ = int_val;

    // uint8, msg_word_count_
    curr_str = input_str_vec[4];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at msg_word_count\n");
        return false;
    }
    msg_word_count_ = int_val;

    // string, bus_name
    curr_str = input_str_vec[5];
    bus_name_ = curr_str;

    // string, xmit_lru_name_
    curr_str = input_str_vec[6];
    xmit_lru_name_ = curr_str;

    // uint8, xmit_lru_addr_
    curr_str = input_str_vec[7];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at xmit_lru_addr\n");
        return false;
    }
    xmit_lru_addr_ = int_val;

    // string, dest_lru_name_
    curr_str = input_str_vec[8];
    dest_lru_name_ = curr_str;

    // uint8, dest_lru_addr_
    curr_str = input_str_vec[9];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at dest_lru_addr\n");
        return false;
    }
    dest_lru_addr_ = int_val;

    // uint8, xmit_lru_subaddr_
    curr_str = input_str_vec[10];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at xmit_lru_subaddr\n");
        return false;
    }
    xmit_lru_subaddr_ = int_val;

    // uint8, dest_lru_subaddr_
    curr_str = input_str_vec[11];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at dest_lru_subaddr\n");
        return false;
    }
    dest_lru_subaddr_ = int_val;

    // float, rate_
    curr_str = input_str_vec[12];
    if (!pt.ConvertDouble(curr_str, double_val))
    {
        printf("ICDElement::FillElements(): returning at rate\n");
        return false;
    }
    rate_ = double_val;

    // uint8, offset_
    curr_str = input_str_vec[13];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at offset\n");
        return false;
    }
    offset_ = int_val;

    // uint8, elem_word_count_
    curr_str = input_str_vec[14];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at elem_word_count\n");
        return false;
    }
    elem_word_count_ = int_val;

    // ICDElementSchema
    curr_str = input_str_vec[15];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at schema\n");
        return false;
    }
    if (int_val > kICDElementSchemaMaxValue)
    {
        printf("ICDElement::FillElements(): schema too large\n");
        return false;
    }
    schema_ = static_cast<ICDElementSchema>(int_val);

    // bool, is_bitlevel_
    curr_str = input_str_vec[16];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at is_bitlevel\n");
        return false;
    }
    if (int_val < 0 || int_val > 1)
    {
        printf("ICDElement::FillElements(): is_bitlevel not 0 or 1\n");
        return false;
    }
    is_bitlevel_ = static_cast<bool>(int_val);

    // bool, is_multiformat_
    curr_str = input_str_vec[17];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at is_multiformat\n");
        return false;
    }
    if (int_val < 0 || int_val > 1)
    {
        printf("ICDElement::FillElements(): is_multiformat not 0 or 1\n");
        return false;
    }
    is_multiformat_ = static_cast<bool>(int_val);

    // uint8, bitmsb_
    curr_str = input_str_vec[18];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at bitmsb\n");
        return false;
    }
    if (is_bitlevel_)
    {
        if (int_val < 1 || int_val > 16)
        {
            printf("ICDElement::FillElements(): bitmsb not in [1, 16]\n");
            return false;
        }
    }
    bitmsb_ = int_val;

    // uint8, bitlsb_
    curr_str = input_str_vec[19];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at bitlsb\n");
        return false;
    }
    if (is_bitlevel_)
    {
        if (int_val < 1 || int_val > 16)
        {
            printf("ICDElement::FillElements(): bitlsb not in [1, 16]\n");
            return false;
        }
    }
    bitlsb_ = int_val;

    // uint8, bit_count_
    curr_str = input_str_vec[20];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at bit_count\n");
        return false;
    }
    bit_count_ = int_val;

    // uint8, classification_
    curr_str = input_str_vec[21];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at classification\n");
        return false;
    }
    classification_ = int_val;

    // string, description_
    curr_str = input_str_vec[22];
    description_ = curr_str;

    // double, msb_val_
    curr_str = input_str_vec[23];
    if (!pt.ConvertDouble(curr_str, double_val))
    {
        printf("ICDElement::FillElements(): returning at msb_val\n");
        return false;
    }
    msb_val_ = double_val;

    // string, uom_
    curr_str = input_str_vec[24];
    uom_ = curr_str;

    // uint16_t label_ 26
    curr_str = input_str_vec[26];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at label_\n");
        return false;
    }
    label_ = int_val;

    // int8_t, sdi_ 27
    curr_str = input_str_vec[27];
    if (!pt.ConvertInt(curr_str, int_val))
    {
        printf("ICDElement::FillElements(): returning at sdi\n");
        return false;
    }
    sdi_ = int_val;

    return true;
}
